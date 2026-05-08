import json
from pathlib import Path

ROOT = Path('/mnt/c/Users/drfor/Documents/Porymap/Decomps/pokeemerald')

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

mg = json.loads((ROOT / 'data/maps/map_groups.json').read_text(encoding='utf-8'))
johto = set(mg.get('gMapGroup_Johto', []))

missing_group = [m for m in MAPS if m not in johto]
missing_files = [m for m in MAPS if not (ROOT / f'data/maps/{m}/map.json').exists()]

print(f'Expected maps: {len(MAPS)}')
print(f'Missing from gMapGroup_Johto: {len(missing_group)}')
if missing_group:
    print('  ' + ', '.join(missing_group))
print(f'Missing map.json files: {len(missing_files)}')
if missing_files:
    print('  ' + ', '.join(missing_files))
