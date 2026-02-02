#!/usr/bin/env python3
"""
Tile swapper: Takes Dusclops.4bpp as template and injects Meowth tiles.
For 16x32 sprites: 2 tiles wide x 4 tiles tall = 8 tiles per frame
Each tile is 8x8 pixels = 64 bytes in 4bpp format
9 frames total = 9 * 8 tiles * 64 bytes = 4608 bytes (but current is 2304)
Actually 16x32 = 512 pixels / 4bpp = 256 bytes per frame
9 frames = 2304 bytes total
"""
import shutil
import struct

# Read the correctly-compiled Meowth tiles
with open('graphics/object_events/pics/pokemon/meowth.4bpp', 'rb') as f:
    meowth_data = bytearray(f.read())

# Read the working Dusclops template
with open('graphics/object_events/pics/pokemon/dusclops.4bpp', 'rb') as f:
    template_data = bytearray(f.read())

# For 16x32: each frame is 256 bytes (2 tiles wide * 4 tiles tall * 64 bytes/tile = 512 bytes... wait)
# Actually: 16x32 pixels = 2x4 tiles. Each tile = 64 bytes. So 8 tiles * 64 = 512 bytes per frame
# But we only have 2304 bytes / 9 frames = 256 bytes per frame
# This means the format uses a compressed arrangement

# Let's just copy over frame-by-frame (256 bytes per frame)
BYTES_PER_FRAME = 256
NUM_FRAMES = 9

for frame in range(NUM_FRAMES):
    start = frame * BYTES_PER_FRAME
    end = start + BYTES_PER_FRAME
    template_data[start:end] = meowth_data[start:end]

# Write the result
with open('graphics/object_events/pics/pokemon/meowth.4bpp', 'wb') as f:
    f.write(template_data)

print("Meowth tiles injected into Dusclops template structure")
print("meowth.4bpp updated successfully")
