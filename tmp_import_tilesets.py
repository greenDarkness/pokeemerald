#!/usr/bin/env python3
"""
Import Johto/Kanto tilesets from pokesequel-master into pokeemerald.

Strategy:
  - Primary tilesets (640 tiles in pokesequel) → crop to 512 tiles in pokeemerald.
    Overflow tiles (indices 512-639) are stored in secondary tileset slots 0-127.
  - Secondary tilesets (384 tiles in pokesequel) → prepend 128 overflow tiles from
    primary → 512-tile secondary sheet (128 overflow + 384 original = 512).
  - metatiles.bin and metatile_attributes.bin are merged the same way:
    primary overflow entries (512-639) prepend secondary entries.
  - Palettes: copy all 16 .pal files; generate .gbapal from them.
  - layouts.json: update the 245 Johto layouts from placeholder tilesets to real ones.
  - Register new tilesets in graphics.h and headers.h.

Usage: wsl -e python3 /mnt/c/Users/drfor/Documents/Porymap/Decomps/pokeemerald/tmp_import_tilesets.py
"""

import json
import os
import shutil
import struct
from pathlib import Path

try:
    from PIL import Image
    HAS_PIL = True
except ImportError:
    HAS_PIL = False
    print("WARNING: Pillow not available. Image processing will be skipped.")

# ── Paths ──────────────────────────────────────────────────────────────────────
POKESEQUEL = Path("/mnt/c/Users/drfor/Documents/Porymap/Decomps/pokesequel-master")
POKEEMERALD = Path("/mnt/c/Users/drfor/Documents/Porymap/Decomps/pokeemerald")

SEQ_PRIMARY_DIR = POKESEQUEL / "data/tilesets/primary"
SEQ_SECONDARY_DIR = POKESEQUEL / "data/tilesets/secondary"
EMR_PRIMARY_DIR = POKEEMERALD / "data/tilesets/primary"
EMR_SECONDARY_DIR = POKEEMERALD / "data/tilesets/secondary"

GRAPHICS_H = POKEEMERALD / "src/data/tilesets/graphics.h"
HEADERS_H = POKEEMERALD / "src/data/tilesets/headers.h"
LAYOUTS_JSON = POKEEMERALD / "data/layouts/layouts.json"
SEQ_LAYOUTS_JSON = POKESEQUEL / "data/layouts/layouts.json"

# ── Constants ─────────────────────────────────────────────────────────────────
SEQ_PRIMARY_TILES = 640   # pokesequel primary tile count
EMR_PRIMARY_TILES = 512   # pokeemerald primary tile count
OVERFLOW_TILES    = 128   # tiles 512-639
TILE_W            = 128   # pixels (16 tiles wide × 8px)
TILE_H_SEQ_PRI    = 320   # pixels (40 rows × 8px = 640 tiles)
TILE_H_EMR_PRI    = 256   # pixels (32 rows × 8px = 512 tiles)
OVERFLOW_H        = 64    # pixels (8 rows × 8px = 128 tiles)

# metatile_attributes_size=2 (from porymap.project.cfg)
METATILE_BYTES     = 16   # 8 tile-slots × 2 bytes each
ATTR_BYTES         = 2    # 2 bytes per metatile attribute
OVERFLOW_METATILES = 128  # entries 512-639 of primary metatiles.bin

# ── Primary tileset mapping ────────────────────────────────────────────────────
# seq_cname → (seq_dir, emr_dir, emr_cname, needs_processing)
# needs_processing=False means data already exists (skip, just register if needed)
PRIMARIES = {
    "gTileset_Johto_General":   ("johto_general",    "johto_general",    "gTileset_JohtoGeneral",   True),
    "gTileset_Johto_Building":  ("johto_building",   "johto_building",   "gTileset_JohtoBuilding",  True),
    "gTileset_Johto_South":     ("johto_south",      "johto_south",      "gTileset_JohtoSouth",     True),
    "gTileset_Johto_NorthEast": ("johto_north_east", "johto_north_east", "gTileset_JohtoNorthEast", True),
    "gTileset_Johto_NorthWest": ("johto_north_west", "johto_north_west", "gTileset_JohtoNorthWest", True),
    # Kanto primaries already exist in pokeemerald (from pokeorigin), skip tile processing
    "gTileset_Kanto_General":   ("kanto_general",    "kanto_general",    "gTileset_KantoGeneral",   False),
    "gTileset_Kanto_Building":  ("kanto_building",   "kanto_building",   "gTileset_KantoBuilding",  False),
}

