"""
Import object_events from pokesequel Johto maps into pokeemerald.

Rules:
  - Only process maps matching Johto name prefixes
  - Skip OBJ_EVENT_GFX_MON_BASE+SPECIES_* (following pokemon system not in pokeemerald)
  - Remap a handful of GFX IDs that exist in pokesequel but not pokeemerald
  - Set script="NULL" (pokesequel scripts don't exist in pokeemerald yet)
  - Set flag="0" (pokesequel flags don't exist in pokeemerald yet)
  - Set trainer_type="TRAINER_TYPE_NONE", trainer_sight_or_berry_tree_id="0"
  - REPLACE pokeemerald's object_events with the filtered pokesequel ones
"""

import json
import os

POKESEQUEL = "/mnt/c/Users/drfor/Documents/Porymap/Decomps/pokesequel-master"
POKEEMERALD = "/mnt/c/Users/drfor/Documents/Porymap/Decomps/pokeemerald"

# Pokesequel GFX IDs not in pokeemerald -> remap or skip (None = skip)
GFX_REMAP = {
    "OBJ_EVENT_GFX_SCIENTIST_M":    "OBJ_EVENT_GFX_SCIENTIST",
    "OBJ_EVENT_GFX_SCIENTIST_F":    "OBJ_EVENT_GFX_SCIENTIST_2",
    "OBJ_EVENT_GFX_ATTENDANT":      "OBJ_EVENT_GFX_ATTENDANT_M",
    "OBJ_EVENT_GFX_JANINE":         "OBJ_EVENT_GFX_WOMAN",
    "OBJ_EVENT_GFX_UNUSED_WOMAN_4": "OBJ_EVENT_GFX_WOMAN",
    "OBJ_EVENT_GFX_SURGE":          "OBJ_EVENT_GFX_LT_SURGE",
    "OBJ_EVENT_GFX_LEGENDARY_SHADOW": None,  # no equivalent, skip
    "OBJ_EVENT_GFX_BIG_SNORLAX":   "OBJ_EVENT_GFX_SNORLAX",
}

# Map name prefixes that identify Johto maps
JOHTO_PREFIXES = (
    "NewBarkTown",
    "CherrygroveCity",
    "VioletCity",
    "AzaleaTown",
    "GoldenrodCity",
    "EcruteakCity",
    "OlivineCity",
    "CianwoodCity",
    "Mahoganytown",
    "MahoganyTown",
    "BlackthornCity",
    "IlexForest",
    "DarkCave",
    "SproutTower",
    "RuinsOfAlph",
    "UnionCave",
    "SlowpokeWell",
    "BurnedTower",
    "TinTower",
    "WhirlIslands",
    "MtMortar",
    "LakeOfRage",
    "LakeOfRageLowTide",
    "MtSilver",
    "NationalPark",
    "IcePath",
    "DragonsDen",
    "TohjoFalls",
    "BellchimeTrail",
    "IndigoPlateau",
    "Gate_AzaleaTown",
    "Gate_EcruteakCity",
    "Gate_GoldenrodCity",
    "Gate_IlexForest",
    "Gate_MahoganyTown",
    "Gate_NationalPark",
    "Gate_Route29",
    "Gate_Route31",
    "Gate_Route40",
    "Gate_Route43",
    "Gate_RuinsOfAlph",
    "SSAqua",
    "Route29",
    "Route30",
    "Route31",
    "Route32",
    "Route33",
    "Route34",
    "Route35",
    "Route36",
    "Route37",
    "Route38",
    "Route39",
    "Route40",
    "Route41",
    "Route42",
    "Route43",
    "Route44",
    "Route45",
    "Route46",
    "Route47",
    "Route48",
    # Kanto routes added in pokeemerald as "new" Johto-related content
    "Route26",
    "Route27",
    "Route28",
)


def is_johto_map(name):
    for prefix in JOHTO_PREFIXES:
        if name == prefix or name.startswith(prefix + "_"):
            return True
    return False


def filter_object_events(events):
    result = []
    remapped = []
    for ev in events:
        gfx = ev.get("graphics_id", "")

        # Skip MON_BASE following-pokemon entries
        if "OBJ_EVENT_GFX_MON_BASE" in gfx or ("SPECIES_" in gfx and "+" in gfx):
            continue

        ev = dict(ev)

        # Remap unrecognised GFX IDs
        if gfx in GFX_REMAP:
            mapped = GFX_REMAP[gfx]
            if mapped is None:
                continue
            remapped.append((gfx, mapped))
            ev["graphics_id"] = mapped

        # Null out pokesequel-specific scripts / flags / trainer data
        ev["script"] = "NULL"
        ev["flag"] = "0"
        ev["trainer_type"] = "TRAINER_TYPE_NONE"
        ev["trainer_sight_or_berry_tree_id"] = "0"

        result.append(ev)
    return result, remapped


def main():
    emerald_maps = os.path.join(POKEEMERALD, "data/maps")
    sequel_maps  = os.path.join(POKESEQUEL,  "data/maps")

    updated = 0
    skipped_no_sequel = 0
    skipped_no_json = 0

    for map_name in sorted(os.listdir(emerald_maps)):
        if not is_johto_map(map_name):
            continue

        sequel_dir  = os.path.join(sequel_maps,  map_name)
        emerald_dir = os.path.join(emerald_maps, map_name)

        if not os.path.isdir(sequel_dir):
            skipped_no_sequel += 1
            continue

        seq_json  = os.path.join(sequel_dir,  "map.json")
        em_json   = os.path.join(emerald_dir, "map.json")

        if not os.path.exists(seq_json) or not os.path.exists(em_json):
            skipped_no_json += 1
            continue

        with open(seq_json, "r", encoding="utf-8") as f:
            seq_data = json.load(f)
        with open(em_json, "r", encoding="utf-8") as f:
            em_data = json.load(f)

        filtered, remapped = filter_object_events(seq_data.get("object_events", []))
        em_data["object_events"] = filtered

        with open(em_json, "w", encoding="utf-8", newline="\n") as f:
            json.dump(em_data, f, indent=2, ensure_ascii=False)
            f.write("\n")

        updated += 1
        remap_str = f" [remapped: {', '.join(f'{a}->{b}' for a,b in remapped)}]" if remapped else ""
        print(f"  OK  {map_name}: {len(filtered)} objects{remap_str}")

    print(f"\nDone: {updated} maps updated.")
    print(f"      {skipped_no_sequel} Johto maps not found in pokesequel (skipped).")
    print(f"      {skipped_no_json} maps missing map.json (skipped).")


if __name__ == "__main__":
    main()
