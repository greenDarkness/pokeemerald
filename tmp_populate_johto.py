#!/usr/bin/env python3
"""
Populate existing Johto map folders in pokeemerald to match pokesequel.
Copy map.json (stripped), create scripts.inc stubs, import layouts.
"""

import os, json, shutil, re

EMERALD  = '/mnt/c/Users/drfor/Documents/Porymap/Decomps/pokeemerald'
SEQUEL   = '/mnt/c/Users/drfor/Documents/Porymap/Decomps/pokesequel-master'

def write_utf8(path, text):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'w', encoding='utf-8', newline='\n') as f:
        f.write(text)

def load_json(path):
    with open(path, 'r', encoding='utf-8-sig') as f:
        return json.load(f)

# Get all Johto maps from pokesequel
mg_sequel = json.load(open(os.path.join(SEQUEL, 'data/maps/map_groups.json')))

johto_maps = set()

# All Johto indoor groups
for group in ['gMapGroup_IndoorNewBark', 'gMapGroup_IndoorCherrygrove', 'gMapGroup_IndoorViolet',
              'gMapGroup_IndoorAzalea', 'gMapGroup_IndoorGoldenrod', 'gMapGroup_IndoorEcruteak',
              'gMapGroup_IndoorOlivine', 'gMapGroup_IndoorCianwood', 'gMapGroup_IndoorMahogany',
              'gMapGroup_IndoorBlackthorn']:
    if group in mg_sequel:
        johto_maps.update(mg_sequel[group])

# Johto routes
if 'gMapGroup_IndoorJohtoRoutes' in mg_sequel:
    johto_maps.update(mg_sequel['gMapGroup_IndoorJohtoRoutes'])

# Johto outdoor
johto_outdoor_set = {
    'NewBarkTown', 'CherrygroveCity', 'VioletCity', 'AzaleaTown',
    'GoldenrodCity', 'EcruteakCity', 'OlivineCity', 'CianwoodCity',
    'SafariZoneGate', 'Mahoganytown', 'BlackthornCity',
    'Route29', 'Route30', 'Route31', 'Route32', 'Route33',
    'Route34', 'Route35', 'Route36', 'Route37', 'Route38',
    'Route39', 'Route40', 'Route41', 'Route42', 'Route43',
    'Route44', 'Route45', 'Route46', 'Route47', 'Route48'
}
for name in mg_sequel.get('gMapGroup_TownsAndRoutes', []):
    if name in johto_outdoor_set:
        johto_maps.add(name)

print(f"Processing {len(johto_maps)} Johto maps\n")

# Valid tilesets
headers_path = os.path.join(EMERALD, 'src', 'data', 'tilesets', 'headers.h')
with open(headers_path, encoding='utf-8') as f:
    valid_tilesets = set(re.findall(r'const struct Tileset (\w+)', f.read()))

# Load layouts
emerald_layouts_obj = load_json(os.path.join(EMERALD, 'data', 'layouts', 'layouts.json'))
sequel_layouts_obj  = load_json(os.path.join(SEQUEL,  'data', 'layouts', 'layouts.json'))

emerald_layout_ids = {l['id'] for l in emerald_layouts_obj['layouts']}
sequel_layouts_by_id = {l['id']: l for l in sequel_layouts_obj['layouts']}

# Process each Johto map
populated_maps = []
pending_layouts = []

for map_name in sorted(johto_maps):
    src_map = os.path.join(SEQUEL, 'data', 'maps', map_name, 'map.json')
    dst_dir = os.path.join(EMERALD, 'data', 'maps', map_name)
    
    if not os.path.exists(src_map):
        print(f"  SKIP {map_name}: not in pokesequel")
        continue
    
    if not os.path.isdir(dst_dir):
        print(f"  SKIP {map_name}: folder missing in pokeemerald")
        continue
    
    # Load from pokesequel and strip events/connections
    map_obj = load_json(src_map)
    map_obj['object_events'] = []
    map_obj['warp_events']   = []
    map_obj['coord_events']  = []
    map_obj['bg_events']     = []
    map_obj['connections']   = None
    
    write_utf8(os.path.join(dst_dir, 'map.json'), json.dumps(map_obj, indent=2))
    write_utf8(os.path.join(dst_dir, 'scripts.inc'),
               f"{map_name}_MapScripts::\n\t.byte 0\n")
    
    layout_id = map_obj.get('layout')
    if layout_id and layout_id not in emerald_layout_ids and layout_id not in pending_layouts:
        pending_layouts.append(layout_id)
    
    populated_maps.append(map_name)
    print(f"  {map_name}")

print(f"\nPopulated {len(populated_maps)} maps")
print(f"Layouts to import: {len(pending_layouts)}\n")

# Import layouts
added_layouts = []
for layout_id in pending_layouts:
    if layout_id not in sequel_layouts_by_id:
        print(f"  SKIP {layout_id}: not in pokesequel")
        continue
    
    layout = dict(sequel_layouts_by_id[layout_id])
    
    if layout.get('primary_tileset') not in valid_tilesets:
        layout['primary_tileset'] = 'gTileset_General'
    if layout.get('secondary_tileset') not in valid_tilesets:
        layout['secondary_tileset'] = 'gTileset_General'
    
    for key in ('border_filepath', 'blockdata_filepath'):
        rel = layout.get(key)
        if not rel:
            continue
        src_bin = os.path.join(SEQUEL, rel)
        dst_bin = os.path.join(EMERALD, rel)
        os.makedirs(os.path.dirname(dst_bin), exist_ok=True)
        if os.path.exists(src_bin):
            shutil.copy2(src_bin, dst_bin)
    
    added_layouts.append(layout)
    emerald_layout_ids.add(layout_id)
    print(f"  {layout_id}")

if added_layouts:
    with open(os.path.join(EMERALD, 'data', 'layouts', 'layouts.json'), 'r', encoding='utf-8') as f:
        raw = f.read()
    snippets = ',\n'.join(json.dumps(l, indent=2) for l in added_layouts)
    last_bracket = raw.rfind(']')
    updated = raw[:last_bracket] + ',\n' + snippets + '\n' + raw[last_bracket:]
    write_utf8(os.path.join(EMERALD, 'data', 'layouts', 'layouts.json'), updated)
    print(f"\nlayouts.json: +{len(added_layouts)} layouts")

# Update map_groups.json
mg_path = os.path.join(EMERALD, 'data', 'maps', 'map_groups.json')
mg_obj = load_json(mg_path)

group_name = 'gMapGroup_Johto'
if group_name in mg_obj:
    existing = list(mg_obj[group_name])
    for m in populated_maps:
        if m not in existing:
            existing.append(m)
    mg_obj[group_name] = existing
    print(f"Updated {group_name}: {len(existing)} maps")
else:
    mg_obj[group_name] = populated_maps
    mg_obj['group_order'].append(group_name)
    print(f"Created {group_name}: {len(populated_maps)} maps")

write_utf8(mg_path, json.dumps(mg_obj, indent=2))

# Append to event_scripts.s
es_path = os.path.join(EMERALD, 'data', 'event_scripts.s')
lines = '\n'.join(f'        .include "data/maps/{m}/scripts.inc"' for m in populated_maps)
with open(es_path, 'a', encoding='utf-8', newline='\n') as f:
    f.write('\n' + lines + '\n')

print(f"event_scripts.s: +{len(populated_maps)} includes")
print(f"\nDONE: Populated {len(populated_maps)} Johto maps with {len(added_layouts)} layouts.")
