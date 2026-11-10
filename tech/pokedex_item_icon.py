"""Draws the Pokédex key item icon (graphics/items/icons/pokedex.png + icon_palettes/pokedex.pal).

Usage: python tech/pokedex_item_icon.py
"""
import math
import os
from PIL import Image

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))

PAL = [
    (180, 180, 180),  # 0 transparent
    (56, 16, 24),     # 1 outline
    (168, 24, 32),    # 2 red shadow
    (232, 56, 48),    # 3 red
    (255, 136, 120),  # 4 red highlight
    (24, 64, 152),    # 5 lens dark
    (64, 152, 248),   # 6 lens
    (184, 228, 255),  # 7 lens light
    (255, 255, 255),  # 8 white
    (96, 144, 96),    # 9 screen dark
    (168, 216, 152),  # 10 screen
    (248, 208, 48),   # 11 yellow light
    (72, 192, 72),    # 12 green light
    (232, 40, 40),    # 13 red light
    (112, 112, 120),  # 14 gray
    (40, 40, 48),     # 15 hinge dark
]

def main():
    im = Image.new('P', (24, 24), 0)
    px = im.load()
    x0, y0, x1, y1 = 4, 2, 19, 21

    # body: rounded rectangle with outline, light top-left and dark bottom-right
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            corner = (x in (x0, x1) and y in (y0, y1))
            if corner:
                continue
            edge = x in (x0, x1) or y in (y0, y1) or (x in (x0 + 1, x1 - 1) and y in (y0, y1)) \
                or (y in (y0 + 1, y1 - 1) and x in (x0, x1))
            if edge:
                px[x, y] = 1
            elif x == x1 - 1 or y == y1 - 1:
                px[x, y] = 2
            elif x == x0 + 1 or y == y0 + 1:
                px[x, y] = 4
            else:
                px[x, y] = 3
    # hinge between the top half and the screen half
    for x in range(x0 + 1, x1):
        px[x, 10] = 15
    px[x0 + 1, 10] = 1
    px[x1 - 1, 10] = 1

    # big blue lens with a white ring and glint
    cx, cy = 8.5, 6.0
    for y in range(16):
        for x in range(24):
            d = math.hypot(x + 0.5 - cx, y + 0.5 - cy)
            if d <= 3.3:
                px[x, y] = 1 if d > 2.75 else (8 if d > 2.1 else (5 if (x + 0.5 - cx) + (y + 0.5 - cy) > 1.0 else 6))
    px[7, 5] = 7
    px[8, 5] = 7
    px[7, 6] = 7

    # three status lights
    for x, c in ((12, 13), (14, 11), (16, 12)):
        px[x, 4] = c
        px[x, 5] = 1 if False else c
        px[x + 1, 4] = 1
        px[x + 1, 5] = 1
    for x in (12, 14, 16):
        px[x, 6] = 1

    # screen
    for y in range(12, 19):
        for x in range(7, 17):
            border = y in (12, 18) or x in (7, 16)
            px[x, y] = 9 if border else 10
    for x in range(9, 13):
        px[x, 14] = 9
    for x in range(9, 15):
        px[x, 16] = 9

    # small buttons under the screen
    px[8, 19] = 15
    px[9, 19] = 15
    px[14, 19] = 14
    px[15, 19] = 14

    flat = []
    for c in PAL:
        flat += list(c)
    im.putpalette(flat)
    im.save(os.path.join(REPO, 'graphics', 'items', 'icons', 'pokedex.png'), bits=4)
    with open(os.path.join(REPO, 'graphics', 'items', 'icon_palettes', 'pokedex.pal'), 'w', newline='\r\n') as f:
        f.write('JASC-PAL\n0100\n16\n')
        for r, g, b in PAL:
            f.write(f'{r} {g} {b}\n')

if __name__ == '__main__':
    main()
