"""Reset the item flags of a location in your save so its items can be picked up again.

Usage:
    python reset_item_flags.py               list locations, then type a number to reset it
    python reset_item_flags.py 12 [15 ...]   reset those location numbers and exit
    python reset_item_flags.py --list        print the location list and exit

Options:
    --save PATH    save file to edit (default: the .sav in the project root)
    --dry-run      show what would be reset without writing anything
    --probe        recompute the save layout with agbcc through WSL (after SaveBlock changes)

A location is a region-map section, so "ROUTE 104" also covers the houses on that route
(maps without a section, like the Johto towers, are grouped per building). Item flags are the
flags of item balls (object events whose script runs finditem) and of hidden items
(hidden_item bg_events). Daily rotating hidden items are skipped: they share one flag per
group and refresh by themselves every day.

The newest save slot is edited in place with its sector checksums fixed, after a backup is
copied to tech/save_backups/. Close the game (or reset the emulator without saving) before
running this, or the next in-game save will overwrite the change. If you saved inside the
location you reset, leave the map and come back for the item balls to reappear.
"""
import argparse
import glob
import json
import os
import re
import shutil
import struct
import subprocess
import sys
import time

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BACKUP_DIR = os.path.join(ROOT, 'tech', 'save_backups')

# include/save.h
SECTOR_SIZE = 0x1000
SECTOR_DATA_SIZE = 3968
NUM_SECTORS_PER_SLOT = 14
NUM_SAVE_SLOTS = 2
SECTOR_SIGNATURE = 0x08012025
FOOTER_ID, FOOTER_CHECKSUM, FOOTER_SIGNATURE, FOOTER_COUNTER = 0xFF4, 0xFF6, 0xFF8, 0xFFC
SECTOR_ID_SAVEBLOCK1_START = 1

# Struct layout as compiled by agbcc. The offset comments in include/global.h are stale, so
# these come from offsetof/sizeof (run with --probe to recompute them after struct changes).
LAYOUT = {
    'sb1_flags': 0x12A8,         # offsetof(struct SaveBlock1, flags)
    'sb1_mod_flags': 0x3658,     # offsetof(struct SaveBlock1, unused_3598), holds MOD_FLAGS
    'sb1_location': 0x4,         # offsetof(struct SaveBlock1, location)
    'sizeof_sb1': 15872,
    'sizeof_sb2': 3892,
    'sizeof_storage': 33744,
    'num_flag_bytes': 434,
    'mod_flags_start': 0xE00,
    'mod_flags_end': 0x1377,
}
PROBE_FIELDS = [
    ('sb1_flags', 'offsetof(struct SaveBlock1, flags)'),
    ('sb1_mod_flags', 'offsetof(struct SaveBlock1, unused_3598)'),
    ('sb1_location', 'offsetof(struct SaveBlock1, location)'),
    ('sizeof_sb1', 'sizeof(struct SaveBlock1)'),
    ('sizeof_sb2', 'sizeof(struct SaveBlock2)'),
    ('sizeof_storage', 'sizeof(struct PokemonStorage)'),
    ('num_flag_bytes', 'NUM_FLAG_BYTES'),
    ('mod_flags_start', 'MOD_FLAGS_START'),
    ('mod_flags_end', 'MOD_FLAGS_END'),
]
SPECIAL_FLAGS_START = 0x4000  # flags at or above this live in RAM only and are never saved

REGION_ORDER = ['hoenn', 'kanto', 'johto', 'sevii', 'other']
REGION_TITLES = {'hoenn': 'Hoenn', 'kanto': 'Kanto', 'johto': 'Johto', 'sevii': 'Sevii Islands', 'other': 'Other'}
REGION_LAYOUT_FILES = [  # same order as night_map.py / GetFlyRegionForMapsec; Hoenn is the fallback
    ('johto', 'region_map_layout_johto.h'),
    ('kanto', 'region_map_layout_kanto.h'),
    ('sevii', 'region_map_layout_sevii_123.h'),
    ('sevii', 'region_map_layout_sevii_45.h'),
    ('sevii', 'region_map_layout_sevii_67.h'),
    ('hoenn', 'region_map_layout.h'),
]
# Locations whose maps are shared between regions (Kanto's Victory Road and Mahogany's hideout
# reuse these maps), so the map graph can't tell where they belong.
LOCATION_OVERRIDES = {
    'MAPSEC_VICTORY_ROAD': ('hoenn', None),
    'MAPSEC_ROCKET_HIDEOUT': ('kanto', None),
    'MAPSEC_DYNAMIC:SSTidalLowerDeck': ('hoenn', 'S.S. TIDAL'),
}


