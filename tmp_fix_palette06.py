#!/usr/bin/env python3
"""
Fix palette index 06 for imported Johto/Kanto secondary tilesets.

Pokesequel uses NUM_PALS_IN_PRIMARY=7, so palette slot 6 comes from primary.
Pokeemerald uses NUM_PALS_IN_PRIMARY=6, so secondary file 06.gbapal is used.
For imported secondaries, copy the matching primary's 06.pal into each
secondary's palettes/06.pal and regenerate 06.gbapal.
"""

from pathlib import Path
import struct

ROOT_EMR = Path("/mnt/c/Users/drfor/Documents/Porymap/Decomps/pokeemerald")
ROOT_SEQ = Path("/mnt/c/Users/drfor/Documents/Porymap/Decomps/pokesequel-master")

SEQ_PRIMARY = ROOT_SEQ / "data/tilesets/primary"
EMR_SECONDARY = ROOT_EMR / "data/tilesets/secondary"

# pokeemerald secondary dir -> pokesequel primary dir for palette 06
SEC_TO_PRI = {
    # johto_general
    "cave_ice": "johto_general",
    "cave_mt_moon": "johto_general",
    "violet_city": "johto_general",
    "cave_default": "johto_general",
    "olivine_city": "johto_general",
    "goldenrod": "johto_general",
    "port_indoor": "johto_general",
    "route38_farmland": "johto_general",
    "route_32": "johto_general",
    "mt_silver_snow": "johto_general",
    "new_bark_town": "johto_general",
    # johto_building
    "trainer_school": "johto_building",
    "goldenrod_underground_rocket": "johto_building",
    "power_plant_generator_room": "johto_building",
    "burned_tower": "johto_building",
    "ecruteak_theater": "johto_building",
    "johto_bike_shop": "johto_building",
    "ruins_of_alph_b1_f": "johto_building",
    "ruins_of_alph_writing": "johto_building",
    "cafe": "johto_building",
    "kurts_house": "johto_building",
    "goldenrod_station": "johto_building",
    "shop_rooftop": "johto_building",
    "blackthorn_gym": "johto_building",
    "johto_mart": "johto_building",
    "azalea_town_gym": "johto_building",
    "ecruteak_city_gym": "johto_building",
    "safari_zone_entrance": "johto_building",
    "goldenrod_underground_tunnel": "johto_building",
    "dragons_den_shrine": "johto_building",
    "players_house": "johto_building",
    "house_2": "johto_building",
    "ssaqua": "johto_building",
    "gate_standard": "johto_building",
    "lighthouse": "johto_building",
    "barn": "johto_building",
    "house_lab": "johto_building",
    "pokemon_center_white": "johto_building",
    "cianwood_city_gym": "johto_building",
    "goldenrod_underground_storage": "johto_building",
    # johto_south
    "ilex_forest": "johto_south",
    "azalea_town": "johto_south",
    # johto_north_west
    "bellchime_trail": "johto_north_west",
    "ecruteak_city": "johto_north_west",
    # johto_north_east
    "safari_zone_johto": "johto_north_east",
    "cave_gray": "johto_north_east",
    "cherrygrove_city": "johto_general",
    "ruins_of_alph_outside": "johto_north_east",
    "cianwood_city": "johto_north_east",
    "blackthorn": "johto_north_east",
    "whirl_islands": "johto_north_east",
    "mahogany_town": "johto_north_east",
    "cave_dragons_den": "johto_north_east",
    # kanto_general
    "national_park": "kanto_general",
}


def pal_to_gbapal(pal_text: str) -> bytes:
    lines = pal_text.splitlines()
    colors = []
    i = 0
    while i < len(lines):
        line = lines[i].strip()
        if line in ("JASC-PAL", "0100"):
            i += 1
            continue
        try:
            count = int(line)
            i += 1
            for _ in range(count):
                if i >= len(lines):
                    break
                parts = lines[i].strip().split()
                if len(parts) >= 3:
                    r, g, b = int(parts[0]), int(parts[1]), int(parts[2])
                    gba = ((r >> 3) & 0x1F) | (((g >> 3) & 0x1F) << 5) | (((b >> 3) & 0x1F) << 10)
                    colors.append(gba)
                i += 1
            break
        except ValueError:
            i += 1

    while len(colors) < 16:
        colors.append(0)
    return struct.pack("<" + "H" * 16, *colors[:16])


def main() -> None:
    fixed = 0
    missing = 0

    for sec_dir, pri_dir in sorted(SEC_TO_PRI.items()):
        src = SEQ_PRIMARY / pri_dir / "palettes" / "06.pal"
        dst_dir = EMR_SECONDARY / sec_dir / "palettes"
        dst_pal = dst_dir / "06.pal"
        dst_gba = dst_dir / "06.gbapal"

        if not src.exists() or not dst_dir.exists():
            print(f"[MISSING] {sec_dir} (src={src.exists()}, dst_dir={dst_dir.exists()})")
            missing += 1
            continue

        pal_text = src.read_text(errors="replace")
        dst_pal.write_text(pal_text)
        dst_gba.write_bytes(pal_to_gbapal(pal_text))
        print(f"[FIXED] {sec_dir} <- {pri_dir}/06.pal")
        fixed += 1

    print(f"\nDone: fixed={fixed}, missing={missing}")


if __name__ == "__main__":
    main()
