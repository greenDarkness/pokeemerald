#!/usr/bin/env python3
"""
Generate the full stat indicator sprite sheet.
7 stats × 12 stages = 84 buttons (32x8 each, 4 tiles per button)
32x8 is the smallest horizontal rectangle sprite the GBA supports that fits our text.
"""

from PIL import Image
import os

OUTPUT_DIR = os.path.join(os.path.dirname(__file__), "..", "graphics", "battle_interface")

# Colors (matching GBA status icon style)
TRANSPARENT = (255, 0, 255)  # Magenta for transparency
GREEN_BG = (56, 168, 72)     # Green for boosts
GREEN_DARK = (32, 112, 48)   # Border for green
RED_BG = (168, 72, 72)       # Red for drops
RED_DARK = (112, 48, 48)     # Border for red
WHITE = (248, 248, 248)      # Text color

# Stats in order
STATS = ["Atk", "Def", "Spd", "SpA", "SpD", "Acc", "Eva"]

# 3x5 pixel font
FONT_3X5 = {
    'A': ["010", "101", "111", "101", "101"],
    'a': ["000", "011", "101", "101", "011"],
    'D': ["110", "101", "101", "101", "110"],
    'd': ["000", "011", "101", "101", "011"],
    'S': ["011", "100", "010", "001", "110"],
    's': ["000", "011", "010", "001", "110"],
    'p': ["000", "110", "101", "110", "100"],
    'E': ["111", "100", "110", "100", "111"],
    'e': ["000", "010", "111", "100", "011"],
    'v': ["000", "101", "101", "101", "010"],
    'c': ["000", "011", "100", "100", "011"],
    'f': ["011", "100", "110", "100", "100"],
    't': ["010", "111", "010", "010", "001"],
    'k': ["100", "101", "110", "110", "101"],
    '+': ["000", "010", "111", "010", "000"],
    '-': ["000", "000", "111", "000", "000"],
    '1': ["010", "110", "010", "010", "111"],
    '2': ["110", "001", "010", "100", "111"],
    '3': ["110", "001", "010", "001", "110"],
    '4': ["101", "101", "111", "001", "001"],
    '5': ["111", "100", "110", "001", "110"],
    '6': ["011", "100", "110", "101", "010"],
    ' ': ["000", "000", "000", "000", "000"],
}

def draw_char(img, char, x, y, color):
    if char not in FONT_3X5:
        return 3
    pattern = FONT_3X5[char]
    for row_idx, row in enumerate(pattern):
        for col_idx, pixel in enumerate(row):
            if pixel == '1':
                px, py = x + col_idx, y + row_idx
                if 0 <= px < img.width and 0 <= py < img.height:
                    img.putpixel((px, py), color)
    return len(pattern[0])

def draw_text(img, text, x, y, color):
    for char in text:
        width = draw_char(img, char, x, y, color)
        x += width + 1
    return x

def create_button(width, height, text, bg_color, border_color, text_color):
    img = Image.new('RGB', (width, height), bg_color)
    
    # Draw 1-pixel border
    for x in range(width):
        img.putpixel((x, 0), border_color)
        img.putpixel((x, height - 1), border_color)
    for y in range(height):
        img.putpixel((0, y), border_color)
        img.putpixel((width - 1, y), border_color)
    
    # Calculate text width (3px per char + 1px spacing)
    text_width = len(text) * 4 - 1
    x_start = (width - text_width) // 2
    y_start = 1
    draw_text(img, text, x_start, y_start, text_color)
    
    return img

