"""Render a stitched night-time overview of a whole region to check building lighting.

Usage:
    python night_map.py kanto|johto|hoenn|sevii|1|2|3|4|all [...] [options]
    python night_map.py --map PetalburgCity [options]

Regions: 1 = Kanto, 2 = Johto, 3 = Hoenn, 4 = Sevii Islands.

Options:
    --day          render daytime colors instead of night (for comparison)
    --no-lights    skip light sprites (Pokemon Center / Mart signs, Rustboro lanterns)
    --no-labels    don't print map names on the image
    --no-marks     don't outline unlit building doors (blue) and Hidden Grotto entrances (red)
    --scale N      integer upscale factor (default 1)
    --out DIR      output folder (default tech/night_maps)

The night look follows the game's code: tileset palettes swap to their night variants
(.swapPalettes), colors flagged in a palette's .pla file blend halfway toward the light color
(DEFAULT_LIGHT_COLOR, or color 0 if index 0 is flagged), every other color gets TINT_NIGHT,
and OBJ_EVENT_GFX_LIGHT_SPRITE objects are drawn with the in-game 12/12 alpha blend.

Building doors with no light near them (night only) are outlined in blue, and the Hidden Grotto
entrances listed in src/data/hidden_grottos.h are outlined in red.
"""
import argparse
import json
import os
import re
import struct
import sys
import time
from collections import deque

from PIL import Image, ImageDraw

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
NUM_PALS_IN_PRIMARY = 6
NUM_PALS_TOTAL = 13
NUM_METATILES_IN_PRIMARY = 512
NUM_TILES_IN_PRIMARY = 512
NATURAL_LIGHT_TYPES = {'MAP_TYPE_TOWN', 'MAP_TYPE_CITY', 'MAP_TYPE_ROUTE', 'MAP_TYPE_OCEAN_ROUTE'}

# src/overworld.c: TINT_NIGHT = Q_8_8(0.456), Q_8_8(0.456), Q_8_8(0.615)
TINT_NIGHT = (int(0.456 * 256), int(0.456 * 256), int(0.615 * 256))
# src/palette.c: DEFAULT_LIGHT_COLOR 0x3f9f
DEFAULT_LIGHT = (0x3f9f & 31, (0x3f9f >> 5) & 31, (0x3f9f >> 10) & 31)
NIGHT_COEFF = 8 * 2  # tint blends use coeff 8*2 for light colors in TimeMixPalettes
LIGHT_BLEND_EVA = LIGHT_BLEND_EVB = 12  # Weather_SetBlendCoeffs(12, 12)

REGIONS = {'1': 'kanto', '2': 'johto', '3': 'hoenn', '4': 'sevii'}
REGION_TITLES = {'kanto': 'Kanto', 'johto': 'Johto', 'hoenn': 'Hoenn', 'sevii': 'Sevii Islands'}
REGION_LAYOUT_FILES = [  # same order as GetFlyRegionForMapsec in src/region_map.c; Hoenn is the fallback
    ('johto', 'region_map_layout_johto.h'),
    ('kanto', 'region_map_layout_kanto.h'),
    ('sevii', 'region_map_layout_sevii_123.h'),
    ('sevii', 'region_map_layout_sevii_45.h'),
    ('sevii', 'region_map_layout_sevii_67.h'),
]

