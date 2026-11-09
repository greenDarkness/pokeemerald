"""Inspect/edit helpers for overworld tileset lighting work. Built on tech/night_map.py.

Typical use (run from tech/lighting):
    from kit import *
    pair('PetalburgCity', (x0, y0, x1, y1), 'name')   # day | night side by side -> out/name.png
    blocks('PetalburgCity')[y][x]                      # metatile id at a map position
    gen = TS('gTileset_General'); sec = TS('gTileset_Petalburg')
    print(dump(gen, sec, 0x21))                        # tile/palette entries + pixel indices
"""
import os
import re
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))
import night_map as N  # noqa: E402
from PIL import Image, ImageDraw  # noqa: E402

ROOT = N.ROOT
SRC = N.Sources()
MAPS = N.load_maps()
BYNAME = {m['_name']: m for m in MAPS.values()}
OUT = os.path.join(HERE, 'out')
os.makedirs(OUT, exist_ok=True)


def fresh():
    """Re-read tileset sources (call after editing headers.h, palettes or tiles)."""
    global SRC
    N.Tileset._cache.clear()
    SRC = N.Sources()


def layout(mapname):
    return BYNAME[mapname]['_layout']


def blocks(mapname):
    lay = layout(mapname)
    b = open(N.rpath(lay['blockdata_filepath']), 'rb').read()
    w, h = lay['width'], lay['height']
    return [[struct.unpack('<H', b[2 * (y * w + x):2 * (y * w + x) + 2])[0] & 0x3FF for x in range(w)] for y in range(h)]


def where(mapname, gids):
    """{metatile id: [(x, y), ...]} for the given metatile ids on a map."""
    out = {}
    for y, row in enumerate(blocks(mapname)):
        for x, g in enumerate(row):
            if g in gids:
                out.setdefault(g, []).append((x, y))
    return out


def warps(mapname):
    return [(w['x'], w['y'], w['dest_map']) for w in BYNAME[mapname].get('warp_events') or []]


def add_light(mapname, x, y, light_type, flag='0'):
    """Append an OBJ_EVENT_GFX_LIGHT_SPRITE object to a map.json (keeps file formatting and the local ids of
    existing objects). light_type: see LIGHT_TYPE_* in src/event_object_movement.c. flag: hidden while set (e.g.
    a story light). Max 64 objects per map."""
    import json
    p = N.rpath('data/maps/%s/map.json' % mapname)
    t = open(p, encoding='utf-8', newline='').read()
    nl = '\r\n' if '\r\n' in t else '\n'
    objs = json.loads(t)['object_events']
    if any(o['graphics_id'] == 'OBJ_EVENT_GFX_LIGHT_SPRITE' and (o['x'], o['y']) == (x, y) for o in objs):
        return len(objs)
    assert len(objs) < 64, '%s already has 64 object events' % mapname
    key = '"object_events": ['
    i = t.index(key) + len(key)
    depth, j = 1, i
    while depth:
        depth += (t[j] == '[') - (t[j] == ']')
        j += 1
    end = j - 1
    m = re.search(r'\n([ \t]*)\{', t[i:end])
    ind = m.group(1) if m else re.search(r'\n([ \t]*)"object_events"', t).group(1) + '  '
    o = json.dumps({'graphics_id': 'OBJ_EVENT_GFX_LIGHT_SPRITE', 'x': x, 'y': y, 'elevation': 3,
                    'movement_type': 'MOVEMENT_TYPE_NONE', 'movement_range_x': 0, 'movement_range_y': 0,
                    'trainer_type': 'TRAINER_TYPE_NONE', 'trainer_sight_or_berry_tree_id': str(light_type),
                    'script': 'NULL', 'flag': flag}, indent=2)
    block = nl.join(ind + line for line in o.split('\n'))
    body = t[i:end].rstrip()
    new = t[:i] + body + (',' if body else '') + nl + block + nl + ind[:-2] + t[end:]
    open(p, 'w', encoding='utf-8', newline='').write(new)
    return len(objs) + 1


