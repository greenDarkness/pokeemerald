"""
Create viridian_city_johto tileset variant.
- Same tiles.png and palettes as viridian_city (visual content identical)
- metatiles.bin uses JohtoNorthEast overflow (needed for Route28's 12 cells in 512-639 range)
- Used by: Route26North_Layout (JohtoGeneral) and Route28_Layout (JohtoNorthEast)
"""
from pathlib import Path
import shutil

root = Path("C:/Users/drfor/Documents/Porymap/Decomps")
em   = root / "pokeemerald"
seq  = root / "pokesequel-master"

src = em / "data/tilesets/secondary/viridian_city"
dst = em / "data/tilesets/secondary/viridian_city_johto"

# Copy all visual assets from viridian_city
(dst / "palettes").mkdir(parents=True, exist_ok=True)
shutil.copy2(src / "tiles.png", dst / "tiles.png")
for pal in (src / "palettes").iterdir():
    shutil.copy2(pal, dst / "palettes" / pal.name)

# Build metatiles.bin from pokesequel source with JohtoNorthEast overflow
seq_meta_src = seq / "data/tilesets/secondary/viridian_city/metatiles.bin"
seq_attr_src  = seq / "data/tilesets/secondary/viridian_city/metatile_attributes.bin"
orig_meta = seq_meta_src.read_bytes()
orig_attr  = seq_attr_src.read_bytes()

prim_meta = (seq / "data/tilesets/primary/johto_north_east/metatiles.bin").read_bytes()
prim_attr  = (seq / "data/tilesets/primary/johto_north_east/metatile_attributes.bin").read_bytes()
overflow_meta = prim_meta[512*16 : 640*16]
overflow_attr  = prim_attr[512*2  : 640*2]

(dst / "metatiles.bin").write_bytes(overflow_meta + orig_meta)
(dst / "metatile_attributes.bin").write_bytes(overflow_attr + orig_attr)

n = (len(overflow_meta) + len(orig_meta)) // 16
print(f"Created viridian_city_johto: {n} metatiles, JohtoNorthEast overflow")