LIGHT_SPRITES = {  # light type -> (pic, palette); see gFieldEffectLightTemplates / SpawnLightSprite
    0: ('graphics/object_events/pics/misc/light.png', 'graphics/object_events/palettes/light.pal'),
    1: ('graphics/object_events/pics/misc/poke_center_light.png', 'graphics/object_events/palettes/neon_light.pal'),
    2: ('graphics/object_events/pics/misc/mart_light.png', 'graphics/object_events/palettes/mart_light.pal'),
    3: ('graphics/object_events/pics/misc/poke_center_emblem_light.png', 'graphics/object_events/palettes/neon_light.pal'),
    4: ('graphics/object_events/pics/misc/mart_emblem_light.png', 'graphics/object_events/palettes/neon_light.pal'),
    6: ('graphics/object_events/pics/misc/dept_store_emblem_light.png', 'graphics/object_events/palettes/neon_light.pal'),
    7: ('graphics/object_events/pics/misc/kanto_poke_center_light.png', 'graphics/object_events/palettes/mart_light.pal'),
    8: ('graphics/object_events/pics/misc/kanto_mart_light.png', 'graphics/object_events/palettes/neon_light.pal'),
    9: ('graphics/object_events/pics/misc/kanto_poke_center_emblem_light.png', 'graphics/object_events/palettes/mart_light.pal'),
    10: ('graphics/object_events/pics/misc/kanto_mart_emblem_light.png', 'graphics/object_events/palettes/neon_light.pal'),
    11: ('graphics/object_events/pics/misc/game_corner_light.png', 'graphics/object_events/palettes/neon_light.pal'),
    12: ('graphics/object_events/pics/misc/lighthouse_light.png', 'graphics/object_events/palettes/light.pal'),
    13: ('graphics/object_events/pics/misc/lighthouse_light.png', 'graphics/object_events/palettes/light.pal'),
}
LIGHT_TYPE_LANTERN_PAIR = 5  # two lanterns at x and x + 3
LIGHT_TYPE_DEPT_STORE_EMBLEM = 6  # centered on its tile's center
# Kanto signs (7, 8; object on the door tile) spawn their emblem (9, 10) one tile up. Sprite centers relative to
# their tile's top-left: sKantoLightCenters in src/event_object_movement.c
KANTO_LIGHT_CENTERS = {7: (-17, 4), 8: (-17, 4), 9: (8, 7), 10: (0, 7), 11: (8, 12), 12: (0, 10), 13: (8, 3)}  # 11: Game Corner, 12/13: lighthouses

HIDDEN_GROTTOS_FILE = 'src/data/hidden_grottos.h'
HIDDEN_GROTTO_ROOM = 'MAP_HIDDEN_GROTTO'
UNLIT_DOOR_COLOR = (40, 120, 255)
GROTTO_COLOR = (255, 40, 40)


def rpath(p):
    return os.path.join(ROOT, p.replace('/', os.sep))


def read_text(p):
    with open(rpath(p), encoding='utf-8', errors='replace') as f:
        return f.read()


def to5(c):
    return c >> 3


def to8(c):
    return (c << 3) | (c >> 2)


def read_jasc(path):
    lines = read_text(path).split()
    if lines[:2] != ['JASC-PAL', '0100']:
        raise ValueError(f'{path}: not a JASC palette')
    n = int(lines[2])
    vals = list(map(int, lines[3:3 + n * 3]))
    return [tuple(vals[i * 3:i * 3 + 3]) for i in range(n)]


def read_pla(pal_path):
    p = rpath(pal_path[:-4] + '.pla')
    if not os.path.exists(p):
        return set()
    out = set()
    with open(p) as f:
        for line in f:
            line = line.strip()
            if line and not line.startswith('#'):
                out.add(int(line.split()[0]))
    return out


# ---------------------------------------------------------------- tilesets

class Sources:
    def __init__(self):
        self.headers = read_text('src/data/tilesets/headers.h')
        self.metatiles_h = read_text('src/data/tilesets/metatiles.h')
        self.graphics = read_text('src/data/tilesets/graphics.h') + read_text('src/graphics.c')

    def tileset_body(self, name):
        m = re.search(r'const struct Tileset %s\s*=\s*\{(.*?)\};' % re.escape(name), self.headers, re.S)
        if not m:
            raise KeyError(f'tileset {name} not found in headers.h')
        return m.group(1)

    def incbin(self, text, sym):
        m = re.search(r'\b%s\[\]\s*=\s*INCBIN_\w+\("([^"]+)"\)' % re.escape(sym), text)
        return m.group(1) if m else None

    def palettes(self, sym):
        m = re.search(r'\b%s\[\]\[16\]\s*=\s*\{(.*?)\};' % re.escape(sym), self.graphics, re.S)
        if not m:
            return None
        return re.findall(r'INCBIN_U16\("([^"]+)"\)', m.group(1))


