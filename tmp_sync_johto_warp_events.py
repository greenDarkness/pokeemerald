#!/usr/bin/env python3
import json
from pathlib import Path


ROOT_DST = Path("C:/Users/drfor/Documents/Porymap/Decomps/pokeemerald")
ROOT_SRC = Path("C:/Users/drfor/Documents/Porymap/Decomps/pokesequel-master")


def load_json(path: Path):
    return json.loads(path.read_text(encoding="utf-8-sig"))


def write_json(path: Path, obj):
    path.write_text(json.dumps(obj, indent=2) + "\n", encoding="utf-8", newline="\n")


def main():
    map_groups_path = ROOT_DST / "data/maps/map_groups.json"
    dst_groups = load_json(map_groups_path)

    # Build valid destination map constants from local map.json IDs.
    valid_dest_maps = {"MAP_DYNAMIC"}
    for map_json in (ROOT_DST / "data/maps").glob("*/map.json"):
        try:
            map_obj = load_json(map_json)
            map_id = map_obj.get("id")
            if isinstance(map_id, str) and map_id.startswith("MAP_"):
                valid_dest_maps.add(map_id)
        except Exception:
            pass

    johto_maps = dst_groups.get("gMapGroup_Johto", [])
    if not johto_maps:
        print("No maps in gMapGroup_Johto; nothing to sync.")
        return

    updated = 0
    unchanged = 0
    filtered = 0
    missing_source = []
    missing_dest = []

    for map_name in johto_maps:
        src_map_json = ROOT_SRC / "data/maps" / map_name / "map.json"
        dst_map_json = ROOT_DST / "data/maps" / map_name / "map.json"

        if not src_map_json.exists():
            missing_source.append(map_name)
            continue
        if not dst_map_json.exists():
            missing_dest.append(map_name)
            continue

        src_obj = load_json(src_map_json)
        dst_obj = load_json(dst_map_json)

        src_warps = src_obj.get("warp_events", [])
        sanitized_warps = []
        for warp in src_warps:
            dest_map = warp.get("dest_map")
            if dest_map in valid_dest_maps:
                sanitized_warps.append(warp)
            else:
                filtered += 1
        dst_warps = dst_obj.get("warp_events", [])

        if dst_warps == sanitized_warps:
            unchanged += 1
            continue

        dst_obj["warp_events"] = sanitized_warps
        write_json(dst_map_json, dst_obj)
        updated += 1

    print("=== Johto Warp Sync Summary ===")
    print(f"Johto maps scanned: {len(johto_maps)}")
    print(f"Updated warp_events: {updated}")
    print(f"Already matching: {unchanged}")
    print(f"Filtered invalid destination warps: {filtered}")
    print(f"Missing source map.json: {len(missing_source)}")
    if missing_source:
        print("  " + ", ".join(sorted(missing_source)))
    print(f"Missing destination map.json: {len(missing_dest)}")
    if missing_dest:
        print("  " + ", ".join(sorted(missing_dest)))


if __name__ == "__main__":
    main()
