#!/usr/bin/env python3
"""
Generate stat indicator sprites for battle interface.
Creates button-style badges like Pokemon Showdown.

Each badge is 16x8 pixels (2 tiles wide) showing "A+1", "D-2", etc.
Boosts have green background, drops have red background.
"""

from PIL import Image

# GBA 4bpp palette - 16 colors max
TRANSPARENT = 0
WHITE = 1
BLACK = 2
GREEN_BG = 3       # Green background for boosts
GREEN_BORDER = 4   # Darker green border
RED_BG = 5         # Red background for drops
RED_BORDER = 6     # Darker red border

# Palette colors (RGB)
PALETTE = [
    (255, 0, 255),   # 0: Transparent (magenta)
    (255, 255, 255), # 1: White (text)
    (0, 0, 0),       # 2: Black (text shadow/outline)
    (32, 192, 32),   # 3: Green background (boost)
    (16, 128, 16),   # 4: Green border (darker)
    (208, 48, 48),   # 5: Red background (drop)
    (160, 32, 32),   # 6: Red border (darker)
] + [(0, 0, 0)] * 9  # Padding to 16 colors

# 3x5 pixel micro font
FONT_3X5 = {
    'A': [".X.", "X.X", "XXX", "X.X", "X.X"],
    'D': ["XX.", "X.X", "X.X", "X.X", "XX."],
    'S': [".XX", "X..", ".X.", "..X", "XX."],
    'X': ["X.X", "X.X", ".X.", "X.X", "X.X"],
    'E': ["XXX", "X..", "XX.", "X..", "XXX"],
    '+': ["...", ".X.", "XXX", ".X.", "..."],
    '-': ["...", "...", "XXX", "...", "..."],
    '0': [".X.", "X.X", "X.X", "X.X", ".X."],
    '1': [".X.", "XX.", ".X.", ".X.", "XXX"],
    '2': ["XX.", "..X", ".X.", "X..", "XXX"],
    '3': ["XX.", "..X", ".X.", "..X", "XX."],
    '4': ["X.X", "X.X", "XXX", "..X", "..X"],
    '5': ["XXX", "X..", "XX.", "..X", "XX."],
    '6': [".XX", "X..", "XX.", "X.X", ".X."],
}

# Stat labels: single char for most, but SA/SD need special handling
STAT_LABELS = ['A', 'D', 'S', 'A', 'D', 'X', 'E']  # SA and SD will be handled specially
STAT_IS_SP = [False, False, False, True, True, False, False]  # True if Sp.Atk or Sp.Def

def create_palette_image(width, height):
    img = Image.new('P', (width, height))
    flat_palette = []
    for r, g, b in PALETTE:
        flat_palette.extend([r, g, b])
    flat_palette.extend([0] * (768 - len(flat_palette)))
    img.putpalette(flat_palette)
    return img

def draw_char(pixels, x, y, char, color, width, height):
    if char not in FONT_3X5:
        return
    for row, line in enumerate(FONT_3X5[char]):
        for col, c in enumerate(line):
            if c == 'X':
                px, py = x + col, y + row
                if 0 <= px < width and 0 <= py < height:
                    pixels[px, py] = color

def draw_button(pixels, x, y, stat_idx, stage, is_boost, width, height):
    """Draw a 16x8 button badge for a stat change."""
    bg_color = GREEN_BG if is_boost else RED_BG
    border_color = GREEN_BORDER if is_boost else RED_BORDER
    
    # Draw border (1px)
    for i in range(16):
        pixels[x + i, y] = border_color
        pixels[x + i, y + 7] = border_color
    for j in range(8):
        pixels[x, y + j] = border_color
        pixels[x + 15, y + j] = border_color
    
    # Fill background
    for j in range(1, 7):
        for i in range(1, 15):
            pixels[x + i, y + j] = bg_color
    
    # Draw stat label
    label = STAT_LABELS[stat_idx]
    is_sp = STAT_IS_SP[stat_idx]
    
    if is_sp:
        # Draw small 's' then the letter (A or D)
        # Tiny 's': just 2x3 pixels
        pixels[x + 2, y + 2] = WHITE
        pixels[x + 3, y + 2] = WHITE
        pixels[x + 2, y + 3] = WHITE
        pixels[x + 3, y + 4] = WHITE
        pixels[x + 2, y + 4] = WHITE
        # Then the main letter
        draw_char(pixels, x + 5, y + 1, label, WHITE, width, height)
        sign_x = x + 9
    else:
        draw_char(pixels, x + 2, y + 1, label, WHITE, width, height)
        sign_x = x + 6
    
    # Draw sign
    sign = '+' if is_boost else '-'
    draw_char(pixels, sign_x, y + 1, sign, WHITE, width, height)
    
    # Draw number
    draw_char(pixels, sign_x + 4, y + 1, str(stage), WHITE, width, height)

def main():
    # Layout: 7 stats x 6 stages x 2 (boost/drop) = 84 buttons
    # Each button is 16x8 (2 tiles wide x 1 tile tall)
    # Arrange as: 7 columns (one per stat), 12 rows (6 boost + 6 drop)
    # Total: 112x96 pixels = 14 tiles wide x 12 tiles tall
    
    # Simpler layout: 
    # Row 0-5: Boost buttons +1 to +6, columns = stats (A, D, S, SA, SD, X, E)
    # Row 6-11: Drop buttons -1 to -6, columns = stats
    
    tile_width = 16  # 2 tiles per button
    tile_height = 8  # 1 tile per button
    num_stats = 7
    num_stages = 6
    
    width = tile_width * num_stats   # 112 pixels
    height = tile_height * (num_stages * 2)  # 96 pixels (6 boost + 6 drop rows)
    
    img = create_palette_image(width, height)
    pixels = img.load()
    
    # Fill with transparent
    for py in range(height):
        for px in range(width):
            pixels[px, py] = TRANSPARENT
    
    # Draw boost buttons (rows 0-5, stages +1 to +6)
    for stage in range(1, 7):
        for stat_idx in range(num_stats):
            bx = stat_idx * tile_width
            by = (stage - 1) * tile_height
            draw_button(pixels, bx, by, stat_idx, stage, True, width, height)
    
    # Draw drop buttons (rows 6-11, stages -1 to -6)
    for stage in range(1, 7):
        for stat_idx in range(num_stats):
            bx = stat_idx * tile_width
            by = (stage - 1 + num_stages) * tile_height
            draw_button(pixels, bx, by, stat_idx, stage, False, width, height)
    
    output_path = "graphics/battle_interface/stat_indicators.png"
    img.save(output_path)
    print(f"Created {output_path} ({width}x{height})")
    print(f"Layout: {num_stats} stats x {num_stages*2} rows = {num_stats * num_stages * 2} buttons")
    print(f"Each button is {tile_width}x{tile_height} pixels (2x1 tiles)")
    print(f"Rows 0-5: Boost +1 to +6 (green)")
    print(f"Rows 6-11: Drop -1 to -6 (red)")
    print(f"Columns: A, D, S, sA, sD, X, E")

if __name__ == "__main__":
    main()
