#!/usr/bin/env python3
"""
Fix Johto map.json files: substitute music/mapsec values that don't exist
in pokeemerald with valid fallbacks.
"""

import os, json, re

EMERALD = '/mnt/c/Users/drfor/Documents/Porymap/Decomps/pokeemerald'

def write_utf8(path, text):
    with open(path, 'w', encoding='utf-8', newline='\n') as f:
        f.write(text)

def load_json(path):
    with open(path, 'r', encoding='utf-8-sig') as f:
        return json.load(f)

# Collect valid constants from pokeemerald
songs_path   = os.path.join(EMERALD, 'include/constants/songs.h')
mapsec_path  = os.path.join(EMERALD, 'include/constants/region_map_sections.h')
weather_path = os.path.join(EMERALD, 'include/constants/weather.h')

with open(songs_path, encoding='utf-8') as f:
    songs_txt = f.read()
valid_music = set(re.findall(r'#define\s+(MUS_\w+)', songs_txt))

with open(mapsec_path, encoding='utf-8') as f:
    mapsec_txt = f.read()
valid_mapsec = set(re.findall(r'(MAPSEC_\w+)', mapsec_txt))

# Get valid map_type, battle_scene, weather from a known working map
working = load_json(os.path.join(EMERALD, 'data/maps/PalletTown/map.json'))
fallback_map_type    = working.get('map_type', 'MAP_TYPE_TOWN')
fallback_battle_scene = working.get('battle_scene', 'MAP_BATTLE_SCENE_NORMAL')
fallback_weather     = working.get('weather', 'WEATHER_NONE')

# Also check map_types and battle_scenes are valid
map_type_path = os.path.join(EMERALD, 'include/constants/map_types.h')
if os.path.exists(map_type_path):
    with open(map_type_path, encoding='utf-8') as f:
        map_type_txt = f.read()
    valid_map_types = set(re.findall(r'#define\s+(MAP_TYPE_\w+)', map_type_txt))
else:
    valid_map_types = None

battle_scene_path = os.path.join(EMERALD, 'include/constants/battle_scene.h')
if os.path.exists(battle_scene_path):
    with open(battle_scene_path, encoding='utf-8') as f:
        battle_txt = f.read()
    valid_battle_scenes = set(re.findall(r'#define\s+(MAP_BATTLE_SCENE_\w+)', battle_txt))
else:
    valid_battle_scenes = None

print(f"Valid music constants: {len(valid_music)}")
print(f"Valid mapsec constants: {len(valid_mapsec)}")

# Johto maps from gMapGroup_Johto in pokeemerald
mg = load_json(os.path.join(EMERALD, 'data/maps/map_groups.json'))
johto_maps = mg.get('gMapGroup_Johto', [])
print(f"Johto maps to fix: {len(johto_maps)}\n")

fixed = 0
for map_name in johto_maps:
    map_path = os.path.join(EMERALD, 'data/maps', map_name, 'map.json')
    if not os.path.exists(map_path):
        continue

    map_obj = load_json(map_path)
    changed = False

    if map_obj.get('music') not in valid_music:
        print(f"  {map_name}: music '{map_obj.get('music')}' -> MUS_DUMMY")
        map_obj['music'] = 'MUS_DUMMY'
        changed = True

    if map_obj.get('region_map_section') not in valid_mapsec:
        print(f"  {map_name}: mapsec '{map_obj.get('region_map_section')}' -> MAPSEC_NONE")
        map_obj['region_map_section'] = 'MAPSEC_NONE'
        changed = True

    if valid_map_types and map_obj.get('map_type') not in valid_map_types:
        print(f"  {map_name}: map_type '{map_obj.get('map_type')}' -> {fallback_map_type}")
        map_obj['map_type'] = fallback_map_type
        changed = True

    if valid_battle_scenes and map_obj.get('battle_scene') not in valid_battle_scenes:
        print(f"  {map_name}: battle_scene '{map_obj.get('battle_scene')}' -> {fallback_battle_scene}")
        map_obj['battle_scene'] = fallback_battle_scene
        changed = True

    # Add missing floor_number field if absent
    if 'floor_number' not in map_obj:
        map_obj['floor_number'] = 0
        changed = True

    if changed:
        write_utf8(map_path, json.dumps(map_obj, indent=2))
        fixed += 1

print(f"\nFixed {fixed} map.json files.")
