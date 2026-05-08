#!/usr/bin/env python3
"""
Step 1: Find all Johto maps in pokesequel that DON'T exist in pokeemerald.
"""

import os, json

EMERALD = '/mnt/c/Users/drfor/Documents/Porymap/Decomps/pokeemerald'
SEQUEL  = '/mnt/c/Users/drfor/Documents/Porymap/Decomps/pokesequel-master'

# Get all Johto maps from pokesequel
mg_sequel = json.load(open(os.path.join(SEQUEL, 'data/maps/map_groups.json')))

johto_maps = set()

# All Johto indoor groups
johto_indoor_groups = [
    'gMapGroup_IndoorNewBark',
    'gMapGroup_IndoorCherrygrove',
    'gMapGroup_IndoorViolet',
    'gMapGroup_IndoorAzalea',
    'gMapGroup_IndoorGoldenrod',
    'gMapGroup_IndoorEcruteak',
    'gMapGroup_IndoorOlivine',
    'gMapGroup_IndoorCianwood',
    'gMapGroup_IndoorMahogany',
    'gMapGroup_IndoorBlackthorn'
]

for group in johto_indoor_groups:
    if group in mg_sequel:
        johto_maps.update(mg_sequel[group])

# Johto routes group
if 'gMapGroup_IndoorJohtoRoutes' in mg_sequel:
    johto_maps.update(mg_sequel['gMapGroup_IndoorJohtoRoutes'])

# Johto outdoor (towns and routes up to postgame)
johto_outdoor_set = {
    'NewBarkTown', 'CherrygroveCity', 'VioletCity', 'AzaleaTown',
    'GoldenrodCity', 'EcruteakCity', 'OlivineCity', 'CianwoodCity',
    'SafariZoneGate', 'Mahoganytown', 'BlackthornCity',
    'Route29', 'Route30', 'Route31', 'Route32', 'Route33',
    'Route34', 'Route35', 'Route36', 'Route37', 'Route38',
    'Route39', 'Route40', 'Route41', 'Route42', 'Route43',
    'Route44', 'Route45', 'Route46', 'Route47', 'Route48'
}

for name in mg_sequel.get('gMapGroup_TownsAndRoutes', []):
    if name in johto_outdoor_set:
        johto_maps.add(name)

print(f"Total Johto maps in pokesequel: {len(johto_maps)}\n")

# Get all existing folders in pokeemerald
emerald_maps_dir = os.path.join(EMERALD, 'data', 'maps')
existing_folders = set(os.listdir(emerald_maps_dir))

print(f"Existing folders in pokeemerald: {len(existing_folders)}\n")

# Find Johto maps that DON'T have folders in pokeemerald
missing_maps = johto_maps - existing_folders

print(f"Johto maps MISSING from pokeemerald (need to create): {len(missing_maps)}\n")

if missing_maps:
    print("Maps to create:")
    for name in sorted(missing_maps):
        print(f"  {name}")
else:
    print("No missing Johto maps - all exist as folders in pokeemerald.")
