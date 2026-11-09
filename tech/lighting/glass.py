"""Find window glass in a region's outdoor maps and light it at night (see README.md, "Bulk glass lighting").

    python glass.py kanto                      # analyze: report, and out/glass_<map>.png (magenta = glass found)
    python glass.py kanto --apply [--verbose]  # light it

Glass = enclosed clusters of saturated light-blue pixels (plus the bright glints inside them) and small
rectangular dark-navy panes, outside water, doors (lit separately, with their animations) and EXCLUDE.

Lighting keeps each pane's shading: every glass day color gets a warm night color from a ramp that keeps the
light/dark order of the colors. Day colors never change.
  1. Secondary palette colors drawn only as glass on every map using the tileset are lit palette-wide.
  2. Other glass tiles get lit copies packed into free slots of the secondary's palettes (6-12): an overlay of
     just the glass pixels when the tile's top layer is free (or fully opaque, which then moves down), else a
     copy of the whole tile. Glass in primary metatiles is lit through a new copy of the metatile in the
     secondary tileset, and the maps are repointed to it. A window is lit only if all its tiles fit.
"""
import argparse
import copy
import os
import re
import struct
from collections import defaultdict, deque

from lighter import Lighter, set_swap, N, TS, MAPS, BYNAME, OUT, Image, blocks, tile_px

VERBOSE = False
LIGHT = N.DEFAULT_LIGHT
COEFF = N.NIGHT_COEFF
# final night look of glass (5-bit): darkest shade -> lightest shade, and white glints
F_DARK, F_LIGHT, F_GLINT = (28, 21, 11), (31, 29, 20), (31, 31, 27)

# Combing results: metatiles (None = whole metatile) or pixel rects (x0, y0, x1, y1) never to light, for blue
# art the detector takes for glass. Keys: the tileset owning the metatile.
EXCLUDE = {
    'gTileset_KantoGeneral': {0x39: None, 0x3a: None},  # Poke Mart emblem
    'gTileset_ViridianCity': {0x298: [(0, 11, 16, 16)], 0x29a: [(0, 11, 16, 16)], 0x29b: [(0, 11, 16, 16)],
                              0x29c: [(0, 11, 16, 16)], 0x2a0: None, 0x2a1: None, 0x2a2: None, 0x2a3: None,
                              0x2a4: None},  # blue flowers in window planters
    'gTileset_SaffronCity': {g: None for g in (0x30d, 0x31a, 0x31d, 0x31e, 0x329, 0x32a, 0x32d, 0x32e)},  # railings
    'gTileset_CinnabarIsland': {0x2ca: None, 0x2cb: None, 0x2cc: None},  # Route 20 rock/shore edges
    'gTileset_SeviiIslands123': {0x2a0: None, 0x2b9: None, 0x2bc: None,  # Two Island stall: display case, jars
                                 0x2ec: None},  # Network Center door's right half (lit by doorglass.py)
    'gTileset_LavenderTown': {g: None for g in (0x320, 0x321, 0x323, 0x328, 0x329, 0x32a, 0x32b, 0x330, 0x331,
                                                0x332, 0x333)},  # Power Plant (abandoned): stays dark
    'gTileset_CeruleanCity': {0x2e9: None,  # bikes outside the bike shop
                              0x2fc: None, 0x2fd: None, 0x304: None, 0x2ff: None},  # Route 5 daycare wall base
    # Johto. Primaries: Mart sign and emblem (lit by sprites), the sliver of gym door glass beside the door,
    # lamp-post bases (the lamp heads are glass and stay)
    'gTileset_JohtoGeneral': {0x18e: None, 0x18f: None, 0x194: None, 0x197: None, 0x15a: None, 0x15c: None, 0x3a: None},
    'gTileset_JohtoSouth': {0x18e: None, 0x18f: None, 0x194: None, 0x197: None, 0x15a: None, 0x15c: None},
    'gTileset_JohtoNorthWest': {0x18e: None, 0x18f: None, 0x194: None, 0x197: None},
    'gTileset_JohtoNorthEast': {0xa5: None, 0xac: None, 0x16a: None, 0x16b: None, 0x15a: None, 0x15c: None},
    'gTileset_AzaleaTown': {0x2ca: None},  # charcoal pot
    'gTileset_Blackthorn': {0x288: None, 0x289: None, 0x28a: None, 0x28b: None},  # latticed upper walls (bits only)
    # the blue panel beside each house door, and its top in the metatile above
    'gTileset_CherrygroveCity': {0x2e2: None, 0x2e4: None, 0x301: None, 0x303: None, 0x2ec: None,
                                 0x2ea: [(12, 0, 16, 16)], 0x309: [(12, 0, 16, 16)], 0x30b: [(0, 0, 4, 16)]},
    'gTileset_CianwoodCity': {**{g: None for g in (0x281, 0x282, 0x284, 0x289, 0x28a, 0x28c, 0x291, 0x292, 0x293,
                                                   0x294, 0x2c0, 0x2c1, 0x2c2, 0x2ca, 0x31c)},  # blue tiled roofs
                              0x296: None, 0x297: None,  # gym door glass sliver
                              0x342: None, 0x345: None},  # Route 41 rocks
    'gTileset_EcruteakCityNw': {0x39a: None, 0x392: None, 0x3a3: None},  # gym door sliver, Mart sign
    'gTileset_VioletCity': {0x3b2: None, 0x3b3: None, 0x3a5: None,  # gym door sliver, Mart sign
                            0x332: None, 0x333: None, 0x2ba: None, 0x2bb: None},  # statue pedestals
    'gTileset_Goldenrod': {0x3c1: None, 0x3c5: None,  # lamp-post bases (Goldenrod, Route 34)
                           0x3dc: None, 0x3dd: None, 0x3de: None, 0x3df: None,  # railing below the tracks
                           0x328: None, 0x332: None, 0x3e3: None, 0x3e4: None, 0x3ed: None,  # railings
                           0x323: None, 0x32b: None,  # Route 34 fence pipes
                           0x3ee: None, 0x3f7: None, 0x3fe: None, 0x3ff: None,  # bikes outside the bike shop
                           0x3aa: None, 0x3ac: None},  # Pokemon Center: only a strip of the pale window
    'gTileset_MahoganyTown': {g: None for g in (0x308, 0x309, 0x310, 0x311, 0x332, 0x333)},  # Lake of Rage trees
    'gTileset_NationalPark': {0x2eb: None, 0x29b: None},  # lamp-post base, trash can
    'gTileset_NewBarkTown': {**{g: [(0, 13, 16, 16)] for g in (0x298, 0x29b, 0x29c, 0x2f0, 0x2f1, 0x2f4)},
                             **{g: None for g in (0x2a6, 0x2b7, 0x2c7, 0x2cf, 0x2f8, 0x2f9, 0x2fc)}},  # planters
    'gTileset_OlivineCity': {**{g: None for g in (0x351, 0x352, 0x358, 0x35b, 0x3a4, 0x3d0)},  # lamp bases, bunting
                             0x305: None, 0x306: None, 0x30d: None, 0x30e: None,  # Route 40 beach umbrella
                             # Glitter Lighthouse: the lamp room stays, the blue stripes down the tower don't
                             0x2d1: [(0, 5, 16, 16)], 0x2d2: [(0, 5, 16, 16)], 0x2d3: [(0, 5, 16, 16)],
                             **{g: None for g in (0x2d9, 0x2da, 0x2db, 0x2e1, 0x2e2, 0x2e3, 0x2e9, 0x2ea, 0x2eb,
                                                  0x2f1, 0x2f2, 0x2f3, 0x2f9, 0x2fa, 0x2fb)}},
    'gTileset_JohtoTrainerHillCourtyard': {g: None for g in (0x3e4, 0x369, 0x371, 0x379, 0x37a, 0x381, 0x382,
                                                             0x38a, 0x36d, 0x375, 0x37c, 0x37d, 0x384, 0x385,
                                                             0x38c)},  # lamp base; the domes' blue panels
    'gTileset_ViridianCityJohto': {0x298: [(0, 13, 16, 16)], 0x29c: [(0, 13, 16, 16)],
                                   0x313: None, 0x314: None, 0x315: None, 0x316: None},  # Route 28 planters
}


