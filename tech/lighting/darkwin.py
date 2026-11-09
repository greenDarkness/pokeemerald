"""Dark window detection: solid rectangles of one dark color (unlit attic windows, eave vents, dark panes) that
glass.py's blue-glass detector misses.

    python darkwin.py johto            # report and comb sheets (out/dark_<map>.png)
    python darkwin.py johto --apply    # light them (warm, like doorways)
"""
import argparse
import os
from collections import defaultdict

import glass
from glass import N, MAPS, BYNAME, Image, OUT

MAX_LUM = 100
MIN_W, MIN_H, MAX_W, MAX_H = 4, 3, 40, 24

# Combing results: only rectangles whose metatiles are all listed here are lit ({tileset: {gid}})
INCLUDE = {
    'gTileset_CherrygroveCity': {0x2e2, 0x2e3, 0x2e4, 0x301, 0x302, 0x303},  # attic vents under the eaves
    'gTileset_NewBarkTown': {0x292, 0x2e2},  # attic vents
    'gTileset_SafariZoneJohto': {0x2c7},  # Safari Zone building windows
    'gTileset_JohtoNorthEast': {0x188, 0x189, 0x18a, 0x190, 0x191, 0x192},  # gatehouse open doorways
    'gTileset_JohtoGeneral': {0x188, 0x189, 0x18a, 0x190, 0x191, 0x192},
    'gTileset_Route38Farmland': {0x3b4},  # Moomoo Farm barn doorway
}


def dark_rects(s):
    """[(x0, y0, x1, y1)] solid single-color dark rectangles (inclusive) in a MapScan."""
    W, H = s.w * 16, s.h * 16
    seen = [[False] * W for _ in range(H)]
    out = []
    for y in range(H):
        row = s.rgb[y]
        for x in range(W):
            c = row[x]
            if seen[y][x] or c is None or glass.lum(c) >= MAX_LUM:
                continue
            stack, comp = [(x, y)], []
            seen[y][x] = True
            while stack:
                px, py = stack.pop()
                comp.append((px, py))
                for nx, ny in ((px + 1, py), (px - 1, py), (px, py + 1), (px, py - 1)):
                    if 0 <= nx < W and 0 <= ny < H and not seen[ny][nx] and s.rgb[ny][nx] == c:
                        seen[ny][nx] = True
                        stack.append((nx, ny))
            xs, ys = [p[0] for p in comp], [p[1] for p in comp]
            x0, x1, y0, y1 = min(xs), max(xs), min(ys), max(ys)
            w, h = x1 - x0 + 1, y1 - y0 + 1
            if len(comp) == w * h and MIN_W <= w <= MAX_W and MIN_H <= h <= MAX_H:
                out.append((x0, y0, x1, y1))
    return out


def collect(region, maps=None):
    secs = N.mapsec_regions()
    N.assign_regions(MAPS, secs)
    owned = glass.owned_secondaries(region)
    water = glass.water_behaviors()
    names, scans = [], {}
    for m in sorted(MAPS.values(), key=lambda m: m['_name']):
        lay = m['_layout']
        if (not lay or m.get('map_type') not in N.NATURAL_LIGHT_TYPES or N.region_of(m, secs) != region
                or lay['secondary_tileset'] not in owned or (maps and m['_name'] not in maps)):
            continue
        s = glass.MapScan(m['_name'], water)
        g = set()
        for x0, y0, x1, y1 in dark_rects(s):
            cells = {(x // 16, y // 16) for x in range(x0, x1 + 1) for y in range(y0, y1 + 1)}
            owner = lambda g: s.prim.name if g < 512 else s.sec.name
            if not all(s.b[cy][cx] in INCLUDE.get(owner(s.b[cy][cx]), ()) for cx, cy in cells):
                continue
            g |= {(x, y) for y in range(y0, y1 + 1) for x in range(x0, x1 + 1)}
        if g:
            names.append(m['_name'])
            scans[m['_name']] = (s, g)
            print(f'{m["_name"]}: {len(g)} dark window px')
    return names, scans


def doorway_ramp(colors):
    import doorways
    return doorways.doorway_ramp(colors)


if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument('region')
    ap.add_argument('--apply', action='store_true')
    ap.add_argument('--maps')
    a = ap.parse_args()
    names, scans = collect(a.region, set(a.maps.split(',')) if a.maps else None)
    if a.apply:
        glass.apply_glass(names, scans, doors_ok=True, palette_wide=False, ramp_fn=doorway_ramp)
    else:
        import comb_sheet
        for n in names:
            comb_sheet.sheet('dark_' + n, *scans[n])