def read_text(rel):
    with open(os.path.join(ROOT, rel), encoding='utf-8', errors='replace') as f:
        return f.read()


# ---------------------------------------------------------------- constants

class Defines:
    """Lazy evaluator for the integer #defines in include/constants (enough for flag IDs)."""

    def __init__(self):
        self.exprs = {}
        self.cache = {}
        files = ['include/constants/flags.h'] + sorted(
            os.path.relpath(p, ROOT) for p in glob.glob(os.path.join(ROOT, 'include', 'constants', '*.h')))
        for rel in files:
            text = read_text(rel).replace('\\\n', ' ')
            text = re.sub(r'/\*.*?\*/', ' ', text, flags=re.S)
            text = re.sub(r'//[^\n]*', '', text)
            for m in re.finditer(r'^[ \t]*#[ \t]*define[ \t]+(\w+)[ \t]+([^\n]+)', text, re.M):
                self.exprs.setdefault(m.group(1), m.group(2).strip())

    def __getitem__(self, name):
        if name not in self.cache:
            self.cache[name] = None  # guards against recursive definitions
            expr = self.exprs[name]
            expr = re.sub(r'\b(0[xX][0-9a-fA-F]+|\d+)[uUlL]+\b', r'\1', expr)
            expr = re.sub(r'\b[A-Za-z_]\w*\b', lambda m: str(self[m.group(0)]), expr)
            expr = expr.replace('/', '//')
            self.cache[name] = int(eval(expr, {'__builtins__': {}}))
        if self.cache[name] is None:
            raise ValueError(f'recursive #define {name}')
        return self.cache[name]

    def get(self, name):
        try:
            return self[name]
        except Exception:
            return None


def probe_layout():
    """Compile a tiny file with the project's agbcc (through WSL) and read back offsetof/sizeof."""
    src = ('#include "global.h"\n#include "pokemon_storage_system.h"\n#include "constants/flags.h"\n'
           'const u32 probe[] = {\n' + ''.join(f'    {expr},\n' for _, expr in PROBE_FIELDS) + '};\n')
    try:
        wsl_root = subprocess.run(['wsl', '-e', 'wslpath', '-a', ROOT], capture_output=True, text=True,
                                  check=True).stdout.strip()
        cmd = (f"cd '{wsl_root}' && cpp -iquote include -Wno-trigraphs -DMODERN=0 -I tools/agbcc/include "
               "-I tools/agbcc -nostdinc -undef -std=gnu89 - | tools/agbcc/bin/agbcc -O2 -o - -")
        res = subprocess.run(['wsl', '-e', 'bash', '-c', cmd], input=src, capture_output=True, text=True)
    except (OSError, subprocess.CalledProcessError) as e:
        sys.exit(f'Could not run WSL for --probe: {e}')
    words = [int(w, 0) for w in re.findall(r'\.word\s+(-?(?:0x[0-9a-fA-F]+|\d+))', res.stdout)]
    if res.returncode or len(words) != len(PROBE_FIELDS):
        sys.exit('Layout probe failed:\n' + (res.stderr or res.stdout))
    return dict(zip((k for k, _ in PROBE_FIELDS), words))


def check_layout(defs):
    """Catch the most likely way the hardcoded layout goes stale: the flag count changing."""
    flags_count = defs.get('FLAGS_COUNT')
    expect = {
        'num_flag_bytes': (flags_count + 7) // 8 if flags_count is not None else None,
        'mod_flags_start': defs.get('MOD_FLAGS_START'),
        'mod_flags_end': defs.get('MOD_FLAGS_END'),
    }
    return [f'{k}: script has {LAYOUT[k]}, headers say {v}' for k, v in expect.items()
            if v is not None and v != LAYOUT[k]]


# ---------------------------------------------------------------- locations

def load_script_bodies():
    bodies = {}
    paths = glob.glob(os.path.join(ROOT, 'data', 'maps', '*', 'scripts.inc'))
    paths += glob.glob(os.path.join(ROOT, 'data', 'scripts', '*.inc'))
    paths += glob.glob(os.path.join(ROOT, 'data', '*.s'))
    label = re.compile(r'^(\w+)::?[ \t]*$', re.M)
    for p in paths:
        with open(p, encoding='utf-8', errors='replace') as f:
            text = f.read()
        marks = list(label.finditer(text))
        for i, m in enumerate(marks):
            end = marks[i + 1].start() if i + 1 < len(marks) else len(text)
            bodies.setdefault(m.group(1), text[m.end():end])
    return bodies


