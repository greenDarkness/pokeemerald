import json, os

base = '/mnt/c/Users/drfor/Documents/Porymap/Decomps/pokeemerald'
layouts_data = json.load(open(f'{base}/data/layouts/layouts.json'))
layout_ids = set(l['id'] for l in layouts_data.get('layouts', []))

mg = json.load(open(f'{base}/data/maps/map_groups.json'))
johto = mg.get('gMapGroup_Johto', [])
print(f'Total Johto maps: {len(johto)}')

# Check missing layouts
missing_layouts = []
for m in johto:
    map_json = json.load(open(f'{base}/data/maps/{m}/map.json'))
    lid = map_json.get('layout', '')
    if lid not in layout_ids:
        missing_layouts.append((m, lid))
print(f'Maps with missing layout in layouts.json: {len(missing_layouts)}')
if missing_layouts:
    print('First 5:', missing_layouts[:5])

# Check bin files for Johto layouts
missing_bins = []
for l in layouts_data.get('layouts', []):
    # Find Johto layouts by checking if their name appears in gMapGroup_Johto folder names
    lid = l.get('id', '')
    border = l.get('border_filepath', '')
    # Use the border path to guess if it's a Johto layout
    folder = border.split('data/layouts/')[-1].split('/border.bin')[0] if 'data/layouts/' in border else ''
    if folder in johto:
        be = os.path.exists(f'{base}/{border}')
        bde = os.path.exists(f'{base}/{l.get("blockdata_filepath", "")}')
        if not be or not bde:
            missing_bins.append((lid, border, be, l.get('blockdata_filepath',''), bde))

print(f'Johto layouts with missing bin files: {len(missing_bins)}')
if missing_bins:
    print('First 5:', missing_bins[:5])

# Check a specific layout from map_groups.h
print('\nSample map_groups.h check:')
with open(f'{base}/include/constants/map_groups.h') as f:
    mgh = f.read()
print('MAP_AZALEA_TOWN in map_groups.h:', 'MAP_AZALEA_TOWN' in mgh)
print('MAP_NEW_BARK_TOWN in map_groups.h:', 'MAP_NEW_BARK_TOWN' in mgh)
print('gMapGroup_Johto comment in map_groups.h:', 'gMapGroup_Johto' in mgh)
