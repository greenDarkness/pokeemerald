"""
Convert new Johto overworld sprite PNGs from horizontal strips to vertical strips.
Horizontal: all frames side-by-side (width=N*frame_w, height=frame_h)
Vertical:   all frames stacked  (width=frame_w, height=N*frame_h)

With gbagfx default metatile 1x1, a vertical strip produces correct frame-by-frame
tile ordering for GBA OAM.  The horizontal format requires -mwidth/-mheight flags.
"""
from PIL import Image
import os

BASE = '/mnt/c/Users/drfor/Documents/Porymap/Decomps/pokeemerald'

# (relative_path, frame_width, frame_height)
SPRITES = [
    # 16x32 NPC sprites
    ('graphics/object_events/pics/people/attendant_m.png',   16, 32),
    ('graphics/object_events/pics/people/attendant_f.png',   16, 32),
    ('graphics/object_events/pics/people/super_nerd.png',    16, 32),
    ('graphics/object_events/pics/people/battle_girl.png',   16, 32),
    ('graphics/object_events/pics/people/engineer.png',      16, 32),
    ('graphics/object_events/pics/people/burglar.png',       16, 32),
    ('graphics/object_events/pics/people/firebreather.png',  16, 32),
    ('graphics/object_events/pics/people/sage.png',          16, 32),
    ('graphics/object_events/pics/people/swimmer_f_land.png', 16, 32),
    ('graphics/object_events/pics/people/swimmer_m_land.png', 16, 32),
    ('graphics/object_events/pics/people/special/kimono.png', 16, 32),
    ('graphics/object_events/pics/people/special/kurt.png',   16, 32),
    ('graphics/object_events/pics/people/special/eusine.png', 16, 32),
    ('graphics/object_events/pics/people/special/elm.png',    16, 32),
    ('graphics/object_events/pics/people/special/silver.png', 16, 32),
    ('graphics/object_events/pics/people/rockets/petrel.png',  16, 32),
    ('graphics/object_events/pics/people/rockets/ariana.png',  16, 32),
    ('graphics/object_events/pics/people/rockets/proton.png',  16, 32),
    ('graphics/object_events/pics/people/rockets/archer.png',  16, 32),
    ('graphics/object_events/pics/people/rockets/giovanni.png', 16, 32),
    ('graphics/object_events/pics/people/gym_leaders/falkner.png', 16, 32),
    ('graphics/object_events/pics/people/gym_leaders/bugsy.png',   16, 32),
    ('graphics/object_events/pics/people/gym_leaders/whitney.png', 16, 32),
    ('graphics/object_events/pics/people/gym_leaders/morty.png',   16, 32),
    ('graphics/object_events/pics/people/gym_leaders/jasmine.png', 16, 32),
    ('graphics/object_events/pics/people/gym_leaders/chuck.png',   16, 32),
    ('graphics/object_events/pics/people/gym_leaders/pryce.png',   16, 32),
    ('graphics/object_events/pics/people/gym_leaders/clair.png',   16, 32),
    ('graphics/object_events/pics/people/elite_four/will.png',     16, 32),
    ('graphics/object_events/pics/people/elite_four/karen.png',    16, 32),
    ('graphics/object_events/pics/people/elite_four/lance.png',    16, 32),
    # 32x32 Pokémon sprites
    ('graphics/object_events/pics/pokemon/shiny_gyarados.png',  32, 32),
    ('graphics/object_events/pics/pokemon/slowpoke_no_tail.png', 32, 32),
    # 64x64 multi-frame large sprites (train_mid/back are single-frame, skip)
    ('graphics/object_events/pics/misc/whirlpool.png', 64, 64),
]

errors = 0
for rel, fw, fh in SPRITES:
    path = os.path.join(BASE, rel)
    img = Image.open(path)
    w, h = img.size

    # Already vertical or single-frame — nothing to do
    if w == fw:
        print(f'  SKIP {rel}: already {w}x{h}')
        continue

    if h != fh or w % fw != 0:
        print(f'  ERROR {rel}: unexpected size {w}x{h} (expected multiple of {fw}x{fh})')
        errors += 1
        continue

    n_frames = w // fw
    new_img = Image.new('P', (fw, fh * n_frames))
    pal = img.getpalette()
    if pal:
        new_img.putpalette(pal)

    for i in range(n_frames):
        frame = img.crop((i * fw, 0, (i + 1) * fw, fh))
        new_img.paste(frame, (0, i * fh))

    new_img.save(path)
    print(f'  OK  {rel}: {w}x{h} -> {fw}x{fh * n_frames} ({n_frames} frames)')

if errors:
    print(f'\n{errors} error(s).')
else:
    print('\nAll done.')
