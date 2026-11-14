"""Export every map's wild encounters to a spreadsheet, with non-Hoenn Pokemon marked in red.

Usage:
    python encounter_table.py [--ref BRANCH] [--out DIR]

Writes wild_encounters.csv and wild_encounters.xlsx to tech/encounter_tables/ (or --out).
One row per map and method; each Pokemon gets one cell with its level range and chance:
  - Grass/Cave, Surfing, Rock Smash, Old/Good/Super Rod: src/data/wild_encounters.json. The chance
    is the slot odds for that method (repeated slots are combined); Encounter Rate is the map's rate.
  - Swarm (Grass/Cave, Surfing, Fishing): the swarm lists in src/tv.c. Only one swarm runs at a
    time; while it does, it takes Encounter Rate (50%) of that method's encounters on the map.
  - Grotto Pool 1/2: src/data/hidden_grottos.h, for maps with a grotto entrance. Encounter Rate is
    the entrance's daily odds of rolling that pool; each entry in the pool is equally likely.
Season-limited entries are tagged like [Winter]; their chance is left out since it depends on the season.

"Native" means the species is in the Hoenn Dex or found in the Safari Zone of the vanilla game,
read from the given git branch (default: master): include/constants/pokedex.h and the
MAP_SAFARI_ZONE_* entries of src/data/wild_encounters.json. Non-native cells are red in the .xlsx;
since a CSV can't hold colors, the CSV lists them in the "Non-native" column instead.

Regions follow the same rules as night_map.py (region-map section, Johto map group, or neighbors).
"""
import argparse
import csv
import json
import os
import re
import subprocess
import sys
import zipfile
from xml.sax.saxutils import escape

from night_map import ROOT, assign_regions, load_maps, mapsec_regions

ENCOUNTERS_FILE = 'src/data/wild_encounters.json'
SPECIES_NAMES_FILE = 'src/data/text/species_names.h'
POKEDEX_CONSTANTS = 'include/constants/pokedex.h'
DEFAULT_OUT = os.path.join(ROOT, 'tech', 'encounter_tables')
OUT_NAME = 'wild_encounters'

REGION_ORDER = ['hoenn', 'kanto', 'sevii', 'johto']
REGION_TITLES = {'kanto': 'Kanto', 'johto': 'Johto', 'hoenn': 'Hoenn', 'sevii': 'Sevii Islands'}
METHOD_TITLES = {
    'land_mons': 'Grass/Cave',
    'water_mons': 'Surfing',
    'rock_smash_mons': 'Rock Smash',
    'old_rod': 'Old Rod',
    'good_rod': 'Good Rod',
    'super_rod': 'Super Rod',
}
METHOD_RANK = {'land_mons': 0, 'water_mons': 1, 'rock_smash_mons': 2, 'old_rod': 3, 'good_rod': 4,
               'super_rod': 5, 'swarm': 6, 'grotto': 7}
SWARMS_FILE = 'src/tv.c'
SWARM_CHANCE = 50  # show->massOutbreak.probability in src/tv.c: share of encounters taken by the swarm
SWARM_TITLES = {'OUTBREAK_LAND': 'Swarm (Grass/Cave)', 'OUTBREAK_SURF': 'Swarm (Surfing)',
                'OUTBREAK_FISHING': 'Swarm (Fishing)'}
GROTTOS_FILE = 'src/data/hidden_grottos.h'
SEASON_NAMES = [(1, 'Spring'), (2, 'Summer'), (4, 'Fall'), (8, 'Winter')]
SEASON_BITS = {name.upper(): bit for bit, name in SEASON_NAMES}
ALL_SEASONS = 15
FIXED_HEADERS = ['Region', 'Map', 'Method', 'Encounter Rate', 'Non-native']


def read_text(path):
    with open(os.path.join(ROOT, path), encoding='utf-8') as f:
        return f.read()


def git_show(ref, path):
    try:
        return subprocess.run(['git', 'show', f'{ref}:{path}'], cwd=ROOT, capture_output=True,
                              text=True, encoding='utf-8', check=True).stdout
    except (OSError, subprocess.CalledProcessError) as e:
        detail = getattr(e, 'stderr', '') or e
        sys.exit(f'Could not read {path} from branch "{ref}": {str(detail).strip()}')


