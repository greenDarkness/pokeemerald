#!/usr/bin/env python3
"""
Update Pokemon Emerald learnsets to use HeartGold/SoulSilver level timings.
This script modifies the level at which Pokemon learn moves, using HGSS as a reference.
Only moves that exist in BOTH games are modified.

Usage:
    python update_learnsets_to_hgss.py --compare   # Compare and show differences
    python update_learnsets_to_hgss.py --apply     # Apply the changes
    python update_learnsets_to_hgss.py --download  # Download fresh data from veekun
"""

import re
import os
import sys
import csv
import json
import urllib.request
from typing import Dict, List, Tuple, Optional, Set

# URLs for CSV data from veekun/pokedex
POKEMON_MOVES_URL = "https://raw.githubusercontent.com/veekun/pokedex/master/pokedex/data/csv/pokemon_moves.csv"
POKEMON_SPECIES_URL = "https://raw.githubusercontent.com/veekun/pokedex/master/pokedex/data/csv/pokemon_species.csv"
MOVES_URL = "https://raw.githubusercontent.com/veekun/pokedex/master/pokedex/data/csv/moves.csv"

# Version group IDs in veekun data
# 3 = Ruby/Sapphire, 4 = FireRed/LeafGreen, 5 = Emerald
# 8 = Diamond/Pearl, 9 = Platinum, 10 = HeartGold/SoulSilver
EMERALD_VERSION_GROUP = 5
HGSS_VERSION_GROUP = 10

# Method ID for level-up moves
LEVEL_UP_METHOD = 1

# Maximum Pokemon ID for Gen 3 (Deoxys = 386)
MAX_GEN3_POKEMON = 386

# Cache file for downloaded data
CACHE_FILE = os.path.join(os.path.dirname(__file__), "hgss_learnsets_cache.json")

# Move name mappings: veekun identifier -> Emerald MOVE_ constant
MOVE_NAME_FIXES = {
    "POISON_GAS": "POISONGAS",
    "SELF_DESTRUCT": "SELFDESTRUCT",
    "SOFT_BOILED": "SOFTBOILED",
    "DOUBLE_SLAP": "DOUBLESLAP",
    "SOLAR_BEAM": "SOLARBEAM",
    "DRAGON_BREATH": "DRAGONBREATH",
    "EXTREME_SPEED": "EXTREMESPEED",
    "ANCIENT_POWER": "ANCIENTPOWER",
    "FEINT_ATTACK": "FAINT_ATTACK",
    "THUNDER_SHOCK": "THUNDERSHOCK",
    "THUNDER_PUNCH": "THUNDERPUNCH",
    "VICE_GRIP": "VICEGRIP",
    "SMELLING_SALTS": "SMELLINGSALT",
    "HI_JUMP_KICK": "HI_JUMP_KICK",
    "DYNAMIC_PUNCH": "DYNAMICPUNCH",
    "LOCK_ON": "LOCK_ON",
    "BUBBLE_BEAM": "BUBBLEBEAM",
    "DOUBLE_KICK": "DOUBLE_KICK",
    "FURY_SWIPES": "FURY_SWIPES",
    "SAND_ATTACK": "SAND_ATTACK",
    "MUD_SLAP": "MUD_SLAP",
    "CONVERSION_2": "CONVERSION_2",
    "METAL_CLAW": "METAL_CLAW",
    "FIRE_PUNCH": "FIRE_PUNCH",
    "ICE_PUNCH": "ICE_PUNCH",
}

# Pokemon name mappings: veekun identifier -> Emerald species name format
POKEMON_NAME_FIXES = {
    "NIDORAN_F": "NIDORAN_F",
    "NIDORAN_M": "NIDORAN_M",
    "MR_MIME": "MR_MIME",
    "FARFETCHD": "FARFETCH_D",
    "HO_OH": "HO_OH",
}


def download_file(url: str) -> str:
    """Download a file from URL and return its content."""
    print(f"Downloading {url.split('/')[-1]}...")
    try:
        with urllib.request.urlopen(url, timeout=60) as response:
            return response.read().decode('utf-8')
    except Exception as e:
        print(f"Error downloading {url}: {e}")
        return ""


