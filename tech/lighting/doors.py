"""Night lighting for door animations (see README.md, "Door animations").

Door animation frames come from graphics/door_anims/*.png and are drawn with fixed BG palettes, so the
lighting done on door metatiles doesn't apply to them. At night the game draws a second, lit-pixels-only
overlay on the top layer, with its own palette loaded into BG palette 13 (left out of the time blend,
so the colors show exactly as written). This module finds the lit doors, builds the overlays, and
picks free VRAM tiles for them in each secondary tileset.

    python doors.py            # report: lit door animations and where they're used
    python doors.py --build    # write overlays, palettes, and src/data/door_night_overlays.h
"""
import argparse
import os
import re
import struct

from kit import N, TS, BYNAME, MAPS, OUT, Image, blocks, tile_px

ROOT = N.ROOT
FIELD_DOOR = N.rpath('src/field_door.c')
LABELS = N.rpath('include/constants/metatile_labels.h')
NIGHT_DIR = N.rpath('graphics/door_anims/night')
HEADER = N.rpath('src/data/door_night_overlays.h')
FRAME_TILES = 8           # small (size 1) doors: 16x32 per frame
DOORWAY = (240, 212, 136)  # final night color of a lit doorway (matches lit doorway metatiles)


KANTO_PRIMARIES = ('gTileset_KantoGeneral', 'gTileset_KantoBuilding')  # IsCurrentMapKanto in field_door.c
REGIONS = ('hoenn', 'kanto', 'sevii')  # regions whose night lighting has been done
# Doors left unlit when closed that still light up (the doorway behind them) while opening
from doorglass import EXCLUDE as OPEN_ONLY  # noqa: E402


def parse_doors():
    """[{name, metatile, size, h, table, png, palettes}] for both door tables (sDoorAnimGraphicsTable for Hoenn
    and Johto maps, sKantoDoorAnimGraphicsTable for maps on a Kanto primary). Only door sizes with night
    overlay support: 1 (16x32 frames) and DOOR_SIZE_KANTO_SMALL (16x16 frames)."""
    src = open(FIELD_DOOR, encoding='utf-8').read()
    pngs = dict(re.findall(r'static const u8 (sDoorAnimTiles_\w+)\[\] = INCBIN_U8\("([^"]+)\.4bpp"\);', src))
    pals = {k: [int(x) for x in v.split(',')]
            for k, v in re.findall(r'static const u8 (sDoorAnimPalettes_\w+)\[\] = \{([^}]*)\};', src)}
    labels = {k: int(v, 16) for k, v in re.findall(r'#define (METATILE_\w+)\s+(0x[0-9A-Fa-f]+)', open(LABELS).read())}
    out = []
    for table_name, table in (('hoenn', 'sDoorAnimGraphicsTable'), ('kanto', 'sKantoDoorAnimGraphicsTable')):
        body = src[src.index(table + '[] ='):]
        body = body[:body.index('};')]
        for name, size, tiles, palettes in re.findall(r'\{\s*(\w+),\s*\w+,\s*(\w+),\s*(\w+),\s*(\w+)\}', body):
            size = {'DOOR_SIZE_KANTO_SMALL': 3, 'DOOR_SIZE_KANTO_LARGE': 4}.get(size, size)
            size = int(size)
            if size not in (1, 3):
                continue
            mt = labels.get(name)
            if mt is None:
                try:
                    mt = int(name, 16)
                except ValueError:
                    continue
            out.append({'name': name, 'metatile': mt, 'size': size, 'h': 16 if size == 3 else 32, 'table': table_name,
                        'tiles': tiles, 'png': pngs[tiles] + '.png', 'palettes': pals[palettes]})
    return out