def item_name(const):
    if not const or not const.startswith('ITEM_'):
        return const or '?'
    words = const[5:].split('_')
    return ' '.join(w if re.fullmatch(r'(TM|HM)\d+', w) else w.capitalize() for w in words)


def mapsec_regions():
    """MAPSEC -> set of regions whose region map shows it (some sections appear on several)."""
    out = {}
    for region, fname in REGION_LAYOUT_FILES:
        for sec in re.findall(r'\bMAPSEC_\w+', read_text('src/data/region_map/' + fname)):
            if sec != 'MAPSEC_NONE':
                regions = out.setdefault(sec, [])
                if region not in regions:
                    regions.append(region)
    return out


def load_locations(defs):
    """Return (locations, map_index). Each location: dict(mapsec, name, region, maps, items)."""
    groups = json.loads(read_text('data/maps/map_groups.json'))
    map_index = {}  # (group, num) -> map dir name
    map_group_of = {}
    for g, gname in enumerate(groups['group_order']):
        for n, mdir in enumerate(groups[gname]):
            map_index[(g, n)] = mdir
            map_group_of[mdir] = gname

    sections = json.loads(read_text('src/data/region_map/region_map_sections.json'))['map_sections']
    sec_names = {s['id']: s.get('name') or s['id'][7:].replace('_', ' ') for s in sections}
    sec_order = {s['id']: i for i, s in enumerate(sections)}
    sec_region = mapsec_regions()
    bodies = load_script_bodies()

    maps = {}
    for p in glob.glob(os.path.join(ROOT, 'data', 'maps', '*', 'map.json')):
        with open(p, encoding='utf-8') as f:
            maps[os.path.basename(os.path.dirname(p))] = json.load(f)

    locs = {}
    unresolved = []
    for mdir, m in sorted(maps.items()):
        sec = m.get('region_map_section', 'MAPSEC_NONE')
        key, name = sec, sec_names.get(sec, sec)
        if sec in ('MAPSEC_NONE', 'MAPSEC_DYNAMIC'):
            # Maps without a real section (e.g. Johto towers): group floors of the same building.
            family = mdir.split('_')[0]
            key, name = f'{sec}:{family}', re.sub(r'(?<=[a-z])(?=[A-Z0-9])', ' ', family).upper()
        found = []
        for o in m.get('object_events') or []:
            flag = o.get('flag', '0')
            body = bodies.get(o.get('script', ''), '')
            if flag in ('0', '') or not re.search(r'\bfinditem\b|\bSTD_FIND_ITEM\b', body):
                continue
            item = re.search(r'\bfinditem\s+(ITEM_\w+)', body)
            found.append((flag, item.group(1) if item else None, 'item ball'))
        for b in m.get('bg_events') or []:
            if b.get('type') == 'hidden_item' and b.get('flag'):
                found.append((b['flag'], b.get('item'), 'hidden'))
        for flag, item, kind in found:
            fid = defs.get(flag)
            if fid is None:
                unresolved.append(flag)
                continue
            if fid == 0 or (fid >= SPECIAL_FLAGS_START):
                continue  # not a saved flag (daily hidden items, special flags)
            loc = locs.setdefault(key, {'mapsec': sec, 'name': name.replace('é', 'E').replace('É', 'E'),
                                        'maps': set(), 'items': []})
            loc['maps'].add(mdir)
            if all(i['flag'] != flag for i in loc['items']):
                loc['items'].append({'flag': flag, 'id': fid, 'item': item_name(item), 'map': mdir, 'kind': kind})

    if unresolved:
        print(f'  warning: could not resolve {len(unresolved)} flag(s), e.g. {unresolved[0]}')

    # Region per map: Johto map group, else the one region map that shows its section, else the
    # nearest maps it connects or warps to (caves/buildings not on a region map, and sections
    # shared by several region maps). Johto's region map reuses some Hoenn/Kanto sections
    # (MAPSEC_VICTORY_ROAD, MAPSEC_ROCKET_HIDEOUT), so it only counts for Johto-group maps.
    map_region = {}
    for mdir, m in maps.items():
        sec = m.get('region_map_section', '')
        if map_group_of.get(mdir) == 'gMapGroup_Johto':
            map_region[mdir] = 'johto'
            continue
        regions = [r for r in sec_region.get(sec, []) if r != 'johto']
        if len(regions) == 1:
            map_region[mdir] = regions[0]
        elif sec.startswith('MAPSEC_KANTO_'):
            map_region[mdir] = 'kanto'
    dir_of = {m.get('id'): mdir for mdir, m in maps.items()}
    neighbors = {mdir: set() for mdir in maps}
    for mdir, m in maps.items():
        near = [c.get('map') for c in m.get('connections') or []]
        near += [w.get('dest_map') for w in m.get('warp_events') or []]
        for n in near:
            if dir_of.get(n) in neighbors and dir_of[n] != mdir:
                neighbors[mdir].add(dir_of[n])
                neighbors[dir_of[n]].add(mdir)
    def pick(votes):
        return max(votes, key=lambda r: (votes[r], -REGION_ORDER.index(r)))

    while True:
        found = {}
        for mdir in maps:
            if mdir in map_region:
                continue
            votes = {}
            for n in neighbors[mdir]:
                if n in map_region:
                    votes[map_region[n]] = votes.get(map_region[n], 0) + 1
            if votes:
                found[mdir] = pick(votes)
        if not found:
            break
        map_region.update(found)
    for mdir, m in maps.items():
        if mdir not in map_region and sec_region.get(m.get('region_map_section')):
            map_region[mdir] = sec_region[m['region_map_section']][0]
    for key, loc in locs.items():
        votes = {}
        for mdir in loc['maps']:
            r = map_region.get(mdir, 'other')
            votes[r] = votes.get(r, 0) + 1
        loc['region'] = pick(votes)
        region, name = LOCATION_OVERRIDES.get(key, (None, None))
        loc['region'] = region or loc['region']
        loc['name'] = name or loc['name']

    # Same name twice in a region (UNDERWATER, UNDERGROUND PATH): fall back to the section ID.
    seen = {}
    for loc in locs.values():
        seen.setdefault((loc['region'], loc['name']), []).append(loc)
    for dupes in seen.values():
        if len(dupes) > 1:
            for loc in dupes:
                loc['name'] = loc['mapsec'][len('MAPSEC_'):].replace('_', ' ')

    ordered = sorted(locs.values(), key=lambda l: (REGION_ORDER.index(l['region']),
                                                   sec_order.get(l['mapsec'], 10 ** 6), l['name']))
    for i, loc in enumerate(ordered, 1):
        loc['num'] = i
    return ordered, map_index