def load_pokemon_names(content: str) -> Dict[int, str]:
    """Load Pokemon names from species CSV content."""
    names = {}
    reader = csv.reader(content.splitlines())
    next(reader)  # Skip header
    for row in reader:
        if len(row) >= 2:
            pokemon_id = int(row[0])
            name = row[1].upper().replace('-', '_')
            # Apply name fixes
            if name in POKEMON_NAME_FIXES:
                name = POKEMON_NAME_FIXES[name]
            names[pokemon_id] = name
    return names


def load_move_names(content: str) -> Dict[int, str]:
    """Load move names from moves CSV content."""
    names = {}
    reader = csv.reader(content.splitlines())
    next(reader)  # Skip header
    for row in reader:
        if len(row) >= 2:
            move_id = int(row[0])
            name = row[1].upper().replace('-', '_')
            # Apply name fixes
            if name in MOVE_NAME_FIXES:
                name = MOVE_NAME_FIXES[name]
            names[move_id] = name
    return names


def load_learnsets_from_content(content: str, version_group: int,
                                 pokemon_names: Dict[int, str],
                                 move_names: Dict[int, str]) -> Dict[str, List[Tuple[int, str]]]:
    """Load level-up learnsets from pokemon_moves CSV for a specific version."""
    learnsets: Dict[str, List[Tuple[int, str]]] = {}
    
    reader = csv.reader(content.splitlines())
    next(reader)  # Skip header
    
    for row in reader:
        if len(row) < 5:
            continue
        
        pokemon_id = int(row[0])
        ver_group = int(row[1])
        move_id = int(row[2])
        method = int(row[3])
        level = int(row[4]) if row[4] else 0
        
        # Filter by version group, method, and Gen 1-3 Pokemon
        if ver_group != version_group or method != LEVEL_UP_METHOD:
            continue
        if pokemon_id > MAX_GEN3_POKEMON:
            continue
        
        pokemon_name = pokemon_names.get(pokemon_id)
        move_name = move_names.get(move_id)
        
        if not pokemon_name or not move_name:
            continue
        
        if pokemon_name not in learnsets:
            learnsets[pokemon_name] = []
        learnsets[pokemon_name].append((level, move_name))
    
    # Sort each learnset by level, then by move name
    for pokemon in learnsets:
        learnsets[pokemon].sort(key=lambda x: (x[0], x[1]))
    
    return learnsets


def download_hgss_data() -> Dict[str, List[Tuple[int, str]]]:
    """Download HGSS learnset data from veekun."""
    print("Downloading learnset data from veekun/pokedex...")
    
    species_content = download_file(POKEMON_SPECIES_URL)
    if not species_content:
        return {}
    
    moves_content = download_file(MOVES_URL)
    if not moves_content:
        return {}
    
    pokemon_moves_content = download_file(POKEMON_MOVES_URL)
    if not pokemon_moves_content:
        return {}
    
    pokemon_names = load_pokemon_names(species_content)
    move_names = load_move_names(moves_content)
    
    print(f"Loaded {len(pokemon_names)} Pokemon and {len(move_names)} moves")
    
    hgss_learnsets = load_learnsets_from_content(
        pokemon_moves_content, HGSS_VERSION_GROUP,
        pokemon_names, move_names
    )
    
    print(f"Loaded HGSS learnsets for {len(hgss_learnsets)} Pokemon")
    
    # Save to cache
    cache_data = {pokemon: moves for pokemon, moves in hgss_learnsets.items()}
    with open(CACHE_FILE, 'w') as f:
        json.dump(cache_data, f, indent=2)
    print(f"Saved to cache: {CACHE_FILE}")
    
    return hgss_learnsets


def load_cached_hgss_data() -> Optional[Dict[str, List[Tuple[int, str]]]]:
    """Load cached HGSS data if available."""
    if not os.path.exists(CACHE_FILE):
        return None
    
    try:
        with open(CACHE_FILE, 'r') as f:
            data = json.load(f)
        # Convert lists back to tuples
        return {pokemon: [(level, move) for level, move in moves] 
                for pokemon, moves in data.items()}
    except Exception as e:
        print(f"Error loading cache: {e}")
        return None


def get_hgss_learnsets(force_download: bool = False) -> Dict[str, List[Tuple[int, str]]]:
    """Get HGSS learnsets, downloading if necessary."""
    if not force_download:
        cached = load_cached_hgss_data()
        if cached:
            print(f"Using cached data ({len(cached)} Pokemon)")
            return cached
    
    return download_hgss_data()


