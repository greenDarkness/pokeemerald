"""
Build johto_day_care secondary tileset for pokeemerald.
Same approach as mahogany_gym:
- tiles.png positions 0-127 = JohtoBuilding primary tiles 512-639 (rolled)
- tiles.png positions 128-511 = Pokesequel pokemon_day_care secondary tiles 0-383
- metatiles.bin = Pokesequel's pokemon_day_care metatiles 0-201 (202 metatiles)
"""
import struct, shutil, os
from PIL import Image

POKESEQUEL = 'C:/Users/drfor/Documents/Porymap/Decomps/pokesequel-master'
DST = 'data/tilesets/secondary/johto_day_care'

johto_prim = Image.open(POKESEQUEL + '/data/tilesets/primary/johto_building/tiles.png')
dc_sec     = Image.open(POKESEQUEL + '/data/tilesets/secondary/pokemon_day_care/tiles.png')

print('JohtoBuilding primary:', johto_prim.size, johto_prim.mode, '=', johto_prim.size[0]//8 * johto_prim.size[1]//8, 'tiles')
print('Pokesequel pokemon_day_care:', dc_sec.size, dc_sec.mode, '=', dc_sec.size[0]//8 * dc_sec.size[1]//8, 'tiles')

# Verify source metatile tile refs are within new PNG bounds
seq_meta = open(POKESEQUEL + '/data/tilesets/secondary/pokemon_day_care/metatiles.bin', 'rb').read()
num_seq_metatiles = len(seq_meta) // 16
all_refs = set()
for off in range(num_seq_metatiles):
    for w in struct.unpack_from('<8H', seq_meta, off*16):
        all_refs.add(w & 0x3FF)

max_prim_ref = max((r for r in all_refs if r < 512), default=-1)
max_sec_ref  = max((r for r in all_refs if r >= 512), default=-1)
max_sec_pos  = max_sec_ref - 512 if max_sec_ref >= 512 else -1
dc_sec_tiles = dc_sec.size[0]//8 * dc_sec.size[1]//8
new_png_tiles = 128 + dc_sec_tiles  # 128 rolled + all secondary tiles

print()
print('Metatile ref validation (Pokesequel):')
print('  Max primary ref:', max_prim_ref, '- OK' if max_prim_ref <= 511 else '- OUT OF RANGE')
print('  Max secondary ref (absolute):', max_sec_ref)
print('  Max secondary PNG position:', max_sec_pos, '  New PNG will have:', new_png_tiles, 'tiles (positions 0-%d)' % (new_png_tiles-1))
print('  Status:', 'OK' if max_sec_pos < new_png_tiles else 'OUT OF RANGE - need more tiles')

# Build output P-mode image
tile_rows = (new_png_tiles + 15) // 16
output = Image.new('P', (128, tile_rows * 8))
output.putpalette(dc_sec.getpalette())

def copy_tile_p(dst, src, dst_idx, src_idx):
    sw = src.size[0] // 8
    dw = dst.size[0] // 8
    sr, sc = divmod(src_idx, sw)
    dr, dc2 = divmod(dst_idx, dw)
    tile = src.crop((sc*8, sr*8, sc*8+8, sr*8+8))
    dst.paste(tile, (dc2*8, dr*8))

# Positions 0-127: JohtoBuilding primary tiles 512-639
for i in range(128):
    copy_tile_p(output, johto_prim, i, 512 + i)

# Positions 128+: Pokesequel pokemon_day_care secondary tiles
for i in range(dc_sec_tiles):
    copy_tile_p(output, dc_sec, 128 + i, i)

# Rebuild directory
if os.path.exists(DST):
    shutil.rmtree(DST)
os.makedirs(DST + '/palettes', exist_ok=True)

output.save(DST + '/tiles.png')
print()
print('Saved tiles.png:', output.size)

# Verify pixel range
result = Image.open(DST + '/tiles.png')
px = list(result.getdata())
print('Pixel range:', min(px), '-', max(px), '  unique:', len(set(px)))

# metatiles.bin: first 202 metatiles from Pokesequel (offsets 0-201)
with open(DST + '/metatiles.bin', 'wb') as f:
    f.write(seq_meta[:202 * 16])
print('Saved metatiles.bin: 202 metatiles')

# metatile_attributes.bin: first 202
seq_attr = open(POKESEQUEL + '/data/tilesets/secondary/pokemon_day_care/metatile_attributes.bin', 'rb').read()
with open(DST + '/metatile_attributes.bin', 'wb') as f:
    f.write(seq_attr[:202 * 2])
print('Saved metatile_attributes.bin: 202 entries')

# Palettes from Pokesequel's pokemon_day_care
seq_pal_dir = POKESEQUEL + '/data/tilesets/secondary/pokemon_day_care/palettes'
for fname in os.listdir(seq_pal_dir):
    shutil.copy2(seq_pal_dir + '/' + fname, DST + '/palettes/' + fname)
print('Copied palettes:', sorted(os.listdir(DST + '/palettes')))
