"""Usage: python tech/pokedex_hgss_reskin.py  (requires the thexaman remote fetched)

Reskin PokedexPlus HGSS screens with Emerald's Pokedex tiles/colors.

Reads the original HGSS art from the thexaman/tx_pokedexPlus_hgss git ref, keeps
every screen's layout (panel geometry) and redraws it in Emerald's style using
Emerald's own palette indices, so the vanilla bg_hoenn/bg_national palettes apply.
"""
import os, struct, subprocess, sys
from collections import Counter, deque
from io import BytesIO
from PIL import Image

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')).replace('\\', '/')
REF = 'thexaman/tx_pokedexPlus_hgss'
SRC = 'graphics/pokedex/hgss/'
OUT = sys.argv[1] if len(sys.argv) > 1 else REPO + '/' + SRC
PREVIEW = os.environ.get('RESKIN_PREVIEW_DIR')  # optional: write RGB previews of every screen here

def git_bytes(path):
    return subprocess.run(['git', '-C', REPO, 'show', f'{REF}:{path}'], capture_output=True, check=True).stdout

def read_pal_text(text):
    lines = text.split('\n')
    n = int(lines[2])
    return [tuple(map(int, l.split())) for l in lines[3:3 + n]]

EM_PAL = read_pal_text(open(REPO + '/graphics/pokedex/bg_hoenn.pal').read())
EM_PAL_NAT = read_pal_text(open(REPO + '/graphics/pokedex/bg_national.pal').read())