def parse_emerald_learnsets(filepath: str) -> Dict[str, List[Tuple[int, str]]]:
    """Parse the Emerald level_up_learnsets.h file."""
    learnsets = {}
    
    with open(filepath, 'r') as f:
        content = f.read()
    
    # Find all learnset arrays
    pattern = r'static const u16 s(\w+)LevelUpLearnset\[\] = \{([^}]+)\}'
    matches = re.findall(pattern, content, re.DOTALL)
    
    for pokemon_name, moves_str in matches:
        moves = []
        # Parse LEVEL_UP_MOVE entries
        move_pattern = r'LEVEL_UP_MOVE\(\s*(\d+),\s*MOVE_(\w+)\)'
        for level, move in re.findall(move_pattern, moves_str):
            moves.append((int(level), move))
        
        # Convert Pokemon name to match veekun format
        name = pokemon_name.upper()
        learnsets[name] = moves
    
    return learnsets


def get_emerald_moves(learnsets: Dict[str, List[Tuple[int, str]]]) -> Set[str]:
    """Get all moves that exist in Emerald's learnsets."""
    moves = set()
    for pokemon, move_list in learnsets.items():
        for _, move in move_list:
            moves.add(move)
    return moves


def compare_learnsets(emerald_learnsets: Dict[str, List[Tuple[int, str]]],
                      hgss_learnsets: Dict[str, List[Tuple[int, str]]],
                      valid_moves: Set[str]) -> Dict[str, List[Tuple[str, int, int]]]:
    """Compare learnsets and return differences."""
    differences = {}
    
    for pokemon in sorted(emerald_learnsets.keys()):
        if pokemon not in hgss_learnsets:
            continue
        
        emerald_moves = emerald_learnsets[pokemon]
        hgss_moves = hgss_learnsets[pokemon]
        
        # Create lookups
        emerald_lookup = {move: level for level, move in emerald_moves}
        hgss_lookup = {move: level for level, move in hgss_moves}
        
        pokemon_diffs = []
        for move, emerald_level in sorted(emerald_lookup.items()):
            if move in hgss_lookup and move in valid_moves:
                hgss_level = hgss_lookup[move]
                if emerald_level != hgss_level:
                    pokemon_diffs.append((move, emerald_level, hgss_level))
        
        if pokemon_diffs:
            differences[pokemon] = pokemon_diffs
    
    return differences


def generate_updated_learnsets(
    original_filepath: str,
    emerald_learnsets: Dict[str, List[Tuple[int, str]]],
    hgss_learnsets: Dict[str, List[Tuple[int, str]]],
    valid_moves: Set[str],
    only_earlier: bool = False
) -> str:
    """Generate the updated learnsets file content.
    
    If only_earlier is True, only apply HGSS levels that are earlier than Emerald.
    Additionally, if HGSS's final move is earlier than Emerald's final move,
    use the HGSS final move level for Emerald's final move too.
    """
    with open(original_filepath, 'r') as f:
        content = f.read()
    
    changes_made = 0
    
    for pokemon in sorted(emerald_learnsets.keys()):
        if pokemon not in hgss_learnsets:
            continue
        
        emerald_moves = emerald_learnsets[pokemon]
        hgss_moves = hgss_learnsets[pokemon]
        
        # Create lookups
        hgss_lookup = {move: level for level, move in hgss_moves}
        emerald_lookup = {move: level for level, move in emerald_moves}
        
        # Find the final move levels in each game
        emerald_final_level = max(level for level, move in emerald_moves) if emerald_moves else 0
        hgss_final_level = max(level for level, move in hgss_moves) if hgss_moves else 0
        
        # Check if any changes needed
        updated_moves = []
        has_changes = False
        
        for level, move in emerald_moves:
            emerald_level = level
            
            # Check if this is Emerald's final move (highest level move)
            is_final_move = (level == emerald_final_level)
            
            if move in hgss_lookup and move in valid_moves:
                hgss_level = hgss_lookup[move]
                
                if only_earlier:
                    # Only use HGSS level if it's earlier (or same)
                    new_level = min(hgss_level, emerald_level)
                else:
                    new_level = hgss_level
                
                if new_level != level:
                    has_changes = True
                updated_moves.append((new_level, move))
            elif is_final_move and hgss_final_level < emerald_final_level:
                # This is Emerald's final move but doesn't exist in HGSS
                # Use HGSS's final move level if it's earlier
                new_level = hgss_final_level
                has_changes = True
                updated_moves.append((new_level, move))
            else:
                updated_moves.append((level, move))
        
        if not has_changes:
            continue
        
        # Sort by level, maintaining original move order for same level
        updated_moves.sort(key=lambda x: x[0])
        
        # Build the replacement pattern
        # Convert POKEMON_NAME back to TitleCase for the array name
        title_name = ''.join(word.title() for word in pokemon.split('_'))
        
        # Match the original learnset array
        pattern = rf'(static const u16 s{title_name}LevelUpLearnset\[\] = \{{)([^}}]+)(\}})'
        
        match = re.search(pattern, content, re.DOTALL)
        if match:
            # Build new moves string
            new_moves_str = "\n"
            for level, move in updated_moves:
                new_moves_str += f"    LEVEL_UP_MOVE({level:2d}, MOVE_{move}),\n"
            new_moves_str += "    LEVEL_UP_END\n"
            
            new_content = match.group(1) + new_moves_str + match.group(3)
            content = content[:match.start()] + new_content + content[match.end():]
            changes_made += 1
    
    print(f"Updated {changes_made} Pokemon learnsets")
    return content


