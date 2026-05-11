import os
pal_dir = '/mnt/c/Users/drfor/Documents/Porymap/Decomps/pokeemerald/graphics/object_events/palettes/'
for name in ['sage', 'pryce', 'bugsy', 'karen', 'lance', 'silver', 'will']:
    pal_path = pal_dir + name + '.pal'
    gbapal_path = pal_dir + name + '.gbapal'
    pal_size = os.path.getsize(pal_path) if os.path.exists(pal_path) else 'N/A'
    gbapal_size = os.path.getsize(gbapal_path) if os.path.exists(gbapal_path) else 'N/A'
    print(f'{name}: .pal={pal_size}b  .gbapal={gbapal_size}b')
    if os.path.exists(pal_path):
        with open(pal_path, 'r') as f:
            lines = f.readlines()
        print(f'  .pal header: {lines[:3]}')