# ── Secondary tileset mapping ──────────────────────────────────────────────────
# seq_cname → (seq_dir, emr_dir, emr_cname, already_registered_in_headers)
# already_registered=True means entry exists in headers.h AND data files exist
SECONDARIES = {
    # === Kanto secondaries already registered from pokeorigin ===
    "gTileset_CeladonCity":             ("celadon_city",       "celadon_city",       "gTileset_CeladonCity",       True),
    "gTileset_CeruleanCity":            ("cerulean_city",      "cerulean_city",      "gTileset_CeruleanCity",      True),
    "gTileset_CeruleanCity_Gym":        ("cerulean_city_gym",  "cerulean_gym",       "gTileset_CeruleanGym",       True),
    "gTileset_DepartmentStore":         ("department_store",   "department_store",   "gTileset_DepartmentStore",   True),
    "gTileset_Fuchsia":                 ("fuchsia",            "fuchsia_city",       "gTileset_FuchsiaCity",       True),
    "gTileset_FuchsiaCity_Gym":         ("fuchsia_city_gym",   "fuchsia_gym",        "gTileset_FuchsiaGym",        True),
    "gTileset_GameCorner":              ("game_corner",        "game_corner",        "gTileset_GameCorner",        True),
    "gTileset_HallOfFame":              ("hall_of_fame",       "hall_of_fame",       "gTileset_HallOfFame",        True),
    "gTileset_IndigoPlateau":           ("indigo_plateau",     "indigo_plateau",     "gTileset_IndigoPlateau",     True),
    "gTileset_Kanto_PokemonCenter":     ("kanto_pokemon_center","kanto_pokemon_center","gTileset_KantoPokemonCenter",True),
    "gTileset_KantoMart":               ("kanto_mart",         "kanto_mart",         "gTileset_KantoMart",         False),
    "gTileset_Lab":                     ("lab",                "lab",                "gTileset_Lab",               True),
    "gTileset_LavenderTown":            ("lavender_town",      "lavender_town",      "gTileset_LavenderTown",      True),
    "gTileset_Lavaridge":               ("lavaridge",          "lavaridge",          "gTileset_Lavaridge",         True),
    "gTileset_Museum":                  ("museum",             "museum",             "gTileset_Museum",            True),
    "gTileset_NavelRock":               ("navel_rock",         "navel_rock",         "gTileset_NavelRock",         True),
    "gTileset_PalletTown":              ("pallet_town",        "pallet_town",        "gTileset_PalletTown",        True),
    "gTileset_PewterCity":              ("pewter_city",        "pewter_city",        "gTileset_PewterCity",        True),
    "gTileset_PokemonLeague":           ("pokemon_league",     "pokemon_league",     "gTileset_PokemonLeague",     True),
    "gTileset_SaffronCity":             ("saffron_city",       "saffron_city",       "gTileset_SaffronCity",       True),
    "gTileset_SaffronCity_Gym":         ("saffron_city_gym",   "saffron_gym",        "gTileset_SaffronGym",        True),
    "gTileset_SaffronCity_FightingDojoVIP": ("saffron_city_dojo_vip","saffron_gym", "gTileset_SaffronGym",        True),
    "gTileset_SeaCottage":              ("sea_cottage",        "sea_cottage",        "gTileset_SeaCottage",        True),
    "gTileset_SilphCo":                 ("silph_co",           "silph_co",           "gTileset_SilphCo",           True),
    "gTileset_SootopolisGym":           ("sootopolis_gym",     "sootopolis_gym",     "gTileset_SootopolisGym",     True),
    "gTileset_Vermilion":               ("vermilion",          "vermilion_city",     "gTileset_VermilionCity",     True),
    "gTileset_VermilionCity_Gym":       ("vermilion_city_gym", "vermilion_gym",      "gTileset_VermilionGym",      True),
    "gTileset_ViridianCity":            ("viridian_city",      "viridian_city",      "gTileset_ViridianCity",      True),
    "gTileset_ViridianCity_Gym":        ("viridian_city_gym",  "viridian_gym",       "gTileset_ViridianGym",       True),
    "gTileset_ViridianForest":          ("viridian_forest",    "viridian_forest",    "gTileset_ViridianForest",    True),
    "gTileset_TrainerHill_Courtyard":   ("trainer_hill",       "trainer_hill",       "gTileset_TrainerHill",       True),

    # === New Johto secondary tilesets ===
    "gTileset_AzaleaTown":              ("azalea_town",              "azalea_town",              "gTileset_AzaleaTown",             False),
    "gTileset_AzaleaTown_Gym":          ("azalea_town_gym",          "azalea_town_gym",          "gTileset_AzaleaTownGym",          False),
    "gTileset_Barn":                    ("barn",                     "barn",                     "gTileset_Barn",                   False),
    "gTileset_BattleTowerInner":        ("battle_tower_inner",       "battle_tower",             "gTileset_BattleTowerInner",       False),
    "gTileset_BellchimeTrail":          ("bellchime_trail",          "bellchime_trail",          "gTileset_BellchimeTrail",         False),
    "gTileset_BikeShop":                ("bike_shop",                "bike_shop",                "gTileset_BikeShop",               False),
    "gTileset_Blackthorn":              ("blackthorn",               "blackthorn",               "gTileset_Blackthorn",             False),
    "gTileset_BlackthornGym":           ("blackthorn_gym",           "blackthorn_gym",           "gTileset_BlackthornGym",          False),
    "gTileset_BurnedTower":             ("burned_tower",             "burned_tower",             "gTileset_BurnedTower",            False),
    "gTileset_Cafe":                    ("cafe",                     "cafe",                     "gTileset_Cafe",                   False),
    "gTileset_Cave_Default":            ("cave_default",             "cave_default",             "gTileset_CaveDefault",            False),
    "gTileset_Cave_DragonsDen":         ("cave_dragons_den",         "cave_dragons_den",         "gTileset_CaveDragonsDen",         False),
    "gTileset_Cave_Gray":               ("cave_gray",                "cave_gray",                "gTileset_CaveGray",               False),
    "gTileset_Cave_Green":              ("cave_green",               "cave_green",               "gTileset_CaveGreen",              False),
    "gTileset_Cave_Ice":                ("cave_ice",                 "cave_ice",                 "gTileset_CaveIce",                False),
    "gTileset_Cave_MtMoon":             ("cave_mt_moon",             "cave_mt_moon",             "gTileset_CaveMtMoon",             False),
    "gTileset_Cave_Sandy":              ("cave_sandy",               "cave_sandy",               "gTileset_CaveSandy",              False),
    "gTileset_CeladonApartments":       ("celadon_apartments",       "celadon_apartments",       "gTileset_CeladonApartments",      False),
    "gTileset_CherrygroveCity":         ("cherrygrove_city",         "cherrygrove_city",         "gTileset_CherrygroveCity",        False),
    "gTileset_CianwoodCity":            ("cianwood_city",            "cianwood_city",            "gTileset_CianwoodCity",           False),
    "gTileset_CianwoodCity_Gym":        ("cianwood_city_gym",        "cianwood_city_gym",        "gTileset_CianwoodGym",            False),
    "gTileset_CyclingRoad":             ("cycling_road",             "cycling_road",             "gTileset_CyclingRoad",            False),
    "gTileset_DragonsDen_Shrine":       ("dragons_den_shrine",       "dragons_den_shrine",       "gTileset_DragonsDenShrine",       False),
    "gTileset_Ecruteak_City":           ("ecruteak_city",            "ecruteak_city",            "gTileset_EcruteakCity",           False),
    "gTileset_EcruteakCity_Gym":        ("ecruteak_city_gym",        "ecruteak_city_gym",        "gTileset_EcruteakGym",            False),
    "gTileset_EcruteakTheater":         ("ecruteak_theater",         "ecruteak_theater",         "gTileset_EcruteakTheater",        False),
    "gTileset_Gate_Standard":           ("gate_standard",            "gate_standard",            "gTileset_GateStandard",           False),
    "gTileset_Goldenrod":               ("goldenrod",                "goldenrod",                "gTileset_Goldenrod",              False),
    "gTileset_Goldenrod_Underground_Storage": ("goldenrod_underground_storage", "goldenrod_underground_storage", "gTileset_GoldenrodUndergroundStorage", False),
    "gTileset_GoldenrodCity_TrainStation": ("goldenrod_station",     "goldenrod_station",        "gTileset_GoldenrodStation",       False),
    "gTileset_GoldenrodUndergroundRocket": ("goldenrod_underground_rocket", "goldenrod_underground_rocket", "gTileset_GoldenrodUndergroundRocket", False),
    "gTileset_GoldenrodUndergroundTunnel": ("goldenrod_underground_tunnel", "goldenrod_underground_tunnel", "gTileset_GoldenrodUndergroundTunnel", False),
    "gTileset_House_2":                 ("house_2",                  "house_2",                  "gTileset_House2",                 False),
    "gTileset_House_Lab":               ("house_lab",                "house_lab",                "gTileset_HouseLab",               False),
    "gTileset_IlexForest":              ("ilex_forest",              "ilex_forest",              "gTileset_IlexForest",             False),
    "gTileset_JohtoBikeShop":           ("johto_bike_shop",          "johto_bike_shop",          "gTileset_JohtoBikeShop",          False),
    "gTileset_JohtoMart":               ("johto_mart",               "johto_mart",               "gTileset_JohtoMart",              False),
    "gTileset_KurtsHouse":              ("kurts_house",              "kurts_house",              "gTileset_KurtsHouse",             False),
    "gTileset_Lighthouse":              ("lighthouse",               "lighthouse",               "gTileset_Lighthouse",             False),
    "gTileset_MahoganyTown":            ("mahogany_town",            "mahogany_town",            "gTileset_MahoganyTown",           False),
    "gTileset_MtSilverSnow":            ("mt_silver_snow",           "mt_silver_snow",           "gTileset_MtSilverSnow",           False),
    "gTileset_NationalPark":            ("national_park",            "national_park",            "gTileset_NationalPark",           False),
    "gTileset_NewBarkTown":             ("new_bark_town",            "new_bark_town",            "gTileset_NewBarkTown",            False),
    "gTileset_OlivineCity":             ("olivine_city",             "olivine_city",             "gTileset_OlivineCity",            False),
    "gTileset_PlayersHouse":            ("players_house",            "players_house",            "gTileset_PlayersHouse",           False),
    "gTileset_PokemonCenter_White":     ("pokemon_center_white",     "pokemon_center_white",     "gTileset_PokemonCenterWhite",     False),
    "gTileset_PokemonDayCare":          ("pokemon_day_care",         "pokemon_day_care",         "gTileset_PokemonDayCare",         False),
    "gTileset_PortIndoor":              ("port_indoor",              "port_indoor",              "gTileset_PortIndoor",             False),
    "gTileset_PowerPlant_GeneratorRoom":("power_plant_generator_room","power_plant_generator_room","gTileset_PowerPlantGeneratorRoom",False),
    "gTileset_Route32":                 ("route_32",                 "route_32",                 "gTileset_Route32",                False),
    "gTileset_Route38_Farmland":        ("route38_farmland",         "route38_farmland",         "gTileset_Route38Farmland",        False),
    "gTileset_RuinsOfAlph_B1F":         ("ruins_of_alph_b1_f",       "ruins_of_alph_b1_f",       "gTileset_RuinsOfAlphB1F",         False),
    "gTileset_RuinsOfAlph_Outside":     ("ruins_of_alph_outside",    "ruins_of_alph_outside",    "gTileset_RuinsOfAlphOutside",     False),
    "gTileset_RuinsOfAlphWriting":      ("ruins_of_alph_writing",    "ruins_of_alph_writing",    "gTileset_RuinsOfAlphWriting",     False),
    "gTileset_SafariZone_Entrance":     ("safari_zone_entrance",     "safari_zone_entrance",     "gTileset_SafariZoneEntrance",     False),
    "gTileset_SafariZoneJohto":         ("safari_zone_johto",        "safari_zone_johto",        "gTileset_SafariZoneJohto",        False),
    "gTileset_ShopRooftop":             ("shop_rooftop",             "shop_rooftop",             "gTileset_ShopRooftop",            False),
    "gTileset_SoulHouse":               ("soul_house",               "soul_house",               "gTileset_SoulHouse",              False),
    "gTileset_ssaqua":                  ("ssaqua",                   "ssaqua",                   "gTileset_SSAqua",                 False),
    "gTileset_TrainerSchool":           ("trainer_school",           "trainer_school",           "gTileset_TrainerSchool",          False),
    "gTileset_Unused3":                 ("unused3",                  "unused3",                  "gTileset_Unused3",                False),
    "gTileset_VioletCity":              ("violet_city",              "violet_city",              "gTileset_VioletCity",             False),
    "gTileset_WhirlIslands":            ("whirl_islands",            "whirl_islands",            "gTileset_WhirlIslands",           False),
}