def excluded(ts_names, gid, x, y):
    for n in ts_names:
        rects = EXCLUDE.get(n, {}).get(gid, False)
        if rects is None:
            return True
        if rects and any(x0 <= x < x1 and y0 <= y < y1 for x0, y0, x1, y1 in rects):
            return True
    return False


def water_behaviors():
    text = N.read_text('include/constants/metatile_behaviors.h')
    vals = {}
    for name, v in re.findall(r'#define (MB_\w+)\s+(0x[0-9A-Fa-f]+|\d+)', text):
        vals[name] = int(v, 0)
    if not vals:  # enum form
        body = text[text.index('enum'):]
        n = 0
        for name, v in re.findall(r'(MB_\w+)\s*(?:=\s*(\w+))?,', body):
            n = int(v, 0) if v else n
            vals[name] = n
            n += 1
    keys = ('WATER', 'POND', 'OCEAN', 'WATERFALL', 'PUDDLE', 'SEAWEED', 'CURRENT', 'DIVE', 'SEA')
    return {v for k, v in vals.items() if any(s in k for s in keys)}


def lum(c):
    return c[0] * 0.3 + c[1] * 0.59 + c[2] * 0.11


def is_glass_color(c):
    r, g, b = c
    # blue/cyan glass; g >= r keeps out purples and lavenders
    return b >= 150 and b - r >= 40 and b >= g and g >= r


def is_dark_glass_color(c):
    r, g, b = c
    return lum(c) < 100 and b - r >= 20 and b >= g


def night_for(final5):
    """Night palette color (8-bit) whose lit blend toward the light color lands on final5."""
    out = []
    for f, l in zip(final5, LIGHT):
        # final = n + (l - n) * COEFF / 32  ->  n = (32 * final - COEFF * l) / (32 - COEFF)
        n = round((32 * f - COEFF * l) / (32 - COEFF))
        out.append(max(0, min(31, n)) << 3)
    return tuple(out)


