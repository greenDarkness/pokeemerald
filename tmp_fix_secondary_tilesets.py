import json

base = '/mnt/c/Users/drfor/Documents/Porymap/Decomps/pokeemerald'
layouts_path = f'{base}/data/layouts/layouts.json'

with open(layouts_path, 'r', encoding='utf-8') as f:
    data = json.load(f)

# Load Johto map list
mg = json.load(open(f'{base}/data/maps/map_groups.json'))
johto = set(mg.get('gMapGroup_Johto', []))

# Build set of Johto layout IDs from map.json files
johto_layout_ids = set()
for m in johto:
    try:
        mj = json.load(open(f'{base}/data/maps/{m}/map.json'))
        johto_layout_ids.add(mj.get('layout', ''))
    except:
        pass

fixed = 0
for layout in data.get('layouts', []):
    if layout['id'] not in johto_layout_ids:
        continue
    if layout.get('secondary_tileset') == 'gTileset_General':
        layout['secondary_tileset'] = 'gTileset_Petalburg'
        fixed += 1

print(f'Fixed {fixed} secondary tilesets')

with open(layouts_path, 'w', encoding='utf-8', newline='\n') as f:
    json.dump(data, f, indent=2)
    f.write('\n')

print('Done.')