# ---------------------------------------------------------------- save file

def checksum(data):
    total = sum(struct.unpack_from(f'<{len(data) // 4}I', data)) & 0xFFFFFFFF
    return ((total >> 16) + total) & 0xFFFF


def chunk_sizes():
    """Bytes of data stored in each logical sector, as sSaveSlotLayout in src/save.c."""
    def chunks(size, n):
        return [max(0, min(size - k * SECTOR_DATA_SIZE, SECTOR_DATA_SIZE)) for k in range(n)]
    return chunks(LAYOUT['sizeof_sb2'], 1) + chunks(LAYOUT['sizeof_sb1'], 4) + chunks(LAYOUT['sizeof_storage'], 9)


class SaveFile:
    def __init__(self, path):
        self.path = path
        with open(path, 'rb') as f:
            self.data = bytearray(f.read())
        if len(self.data) < NUM_SAVE_SLOTS * NUM_SECTORS_PER_SLOT * SECTOR_SIZE:
            raise ValueError(f'{os.path.basename(path)} is only {len(self.data)} bytes; expected a 128 KB flash save')
        self.sizes = chunk_sizes()
        status = [self._slot_status(s) for s in range(NUM_SAVE_SLOTS)]
        ok = [s for s in range(NUM_SAVE_SLOTS) if status[s][0] == 'ok']
        if not ok:
            raise ValueError('no valid save slot found (empty or corrupt save, or the save layout changed)')
        if len(ok) == 2:
            c1, c2 = status[0][1], status[1][1]
            if (c1, c2) in ((0xFFFFFFFF, 0), (0, 0xFFFFFFFF)):
                self.slot = 1 if (c1 + 1) & 0xFFFFFFFF < (c2 + 1) & 0xFFFFFFFF else 0
            else:
                self.slot = 1 if c1 < c2 else 0
        else:
            self.slot = ok[0]
        self.counter = status[self.slot][1]
        self.sector_of = status[self.slot][2]  # logical id -> absolute sector index

    def _slot_status(self, slot):
        valid, counter, where = 0, 0, {}
        for i in range(NUM_SECTORS_PER_SLOT):
            base = (slot * NUM_SECTORS_PER_SLOT + i) * SECTOR_SIZE
            sid, chk, sig, cnt = struct.unpack_from('<HHII', self.data, base + FOOTER_ID)
            if sig != SECTOR_SIGNATURE or sid >= NUM_SECTORS_PER_SLOT:
                continue
            if chk == checksum(self.data[base:base + self.sizes[sid]]):
                valid |= 1 << sid
                counter = cnt
                where[sid] = slot * NUM_SECTORS_PER_SLOT + i
        return ('ok' if valid == (1 << NUM_SECTORS_PER_SLOT) - 1 else 'bad', counter, where)

    def _sb1_addr(self, offset):
        chunk, local = divmod(offset, SECTOR_DATA_SIZE)
        sector = self.sector_of[SECTOR_ID_SAVEBLOCK1_START + chunk]
        return sector, sector * SECTOR_SIZE + local

    def _flag_addr(self, fid):
        if LAYOUT['mod_flags_start'] <= fid <= LAYOUT['mod_flags_end']:
            offset = LAYOUT['sb1_mod_flags'] + (fid - LAYOUT['mod_flags_start']) // 8
        else:  # mirrors GetFlagPointer in src/event_data.c
            offset = LAYOUT['sb1_flags'] + fid // 8
        return self._sb1_addr(offset)

    def get_flag(self, fid):
        return bool(self.data[self._flag_addr(fid)[1]] & (1 << (fid & 7)))

    def clear_flag(self, fid):
        sector, addr = self._flag_addr(fid)
        self.data[addr] &= ~(1 << (fid & 7)) & 0xFF
        return sector

    def location(self):
        _, addr = self._sb1_addr(LAYOUT['sb1_location'])
        return struct.unpack_from('<bb', self.data, addr)

    def fix_checksums(self, sectors):
        for sector in sectors:
            base = sector * SECTOR_SIZE
            sid = struct.unpack_from('<H', self.data, base + FOOTER_ID)[0]
            struct.pack_into('<H', self.data, base + FOOTER_CHECKSUM, checksum(self.data[base:base + self.sizes[sid]]))

    def write(self):
        os.makedirs(BACKUP_DIR, exist_ok=True)
        stem = os.path.splitext(os.path.basename(self.path))[0]
        backup = os.path.join(BACKUP_DIR, f'{stem}_{time.strftime("%Y%m%d_%H%M%S")}.sav')
        shutil.copy2(self.path, backup)
        with open(self.path, 'r+b') as f:
            f.write(self.data)
        return backup


