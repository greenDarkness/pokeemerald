#!/bin/bash
# Regenerate new Johto overworld sprite .4bpp files with correct metatile dimensions
# for proper frame-by-frame tile packing (as required by overworld_frame macro)

cd /mnt/c/Users/drfor/Documents/Porymap/Decomps/pokeemerald
GFX=./tools/gbagfx/gbagfx

echo "=== Regenerating 16x32 NPC sprites (mwidth 2 mheight 4) ==="
for png in \
    graphics/object_events/pics/people/attendant_m.png \
    graphics/object_events/pics/people/attendant_f.png \
    graphics/object_events/pics/people/super_nerd.png \
    graphics/object_events/pics/people/battle_girl.png \
    graphics/object_events/pics/people/engineer.png \
    graphics/object_events/pics/people/burglar.png \
    graphics/object_events/pics/people/firebreather.png \
    graphics/object_events/pics/people/sage.png \
    graphics/object_events/pics/people/swimmer_f_land.png \
    graphics/object_events/pics/people/swimmer_m_land.png \
    graphics/object_events/pics/people/special/kimono.png \
    graphics/object_events/pics/people/special/kurt.png \
    graphics/object_events/pics/people/special/eusine.png \
    graphics/object_events/pics/people/special/elm.png \
    graphics/object_events/pics/people/special/silver.png \
    graphics/object_events/pics/people/rockets/petrel.png \
    graphics/object_events/pics/people/rockets/ariana.png \
    graphics/object_events/pics/people/rockets/proton.png \
    graphics/object_events/pics/people/rockets/archer.png \
    graphics/object_events/pics/people/rockets/giovanni.png \
    graphics/object_events/pics/people/gym_leaders/falkner.png \
    graphics/object_events/pics/people/gym_leaders/bugsy.png \
    graphics/object_events/pics/people/gym_leaders/whitney.png \
    graphics/object_events/pics/people/gym_leaders/morty.png \
    graphics/object_events/pics/people/gym_leaders/jasmine.png \
    graphics/object_events/pics/people/gym_leaders/chuck.png \
    graphics/object_events/pics/people/gym_leaders/pryce.png \
    graphics/object_events/pics/people/gym_leaders/clair.png \
    graphics/object_events/pics/people/elite_four/will.png \
    graphics/object_events/pics/people/elite_four/karen.png \
    graphics/object_events/pics/people/elite_four/lance.png
do
    out="${png%.png}.4bpp"
    $GFX "$png" "$out" -mwidth 2 -mheight 4
    echo "  $out"
done

echo "=== Regenerating 32x32 following sprites (mwidth 4 mheight 4) ==="
for png in \
    graphics/object_events/pics/pokemon/shiny_gyarados.png \
    graphics/object_events/pics/pokemon/slowpoke_no_tail.png
do
    out="${png%.png}.4bpp"
    $GFX "$png" "$out" -mwidth 4 -mheight 4
    echo "  $out"
done

echo "=== Regenerating 64x64 large sprites (mwidth 8 mheight 8) ==="
for png in \
    graphics/object_events/pics/misc/whirlpool.png \
    graphics/object_events/pics/misc/train_mid.png \
    graphics/object_events/pics/misc/train_back.png
do
    out="${png%.png}.4bpp"
    $GFX "$png" "$out" -mwidth 8 -mheight 8
    echo "  $out"
done

echo "Done."