def main():
    """Main function."""
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)
    learnsets_path = os.path.join(project_root, "src", "data", "pokemon", "level_up_learnsets.h")
    
    if not os.path.exists(learnsets_path):
        print(f"Error: Could not find {learnsets_path}")
        return 1
    
    # Parse command line arguments
    force_download = "--download" in sys.argv
    apply_changes = "--apply" in sys.argv
    
    # Load HGSS data
    hgss_learnsets = get_hgss_learnsets(force_download)
    if not hgss_learnsets:
        print("Error: Failed to load HGSS learnset data")
        return 1
    
    # Parse Emerald learnsets
    print(f"Parsing {learnsets_path}...")
    emerald_learnsets = parse_emerald_learnsets(learnsets_path)
    print(f"Found {len(emerald_learnsets)} Emerald learnsets")
    
    # Get valid moves (only moves that exist in Emerald)
    valid_moves = get_emerald_moves(emerald_learnsets)
    print(f"Found {len(valid_moves)} unique moves in Emerald")
    
    # Compare learnsets
    print("\n" + "=" * 70)
    print("EMERALD vs HGSS LEARNSET COMPARISON (Only Earlier Levels Applied)")
    print("=" * 70)
    
    differences = compare_learnsets(emerald_learnsets, hgss_learnsets, valid_moves)
    
    total_changes = 0
    earlier_count = 0
    later_count = 0
    skipped_count = 0
    
    for pokemon in sorted(differences.keys()):
        diffs = differences[pokemon]
        printed_pokemon = False
        for move, em_lvl, hg_lvl in diffs:
            if hg_lvl < em_lvl:
                # This change will be applied
                if not printed_pokemon:
                    print(f"\n{pokemon}:")
                    printed_pokemon = True
                diff = em_lvl - hg_lvl
                print(f"  {move}: Lv{em_lvl} -> Lv{hg_lvl} (earlier, -{diff})")
                total_changes += 1
                earlier_count += 1
            else:
                # This would delay the move, skip it
                skipped_count += 1
                later_count += 1
    
    print("\n" + "=" * 70)
    print(f"SUMMARY: {len(differences)} Pokemon have differences")
    print(f"  {earlier_count} moves will be learned EARLIER (applied)")
    print(f"  {later_count} moves would be later in HGSS (SKIPPED - keeping Emerald level)")
    print("=" * 70)
    
    if apply_changes:
        print("\nApplying changes (only earlier levels)...")
        new_content = generate_updated_learnsets(
            learnsets_path, emerald_learnsets, hgss_learnsets, valid_moves,
            only_earlier=True  # Only apply HGSS levels if they're earlier
        )
        
        # Write backup
        backup_path = learnsets_path + ".backup"
        if not os.path.exists(backup_path):
            with open(learnsets_path, 'r') as f:
                with open(backup_path, 'w') as bf:
                    bf.write(f.read())
            print(f"Backup saved to {backup_path}")
        
        # Write updated file
        with open(learnsets_path, 'w') as f:
            f.write(new_content)
        print(f"Updated {learnsets_path}")
    else:
        print("\nRun with --apply to apply these changes")
    
    return 0


if __name__ == "__main__":
    sys.exit(main())