def safari_zone_species(ref):
    """SPECIES_* constants found in any Safari Zone map's encounters on git branch `ref`."""
    data = json.loads(git_show(ref, ENCOUNTERS_FILE))
    species = set()
    for group in data['wild_encounter_groups']:
        for enc in group['encounters']:
            if enc.get('map', '').startswith('MAP_SAFARI_ZONE'):
                for info in enc.values():
                    if isinstance(info, dict):
                        species.update(m['species'] for m in info.get('mons', []))
    return species


def hoenn_dex_species(ref):
    """SPECIES_* constants in the Hoenn Dex on git branch `ref`."""
    text = git_show(ref, POKEDEX_CONSTANTS)
    # The HOENN_DEX_* enum orders every species; only entries up to HOENN_DEX_COUNT are in the Hoenn Dex.
    last = re.search(r'#define\s+HOENN_DEX_COUNT\s+HOENN_DEX_(\w+)', text)
    names = [n for n in re.findall(r'^\s*HOENN_DEX_(\w+)\s*,', text, re.M) if n != 'NONE']
    if not last or last.group(1) not in names:
        sys.exit(f'Could not find the Hoenn Dex (HOENN_DEX_COUNT) in {ref}:{POKEDEX_CONSTANTS}')
    return {'SPECIES_' + n for n in names[:names.index(last.group(1)) + 1]}


def species_display_names():
    names = {}
    for const, name in re.findall(r'\[(SPECIES_\w+)\]\s*=\s*_\("([^"]*)"\)', read_text(SPECIES_NAMES_FILE)):
        # BULBASAUR -> Bulbasaur, MR. MIME -> Mr. Mime, HO-OH -> Ho-Oh, FARFETCH'D -> Farfetch'd
        names[const] = re.sub(r"(^|[ \-])(\w)(\S*?)(?=$|[ \-])",
                              lambda m: m.group(1) + m.group(2).upper() + m.group(3).lower(), name)
    return names


def species_name(const, names):
    return names.get(const) or const.removeprefix('SPECIES_').replace('_', ' ').title()


def map_display_name(name):
    # GraniteCave_1F -> Granite Cave 1F, Route101 -> Route 101, AlteringCave7 -> Altering Cave 7
    name = re.sub(r'(?<=[a-z])(?=[A-Z0-9])', ' ', name)
    return re.sub(r'\s*_\s*', ' ', name).strip()


def methods(fields, encounter):
    """[(method key, encounter rate, [(slot chance, mon), ...]), ...] for one map."""
    out = []
    for field in fields:
        info = encounter.get(field['type'])
        if not info:
            continue
        mons = info['mons']
        rates = field['encounter_rates']
        groups = field.get('groups') or {field['type']: list(range(len(rates)))}
        for key, slots in groups.items():
            slot_mons = [(rates[i], mons[i]) for i in slots if i < len(mons) and i < len(rates)]
            if slot_mons:
                out.append((key, info.get('encounter_rate', 0), slot_mons))
    return out


def combine_slots(slot_mons):
    """Merge repeated species: [(species, min level, max level, chance %, seasons), ...] in first-seen order."""
    total = sum(chance for chance, _ in slot_mons) or 1
    merged = {}
    for chance, mon in slot_mons:
        sp = mon['species']
        if sp in merged:
            lo, hi, c = merged[sp]
            merged[sp] = (min(lo, mon['min_level']), max(hi, mon['max_level']), c + chance)
        else:
            merged[sp] = (mon['min_level'], mon['max_level'], chance)
    return [(sp, lo, hi, 100 * c / total, ALL_SEASONS) for sp, (lo, hi, c) in merged.items()]


def combine_entries(entries, with_chance):
    """Pool/list entries [(species, level, seasons), ...] -> combine_slots-style tuples, one per species
    and season mask. Each entry is equally likely, so the chance is only shown when every entry
    is available all year (otherwise the odds change with the season)."""
    show = with_chance and all(s == ALL_SEASONS for _, _, s in entries)
    merged = {}
    for sp, level, seasons in entries:
        lo, hi, n = merged.get((sp, seasons), (level, level, 0))
        merged[(sp, seasons)] = (min(lo, level), max(hi, level), n + 1)
    return [(sp, lo, hi, 100 * n / len(entries) if show else None, seasons)
            for (sp, seasons), (lo, hi, n) in merged.items()]


def seasons_label(mask):
    if mask & ALL_SEASONS == ALL_SEASONS:
        return ''
    return '/'.join(name for bit, name in SEASON_NAMES if mask & bit)


