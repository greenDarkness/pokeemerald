#!/usr/bin/env python3
"""
Check and fix the palette of a 4bpp indexed PNG for pokeemerald object events.
- Ensures the PNG uses 16 or fewer colors.
- Optionally remaps all pixels to a target palette order (e.g., to match a .pal file or a reference PNG).
- Reports palette usage and can output a fixed PNG.
"""
from PIL import Image
import sys

def main():
    if len(sys.argv) < 2:
        print("Usage: python fix_palette.py input.png [output_fixed.png]")
        sys.exit(1)
    in_path = sys.argv[1]
    out_path = sys.argv[2] if len(sys.argv) > 2 else None
    img = Image.open(in_path)
    if img.mode != 'P':
        print("Error: PNG must be indexed (mode 'P').")
        sys.exit(1)
    palette = img.getpalette()[:48]  # Only first 16 colors (16*3)
    used = set(img.getdata())
    print(f"Colors used: {sorted(used)} (total: {len(used)})")
    if len(used) > 16:
        print("Error: More than 16 colors used! Reduce colors to 16 or fewer.")
        sys.exit(1)
    # Optionally, remap palette indices to sequential order (0-15)
    remap = {old:new for new, old in enumerate(sorted(used))}
    print(f"Palette remap: {remap}")
    if out_path:
        # Remap all pixels
        data = [remap[p] for p in img.getdata()]
        img2 = Image.new('P', img.size)
        img2.putdata(data)
        # Build new palette (fill unused with 0)
        new_palette = [0]*48
        for new, old in remap.items():
            new_palette[new*3:new*3+3] = palette[old*3:old*3+3]
        img2.putpalette(new_palette)
        img2.save(out_path)
        print(f"Wrote fixed PNG: {out_path}")
    else:
        print("No output file specified, only checked palette.")

if __name__ == "__main__":
    main()