def lit_copies():
    """{(secondary, copy id): original id} from src/data/lit_door_metatiles.h (tech/lighting/doorglass.py)."""
    try:
        text = open(N.rpath('src/data/lit_door_metatiles.h'), encoding='utf-8').read()
    except FileNotFoundError:
        return {}
    return {(s, int(c, 16)): int(o, 16) for s, c, o in re.findall(r'\{&(\w+), (0x\w+), (0x\w+)\}', text)}


def door_uses(doors):
    """{door index: [(map, x, y)]} for doors placed on outdoor maps (lit copies count as their original)."""
    by_mt = {}
    for i, d in enumerate(doors):
        by_mt.setdefault((d['table'], d['metatile']), []).append(i)
    copies = lit_copies()
    secs = N.mapsec_regions()
    N.assign_regions(MAPS, secs)
    uses = {}
    for m in MAPS.values():
        lay = m['_layout']
        if not lay or m.get('map_type') not in N.NATURAL_LIGHT_TYPES or N.region_of(m, secs) not in REGIONS:
            continue
        table = 'kanto' if lay['primary_tileset'] in KANTO_PRIMARIES else 'hoenn'
        sec = lay['secondary_tileset']
        for y, row in enumerate(blocks(m['_name'])):
            for x, g in enumerate(row):
                g0 = copies.get((sec, g), g)
                for i in by_mt.get((table, g0), ()):
                    # secondary metatile ids repeat across tilesets: METATILE_<Tileset>_... names the owner
                    # (and primary ids repeat across primaries: the label must name this map's primary)
                    parts = doors[i]['name'].split('_')
                    owner = 'gTileset_' + parts[1] if len(parts) > 1 else None
                    if owner != (sec if g0 >= 0x200 else lay['primary_tileset']):
                        continue
                    uses.setdefault(i, []).append((m['_name'], x, y))
    return uses


class Renderer:
    """Day/night metatile and door-frame rendering for one map's tilesets."""
    def __init__(self, mapname):
        lay = BYNAME[mapname]['_layout']
        self.prim, self.sec = lay['primary_tileset'], lay['secondary_tileset']
        self.day = N.MetatileRenderer(N.Sources(), self.prim, self.sec, False)
        self.night = N.MetatileRenderer(N.Sources(), self.prim, self.sec, True)
        self.gen, self.ts = TS(self.prim), TS(self.sec)

    def closed(self, mapname, x, y, h=32):
        """Day RGB, night RGB and night lit mask of the closed door: 16x32 (metatiles at y - 1 and y), or 16x16."""
        b = blocks(mapname)
        imgs = []
        for r in (self.day, self.night):
            im = Image.new('RGB', (16, h))
            lit = Image.new('L', (16, h))
            for k, yy in enumerate((y - 1, y) if h == 32 else (y,)):
                a, l = r.get(b[yy][x])
                im.paste(a.convert('RGB'), (0, 16 * k))
                lit.paste(l, (0, 16 * k))
            imgs += [im, lit]
        return imgs[0], imgs[2], imgs[3]

    def pal(self, p):
        return (self.gen if p < 6 else self.ts).pal(p)