def ramp(colors):
    """Night palette colors for a set of glass day colors: same light/dark order, warm."""
    shades = sorted({c for c in colors if lum(c) <= 200}, key=lum)
    out = {}
    for i, c in enumerate(shades):
        t = i / max(1, len(shades) - 1)
        out[c] = night_for(tuple(round(a + (b - a) * t) for a, b in zip(F_DARK, F_LIGHT)))
    for c in colors:
        if c not in out:
            out[c] = night_for(F_GLINT)
    return out


class MapScan:
    """Day colors and (palette, index, metatile, entry) of every visible pixel of a map."""
    def __init__(self, name, water):
        lay = BYNAME[name]['_layout']
        self.name = name
        self.prim, self.sec = TS(lay['primary_tileset']), TS(lay['secondary_tileset'])
        self.r = N.MetatileRenderer(N.Sources(), lay['primary_tileset'], lay['secondary_tileset'], False)
        self.b = blocks(name)
        self.h, self.w = len(self.b), len(self.b[0])
        W, H = self.w * 16, self.h * 16
        self.rgb = [[None] * W for _ in range(H)]
        self.src = [[None] * W for _ in range(H)]   # (P, c, gid, k)
        self.wet = [[False] * W for _ in range(H)]
        cache = {}
        self.door_cells = set()
        door_set = set(N.door_behaviors())
        # Kanto and Johto doors animate only their own metatile (DOOR_SIZE_KANTO_SMALL); Hoenn ones also the one above
        above = 0 if (lay['primary_tileset'] == 'gTileset_KantoGeneral'
                      or lay['primary_tileset'].startswith('gTileset_Johto')) else 1
        for my in range(self.h):
            for mx in range(self.w):
                gid = self.b[my][mx]
                if self.behavior(gid) in door_set:
                    self.door_cells |= {(mx, my), (mx, my - above)}
                if gid not in cache:
                    cache[gid] = self._meta(gid, water)
                cols, src, wet = cache[gid]
                for y in range(16):
                    for x in range(16):
                        self.rgb[my * 16 + y][mx * 16 + x] = cols[y][x]
                        self.src[my * 16 + y][mx * 16 + x] = src[y][x]
                        self.wet[my * 16 + y][mx * 16 + x] = wet
        # (palette, index) pairs drawn by water metatiles anywhere on this map
        self.water_keys = {self.src[y][x][:2] for y in range(H) for x in range(W)
                           if self.wet[y][x] and self.src[y][x] and self.src[y][x][1]}

    def behavior(self, gid):
        if gid < 512:
            return self.r.prim.behavior(gid)
        return self.r.sec.behavior(gid - 512) if self.r.sec else 0

    def _meta(self, gid, water):
        ts = self.prim if gid < 512 else self.sec
        try:
            e = ts.entries(gid)
        except Exception:
            return [[(0, 0, 0)] * 16 for _ in range(16)], [[None] * 16 for _ in range(16)], False
        cols = [[(0, 0, 0)] * 16 for _ in range(16)]
        src = [[None] * 16 for _ in range(16)]
        pals = self.r.pals.colors
        for k in range(8):
            t, hf, vf, P = e[k]
            if P >= 13:
                continue
            try:
                g = tile_px(self.prim, self.sec, t)
            except IndexError:
                continue
            q = k % 4
            for j in range(8):
                for i in range(8):
                    c = g[7 - j if vf else j][7 - i if hf else i]
                    if k >= 4 and c == 0:
                        continue
                    y, x = (q // 2) * 8 + j, (q % 2) * 8 + i
                    cols[y][x] = pals[P][c]
                    src[y][x] = (P, c, gid, k)
        return cols, src, self.behavior(gid) in water


def find_glass(scan, max_area=400, max_w=40, max_h=32, mullioned=False):
    """Window panes: clusters of light-blue glass pixels, plus small rectangular dark-navy panes. mullioned: also
    panes only 2 px on a side when they're part of a grid (another such pane within 2 px), e.g. Johto's divided
    windows."""
    light = _find(scan, is_glass_color, max_area, max_w, max_h, rect=0, sat=90, min_wh=3)
    if mullioned:
        small = _find(scan, is_glass_color, 24, 8, 8, rect=0.75, sat=90, min_wh=2) - light
        light |= {p for p in small if sum(1 for dx in range(-3, 4) for dy in range(-3, 4)
                                          if (p[0] + dx, p[1] + dy) in small) > 8}
    dark = _find(scan, is_dark_glass_color, 48, 8, 12, rect=0.75, min_wh=3)
    keep = set()
    for (x, y) in light | dark:
        if (x // 16, y // 16) in scan.door_cells:
            continue
        src = scan.src[y][x]
        if src:
            owner = scan.prim.name if src[2] < 512 else scan.sec.name
            if excluded([owner], src[2], x % 16, y % 16):
                continue
        keep.add((x, y))
    return keep


def _find(scan, pred, max_area, max_w, max_h, rect, min_wh=1, sat=0):
    H, W = len(scan.rgb), len(scan.rgb[0])
    cand = [[pred(scan.rgb[y][x]) and not scan.wet[y][x] for x in range(W)] for y in range(H)]
    seen = [[False] * W for _ in range(H)]
    glass = set()
    for y0 in range(H):
        for x0 in range(W):
            if not cand[y0][x0] or seen[y0][x0]:
                continue
            comp, q = [], deque([(x0, y0)])
            seen[y0][x0] = True
            while q:
                x, y = q.popleft()
                comp.append((x, y))
                for nx, ny in ((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)):
                    if 0 <= nx < W and 0 <= ny < H and cand[ny][nx] and not seen[ny][nx]:
                        seen[ny][nx] = True
                        q.append((nx, ny))
            xs, ys = [p[0] for p in comp], [p[1] for p in comp]
            bw, bh = max(xs) - min(xs) + 1, max(ys) - min(ys) + 1
            if len(comp) < 4 or len(comp) > max_area or bw > max_w or bh > max_h or min(bw, bh) < min_wh:
                continue
            if rect and len(comp) < rect * bw * bh:
                continue
            # glass is a saturated blue somewhere; pale blue-grey walls and roof trim aren't
            if sat and max(scan.rgb[y][x][2] - scan.rgb[y][x][0] for x, y in comp) < sat:
                continue
            if min(xs) == 0 or min(ys) == 0 or max(xs) == W - 1 or max(ys) == H - 1:
                continue
            # water drawn by shore/corner metatiles without a water behavior: water colors, next to water
            wk = scan.water_keys
            if wk and sum(1 for x, y in comp if scan.src[y][x] and scan.src[y][x][:2] in wk) > 0.3 * len(comp):
                cells = {(x // 16, y // 16) for x, y in comp}
                if any(scan.wet[min(H - 1, max(0, (cy + dy) * 16))][min(W - 1, max(0, (cx + dx) * 16))]
                       for cx, cy in cells for dx in (-1, 0, 1) for dy in (-1, 0, 1)):
                    continue
            cs = set(comp)
            # glints: bright pixels enclosed by the pane (not reachable from the bbox edge around it)
            x0b, x1b, y0b, y1b = min(xs) - 1, max(xs) + 1, min(ys) - 1, max(ys) + 1
            outside, q = set(), deque()
            for x in range(x0b, x1b + 1):
                q.extend([(x, y0b), (x, y1b)])
            for y in range(y0b, y1b + 1):
                q.extend([(x0b, y), (x1b, y)])
            while q:
                p = q.popleft()
                if p in outside or p in cs or not (x0b <= p[0] <= x1b and y0b <= p[1] <= y1b):
                    continue
                outside.add(p)
                x, y = p
                q.extend([(x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)])
            for y in range(y0b + 1, y1b):
                for x in range(x0b + 1, x1b):
                    if (x, y) not in cs and (x, y) not in outside and lum(scan.rgb[y][x]) > 170:
                        cs.add((x, y))
            glass |= cs
    return glass


REGION_ORDER = ('hoenn', 'kanto', 'sevii', 'johto')  # tie-break for owned_secondaries (regions done first win)
# Secondaries another region owns (and lit, committed) with doors on this region's maps: the region's pass lights
# only those doors (doorglass.py). --reset restores them to HEAD without unlighting them.
EXTRA_DOOR_SECONDARIES = {'johto': {'gTileset_IndigoPlateau'}}  # Indigo Plateau (Route 23 makes it Kanto's)


def owned_secondaries(region):
    """Secondary tilesets a region's lighting pass owns: those its outdoor maps use most (shared tilesets such as
    gTileset_CeladonCity, also used by a few Sevii and prototype maps, belong to one region only)."""
    from collections import Counter, defaultdict
    secs = N.mapsec_regions()
    N.assign_regions(MAPS, secs)
    cnt = defaultdict(Counter)
    for m in MAPS.values():
        lay = m['_layout']
        if lay and m.get('map_type') in N.NATURAL_LIGHT_TYPES:
            cnt[lay['secondary_tileset']][N.region_of(m, secs)] += 1
    owner = {s: max(c, key=lambda r: (c[r], -REGION_ORDER.index(r) if r in REGION_ORDER else -99))
             for s, c in cnt.items()}
    return {s for s, r in owner.items() if r == region}


def analyze(region):
    secs = N.mapsec_regions()
    N.assign_regions(MAPS, secs)
    owned = owned_secondaries(region)
    names = sorted(m['_name'] for m in MAPS.values() if m['_layout'] and m.get('map_type') in N.NATURAL_LIGHT_TYPES
                   and N.region_of(m, secs) == region and m['_layout']['secondary_tileset'] in owned)
    # also scan other outdoor maps sharing these secondaries, so palette-wide decisions see every use
    shared = {BYNAME[n]['_layout']['secondary_tileset'] for n in names}
    prim = BYNAME[names[0]]['_layout']['primary_tileset']
    names += sorted(m['_name'] for m in MAPS.values() if m['_layout'] and m.get('map_type') in N.NATURAL_LIGHT_TYPES
                    and m['_name'] not in names and m['_layout']['secondary_tileset'] in shared
                    and m['_layout']['primary_tileset'] == prim)
    water = water_behaviors()
    scans = {}
    for n in names:
        s = MapScan(n, water)
        scans[n] = (s, find_glass(s, mullioned=(region == 'johto')))
    return names, scans


def report(names, scans):
    for n in names:
        s, g = scans[n]
        r = N.MetatileRenderer(N.Sources(), s.prim.name, s.sec.name, False)
        canvas = Image.new('RGB', (s.w * 16, s.h * 16))
        for my in range(s.h):
            for mx in range(s.w):
                canvas.paste(r.get(s.b[my][mx])[0].convert('RGB'), (mx * 16, my * 16))
        px = canvas.load()
        for (x, y) in g:
            px[x, y] = (255, 0, 255)
        canvas.resize((canvas.width * 2, canvas.height * 2), Image.NEAREST).save(os.path.join(OUT, f'glass_{n}.png'))
        print(f'{n}: {len(g)} glass px')


def labelled_metatiles(ts_name):
    """Metatile ids named in metatile_labels.h for a tileset (scripts may set or compare them)."""
    text = N.read_text('include/constants/metatile_labels.h')
    short = ts_name.replace('gTileset_', '')
    return {int(v, 16) for k, v in re.findall(r'#define METATILE_(\w+)\s+(0x[0-9A-Fa-f]+)', text)
            if k.startswith(short + '_')}


def unused_metatiles(ts):
    """Secondary metatile ids (global) that no layout's map or border uses and no label names: free slots for
    copies once the secondary has all 512 metatiles."""
    import json
    lays = json.load(open(N.rpath('data/layouts/layouts.json')))['layouts']
    used = set(labelled_metatiles(ts.name))
    for lay in lays:
        if lay.get('secondary_tileset') != ts.name:
            continue
        for key in ('blockdata_filepath', 'border_filepath'):
            b = open(N.rpath(lay[key]), 'rb').read()
            used |= {v & 0x3FF for v in struct.unpack('<%dH' % (len(b) // 2), b)}
    return [g for g in range(ts.base, ts.base + ts.n()) if g not in used and g != 0x3FF]  # 0x3FF: MAPGRID_UNDEFINED


def append_metatile(ts, entries, attr):
    """Add a metatile to a secondary tileset: appended while there's room for 512, else written over an unused
    one. Returns its global id."""
    words = struct.pack('<8H', *[t | (int(h) << 10) | (int(v) << 11) | (p << 12) for t, h, v, p in entries])
    a = bytearray(open(ts.attr_path, 'rb').read())
    if ts.n() < 512:
        ts.mt += words
        a += struct.pack('<H', attr)
        gid = ts.base + ts.n() - 1
    else:
        if not hasattr(ts, 'free_slots'):
            ts.free_slots = unused_metatiles(ts)
        if not ts.free_slots:
            raise RuntimeError(f'{ts.name}: no free metatile for a copy')
        gid = ts.free_slots.pop()
        o = gid - ts.base
        ts.mt[o * 16:o * 16 + 16] = words
        a[o * 2:o * 2 + 2] = struct.pack('<H', attr)
    open(ts.attr_path, 'wb').write(bytes(a))
    return gid


def drop_metatile(ts, gid):
    """Undo append_metatile(ts, ...) that returned gid: remove an appended one, or free a reused slot."""
    if gid == ts.base + ts.n() - 1 and not hasattr(ts, 'free_slots'):
        ts.mt = ts.mt[:-16]
        a = bytearray(open(ts.attr_path, 'rb').read())[:-2]
        open(ts.attr_path, 'wb').write(bytes(a))
    else:
        ts.free_slots.append(gid)


def attr_of(ts, gid):
    a = open(ts.attr_path, 'rb').read()
    o = (gid - ts.base) * 2
    return struct.unpack('<H', a[o:o + 2])[0]


class PalettePool:
    """Free slots of a secondary tileset's palettes 6-12, for packing lit copies of glass tiles."""
    def __init__(self, L):
        self.L, self.S = L, L.ts
        self.cols, self.night, self.lit, self.free = {}, {}, {}, {}
        for P in range(6, 13):
            cols = [tuple(c) for c in self.S.pal(P)]
            night = [tuple(c) for c in self.S.pal((P + 9) % 16)] if P in self.S.swap else list(cols)
            used = L.used_visible(P)
            self.cols[P], self.night[P], self.lit[P] = cols, night, set(self.S.pla(P))
            self.free[P] = [j for j in range(1, 16) if j not in used]

    def _find(self, P, key):
        day, lit, night = key
        for j in range(1, 16):
            if j in self.free[P]:
                continue
            if self.cols[P][j] == day and (j in self.lit[P]) == lit and (night if lit else day) == self.night[P][j]:
                return j
        return None

    def fit(self, keys):
        """(palette, {key: index}) using the fewest new slots, or None."""
        best = None
        for P in range(6, 13):
            got, need = {}, []
            for key in keys:
                j = self._find(P, key)
                if j is None:
                    need.append(key)
                else:
                    got[key] = j
            if len(need) <= len(self.free[P]) and (best is None or len(need) < best[0]):
                best = (len(need), P, got, need)
        if best is None:
            return None
        _, P, got, need = best
        for key in need:
            j = self.free[P].pop(0)
            day, lit, night = key
            self.cols[P][j], self.night[P][j] = day, (night if lit else day)
            (self.lit[P].add if lit else self.lit[P].discard)(j)
            got[key] = j
        return P, got

    def save(self, touched):
        S = self.S
        for P in touched:
            S.write_pal(P, self.cols[P])
            S.write_pla(P, self.lit[P])
            if self.lit[P] or P in S.swap:
                if P not in S.swap:
                    set_swap(S.name, P)
                    S.swap.add(P)
                S.write_pal((P + 9) % 16, self.night[P])


def apply_glass(names, scans, doors_ok=False, palette_wide=True, ramp_fn=None):
    """doors_ok: also light pixels on door metatiles (open doorways). palette_wide: allow step 1.
    ramp_fn: day colors -> {day: night palette color} (default ramp)."""
    ramp_ = ramp_fn or ramp
    doors = set(N.door_behaviors())
    units = defaultdict(lambda: defaultdict(set))   # sec -> (gid, k) -> {(x, y) metatile coords}
    for n in names:
        s, g = scans[n]
        for (x, y) in g:
            src = s.src[y][x]
            if src:
                units[s.sec.name][(src[2], src[3])].add((x % 16, y % 16))
    for sn, ents in sorted(units.items()):
        L = Lighter(sn)
        S, G = L.ts, L.gen
        skip = labelled_metatiles(G.name) | labelled_metatiles(sn)

        # windows: glass pixels within 2 px of each other form one window, as a set of (gid, quadrant) units
        windows = []
        for n in names:
            s, g = scans[n]
            if s.sec.name != sn:
                continue
            seen = set()
            for p0 in g:
                if p0 in seen:
                    continue
                seen.add(p0)
                comp, q2 = [], deque([p0])
                while q2:
                    x, y = q2.popleft()
                    comp.append((x, y))
                    for dx in range(-2, 3):
                        for dy in range(-2, 3):
                            np_ = (x + dx, y + dy)
                            if np_ in g and np_ not in seen:
                                seen.add(np_)
                                q2.append(np_)
                windows.append(frozenset((s.src[y][x][2], s.src[y][x][3] % 4) for (x, y) in comp if s.src[y][x]))
        windows = list(set(windows))

        # 1. secondary palette colors drawn only as glass on every map using this tileset: light palette-wide
        hits = defaultdict(lambda: [0, 0])
        for n in names:
            s, g = scans[n]
            if s.sec.name != sn:
                continue
            for y, row in enumerate(s.src):
                for x, src in enumerate(row):
                    if src and src[0] >= 6 and src[1]:
                        glass_px = (x, y) in g and src[2] not in skip
                        hits[(src[0], src[1])][0 if glass_px else 1] += 1
        wide = {pc for pc, (gl, ot) in hits.items() if gl and not ot} if palette_wide else set()
        if wide:
            nights = ramp_({tuple(S.pal(P)[c]) for P, c in wide})
            for P in sorted({P for P, _ in wide}):
                idxs = {c for PP, c in wide if PP == P}
                cols = S.pal(P)
                if P not in S.swap:
                    S.write_pal((P + 9) % 16, cols)
                    set_swap(sn, P)
                    S.swap.add(P)
                nc = S.pal((P + 9) % 16)
                for c in idxs:
                    nc[c] = nights[tuple(cols[c])]
                S.write_pal((P + 9) % 16, nc)
                S.write_pla(P, set(S.pla(P)) | idxs)
            print(f'  {sn}: palette-wide glass {sorted(wide)}')

        # 2. the rest, tile by tile
        quads = defaultdict(lambda: defaultdict(set))  # (gid, q) -> layer k -> {(x, y) metatile coords}
        for (gid, k), pts in ents.items():
            ts = G if gid < 512 else S
            if (attr_of(ts, gid) & 0xFF in doors and not doors_ok) or gid in skip:
                continue
            t, hf, vf, P = ts.entries(gid)[k]
            if P >= 6:  # drop pixels already lit palette-wide
                g0 = tile_px(G, S, t)
                pts = {(x, y) for (x, y) in pts
                       if (P, g0[7 - (y % 8) if vf else y % 8][7 - (x % 8) if hf else x % 8]) not in wide}
            if pts:
                quads[(gid, k % 4)][k] |= pts

        def keep(P, c):
            """Key for a pixel a tile copy doesn't newly light: its current look, incl. any light it has already
            (palette-wide glass, earlier passes), so copying a tile into another palette never unlights it."""
            src_ts = G if P < 6 else S
            day = tuple(src_ts.pal(P)[c])
            if c in src_ts.pla(P):
                night = tuple(src_ts.pal((P + 9) % 16)[c]) if P in src_ts.swap else day
                return (day, True, night)
            return (day, False, day)

        def daycol(P, c):
            return tuple((G if P < 6 else S).pal(P)[c])

        def pix(ent, x, y):
            t, hf, vf, P = ent
            return tile_px(G, S, t)[7 - (y % 8) if vf else y % 8][7 - (x % 8) if hf else x % 8]
        allglass = set()
        for (gid, q), byk in quads.items():
            e = (G if gid < 512 else S).entries(gid)
            for k, pts in byk.items():
                allglass |= {daycol(e[k][3], pix(e[k], x, y)) for (x, y) in pts}
        nights = ramp_(allglass)
        pool = PalettePool(L)
        touched, entries, layer_cov = set(), {}, set()
        failed = overlays = 0
        prepared, unfit = [], set()
        for (gid, q), byk in quads.items():
            e = (G if gid < 512 else S).entries(gid)
            bot, top = e[q], e[4 + q]
            top_px = tile_px(G, S, top[0]) if top[0] else None
            top_full = top_px is not None and all(c for r in top_px for c in r)
            # overlay: glass on the bottom with an empty top, or on a fully opaque top
            if (set(byk) == {q} and not top[0]) or (set(byk) == {4 + q} and top_full):
                k = next(iter(byk))
                keys = {}
                for (x, y) in byk[k]:
                    day = daycol(e[k][3], pix(e[k], x, y))
                    keys[(x % 8, y % 8)] = (day, True, nights[day])
                prepared.append((len(set(keys.values())), gid, q, 'overlay', k, keys))
            elif len(byk) == 1:
                k = next(iter(byk))
                t, hf, vf, P = e[k]
                g = tile_px(G, S, t)
                tp = {(7 - (x % 8) if hf else x % 8, 7 - (y % 8) if vf else y % 8) for (x, y) in byk[k]}
                keys = {}
                for j in range(8):
                    for i in range(8):
                        c = g[j][i]
                        if c:
                            day = daycol(P, c)
                            keys[(i, j)] = (day, True, nights[day]) if (i, j) in tp else keep(P, c)
                prepared.append((len(set(keys.values())), gid, q, 'tile', k, keys))
            elif set(byk) == {q, 4 + q} and bot[0] and top[0]:
                # glass on both layers (top tile partly transparent over it): one merged copy on the bottom layer,
                # top layer emptied. Fine for tiles drawn under sprites anyway (glass is never walked behind).
                def oriented(ent):
                    t, hf, vf, _ = ent
                    gg = tile_px(G, S, t)
                    return [[gg[7 - j if vf else j][7 - i if hf else i] for i in range(8)] for j in range(8)]
                gb, gt = oriented(bot), oriented(top)
                lit_px = {(x % 8, y % 8) for kk in byk for (x, y) in byk[kk]}
                keys = {}
                for j in range(8):
                    for i in range(8):
                        P, c = (top[3], gt[j][i]) if gt[j][i] else (bot[3], gb[j][i])
                        if not c and not gt[j][i]:
                            continue
                        day = daycol(P, c)
                        keys[(i, j)] = (day, True, nights[day]) if (i, j) in lit_px else keep(P, c)
                prepared.append((len(set(keys.values())), gid, q, 'merged', q, keys))
            else:
                unfit.add((gid, q))

        # pack tile by tile (trying cheapest-first and biggest-first), then unlight tiles that only appear in
        # windows left half lit and pack again with the freed slots
        def run(items, order, p):
            placed, fails = [], set()
            for r in sorted(items, key=order):
                f = p.fit(set(r[5].values()))
                if f is None:
                    fails.add((r[1], r[2]))
                else:
                    placed.append((r, f))
            return placed, fails
        orders = [lambda r: (r[3] != 'overlay', r[0]), lambda r: (r[3] != 'overlay', -r[0])]
        dropped = set()
        for _ in range(4):
            items = [r for r in prepared if (r[1], r[2]) not in dropped]
            order = min(orders, key=lambda o: len(run(items, o, PalettePool(L))[1]))
            placed, fails = run(items, order, PalettePool(L))
            bad = fails | unfit | dropped
            lit_units = {(r[1], r[2]) for r, _ in placed}
            partial = [w for w in windows if w & bad and w & lit_units]
            more = {u for u in lit_units
                    if all(w in partial for w in windows if u in w) and any(u in w for w in partial)}
            if not more:
                break
            dropped |= more
        pool = PalettePool(L)
        placed, fails = run([r for r in prepared if (r[1], r[2]) not in dropped], order, pool)
        failed = len(fails | unfit | dropped)
        if VERBOSE:
            for w in windows:
                if w & (fails | unfit | dropped) and w & {(r[1], r[2]) for r, _ in placed}:
                    print('    half-lit window', sn, sorted((hex(g_), q_) for g_, q_ in w)[:6])
        for (_, gid, q, kind, k, keys), (Pn, idx) in placed:
            touched.add(Pn)
            e = (G if gid < 512 else S).entries(gid)
            g = [[0] * 8 for _ in range(8)]
            for (i, j), key in keys.items():
                g[j][i] = idx[key]
            if kind == 'overlay':
                if k >= 4:  # opaque top tile moves down, the overlay takes its place
                    entries[(gid, q)] = e[4 + q]
                entries[(gid, 4 + q)] = (L._store(g), False, False, Pn)
                layer_cov.add(gid)
                overlays += 1
            elif kind == 'merged':
                entries[(gid, q)] = (L._store(g), False, False, Pn)
                entries[(gid, 4 + q)] = (0, False, False, 0)
            else:
                entries[(gid, k)] = (L._store(g), e[k][1], e[k][2], Pn)
        pool.save(touched)

        # write metatiles: secondary ones in place, primary ones as new secondary copies
        new_ids = {}
        by_gid = defaultdict(dict)
        for (gid, k), ent in entries.items():
            by_gid[gid][k] = ent
        for gid, ks in by_gid.items():
            src_ts = G if gid < 512 else S
            attr = attr_of(src_ts, gid)
            if gid in layer_cov and attr >> 12 == 0:  # NORMAL -> COVERED, so the overlay stays under sprites
                attr = (attr & 0x0FFF) | (1 << 12)
            if gid < 512:
                e = G.entries(gid)
                for k, ent in ks.items():
                    e[k] = ent
                new_ids[gid] = append_metatile(S, e, attr)
            else:
                for k, (t, hf, vf, p) in ks.items():
                    S.set_entry(gid, k, tile=t, pal=p, hf=hf, vf=vf)
                if attr != attr_of(S, gid):
                    S.set_layer(gid, attr >> 12)
        L.save()
        maps = sorted(m['_name'] for m in MAPS.values() if m['_layout'] and m['_layout']['secondary_tileset'] == sn)
        for mn in maps:
            p = N.rpath(BYNAME[mn]['_layout']['blockdata_filepath'])
            b = bytearray(open(p, 'rb').read())
            cnt = 0
            for o in range(0, len(b) - 1, 2):
                v = struct.unpack('<H', b[o:o + 2])[0]
                if (v & 0x3FF) in new_ids:
                    b[o:o + 2] = struct.pack('<H', (v & ~0x3FF) | new_ids[v & 0x3FF])
                    cnt += 1
            if cnt:
                open(p, 'wb').write(bytes(b))
        print(f'{sn}: {len(entries)} glass entries ({overlays} overlays) lit in pals {sorted(touched)}, '
              f'{len(new_ids)} primary metatile copies, {failed} tiles did not fit')


def reset(region):
    """Undo all night lighting on the region's secondary tilesets: tiles, metatiles and palettes back to git HEAD,
    then every .pla and .swapPalettes entry removed (headers.h is edited, not checked out, since it holds other
    regions' work), and the layouts of maps using those tilesets restored (primary-copy repointing)."""
    import subprocess
    secs = N.mapsec_regions()
    N.assign_regions(MAPS, secs)
    sec_names = sorted(owned_secondaries(region))
    dirs, layouts = [], set()
    for sn in sec_names:
        dirs.append(os.path.relpath(TS(sn).dir, N.ROOT).replace(os.sep, '/'))
        layouts |= {m['_layout']['blockdata_filepath'] for m in MAPS.values()
                    if m['_layout'] and m['_layout']['secondary_tileset'] == sn}
    subprocess.run(['git', 'checkout', 'HEAD', '--'] + dirs + sorted(layouts), cwd=N.ROOT, check=True)
    subprocess.run(['git', 'clean', '-fdq', '--'] + dirs, cwd=N.ROOT, check=True)
    text = open(N.rpath('src/data/tilesets/headers.h'), encoding='utf-8').read()
    # door-only extras: back to HEAD as committed (lit by their owner), header entry included
    extras = sorted(EXTRA_DOOR_SECONDARIES.get(region, ()))
    if extras:
        xdirs = [os.path.relpath(TS(sn).dir, N.ROOT).replace(os.sep, '/') for sn in extras]
        xlay = sorted({m['_layout']['blockdata_filepath'] for m in MAPS.values()
                       if m['_layout'] and m['_layout']['secondary_tileset'] in extras})
        subprocess.run(['git', 'checkout', 'HEAD', '--'] + xdirs + xlay, cwd=N.ROOT, check=True)
        head = subprocess.run(['git', 'show', 'HEAD:src/data/tilesets/headers.h'], cwd=N.ROOT, check=True,
                              capture_output=True, text=True, encoding='utf-8').stdout
        for sn in extras:
            pat = r'const struct Tileset %s\s*=\s*\{.*?\};' % re.escape(sn)
            text = re.sub(pat, lambda _: re.search(pat, head, re.S).group(0), text, count=1, flags=re.S)
    for sn in sec_names:
        ts = TS(sn)
        for i in range(16):
            if ts.pla(i):
                ts.write_pla(i, set())
        m = re.search(r'(const struct Tileset %s\s*=\s*\{)(.*?)(\};)' % re.escape(sn), text, re.S)
        body = re.sub(r'\n[ \t]*\.swapPalettes[ \t]*=[^\n]*,[ \t]*(?=\n)', '', m.group(2))
        text = text[:m.start(2)] + body + text[m.end(2):]
    open(N.rpath('src/data/tilesets/headers.h'), 'w', encoding='utf-8', newline='').write(text)
    import doorglass  # lit door copies in these tilesets are gone
    doorglass.write_header(set(sec_names) | set(extras), [])
    print(f'reset {len(sec_names)} tilesets, {len(layouts)} layouts')


if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument('region')
    ap.add_argument('--apply', action='store_true')
    ap.add_argument('--reset', action='store_true', help='undo the region\'s tileset lighting first')
    ap.add_argument('--verbose', action='store_true')
    a = ap.parse_args()
    VERBOSE = a.verbose
    if a.reset:
        reset(a.region)
        if not a.apply:
            raise SystemExit
    names, scans = analyze(a.region)
    if a.apply:
        apply_glass(names, scans)
    else:
        report(names, scans)