def render(mapname, box, night, scale=4, grid=True):
    fresh()
    lay = layout(mapname)
    r = N.MetatileRenderer(SRC, lay['primary_tileset'], lay['secondary_tileset'], night)
    bl = blocks(mapname)
    x0, y0, x1, y1 = box
    img = Image.new('RGB', ((x1 - x0) * 16, (y1 - y0) * 16))
    for y in range(y0, y1):
        for x in range(x0, x1):
            if 0 <= y < len(bl) and 0 <= x < len(bl[0]):
                img.paste(r.get(bl[y][x])[0], ((x - x0) * 16, (y - y0) * 16))
    if night:
        ls = N.LightSprites()
        lm = Image.new('L', img.size)
        for o in BYNAME[mapname].get('object_events') or []:
            if o.get('graphics_id') == 'OBJ_EVENT_GFX_LIGHT_SPRITE':
                ls.draw(img, lm, o.get('trainer_sight_or_berry_tree_id', 0) or 0,
                        (int(o['x']) - x0) * 16, (int(o['y']) - y0) * 16)
    img = img.resize((img.width * scale, img.height * scale), Image.NEAREST)
    if grid:
        d = ImageDraw.Draw(img)
        for x in range(x0, x1):
            d.text(((x - x0) * 16 * scale + 1, 0), str(x), fill=(255, 0, 255))
        for y in range(y0, y1):
            d.text((0, (y - y0) * 16 * scale + 1), str(y), fill=(255, 0, 255))
    return img


def pair(mapname, box, name, scale=4):
    """Day and night renders of a map region side by side, saved to out/<name>.png."""
    a = render(mapname, box, False, scale)
    b = render(mapname, box, True, scale)
    s = Image.new('RGB', (a.width * 2 + 6, a.height), (255, 0, 255))
    s.paste(a, (0, 0))
    s.paste(b, (a.width + 6, 0))
    p = os.path.join(OUT, name + '.png')
    s.save(p)
    return p


def stack(paths, name):
    """Stack several images vertically into out/<name>.png."""
    ims = [Image.open(p) for p in paths]
    s = Image.new('RGB', (max(i.width for i in ims), sum(i.height for i in ims) + 6 * len(ims)), (255, 0, 255))
    y = 0
    for i in ims:
        s.paste(i, (0, y))
        y += i.height + 6
    p = os.path.join(OUT, name + '.png')
    s.save(p)
    return p