# ---------------------------------------------------------------- HGSS input
def load_tiles(png_bytes):
    im = Image.open(BytesIO(png_bytes))
    px = im.load()
    w, h = im.size
    tiles = []
    for ty in range(h // 8):
        for tx in range(w // 8):
            tiles.append([[px[tx * 8 + x, ty * 8 + y] & 0xF for x in range(8)] for y in range(8)])
    return tiles

def tilemap_to_pixels(tiles, data, width=32):
    n = len(data) // 2
    rows = n // width
    img = [[0] * (width * 8) for _ in range(rows * 8)]
    for i in range(n):
        e = struct.unpack_from('<H', data, i * 2)[0]
        t, hf, vf = e & 0x3FF, (e >> 10) & 1, (e >> 11) & 1
        tile = tiles[t] if t < len(tiles) else [[0] * 8 for _ in range(8)]
        cx, cy = (i % width) * 8, (i // width) * 8
        for y in range(8):
            for x in range(8):
                img[cy + y][cx + x] = tile[7 - y if vf else y][7 - x if hf else x]
    return img, rows

# ---------------------------------------------------------------- Emerald style
STRIPE = [12, 13, 13, 12]          # Emerald green stripes, period 4 (by screen y)
BG_SET = {1, 2, 3, 4, 5}           # HGSS grid background colors
ACCENT = {6, 7}                    # HGSS red/pink

# Emerald select-bar font (5x9), taken from graphics/pokedex/menu.png; F, O, T, V drawn to match.
FONT = {
 'A': ['.###.', '#...#', '#...#', '#...#', '#...#', '#####', '#...#', '#...#', '#...#'],
 'R': ['####.', '#...#', '#...#', '#...#', '#...#', '####.', '#.#..', '#..#.', '#...#'],
 'E': ['#####', '#....', '#....', '#....', '####.', '#....', '#....', '#....', '#####'],
 'C': ['.###.', '#...#', '#....', '#....', '#....', '#....', '#....', '#...#', '.###.'],
 'Y': ['#...#', '#...#', '#...#', '.#.#.', '.#.#.', '..#..', '..#..', '..#..', '..#..'],
 'S': ['.###.', '#...#', '#....', '#....', '.###.', '....#', '....#', '#...#', '.###.'],
 'I': ['#####', '..#..', '..#..', '..#..', '..#..', '..#..', '..#..', '..#..', '#####'],
 'Z': ['#####', '....#', '...#.', '...#.', '..#..', '.#...', '.#...', '#....', '#####'],
 'N': ['#...#', '##..#', '##..#', '#.#.#', '#.#.#', '#..##', '#..##', '#...#', '#...#'],
 'F': ['#####', '#....', '#....', '#....', '####.', '#....', '#....', '#....', '#....'],
 'O': ['.###.', '#...#', '#...#', '#...#', '#...#', '#...#', '#...#', '#...#', '.###.'],
 'T': ['#####', '..#..', '..#..', '..#..', '..#..', '..#..', '..#..', '..#..', '..#..'],
 'V': ['#...#', '#...#', '#...#', '#...#', '.#.#.', '.#.#.', '.#.#.', '..#..', '..#..'],
}
# (label, first tile column, width in tiles); arrows live in columns 0 and 29
TABS = [('INFO', 1, 5), ('AREA', 6, 5), ('STATS', 11, 5), ('EVO', 16, 4), ('CRY', 20, 4), ('SIZE', 24, 5)]
ROW_SELECTED, ROW_UNSELECTED = 2, 4   # same rows vanilla uses for its select bar

def draw_bar(selected):
    """Return (pixels[16][240], rows[30]) for an Emerald-style tab bar."""
    W = 240
    px = [[7] * W for _ in range(16)]
    for x in range(W):
        px[15][x] = 8
    rows = [ROW_SELECTED] * 30
    for label, c0, cw in TABS:
        r = ROW_SELECTED if label == selected else ROW_UNSELECTED
        for c in range(c0, c0 + cw):
            rows[c] = r
        x0, x1 = c0 * 8 + 1, (c0 + cw) * 8 - 2  # inclusive outline columns
        y0, y1 = 1, 14
        for y in range(y0, y1 + 1):
            for x in range(x0, x1 + 1):
                px[y][x] = 12
        for x in range(x0 + 2, x1 - 1):
            px[y0][x] = 6; px[y1][x] = 6
        for y in range(y0 + 2, y1 - 1):
            px[y][x0] = 6; px[y][x1] = 6
        for (x, y) in [(x0 + 1, y0 + 1), (x1 - 1, y0 + 1), (x0 + 1, y1 - 1), (x1 - 1, y1 - 1)]:
            px[y][x] = 6
        for (x, y) in [(x0, y0), (x0 + 1, y0), (x0, y0 + 1), (x1, y0), (x1 - 1, y0), (x1, y0 + 1),
                       (x0, y1), (x0 + 1, y1), (x0, y1 - 1), (x1, y1), (x1 - 1, y1), (x1, y1 - 1)]:
            px[y][x] = 7
        tw = len(label) * 6 - 1
        tx = x0 + (x1 - x0 + 1 - tw) // 2
        ink = set()
        for i, ch in enumerate(label):
            for gy, line in enumerate(FONT[ch]):
                for gx, c in enumerate(line):
                    if c == '#':
                        ink.add((tx + i * 6 + gx, 3 + gy))
        for (x, y) in ink:
            for dx, dy in ((1, 0), (0, 1), (1, 1)):
                if (x + dx, y + dy) not in ink:
                    px[y + dy][x + dx] = 13
        for (x, y) in ink:
            px[y][x] = 15
    # L/R arrows
    for side, cx in (('L', 0), ('R', 29)):
        for i in range(4):
            h = i if side == 'L' else 3 - i
            x = cx * 8 + 2 + i
            for y in range(7 - h, 9 + h):
                px[y][x] = 1
    return px, rows

def reskin(H, rows_count, bar=None, flood=True, seeds_extra=()):
    """H = HGSS index image. Returns (E, accent_mask, bar_rows)."""
    Wpx, Hpx = 240, min(rows_count * 8, 160)
    full_h = rows_count * 8
    top = 16 if bar is not None else 0
    E = [row[:] for row in H]
    acc = [[False] * 256 for _ in range(full_h)]
    outside = [[False] * 256 for _ in range(full_h)]
    bgm = [[False] * 256 for _ in range(full_h)]
    # flood the HGSS grid background
    if flood:
        q = deque()
        seeds = [(x, top) for x in range(Wpx)] + [(x, Hpx - 1) for x in range(Wpx)] + \
                [(0, y) for y in range(top, Hpx)] + [(Wpx - 1, y) for y in range(top, Hpx)] + list(seeds_extra)
        for (x, y) in seeds:
            if H[y][x] in BG_SET and not bgm[y][x]:
                bgm[y][x] = True; q.append((x, y))
        while q:
            x, y = q.popleft()
            for nx, ny in ((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)):
                if top <= ny < Hpx and 0 <= nx < Wpx and not bgm[ny][nx] and H[ny][nx] in BG_SET:
                    bgm[ny][nx] = True; q.append((nx, ny))
    for y in range(full_h):
        for x in range(256):
            outside[y][x] = bgm[y][x] or H[y][x] == 0 or x >= Wpx or y >= Hpx or y < top
    # distance from outside (4-connected) for the 2px Emerald frame
    dist = [[0 if outside[y][x] else 99 for x in range(256)] for y in range(full_h)]
    q = deque((x, y) for y in range(full_h) for x in range(256) if outside[y][x])
    while q:
        x, y = q.popleft()
        for nx, ny in ((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)):
            if 0 <= ny < full_h and 0 <= nx < 256 and dist[ny][nx] > dist[y][x] + 1:
                dist[ny][nx] = dist[y][x] + 1; q.append((nx, ny))

    def run(x, y, dx, dy, colors=ACCENT):
        n = 0
        while 0 <= x < 256 and 0 <= y < full_h and H[y][x] in colors:
            n += 1; x += dx; y += dy
        return n

    def is_frame(x, y):
        # outermost border pixel of a panel: black, touching the outside, part of a long straight edge
        if not (0 <= x < 256 and 0 <= y < full_h) or H[y][x] != 15 or dist[y][x] != 1:
            return False
        return (run(x, y, 1, 0, {15}) + run(x, y, -1, 0, {15}) > 12 or
                run(x, y, 0, 1, {15}) + run(x, y, 0, -1, {15}) > 12)

    def next_to_frame(x, y):
        return any(is_frame(x + dx, y + dy) for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)))

    for y in range(top, full_h):
        for x in range(256):
            h = H[y][x]
            if bgm[y][x]:
                E[y][x] = STRIPE[y % 4]
                continue
            if h == 0 or x >= Wpx:
                E[y][x] = 0
                continue
            d = dist[y][x]
            if h == 15:
                E[y][x] = 5 if d == 2 and next_to_frame(x, y) else 6
            elif h in ACCENT:
                vr = run(x, y, 0, 1) + run(x, y, 0, -1) - 1
                hr = run(x, y, 1, 0) + run(x, y, -1, 0) - 1
                if vr >= 14 and hr <= 4:
                    acc[y][x] = True
                    E[y][x] = 4 if h == 7 else 3
                else:
                    E[y][x] = 7 if h == 7 else 5
            elif h in (8, 9, 11):
                E[y][x] = 8
            elif h == 10:
                E[y][x] = 5
            elif h in (1, 2, 3) and d == 2 and next_to_frame(x, y):
                E[y][x] = 5
            else:
                E[y][x] = {1: 1, 2: 2, 3: 3, 4: 4, 5: 5, 12: 12, 13: 7, 14: 14}.get(h, h)
    # yellow inner edge of vertical accents (Emerald's 4,4,3,3,2 gradient)
    for y in range(top, full_h):
        for x in range(1, 255):
            if not acc[y][x] and H[y][x] == 1 and (acc[y][x - 1] and E[y][x - 1] == 3 or acc[y][x + 1] and E[y][x + 1] == 3):
                acc[y][x] = True
                E[y][x] = 2
    # small black glyphs drawn directly on the background become white text with a dark shadow
    seen = set()
    for y0 in range(top, Hpx):
        for x0 in range(Wpx):
            if H[y0][x0] != 15 or (x0, y0) in seen:
                continue
            comp, q = [], deque([(x0, y0)])
            seen.add((x0, y0))
            while q:
                x, y = q.popleft(); comp.append((x, y))
                for dx in (-1, 0, 1):
                    for dy in (-1, 0, 1):
                        nx, ny = x + dx, y + dy
                        if top <= ny < Hpx and 0 <= nx < Wpx and (nx, ny) not in seen and H[ny][nx] == 15:
                            seen.add((nx, ny)); q.append((nx, ny))
            xs = [p[0] for p in comp]; ys = [p[1] for p in comp]
            if max(xs) - min(xs) > 14 or max(ys) - min(ys) > 10:
                continue
            if not any(bgm[y + dy][x + dx] for x, y in comp for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))):
                continue
            for x, y in comp:
                E[y][x] = 1
            inside = set(comp)
            for y in range(min(ys), max(ys) + 1):
                for x in range(min(xs), max(xs) + 1):
                    if (x, y) not in inside and not bgm[y][x] and H[y][x] in (1, 4):
                        bgm[y][x] = True
                        E[y][x] = STRIPE[y % 4]
            for x, y in comp:
                for dx, dy in ((1, 0), (0, 1), (1, 1)):
                    nx, ny = x + dx, y + dy
                    if bgm[ny][nx] and H[ny][nx] == 4:
                        E[ny][nx] = 6
    bar_rows = None
    if bar is not None:
        bpx, bar_rows = draw_bar(bar)
        for y in range(16):
            for x in range(256):
                E[y][x] = bpx[y][x] if x < 240 else 0
    return E, acc, bar_rows