class Tileset:
    _cache = {}

    @classmethod
    def get(cls, src, name):
        if name not in cls._cache:
            cls._cache[name] = cls(src, name)
        return cls._cache[name]

    def __init__(self, src, name):
        self.name = name
        body = src.tileset_body(name)
        field = lambda f: (re.search(r'\.%s\s*=\s*([^,\n]+)' % f, body) or [None, None])[1]
        self.secondary = (field('isSecondary') or 'FALSE').strip() == 'TRUE'
        sw = (re.search(r'\.swapPalettes\s*=\s*([^\n]*)', body) or [None, ''])[1]  # SWAP_PAL(x) / SWAP_PALS*(...)
        self.swap = {int(x) for args in re.findall(r'\(([^()]*)\)', sw) for x in re.findall(r'\d+', args)}
        mt_path = src.incbin(src.metatiles_h + src.graphics, field('metatiles').strip())
        if not mt_path:
            raise KeyError(f'{name}: metatiles INCBIN not found')
        self.metatiles = open(rpath(mt_path), 'rb').read()
        attr_sym = (field('metatileAttributes') or '').strip()
        attr_path = src.incbin(src.metatiles_h + src.graphics, attr_sym) if attr_sym else None
        self.attributes = open(rpath(attr_path), 'rb').read() if attr_path and os.path.exists(rpath(attr_path)) else b''
        folder = os.path.dirname(mt_path)
        tiles_path = src.incbin(src.graphics, field('tiles').strip())
        tiles_png = re.sub(r'\.4bpp(\.lz)?$', '.png', tiles_path) if tiles_path else folder + '/tiles.png'
        self.tiles = self._load_tiles(tiles_png)
        pal_list = src.palettes(field('palettes').strip()) or [f'{folder}/palettes/{i:02d}.gbapal' for i in range(16)]
        self.pal_paths = [p.replace('.gbapal', '.pal') for p in pal_list]

    def _load_tiles(self, path):
        img = Image.open(rpath(path))
        if img.mode not in ('P', 'L', '1'):
            raise ValueError(f'{path}: tiles must be an indexed PNG (got {img.mode})')
        if img.mode == '1':
            img = img.convert('L')
        # Raw color indices as an 'L' image (point() on 'P' images would remap the palette instead)
        raw = Image.frombytes('L', img.size, img.tobytes()).point(lambda v: v & 15)
        w, h = raw.width // 8, raw.height // 8
        return [raw.crop(((t % w) * 8, (t // w) * 8, (t % w) * 8 + 8, (t // w) * 8 + 8)) for t in range(w * h)]

    def entries(self, local_id):
        o = local_id * 16
        if o + 16 > len(self.metatiles):
            return None
        return struct.unpack('<8H', self.metatiles[o:o + 16])

    def behavior(self, local_id):
        o = local_id * 2
        if o + 2 > len(self.attributes):
            return None
        return struct.unpack('<H', self.attributes[o:o + 2])[0] & 0xFF  # METATILE_ATTR_BEHAVIOR_MASK


def door_behaviors():
    """Values of the door metatile behaviors, parsed from the MB_ enum."""
    text = read_text('include/constants/metatile_behaviors.h')
    body = text[text.index('enum'):]
    body = body[body.index('{') + 1:body.index('}')]
    values, n = {}, -1
    for line in body.split('\n'):
        line = re.sub(r'//.*', '', line).strip().rstrip(',')
        if not line.startswith('MB_'):
            continue
        if '=' in line:
            name, val = (s.strip() for s in line.split('=', 1))
            n = int(val, 0)
        else:
            name, n = line, n + 1
        values[name] = n
    return {values[k] for k in ('MB_NON_ANIMATED_DOOR', 'MB_ANIMATED_DOOR', 'MB_PETALBURG_GYM_DOOR') if k in values}


class Palettes:
    """The 13 BG palettes as the game holds them for one primary/secondary pair."""

    def __init__(self, prim, sec, night):
        self.colors = []
        self.lit = []
        for i in range(NUM_PALS_TOTAL):
            ts = prim if i < NUM_PALS_IN_PRIMARY else sec
            if ts is None or i >= len(ts.pal_paths):
                self.colors.append([(0, 0, 0)] * 16)
                self.lit.append([False] * 16)
                continue
            path = ts.pal_paths[i]
            base = [tuple(map(to5, c)) for c in read_jasc(path)][:16]
            base += [(0, 0, 0)] * (16 - len(base))
            high = read_pla(path)
            if night and i in ts.swap:  # UpdateAltBgPalettes with altWeight 0 = full night palette
                alt = [tuple(map(to5, c)) for c in read_jasc(ts.pal_paths[(i + 9) % 16])][:16]
                base = alt + base[len(alt):]
            lit = [j in high for j in range(16)]
            if night:
                base = self._time_mix(base, high)
            self.colors.append([tuple(map(to8, c)) for c in base])
            self.lit.append(lit)

    @staticmethod
    def _time_mix(cols, high):
        # TimeMixPalettes at night: bld0 == bld1 == night tint, weight 256
        alt = cols[0] if 0 in high else None
        out = [cols[0]]
        for j in range(1, 16):
            r, g, b = cols[j]
            if j in high:
                tr, tg, tb = alt or DEFAULT_LIGHT
                r = r + (((tr - r) * NIGHT_COEFF) >> 5)
                g = g + (((tg - g) * NIGHT_COEFF) >> 5)
                b = b + (((tb - b) * NIGHT_COEFF) >> 5)
            else:
                r = min(31, (TINT_NIGHT[0] * r) >> 8)
                g = min(31, (TINT_NIGHT[1] * g) >> 8)
                b = min(31, (TINT_NIGHT[2] * b) >> 8)
            out.append((r, g, b))
        return out


class MetatileRenderer:
    def __init__(self, src, prim_name, sec_name, night):
        self.prim = Tileset.get(src, prim_name)
        self.sec = Tileset.get(src, sec_name) if sec_name and sec_name not in ('0', 'NULL') else None
        self.pals = Palettes(self.prim, self.sec, night)
        self.backdrop = self.pals.colors[0][0]
        self.cache = {}
        self.flat = []
        for p in range(NUM_PALS_TOTAL):
            flat = []
            for c in self.pals.colors[p]:
                flat.extend(c)
            self.flat.append(flat * 16)
        self.lit_lut = [[255 if v else 0 for v in self.pals.lit[p]] * 16 for p in range(NUM_PALS_TOTAL)]

    def _tile(self, tid):
        if tid < NUM_TILES_IN_PRIMARY:
            return self.prim.tiles[tid] if tid < len(self.prim.tiles) else None
        tid -= NUM_TILES_IN_PRIMARY
        if self.sec and tid < len(self.sec.tiles):
            return self.sec.tiles[tid]
        return None

    def get(self, gid):
        if gid in self.cache:
            return self.cache[gid]
        if gid < NUM_METATILES_IN_PRIMARY:
            ents = self.prim.entries(gid)
        else:
            ents = self.sec.entries(gid - NUM_METATILES_IN_PRIMARY) if self.sec else None
        img = Image.new('RGB', (16, 16), self.backdrop)
        lit = Image.new('L', (16, 16), 0)
        if ents:
            for k, v in enumerate(ents):
                tile = self._tile(v & 0x3FF)
                pal = v >> 12
                if tile is None or pal >= NUM_PALS_TOTAL:
                    continue
                if v & 0x400:
                    tile = tile.transpose(Image.FLIP_LEFT_RIGHT)
                if v & 0x800:
                    tile = tile.transpose(Image.FLIP_TOP_BOTTOM)
                pos = ((k & 1) * 8, ((k >> 1) & 1) * 8)
                mask = tile.point(lambda x: 0 if x == 0 else 255)
                colored = Image.frombytes('P', tile.size, tile.tobytes())
                colored.putpalette(self.flat[pal])
                img.paste(colored.convert('RGB'), pos, mask)
                lit.paste(tile.point(self.lit_lut[pal]), pos, mask)
        self.cache[gid] = (img, lit)
        return self.cache[gid]


# ---------------------------------------------------------------- maps & regions

def load_maps():
    groups = json.loads(read_text('data/maps/map_groups.json'))
    layouts = {l['id']: l for l in json.loads(read_text('data/layouts/layouts.json'))['layouts'] if 'id' in l}
    maps = {}
    for grp in groups['group_order']:
        for name in groups[grp]:
            p = f'data/maps/{name}/map.json'
            if not os.path.exists(rpath(p)):
                continue
            mp = json.loads(read_text(p))
            mp['_name'] = name
            mp['_group'] = grp
            mp['_layout'] = layouts.get(mp.get('layout'))
            maps[mp['id']] = mp
    return maps


def mapsec_regions():
    out = {}
    for region, fname in REGION_LAYOUT_FILES:
        text = read_text('src/data/region_map/' + fname)
        for sec in re.findall(r'\bMAPSEC_\w+', text):
            if sec != 'MAPSEC_NONE':
                out.setdefault(sec, region)
    for sec in re.findall(r'\bMAPSEC_\w+', read_text('src/data/region_map/region_map_layout.h')):
        if sec != 'MAPSEC_NONE':
            out.setdefault(sec, 'hoenn')
    return out


def assign_regions(maps, secs):
    """Region per map: Johto map group, else its region-map section, else the majority of the
    maps it connects or warps to (for sections like Viridian Forest that aren't on any region map)."""
    region = {}
    for i, m in maps.items():
        if m['_group'] == 'gMapGroup_Johto':
            region[i] = 'johto'
        elif m.get('region_map_section') in secs:
            region[i] = secs[m['region_map_section']]
        elif str(m.get('region_map_section', '')).startswith('MAPSEC_KANTO_'):
            region[i] = 'kanto'
    incoming = {}
    for i, m in maps.items():
        for c in m.get('connections') or []:
            incoming.setdefault(c.get('map'), []).append(i)
        for w in m.get('warp_events') or []:
            incoming.setdefault(w.get('dest_map'), []).append(i)
    for _ in range(6):
        for i, m in maps.items():
            if i in region:
                continue
            votes = {}
            near = [c.get('map') for c in m.get('connections') or []]
            near += [w.get('dest_map') for w in m.get('warp_events') or []]
            near += incoming.get(i, [])
            for n in near:
                if n in region:
                    votes[region[n]] = votes.get(region[n], 0) + 1
            if votes:
                region[i] = max(votes, key=votes.get)
    for m in maps.values():
        m['_region'] = region.get(m['id'], 'hoenn')


def region_of(mp, secs=None):
    return mp.get('_region', 'hoenn')


def load_hidden_grottos():
    """{map_id: [(x, y), ...]} of the grotto entrances in HIDDEN_GROTTOS_FILE (commented-out ones skipped)."""
    if not os.path.exists(rpath(HIDDEN_GROTTOS_FILE)):
        return {}
    text = read_text(HIDDEN_GROTTOS_FILE)
    text = re.sub(r'/\*.*?\*/', ' ', text, flags=re.S)
    text = re.sub(r'//[^\n]*', '', text)
    out = {}
    for arr in re.finditer(r'\bsHiddenGrottos\w+\s*\[\s*\]\s*=\s*\{(.*?)\n\};', text, re.S):
        for entry in re.finditer(r'\{([^{}]*)\}', arr.group(1)):
            body = entry.group(1)
            m = re.search(r'\.location\s*=\s*MAP_NUM\(\s*(MAP_\w+)\s*\)', body)
            x = re.search(r'\.x\s*=\s*(\d+)', body)
            y = re.search(r'\.y\s*=\s*(\d+)', body)
            if m and x and y:
                out.setdefault(m.group(1), []).append((int(x.group(1)), int(y.group(1))))
    return out


def stitch(region_maps):
    """Place connected maps on a shared grid (in metatiles). Returns {map_id: (x, y)} grouped by component."""
    ids = set(region_maps)
    placed = {}
    components = []
    for start in sorted(ids, key=lambda i: -(region_maps[i]['_layout']['width'] * region_maps[i]['_layout']['height'])):
        if start in placed:
            continue
        comp = {start: (0, 0)}
        placed[start] = True
        q = deque([start])
        while q:
            a = q.popleft()
            ax, ay = comp[a]
            la = region_maps[a]['_layout']
            for c in region_maps[a].get('connections') or []:
                b = c.get('map')
                if b not in ids or b in placed:
                    continue
                lb = region_maps[b]['_layout']
                o = int(c.get('offset', 0))
                d = c.get('direction')
                if d == 'up':
                    pos = (ax + o, ay - lb['height'])
                elif d == 'down':
                    pos = (ax + o, ay + la['height'])
                elif d == 'left':
                    pos = (ax - lb['width'], ay + o)
                elif d == 'right':
                    pos = (ax + la['width'], ay + o)
                else:
                    continue
                comp[b] = pos
                placed[b] = True
                q.append(b)
        components.append(comp)
    return components


def pack(components, region_maps, gap=4):
    """Shelf-pack connected components into one canvas; returns ({map_id: (x, y)}, (w, h)) in metatiles."""
    boxes = []
    for comp in components:
        x0 = min(x for x, y in comp.values())
        y0 = min(y for x, y in comp.values())
        x1 = max(x + region_maps[m]['_layout']['width'] for m, (x, y) in comp.items())
        y1 = max(y + region_maps[m]['_layout']['height'] for m, (x, y) in comp.items())
        boxes.append((comp, x0, y0, x1 - x0, y1 - y0))
    boxes.sort(key=lambda b: -b[3] * b[4])
    if not boxes:
        return {}, (0, 0)
    main = boxes[0]
    shelf_w = max(main[3], 120)
    out = {}
    for m, (x, y) in main[0].items():
        out[m] = (x - main[1], y - main[2])
    cur_x, cur_y, row_h = 0, main[4] + gap, 0
    for comp, x0, y0, w, h in boxes[1:]:
        if cur_x and cur_x + w > shelf_w:
            cur_x, cur_y, row_h = 0, cur_y + row_h + gap, 0
        for m, (x, y) in comp.items():
            out[m] = (cur_x + x - x0, cur_y + y - y0)
        cur_x += w + gap
        row_h = max(row_h, h)
    W = max(x + region_maps[m]['_layout']['width'] for m, (x, y) in out.items())
    H = max(y + region_maps[m]['_layout']['height'] for m, (x, y) in out.items())
    return out, (W, H)


# ---------------------------------------------------------------- light sprites

class LightSprites:
    def __init__(self):
        self.sprites = {}
        for t, (pic, pal) in LIGHT_SPRITES.items():
            img = Image.open(rpath(pic))
            cols = [tuple(map(to5, c)) for c in read_jasc(pal)]
            idx = Image.frombytes('L', img.size, img.tobytes()).point(lambda v: v & 15)
            self.sprites[t] = (idx, cols)

    def draw(self, canvas, lit_mask, light_type, tile_px_x, tile_px_y):
        light_type = int(light_type)
        if light_type == LIGHT_TYPE_LANTERN_PAIR:
            self.draw(canvas, lit_mask, 0, tile_px_x, tile_px_y)
            self.draw(canvas, lit_mask, 0, tile_px_x + 3 * 16, tile_px_y)
            return
        if light_type in (1, 2):  # signs also light the building's emblem 1 tile up-left
            self.draw(canvas, lit_mask, light_type + 2, tile_px_x - 16, tile_px_y - 16)
        if light_type in (7, 8):  # Kanto signs also light the emblem 1 tile up
            self.draw(canvas, lit_mask, light_type + 2, tile_px_x, tile_px_y - 16)
        light_type = min(light_type, max(self.sprites))
        idx, cols = self.sprites[light_type]
        w, h = idx.size
        if light_type == 0:  # SpawnLightSprite: 32x32, x += 8, y += 22 - 16
            cx, cy = tile_px_x + 8, tile_px_y + 6
        elif light_type == LIGHT_TYPE_DEPT_STORE_EMBLEM:
            cx, cy = tile_px_x + 8, tile_px_y + 8
        elif light_type in KANTO_LIGHT_CENTERS:
            cx, cy = tile_px_x + KANTO_LIGHT_CENTERS[light_type][0], tile_px_y + KANTO_LIGHT_CENTERS[light_type][1]
        else:  # signs (16x16) and emblems (32x32) are centered on the tile's top-left corner
            cx, cy = tile_px_x, tile_px_y
        left, top = cx - w // 2, cy - h // 2
        px = canvas.load()
        lm = lit_mask.load()
        W, H = canvas.size
        for y in range(h):
            for x in range(w):
                c = idx.getpixel((x, y))
                if c == 0:
                    continue
                X, Y = left + x, top + y
                if not (0 <= X < W and 0 <= Y < H):
                    continue
                sr, sg, sb = cols[c]
                br, bg, bb = (v >> 3 for v in px[X, Y])
                px[X, Y] = tuple(to8(min(31, (s * LIGHT_BLEND_EVA + b * LIGHT_BLEND_EVB) >> 4))
                                 for s, b in ((sr, br), (sg, bg), (sb, bb)))
                lm[X, Y] = 255


# ---------------------------------------------------------------- rendering

def render_map_into(src, mp, canvas, lit_mask, ox, oy, night, renderers):
    lay = mp['_layout']
    key = (lay['primary_tileset'], lay['secondary_tileset'])
    if key not in renderers:
        renderers[key] = MetatileRenderer(src, key[0], key[1], night)
    r = renderers[key]
    blk = open(rpath(lay['blockdata_filepath']), 'rb').read()
    w, h = lay['width'], lay['height']
    for y in range(h):
        for x in range(w):
            o = 2 * (y * w + x)
            if o + 2 > len(blk):
                continue
            gid = struct.unpack('<H', blk[o:o + 2])[0] & 0x3FF
            img, lit = r.get(gid)
            canvas.paste(img, (ox + x * 16, oy + y * 16))
            lit_mask.paste(lit, (ox + x * 16, oy + y * 16))
    return r, blk


def metatile_behavior(renderer, blk, w, x, y):
    o = 2 * (y * w + x)
    if not (0 <= x < w) or o < 0 or o + 2 > len(blk):
        return None
    gid = struct.unpack('<H', blk[o:o + 2])[0] & 0x3FF
    if gid < NUM_METATILES_IN_PRIMARY:
        return renderer.prim.behavior(gid)
    return renderer.sec.behavior(gid - NUM_METATILES_IN_PRIMARY) if renderer.sec else None


def find_unlit_doors(mp, maps, lit_mask, ox, oy, renderer, blk, doors):
    """Door warps into indoor maps with no lit pixel in the building face around the door.
    Warps that aren't on a door metatile (hidden warps, stairs, cave mouths, archways) are ignored."""
    out = []
    seen = []
    for wp in mp.get('warp_events') or []:
        dest = maps.get(wp.get('dest_map'))
        if not dest or dest.get('map_type') != 'MAP_TYPE_INDOOR':
            continue
        x, y = int(wp['x']), int(wp['y'])
        if doors and metatile_behavior(renderer, blk, mp['_layout']['width'], x, y) not in doors:
            continue
        if any(abs(x - a) <= 1 and abs(y - b) <= 1 for a, b in seen):
            continue
        seen.append((x, y))
        box = (ox + (x - 4) * 16, oy + (y - 3) * 16, ox + (x + 5) * 16, oy + (y + 1) * 16)
        box = (max(0, box[0]), max(0, box[1]), min(lit_mask.width, box[2]), min(lit_mask.height, box[3]))
        if box[2] <= box[0] or box[3] <= box[1] or lit_mask.crop(box).getbbox() is None:
            out.append((x, y, dest['_name']))
    return out


def render_region(region, args, src, maps, secs):
    night = not args.day
    if region == 'single':
        target = next((m for m in maps.values() if m['_name'].lower() == args.map.lower() or m['id'].lower() == args.map.lower()), None)
        if not target:
            sys.exit(f'Map not found: {args.map}')
        region_maps = {target['id']: target}
        title = target['_name']
    else:
        region_maps = {i: m for i, m in maps.items()
                       if m.get('map_type') in NATURAL_LIGHT_TYPES and m['_layout'] and region_of(m, secs) == region
                       and i != HIDDEN_GROTTO_ROOM}
        title = REGION_TITLES[region]
    if not region_maps:
        print(f'  no outdoor maps found for {title}')
        return None
    positions, (W, H) = pack(stitch(region_maps), region_maps)
    print(f'  {title}: {len(region_maps)} maps, {W}x{H} metatiles ({W * 16}x{H * 16} px)')
    canvas = Image.new('RGB', (W * 16, H * 16), (16, 16, 24))
    lit_mask = Image.new('L', canvas.size, 0)
    renderers = {}
    rendered = {}
    order = sorted(positions.items(), key=lambda kv: kv[1][::-1])
    for i, (mid, (x, y)) in enumerate(order):
        try:
            rendered[mid] = render_map_into(src, region_maps[mid], canvas, lit_mask, x * 16, y * 16, night, renderers)
        except Exception as e:  # keep going so one broken tileset doesn't hide the rest
            print(f'  ! skipped {region_maps[mid]["_name"]}: {e}')
        if (i + 1) % 10 == 0 or i + 1 == len(order):
            print(f'  rendered {i + 1}/{len(order)} maps', flush=True)
    lights = 0
    if night and not args.no_lights:
        ls = LightSprites()
        # A light listed by two connected maps at the same spot only exists once in-game, so draw it once.
        drawn = set()
        for mid, (x, y) in positions.items():
            for o in region_maps[mid].get('object_events') or []:
                if o.get('graphics_id') == 'OBJ_EVENT_GFX_LIGHT_SPRITE' and str(o.get('flag', '0')) in ('0', ''):
                    key = (int(o.get('trainer_sight_or_berry_tree_id', 0) or 0), x + int(o['x']), y + int(o['y']))
                    if key in drawn:
                        continue
                    drawn.add(key)
                    ls.draw(canvas, lit_mask, key[0], key[1] * 16, key[2] * 16)
                    lights += 1
    unlit = []
    doors = door_behaviors()
    for mid, (x, y) in positions.items():
        if mid not in rendered:
            continue
        renderer, blk = rendered[mid]
        for (dx, dy, dest) in find_unlit_doors(region_maps[mid], maps, lit_mask, x * 16, y * 16, renderer, blk, doors):
            unlit.append((region_maps[mid]['_name'], dx, dy, dest, x + dx, y + dy))
    grottos = []
    grotto_spots = load_hidden_grottos()
    for mid, (x, y) in positions.items():
        for gx, gy in grotto_spots.get(mid, []):
            grottos.append((region_maps[mid]['_name'], gx, gy, x + gx, y + gy))
    draw = ImageDraw.Draw(canvas)
    if night and not args.no_marks:
        for name, dx, dy, dest, gx, gy in unlit:
            draw.rectangle((gx * 16 - 1, gy * 16 - 1, gx * 16 + 16, gy * 16 + 16), outline=UNLIT_DOOR_COLOR, width=2)
    if not args.no_marks:
        for name, dx, dy, gx, gy in grottos:
            draw.rectangle((gx * 16 - 1, gy * 16 - 1, gx * 16 + 16, gy * 16 + 16), outline=GROTTO_COLOR, width=2)
    if not args.no_labels:
        for mid, (x, y) in positions.items():
            label = region_maps[mid]['_name']
            draw.rectangle((x * 16 + 2, y * 16 + 2, x * 16 + 6 + 6 * len(label), y * 16 + 14), fill=(0, 0, 0))
            draw.text((x * 16 + 4, y * 16 + 3), label, fill=(255, 255, 255))
    if args.scale > 1:
        canvas = canvas.resize((canvas.width * args.scale, canvas.height * args.scale), Image.NEAREST)
    os.makedirs(args.out, exist_ok=True)
    fname = f"{(title if region == 'single' else region).replace(' ', '_').lower()}_{'night' if night else 'day'}.png"
    out = os.path.join(args.out, fname)
    try:
        canvas.save(out, optimize=False)
    except OSError:
        # The previous image is probably open (locked) in a viewer.
        out = os.path.join(args.out, fname[:-4] + time.strftime('_%H%M%S') + '.png')
        canvas.save(out, optimize=False)
    print(f'  light sprites: {lights}')
    if night:
        print(f'  doors with no light nearby: {len(unlit)}' + (' (outlined in blue)' if unlit and not args.no_marks else ''))
        for name, dx, dy, dest, gx, gy in sorted(unlit):
            print(f'    {name} ({dx},{dy}) -> {dest}')
    print(f'  hidden grottos: {len(grottos)}' + (' (outlined in red)' if grottos and not args.no_marks else ''))
    for name, dx, dy, gx, gy in sorted(grottos):
        print(f'    {name} ({dx},{dy})')
    print(f'  saved {out}')
    return out


def parse_regions(tokens):
    out = []
    for t in tokens:
        for part in re.split(r'[,\s]+', t.strip().lower()):
            if not part:
                continue
            if part == 'all':
                out.extend(['kanto', 'johto', 'hoenn', 'sevii'])
            elif part in REGIONS:
                out.append(REGIONS[part])
            elif part in REGIONS.values():
                out.append(part)
            else:
                sys.exit(f'Unknown region "{part}". Use kanto, johto, hoenn, sevii, 1-4 or all.')
    seen = []
    for r in out:
        if r not in seen:
            seen.append(r)
    return seen


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('regions', nargs='*', help='kanto/johto/hoenn/sevii, 1-4, or all')
    ap.add_argument('--map', help='render a single map by folder name (e.g. PetalburgCity)')
    ap.add_argument('--day', action='store_true')
    ap.add_argument('--no-lights', action='store_true')
    ap.add_argument('--no-labels', action='store_true')
    ap.add_argument('--no-marks', action='store_true')
    ap.add_argument('--scale', type=int, default=1)
    ap.add_argument('--out', default=os.path.join(ROOT, 'tech', 'night_maps'))
    args = ap.parse_args()

    regions = parse_regions(args.regions)
    if not regions and not args.map:
        ap.print_help()
        return 1
    src = Sources()
    maps = load_maps()
    secs = mapsec_regions()
    assign_regions(maps, secs)
    if args.map:
        render_region('single', args, src, maps, secs)
    for r in regions:
        print(f'{REGION_TITLES[r]}:')
        render_region(r, args, src, maps, secs)
    return 0


if __name__ == '__main__':
    sys.exit(main())
