from PIL import Image
import os

sprites = [
    'graphics/object_events/pics/people/gym_leaders/falkner.png',
    'graphics/object_events/pics/people/special/silver.png',
    'graphics/object_events/pics/people/rockets/petrel.png',
    'graphics/object_events/pics/misc/whirlpool.png',
]

for rel in sprites:
    p = '/mnt/c/Users/drfor/Documents/Porymap/Decomps/pokeemerald/' + rel
    img = Image.open(p)
    data = list(img.getdata())
    uniq = sorted(set(data))
    print(f'{rel}: mode={img.mode} size={img.size} max_idx={max(data)} unique={uniq[:20]}')