def frames_of(door):
    """[(16 x h index image)] for each frame the door's open/close animation shows."""
    h = door['h']
    im = Image.open(N.rpath(door['png']))
    idx = Image.frombytes('L', im.size, im.tobytes()).point(lambda v: v & 15)
    src = open(FIELD_DOOR, encoding='utf-8').read()
    name = 'sKantoSmallDoorOpenAnimFrames' if door['size'] == 3 else 'sDoorOpenAnimFrames'
    body = re.search(name + r'\[\] =\s*\{(.*?)\};', src, re.S).group(1)
    used = [int(o, 0) for o in re.findall(r'\{\s*\d+,\s*(-?\w+)\s*\}', body) if o not in ('-1',)]
    step = h // 8 * 2 * 32  # bytes per frame
    n = min(im.height // h, max(used) // step + 1)
    return [idx.crop((0, h * f, 16, h * f + h)) for f in range(n)]


def frame_rgb(r, door, fr, night=False):
    cols_by_pal = (r.night if night else r.day).pals.colors
    h = door['h']
    out = Image.new('RGB', (16, h))
    for t in range(h // 8 * 2):
        tx, ty = (t % 2) * 8, (t // 2) * 8
        cols = cols_by_pal[door['palettes'][t]]
        for j in range(8):
            for i in range(8):
                out.putpixel((tx + i, ty + j), tuple(cols[fr.getpixel((tx + i, ty + j))]))
    return out


def overlay_for(r, door, mapname, x, y):
    """Lit pixels per frame: [{(px, py): (day rgb, night rgb)}]: the day color the animation frame shows there and
    the final night color the overlay shows. Pixels showing the interior revealed by the open door
    get the warm doorway glow (default_night, so shading survives); pixels matching the closed door keep its
    night glow; sliding glass keeps the glow its color has on that row of the closed door."""
    from lighter import default_night
    H = door['h']
    day, night, lit = r.closed(mapname, x, y, H)
    glow_by_row = {}  # (row, day color) -> night glow color, from the closed door's lit pixels
    for py in range(H):
        for px in range(16):
            if lit.getpixel((px, py)):
                glow_by_row.setdefault((py, day.getpixel((px, py))), night.getpixel((px, py)))
    frames = frames_of(door)
    opened = frame_rgb(r, door, frames[-1])
    revealed = {(px, py) for py in range(H) for px in range(16) if opened.getpixel((px, py)) != day.getpixel((px, py))}
    if revealed:
        # interior pixels whose color happens to match the closed door at that spot
        cols = {opened.getpixel(p) for p in revealed}
        x0, x1 = min(p[0] for p in revealed), max(p[0] for p in revealed)
        y0, y1 = min(p[1] for p in revealed), max(p[1] for p in revealed)
        revealed |= {(px, py) for py in range(y0, y1 + 1) for px in range(x0, x1 + 1)
                     if opened.getpixel((px, py)) in cols and not lit.getpixel((px, py))}
    out = []
    for fr in frames:
        rgb = frame_rgb(r, door, fr)
        o = {}
        for py in range(H):
            for px in range(16):
                c = rgb.getpixel((px, py))
                if (px, py) in revealed and c == opened.getpixel((px, py)):
                    o[(px, py)] = (c, default_night(c))
                elif c == day.getpixel((px, py)):
                    if lit.getpixel((px, py)):
                        o[(px, py)] = (c, night.getpixel((px, py)))
                elif (py, c) in glow_by_row:
                    o[(px, py)] = (c, glow_by_row[(py, c)])
        out.append(o)
    return out, (day, night, lit)


def building_lit(r, mapname, x, y):
    """Whether the building around a door glows at night: lit pixels on the facade around it (2 metatiles to each
    side, 3 up), or a light sprite (sign) next to it."""
    b = blocks(mapname)
    n = 0
    for yy in range(max(0, y - 3), y + 1):
        for xx in range(max(0, x - 2), min(len(b[0]), x + 3)):
            n += sum(1 for v in r.night.get(b[yy][xx])[1].getdata() if v)
    lights = {(o['x'], o['y']) for o in BYNAME[mapname].get('object_events') or []
              if o.get('graphics_id') == 'OBJ_EVENT_GFX_LIGHT_SPRITE'}
    return n >= 4 or any(abs(lx - x) <= 3 and 0 <= y - ly <= 3 for lx, ly in lights)


def lit_doors():
    """[(door, uses)] for door animations placed outdoors on lit buildings (the closed door glows, or the facade
    around it does): their animation shows the lit room behind the door. uses: only the lit buildings."""
    doors = parse_doors()
    out = []
    renderers = {}
    for i, d in sorted(door_uses(doors).items()):
        lit_uses = []
        for mapname, x, y in d:
            if mapname not in renderers:
                renderers[mapname] = Renderer(mapname)
            r = renderers[mapname]
            sec = BYNAME[mapname]['_layout']['secondary_tileset']
            if (any(r.closed(mapname, x, y, doors[i]['h'])[2].getdata()) or (sec, doors[i]['metatile']) in OPEN_ONLY
                    or building_lit(r, mapname, x, y)):
                lit_uses.append((mapname, x, y))
        if lit_uses:
            out.append((doors[i], lit_uses))
    return out


def preview(name='door_preview', scale=6):
    """Rows of: closed day, frames day | closed night, frames night with the overlay -> out/<name>.png"""
    rows = []
    for door, d in lit_doors():
        mapname, x, y = d[0]
        r = Renderer(mapname)
        ov, (day, night, lit) = overlay_for(r, door, mapname, x, y)
        frames = frames_of(door)
        cells = [day] + [frame_rgb(r, door, fr) for fr in frames] + [night]
        for f, fr in enumerate(frames):
            im = frame_rgb(r, door, fr, night=True)
            for (px, py), (dc, c) in ov[f].items():
                im.putpixel((px, py), tuple(c))
            cells.append(im)
        row = Image.new('RGB', (len(cells) * 18, door['h']), (255, 0, 255))
        for k, c in enumerate(cells):
            row.paste(c, (k * 18, 0))
        rows.append(row)
    im = Image.new('RGB', (max(r.width for r in rows), 34 * len(rows)), (40, 40, 40))
    for k, r in enumerate(rows):
        im.paste(r, (0, 34 * k))
    im = im.resize((im.width * scale, im.height * scale), Image.NEAREST)
    p = os.path.join(OUT, name + '.png')
    im.save(p)
    return p


def to5(c):
    return tuple(v >> 3 for v in c)


def pre_light(final5):
    """Night palette color whose light blend (halfway to the light color) gives final5."""
    import glass
    return tuple(v >> 3 for v in glass.night_for(final5))


def reduce_pairs(pairs, n):
    """Map (day5, night5) color pairs onto at most n of them, merging the closest pairs."""
    reps = {p: p for p in pairs}
    groups = {p: [p] for p in pairs}
    while len(groups) > n:
        keys = list(groups)
        best = None
        for i, a in enumerate(keys):
            for b in keys[i + 1:]:
                d = sum((x - y) ** 2 for x, y in zip(a[0] + a[1], b[0] + b[1]))
                if best is None or d < best[0]:
                    best = (d, a, b)
        _, a, b = best
        keep, drop = (a, b) if len(groups[a]) >= len(groups[b]) else (b, a)
        groups[keep] += groups.pop(drop)
    for k, members in groups.items():
        for m in members:
            reps[m] = k
    return reps


def build():
    """Write graphics/door_anims/night/*.png and src/data/door_night_overlays.h."""
    os.makedirs(NIGHT_DIR, exist_ok=True)
    for f in os.listdir(NIGHT_DIR):
        if f.endswith('.png'):
            os.remove(os.path.join(NIGHT_DIR, f))
    entries = []   # (metatile name, secondary or None, data key)
    data = {}      # key -> (file stem, frames, frame height)
    for door, uses in lit_doors():
        per_sec = {}
        for mapname, x, y in uses:
            sec = BYNAME[mapname]['_layout']['secondary_tileset']
            if sec in per_sec:
                continue
            ov, _ = overlay_for(Renderer(mapname), door, mapname, x, y)
            per_sec[sec] = (door['h'], tuple(tuple(sorted((p, (to5(dc), to5(c))) for p, (dc, c) in f.items()))
                                             for f in ov))
        keys = set(per_sec.values())
        if not any(any(f) for k in keys for f in k[1]):
            continue
        # Secondary metatile ids repeat across tilesets, so those entries always name their tileset;
        # primary Hoenn doors share one entry when every tileset gives the same overlay. Kanto entries always name
        # their tileset: Kanto and Hoenn door metatile ids overlap.
        for sec, key in sorted(per_sec.items(), key=lambda kv: kv[0] or ''):
            if key not in data:
                stem = os.path.splitext(os.path.basename(door['png']))[0]
                if any(d[0] == stem for d in data.values()):
                    stem = '%s_%d' % (stem, len(data))
                data[key] = (stem, key)
            entries.append((door['name'], sec, key))
    lines = ['// Generated by tech/lighting/doors.py --build from the night renders of each lit door. Do not edit.',
             '// Night overlays for door animations: lit pixels only, drawn as a sprite over the animating door.', '']
    names = {}
    for key, (stem, (h, frames)) in data.items():
        pairs = reduce_pairs(sorted({c for f in frames for _, c in f}), 15)
        cols = sorted(set(pairs.values()))
        pal_day = [(31, 0, 31)] + [d for d, _ in cols] + [(0, 0, 0)] * (15 - len(cols))
        pal_night = [(31, 0, 31)] + [pre_light(n) for _, n in cols] + [(0, 0, 0)] * (15 - len(cols))
        img = Image.new('P', (16, h * len(frames)), 0)
        img.putpalette([v << 3 for _, c in [((0, 0, 0), (31, 0, 31))] + cols + [((0, 0, 0), (0, 0, 0))] * (15 - len(cols))
                        for v in c])
        for k, f in enumerate(frames):
            for (px, py), c in f:
                img.putpixel((px, h * k + py), cols.index(pairs[c]) + 1)
        img.save(os.path.join(NIGHT_DIR, stem + '.png'))
        ident = ''.join(w.capitalize() for w in stem.split('_'))
        names[key] = (ident, len(frames))
        lines.append(f'static const u32 sDoorNightTiles_{ident}[] = INCBIN_U32("graphics/door_anims/night/{stem}.4bpp");')
        # day colors flagged as lights (bit 15), like the lit tileset colors they cover
        lines.append(f'static const u16 sDoorNightPalDay_{ident}[] = {{RGB(31, 0, 31), ' +
                     ', '.join('RGB(%d, %d, %d) | RGB_ALPHA' % c for c in pal_day[1:len(cols) + 1]) +
                     ''.join(', RGB(0, 0, 0)' for _ in range(15 - len(cols))) + '};')
        lines.append(f'static const u16 sDoorNightPalNight_{ident}[] = {{' +
                     ', '.join('RGB(%d, %d, %d)' % c for c in pal_night) + '};')
    lines += ['']
    for sec in sorted({sec for _, sec, _ in entries if sec}):
        lines.append(f'extern const struct Tileset {sec};')
    lines += ['', 'static const struct DoorNightOverlay sDoorNightOverlays[] =', '{']
    for name, sec, key in entries:
        ident, n = names[key]
        lines.append(f'    {{{name}, {"&" + sec if sec else "NULL"}, sDoorNightTiles_{ident}, sDoorNightPalDay_{ident}, '
                     f'sDoorNightPalNight_{ident}, {n}}},')
    lines += ['    {0},', '};', '']
    open(HEADER, 'w', newline='\n').write('\n'.join(lines))
    print(f'{len(entries)} door entries, {len(data)} overlays -> {HEADER}')


def report():
    doors = parse_doors()
    uses = door_uses(doors)
    for i, d in sorted(uses.items()):
        door = doors[i]
        mapname, x, y = d[0]
        r = Renderer(mapname)
        _, _, lit = r.closed(mapname, x, y, door['h'])
        n = sum(1 for v in lit.getdata() if v)
        secs = sorted({BYNAME[mn]['_layout']['secondary_tileset'] for mn, _, _ in d})
        print(f"{door['name']:48s} size {door['size']} lit px {n:3d}  {door['png']}  pals {door['palettes']}  "
              f"{len(d)} uses  {secs}")


if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument('--build', action='store_true')
    a = ap.parse_args()
    if a.build:
        build()
    else:
        report()
