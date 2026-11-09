"""Generate the Kanto Pokémon Center / Mart sign and emblem light sprites from the building art.

    python kanto_signs.py     # writes graphics/object_events/pics/misc/kanto_*_light.png, prints sprite offsets

Kanto PCs and Marts (KantoGeneral) draw the plaque left of the door and a larger emblem centered over it, so
the Hoenn sprites don't fit. Each sprite is 32x32 and drawn with its center at the door tile's top-left
corner + the printed offset (see sKantoLightCenters in src/event_object_movement.c and night_map.py).
Colors use the neon palette (graphics/object_events/palettes/neon_light.pal): reds 8-12, blues 1-7.
"""
import math
import os

from kit import N, Image, render

PICS = N.rpath('graphics/object_events/pics/misc')
NEON = N.rpath('graphics/object_events/palettes/neon_light.pal')
LIGHT = N.rpath('graphics/object_events/palettes/light.pal')  # warm lantern glow
MART_NEON = N.rpath('graphics/object_events/palettes/mart_light.pal')  # brighter reds 8-12 (Kanto PC, Network Center)
# reference buildings: (map, door tile)
PC_DOOR = ('ViridianCity', (26, 26))
MART_DOOR = ('ViridianCity', (36, 19))


def lum(c):
    return c[0] * 0.3 + c[1] * 0.59 + c[2] * 0.11


def is_red(c):
    return c[0] > 120 and c[0] - max(c[1], c[2]) > 50


def is_blue(c):
    return c[2] > 120 and c[2] - c[0] > 50


def area(mp, door):
    """Day pixels of the 6x4 tiles around a door: x door-3..door+2, y door-3..door. Returns (img, origin px)."""
    dx, dy = door
    box = (dx - 3, dy - 3, dx + 3, dy + 1)
    return render(mp, box, False, 1, grid=False).convert('RGB'), (box[0] * 16, box[1] * 16), (dx * 16, dy * 16)


