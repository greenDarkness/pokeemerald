import os
import subprocess

pal_dir = '/mnt/c/Users/drfor/Documents/Porymap/Decomps/pokeemerald/graphics/object_events/palettes/'
gbagfx = '/mnt/c/Users/drfor/Documents/Porymap/Decomps/pokeemerald/tools/gbagfx/gbagfx'

# Map name -> number of colors it has -> how many to add
palettes = ['sage', 'pryce', 'bugsy', 'karen', 'lance', 'silver', 'will']

for name in palettes:
    pal_path = pal_dir + name + '.pal'
    gbapal_path = pal_dir + name + '.gbapal'

    with open(pal_path, 'r') as f:
        lines = f.readlines()

    num_colors = int(lines[2].strip())
    if num_colors < 16:
        needed = 16 - num_colors
        print(f'{name}: has {num_colors} colors, adding {needed} black entries')
        # Patch the count line
        lines[2] = '16\n'
        # Add black entries
        for i in range(needed):
            lines.append('0 0 0\n')
        with open(pal_path, 'w') as f:
            f.writelines(lines)
        # Regenerate .gbapal
        result = subprocess.run([gbagfx, pal_path, gbapal_path], capture_output=True, text=True)
        if result.returncode == 0:
            size = os.path.getsize(gbapal_path)
            print(f'  -> {gbapal_path}: {size} bytes')
        else:
            print(f'  ERROR: {result.stderr}')
    else:
        print(f'{name}: already has {num_colors} colors, skipping')
