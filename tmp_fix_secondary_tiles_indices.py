#!/usr/bin/env python3
"""
Regenerate imported Johto/Kanto secondary tileset PNGs in indexed mode.

Previous merge logic converted to RGB, which broke 4bpp tile indices.
This script rebuilds tiles.png as mode 'P' using primary overflow + secondary
source tiles from pokesequel while preserving palette indices (0-15).
"""

from pathlib import Path
from PIL import Image

ROOT_EMR = Path("/mnt/c/Users/drfor/Documents/Porymap/Decomps/pokeemerald")
ROOT_SEQ = Path("/mnt/c/Users/drfor/Documents/Porymap/Decomps/pokesequel-master")

SEQ_PRIMARY = ROOT_SEQ / "data/tilesets/primary"
SEQ_SECONDARY = ROOT_SEQ / "data/tilesets/secondary"
EMR_SECONDARY = ROOT_EMR / "data/tilesets/secondary"

# pokeemerald secondary dir -> (pokesequel secondary dir, pokesequel primary dir)
SEC_MAP = {
    "azalea_town": ("azalea_town", "johto_south"),
    "azalea_town_gym": ("azalea_town_gym", "johto_building"),
    "barn": ("barn", "johto_building"),
    "bellchime_trail": ("bellchime_trail", "johto_north_west"),
    "blackthorn": ("blackthorn", "johto_north_east"),
    "blackthorn_gym": ("blackthorn_gym", "johto_building"),
    "burned_tower": ("burned_tower", "johto_building"),
    "cafe": ("cafe", "johto_building"),
    "cave_default": ("cave_default", "johto_general"),
    "cave_dragons_den": ("cave_dragons_den", "johto_north_east"),
    "cave_gray": ("cave_gray", "johto_north_east"),
    "cave_ice": ("cave_ice", "johto_general"),
    "cave_mt_moon": ("cave_mt_moon", "johto_general"),
    "cherrygrove_city": ("cherrygrove_city", "johto_general"),
    "cianwood_city": ("cianwood_city", "johto_north_east"),
    "cianwood_city_gym": ("cianwood_city_gym", "johto_building"),
    "dragons_den_shrine": ("dragons_den_shrine", "johto_building"),
    "ecruteak_city": ("ecruteak_city", "johto_north_west"),
    "ecruteak_city_gym": ("ecruteak_city_gym", "johto_building"),
    "ecruteak_theater": ("ecruteak_theater", "johto_building"),
    "gate_standard": ("gate_standard", "johto_building"),
    "goldenrod": ("goldenrod", "johto_general"),
    "goldenrod_station": ("goldenrod_station", "johto_building"),
    "goldenrod_underground_rocket": ("goldenrod_underground_rocket", "johto_building"),
    "goldenrod_underground_storage": ("goldenrod_underground_storage", "johto_building"),
    "goldenrod_underground_tunnel": ("goldenrod_underground_tunnel", "johto_building"),
    "house_2": ("house_2", "johto_building"),
    "house_lab": ("house_lab", "johto_building"),
    "ilex_forest": ("ilex_forest", "johto_south"),
    "johto_bike_shop": ("johto_bike_shop", "johto_building"),
    "johto_mart": ("johto_mart", "johto_building"),
    "kurts_house": ("kurts_house", "johto_building"),
    "lighthouse": ("lighthouse", "johto_building"),
    "mahogany_town": ("mahogany_town", "johto_north_east"),
    "mt_silver_snow": ("mt_silver_snow", "johto_general"),
    "national_park": ("national_park", "kanto_general"),
    "new_bark_town": ("new_bark_town", "johto_general"),
    "olivine_city": ("olivine_city", "johto_general"),
    "players_house": ("players_house", "johto_building"),
    "pokemon_center_white": ("pokemon_center_white", "johto_building"),
    "port_indoor": ("port_indoor", "johto_general"),
    "power_plant_generator_room": ("power_plant_generator_room", "johto_building"),
    "route38_farmland": ("route38_farmland", "johto_general"),
    "route_32": ("route_32", "johto_general"),
    "ruins_of_alph_b1_f": ("ruins_of_alph_b1_f", "johto_building"),
    "ruins_of_alph_outside": ("ruins_of_alph_outside", "johto_north_east"),
    "ruins_of_alph_writing": ("ruins_of_alph_writing", "johto_building"),
    "safari_zone_entrance": ("safari_zone_entrance", "johto_building"),
    "safari_zone_johto": ("safari_zone_johto", "johto_north_east"),
    "shop_rooftop": ("shop_rooftop", "johto_building"),
    "ssaqua": ("ssaqua", "johto_building"),
    "trainer_school": ("trainer_school", "johto_building"),
    "violet_city": ("violet_city", "johto_general"),
    "whirl_islands": ("whirl_islands", "johto_north_east"),
}


def rebuild(emr_sec: str, seq_sec: str, seq_pri: str) -> bool:
    sec_src = SEQ_SECONDARY / seq_sec / "tiles.png"
    pri_src = SEQ_PRIMARY / seq_pri / "tiles.png"
    dst = EMR_SECONDARY / emr_sec / "tiles.png"

    if not sec_src.exists() or not pri_src.exists() or not dst.parent.exists():
        print(f"[MISSING] {emr_sec}")
        return False

    sec = Image.open(sec_src).convert("P")
    pri = Image.open(pri_src).convert("P")
    overflow = pri.crop((0, 256, 128, 320)).convert("P")

    merged = Image.new("P", (128, 64 + sec.height))
    merged.putpalette(sec.getpalette())
    overflow.putpalette(sec.getpalette())
    merged.paste(overflow, (0, 0))
    merged.paste(sec, (0, 64))
    merged.save(dst)

    max_index = max(merged.getdata())
    print(f"[OK] {emr_sec}: mode=P size={merged.size[0]}x{merged.size[1]} max_index={max_index}")
    return True


def main() -> None:
    done = 0
    for emr_sec, (seq_sec, seq_pri) in sorted(SEC_MAP.items()):
        if rebuild(emr_sec, seq_sec, seq_pri):
            done += 1
    print(f"\nRebuilt secondary tilesets: {done}/{len(SEC_MAP)}")


if __name__ == "__main__":
    main()