def format_mon(name, lo, hi, pct, seasons):
    text = f'{name} ' + (f'Lv{lo}' if lo == hi else f'Lv{lo}-{hi}')
    if pct is not None:
        text += f' ({pct:.0f}%)' if abs(pct - round(pct)) < 0.05 else f' ({pct:.1f}%)'
    label = seasons_label(seasons)
    return text + (f' [{label}]' if label else '')


# ---------------------------------------------------------------- C data (swarms, grottos)

def strip_c_comments(text):
    text = re.sub(r'/\*.*?\*/', ' ', text, flags=re.S)
    return re.sub(r'//[^\n]*', '', text)


def c_arrays(text, struct):
    """{array name: [entry body, ...]} for every `static const struct <struct> name[] = { ... };`."""
    out = {}
    for m in re.finditer(r'\bstruct\s+' + struct + r'\s+(\w+)\s*\[\s*\]\s*=\s*\{(.*?)\n\};', text, re.S):
        out[m.group(1)] = re.findall(r'\{((?:[^{}]|\{[^{}]*\})*)\}', m.group(2))
    return out


def c_field(body, name, defines=None, default=None):
    m = re.search(r'\.' + name + r'\s*=\s*([^,]+?)\s*(?:,|$)', body.strip())
    if not m:
        return default
    value = m.group(1).strip()
    for _ in range(8):
        if defines and value in defines:
            value = defines[value]
    mp = re.fullmatch(r'MAP_NUM\(\s*(MAP_\w+)\s*\)', value)
    return mp.group(1) if mp else value


def c_int(value, default=0):
    try:
        return int(value, 0)
    except (TypeError, ValueError):
        return default


def season_mask(expr):
    if not expr:
        return ALL_SEASONS
    mask = 0
    for tag in re.findall(r'SEASON_TAG_(\w+)', expr):
        mask |= ALL_SEASONS if tag == 'ALL' else SEASON_BITS.get(tag, 0)
    return mask or ALL_SEASONS


def load_swarms():
    """{(map, method key): [(species, level, seasons), ...]} from every swarm list in src/tv.c."""
    text = strip_c_comments(read_text(SWARMS_FILE))
    out = {}
    for name, entries in c_arrays(text, 'OutbreakSpecies').items():
        for body in entries:
            species = c_field(body, 'species')
            maps = re.search(r'\.maps\s*=\s*\{([^}]*)\}', body)
            if not species or not maps:
                continue
            kind = c_field(body, 'encounterType', default='OUTBREAK_LAND')
            for map_id in re.findall(r'MAP_\w+', maps.group(1)):
                out.setdefault((map_id, kind), []).append(
                    (species, c_int(c_field(body, 'level')), season_mask(c_field(body, 'seasons'))))
    return out


def load_grottos():
    """{map: {'chances': [(mon1, mon2), ...], 'seasons': mask, 'pools': {1: [...], 2: [...]}}} for
    every map with a grotto entrance in src/data/hidden_grottos.h."""
    text = strip_c_comments(read_text(GROTTOS_FILE))
    defines = dict(re.findall(r'^\s*#define\s+(\w+)\s+(\S+)', text, re.M))
    arrays = c_arrays(text, 'HiddenGrotto')
    mons = c_arrays(text, 'HiddenGrottoMon')
    out = {}
    for name, entries in arrays.items():
        region = name.removeprefix('sHiddenGrottos')
        for body in entries:
            map_id = c_field(body, 'location')
            if not map_id:
                continue
            g = out.setdefault(map_id, {'chances': [], 'seasons': 0, 'pools': {1: [], 2: []}, 'region': region})
            g['chances'].append(tuple(c_int(c_field(body, f'mon{p}Chance', defines)) for p in (1, 2)))
            g['seasons'] |= season_mask(c_field(body, 'seasons'))
        for pool in (1, 2):
            for body in mons.get(f'sHiddenGrottoMons{pool}{region}', []):
                map_id = c_field(body, 'location')
                if map_id in out and out[map_id]['region'] == region:
                    out[map_id]['pools'][pool].append(
                        (c_field(body, 'species'), c_int(c_field(body, 'level', defines)),
                         season_mask(c_field(body, 'seasons'))))
    return out


# ---------------------------------------------------------------- rows