# ---------------------------------------------------------------- retile
class Tileset:
    def __init__(self):
        self.tiles = [tuple([0] * 64)]
        self.index = {self.tiles[0]: (0, 0, 0)}
        self.conflicts = 0

    def add(self, pix):
        key = tuple(pix)
        if key in self.index:
            return self.index[key]
        t = len(self.tiles)
        self.tiles.append(key)
        for hf in (0, 1):
            for vf in (0, 1):
                k = tuple(pix[(7 - y if vf else y) * 8 + (7 - x if hf else x)] for y in range(8) for x in range(8))
                self.index.setdefault(k, (t, hf, vf))
        return (t, 0, 0)

def build_tilemap(ts, E, acc, rows_count, bar_rows=None, width=32, tile_rows=None):
    out = bytearray()
    for ty in range(rows_count):
        for tx in range(width):
            pix = [E[ty * 8 + y][tx * 8 + x] for y in range(8) for x in range(8)]
            row = 0
            if bar_rows is not None and ty < 2:
                row = bar_rows[tx] if tx < 30 else 0
            elif tile_rows is not None and tile_rows.get((tx, ty)) is not None:
                row = tile_rows[(tx, ty)]
            elif any(acc[ty * 8 + y][tx * 8 + x] for y in range(8) for x in range(8)):
                row = 3
                fixed = [{2: 1, 3: 5, 4: 5}.get(p, p) if not acc[ty * 8 + i // 8][tx * 8 + i % 8] else p for i, p in enumerate(pix)]
                if fixed != pix:
                    ts.conflicts += 1
                pix = fixed
            if not any(pix) and (tile_rows is None or (tx, ty) not in tile_rows):
                row = 0
            t, hf, vf = ts.add(pix)
            out += struct.pack('<H', t | (hf << 10) | (vf << 11) | (row << 12))
    return bytes(out)

def save_tileset(ts, path, pal=EM_PAL):
    n = len(ts.tiles)
    rows = (n + 15) // 16
    im = Image.new('P', (128, rows * 8), 0)
    px = im.load()
    for i, t in enumerate(ts.tiles):
        for p in range(64):
            px[(i % 16) * 8 + p % 8, (i // 16) * 8 + p // 8] = t[p]
    flat = []
    for c in pal[:16]:
        flat += list(c)
    im.putpalette(flat)
    im.save(path, bits=4)

def preview(binf_bytes, ts, name, pal, width=32, base=None):
    if not PREVIEW:
        return None
    data = binf_bytes
    n = len(data) // 2
    rows = n // width
    img = base.copy() if base else Image.new('RGB', (width * 8, rows * 8), (128, 0, 128))
    px = img.load()
    for i in range(n):
        e = struct.unpack_from('<H', data, i * 2)[0]
        t, hf, vf, p = e & 0x3FF, (e >> 10) & 1, (e >> 11) & 1, e >> 12
        tile = ts.tiles[t]
        for y in range(8):
            for x in range(8):
                c = tile[(7 - y if vf else y) * 8 + (7 - x if hf else x)]
                if c == 0 and base is not None:
                    continue
                if (i // width) * 8 + y < img.size[1]:
                    px[(i % width) * 8 + x, (i // width) * 8 + y] = pal[p * 16 + c] if c else (pal[0] if base is None else px[0, 0])
    img.crop((0, 0, 240, min(160, rows * 8))).resize((480, min(320, rows * 16)), Image.NEAREST).save(os.path.join(PREVIEW, name + '.png'))
    return img

# ---------------------------------------------------------------- groups
GROUPS = {
    'HGSS_tileset_menu1.png': [
        ('HGSS_tilemap_info_screen.bin', 'INFO'),
        ('HGSS_tilemap_stats_screen.bin', 'STATS'),
        ('HGSS_SelectBar.bin', 'AREA'),
    ],
    'HGSS_tileset_menu2.png': [
        ('HGSS_tilemap_evo_screen.bin', 'EVO'),
        ('HGSS_tilemap_evo_screen_PE.bin', 'EVO'),
        ('HGSS_tilemap_forms_screen.bin', 'EVO'),
    ],
    'HGSS_tileset_menu3.png': [
        ('HGSS_tilemap_cry_screen.bin', 'CRY'),
        ('HGSS_tilemap_size_screen.bin', 'SIZE'),
        ('HGSS_tilemap_local_screen.bin', None),
    ],
    'HGSS_tileset_menu_list.png': [
        ('HGSS_tilemap_list_screen.bin', None),
        ('HGSS_tilemap_list_screen_underlay.bin', None),
        ('HGSS_tilemap_start_menu.bin', None),
        ('HGSS_tilemap_start_menu_1.bin', None),
        ('HGSS_tilemap_start_menu_2.bin', None),
        ('HGSS_tilemap_start_menu_3.bin', None),
        ('HGSS_tilemap_start_menu_search_results.bin', None),
    ],
}
# Tile budget per tileset (each is loaded with 0x2000 bytes = 256 tiles)
TILE_LIMITS = {}
NO_FLOOD = {'HGSS_tilemap_list_screen_underlay.bin', 'HGSS_tilemap_start_menu.bin', 'HGSS_tilemap_start_menu_search_results.bin', 'HGSS_SelectBar.bin'}

def remap_interface(name):
    """HGSS interface sprites use red (7) and gray scrollbar (5); Emerald uses magenta (11) and green (12)."""
    im = Image.open(BytesIO(git_bytes(SRC + name)))
    px = im.load()
    w, h = im.size
    out = Image.new('P', (w, h), 0)
    opx = out.load()
    for y in range(h):
        for x in range(w):
            c = px[x, y] & 0xF
            opx[x, y] = {7: 11, 5: 12}.get(c, c)
    # scrollbar thumb (tile 3) exactly as Emerald draws it
    with Image.open(EM_DIR + 'interface.png') as em:
        epx = em.load()
        for y in range(8):
            for x in range(24, 32):
                opx[x, y] = epx[x, y] & 0xF
    flat = []
    for c in EM_PAL[:16]:
        flat += list(c)
    out.putpalette(flat)
    out.save(os.path.join(OUT, name), bits=4)

def copy_file(src, dst):
    with open(src, 'rb') as f:
        data = f.read()
    with open(os.path.join(OUT, dst), 'wb') as f:
        f.write(data)

def write_palettes_and_search():
    pd = REPO + '/graphics/pokedex/'
    for dst, src in [('HGSS_palette_default.pal', 'bg_hoenn.pal'), ('HGSS_palette_default_dark.pal', 'bg_hoenn.pal'),
                     ('HGSS_palette_national.pal', 'bg_national.pal'), ('HGSS_palette_national_dark.pal', 'bg_national.pal'),
                     ('HGSS_palette_search_results.pal', 'search_results_bg.pal'), ('HGSS_palette_search_results_dark.pal', 'search_results_bg.pal'),
                     ('HGSS_palette_search_menu.pal', 'search_menu.pal'), ('HGSS_palette_search_menu_dark.pal', 'search_menu.pal'),
                     ('HGSS_tileset_menu_search.png', 'search_menu.png'), ('HGSS_tileset_menu_search_DECA.png', 'search_menu.png'),
                     ('HGSS_tilemap_search_screen_hoenn.bin', 'search_menu_hoenn.bin'),
                     ('HGSS_tilemap_search_screen_national.bin', 'search_menu_national.bin')]:
        copy_file(pd + src, dst)

EM_DIR = REPO + '/graphics/pokedex/'

# Text in the game's normal font (ink 6, shadow 3), as baked into the Pokedex start menus
FONT_CHARS = {' ': 0x00, 'é': 0x1B, **{c: 0xBB + i for i, c in enumerate('ABCDEFGHIJKLMNOPQRSTUVWXYZ')}}

def render_text(text):
    import re
    with open(REPO + '/src/fonts.c', encoding='utf-8') as f:
        block = f.read().split('gFontNormalLatinGlyphWidths[] = {')[1].split('};')[0]
    widths = [int(v) for v in re.findall(r'\d+', block)]
    font = Image.open(REPO + '/graphics/fonts/latin_normal.png')
    fpx = font.load()
    ink, x = {}, 0
    for c in text:
        g = FONT_CHARS[c]
        cx, cy = (g % 16) * 16, (g // 16) * 16
        for yy in range(16):
            for xx in range(widths[g]):
                v = fpx[cx + xx, cy + yy]
                if v in (1, 2):
                    ink[(x + xx, yy)] = 6 if v == 1 else 3
        x += widths[g]
    return ink

MENU_WIDEN = 30          # main menu box grows left so "UNMARK ALL CAPTURED" fits
MENU_TEXT_X, MENU_TEXT_Y = 144 - MENU_WIDEN, 8   # the menu cursor sprite sits 8px left of MENU_TEXT_X

# Start-menu variants: bit 0 = "all seen" display flag on, bit 1 = "all captured" display flag on
MAIN_MENU_VARIANTS = {
    'HGSS_tilemap_start_menu.bin': 0,
    'HGSS_tilemap_start_menu_1.bin': 1,
    'HGSS_tilemap_start_menu_2.bin': 2,
    'HGSS_tilemap_start_menu_3.bin': 3,
}

def main_menu_items(variant):
    return ['LOCAL',
            ('UNMARK' if variant & 1 else 'MARK') + ' ALL SEEN',
            ('UNMARK' if variant & 2 else 'MARK') + ' ALL CAPTURED',
            'LIST TOP', 'LIST BOTTOM']

def build_main_menu(E, variant):
    """Main-page start menu: the 5-entry search-results box, widened, with new entries."""
    rows = len(E)
    box_l = min(x for y in range(rows) for x in range(240) if E[y][x] == 6)
    for y in range(rows):
        row = E[y]
        if not any(row[box_l:box_l + 8]):
            continue
        edge = row[box_l:box_l + 6]
        fill = row[box_l + 6]
        row[box_l - MENU_WIDEN:box_l - MENU_WIDEN + 6] = edge
        for x in range(box_l - MENU_WIDEN + 6, box_l + 7):
            row[x] = fill
        if fill == 1:
            for x in range(box_l + 7, 232):
                row[x] = 1
    for i, text in enumerate(main_menu_items(variant)):
        ink = render_text(text)
        assert MENU_TEXT_X + max(x for x, _ in ink) < 228, text
        for (x, y), v in ink.items():
            E[MENU_TEXT_Y + i * 16 + y][MENU_TEXT_X + x] = v

def draw_panel(E, x0, y0, x1, y1, fill=1):
    """Emerald's rounded panel: dark outline (6), gray inner line (5), filled interior. Bounds inclusive."""
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            E[y][x] = fill
    for x in range(x0 + 2, x1 - 1):
        E[y0][x] = 6; E[y1][x] = 6
        E[y0 + 1][x] = 5; E[y1 - 1][x] = 5
    for y in range(y0 + 2, y1 - 1):
        E[y][x0] = 6; E[y][x1] = 6
        E[y][x0 + 1] = 5; E[y][x1 - 1] = 5
    for (cx, cy, dx, dy) in ((x0, y0, 1, 1), (x1, y0, -1, 1), (x0, y1, 1, -1), (x1, y1, -1, -1)):
        E[cy][cx + dx] = 6
        E[cy + dy][cx] = 6
        E[cy + dy][cx + dx] = 6
        E[cy + dy][cx + 2 * dx] = 5
        E[cy + 2 * dy][cx + dx] = 5
        E[cy + 2 * dy][cx + 2 * dx] = 5

def build_local_screen():
    """Local area screen: yellow header panel (map, time, method) above a yellow species list,
    with Emerald-style button badges. Badge positions must match the text layout in
    LocalDex_Draw (src/pokedex_plus_hgss.c)."""
    E = [[STRIPE[y % 4] if x < 240 else 0 for x in range(256)] for y in range(168)]
    draw_panel(E, 4, 2, 235, 35, fill=10)
    draw_panel(E, 4, 38, 235, 145, fill=10)
    for letter, x, y in LOCAL_BADGES:
        draw_badge(E, x, y, letter)
    return E

# 5x7 letters for button badges
BADGE_FONT = {
    'A': ['.###.', '#...#', '#...#', '#####', '#...#', '#...#', '#...#'],
    'B': ['####.', '#...#', '#...#', '####.', '#...#', '#...#', '####.'],
    'L': ['#....', '#....', '#....', '#....', '#....', '#....', '#####'],
    'R': ['####.', '#...#', '#...#', '####.', '#.#..', '#..#.', '#...#'],
}
# (letter, x, y) of each 11x11 badge on the local screen
LOCAL_BADGES = [('L', 138, 6), ('R', 220, 6), ('A', 6, 148), ('B', 62, 148)]

def draw_badge(E, x0, y0, letter):
    """Magenta button badge (black outline, magenta fill, black letter), like the START/SELECT badges."""
    for y in range(11):
        inset = {0: 2, 1: 1, 9: 1, 10: 2}.get(y, 0)
        for x in range(inset, 11 - inset):
            edge = y in (0, 10) or x in (inset, 10 - inset) or (inset == 0 and x in (0, 10))
            E[y0 + y][x0 + x] = 15 if edge else 11
    for gy, row in enumerate(BADGE_FONT[letter]):
        for gx, c in enumerate(row):
            if c == '#':
                E[y0 + 2 + gy][x0 + 3 + gx] = 15

# Time-of-day icons (16x16 frames: MORNING, DAY, NIGHT) for the local screen's time sprite
TIME_ICON_PAL = [(0, 0, 0), (41, 57, 65), (255, 132, 0), (255, 214, 0), (255, 248, 168), (255, 255, 255),
                 (41, 57, 106), (230, 226, 160), (98, 148, 230), (255, 160, 120), (255, 0, 189), (16, 16, 16),
                 (74, 189, 74), (24, 115, 41), (0, 0, 0), (0, 0, 0)]
INK = 1          # dark outline / line colour
SUN_FILL = 3     # yellow
SUN_LIGHT = 4    # pale highlight
RISE_FILL = 2    # orange (morning sun)
MOON_FILL = 7    # pale moon

def build_time_icons():
    """16x16 frames: MORNING, DAY, NIGHT, L badge, R badge (the badges are used on the area screen).
    Line-art style: dark outlines, dash rays, horizon lines, crescent with sparkles."""
    import math
    im = Image.new('P', (16, 80), 0)
    px = im.load()

    def shape(fy, inside, fill):
        """Fill pixels where inside(x, y) and outline (4-neighbour boundary) them in INK."""
        mask = [[inside(x + 0.5, y + 0.5) for x in range(16)] for y in range(16)]
        for y in range(16):
            for x in range(16):
                if not mask[y][x]:
                    continue
                edge = any(not (0 <= x + dx < 16 and 0 <= y + dy < 16) or not mask[y + dy][x + dx]
                           for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)))
                px[x, fy + y] = INK if edge else fill

    def dash(fy, cx, cy, angle_deg, r0, r1):
        a = math.radians(angle_deg)
        steps = int((r1 - r0) * 4) + 1
        for t in range(steps):
            rr = r0 + t / 4
            x = int(round(cx + math.cos(a) * rr - 0.5))
            y = int(round(cy - math.sin(a) * rr - 0.5))
            if 1 <= x < 15 and 1 <= y < 15:
                px[x, fy + y] = INK

    # MORNING: half sun on the horizon, five rays, water lines below
    shape(0, lambda x, y: y <= 10 and math.hypot(x - 8, y - 10.5) <= 5.6, RISE_FILL)
    px[6, 7] = SUN_LIGHT
    px[7, 7] = SUN_LIGHT
    px[6, 8] = SUN_LIGHT
    for ang in (90, 45, 135, 12, 168):
        dash(0, 8, 10.5, ang, 7.0, 8.6)
    for x in range(1, 15):
        px[x, 10] = INK
    for x in range(3, 13):
        px[x, 12] = INK
    for x in range(5, 11):
        px[x, 14] = INK

    # DAY: round sun with eight dash rays, clear of the frame edges
    shape(16, lambda x, y: math.hypot(x - 8, y - 8) <= 3.9, SUN_FILL)
    px[6, 16 + 6] = SUN_LIGHT
    px[7, 16 + 6] = SUN_LIGHT
    px[6, 16 + 7] = SUN_LIGHT
    for i in range(8):
        dash(16, 8, 8, i * 45, 5.4, 6.9)

    # NIGHT: crescent moon opening to the upper left, two sparkles beside it
    shape(32, lambda x, y: math.hypot(x - 9.0, y - 8.5) <= 5.8 and math.hypot(x - 6.2, y - 7.0) > 4.6, MOON_FILL)
    BIG_SPARKLE = ['..#..', '..#..', '.###.', '#####', '.###.', '..#..', '..#..']
    SMALL_SPARKLE = ['.#.', '###', '.#.']
    for pattern, ox, oy in ((BIG_SPARKLE, 1, 5), (SMALL_SPARKLE, 5, 2)):
        for gy, row in enumerate(pattern):
            for gx, ch in enumerate(row):
                if ch == '#':
                    px[ox + gx, 32 + oy + gy] = INK

    # L / R badges, drawn like the background badges (magenta, near-black outline and letter)
    for frame, letter in ((3, 'L'), (4, 'R')):
        E = [[0] * 16 for _ in range(16)]
        draw_badge(E, 2, 2, letter)
        for y in range(16):
            for x in range(16):
                if E[y][x]:
                    px[x, frame * 16 + y] = {15: 11, 11: 10}[E[y][x]]
    flat = []
    for c in TIME_ICON_PAL:
        flat += list(c)
    im.putpalette(flat)
    im.save(os.path.join(OUT, 'local_time_icons.png'), bits=4)

def master_pixels(binf, png='menu.png', width=32):
    """Index image and per-tile palette rows of one of Emerald's own Pokedex tilemaps."""
    with open(EM_DIR + png, 'rb') as f:
        tiles = load_tiles(f.read())
    with open(EM_DIR + binf, 'rb') as f:
        data = f.read()
    E, rows = tilemap_to_pixels(tiles, data, width)
    prow = {}
    for i in range(len(data) // 2):
        prow[(i % width, i // width)] = struct.unpack_from('<H', data, i * 2)[0] >> 12
    return E, rows, prow

def edit_info(E):
    # Emerald's footprint circle makes room for the type icons; the footprint moves into the gray name band
    for y in range(56, 90):
        for x in range(186, 224):
            if E[y][x] == 2:
                E[y][x] = 1

MASTER_BODIES = {
    'HGSS_tilemap_info_screen.bin': ('info_screen.bin', edit_info),
    'HGSS_tilemap_cry_screen.bin': ('cry_screen.bin', None),
    'HGSS_tilemap_size_screen.bin': ('size_screen.bin', None),
}

def master_screen(binf, bar, rows_count):
    """Emerald's own screen (palette rows kept) below the Emerald-style tab bar."""
    src, edit = MASTER_BODIES[binf]
    M, mrows, prow = master_pixels(src)
    if edit:
        edit(M)
    E = [[0] * 256 for _ in range(rows_count * 8)]
    tile_rows = {}
    for ty in range(2, min(mrows, rows_count)):
        for tx in range(32):
            tile_rows[(tx, ty)] = prow[(tx, ty)]
            for y in range(8):
                E[ty * 8 + y][tx * 8:tx * 8 + 8] = M[ty * 8 + y][tx * 8:tx * 8 + 8]
    bpx, bar_rows = draw_bar(bar)
    for y in range(16):
        E[y][:240] = bpx[y]
    acc = [[False] * 256 for _ in range(rows_count * 8)]
    return E, acc, bar_rows, tile_rows

# List screen: [seen + stat boxes | mon box | Emerald's yellow list], with Emerald's POKeDEX banner on top.
BOXES_SRC_X, BOXES_DST_X, BOXES_W, BOXES_DY = 180, 0, 60, 4
MONBOX_SRC_X, MONBOX_DST_X, MONBOX_W, MONBOX_Y0, MONBOX_Y1 = 112, 60, 68, 22, 138
LIST_X = 128
POINTER_Y0, POINTER_Y1 = 73, 87

def compose_list(E1, acc1, E3, rows_count):
    L, _, _ = master_pixels('list.bin')
    U, _, _ = master_pixels('list_underlay.bin')
    h = rows_count * 8
    N1 = [[STRIPE[y % 4] if x < 240 else 0 for x in range(256)] for y in range(h)]
    N3 = [row[:] for row in N1]
    A1 = [[False] * 256 for _ in range(h)]
    for y in range(BOXES_DY, 160):
        for x in range(BOXES_W):
            N1[y][BOXES_DST_X + x] = E1[y - BOXES_DY][BOXES_SRC_X + x]
            A1[y][BOXES_DST_X + x] = acc1[y - BOXES_DY][BOXES_SRC_X + x]
    for y in range(160):
        for x in range(MONBOX_W):
            if MONBOX_Y0 <= y < MONBOX_Y1:
                N1[y][MONBOX_DST_X + x] = E1[y][MONBOX_SRC_X + x]
            N3[y][MONBOX_DST_X + x] = E3[y][MONBOX_SRC_X + x]
    for y in range(160):
        for x in range(LIST_X, 240):
            N1[y][x] = L[y][x]
            N3[y][x] = U[y][x]
        # Emerald's mon box frame sat at x 128-129; ours ends at 126
        if not POINTER_Y0 <= y < POINTER_Y1:
            for x in (LIST_X, LIST_X + 1):
                N1[y][x] = STRIPE[y % 4]
    # white pointer from the mon box to the selected entry, as in Emerald
    for y in range(POINTER_Y0, POINTER_Y1):
        for x in (LIST_X - 2, LIST_X - 1):
            N1[y][x] = 1
    # widen the selection bar to the left so the caught ball sits on it
    for y in range(72, 88):
        left = LIST_X + 8 + {72: 2, 73: 1, 86: 1, 87: 2}.get(y, 0)
        for x in range(left, LIST_X + 14):
            N3[y][x] = 1
    # Emerald's POKeDEX banner
    for y in range(8, 16):
        for x in range(8, 128):
            N1[y][x] = L[y][x]
    return N1, A1, N3

def main():
    if PREVIEW:
        os.makedirs(PREVIEW, exist_ok=True)
    remap_interface('HGSS_tileset_interface.png')
    remap_interface('HGSS_tileset_interface_DECA.png')
    write_palettes_and_search()
    build_time_icons()
    for tileset, screens in GROUPS.items():
        src_tiles = load_tiles(git_bytes(SRC + tileset))
        ts = Tileset()
        results = []
        if tileset == 'HGSS_tileset_menu_list.png':
            H1, rows = tilemap_to_pixels(src_tiles, git_bytes(SRC + 'HGSS_tilemap_list_screen.bin'))
            H3, _ = tilemap_to_pixels(src_tiles, git_bytes(SRC + 'HGSS_tilemap_list_screen_underlay.bin'))
            E1, acc1, _ = reskin(H1, rows)
            E3, _, _ = reskin(H3, rows, flood=False)
            N1, A1, N3 = compose_list(E1, acc1, E3, rows)
            composed = {'HGSS_tilemap_list_screen.bin': (N1, A1), 'HGSS_tilemap_list_screen_underlay.bin': (N3, None)}
        for binf, bar in screens:
            tile_rows = None
            bar_rows = None
            if binf == 'HGSS_tilemap_local_screen.bin':
                E = build_local_screen()
                rows = len(E) // 8
                acc = [[False] * 256 for _ in range(rows * 8)]
                tm = build_tilemap(ts, E, acc, rows)
                results.append((binf, tm))
                continue
            if binf in MAIN_MENU_VARIANTS:
                H, rows = tilemap_to_pixels(src_tiles, git_bytes(SRC + 'HGSS_tilemap_start_menu_search_results.bin'))
                E, acc, bar_rows = reskin(H, rows, flood=False)
                build_main_menu(E, MAIN_MENU_VARIANTS[binf])
                tm = build_tilemap(ts, E, acc, rows, bar_rows)
                results.append((binf, tm))
                continue
            data = git_bytes(SRC + binf)
            H, rows = tilemap_to_pixels(src_tiles, data)
            if binf == 'HGSS_SelectBar.bin':
                E = [[0] * 256 for _ in range(rows * 8)]
                acc = [[False] * 256 for _ in range(rows * 8)]
                bpx, bar_rows = draw_bar(bar)
                for y in range(16):
                    for x in range(240):
                        E[y][x] = bpx[y][x]
            elif binf in MASTER_BODIES:
                E, acc, bar_rows, tile_rows = master_screen(binf, bar, rows)
            elif tileset == 'HGSS_tileset_menu_list.png' and binf in composed:
                E, acc = composed[binf]
                acc = acc or [[False] * 256 for _ in range(rows * 8)]
            else:
                E, acc, bar_rows = reskin(H, rows, bar=bar, flood=binf not in NO_FLOOD)
            tm = build_tilemap(ts, E, acc, rows, bar_rows, tile_rows=tile_rows)
            results.append((binf, tm))
        print(f'{tileset}: {len(ts.tiles)} tiles, {ts.conflicts} row-3 conflict tiles')
        assert len(ts.tiles) <= TILE_LIMITS.get(tileset, 256), 'tileset too large for its load size'
        save_tileset(ts, os.path.join(OUT, tileset))
        if tileset == 'HGSS_tileset_menu_list.png':
            save_tileset(ts, os.path.join(OUT, 'HGSS_tileset_menu_list_DECA.png'))
        for binf, tm in results:
            open(os.path.join(OUT, binf), 'wb').write(tm)
            preview(tm, ts, binf[:-4], EM_PAL)
            preview(tm, ts, binf[:-4] + '_national', EM_PAL_NAT)
        if tileset == 'HGSS_tileset_menu_list.png':
            under = dict(results)
            base = preview(under['HGSS_tilemap_list_screen_underlay.bin'], ts, 'tmp_under', EM_PAL)
            preview(under['HGSS_tilemap_list_screen.bin'], ts, 'list_composite', EM_PAL, base=base)

if __name__ == '__main__':
    main()
