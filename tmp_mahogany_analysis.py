from PIL import Image
import struct, os

# Check if all Johto secondaries share the same first 128 tiles (JohtoBuilding rolled tiles)
aza = Image.open('C:/Users/drfor/Documents/Porymap/Decomps/pokeemerald/data/tilesets/secondary/azalea_town_gym/tiles.png').convert('RGBA')
cia = Image.open('C:/Users/drfor/Documents/Porymap/Decomps/pokeemerald/data/tilesets/secondary/cianwood_gym/tiles.png').convert('RGBA')
eco = Image.open('C:/Users/drfor/Documents/Porymap/Decomps/pokeemerald/data/tilesets/secondary/ecruteak_gym/tiles.png').convert('RGBA')

def get_tile(img, idx):
    r, c = divmod(idx, 16)
    return img.crop((c*8, r*8, c*8+8, r*8+8)).tobytes()

mm_ac = sum(1 for i in range(128) if get_tile(aza,i) != get_tile(cia,i))
mm_ae = sum(1 for i in range(128) if get_tile(aza,i) != get_tile(eco,i))
print('First 128 tile mismatches azalea vs cianwood:', mm_ac)
print('First 128 tile mismatches azalea vs ecruteak:', mm_ae)
print()

# Catalog all tile refs used by Mahogany Gym metatile offsets 0-125
seq_meta = open('C:/Users/drfor/Documents/Porymap/Decomps/pokesequel-master/data/tilesets/secondary/sootopolis_gym/metatiles.bin', 'rb').read()
all_refs = set()
for off in range(126):
    words = struct.unpack_from('<8H', seq_meta[off*16:(off+1)*16])
    for w in words:
        all_refs.add(w & 0x3FF)

prim = sorted([r for r in all_refs if r < 640])
sec  = sorted([r for r in all_refs if r >= 640])
rolled = sorted([r for r in prim if r >= 512])
print('Primary refs count:', len(prim), ' min:', hex(min(prim)), ' max:', hex(max(prim)))
print('Refs 512-639 (rolled into johto secondary pos 0-127):', rolled)
print('Secondary refs (>=640) count:', len(sec), ' min:', hex(min(sec)), ' max:', hex(max(sec)))
print()

# Can we find all primary refs (0-511) in pokeemerald's JohtoBuilding primary?
johto_prim = Image.open('C:/Users/drfor/Documents/Porymap/Decomps/pokeemerald/data/tilesets/primary/johto_building/tiles.png').convert('RGBA')
print('JohtoBuilding primary tiles.png size:', johto_prim.size, '=', johto_prim.size[0]//8 * johto_prim.size[1]//8, 'tiles')

poke_seq_img = Image.open('C:/Users/drfor/Documents/Porymap/Decomps/pokesequel-master/data/tilesets/secondary/sootopolis_gym/tiles.png').convert('RGBA')
seq_johto_prim = Image.open('C:/Users/drfor/Documents/Porymap/Decomps/pokesequel-master/data/tilesets/primary/johto_building/tiles.png').convert('RGBA')
print('Pokesequel JohtoBuilding primary size:', seq_johto_prim.size, '=', seq_johto_prim.size[0]//8 * seq_johto_prim.size[1]//8, 'tiles')
print()

# Build hash lookup for pokeemerald JohtoBuilding primary (absolute 0-511)
poke_prim_hash = {}
for i in range(512):
    tb = get_tile(johto_prim, i)
    if tb not in poke_prim_hash:
        poke_prim_hash[tb] = i  # absolute = i

# Build hash lookup for pokeemerald azalea secondary positions 0-127 (absolute 512-639)
# These ARE the JohtoBuilding rolled tiles
aza_rolled_hash = {}
for i in range(128):
    tb = get_tile(aza, i)
    if tb not in aza_rolled_hash:
        aza_rolled_hash[tb] = 512 + i  # absolute

# Build hash lookup for pokeemerald's other Johto secondaries (to find secondary tiles 640+)
# Try multiple secondaries to maximize coverage
poke_sec_hash = {}
for name, start_abs in [
    ('azalea_town_gym', 512), ('cianwood_gym', 512), ('ecruteak_gym', 512),
    ('blackthorn_gym', 512), ('ecruteak_theater', 512), ('goldenrod_station', 512)]:
    p = 'C:/Users/drfor/Documents/Porymap/Decomps/pokeemerald/data/tilesets/secondary/' + name
    if not os.path.isdir(p): continue
    img = Image.open(p + '/tiles.png').convert('RGBA')
    n = img.size[0]//8 * img.size[1]//8
    for i in range(128, n):  # skip first 128 (rolled tiles), they're primary-specific
        tb = get_tile(img, i)
        abs_idx = start_abs + i
        if tb not in poke_sec_hash:
            poke_sec_hash[tb] = abs_idx

print('Pokeemerald secondary tile pool size:', len(poke_sec_hash))

# Now try to remap Pokesequel refs to pokeemerald
# Pokesequel primary: tile i is just at absolute i
# Pokesequel secondary: tile at absolute 640+j = PNG position j in sootopolis_gym
missed = []
for ref in prim:
    # Primary tile in 0-511
    if ref < 512:
        tb = get_tile(seq_johto_prim, ref)  # Pokesequel JohtoBuilding primary tile ref
        if tb not in poke_prim_hash:
            missed.append(('primary', hex(ref)))
    else:
        # 512-639: rolled into secondary pos 0-127
        tb = get_tile(seq_johto_prim, ref)  # still primary in Pokesequel
        if tb not in aza_rolled_hash:
            missed.append(('rolled', hex(ref)))

for ref in sec:
    # Secondary tile: Pokesequel abs 640+ = sootopolis_gym PNG position ref-640
    tb = get_tile(poke_seq_img, ref - 640)
    if tb not in poke_sec_hash:
        missed.append(('secondary', hex(ref)))

print('Unmapped tile refs:', len(missed))
if missed:
    for m in missed[:20]:
        print(' ', m)
