"""
Fix metatile tables for Johto secondary tilesets that were imported without
the 128-entry overflow block.

In pokesequel, primary tilesets have 640 metatiles (0-639).
In pokeemerald with NUM_METATILES_IN_PRIMARY=512, secondary metatile IDs start
at 512. To bridge the gap, the first 128 secondary entries must contain copies
of primary metatiles 512-639 ("overflow"), then the original secondary data
follows at indices 128+.
"""
from pathlib import Path
import shutil

root = Path("C:/Users/drfor/Documents/Porymap/Decomps")
em   = root / "pokeemerald"
seq  = root / "pokesequel-master"

def get_overflow(primary_name):
    """Return (meta_overflow_bytes, attr_overflow_bytes) from primary 512..639."""
    # pokeemerald primaries were cropped to 512, so use pokesequel source
    seq_meta = seq / "data/tilesets/primary" / primary_name / "metatiles.bin"
    seq_attr = seq / "data/tilesets/primary" / primary_name / "metatile_attributes.bin"
    meta = seq_meta.read_bytes()
    attr = seq_attr.read_bytes()
    overflow_meta = meta[512*16 : 640*16]
    overflow_attr = attr[512*2  : 640*2]
    assert len(overflow_meta) == 128*16, f"meta overflow bad size: {len(overflow_meta)}"
    assert len(overflow_attr) == 128*2,  f"attr overflow bad size: {len(overflow_attr)}"
    return overflow_meta, overflow_attr

def fix_inplace(secondary_name, primary_name):
    sec_dir = em / "data/tilesets/secondary" / secondary_name
    meta_path = sec_dir / "metatiles.bin"
    attr_path = sec_dir / "metatile_attributes.bin"

    orig_meta = meta_path.read_bytes()
    orig_attr = attr_path.read_bytes()
    n_meta = len(orig_meta) // 16
    n_attr = len(orig_attr) // 2

    print(f"  {secondary_name}: {n_meta} metatiles + 128 overflow from {primary_name}")

    overflow_meta, overflow_attr = get_overflow(primary_name)
    combined_meta = overflow_meta + orig_meta
    combined_attr = overflow_attr + orig_attr
    assert len(combined_meta) // 16 <= 512, f"exceeds 512: {len(combined_meta)//16}"

    meta_path.write_bytes(combined_meta)
    attr_path.write_bytes(combined_attr)
    print(f"    -> {len(combined_meta)//16} metatiles, {len(combined_attr)//2} attrs")

# ---- In-place fixes ----
# For each: no Hoenn conflict; just prepend overflow from dominant primary
print("=== In-place overflow fixes ===")
fix_inplace("blackthorn",           "johto_north_east")  # dominant: BlackthornCity/Route45/MtSilver use JohtoNE
fix_inplace("house_lab",            "johto_building")
fix_inplace("kanto_pokemon_center", "johto_building")     # no cells in 512-639 range; JohtoBuilding is dominant
fix_inplace("viridian_city",        "kanto_general")      # for ViridianCity/Route2/Route22 (KantoGeneral primary)

print()
print("Done.")
