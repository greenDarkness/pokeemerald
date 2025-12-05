#!/usr/bin/env python3
"""
Create the PC button tilemap file for party menu
"""
import struct

# The cancel button tilemap from xxd:
# 0a10 0b10 0b10 0b10 0b10 0b10 0c10  (row 1)
# 1210 0b18 0b18 0b18 0b18 0b18 1310  (row 2)
#
# Format: Each entry is 2 bytes (little endian u16)
# Low 10 bits = tile index
# Bits 10-11 = horizontal/vertical flip
# Bits 12-15 = palette number
#
# 0x10 = palette 1 (0x1 << 12 = 0x1000, but stored as 0x10 in low byte position)
# Actually: 0x100A = tile 0x0A, palette 1, no flip
# 0x180B = tile 0x0B, palette 1, h-flip (0x0800 = h-flip)

# The issue is tile 0x0B has a dark line pattern
# Looking at the tiles:
# - 0x0A = top-left corner
# - 0x0B = middle fill (has the dark line issue in certain rows)
# - 0x0C = top-right corner
# - 0x12 = bottom-left corner
# - 0x13 = bottom-right corner

# For a clean button, we could create the tilemap the same way
# but the visual fix needs to happen either:
# 1. In the bg.png tileset itself (edit tile 0x0B)
# 2. Or use different tiles

# For now, let's create a PC button tilemap that's the same as cancel
# The actual "fix" will be done by modifying the tile in bg.png

# PC button tilemap - 7 tiles wide x 2 tiles tall
# We'll position it at (1, 11) which is tile position for the START PC area
# But we need it for BG1 at the right position

# Actually, let's create a simpler approach:
# Use the same tilemap structure but we position it differently in code

# Cancel button tilemap data (7x2 tiles = 14 entries = 28 bytes)
cancel_tilemap = [
    # Row 1: tiles 0x0A, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0C with palette 1
    0x100A, 0x100B, 0x100B, 0x100B, 0x100B, 0x100B, 0x100C,
    # Row 2: tiles 0x12, 0x0B (h-flip), 0x0B (h-flip), 0x0B (h-flip), 0x0B (h-flip), 0x0B (h-flip), 0x13
    0x1012, 0x180B, 0x180B, 0x180B, 0x180B, 0x180B, 0x1013,
]

# Write the tilemap to a binary file
with open("graphics/party_menu/start_button.bin", "wb") as f:
    for entry in cancel_tilemap:
        f.write(struct.pack('<H', entry))

print("Created graphics/party_menu/start_button.bin")
print(f"Tilemap entries: {[hex(x) for x in cancel_tilemap]}")

# Now let's also fix the bg.png tile 0x0B to remove the dark line
from PIL import Image

bg = Image.open("graphics/party_menu/bg.png")
pixels = list(bg.getdata())
width = bg.size[0]

tile_w, tile_h = 8, 8
tiles_per_row = width // tile_w

# Tile 0x0B is at row 1, col 3 (0x0B = 11 = 1*8 + 3)
tile_row = 0x0B // tiles_per_row  # = 1
tile_col = 0x0B % tiles_per_row   # = 3

print(f"\nTile 0x0B is at tile position ({tile_col}, {tile_row})")
print(f"Pixel position: ({tile_col * tile_w}, {tile_row * tile_h})")

# Print the tile before fix
print("\nTile 0x0B before fix:")
for y in range(tile_h):
    row = ""
    for x in range(tile_w):
        px = tile_col * tile_w + x
        py = tile_row * tile_h + y
        idx = py * width + px
        row += f"{pixels[idx]:02X}"
    print(f"  Row {y}: {row}")

# The dark line is the row of 0x11 pixels
# We need to replace 0x11 with something lighter
# Looking at the button, 0x1F is the inner fill color

# Fix the tile - replace dark pixels in the middle area
new_pixels = list(pixels)
for y in range(tile_h):
    for x in range(tile_w):
        px = tile_col * tile_w + x
        py = tile_row * tile_h + y
        idx = py * width + px
        # The pattern shows 0x11 appears in certain positions
        # Replace dark 0x11 with 0x1F (light fill)
        if new_pixels[idx] == 0x11:
            new_pixels[idx] = 0x1F

# Print the tile after fix
print("\nTile 0x0B after fix:")
for y in range(tile_h):
    row = ""
    for x in range(tile_w):
        px = tile_col * tile_w + x
        py = tile_row * tile_h + y
        idx = py * width + px
        row += f"{new_pixels[idx]:02X}"
    print(f"  Row {y}: {row}")

# Save the fixed bg.png
fixed_bg = Image.new('P', bg.size)
fixed_bg.putpalette(bg.getpalette())
fixed_bg.putdata(new_pixels)
fixed_bg.save("graphics/party_menu/bg.png")
print("\nFixed graphics/party_menu/bg.png (replaced 0x11 with 0x1F in tile 0x0B)")
