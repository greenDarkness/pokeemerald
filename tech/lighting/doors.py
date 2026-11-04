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


def parse_doors():
    """[{metatile, size, tiles, png, palettes}] for the Hoenn door table (sDoorAnimGraphicsTable)."""
    src = open(FIELD_DOOR, encoding='utf-8').read()
    pngs = dict(re.findall(r'static const u8 (sDoorAnimTiles_\w+)\[\] = INCBIN_U8\("([^"]+)\.4bpp"\);', src))
    pals = {k: [int(x) for x in v.split(',')]
            for k, v in re.findall(r'static const u8 (sDoorAnimPalettes_\w+)\[\] = \{([^}]*)\};', src)}
    labels = {k: int(v, 16) for k, v in re.findall(r'#define (METATILE_\w+)\s+(0x[0-9A-Fa-f]+)', open(LABELS).read())}
    table = src[src.index('sDoorAnimGraphicsTable[] ='):]
    table = table[:table.index('};')]
    out = []
    for name, size, tiles, palettes in re.findall(r'\{\s*(\w+),\s*\w+,\s*(\d+),\s*(\w+),\s*(\w+)\}', table):
        mt = labels.get(name, None)
        if mt is None:
            try:
                mt = int(name, 16)
            except ValueError:
                continue
        out.append({'name': name, 'metatile': mt, 'size': int(size), 'tiles': tiles,
                    'png': pngs[tiles] + '.png', 'palettes': pals[palettes]})
    return out


def door_uses(doors):
    """{door index: [(map, x, y)]} for doors placed on outdoor maps with General as the primary tileset."""
    by_mt = {}
    for i, d in enumerate(doors):
        by_mt.setdefault(d['metatile'], []).append(i)
    uses = {}
    for m in MAPS.values():
        lay = m['_layout']
        if not lay or m.get('map_type') not in N.NATURAL_LIGHT_TYPES or lay['primary_tileset'] != 'gTileset_General':
            continue
        for y, row in enumerate(blocks(m['_name'])):
            for x, g in enumerate(row):
                for i in by_mt.get(g, ()):
                    # secondary metatile ids repeat across tilesets: METATILE_<Tileset>_... names the owner
                    parts = doors[i]['name'].split('_')
                    owner = 'gTileset_' + parts[1] if len(parts) > 1 else None
                    if g >= 0x200 and owner != lay['secondary_tileset']:
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

    def closed(self, mapname, x, y):
        """Day RGB, night RGB and night lit mask of the closed door (metatiles at y - 1 and y), 16x32."""
        b = blocks(mapname)
        imgs = []
        for r in (self.day, self.night):
            im = Image.new('RGB', (16, 32))
            lit = Image.new('L', (16, 32))
            for k, yy in enumerate((y - 1, y)):
                a, l = r.get(b[yy][x])
                im.paste(a.convert('RGB'), (0, 16 * k))
                lit.paste(l, (0, 16 * k))
            imgs += [im, lit]
        return imgs[0], imgs[2], imgs[3]

    def pal(self, p):
        return (self.gen if p < 6 else self.ts).pal(p)


