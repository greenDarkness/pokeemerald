import json
import os
import shutil
from pathlib import Path

ROOT_DST = Path('/mnt/c/Users/drfor/Documents/Porymap/Decomps/pokeemerald')
ROOT_SRC = Path('/mnt/c/Users/drfor/Documents/Porymap/Decomps/pokesequel-master')

# User-approved candidate list after pruning placeholders and VictoryRoadKanto maps.
MAPS = [
    'Route26', 'Route26North', 'Route27', 'Route28', 'Route26_House1', 'Route26_House2',
    'DarkCave_SouthSide', 'DarkCave_NorthSide', 'SproutTower_1F', 'SproutTower_2F', 'SproutTower_3F',
    'RuinsOfAlph_Outside', 'RuinsOfAlph_B1F', 'RuinsOfAlph_PuzzleAndRewardChambers',
    'RuinsOfAlph_WordsRoom1', 'RuinsOfAlph_WordsRoom2', 'RuinsOfAlph_WordsRoom3', 'RuinsOfAlph_WordsRoom4',
    'UnionCave_1F', 'UnionCave_B1F', 'UnionCave_B2F', 'SlowpokeWell_B1F', 'SlowpokeWell_B2F',
    'IlexForest', 'NationalPark_Normal', 'NationalPark_BugContest', 'BurnedTower_1F', 'BurnedTower_B1F',
    'CliffEdgeGate', 'CliffEdgeCave', 'MtMortar_1F_South', 'MtMortar_1F_North', 'MtMortar_2F', 'MtMortar_B1F',
    'LakeOfRage', 'LakeOfRageLowTide', 'IcePath_1F', 'IcePath_B1F', 'IcePath_B2F', 'IcePath_B3F', 'IcePath_B4F',
    'DragonsDen_Entrance', 'DragonsDen_Cavern', 'DragonsDen_Shrine', 'WhirlIslands_1F', 'WhirlIslands_B1F',
    'WhirlIslands_B1F_Inner', 'WhirlIslands_B2F', 'WhirlIslands_B3F', 'WhirlIslands_Descent',
    'WhirlIslands_LugiaChamber', 'TinTower_1F', 'TinTower_2F', 'TinTower_3F', 'TinTower_4F', 'TinTower_5F',
    'TinTower_6F', 'TinTower_7F', 'TinTower_8F', 'TinTower_9F', 'TinTower_RoofDay', 'TinTower_RoofNight',
    'TohjoFalls_Cavern', 'TohjoFalls_GiovanniRoom', 'MtSilver_Outside', 'MtSilver_1F_ItemRoom',
    'MtSilver_1F_WaterfallRoom', 'MtSilver_1F_MoltresRoom', 'MtSilver_MountainSide', 'MtSilver_2F',
    'MtSilver_3F', 'MtSilver_Snow', 'MtSilver_SummitDay', 'MtSilver_SummitNight', 'EmbeddedTower',
    'SSAqua_1F', 'SSAqua_B1F', 'SSAqua_CaptainsRoom', 'SSAqua_PlayersRoom', 'SSAqua_RoomNW', 'SSAqua_RoomNE',
    'SSAqua_RoomNNE', 'SSAqua_RoomSSW', 'SSAqua_RoomSSE', 'SSAqua_RoomSE', 'SSAqua_RoomSW',
    'SafariZone_Top_Left', 'SafariZone_Low_Mid', 'SafariZone_Enterance', 'SafariZone_Low_Left',
    'SafariZone_Low_Right', 'SafariZone_Top_Mid', 'SafariZone_Top_Right', 'SafariZone1', 'SafariZone2',
    'SafariZone3', 'SafariZoneIndoor', 'MtMoon_Cave', 'MtMoon_Outside', 'MtMoon_Shop'
]

MAPS = sorted(set(MAPS))

maps_json_path = ROOT_DST / 'data/maps/map_groups.json'
layouts_json_dst_path = ROOT_DST / 'data/layouts/layouts.json'
layouts_json_src_path = ROOT_SRC / 'data/layouts/layouts.json'
event_scripts_path = ROOT_DST / 'data/event_scripts.s'

maps_data = json.loads(maps_json_path.read_text(encoding='utf-8'))
layouts_dst = json.loads(layouts_json_dst_path.read_text(encoding='utf-8'))
layouts_src = json.loads(layouts_json_src_path.read_text(encoding='utf-8'))

johto_group = maps_data.setdefault('gMapGroup_Johto', [])
existing_map_groups = set()
for k, v in maps_data.items():
    if k == 'group_order':
        continue
    for name in v:
        existing_map_groups.add(name)