def find_save(explicit):
    if explicit:
        return explicit if os.path.isfile(explicit) else sys.exit(f'Save file not found: {explicit}')
    saves = sorted(glob.glob(os.path.join(ROOT, '*.sav')))
    if not saves:
        sys.exit(f'No .sav file found in {ROOT}')
    preferred = os.path.join(ROOT, 'pokeemerald.sav')
    if len(saves) > 1 and preferred not in saves:
        print('  Several save files found; using the most recently modified one (pass --save to choose).')
    if preferred in saves:
        return preferred
    return max(saves, key=os.path.getmtime)


# ---------------------------------------------------------------- UI

def print_locations(locs, save, filt=None):
    width = shutil.get_terminal_size((120, 40)).columns
    entries = {}
    for loc in locs:
        if filt and filt not in loc['name'].lower() and filt not in loc['mapsec'].lower():
            continue
        got = sum(save.get_flag(i['id']) for i in loc['items'])
        text = f"{loc['num']:>4}  {loc['name'][:24]:<24} {got:>2}/{len(loc['items']):<2}"
        entries.setdefault(loc['region'], []).append(text)
    if not entries:
        print('  No matching locations.')
        return
    cell = max(len(t) for ts in entries.values() for t in ts) + 3
    cols = max(1, (width - 1) // cell)
    for region in REGION_ORDER:
        items = entries.get(region)
        if not items:
            continue
        print(f'\n  {REGION_TITLES[region]}')
        rows = (len(items) + cols - 1) // cols
        for r in range(rows):
            print(''.join(items[c * rows + r].ljust(cell) for c in range(cols) if c * rows + r < len(items)).rstrip())
    print('\n  (collected / total item flags)')


def reset_locations(nums, locs, save, map_index, dry_run):
    by_num = {l['num']: l for l in locs}
    bad = [n for n in nums if n not in by_num]
    if bad:
        print(f"  Unknown location number(s): {', '.join(map(str, bad))}")
        return False
    touched = set()
    cleared = 0
    here = map_index.get(save.location())
    for n in nums:
        loc = by_num[n]
        was_set = [i for i in loc['items'] if save.get_flag(i['id'])]
        print(f"\n  {loc['num']}. {loc['name']} ({REGION_TITLES[loc['region']]}): "
              f"{len(was_set)} of {len(loc['items'])} item flag(s) to reset")
        for i in sorted(loc['items'], key=lambda i: (i['map'], i['kind'], i['item'])):
            mark = 'x' if i in was_set else ' '
            print(f"    [{mark}] {i['map'][:28]:<28} {i['item'][:18]:<18} {i['kind']:<9} {i['flag']}")
        for i in was_set:
            touched.add(save.clear_flag(i['id']))
        cleared += len(was_set)
        if was_set and here in loc['maps']:
            print(f'  Note: the save is on {here}; leave the map and come back for the items to reappear.')
    if not cleared:
        print('\n  Nothing to reset - every item there is still uncollected.')
        return True
    if dry_run:
        print(f'\n  Dry run: {cleared} flag(s) would be cleared. Save not modified.')
        return True
    save.fix_checksums(touched)
    backup = save.write()
    check = SaveFile(save.path)
    left = [i for n in nums for i in by_num[n]['items'] if check.get_flag(i['id'])]
    if left or check.slot != save.slot:
        sys.exit(f'  Verification failed after writing; restore the backup: {backup}')
    print(f'\n  Cleared {cleared} flag(s) in {os.path.basename(save.path)} (slot {save.slot + 1}).')
    print(f'  Backup: {os.path.relpath(backup, ROOT)}')
    return True


def parse_numbers(text):
    tokens = re.split(r'[,\s]+', text.strip())
    if tokens and all(t.isdigit() for t in tokens if t):
        return [int(t) for t in tokens if t]
    return None


def main():
    if hasattr(sys.stdout, 'reconfigure'):
        sys.stdout.reconfigure(errors='replace')
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('locations', nargs='*', type=int, help='location numbers to reset')
    ap.add_argument('--save', help='save file to edit')
    ap.add_argument('--list', action='store_true', help='print the location list and exit')
    ap.add_argument('--dry-run', action='store_true', help="don't write the save")
    ap.add_argument('--probe', action='store_true', help='recompute the save layout with agbcc via WSL')
    args = ap.parse_args()

    defs = Defines()
    problems = check_layout(defs)
    if args.probe or problems:
        if problems:
            print('  Save layout looks stale (' + '; '.join(problems) + '), probing with agbcc...')
        LAYOUT.update(probe_layout())
        print('  Layout: ' + ', '.join(f'{k}={v:#x}' for k, v in LAYOUT.items()))
        print('  (update LAYOUT in tech/reset_item_flags.py with these values to skip probing)')

    path = find_save(args.save)
    try:
        save = SaveFile(path)
    except ValueError as e:
        sys.exit(f'  {path}: {e}')
    locs, map_index = load_locations(defs)
    here = map_index.get(save.location(), 'unknown map')
    print(f'  Save: {path}  (slot {save.slot + 1}, save count {save.counter}, on {here})')

    if args.locations:
        ok = reset_locations(args.locations, locs, save, map_index, args.dry_run)
        sys.exit(0 if ok else 1)
    print_locations(locs, save)
    if args.list:
        return

    while True:
        try:
            text = input('\n  Location number(s) to reset, text to search, L to list, Q to quit: ').strip()
        except (EOFError, KeyboardInterrupt):
            print()
            return
        if not text:
            continue
        if text.lower() in ('q', 'quit', 'exit'):
            return
        if text.lower() in ('l', 'list'):
            print_locations(locs, save)
            continue
        nums = parse_numbers(text)
        if nums is None:
            print_locations(locs, save, text.lower())
            continue
        reset_locations(nums, locs, save, map_index, args.dry_run)
        save = SaveFile(path)


if __name__ == '__main__':
    main()
