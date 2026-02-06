#!/usr/bin/env python3
"""
Remap a PNG's palette to match a .gbapal file (pokeemerald format).
- Input: PNG (indexed, 4bpp), .gbapal (32 bytes, 16 colors, little-endian BGR555)
- Output: PNG with palette order matching the .gbapal file
"""
from PIL import Image
import sys
import struct

def read_gbapal(path):
    with open(path, 'rb') as f:
        data = f.read(32)
    # Each color: 2 bytes, little-endian, BGR555
    colors = []
    for i in range(16):
        val = struct.unpack('<H', data[i*2:i*2+2])[0]
        b = (val >> 10) & 0x1F
        g = (val >> 5) & 0x1F
        r = val & 0x1F
        # Convert to 8-bit RGB
        r = int(r * 255 / 31)
        g = int(g * 255 / 31)
        b = int(b * 255 / 31)
        colors.append((r, g, b))
    return colors

def main():
    if len(sys.argv) != 4:
        print("Usage: python remap_png_palette.py input.png input.gbapal output.png")
        sys.exit(1)
    png_path, pal_path, out_path = sys.argv[1:4]
    img = Image.open(png_path)
    if img.mode != 'P':
        print("Error: PNG must be indexed (mode 'P').")
        sys.exit(1)
    gbapal = read_gbapal(pal_path)
    # Build mapping from PNG palette color to gbapal index
    png_palette = img.getpalette()[:48]  # Only first 16 colors
    png_colors = [(png_palette[i], png_palette[i+1], png_palette[i+2]) for i in range(0, 48, 3)]
    # For each gbapal color, find the closest PNG palette color
    import math
    def dist(c1, c2):
        return sum((a-b)**2 for a, b in zip(c1, c2))
    mapping = {}
    for gbapal_idx, gbapal_col in enumerate(gbapal):
        best = min(range(len(png_colors)), key=lambda i: dist(gbapal_col, png_colors[i]))
        mapping[best] = gbapal_idx
    # Remap all pixels
    data = [mapping.get(p, 0) for p in img.getdata()]
    img2 = Image.new('P', img.size)
    img2.putdata(data)
    # Set new palette
    flat_gbapal = [v for rgb in gbapal for v in rgb]
    img2.putpalette(flat_gbapal + [0]*(768-len(flat_gbapal)))
    img2.save(out_path)
    print(f"Wrote {out_path} with palette from {pal_path}")

if __name__ == "__main__":
    main()