# ── Palette helpers ────────────────────────────────────────────────────────────
def read_pal_file(pal_path):
    """Read a JASC-PAL file and return list of (r,g,b) tuples."""
    lines = Path(pal_path).read_text(errors="replace").splitlines()
    colors = []
    i = 0
    while i < len(lines):
        line = lines[i].strip()
        if line in ("JASC-PAL", "0100"):
            i += 1
            continue
        try:
            count = int(line)
            # This is the color count line; next 'count' lines are colors
            i += 1
            for _ in range(count):
                if i < len(lines):
                    parts = lines[i].strip().split()
                    if len(parts) >= 3:
                        colors.append((int(parts[0]), int(parts[1]), int(parts[2])))
                    i += 1
            break
        except ValueError:
            try:
                parts = line.split()
                if len(parts) >= 3:
                    colors.append((int(parts[0]), int(parts[1]), int(parts[2])))
            except (ValueError, IndexError):
                pass
            i += 1
    # Pad to 16 colors if needed
    while len(colors) < 16:
        colors.append((0, 0, 0))
    return colors[:16]

def rgb_to_gba(r, g, b):
    """Convert 8-bit RGB to 15-bit GBA BGR555."""
    return ((r >> 3) & 0x1F) | (((g >> 3) & 0x1F) << 5) | (((b >> 3) & 0x1F) << 10)