def build_rows(native, names):
    data = json.loads(read_text(ENCOUNTERS_FILE))
    maps = load_maps()
    assign_regions(maps, mapsec_regions())
    map_order = {map_id: i for i, map_id in enumerate(maps)}
    rows = []

    def add(map_id, method, method_rank, rate, mons):
        mp = maps.get(map_id, {})
        region = mp.get('_region', 'hoenn')
        map_name = map_display_name(mp.get('name') or map_id.removeprefix('MAP_').title())
        cells = [(format_mon(species_name(sp, names), lo, hi, pct, seasons), sp not in native)
                 for sp, lo, hi, pct, seasons in mons]
        foreign = list(dict.fromkeys(species_name(m[0], names) for m in mons if m[0] not in native))
        rows.append({
            'sort': (REGION_ORDER.index(region) if region in REGION_ORDER else len(REGION_ORDER),
                     map_order.get(map_id, len(map_order)), method_rank, len(rows)),
            'fixed': [REGION_TITLES.get(region, region.title()), map_name, method, rate, ', '.join(foreign)],
            'cells': cells,
        })

    for group in data['wild_encounter_groups']:
        if not group.get('for_maps'):
            continue  # Battle Pyramid/Pike placeholders; their real mons are picked in code
        for enc in group['encounters']:
            for key, rate, slot_mons in methods(group['fields'], enc):
                add(enc['map'], METHOD_TITLES.get(key, key), METHOD_RANK.get(key, 0), rate, combine_slots(slot_mons))

    for (map_id, kind), entries in load_swarms().items():
        add(map_id, SWARM_TITLES.get(kind, f'Swarm ({kind})'), METHOD_RANK['swarm'], f'{SWARM_CHANCE}%',
            combine_entries(entries, with_chance=False))

    for map_id, g in load_grottos().items():
        for pool in (1, 2):
            chances = sorted({c[pool - 1] for c in g['chances'] if c[pool - 1] > 0})
            entries = g['pools'][pool]
            if not chances or not entries:
                continue
            method = f'Grotto Pool {pool}'
            label = seasons_label(g['seasons'])
            if label:
                method += f' [{label}]'
            add(map_id, method, METHOD_RANK['grotto'] + pool, '/'.join(f'{c}%' for c in chances),
                combine_entries(entries, with_chance=True))

    rows.sort(key=lambda r: r['sort'])
    return rows

def write_csv(path, headers, rows):
    with open(path, 'w', newline='', encoding='utf-8-sig') as f:
        w = csv.writer(f)
        w.writerow(headers)
        for r in rows:
            w.writerow(r['fixed'] + [text for text, _ in r['cells']])


def col_letter(i):
    s = ''
    i += 1
    while i:
        i, rem = divmod(i - 1, 26)
        s = chr(65 + rem) + s
    return s


def xlsx_cell(ref, value, style):
    s = f' s="{style}"' if style else ''
    if isinstance(value, (int, float)):
        return f'<c r="{ref}"{s}><v>{value}</v></c>'
    if value == '':
        return f'<c r="{ref}"{s}/>' if style else ''
    return f'<c r="{ref}"{s} t="inlineStr"><is><t xml:space="preserve">{escape(str(value))}</t></is></c>'


