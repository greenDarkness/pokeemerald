import json, os

base = '/mnt/c/Users/drfor/Documents/Porymap/Decomps/pokeemerald'
mg = json.load(open(f'{base}/data/maps/map_groups.json'))
johto = mg.get('gMapGroup_Johto', [])
layouts_data = json.load(open(f'{base}/data/layouts/layouts.json'))
layout_map = {l['id']: l for l in layouts_data.get('layouts', [])}

errors = []

for m in johto:
    map_path = f'{base}/data/maps/{m}/map.json'
    # JSON validity
    try:
        d = json.load(open(map_path))
    except Exception as e:
        errors.append(f'JSON error in {m}: {e}')
        continue

    # Layout exists
    lid = d.get('layout', '')
    if lid not in layout_map:
        errors.append(f'{m}: layout {lid} not in layouts.json')
        continue

    l = layout_map[lid]
    # Bin file sizes
    for key in ('border_filepath', 'blockdata_filepath'):
        fp = f'{base}/{l[key]}'
        if not os.path.exists(fp):
            errors.append(f'{m}: missing {key}: {fp}')
        elif os.path.getsize(fp) == 0:
            errors.append(f'{m}: EMPTY {key}: {fp}')

if errors:
    print(f'{len(errors)} errors found:')
    for e in errors[:20]:
        print(' ', e)
else:
    print('No errors found in 145 Johto maps')

# Also check for duplicate layout IDs
layout_ids = [l['id'] for l in layouts_data.get('layouts', [])]
from collections import Counter
dupes = [lid for lid, cnt in Counter(layout_ids).items() if cnt > 1]
print(f'Duplicate layout IDs: {len(dupes)}', dupes[:5] if dupes else '')
