#!/usr/bin/env python3
"""Generates src/data/shoreline_spots.h: where each map's wading shoreline is.

Run by make (json_data_rules.mk) whenever a layout, map or tileset changes.

Every map whose layout has walkable shallow water (MB_SHALLOW_WATER) gets one entry per
patch of it. Nearby cells are grouped into one patch; a patch's position is the shallow
cell closest to the patch's middle, stored as a fraction of the map size (0-255 per axis)
so the Pokédex area screen can place a marker inside the map's region map section.
Which Pokémon wade there is decided at runtime from src/data/shorelines.h.

Usage: gen_shoreline_spots.py <output header>
"""
import json
import os
import re
import struct
import sys

MB_SHALLOW_WATER = 0x17
NUM_METATILES_IN_PRIMARY = 512
METATILE_ID_MASK = 0x3FF
LINK_DISTANCE = 4      # cells this close (in both axes) are part of the same patch
MERGE_DISTANCE = 12    # patches whose middles are this close are merged
MAX_SPOTS_PER_MAP = 4  # biggest patches are kept

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))


def read(path, mode='r'):
    with open(os.path.join(ROOT, path), mode) as f:
        return f.read()


def load_tileset_attributes():
    headers = read('src/data/tilesets/headers.h')
    tileset_attrs = {}
    for m in re.finditer(r'const struct Tileset (gTileset_\w+) =\s*\{(.*?)\};', headers, re.S):
        a = re.search(r'\.metatileAttributes\s*=\s*(\w+)', m.group(2))
        if a:
            tileset_attrs[m.group(1)] = a.group(1)
    attr_files = {}
    for path in ('src/data/tilesets/metatiles.h', 'src/data/tilesets/graphics.h'):
        if os.path.exists(os.path.join(ROOT, path)):
            for m in re.finditer(r'(gMetatileAttributes_\w+)\[\]\s*=\s*INCBIN_U16\("([^"]+)"\)', read(path)):
                attr_files[m.group(1)] = m.group(2)
    cache = {}

    def attributes(tileset):
        if tileset not in cache:
            path = attr_files.get(tileset_attrs.get(tileset))
            if path is None:
                cache[tileset] = ()
            else:
                data = read(path, 'rb')
                cache[tileset] = struct.unpack('<%dH' % (len(data) // 2), data)
        return cache[tileset]

    return attributes


def shallow_cells(layout, attributes):
    primary = attributes(layout['primary_tileset'])
    secondary = attributes(layout['secondary_tileset'])
    data = read(layout['blockdata_filepath'], 'rb')
    width = layout['width']
    cells = []
    for i, block in enumerate(struct.unpack('<%dH' % (len(data) // 2), data)):
        metatile = block & METATILE_ID_MASK
        if metatile < NUM_METATILES_IN_PRIMARY:
            attr = primary[metatile] if metatile < len(primary) else 0
        else:
            metatile -= NUM_METATILES_IN_PRIMARY
            attr = secondary[metatile] if metatile < len(secondary) else 0
        if (attr & 0xFF) == MB_SHALLOW_WATER:
            cells.append((i % width, i // width))
    return cells


def middle(cells):
    return (sum(x for x, _ in cells) / len(cells), sum(y for _, y in cells) / len(cells))


def patches(cells):
    parent = list(range(len(cells)))

    def find(i):
        while parent[i] != i:
            parent[i] = parent[parent[i]]
            i = parent[i]
        return i

    def join(a, b):
        parent[find(a)] = find(b)

    by_pos = {c: i for i, c in enumerate(cells)}
    for i, (x, y) in enumerate(cells):
        for dx in range(-LINK_DISTANCE, LINK_DISTANCE + 1):
            for dy in range(-LINK_DISTANCE, LINK_DISTANCE + 1):
                j = by_pos.get((x + dx, y + dy))
                if j is not None:
                    join(i, j)
    groups = {}
    for i, c in enumerate(cells):
        groups.setdefault(find(i), []).append(c)
    groups = list(groups.values())

    merged = True
    while merged:
        merged = False
        for a in range(len(groups)):
            for b in range(a + 1, len(groups)):
                (ax, ay), (bx, by) = middle(groups[a]), middle(groups[b])
                if abs(ax - bx) <= MERGE_DISTANCE and abs(ay - by) <= MERGE_DISTANCE:
                    groups[a] += groups.pop(b)
                    merged = True
                    break
            if merged:
                break
    groups.sort(key=len, reverse=True)
    return groups[:MAX_SPOTS_PER_MAP]


def main():
    out_path = sys.argv[1] if len(sys.argv) > 1 else os.path.join(ROOT, 'src/data/shoreline_spots.h')
    layouts = {l['id']: l for l in json.loads(read('data/layouts/layouts.json'))['layouts'] if 'id' in l}
    group_order = json.loads(read('data/maps/map_groups.json'))['group_order']
    groups = json.loads(read('data/maps/map_groups.json'))
    attributes = load_tileset_attributes()

    lines = []
    for group in group_order:
        for map_name in groups[group]:
            map_json = json.loads(read(os.path.join('data/maps', map_name, 'map.json')))
            layout = layouts.get(map_json['layout'])
            if layout is None or 'blockdata_filepath' not in layout:
                continue
            cells = shallow_cells(layout, attributes)
            if not cells:
                continue
            for patch in patches(cells):
                mx, my = middle(patch)
                x, y = min(patch, key=lambda c: (c[0] - mx) ** 2 + (c[1] - my) ** 2)
                fx = min(255, int((x + 0.5) * 256 / layout['width']))
                fy = min(255, int((y + 0.5) * 256 / layout['height']))
                lines.append('    { .map = %s, .x = %d, .y = %d }, // %d cells' % (map_json['id'], fx, fy, len(patch)))

    text = ('// Generated by tech/gen_shoreline_spots.py from the map layouts. Do not edit.\n'
            '// Patches of walkable shallow water (MB_SHALLOW_WATER); x/y = position in the map, 0-255 per axis.\n\n'
            'static const struct ShorelineSpot sShorelineSpots[] = {\n'
            + ''.join(l + '\n' for l in lines) +
            '    { .map = MAP_UNDEFINED }, // end of list\n'
            '};\n')
    with open(out_path, 'w', newline='\n') as f:
        f.write(text)


if __name__ == '__main__':
    main()