def rank_map(colors, idxs):
    """Map colors to palette indices by brightness rank (darkest -> idxs[0])."""
    cs = sorted(set(colors), key=lum)
    return {c: idxs[min(len(idxs) - 1, i * len(idxs) // max(1, len(cs)))] for i, c in enumerate(cs)}


def save(name, px, pal):
    im = Image.new('P', (32, 32), 0)
    im.putpalette([v for c in pal for v in c])
    im.info['transparency'] = 0
    for (x, y), c in px.items():
        im.putpixel((x, y), c)
    im.save(os.path.join(PICS, name))


def sign(img, origin, door_px, pred, letter_idx, fill, frame):
    """Plaque left of the door: the letter cluster's bbox grown to the plaque (light pixels around it)."""
    W, H = img.size
    # plaque search: left of the door, door row
    dx, dy = door_px[0] - origin[0], door_px[1] - origin[1]
    letters = [(x, y) for y in range(dy - 2, dy + 16) for x in range(dx - 40, dx) if 0 <= y < H and pred(img.getpixel((x, y)))]
    x0, x1 = min(p[0] for p in letters), max(p[0] for p in letters)
    y0, y1 = min(p[1] for p in letters), max(p[1] for p in letters)
    # grow over the plaque background (light, unsaturated) by up to 3 px
    def plaque(c):
        return lum(c) > 150 and not pred(c)
    while x0 > 0 and plaque(img.getpixel((x0 - 1, (y0 + y1) // 2))) and (letters[0][0] - x0) < 3:
        x0 -= 1
    while x1 < W - 1 and plaque(img.getpixel((x1 + 1, (y0 + y1) // 2))) and x1 - max(p[0] for p in letters) < 3:
        x1 += 1
    y0 -= 1
    y1 += 1
    cmap = rank_map([img.getpixel(p) for p in letters], letter_idx)
    cx, cy = (x0 + x1 + 1) // 2, (y0 + y1 + 1) // 2
    px = {}
    for y in range(y0 - 1, y1 + 2):
        for x in range(x0 - 1, x1 + 2):
            c = img.getpixel((x, y))
            v = cmap[c] if pred(c) else (frame if (x in (x0 - 1, x1 + 1) or y in (y0 - 1, y1 + 1)) else fill)
            px[(x - cx + 16, y - cy + 16)] = v
    return px, (origin[0] + cx - door_px[0], origin[1] + cy - door_px[1])


def emblem(img, origin, door_px, pred, ball_idx, fill, ring, rect):
    """Ball above the door: colored pixels lit by brightness rank, the white parts inside it dimly filled,
    and a 1 px ring around it (ambient glow, like the Hoenn emblems). rect: (x0, y0, x1, y1) search box
    relative to the door tile's top-left (the roof above shares the ball's colors)."""
    W, H = img.size
    dx, dy = door_px[0] - origin[0], door_px[1] - origin[1]
    pts = [(x, y) for y in range(dy + rect[1], dy + rect[3]) for x in range(dx + rect[0], dx + rect[2])
           if pred(img.getpixel((x, y)))]
    xs, ys = [p[0] for p in pts], [p[1] for p in pts]
    cx, cy = (min(xs) + max(xs) + 1) / 2, (min(ys) + max(ys) + 1) / 2
    r = max(max(xs) - min(xs) + 1, max(ys) - min(ys) + 1) / 2
    cmap = rank_map([img.getpixel(p) for p in pts], ball_idx)
    ocx, ocy = int(cx), int(cy)
    px = {}
    for y in range(int(cy - r - 2), int(cy + r + 3)):
        for x in range(int(cx - r - 2), int(cx + r + 3)):
            d = math.hypot(x + 0.5 - cx, y + 0.5 - cy)
            c = img.getpixel((x, y)) if 0 <= x < W and 0 <= y < H else None
            if c is not None and pred(c) and d <= r + 0.5:
                v = cmap[c]
            elif d <= r - 0.3:
                v = fill
            elif d <= r + 0.9:
                v = ring
            else:
                continue
            sx, sy = x - ocx + 16, y - ocy + 16
            if 0 <= sx < 32 and 0 <= sy < 32:
                px[(sx, sy)] = v
    return px, (origin[0] + ocx - door_px[0], origin[1] + ocy - door_px[1])


def game_corner():
    """Celadon Game Corner: the striped fan above the door (amber/cream) with a dim 1 px ring around it, and the
    7 dots on the arch, centered
    on tile (34, 20) top-left + (8, 12)."""
    img = render('CeladonCity', (33, 19, 36, 22), False, 1, grid=False).convert('RGB')
    ox, oy = 33 * 16, 19 * 16
    cx, cy = 34 * 16 + 8, 20 * 16 + 12
    fan = {}
    for y in range(20 * 16 + 9, 20 * 16 + 16):
        for x in range(34 * 16, 35 * 16):
            c = img.getpixel((x - ox, y - oy))
            if c in ((214, 173, 74), (247, 239, 148)):
                fan[(x, y)] = 14 if lum(c) > 200 else 13
    px = {}
    # the 7 dots on the arch around it (same pale yellow), bright
    for y in range(cy - 16, cy + 16):
        for x in range(cx - 16, cx + 16):
            if (x, y) not in fan and img.getpixel((x - ox, y - oy)) == (247, 239, 148):
                px[(x - cx + 16, y - cy + 16)] = 14
    bottom = max(y for _, y in fan)
    for (x, y), v in fan.items():
        px[(x - cx + 16, y - cy + 16)] = v
        for dx in (-1, 0, 1):
            for dy in (-1, 0, 1):
                p = (x + dx, y + dy)
                if p not in fan and p[1] <= bottom:
                    px.setdefault((p[0] - cx + 16, p[1] - cy + 16), 15)
    return px, (8, 12)


def joyful_game_corner():
    """Two Island Joyful Game Corner: the 3 dots on the marquee, bright with an amber halo. Centered on tile
    (39, 8) top-left + (8, 10)."""
    img = render('TwoIsland', (38, 8, 42, 10), False, 1, grid=False).convert('RGB')
    ox, oy = 38 * 16, 8 * 16
    cx, cy = 39 * 16 + 8, 8 * 16 + 10
    dots = {}
    for y in range(133, 142):
        for x in range(cx - 16, cx + 16):
            c = img.getpixel((x - ox, y - oy))
            if c in ((255, 255, 255), (239, 231, 74), (239, 189, 0)):
                dots[(x, y)] = 13 if c == (239, 189, 0) else 14
    px = {}
    for (x, y) in dots:
        for dx in range(-2, 3):
            for dy in range(-2, 3):
                p = (x + dx, y + dy)
                if p not in dots and dx * dx + dy * dy <= 5:
                    px[(p[0] - cx + 16, p[1] - cy + 16)] = 15
    for (x, y), v in dots.items():
        px[(x - cx + 16, y - cy + 16)] = v
    return px, (8, 10)


def network_center():
    """One Island Network Center: the white arcs on the roof as red neon tubes (mart_light.pal reds 8-12, like the
    Kanto PC), glow between them, the dark semicircle behind them a dim plate and a 1 px halo around it. Centered
    on tile (14, 3) top-left + (8, 8)."""
    img = render('OneIsland', (13, 2, 16, 5), False, 1, grid=False).convert('RGB')
    ox, oy = 13 * 16, 2 * 16
    cx, cy = 14 * 16 + 8, 3 * 16 + 8
    ecx, ecy, rx, ry = 232, 62.5, 16, 13.6  # the semicircle (absolute px)

    def inside(x, y, grow=0):
        return y <= 62 and ((x + 0.5 - ecx) / (rx + grow)) ** 2 + ((y + 0.5 - ecy) / (ry + grow)) ** 2 <= 1
    arcs = {(x, y) for y in range(49, 63) for x in range(216, 248)
            if inside(x, y) and img.getpixel((x - ox, y - oy)) == (239, 239, 247)}
    px = {}
    for y in range(cy - 16, cy + 16):
        for x in range(cx - 16, cx + 16):
            if (x, y) in arcs:
                v = 11
            elif inside(x, y):
                near = any((x + dx, y + dy) in arcs for dx in (-1, 0, 1) for dy in (-1, 0, 1))
                v = 9 if near else 8
            elif inside(x, y, 1.2):
                v = 8
            else:
                continue
            px[(x - cx + 16, y - cy + 16)] = v
    return px, (8, 8)


def is_outline(c):
    """Badge outlines: dark, unsaturated greys and navies."""
    return lum(c) < 135 and max(c) - min(c) < 60


def badge(img, origin, door_px, seed, wing_bottom, ball, ball_map, plate_map):
    """The whole white badge over a PC/Mart door, lit: the Poke Ball (an ellipse (cx, cy, rx, ry)) via ball_map,
    the rest of the badge (flood-filled from seed, down to row wing_bottom, inside the dark outline) via
    plate_map. Outlines stay unlit. All positions relative to the door tile's top-left."""
    W, H = img.size
    dx, dy = door_px[0] - origin[0], door_px[1] - origin[1]
    bcx, bcy, rx, ry = ball
    bcx, bcy = bcx + dx, bcy + dy

    def in_ball(x, y):
        return ((x + 0.5 - bcx) / rx) ** 2 + ((y + 0.5 - bcy) / ry) ** 2 <= 1
    lit = {}
    stack, seen = [(dx + seed[0], dy + seed[1])], set()
    while stack:
        x, y = stack.pop()
        if (x, y) in seen or not (0 <= x < W and 0 <= y < H) or y > dy + wing_bottom:
            continue
        seen.add((x, y))
        c = img.getpixel((x, y))
        # badge = light, unsaturated pixels (stops at the outline, the roof and walls' colors) or the ball
        if abs(x + 0.5 - bcx) > 16 or (not in_ball(x, y) and (lum(c) < 150 or max(c) - min(c) >= 60)):
            continue
        if not in_ball(x, y):
            lit[(x, y)] = plate_map(c)
        stack += [(x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)]
    for y in range(int(bcy - ry), int(bcy + ry) + 1):
        for x in range(int(bcx - rx), int(bcx + rx) + 1):
            if 0 <= x < W and 0 <= y < H and in_ball(x, y) and not is_outline(img.getpixel((x, y))):
                lit[(x, y)] = ball_map(img.getpixel((x, y)))
    lit = {p: v for p, v in lit.items() if v}
    xs, ys = [p[0] for p in lit], [p[1] for p in lit]
    cx, cy = (min(xs) + max(xs) + 1) // 2, (min(ys) + max(ys) + 1) // 2
    px = {(x - cx + 16, y - cy + 16): v for (x, y), v in lit.items() if 0 <= x - cx + 16 < 32 and 0 <= y - cy + 16 < 32}
    assert len(px) == len(lit), 'badge larger than 32x32'
    return px, (cx - dx, cy - dy)


def mart_ball(c):
    """Like the Hoenn Mart emblem: whites brightest cyan, blues mid."""
    lv = lum(c)
    return 6 if lv >= 200 else 5 if lv >= 165 else 4 if lv >= 140 else 3 if lv >= 110 else 2


def pc_ball_map(img, origin, door_px, ball):
    """Neon like the Mart badge (bright reds in mart_light.pal 8-12): whites brightest, reds by brightness rank."""
    dx, dy = door_px[0] - origin[0], door_px[1] - origin[1]
    reds = [img.getpixel((x, y)) for y in range(dy + ball[1] - ball[3], dy + ball[1] + ball[3])
            for x in range(dx + ball[0] - ball[2], dx + ball[0] + ball[2]) if is_red(img.getpixel((x, y)))]
    cmap = rank_map(reds, (10, 11))
    return lambda c: cmap[c] if c in cmap else 12 if lum(c) >= 200 else 9


def lighthouse():
    """Warm round glow for the Vermilion dock lighthouses' lamp rooms (light.pal: 1 dim .. 15 bright)."""
    px = {}
    for y in range(32):
        for x in range(32):
            d = math.hypot(x + 0.5 - 16, y + 0.5 - 16)
            if d <= 3:
                px[(x, y)] = 15
            elif d <= 11:
                px[(x, y)] = max(1, 15 - round((d - 3) / 8 * 14))
    return px, (0, 10)


def place():
    """Add Kanto PC/Mart sign lights wherever the building front is drawn (also on copy maps like
    SaffronCity_Connection, which have no warps) and the Game Corner light. Idempotent."""
    from kit import MAPS, blocks, add_light
    import doors
    copies = doors.lit_copies()
    secs = N.mapsec_regions()
    N.assign_regions(MAPS, secs)
    for m in sorted(MAPS.values(), key=lambda m: m['_name']):
        lay = m['_layout']
        if (not lay or lay['primary_tileset'] != 'gTileset_KantoGeneral' or m.get('map_type') not in N.NATURAL_LIGHT_TYPES
                or N.region_of(m, secs) not in ('kanto', 'sevii')):
            continue
        b = blocks(m['_name'])
        for y in range(2, len(b)):
            for x in range(len(b[0])):
                g = copies.get((lay['secondary_tileset'], b[y][x]), b[y][x])
                if g == 0x62 and b[y - 2][x] in (0x52, 0x32):
                    add_light(m['_name'], x, y, 7 if b[y - 2][x] == 0x52 else 8)
    add_light('CeladonCity', 34, 20, 11)
    for x in (21, 26):  # Vermilion dock lighthouses
        add_light('VermilionCity', x, 32, 12)
    add_light('LilycoveCity', 61, 33, 13)  # Lilycove's small lighthouse (Hoenn, same light types)
    add_light('TwoIsland', 39, 8, 14)  # Joyful Game Corner dots
    add_light('OneIsland', 14, 3, 15)  # Network Center emblem


def main():
    out = {}
    img, o, d = area(*PC_DOOR)
    out['kanto_poke_center_light.png'] = sign(img, o, d, is_red, (10, 11, 12), 9, 8)
    ball = (8, -9, 7, 9)  # Poke Ball ellipse (cx, cy, rx, ry) relative to the door tile's top-left
    out['kanto_poke_center_emblem_light.png'] = badge(img, o, d, (-4, -10), -5, ball, pc_ball_map(img, o, d, ball),
                                                      lambda c: 9 if lum(c) >= 150 else 0)
    img, o, d = area(*MART_DOOR)
    out['kanto_mart_light.png'] = sign(img, o, d, is_red, (10, 11, 12), 9, 8)  # red letters, as in Kanto
    out['kanto_mart_emblem_light.png'] = badge(img, o, d, (-12, -10), -5, (0, -9, 7, 9), mart_ball,
                                               lambda c: 3 if lum(c) >= 150 else 0)
    out['game_corner_light.png'] = game_corner()
    out['lighthouse_light.png'] = lighthouse()
    out['joyful_game_corner_light.png'] = joyful_game_corner()
    out['network_center_emblem_light.png'] = network_center()
    for name, (px, center) in out.items():
        pals = {'lighthouse_light.png': LIGHT, 'network_center_emblem_light.png': MART_NEON}
        save(name, px, N.read_jasc(pals.get(name, NEON)))
        print(f'{name}: center = tile top-left + {center}')
    place()


if __name__ == '__main__':
    main()
