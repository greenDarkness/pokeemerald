"""
Build mahogany_gym secondary tileset for pokeemerald.

Key insight:
- JohtoBuilding primary first 512 tiles are identical between pokeemerald and Pokesequel.
- Pokesequel's JohtoBuilding primary tiles 512-639 need to go at mahogany_gym secondary
  positions 0-127 (absolute 512-639) - NO absolute index change.
- Pokesequel's sootopolis_gym secondary tiles 0-383 go at mahogany_gym secondary
  positions 128-511 (absolute 640-1023) - NO absolute index change.
- So Pokesequel's sootopolis_gym metatiles.bin (offsets 0-125) is VALID as-is
  (all tile refs reference the correct absolute positions).
"""
from PIL import Image
import struct, shutil, os

POKEEMERALD = 'C:/Users/drfor/Documents/Porymap/Decomps/pokeemerald'
POKESEQUEL  = 'C:/Users/drfor/Documents/Porymap/Decomps/pokesequel-master'
DST = POKEEMERALD + '/data/tilesets/secondary/mahogany_gym'

# Open source images in RGBA for pixel manipulation
johto_prim_img = Image.open(POKESEQUEL + '/data/tilesets/primary/johto_building/tiles.png').convert('RGBA')
soot_sec_img   = Image.open(POKESEQUEL + '/data/tilesets/secondary/sootopolis_gym/tiles.png').convert('RGBA')

print('JohtoBuilding primary:', johto_prim_img.size, '= %d tiles' % (johto_prim_img.size[0]//8 * johto_prim_img.size[1]//8))
print('Sootopolis secondary:', soot_sec_img.size,   '= %d tiles' % (soot_sec_img.size[0]//8 * soot_sec_img.size[1]//8))

# Build 128x256 output (512 tiles = 32 rows)
output = Image.new('RGBA', (128, 256), (0, 0, 0, 255))

def paste_tile(dst_img, src_img, dst_idx, src_idx):
    """Copy tile at src_idx in src_img to dst_idx in dst_img."""
    sw = src_img.size[0] // 8
    dw = dst_img.size[0] // 8
    sr, sc = divmod(src_idx, sw)
    dr, dc = divmod(dst_idx, dw)
    tile = src_img.crop((sc*8, sr*8, sc*8+8, sr*8+8))
    dst_img.paste(tile, (dc*8, dr*8))

# Positions 0-127 (absolute 512-639): JohtoBuilding primary tiles 512-639
for i in range(128):
    paste_tile(output, johto_prim_img, i, 512 + i)

# Positions 128-511 (absolute 640-1023): Pokesequel sootopolis secondary tiles 0-383
for i in range(384):
    paste_tile(output, soot_sec_img, 128 + i, i)

# Rebuild the mahogany_gym directory
if os.path.exists(DST):
    shutil.rmtree(DST)
os.makedirs(DST + '/palettes', exist_ok=True)

# Save tiles.png
output.save(DST + '/tiles.png')
print('Saved tiles.png (128x256, 512 tiles)')

# metatiles.bin: Pokesequel sootopolis offsets 0-125 (126 metatiles)
seq_meta = open(POKESEQUEL + '/data/tilesets/secondary/sootopolis_gym/metatiles.bin', 'rb').read()
with open(DST + '/metatiles.bin', 'wb') as f:
    f.write(seq_meta[:126 * 16])
print('Saved metatiles.bin (%d metatiles)' % (126))

# metatile_attributes.bin: Pokesequel sootopolis offsets 0-125
seq_attr = open(POKESEQUEL + '/data/tilesets/secondary/sootopolis_gym/metatile_attributes.bin', 'rb').read()
with open(DST + '/metatile_attributes.bin', 'wb') as f:
    f.write(seq_attr[:126 * 2])
print('Saved metatile_attributes.bin (%d entries)' % 126)

# Palettes: copy from Pokesequel's sootopolis_gym
seq_pal_dir = POKESEQUEL + '/data/tilesets/secondary/sootopolis_gym/palettes'
for fname in os.listdir(seq_pal_dir):
    shutil.copy2(seq_pal_dir + '/' + fname, DST + '/palettes/' + fname)
print('Copied palettes:', sorted(os.listdir(DST + '/palettes')))

# Verify tile refs in metatiles.bin are within range
meta = open(DST + '/metatiles.bin', 'rb').read()
all_refs = set()
for off in range(126):
    for w in struct.unpack_from('<8H', meta, off*16):
        all_refs.add(w & 0x3FF)

max_primary_ref = max((r for r in all_refs if r < 512), default=-1)
max_secondary_ref = max((r for r in all_refs if r >= 512), default=-1)
max_png_position = max_secondary_ref - 512 if max_secondary_ref >= 512 else -1
print()
print('Tile ref validation:')
print('  Max primary ref: %d (limit 511)' % max_primary_ref, '- OK' if max_primary_ref <= 511 else '- OUT OF RANGE!')
print('  Max secondary ref: %d (absolute)' % max_secondary_ref)
print('  Max secondary PNG position: %d (PNG has 512 tiles, limit 511)' % max_png_position,
      '- OK' if max_png_position <= 511 else '- OUT OF RANGE!')
print()
print('Done! mahogany_gym directory:')
for root, dirs, files in os.walk(DST):
    for f in files:
        fp = os.path.join(root, f)
        print(' ', os.path.relpath(fp, DST), os.path.getsize(fp), 'bytes')
