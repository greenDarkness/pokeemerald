"""Light open doorways (dark openings with no door) on a region's outdoor maps.

    python doorways.py kanto            # report pixels per map
    python doorways.py kanto --apply

Each rule picks visible day pixels of one color in some rows of the metatiles around an anchor metatile, and
glass.apply_glass lights them (warm, shading kept), copying primary metatiles into the secondary tilesets.
"""
import argparse
from collections import defaultdict

import glass
from glass import N, MAPS, BYNAME, blocks

DOORWAY = (30, 26, 17)  # final night look of a lit doorway (5-bit), like Hoenn's lit doorways


def doorway_ramp(colors):
    return {c: glass.night_for(DOORWAY) for c in colors}


def floor_ramp(colors):
    """Floor/threshold just outside a lit doorway: a little dimmer than the doorway."""
    return {c: glass.night_for((28, 24, 15)) for c in colors}


def window_ramp(colors):
    """Pale glass: every shade on the warm glass ramp, keeping light/dark order (no white glints)."""
    cs = sorted(set(colors), key=glass.lum)
    out = {}
    for i, c in enumerate(cs):
        t = i / max(1, len(cs) - 1)
        out[c] = glass.night_for(tuple(round(a + (b - a) * t) for a, b in zip(glass.F_DARK, glass.F_LIGHT)))
    return out


def marquee_ramp(colors):
    """Game Corner marquee: pale yellow -> bright cream, gold -> amber."""
    out = {}
    for c in colors:
        out[c] = glass.night_for((31, 30, 22) if glass.lum(c) > 200 else (31, 24, 10))
    return out


# rule: (anchor metatile ids, owner tileset, [(dx, dy, y0, y1, x0, x1)] metatile offsets and pixel box,
#        day colors to light, ramp)
RULES = {
    'kanto': [
        # gatehouse doorways (KantoGeneral 0x190-0x192): the dark interior under the lintel and in the door
        ({0x190, 0x191, 0x192}, 'gTileset_KantoGeneral', [(0, -1, 13, 16, 0, 16), (0, 0, 0, 8, 0, 16)],
         {(90, 90, 115)}, doorway_ramp),
        # Cerulean bike shop windows above the awning (too pale for glass.py)
        ({0x28d}, 'gTileset_CeruleanCity', [(0, 0, 1, 5, 3, 16)],  # x 1-2: the wall's light edge
         {(206, 206, 222), (173, 222, 239), (255, 255, 255)}, window_ramp),
        ({0x28e, 0x28f}, 'gTileset_CeruleanCity', [(0, 0, 1, 5, 0, 16)],
         {(206, 206, 222), (173, 222, 239), (255, 255, 255)}, window_ramp),
        # Saffron Fighting Dojo: the open entrance
        ({0x333}, 'gTileset_SaffronCity', [(0, 0, 1, 14, 0, 16)], {(66, 74, 107)}, doorway_ramp),
        ({0x333}, 'gTileset_SaffronCity', [(0, 0, 14, 16, 0, 16)], {(123, 123, 132)}, floor_ramp),  # threshold
        # Bill's house (Route 25): the opening above the door
        ({0x2da}, 'gTileset_CeruleanCity', [(0, 0, 5, 9, 2, 14)], {(66, 74, 107)}, doorway_ramp),
        # Silph Co.'s glass dome (SaffronCity and SaffronCity_Connection; glass.py skips it as too big). The side
        # panes by the entrance (0x2bb, 0x2bd) share these colors, so they're included and the colors go palette-wide
        ({0x2c5, 0x2c6, 0x2c7, 0x2cd, 0x2ce, 0x2cf, 0x2d5, 0x2d6, 0x2d7, 0x2bb, 0x2bd}, 'gTileset_SaffronCity',
         [(0, 0, 0, 16, 0, 16)], {(173, 231, 255), (123, 206, 214), (222, 247, 255)}, window_ramp),
        # (Celadon Game Corner dots and fan: a light sprite, kanto_signs.game_corner; Celadon's palettes are full)
    ],
}


def collect(region):
    secs = N.mapsec_regions()
    N.assign_regions(MAPS, secs)
    water = glass.water_behaviors()
    names, scans = [], {}
    global ramps
    ramps = {}
    for m in sorted(MAPS.values(), key=lambda m: m['_name']):
        lay = m['_layout']
        if not lay or m.get('map_type') not in N.NATURAL_LIGHT_TYPES or N.region_of(m, secs) != region:
            continue
        b = blocks(m['_name'])
        cells = defaultdict(set)  # (mx, my) -> {(y0, y1, x0, x1, colors, ramp)}
        for anchors, owner, offs, colors, rfn in RULES[region]:
            if owner not in (lay['primary_tileset'], lay['secondary_tileset']):
                continue
            for my, row in enumerate(b):
                for mx, g in enumerate(row):
                    if g in anchors:
                        for dx, dy, y0, y1, x0, x1 in offs:
                            cells[(mx + dx, my + dy)].add((y0, y1, x0, x1, frozenset(colors), rfn))
        if not cells:
            continue
        s = glass.MapScan(m['_name'], water)
        g = set()
        for (mx, my), specs in cells.items():
            if not (0 <= my < s.h and 0 <= mx < s.w):
                continue
            for y0, y1, x0, x1, colors, rfn in specs:
                for y in range(y0, y1):
                    for x in range(x0, x1):
                        px, py = mx * 16 + x, my * 16 + y
                        if s.rgb[py][px] in colors:
                            g.add((px, py))
                            ramps[tuple(v >> 3 for v in s.rgb[py][px])] = rfn
        names.append(m['_name'])
        scans[m['_name']] = (s, g)
        print(f'{m["_name"]}: {len(g)} doorway px')
    return names, scans


if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument('region')
    ap.add_argument('--apply', action='store_true')
    ap.add_argument('--verbose', action='store_true')
    a = ap.parse_args()
    glass.VERBOSE = a.verbose
    names, scans = collect(a.region)
    if a.apply:
        def by_rule(colors):
            out = {}
            k5 = lambda c: tuple(v >> 3 for v in c)
            for rfn in set(ramps.values()):
                out.update(rfn([c for c in colors if ramps.get(k5(c)) is rfn]))
            return out
        glass.apply_glass(names, scans, doors_ok=True, palette_wide=True, ramp_fn=by_rule)
