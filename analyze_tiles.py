#!/usr/bin/env python3
"""
Analyze and fix tile ordering for GBA overworld sprites.
For 16x32 sprites (2 tiles wide, 4 tiles tall):
- Each tile is 8x8 pixels = 64 bytes in 4bpp
- Per frame: 2*4 = 8 tiles = 512 bytes... but actual is 256 bytes per frame
  
This suggests tiles might be arranged differently than expected.
Let's analyze the structure.
"""

import struct

def analyze_sprite(filepath, name):
    with open(filepath, 'rb') as f:
        data = bytearray(f.read())
    
    print(f"\n{name}:")
    print(f"  Total size: {len(data)} bytes")
    print(f"  Frames: {len(data) // 256}")
    print(f"  Bytes per frame: {len(data) // 9 if len(data) % 9 == 0 else 'irregular'}")
    
    # Show first 64 bytes of each frame (first tile)
    for frame in range(min(2, len(data) // 256)):
        offset = frame * 256
        print(f"  Frame {frame} first 32 bytes: {data[offset:offset+32].hex()}")

analyze_sprite('graphics/object_events/pics/pokemon/dusclops.4bpp', 'Dusclops (4bpp)')
analyze_sprite('graphics/object_events/pics/pokemon/meowth.4bpp', 'Meowth (4bpp)')

# Try to understand the mapping
# If Dusclops works, let's see if we can find a pattern
with open('graphics/object_events/pics/pokemon/dusclops.4bpp', 'rb') as f:
    dus_data = f.read()
with open('graphics/object_events/pics/pokemon/meowth.4bpp', 'rb') as f:
    meo_data = f.read()

# Check for repeating patterns or recognizable structure
print("\n\nSearching for pattern differences...")
for i in range(0, min(256, len(dus_data)), 64):
    dus_tile = dus_data[i:i+64]
    meo_tile = meo_data[i:i+64]
    if dus_tile != meo_tile:
        print(f"Tile offset {i}: Different (first 8 bytes dus={dus_tile[:8].hex()} meo={meo_tile[:8].hex()})")
