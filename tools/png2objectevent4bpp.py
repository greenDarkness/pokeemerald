#!/usr/bin/env python3
"""
Convert a 16x32 object event PNG (indexed, 4bpp, 9 frames horizontally) to a .4bpp file in row-major tile order for pokeemerald object events.
- Input: PNG (144x32, 9 frames of 16x32 each, 4bpp indexed)
- Output: .4bpp file (2304 bytes, 9 frames × 8 tiles × 32 bytes)
"""
from PIL import Image
import sys
import os

def extract_tiles(img, frame_x, frame_y, frame_w, frame_h):
    """Extracts 8x8 tiles from a frame, returns a list of 32-byte arrays in row-major order (left-to-right, top-to-bottom)."""
    tiles = []
    for tile_row in range(frame_h // 8):
        for tile_col in range(frame_w // 8):
            tile = bytearray()
            for y in range(8):
                row = []
                for x in range(8):
                    px = img.getpixel((frame_x + tile_col*8 + x, frame_y + tile_row*8 + y))
                    row.append(px & 0xF)
                # Pack 2 pixels per byte (4bpp)
                for i in range(0, 8, 2):
                    tile.append((row[i] & 0xF) | ((row[i+1] & 0xF) << 4))
            tiles.append(tile)
    return tiles

def main():
    if len(sys.argv) != 3:
        print("Usage: python png2objectevent4bpp.py input.png output.4bpp")
        sys.exit(1)
    in_path, out_path = sys.argv[1:3]
    img = Image.open(in_path)
    if img.mode != 'P':
        print("Error: PNG must be 4bpp indexed (mode 'P').")
        sys.exit(1)
    width, height = img.size
    frame_w, frame_h = 16, 32
    num_frames = width // frame_w
    if height != 32 or num_frames != 9:
        print(f"Error: Expected 9 frames of 16x32 (image size 144x32), got {width}x{height}.")
        sys.exit(1)
    all_tiles = []
    for frame in range(num_frames):
        tiles = extract_tiles(img, frame*frame_w, 0, frame_w, frame_h)
        if len(tiles) != 8:
            print(f"Error: Frame {frame} did not produce 8 tiles.")
            sys.exit(1)
        all_tiles.extend(tiles)
    with open(out_path, 'wb') as f:
        for tile in all_tiles:
            f.write(tile)
    print(f"Wrote {out_path} ({len(all_tiles)*32} bytes)")

if __name__ == "__main__":
    main()
