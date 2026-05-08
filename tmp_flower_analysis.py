import json, struct
from PIL import Image

POKESEQUEL = 'C:/Users/drfor/Documents/Porymap/Decomps/pokesequel-master'
layouts = json.load(open('data/layouts/layouts.json'))
seq_layouts = json.load(open(POKESEQUEL + '/data/layouts/layouts.json'))

ids = ['LAYOUT_ROUTE117_POKEMON_DAY_CARE', 'LAYOUT_GOLDENROD_CITY_FLOWER_SHOP', 'LAYOUT_ROUTE34_DAY_CARE']

print('=== pokeemerald ===')
for l in layouts['layouts']:
    if l['id'] in ids:
        print(l['id'])
        print('  primary:', l.get('primary_tileset'))
        print('  secondary:', l.get('secondary_tileset'))

print()
print('=== Pokesequel ===')
for l in seq_layouts['layouts']:
    if l['id'] in ids:
        print(l['id'])
        print('  primary:', l.get('primary_tileset'))
        print('  secondary:', l.get('secondary_tileset'))

# Check tile refs in pokeemerald's current pokemon_day_care metatiles.bin
print()
meta = open('data/tilesets/secondary/pokemon_day_care/metatiles.bin', 'rb').read()
refs = set()
for off in range(len(meta)//16):
    for w in struct.unpack_from('<8H', meta, off*16):
        refs.add(w & 0x3FF)
print('pokeemerald pokemon_day_care tile refs: max=%d, range %d-%d' % (max(refs), min(refs), max(refs)))
sec_refs = sorted(r for r in refs if r >= 512)
print('  secondary (>=512):', sec_refs)

# Check all 3 map.bin after -0x80 patch simulation
print()
for l in layouts['layouts']:
    if l['id'] in ids:
        path = l.get('blockdata_filepath')
        if not path:
            continue
        import os
        if not os.path.exists(path):
            print(l['id'], '- map.bin missing')
            continue
        data = open(path, 'rb').read()
        vals = [v & 0x3FF for v in struct.unpack_from('<%dH' % (len(data)//2), data)]
        sec = [v for v in vals if v >= 0x200]
        high = [v for v in sec if v >= 0x280]
        print(l['id'])
        if high:
            patched_offsets = sorted(set(v - 0x280 for v in high))
            print('  Needs patch: %d refs, after patch offsets 0+0x80=%d .. max=%d' % (
                len(high), min(patched_offsets), max(patched_offsets)))
        else:
            already = sorted(set(v - 0x200 for v in sec)) if sec else []
            print('  Already patched: offsets %s .. %s' % (min(already) if already else '-', max(already) if already else '-'))