def main():
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    
    # Layout: 7 stats (columns) × 12 stages (rows)
    # Rows 0-5: +1 to +6 (boost, green)
    # Rows 6-11: -1 to -6 (drop, red)
    # Using 32x8 sprites (GBA's smallest horizontal rectangle that fits)
    
    button_w, button_h = 32, 8
    sheet_w = len(STATS) * button_w  # 7 * 32 = 224
    sheet_h = 12 * button_h          # 12 * 8 = 96
    
    # Create sprite sheet with magenta background (transparent)
    sheet = Image.new('RGB', (sheet_w, sheet_h), TRANSPARENT)
    
    for col, stat in enumerate(STATS):
        x = col * button_w
        
        # Boosts (+1 to +6) in rows 0-5
        for stage in range(1, 7):
            row = stage - 1
            y = row * button_h
            text = f"{stat} +{stage}"  # With space for readability
            btn = create_button(button_w, button_h, text, GREEN_BG, GREEN_DARK, WHITE)
            sheet.paste(btn, (x, y))
        
        # Drops (-1 to -6) in rows 6-11
        for stage in range(1, 7):
            row = 5 + stage
            y = row * button_h
            text = f"{stat} -{stage}"  # With space for readability
            btn = create_button(button_w, button_h, text, RED_BG, RED_DARK, WHITE)
            sheet.paste(btn, (x, y))
    
    # Convert to indexed color (P mode) with our palette
    # GBA requires indexed PNGs
    palette_colors = [
        TRANSPARENT,  # 0: Transparent
        WHITE,        # 1: White text
        GREEN_BG,     # 2: Green background
        GREEN_DARK,   # 3: Green border
        RED_BG,       # 4: Red background
        RED_DARK,     # 5: Red border
    ]
    # Fill rest of palette with black
    for _ in range(6, 16):
        palette_colors.append((0, 0, 0))
    
    # Create palette image
    palette_img = Image.new('P', (1, 1))
    flat_palette = []
    for color in palette_colors:
        flat_palette.extend(color)
    # Extend to 256 colors (768 values)
    flat_palette.extend([0] * (768 - len(flat_palette)))
    palette_img.putpalette(flat_palette)
    
    # Convert RGB image to indexed using the palette
    indexed_sheet = sheet.quantize(palette=palette_img, dither=0)
    
    # Save sprite sheet
    sheet_path = os.path.join(OUTPUT_DIR, "stat_indicators.png")
    indexed_sheet.save(sheet_path)
    
    # Create palette file (JASC-PAL format)
    pal_path = os.path.join(OUTPUT_DIR, "stat_indicators.pal")
    with open(pal_path, 'w') as f:
        f.write("JASC-PAL\n")
        f.write("0100\n")
        f.write("16\n")
        # Index 0: Transparent (magenta)
        f.write(f"{TRANSPARENT[0]} {TRANSPARENT[1]} {TRANSPARENT[2]}\n")
        # Index 1: White (text)
        f.write(f"{WHITE[0]} {WHITE[1]} {WHITE[2]}\n")
        # Index 2: Green background
        f.write(f"{GREEN_BG[0]} {GREEN_BG[1]} {GREEN_BG[2]}\n")
        # Index 3: Green dark (border)
        f.write(f"{GREEN_DARK[0]} {GREEN_DARK[1]} {GREEN_DARK[2]}\n")
        # Index 4: Red background
        f.write(f"{RED_BG[0]} {RED_BG[1]} {RED_BG[2]}\n")
        # Index 5: Red dark (border)
        f.write(f"{RED_DARK[0]} {RED_DARK[1]} {RED_DARK[2]}\n")
        # Fill rest with black
        for i in range(6, 16):
            f.write("0 0 0\n")
    
    # Create scaled preview
    scale = 4
    preview = sheet.resize((sheet_w * scale, sheet_h * scale), Image.NEAREST)
    preview_path = os.path.join(OUTPUT_DIR, "stat_indicators_preview.png")
    preview.save(preview_path)
    
    print(f"Created {sheet_path}")
    print(f"  Size: {sheet_w}x{sheet_h} pixels ({sheet_w//8}x{sheet_h//8} tiles)")
    print(f"  Each button: {button_w}x{button_h} ({button_w//8} tiles wide)")
    print(f"  Layout: {len(STATS)} stats × 12 stages = {len(STATS) * 12} buttons")
    print(f"  Rows 0-5: boosts (+1 to +6, green)")
    print(f"  Rows 6-11: drops (-1 to -6, red)")
    print(f"  Total tiles: {(sheet_w//8) * (sheet_h//8)}")
    print(f"Created palette: {pal_path}")
    print(f"Preview: {preview_path}")

if __name__ == "__main__":
    main()