def write_gbapal(colors, out_path):
    """Write a .gbapal binary file from a list of 16 (r,g,b) tuples."""
    data = struct.pack('<' + 'H' * 16, *(rgb_to_gba(*c) for c in colors))
    Path(out_path).write_bytes(data)

def copy_palettes(src_pal_dir, dst_pal_dir, fallback_dir=None, forced_sources=None):
    """
    Copy palette files (00-15) from src to dst.
    For missing files, try fallback_dir.
        forced_sources allows overriding specific palette indices:
            {6: Path(".../06.pal")}
    Generates .gbapal from .pal files.
    """
    dst_pal_dir = Path(dst_pal_dir)
    dst_pal_dir.mkdir(parents=True, exist_ok=True)
    forced_sources = forced_sources or {}

    for idx in range(16):
        num = f"{idx:02d}"
        forced_src = forced_sources.get(idx)
        pal_src = Path(src_pal_dir) / f"{num}.pal"
        fb_src  = Path(fallback_dir) / f"{num}.pal" if fallback_dir else None

        # Try main source, then fallback
        chosen = None
        if forced_src and Path(forced_src).exists():
            chosen = Path(forced_src)
        elif pal_src.exists():
            chosen = pal_src
        elif fb_src and fb_src.exists():
            chosen = fb_src

        dst_pal = dst_pal_dir / f"{num}.pal"
        dst_gbapal = dst_pal_dir / f"{num}.gbapal"

        if chosen:
            shutil.copy2(chosen, dst_pal)
            try:
                colors = read_pal_file(chosen)
                write_gbapal(colors, dst_gbapal)
            except Exception as e:
                print(f"  WARNING: could not generate {dst_gbapal.name}: {e}")
        else:
            # Write empty (all-black) palette
            if not dst_pal.exists():
                dst_pal.write_text("JASC-PAL\n0100\n16\n" + "0 0 0\n" * 16)
            if not dst_gbapal.exists():
                write_gbapal([(0, 0, 0)] * 16, dst_gbapal)

# ── Overflow cache ─────────────────────────────────────────────────────────────
# primary_emr_dir → overflow data
_overflow_tiles_cache   = {}   # dir → PIL Image (128 × OVERFLOW_H)
_overflow_meta_cache    = {}   # dir → bytes (OVERFLOW_METATILES × METATILE_BYTES)
_overflow_attr_cache    = {}   # dir → bytes (OVERFLOW_METATILES × ATTR_BYTES)

