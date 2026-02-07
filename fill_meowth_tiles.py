#!/usr/bin/env python3
"""
Fill Meowth's 4bpp sprite with actual tile data, padding only unused tiles.
Assumes you have a source Meowth tile file (meowth_tiles.bin) with the correct tile data for each frame.
"""

import os

SRC_TILE_FILE = 'meowth_tiles.bin'  # Should contain 8 tiles (512 bytes) per frame, or fewer if Meowth uses less
DST_FILE = 'graphics/object_events/pics/pokemon/meowth.4bpp'
NUM_FRAMES = 9
TILES_PER_FRAME = 8
TILE_SIZE = 32
FRAME_SIZE = TILES_PER_FRAME * TILE_SIZE  # 256 bytes

# Read source tile data
if not os.path.exists(SRC_TILE_FILE):
    print(f"Source tile file '{SRC_TILE_FILE}' not found.")
    exit(1)

with open(SRC_TILE_FILE, 'rb') as f:
    src_data = f.read()

# Prepare output
output = bytearray()
for frame in range(NUM_FRAMES):
    frame_offset = frame * FRAME_SIZE
    # If not enough data, pad with zeros
    if len(src_data) >= frame_offset + FRAME_SIZE:
        output.extend(src_data[frame_offset:frame_offset+FRAME_SIZE])
    else:
        # Fill with available data, pad the rest
        available = src_data[frame_offset:]
        output.extend(available)
        output.extend(b'\x00' * (FRAME_SIZE - len(available)))

with open(DST_FILE, 'wb') as f:
    f.write(output)

print(f"Meowth.4bpp filled and padded. Output size: {len(output)} bytes.")