class TS:
    """Editable tileset: tiles.png indices, metatiles.bin, metatile_attributes.bin, palettes."""

    def __init__(self, name):
        fresh()
        t = N.Tileset.get(SRC, name)
        self.name, self.secondary, self.swap, self.pal_paths = name, t.secondary, t.swap, t.pal_paths
        body = SRC.tileset_body(name)
        mt_sym = re.search(r'\.metatiles\s*=\s*(\w+)', body).group(1)
        self.mt_path = N.rpath(SRC.incbin(SRC.metatiles_h + SRC.graphics, mt_sym))
        self.dir = os.path.dirname(self.mt_path)
        self.attr_path = os.path.join(self.dir, 'metatile_attributes.bin')
        self.png_path = os.path.join(self.dir, 'tiles.png')
        self.img = Image.open(self.png_path)
        self.img.load()
        self.tw = self.img.width // 8
        self.mt = bytearray(open(self.mt_path, 'rb').read())
        self.base = 512 if self.secondary else 0

    # metatiles (global ids: primary 0-511, secondary 512+)
    def n(self):
        return len(self.mt) // 16

    def entries(self, gid):
        o = (gid - self.base) * 16
        return [(v & 0x3FF, bool(v & 0x400), bool(v & 0x800), v >> 12) for v in struct.unpack('<8H', self.mt[o:o + 16])]

    def set_entry(self, gid, k, tile=None, pal=None, hf=None, vf=None):
        """k = 0-3 bottom layer, 4-7 top layer (TL, TR, BL, BR)."""
        o = (gid - self.base) * 16 + k * 2
        v = struct.unpack('<H', self.mt[o:o + 2])[0]
        t, h, f, p = v & 0x3FF, (v >> 10) & 1, (v >> 11) & 1, v >> 12
        t = t if tile is None else tile
        p = p if pal is None else pal
        h = h if hf is None else int(hf)
        f = f if vf is None else int(vf)
        self.mt[o:o + 2] = struct.pack('<H', t | (h << 10) | (f << 11) | (p << 12))

    def save_mt(self):
        open(self.mt_path, 'wb').write(bytes(self.mt))

    def layer(self, gid):
        """Metatile layer type: 0 NORMAL, 1 COVERED (both layers under sprites), 2 SPLIT."""
        a = open(self.attr_path, 'rb').read()
        o = (gid - self.base) * 2
        return struct.unpack('<H', a[o:o + 2])[0] >> 12

    def set_layer(self, gid, layer):
        a = bytearray(open(self.attr_path, 'rb').read())
        o = (gid - self.base) * 2
        v = struct.unpack('<H', a[o:o + 2])[0]
        a[o:o + 2] = struct.pack('<H', (v & 0x0FFF) | (layer << 12))
        open(self.attr_path, 'wb').write(bytes(a))

    # tiles (local tile index)
    def px(self, lt):
        x, y = (lt % self.tw) * 8, (lt // self.tw) * 8
        return [[self.img.getpixel((x + i, y + j)) & 15 for i in range(8)] for j in range(8)]

    def set_px(self, lt, grid):
        x, y = (lt % self.tw) * 8, (lt // self.tw) * 8
        for j in range(8):
            for i in range(8):
                self.img.putpixel((x + i, y + j), grid[j][i])

    def ntiles(self):
        return self.tw * (self.img.height // 8)

    def save_tiles(self):
        self.img.save(self.png_path)

    def used_tiles(self):
        u = set()
        for m in range(self.n()):
            for (t, h, v, p) in self.entries(m + self.base):
                u.add(t)
        return u

    def free_tiles(self):
        """Local tile indices that no metatile of this tileset references and that are blank."""
        u = self.used_tiles()
        return [lt for lt in range(1, self.ntiles())
                if lt + self.base not in u and all(c == 0 for r in self.px(lt) for c in r)]

    # palettes
    def pal(self, i):
        return N.read_jasc(self.pal_paths[i])

    def write_pal(self, i, cols):
        p = N.rpath(self.pal_paths[i])
        raw = open(p, 'rb').read().decode()
        nl = '\r\n' if '\r\n' in raw else '\n'
        open(p, 'wb').write((nl.join(['JASC-PAL', '0100', '16'] + ['%d %d %d' % tuple(c) for c in cols]) + nl).encode())

    def pla(self, i):
        return N.read_pla(self.pal_paths[i])

    def write_pla(self, i, idxs):
        p = N.rpath(self.pal_paths[i][:-4] + '.pla')
        if not idxs:
            if os.path.exists(p):
                os.remove(p)
            return
        open(p, 'w', newline='\n').write('# Color indices to set high bit\n' + '\n'.join(str(x) for x in sorted(idxs)) + '\n')


def tile_px(prim, sec, tid):
    return prim.px(tid) if tid < 512 else sec.px(tid - 512)


def dump(prim, sec, gid, layer=None):
    """Text dump of a metatile: entries, then a 16x16 grid of color indices per layer."""
    ts = prim if gid < 512 else sec
    e = ts.entries(gid)
    lines = ['meta %03x: ' % gid + ' '.join('%d%s%s/p%d' % (t, 'h' if h else '', 'v' if v else '', p) for t, h, v, p in e)]
    for L in ([0, 1] if layer is None else [layer]):
        g = [['.'] * 16 for _ in range(16)]
        for k in range(4 * L, 4 * L + 4):
            t, hf, vf, p = e[k]
            q = k % 4
            pxs = tile_px(prim, sec, t)
            for j in range(8):
                for i in range(8):
                    c = pxs[7 - j if vf else j][7 - i if hf else i]
                    g[(q // 2) * 8 + j][(q % 2) * 8 + i] = '%x' % c
        lines.append(' L%d' % L)
        lines += ['   ' + ''.join(r[:8]) + ' ' + ''.join(r[8:]) for r in g]
    return '\n'.join(lines)


def uses(tsnames, tid):
    """[(tileset, metatile, entry k, palette)] for every metatile entry drawing tile tid."""
    out = []
    for n in tsnames:
        ts = TS(n)
        for m in range(ts.n()):
            for k, (t, h, v, p) in enumerate(ts.entries(m + ts.base)):
                if t == tid:
                    out.append((n.replace('gTileset_', ''), hex(m + ts.base), k, p))
    return out


def secondaries_of(prim_name):
    return sorted({l['secondary_tileset'] for l in N.json.loads(N.read_text('data/layouts/layouts.json'))['layouts']
                   if l.get('primary_tileset') == prim_name} - {'0', 'NULL', None})