def get_primary_overflow(seq_dir_name):
    """Load overflow tile/metatile/attr data from a pokesequel primary tileset."""
    if seq_dir_name in _overflow_tiles_cache:
        return (_overflow_tiles_cache[seq_dir_name],
                _overflow_meta_cache[seq_dir_name],
                _overflow_attr_cache[seq_dir_name])

    seq_dir = SEQ_PRIMARY_DIR / seq_dir_name

    # Tiles overflow (rows 32-39 of the 320px primary)
    if HAS_PIL:
        img = Image.open(seq_dir / "tiles.png")
        overflow_img = img.crop((0, EMR_PRIMARY_TILES // 16 * 8, TILE_W, TILE_H_SEQ_PRI))
        # OVERFLOW_H = 64px; rows 32-39 (y=256 to y=320 in 320px image)
        overflow_img = img.crop((0, TILE_H_EMR_PRI, TILE_W, TILE_H_SEQ_PRI))
    else:
        overflow_img = None

    # metatiles.bin overflow (entries 512-639)
    meta_data = (seq_dir / "metatiles.bin").read_bytes()
    overflow_meta = meta_data[EMR_PRIMARY_TILES * METATILE_BYTES : SEQ_PRIMARY_TILES * METATILE_BYTES]

    # metatile_attributes.bin overflow (entries 512-639)
    attr_data = (seq_dir / "metatile_attributes.bin").read_bytes()
    overflow_attr = attr_data[EMR_PRIMARY_TILES * ATTR_BYTES : SEQ_PRIMARY_TILES * ATTR_BYTES]

    _overflow_tiles_cache[seq_dir_name]  = overflow_img
    _overflow_meta_cache[seq_dir_name]   = overflow_meta
    _overflow_attr_cache[seq_dir_name]   = overflow_attr
    return overflow_img, overflow_meta, overflow_attr

# ── Primary processing ─────────────────────────────────────────────────────────
def process_primary(seq_cname, seq_dir_name, emr_dir_name, emr_cname):
    """Crop primary tileset from 640 to 512 tiles. Copy palettes."""
    seq_dir = SEQ_PRIMARY_DIR / seq_dir_name
    emr_dir = EMR_PRIMARY_DIR / emr_dir_name
    emr_dir.mkdir(parents=True, exist_ok=True)

    tiles_dst = emr_dir / "tiles.png"
    if tiles_dst.exists():
        print(f"  [SKIP tiles] {emr_dir_name}/tiles.png already exists")
    elif HAS_PIL:
        img = Image.open(seq_dir / "tiles.png")
        cropped = img.crop((0, 0, TILE_W, TILE_H_EMR_PRI))
        cropped.save(tiles_dst)
        print(f"  [TILES] Cropped {TILE_H_SEQ_PRI}px → {TILE_H_EMR_PRI}px for {emr_dir_name}")

    meta_dst = emr_dir / "metatiles.bin"
    if meta_dst.exists():
        print(f"  [SKIP meta] {emr_dir_name}/metatiles.bin already exists")
    else:
        meta_data = (seq_dir / "metatiles.bin").read_bytes()
        meta_dst.write_bytes(meta_data[:EMR_PRIMARY_TILES * METATILE_BYTES])
        print(f"  [META] Cropped to {EMR_PRIMARY_TILES} entries for {emr_dir_name}")

    attr_dst = emr_dir / "metatile_attributes.bin"
    if attr_dst.exists():
        print(f"  [SKIP attr] {emr_dir_name}/metatile_attributes.bin already exists")
    else:
        attr_data = (seq_dir / "metatile_attributes.bin").read_bytes()
        attr_dst.write_bytes(attr_data[:EMR_PRIMARY_TILES * ATTR_BYTES])
        print(f"  [ATTR] Cropped to {EMR_PRIMARY_TILES} entries for {emr_dir_name}")

    # Palettes: copy all 16 from pokesequel primary
    pal_dst = emr_dir / "palettes"
    if not any(pal_dst.glob("00.pal")):
        copy_palettes(seq_dir / "palettes", pal_dst)
        print(f"  [PAL] Copied palettes for {emr_dir_name}")
    else:
        print(f"  [SKIP pal] Palettes already present for {emr_dir_name}")

# ── Secondary processing ───────────────────────────────────────────────────────
def process_secondary(seq_cname, seq_dir_name, emr_dir_name, emr_cname,
                       primary_seq_dir_name):
    """
    Merge overflow tiles/metatiles from primary with secondary data.
    Result: secondary sheet has 128 overflow tiles at slots 0-127,
    then original secondary tiles at slots 128+.
    """
    seq_dir = SEQ_SECONDARY_DIR / seq_dir_name
    emr_dir = EMR_SECONDARY_DIR / emr_dir_name

    if not seq_dir.exists():
        print(f"  [MISSING SEQ] {seq_dir_name} not found in pokesequel, skipping")
        return

    emr_dir.mkdir(parents=True, exist_ok=True)

    # Get overflow data from the associated primary
    overflow_img, overflow_meta, overflow_attr = get_primary_overflow(primary_seq_dir_name)

    # ── Tiles ──
    tiles_dst = emr_dir / "tiles.png"
    if tiles_dst.exists():
        print(f"  [SKIP tiles] {emr_dir_name}/tiles.png already exists")
    elif HAS_PIL and overflow_img is not None:
        sec_img = Image.open(seq_dir / "tiles.png")
        sec_h = sec_img.height
        # Merged height: overflow rows + secondary rows
        merged_h = OVERFLOW_H + sec_h
        # Preserve raw 4bpp indices by staying in indexed mode (P).
        # Converting to RGB/RGBA destroys tile palette indices and causes
        # incorrect colors in editors/game conversion.
        sec_p = sec_img.convert("P")
        over_p = overflow_img.convert("P")
        merged = Image.new("P", (TILE_W, merged_h))
        merged.putpalette(sec_p.getpalette())
        over_p.putpalette(sec_p.getpalette())
        merged.paste(over_p, (0, 0))
        merged.paste(sec_p, (0, OVERFLOW_H))
        merged.save(tiles_dst)
        print(f"  [TILES] Merged {OVERFLOW_TILES} overflow + {sec_h//8*16} sec tiles → {merged_h//8*16} tiles for {emr_dir_name}")

    # ── metatiles.bin ──
    meta_dst = emr_dir / "metatiles.bin"
    if meta_dst.exists():
        print(f"  [SKIP meta] {emr_dir_name}/metatiles.bin already exists")
    else:
        sec_meta = (seq_dir / "metatiles.bin").read_bytes()
        merged_meta = overflow_meta + sec_meta
        meta_dst.write_bytes(merged_meta)
        n_overflow = len(overflow_meta) // METATILE_BYTES
        n_sec = len(sec_meta) // METATILE_BYTES
        print(f"  [META] Merged {n_overflow} overflow + {n_sec} sec entries → {n_overflow+n_sec} for {emr_dir_name}")

    # ── metatile_attributes.bin ──
    attr_dst = emr_dir / "metatile_attributes.bin"
    if attr_dst.exists():
        print(f"  [SKIP attr] {emr_dir_name}/metatile_attributes.bin already exists")
    else:
        sec_attr = (seq_dir / "metatile_attributes.bin").read_bytes()
        merged_attr = overflow_attr + sec_attr
        attr_dst.write_bytes(merged_attr)
        print(f"  [ATTR] Merged attr for {emr_dir_name}")

    # ── Palettes ──
    pal_dst = emr_dir / "palettes"
    if not any(pal_dst.glob("00.pal")):
        # Pokesequel uses 7 primary palettes; in pokeemerald, slot 06 comes
        # from secondary. Force secondary 06 to primary's 06 to keep colors.
        primary_pal_dir = SEQ_PRIMARY_DIR / primary_seq_dir_name / "palettes"
        copy_palettes(
            seq_dir / "palettes",
            pal_dst,
            fallback_dir=primary_pal_dir,
            forced_sources={6: primary_pal_dir / "06.pal"}
        )
        print(f"  [PAL] Copied palettes for {emr_dir_name}")
    else:
        print(f"  [SKIP pal] Palettes already present for {emr_dir_name}")

# ── layouts.json update ────────────────────────────────────────────────────────
def update_layouts_json(seq_by_id, primary_emr_map, secondary_emr_map):
    """
    Update pokeemerald's layouts.json:
    For each layout currently using placeholder tilesets, find the matching
    pokesequel layout by ID and replace with correct tileset C names.
    """
    PLACEHOLDER_PRIMARY   = "gTileset_General"
    PLACEHOLDER_SECONDARY = "gTileset_Petalburg"

    with open(LAYOUTS_JSON) as f:
        emr_data = json.load(f)

    updated = 0
    skipped_no_match = 0

    for entry in emr_data["layouts"]:
        if (entry.get("primary_tileset")   != PLACEHOLDER_PRIMARY or
            entry.get("secondary_tileset") != PLACEHOLDER_SECONDARY):
            continue  # Already updated or not a Johto placeholder

        layout_id = entry["id"]
        seq_entry = seq_by_id.get(layout_id)
        if seq_entry is None:
            print(f"  [NO MATCH] {layout_id} not found in pokesequel layouts.json")
            skipped_no_match += 1
            continue

        seq_pri = seq_entry["primary_tileset"]
        seq_sec = seq_entry["secondary_tileset"]

        emr_pri = primary_emr_map.get(seq_pri)
        emr_sec = secondary_emr_map.get(seq_sec)

        if emr_pri is None:
            print(f"  [UNKNOWN PRIMARY] {seq_pri} for {layout_id}")
            continue
        if emr_sec is None:
            print(f"  [UNKNOWN SECONDARY] {seq_sec} for {layout_id}")
            continue

        entry["primary_tileset"]   = emr_pri
        entry["secondary_tileset"] = emr_sec
        updated += 1

    with open(LAYOUTS_JSON, "w") as f:
        json.dump(emr_data, f, indent=2)
        f.write("\n")

    print(f"\nlayouts.json: {updated} layouts updated, {skipped_no_match} had no match in pokesequel")

# ── Code registration ──────────────────────────────────────────────────────────
def is_registered_in_graphics(emr_cname):
    content = GRAPHICS_H.read_text()
    return f"gTilesetTiles_{emr_cname[len('gTileset_'):]}" in content

def is_registered_in_headers(emr_cname):
    content = HEADERS_H.read_text()
    return f"gTileset_{emr_cname[len('gTileset_'):]}" in content or emr_cname in content

def graphics_primary_block(emr_cname, emr_dir):
    name = emr_cname[len("gTileset_"):]
    path_base = f"data/tilesets/primary/{emr_dir}"
    lines = [
        f"const u32 gTilesetTiles_{name}[] = INCBIN_U32(\"{path_base}/tiles.4bpp.lz\");",
        f"",
        f"const u16 ALIGNED(4) gTilesetPalettes_{name}[][16] =",
        f"{{",
    ]
    for i in range(16):
        lines.append(f"    INCBIN_U16(\"{path_base}/palettes/{i:02d}.gbapal\"),")
    lines.append("};")
    lines.append("")
    lines.append(f"const u16 gMetatiles_{name}[] = INCBIN_U16(\"{path_base}/metatiles.bin\");")
    lines.append(f"const u16 gMetatileAttributes_{name}[] = INCBIN_U16(\"{path_base}/metatile_attributes.bin\");")
    lines.append("")
    return "\n".join(lines)

def graphics_secondary_block(emr_cname, emr_dir):
    name = emr_cname[len("gTileset_"):]
    path_base = f"data/tilesets/secondary/{emr_dir}"
    lines = [
        f"const u32 gTilesetTiles_{name}[] = INCBIN_U32(\"{path_base}/tiles.4bpp.lz\");",
        f"",
        f"const u16 ALIGNED(4) gTilesetPalettes_{name}[][16] =",
        f"{{",
    ]
    for i in range(16):
        lines.append(f"    INCBIN_U16(\"{path_base}/palettes/{i:02d}.gbapal\"),")
    lines.append("};")
    lines.append("")
    lines.append(f"const u16 gMetatiles_{name}[] = INCBIN_U16(\"{path_base}/metatiles.bin\");")
    lines.append(f"const u16 gMetatileAttributes_{name}[] = INCBIN_U16(\"{path_base}/metatile_attributes.bin\");")
    lines.append("")
    return "\n".join(lines)

def headers_primary_block(emr_cname):
    name = emr_cname[len("gTileset_"):]
    return (
        f"const struct Tileset {emr_cname} =\n"
        f"{{\n"
        f"    .isCompressed = TRUE,\n"
        f"    .isSecondary = FALSE,\n"
        f"    .tiles = gTilesetTiles_{name},\n"
        f"    .palettes = gTilesetPalettes_{name},\n"
        f"    .metatiles = gMetatiles_{name},\n"
        f"    .metatileAttributes = gMetatileAttributes_{name},\n"
        f"    .callback = NULL,\n"
        f"}};\n"
    )

def headers_secondary_block(emr_cname):
    name = emr_cname[len("gTileset_"):]
    return (
        f"const struct Tileset {emr_cname} =\n"
        f"{{\n"
        f"    .isCompressed = TRUE,\n"
        f"    .isSecondary = TRUE,\n"
        f"    .tiles = gTilesetTiles_{name},\n"
        f"    .palettes = gTilesetPalettes_{name},\n"
        f"    .metatiles = gMetatiles_{name},\n"
        f"    .metatileAttributes = gMetatileAttributes_{name},\n"
        f"    .callback = NULL,\n"
        f"}};\n"
    )

def append_to_graphics(blocks):
    if not blocks:
        return
    content = GRAPHICS_H.read_text()
    addition = "\n// ===== Johto Tilesets =====\n" + "\n".join(blocks) + "\n"
    # Append before the final #endif or at end of file
    content = content.rstrip() + "\n" + addition
    GRAPHICS_H.write_text(content)
    print(f"  [GRAPHICS.H] Added {len(blocks)} blocks")

def append_to_headers(primary_blocks, secondary_blocks):
    content = HEADERS_H.read_text()
    addition_lines = []
    if primary_blocks:
        addition_lines.append("\n// ===== Johto Primary Tilesets =====\n")
        addition_lines.extend(primary_blocks)
    if secondary_blocks:
        addition_lines.append("\n// ===== Johto Secondary Tilesets =====\n")
        addition_lines.extend(secondary_blocks)
    if not addition_lines:
        return
    addition = "\n".join(addition_lines) + "\n"
    content = content.rstrip() + "\n" + addition
    HEADERS_H.write_text(content)
    print(f"  [HEADERS.H] Added {len(primary_blocks)} primary + {len(secondary_blocks)} secondary blocks")

# ── Main ───────────────────────────────────────────────────────────────────────
def main():
    print("=" * 60)
    print("Johto/Kanto Tileset Import")
    print("=" * 60)

    # ── Load layouts ──
    print("\n[1] Loading layouts.json files...")
    with open(SEQ_LAYOUTS_JSON) as f:
        seq_data = json.load(f)
    seq_by_id = {entry["id"]: entry for entry in seq_data["layouts"]}

    with open(LAYOUTS_JSON) as f:
        emr_data = json.load(f)

    # Find which (primary, secondary) pairs are needed by the 245 Johto layouts
    PLACEHOLDER_PRIMARY   = "gTileset_General"
    PLACEHOLDER_SECONDARY = "gTileset_Petalburg"

    needed_pairs = {}   # seq_pri → set of seq_sec
    sec_primary_counts = {}  # seq_sec → {seq_pri: count}
    placeholder_layout_count = 0
    for entry in emr_data["layouts"]:
        if (entry.get("primary_tileset")   == PLACEHOLDER_PRIMARY and
            entry.get("secondary_tileset") == PLACEHOLDER_SECONDARY):
            placeholder_layout_count += 1
            layout_id = entry["id"]
            seq_entry = seq_by_id.get(layout_id)
            if seq_entry:
                pri = seq_entry["primary_tileset"]
                sec = seq_entry["secondary_tileset"]
                needed_pairs.setdefault(pri, set()).add(sec)
                sec_primary_counts.setdefault(sec, {})
                sec_primary_counts[sec][pri] = sec_primary_counts[sec].get(pri, 0) + 1

    total_layouts = sum(len(v) for v in needed_pairs.values())
    print(f"  Found {placeholder_layout_count} Johto layouts needing tileset updates")
    print(f"  Unique secondary pairings: {total_layouts}")
    for pri, secs in sorted(needed_pairs.items()):
        print(f"    {pri}: {len(secs)} secondary tilesets")

    # ── Process primary tilesets ──
    print("\n[2] Processing primary tilesets...")
    for seq_cname, (seq_dir, emr_dir, emr_cname, needs_processing) in PRIMARIES.items():
        if seq_cname not in needed_pairs and seq_cname not in [
            "gTileset_Kanto_General", "gTileset_Kanto_Building"
        ]:
            continue
        print(f"\n  {seq_cname} → {emr_cname}")
        if needs_processing:
            process_primary(seq_cname, seq_dir, emr_dir, emr_cname)
        else:
            print(f"  [SKIP] {emr_dir} already has data from pokeorigin")

    # ── Process secondary tilesets ──
    print("\n[3] Processing secondary tilesets...")

    # Build reverse map: seq_sec_cname → dominant primary seq_dir by frequency.
    # Some secondaries are reused with multiple primaries; choosing the most-used
    # pairing avoids accidental first-seen mismatches.
    sec_to_primary_seq_dir = {}
    for seq_sec_cname, pri_counts in sec_primary_counts.items():
        ranked = sorted(pri_counts.items(), key=lambda kv: (-kv[1], kv[0]))
        best_pri_cname, best_count = ranked[0]
        pri_info = PRIMARIES.get(best_pri_cname)
        if pri_info is None:
            continue
        sec_to_primary_seq_dir[seq_sec_cname] = pri_info[0]
        if len(ranked) > 1:
            detail = ", ".join(f"{pri}={count}" for pri, count in ranked)
            print(f"  [AMBIGUOUS] {seq_sec_cname}: using {best_pri_cname} ({best_count}); choices: {detail}")

    for seq_cname, primary_seq_dir in sec_to_primary_seq_dir.items():
        info = SECONDARIES.get(seq_cname)
        if info is None:
            print(f"\n  [UNMAPPED] {seq_cname} — add to SECONDARIES dict")
            continue
        seq_dir, emr_dir, emr_cname, already_registered = info

        # Skip data processing if data already exists OR if already registered
        # (registered implies data was imported from pokeorigin)
        if already_registered:
            emr_sec_dir = EMR_SECONDARY_DIR / emr_dir
            if (emr_sec_dir / "tiles.png").exists():
                print(f"  [SKIP] {emr_dir} already has data")
                continue

        print(f"\n  {seq_cname} → {emr_cname} (primary: {primary_seq_dir})")
        process_secondary(seq_cname, seq_dir, emr_dir, emr_cname, primary_seq_dir)

    # ── Build name maps for layouts.json update ──
    print("\n[4] Building name maps...")
    primary_emr_map = {
        seq_cname: info[2]
        for seq_cname, info in PRIMARIES.items()
    }
    secondary_emr_map = {
        seq_cname: info[2]
        for seq_cname, info in SECONDARIES.items()
    }

    # ── Update layouts.json ──
    print("\n[5] Updating layouts.json...")
    update_layouts_json(seq_by_id, primary_emr_map, secondary_emr_map)

    # ── Register new tilesets in graphics.h and headers.h ──
    print("\n[6] Registering new tilesets in graphics.h and headers.h...")

    new_graphics_primary   = []
    new_graphics_secondary = []
    new_headers_primary    = []
    new_headers_secondary  = []

    for seq_cname, (seq_dir, emr_dir, emr_cname, needs_processing) in PRIMARIES.items():
        if seq_cname not in needed_pairs:
            continue  # Not needed for our layouts
        if is_registered_in_graphics(emr_cname):
            print(f"  [SKIP] {emr_cname} already in graphics.h")
            continue
        new_graphics_primary.append(graphics_primary_block(emr_cname, emr_dir))
        print(f"  [GRAPHICS] Adding primary {emr_cname}")

    for seq_cname, (seq_dir, emr_dir, emr_cname, needs_processing) in PRIMARIES.items():
        if seq_cname not in needed_pairs:
            continue
        if is_registered_in_headers(emr_cname):
            print(f"  [SKIP] {emr_cname} already in headers.h")
            continue
        new_headers_primary.append(headers_primary_block(emr_cname))
        print(f"  [HEADERS] Adding primary {emr_cname}")

    # Collect unique secondary cnames needed
    seen_sec = set()
    for seq_cname, primary_seq_dir in sec_to_primary_seq_dir.items():
        info = SECONDARIES.get(seq_cname)
        if info is None:
            continue
        seq_dir, emr_dir, emr_cname, already_registered = info
        if emr_cname in seen_sec:
            continue
        seen_sec.add(emr_cname)

        if not is_registered_in_graphics(emr_cname):
            new_graphics_secondary.append(graphics_secondary_block(emr_cname, emr_dir))
            print(f"  [GRAPHICS] Adding secondary {emr_cname}")
        if not is_registered_in_headers(emr_cname):
            new_headers_secondary.append(headers_secondary_block(emr_cname))
            print(f"  [HEADERS] Adding secondary {emr_cname}")

    if new_graphics_primary or new_graphics_secondary:
        append_to_graphics(new_graphics_primary + new_graphics_secondary)
    if new_headers_primary or new_headers_secondary:
        append_to_headers(new_headers_primary, new_headers_secondary)

    print("\n" + "=" * 60)
    print("Import complete!")
    print("Next steps:")
    print("  1. Review the appended sections in graphics.h and headers.h")
    print("  2. Generate tiles.4bpp.lz from tiles.png for each tileset")
    print("     (Porymap will render from PNG directly)")
    print("  3. Open Porymap to verify maps load correctly")
    print("=" * 60)

if __name__ == "__main__":
    main()