src_layout_by_id = {l['id']: l for l in layouts_src['layouts']}
dst_layout_by_id = {l['id']: l for l in layouts_dst['layouts']}

with event_scripts_path.open('r', encoding='utf-8') as f:
    event_lines = f.read().splitlines()
include_set = set()
for line in event_lines:
    line = line.strip()
    if line.startswith('.include "data/maps/') and line.endswith('/scripts.inc"'):
        include_set.add(line)

added_to_group = 0
added_event_includes = 0
created_map_dirs = 0
copied_map_json = 0
created_scripts_stub = 0
layouts_added = 0
copied_layout_bins = 0
missing_source_maps = []
missing_source_layouts = []

for map_name in MAPS:
    src_map_dir = ROOT_SRC / f'data/maps/{map_name}'
    dst_map_dir = ROOT_DST / f'data/maps/{map_name}'
    src_map_json = src_map_dir / 'map.json'
    dst_map_json = dst_map_dir / 'map.json'

    if not src_map_json.exists():
        missing_source_maps.append(map_name)
        continue

    if not dst_map_dir.exists():
        dst_map_dir.mkdir(parents=True, exist_ok=True)
        created_map_dirs += 1

    map_data = json.loads(src_map_json.read_text(encoding='utf-8'))

    # Map-only normalization used in this repo workflow.
    map_data['music'] = 'MUS_DUMMY'
    map_data['region_map_section'] = 'MAPSEC_NONE'
    map_data['connections'] = None
    map_data['object_events'] = []
    map_data['warp_events'] = []
    map_data['coord_events'] = []
    map_data['bg_events'] = []
    map_data['floor_number'] = 0

    dst_map_json.write_text(json.dumps(map_data, indent=2) + '\n', encoding='utf-8', newline='\n')
    copied_map_json += 1

    scripts_inc_path = dst_map_dir / 'scripts.inc'
    scripts_inc_path.write_text(f'{map_name}_MapScripts::\n\t.byte 0\n', encoding='utf-8', newline='\n')
    created_scripts_stub += 1

    include_line = f'.include "data/maps/{map_name}/scripts.inc"'
    if include_line not in include_set:
        event_lines.append(f'        {include_line}')
        include_set.add(include_line)
        added_event_includes += 1

    if map_name not in johto_group:
        johto_group.append(map_name)
        added_to_group += 1

    layout_id = map_data.get('layout')
    if layout_id and layout_id not in dst_layout_by_id:
        src_layout = src_layout_by_id.get(layout_id)
        if not src_layout:
            missing_source_layouts.append((map_name, layout_id))
        else:
            layout_entry = dict(src_layout)
            # Safe placeholders so Porymap accepts primary/secondary roles.
            layout_entry['primary_tileset'] = 'gTileset_General'
            layout_entry['secondary_tileset'] = 'gTileset_Petalburg'

            layouts_dst['layouts'].append(layout_entry)
            dst_layout_by_id[layout_id] = layout_entry
            layouts_added += 1

            for key in ('border_filepath', 'blockdata_filepath'):
                rel = layout_entry[key]
                src_file = ROOT_SRC / rel
                dst_file = ROOT_DST / rel
                if src_file.exists():
                    dst_file.parent.mkdir(parents=True, exist_ok=True)
                    shutil.copy2(src_file, dst_file)
                    copied_layout_bins += 1

# Keep map list order stable.
johto_group.sort()

maps_json_path.write_text(json.dumps(maps_data, indent=2) + '\n', encoding='utf-8', newline='\n')
layouts_json_dst_path.write_text(json.dumps(layouts_dst, indent=2) + '\n', encoding='utf-8', newline='\n')
event_scripts_path.write_text('\n'.join(event_lines) + '\n', encoding='utf-8', newline='\n')

print('=== Import Summary ===')
print(f'Candidate maps: {len(MAPS)}')
print(f'Missing source map.json: {len(missing_source_maps)}')
if missing_source_maps:
    print('  ' + ', '.join(sorted(missing_source_maps)))
print(f'Map dirs created: {created_map_dirs}')
print(f'map.json written: {copied_map_json}')
print(f'scripts.inc stubs written: {created_scripts_stub}')
print(f'Added to gMapGroup_Johto: {added_to_group}')
print(f'Added includes to event_scripts.s: {added_event_includes}')
print(f'Layouts added: {layouts_added}')
print(f'Layout files copied (border/map bins): {copied_layout_bins}')
print(f'Missing source layouts: {len(missing_source_layouts)}')
if missing_source_layouts:
    for m, lid in missing_source_layouts[:20]:
        print(f'  {m}: {lid}')