def frames_of(door):
    """[(16x32 index image)] for each animation frame the open/close animation of a small door shows."""
    im = Image.open(N.rpath(door['png']))
    idx = Image.frombytes('L', im.size, im.tobytes()).point(lambda v: v & 15)
    src = open(FIELD_DOOR, encoding='utf-8').read()
    body = re.search(r'sDoorOpenAnimFrames\[\] =\s*\{(.*?)\};', src, re.S).group(1)
    used = [int(o, 0) for o in re.findall(r'\{\s*\d+,\s*(-?\w+)\s*\}', body) if o not in ('-1',)]
    n = min(im.height // 32, max(used) // 0x100 + 1)
    return [idx.crop((0, 32 * f, 16, 32 * f + 32)) for f in range(n)]


def frame_rgb(r, door, fr, night=False):
    cols_by_pal = (r.night if night else r.day).pals.colors
    out = Image.new('RGB', (16, 32))
    for t in range(8):
        tx, ty = (t % 2) * 8, (t // 2) * 8
        cols = cols_by_pal[door['palettes'][t]]
        for j in range(8):
            for i in range(8):
                out.putpixel((tx + i, ty + j), tuple(cols[fr.getpixel((tx + i, ty + j))]))
    return out


def overlay_for(r, door, mapname, x, y):
    """Lit-pixel colors per frame: [{(px, py): rgb}]. Pixels showing the interior revealed by the open door
    get the warm doorway glow (default_night, so shading survives); pixels matching the closed door keep its
    night glow; sliding glass keeps the glow its color has on that row of the closed door."""
    from lighter import default_night
    day, night, lit = r.closed(mapname, x, y)
    glow_by_row = {}  # (row, day color) -> night glow color, from the closed door's lit pixels
    for py in range(32):
        for px in range(16):
            if lit.getpixel((px, py)):
                glow_by_row.setdefault((py, day.getpixel((px, py))), night.getpixel((px, py)))
    frames = frames_of(door)
    opened = frame_rgb(r, door, frames[-1])
    revealed = {(px, py) for py in range(32) for px in range(16) if opened.getpixel((px, py)) != day.getpixel((px, py))}
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
        for py in range(32):
            for px in range(16):
                c = rgb.getpixel((px, py))
                if (px, py) in revealed and c == opened.getpixel((px, py)):
                    o[(px, py)] = default_night(c)
                elif c == day.getpixel((px, py)):
                    if lit.getpixel((px, py)):
                        o[(px, py)] = night.getpixel((px, py))
                elif (py, c) in glow_by_row:
                    o[(px, py)] = glow_by_row[(py, c)]
        out.append(o)
    return out, (day, night, lit)


def lit_doors():
    """[(door, uses)] for door animations placed outdoors whose closed metatiles glow at night."""
    doors = parse_doors()
    out = []
    for i, d in sorted(door_uses(doors).items()):
        mapname, x, y = d[0]
        r = Renderer(mapname)
        if any(r.closed(mapname, x, y)[2].getdata()):
            out.append((doors[i], d))
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
            for (px, py), c in ov[f].items():
                im.putpixel((px, py), tuple(c))
            cells.append(im)
        row = Image.new('RGB', (len(cells) * 18, 32), (255, 0, 255))
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


def build():
    """Write graphics/door_anims/night/*.png and src/data/door_night_overlays.h."""
    os.makedirs(NIGHT_DIR, exist_ok=True)
    for f in os.listdir(NIGHT_DIR):
        if f.endswith('.png'):
            os.remove(os.path.join(NIGHT_DIR, f))
    entries = []   # (metatile name, secondary or None, data key)
    data = {}      # key -> (file stem, frames, palette)
    for door, uses in lit_doors():
        per_sec = {}
        for mapname, x, y in uses:
            sec = BYNAME[mapname]['_layout']['secondary_tileset']
            if sec in per_sec:
                continue
            ov, _ = overlay_for(Renderer(mapname), door, mapname, x, y)
            per_sec[sec] = tuple(tuple(sorted((p, to5(c)) for p, c in f.items())) for f in ov)
        keys = set(per_sec.values())
        if not any(any(f) for k in keys for f in k):
            continue
        # Secondary metatile ids repeat across tilesets, so those entries always name their tileset;
        # primary (General) doors share one entry when every tileset gives the same overlay.
        if door['metatile'] < 0x200 and len(keys) == 1:
            per_sec = {None: next(iter(keys))}
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
    for key, (stem, frames) in data.items():
        cols = sorted({c for f in frames for _, c in f})
        assert len(cols) <= 15, f'{stem}: {len(cols)} colors'
        pal = [(31, 0, 31)] + cols + [(0, 0, 0)] * (15 - len(cols))
        img = Image.new('P', (16, 32 * len(frames)), 0)
        img.putpalette([v << 3 for c in pal for v in c])
        for k, f in enumerate(frames):
            for (px, py), c in f:
                img.putpixel((px, 32 * k + py), cols.index(c) + 1)
        img.save(os.path.join(NIGHT_DIR, stem + '.png'))
        ident = ''.join(w.capitalize() for w in stem.split('_'))
        names[key] = (ident, len(frames))
        lines.append(f'static const u32 sDoorNightTiles_{ident}[] = INCBIN_U32("graphics/door_anims/night/{stem}.4bpp");')
        lines.append(f'static const u16 sDoorNightPal_{ident}[] = {{' +
                     ', '.join('RGB(%d, %d, %d)' % c for c in pal) + '};')
    lines += ['']
    for sec in sorted({sec for _, sec, _ in entries if sec}):
        lines.append(f'extern const struct Tileset {sec};')
    lines += ['', 'static const struct DoorNightOverlay sDoorNightOverlays[] =', '{']
    for name, sec, key in entries:
        ident, n = names[key]
        lines.append(f'    {{{name}, {"&" + sec if sec else "NULL"}, sDoorNightTiles_{ident}, sDoorNightPal_{ident}, {n}}},')
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
        _, _, lit = r.closed(mapname, x, y)
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
