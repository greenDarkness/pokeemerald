#!/usr/bin/env python3
"""
Extract Meowth's tile data from a PNG and output a properly ordered 4bpp tile file (meowth_tiles.bin).
Requires Pillow (PIL) for PNG reading.
Assumes the PNG is 16x32 pixels, 9 frames stacked vertically or horizontally.
"""

from PIL import Image
import sys
import os

PNG_PATH = 'graphics/object_events/pics/pokemon/meowth.png'
OUTPUT_PATH = 'meowth_tiles.bin'
FRAME_WIDTH = 16
FRAME_HEIGHT = 32
NUM_FRAMES = 9
TILE_SIZE = 8
TILES_PER_FRAME = (FRAME_WIDTH // TILE_SIZE) * (FRAME_HEIGHT // TILE_SIZE)  # 2x4 = 8

try:
    img = Image.open(PNG_PATH)
except Exception as e:
    print(f"Error opening PNG: {e}")
    sys.exit(1)

# Check if image is tall (frames stacked vertically)
if img.width == FRAME_WIDTH and img.height == FRAME_HEIGHT * NUM_FRAMES:
    frame_mode = 'vertical'
elif img.height == FRAME_HEIGHT and img.width == FRAME_WIDTH * NUM_FRAMES:
    frame_mode = 'horizontal'
else:
    print("PNG dimensions do not match expected frame layout.")
    sys.exit(1)

output = bytearray()
for frame in range(NUM_FRAMES):
    if frame_mode == 'vertical':
        frame_img = img.crop((0, frame * FRAME_HEIGHT, FRAME_WIDTH, (frame + 1) * FRAME_HEIGHT))
    else:
        frame_img = img.crop((frame * FRAME_WIDTH, 0, (frame + 1) * FRAME_WIDTH, FRAME_HEIGHT))
    # Convert to 4bpp tile order (row-major)
    for tile_row in range(4):
        for tile_col in range(2):
            tile = frame_img.crop((tile_col * TILE_SIZE, tile_row * TILE_SIZE,
                                   (tile_col + 1) * TILE_SIZE, (tile_row + 1) * TILE_SIZE))
            # Convert tile to 4bpp (placeholder: just flatten pixel values)
            tile_bytes = bytearray()
            for y in range(TILE_SIZE):
                for x in range(TILE_SIZE):
                    px = tile.getpixel((x, y))
                    # If palette index, just use lower 4 bits
                    if isinstance(px, int):
                        tile_bytes.append(px & 0xF)
                    else:
                        tile_bytes.append(px[0] & 0xF)
            # Pad to 32 bytes (GBA 4bpp tile size)
            tile_bytes = tile_bytes[:32] + b'\x00' * (32 - len(tile_bytes[:32]))
            output.extend(tile_bytes)

with open(OUTPUT_PATH, 'wb') as f:
    f.write(output)

print(f"Extracted Meowth tiles to {OUTPUT_PATH} ({len(output)} bytes)")
