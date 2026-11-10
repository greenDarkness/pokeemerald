"""Draws the POKéDEX key item icon (graphics/items/icons/pokedex.png + icon_palettes/pokedex.pal).

The open Gen 3 (RSE) Pokédex in the three-quarter view of the other item icons, built on the Coin
Case icon's open-clamshell shape. The body is hand-placed pixel art (lid with an inset bevelled panel,
green button and U-notch, hinge cylinder, base faces and screen); the round parts - the silver lens
disc and the hinge cap - are drawn as shaded ellipses and outlined on top.

Usage: python tech/pokedex_item_icon.py [preview.png]
"""
import math
import os
import sys
from PIL import Image

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))

PAL = [
    (180, 180, 180),  # 0 transparent
    (49, 41, 41),     # 1 outline
    (255, 189, 131),  # 2 orange highlight
    (248, 140, 74),   # 3 orange light
    (230, 98, 41),    # 4 orange
    (189, 70, 32),    # 5 orange shade
    (115, 41, 24),    # 6 orange dark
    (255, 255, 255),  # 7 white
    (205, 205, 213),  # 8 silver
    (139, 139, 148),  # 9 gray
    (189, 222, 246),  # a lens light
    (98, 148, 213),   # b lens
    (57, 57, 65),     # c black trim
    (213, 230, 205),  # d screen
    (57, 180, 74),    # e green
    (156, 230, 139),  # f green light
]

# Lid (rows 2-11): lit left rim, inset panel (dark top/left edge, lit bottom edge) holding the
# green button, two-tone right edge, U-notch at the top. Hinge cylinder (row 12). Base (rows 13-21):
# top face lit at the front edge, screen, sliders, front face and right face shaded darker downwards.
BODY = [
    "........................",
    "........................",
    "...........111....1111..",
    "..........1231....13351.",
    "..........12331111334561",
    "..........12333333333561",
    "..........12355555553561",
    "..........12354fe4443561",
    ".........123354ee4443561",
    ".........123354554443561",
    ".........123353333333561",
    "........1233333333333561",
    "........1666666666666111",
    "......116444444444444...",
    "....113333333cccccc3311.",
    "..113333333ccdddd7c37751",
    ".1333333333cdd9dd9c37551",
    ".122222222ccccccc5555551",
    ".1444444444444455555561.",
    ".145555555555555566661..",
    ".15555555555555566661...",
    ".16666666666666661111...",
    "..11111111111111111.....",
    "........................",
]


def ellipse(img, cx, cy, rx, ry, shade):
    mask = set()
    for y in range(24):
        for x in range(24):
            nx, ny = (x + 0.5 - cx) / rx, (y + 0.5 - cy) / ry
            if nx * nx + ny * ny <= 1.0:
                img[y][x] = shade(nx, ny)
                mask.add((x, y))
    return mask


def outline(img, mask):
    for (x, y) in list(mask):
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            if (x + dx, y + dy) not in mask:
                img[y][x] = 1
                break


def main():
    assert len(BODY) == 24 and all(len(r) == 24 for r in BODY), [len(r) for r in BODY]
    img = [[0 if c == '.' else int(c, 16) for c in row] for row in BODY]

    # Lens disc on the base's front-left: black side band below, then the top with a black rim,
    # silver ring (white glint top-left, gray at the bottom), black bar and blue lens with a glint
    side = ellipse(img, 6.0, 17.3, 5.3, 3.4, lambda nx, ny: 0xc)

    def disc_top(nx, ny):
        d = math.hypot(nx, ny)
        if d > 0.8:
            return 0xc
        if d < 0.4:
            return 0xa if (nx + ny) < -0.15 else 0xb
        if abs(ny) < 0.17:
            return 0xc
        if nx < -0.15 and ny < 0:
            return 7
        return 8 if ny < 0.5 else 9

    top = ellipse(img, 6.0, 15.9, 5.3, 3.4, disc_top)
    outline(img, side | top)

    # green button just right of the disc, at the front of the base
    for (x, y), c in {(12, 17): 0xf, (13, 17): 0xe}.items():
        img[y][x] = c

    # hinge cap: silver end of the hinge cylinder on the right
    cap = ellipse(img, 21.5, 12.6, 1.9, 1.9,
                  lambda nx, ny: 7 if (nx < -0.1 and ny < -0.1) else (8 if ny < 0.35 else 9))
    outline(img, cap)

    im = Image.new('P', (24, 24), 0)
    px = im.load()
    for y in range(24):
        for x in range(24):
            px[x, y] = img[y][x]
    flat = []
    for c in PAL:
        flat += list(c)
    im.putpalette(flat)
    im.save(os.path.join(REPO, 'graphics', 'items', 'icons', 'pokedex.png'), bits=4)
    with open(os.path.join(REPO, 'graphics', 'items', 'icon_palettes', 'pokedex.pal'), 'w', newline='\r\n') as f:
        f.write('JASC-PAL\n0100\n16\n')
        for r, g, b in PAL:
            f.write(f'{r} {g} {b}\n')

    if len(sys.argv) > 1:
        prev = Image.new('RGB', (24, 24), (248, 248, 248))
        pp = prev.load()
        for y in range(24):
            for x in range(24):
                if px[x, y]:
                    pp[x, y] = PAL[px[x, y]]
        prev.resize((192, 192), Image.NEAREST).save(sys.argv[1])


if __name__ == '__main__':
    main()
