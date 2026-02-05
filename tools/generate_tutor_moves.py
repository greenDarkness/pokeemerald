import re
from collections import OrderedDict

# Path to level-up learnsets and pointer table
LEARNSETS_PATH = "src/data/pokemon/level_up_learnsets.h"
POINTERS_PATH = "src/data/pokemon/level_up_learnset_pointers.h"
OUTPUT_PATH = "src/data/pokemon/tutor_moves.h"

# Regex to extract arrays and moves
array_re = re.compile(r'static const u16 (s\w+LevelUpLearnset)\[\] = {([^}]*)}', re.MULTILINE | re.DOTALL)
move_re = re.compile(r'LEVEL_UP_MOVE\([^,]+, ([A-Z0-9_]+)\)')

# Read learnsets
with open(LEARNSETS_PATH, encoding='utf-8') as f:
    learnsets_text = f.read()

species_moves = {}
for array_match in array_re.finditer(learnsets_text):
    array_name = array_match.group(1)
    array_body = array_match.group(2)
    moves = OrderedDict()
    for move_match in move_re.finditer(array_body):
        move = move_match.group(1)
        moves[move] = None
    species_moves[array_name] = list(moves.keys())

# Read pointer table
with open(POINTERS_PATH, encoding='utf-8') as f:
    pointers_text = f.read()

pointer_re = re.compile(r'\[([A-Z0-9_]+)\] = (s\w+LevelUpLearnset)')
species_order = []
species_to_array = {}
for ptr_match in pointer_re.finditer(pointers_text):
    species = ptr_match.group(1)
    array_name = ptr_match.group(2)
    species_order.append(species)
    species_to_array[species] = array_name

# Write output header
with open(OUTPUT_PATH, 'w', encoding='utf-8') as out:
    out.write('#ifndef GUARD_TUTOR_MOVES_H\n#define GUARD_TUTOR_MOVES_H\n\n')
    out.write('#include "constants/species.h"\n#include "constants/moves.h"\n\n')
    # Write arrays
    for array_name, moves in species_moves.items():
        out.write(f'static const u16 {array_name.replace("LevelUpLearnset", "TutorMoves")}[] = {{\n')
        for move in moves:
            out.write(f'    {move},\n')
        out.write('    MOVE_NONE\n};\n\n')
    # Write pointer array
    out.write('const u16 *const tutorMoves[NUM_SPECIES] = {\n')
    for species in species_order:
        arr = species_to_array[species].replace("LevelUpLearnset", "TutorMoves")
        out.write(f'    [{species}] = {arr},\n')
    out.write('};\n\n#endif // GUARD_TUTOR_MOVES_H\n')