def write_xlsx(path, headers, rows):
    """Minimal single-sheet .xlsx (no third-party libraries). Style 1 = header, 2 = non-native (red)."""
    widths = [len(h) for h in headers]
    for r in rows:
        for i, v in enumerate(r['fixed'] + [t for t, _ in r['cells']]):
            widths[i] = max(widths[i], len(str(v)))
    last_col = col_letter(len(headers) - 1)
    sheet_rows = ['<row r="1">' + ''.join(xlsx_cell(f'{col_letter(i)}1', h, 1) for i, h in enumerate(headers)) + '</row>']
    for n, r in enumerate(rows, start=2):
        cells = [xlsx_cell(f'{col_letter(i)}{n}', v, 0) for i, v in enumerate(r['fixed'])]
        cells += [xlsx_cell(f'{col_letter(len(r["fixed"]) + i)}{n}', t, 2 if foreign else 0)
                  for i, (t, foreign) in enumerate(r['cells'])]
        sheet_rows.append(f'<row r="{n}">' + ''.join(cells) + '</row>')
    cols = ''.join(f'<col min="{i + 1}" max="{i + 1}" width="{min(w, 60) + 2}" customWidth="1"/>'
                   for i, w in enumerate(widths))
    sheet = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
             '<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">'
             '<sheetViews><sheetView workbookViewId="0"><pane xSplit="3" ySplit="1" topLeftCell="D2" '
             'activePane="bottomRight" state="frozen"/></sheetView></sheetViews>'
             f'<cols>{cols}</cols><sheetData>{"".join(sheet_rows)}</sheetData>'
             f'<autoFilter ref="A1:{last_col}{len(rows) + 1}"/></worksheet>')
    styles = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
              '<styleSheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">'
              '<fonts count="3"><font><sz val="11"/><name val="Calibri"/></font>'
              '<font><b/><sz val="11"/><name val="Calibri"/></font>'
              '<font><b/><sz val="11"/><color rgb="FFFFFFFF"/><name val="Calibri"/></font></fonts>'
              '<fills count="4"><fill><patternFill patternType="none"/></fill><fill><patternFill patternType="gray125"/></fill>'
              '<fill><patternFill patternType="solid"><fgColor rgb="FFD9D9D9"/></patternFill></fill>'
              '<fill><patternFill patternType="solid"><fgColor rgb="FFE02020"/></patternFill></fill></fills>'
              '<borders count="1"><border/></borders>'
              '<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>'
              '<cellXfs count="3"><xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0"/>'
              '<xf numFmtId="0" fontId="1" fillId="2" borderId="0" xfId="0" applyFont="1" applyFill="1"/>'
              '<xf numFmtId="0" fontId="2" fillId="3" borderId="0" xfId="0" applyFont="1" applyFill="1"/></cellXfs>'
              '<cellStyles count="1"><cellStyle name="Normal" xfId="0" builtinId="0"/></cellStyles>'
              '</styleSheet>')
    workbook = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                '<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" '
                'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">'
                '<sheets><sheet name="Wild Encounters" sheetId="1" r:id="rId1"/></sheets>'
                '<definedNames><definedName name="_xlnm._FilterDatabase" localSheetId="0" hidden="1">'
                f"'Wild Encounters'!$A$1:${last_col}${len(rows) + 1}</definedName></definedNames></workbook>")
    rel = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'
    files = {
        '[Content_Types].xml':
            '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
            '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
            '<Default Extension="xml" ContentType="application/xml"/>'
            '<Override PartName="/xl/workbook.xml" '
            'ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>'
            '<Override PartName="/xl/worksheets/sheet1.xml" '
            'ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>'
            '<Override PartName="/xl/styles.xml" '
            'ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/></Types>',
        '_rels/.rels':
            '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
            f'<Relationship Id="rId1" Type="{rel}/officeDocument" Target="xl/workbook.xml"/></Relationships>',
        'xl/_rels/workbook.xml.rels':
            '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
            f'<Relationship Id="rId1" Type="{rel}/worksheet" Target="worksheets/sheet1.xml"/>'
            f'<Relationship Id="rId2" Type="{rel}/styles" Target="styles.xml"/></Relationships>',
        'xl/workbook.xml': workbook,
        'xl/styles.xml': styles,
        'xl/worksheets/sheet1.xml': sheet,
    }
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        for name, content in files.items():
            z.writestr(name, content)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--ref', default='master',
                    help='git branch whose Hoenn Dex and Safari Zone define "native" (default: master)')
    ap.add_argument('--out', default=DEFAULT_OUT, help='output folder (default: tech/encounter_tables)')
    args = ap.parse_args()

    dex = hoenn_dex_species(args.ref)
    safari = safari_zone_species(args.ref)
    native = dex | safari
    rows = build_rows(native, species_display_names())
    width = max((len(r['cells']) for r in rows), default=0)
    headers = FIXED_HEADERS + [f'Pokemon {i + 1}' for i in range(width)]

    os.makedirs(args.out, exist_ok=True)
    written = []
    for ext, writer in (('csv', write_csv), ('xlsx', write_xlsx)):
        path = os.path.join(args.out, f'{OUT_NAME}.{ext}')
        try:
            writer(path, headers, rows)
            written.append(path)
        except PermissionError:
            print(f'  Could not write {path} - is it open in Excel? Close it and run again.')

    maps = len({(r['fixed'][0], r['fixed'][1]) for r in rows})
    flagged = sum(1 for r in rows if r['fixed'][4])
    print(f'  {len(native)} native species from "{args.ref}": {len(dex)} Hoenn Dex '
          f'+ {len(safari - dex)} more from the Safari Zone.')
    print(f'  {maps} maps, {len(rows)} encounter tables, {flagged} with non-native Pokemon.')
    for p in written:
        print(f'  Wrote {os.path.relpath(p, ROOT)}')
    return 0 if len(written) == 2 else 1


if __name__ == '__main__':
    sys.exit(main())
