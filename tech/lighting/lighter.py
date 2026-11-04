"""Pixel-precise night glow for secondary tilesets.

    from lighter import *
    L = Lighter('gTileset_Rustboro')
    L.glow(0x2b5, [(2, 0, 14, 11)], idx_filter={7})          # light metatile pixels (rects in 0..16 coords)
    L.glow_overlay(0x2ac, [(0, 11, 16, 16)], {7})             # same, via a lit overlay on the top layer
    L.save()

How lighting works (see README.md): a color glows at night if its index is listed in the palette's .pla.
Dark colors only blend halfway toward the light, so glow() also gives the lit index a warm night color
through the tileset's night palette (.swapPalettes -> palette file (P + 9) % 16). Day colors never change.
glow() never edits shared tiles: it writes a modified copy into a free tile and repoints the metatile.
"""
import subprocess

from kit import *  # noqa: F401,F403
from kit import N, TS, tile_px, OUT, HERE, Image, os, re, struct

HEADERS = N.rpath('src/data/tilesets/headers.h')
HEAD_DIR = os.path.join(HERE, '.head', 'data', 'tilesets')
WARM = (31, 27, 18)  # 5-bit night target for dark glass / doorways


def snapshot_head(rev='HEAD'):
    """Extract data/tilesets at a git revision into tech/lighting/.head (used by restore_* helpers)."""
    dst = os.path.join(HERE, '.head')
    os.makedirs(dst, exist_ok=True)
    tar = os.path.join(dst, 'ts.tar')
    subprocess.run(['git', 'archive', rev, 'data/tilesets', '-o', tar], cwd=N.ROOT, check=True)
    subprocess.run(['tar', '-xf', tar, '-C', dst], check=True)
    os.remove(tar)


def lum(c):
    return (c[0] * 3 + c[1] * 6 + c[2]) / 10 / 8


def default_night(day8):
    """Night color for a lit pixel: light colors keep their color (the lit blend warms them),
    dark ones get a warm bright color so they read as a lit window or doorway."""
    l = lum(day8)
    if l >= 20:
        return tuple(day8)
    k = 0.78 + 0.22 * (l / 20)
    return tuple(min(255, int(round(v * k)) << 3) for v in WARM)


def set_swap(ts_name, pal):
    """Add an absolute palette number to a tileset's .swapPalettes in headers.h."""
    text = open(HEADERS, encoding='utf-8').read()
    m = re.search(r'(const struct Tileset %s\s*=\s*\{)(.*?)(\};)' % re.escape(ts_name), text, re.S)
    body = m.group(2)
    sm = re.search(r'\n([ \t]*)\.swapPalettes[ \t]*=[ \t]*([^\n]*?),[ \t]*(?=\n)', body)
    cur = set()
    if sm:
        cur = {int(x) for a in re.findall(r'\(([^()]*)\)', sm.group(2)) for x in re.findall(r'\d+', a)}
    if pal in cur:
        return
    cur.add(pal)
    vals = sorted(cur)
    parts = []
    while vals:
        chunk, vals = vals[:4], vals[4:]
        macro = {1: 'SWAP_PAL', 2: 'SWAP_PALS', 3: 'SWAP_PALS3', 4: 'SWAP_PALS4'}[len(chunk)]
        parts.append('%s(%s)' % (macro, ', '.join(map(str, chunk))))
    expr = ' | '.join(parts)
    if sm:
        body = body.replace(sm.group(0), '\n%s.swapPalettes = %s,' % (sm.group(1), expr))
    else:
        body = re.sub(r'(\n([ \t]*)\.isCompressed[ \t]*=[ \t]*[^,\n]+,)',
                      lambda mm: mm.group(1) + '\n%s.swapPalettes = %s,' % (mm.group(2), expr), body, count=1)
    text = text[:m.start(2)] + body + text[m.end(2):]
    open(HEADERS, 'w', encoding='utf-8', newline='').write(text)


class Lighter:
    def __init__(self, sec_name):
        if sec_name == 'gTileset_General':
            raise ValueError('Lighter works on secondary tilesets; edit the primary with TS directly')
        self.gen = TS('gTileset_General')
        self.ts = TS(sec_name)
        self.name = sec_name
        self.copies = {}
        self._free_tiles = None

    # ---------------- palettes
    def used(self, P):
        """Color indices drawn by any tile using palette P in this tileset."""
        u = set()
        for m in range(self.ts.n()):
            for (t, h, v, p) in self.ts.entries(m + 512):
                if p == P and t:
                    for r in tile_px(self.gen, self.ts, t):
                        u.update(r)
        return u

    def night_path_idx(self, P):
        return (P + 9) % 16

    def ensure_swap(self, P):
        if P in self.ts.swap:
            return
        self.ts.write_pal(self.night_path_idx(P), self.ts.pal(P))
        set_swap(self.name, P)
        self.ts.swap.add(P)

    def _looks(self, P, c):
        """(day5, lit, night5) of index c in palette P as the game shows it."""
        src = self.gen if P < 6 else self.ts
        cols, pla = src.pal(P), src.pla(P)
        night = src.pal((P + 9) % 16)[c] if P in src.swap else cols[c]
        f = lambda col: tuple(v >> 3 for v in col)
        return f(cols[c]), c in pla, f(night)

    def alloc(self, P, day8, night8, reserve=()):
        """Index in palette P with day color day8, lit, night color night8 (reuses or claims a free slot)."""
        assert P >= 6, 'secondary palettes only'
        cols, pla = self.ts.pal(P), self.ts.pla(P)
        ncols = self.ts.pal(self.night_path_idx(P)) if P in self.ts.swap else None
        for j in range(1, 16):
            if j in pla and tuple(cols[j]) == tuple(day8):
                if tuple(ncols[j] if ncols else cols[j]) == tuple(night8):
                    return j
        used = self.used(P) | set(reserve)
        for j in range(1, 16):
            if j not in used and j not in pla:
                cols[j] = day8
                self.ts.write_pal(P, cols)
                if tuple(night8) != tuple(day8) or P in self.ts.swap:
                    self.ensure_swap(P)
                    ncols = self.ts.pal(self.night_path_idx(P))
                    ncols[j] = night8
                    self.ts.write_pal(self.night_path_idx(P), ncols)
                pla.add(j)
                self.ts.write_pla(P, pla)
                return j
        raise RuntimeError(f'{self.name}: palette {P} has no free index')

    def orphans(self, P):
        """Lit (.pla) indices of palette P that no metatile in this tileset draws (left over from earlier edits)."""
        return sorted(set(self.ts.pla(P)) - self.used(P))

    def reclaim(self, P, idxs=None):
        """Un-light orphaned indices so alloc can reuse them. Check first that no door animation
        (src/field_door.c sDoorAnimPalettes_*) for this tileset uses palette P."""
        idxs = set(self.orphans(P) if idxs is None else idxs)
        assert not idxs & self.used(P), 'index still drawn by a tile'
        self.ts.write_pla(P, set(self.ts.pla(P)) - idxs)
        return sorted(idxs)

    def _can_alloc(self, P, want):
        cols, pla = self.ts.pal(P), self.ts.pla(P)
        ncols = self.ts.pal(self.night_path_idx(P)) if P in self.ts.swap else cols
        need = 0
        for day8, n8 in set(want.values()):
            if not any(j in pla and tuple(cols[j]) == tuple(day8) and tuple(ncols[j]) == tuple(n8) for j in range(1, 16)):
                need += 1
        used = self.used(P)
        return need <= sum(1 for j in range(1, 16) if j not in used and j not in pla)

    def relocate(self, P, tile_indices, need=1):
        """A secondary palette that shows every index in tile_indices exactly like P and has free slots.
        Falls back to an unused palette made a copy of P."""
        for P2 in range(6, 13):
            if P2 == P:
                continue
            if all(self._looks(P2, c) == self._looks(P, c) for c in tile_indices if c):
                used, pla = self.used(P2), self.ts.pla(P2)
                if sum(1 for j in range(1, 16) if j not in used and j not in pla) >= need:
                    return P2
        if P < 6:
            return None
        busy = {p for m in range(self.ts.n()) for (t, h, v, p) in self.ts.entries(m + 512) if t}
        for P2 in range(6, 13):
            if P2 in busy or P2 == P:
                continue
            self.ts.write_pal(P2, self.ts.pal(P))
            self.ts.write_pla(P2, self.ts.pla(P))
            if P in self.ts.swap:
                self.ensure_swap(P2)
                self.ts.write_pal(self.night_path_idx(P2), self.ts.pal(self.night_path_idx(P)))
            elif P2 in self.ts.swap:
                self.ts.write_pal(self.night_path_idx(P2), self.ts.pal(P))
            free = sum(1 for j in range(1, 16) if j not in tile_indices and j not in self.ts.pla(P2))
            if free >= need:
                return P2
        return None

    # ---------------- tiles
    def reserved_tiles(self):
        """Local tile indices that must not be reused: door animations draw into the last 16 tiles of VRAM
        (secondary 496-511, see DOOR_TILE_START_* in src/field_door.c) and tileset animations overwrite
        their destination tiles (src/tileset_anims.c)."""
        res = set(range(496, 512))
        name = self.name.replace('gTileset_', '')
        text = N.read_text('src/tileset_anims.c')
        for line in text.split('\n'):
            if f'gTilesetAnims_{name}_' not in line:
                continue
            m = re.search(r'NUM_TILES_IN_PRIMARY \+ (\d+)\)\), (\d+) \* TILE_SIZE_4BPP', line)
            if m:
                res.update(range(int(m.group(1)), int(m.group(1)) + int(m.group(2))))
        for arr in re.findall(r'gTilesetAnims_%s_\w+_VDests\[\]\s*=\s*\{(.*?)\};' % name, text, re.S):
            for n in re.findall(r'NUM_TILES_IN_PRIMARY \+ (\d+)', arr):
                res.update(range(int(n), int(n) + 4))
        return res

    def free_tile(self):
        if self._free_tiles is None:
            used = self.ts.used_tiles()
            res = self.reserved_tiles()
            # unreferenced tiles (blank or leftovers) outside animation/door ranges
            self._free_tiles = [lt for lt in range(1, min(self.ts.ntiles(), 496))
                                if lt + 512 not in used and lt not in res]
        if not self._free_tiles:
            img = self.ts.img
            if self.ts.ntiles() + self.ts.tw > 496:
                raise RuntimeError('no free tiles')
            new = Image.new('P', (img.width, img.height + 8), 0)
            new.putpalette(img.getpalette())
            new.paste(img, (0, 0))
            first = self.ts.ntiles()
            self.ts.img = new
            self._free_tiles = list(range(first, self.ts.ntiles()))
        return self._free_tiles.pop(0)

    def relocate_tile(self, gid, k):
        """Move one metatile entry's tile to a fresh safe tile (e.g. out of a reserved range)."""
        t, hf, vf, p = self.ts.entries(gid)[k]
        g = tile_px(self.gen, self.ts, t)
        lt = self.free_tile()
        self.ts.set_px(lt, g)
        self.ts.set_entry(gid, k, tile=lt + 512)
        return lt + 512

    def _store(self, grid):
        key = tuple(map(tuple, grid))
        if key not in self.copies:
            lt = self.free_tile()
            self.ts.set_px(lt, grid)
            self.copies[key] = lt + 512
        return self.copies[key]

    @staticmethod
    def _points(pixels):
        pts = set()
        for p in pixels:
            if len(p) == 4:
                x0, y0, x1, y1 = p
                pts.update((x, y) for x in range(x0, x1) for y in range(y0, y1))
            else:
                pts.add(tuple(p))
        return pts

    # ---------------- main operations
    def glow(self, gid, pixels, idx_filter=None, night=None, layer=None):
        """Make metatile pixels glow. pixels: (x, y) points or (x0, y0, x1, y1) rects in 0..16 metatile
        coords. idx_filter: only pixels whose current color index is in this set.
        night: {color index: night RGB8} overrides (default: default_night). layer: 0/1 to force a layer
        (default: the visible one). Returns the number of lit pixels."""
        e = self.ts.entries(gid)
        picks = {}
        for (x, y) in self._points(pixels):
            q = (y // 8) * 2 + (x // 8)
            ks = [layer * 4 + q] if layer is not None else [4 + q, q]
            for k in ks:
                t, hf, vf, P = e[k]
                if t == 0 and k >= 4:
                    continue
                g = tile_px(self.gen, self.ts, t)
                i = 7 - (x % 8) if hf else x % 8
                j = 7 - (y % 8) if vf else y % 8
                c = g[j][i]
                if c == 0 and k >= 4:
                    continue
                if idx_filter is None or c in idx_filter:
                    picks.setdefault(k, []).append((i, j, c))
                break
        total = 0
        for k, pk in picks.items():
            t, hf, vf, P = self.ts.entries(gid)[k]
            g = [row[:] for row in tile_px(self.gen, self.ts, t)]
            tile_idx = {c for r in g for c in r}
            src_cols = (self.gen if P < 6 else self.ts).pal(P)
            want = {}
            for (i, j, c) in pk:
                day8 = tuple(src_cols[c])
                want[c] = (day8, tuple((night or {}).get(c, default_night(day8))))
            target = P
            if P < 6 or not self._can_alloc(P, want):
                target = self.relocate(P, tile_idx, need=len(set(want.values())))
                if target is None:
                    raise RuntimeError(f'{self.name} {hex(gid)} k{k}: no palette room (pal {P})')
            for (i, j, c) in pk:
                g[j][i] = self.alloc(target, *want[c], reserve=tile_idx)
            self.ts.set_entry(gid, k, tile=self._store(g), pal=target)
            total += len(pk)
        return total

    def glow_overlay(self, gid, pixels, idx_filter, night=None):
        """Light pixels with a lit-pixels-only tile on the top layer (any palette with room). The drawn
        tile of each affected quadrant moves to the bottom layer and the metatile becomes COVERED.
        Use when the tile's own palette (often a primary one) has no room. night: {index: RGB8} or a function
        (x, y, index) -> RGB8 or None, with x, y in 0..16 metatile coords (e.g. a blue band atop door glass)."""
        e = self.ts.entries(gid)
        byq = {}
        for (x, y) in self._points(pixels):
            byq.setdefault((y // 8) * 2 + (x // 8), []).append((x % 8, y % 8))
        total = 0
        for q, ps in byq.items():
            top, bot = e[4 + q], e[q]
            if top[0]:
                tg = tile_px(self.gen, self.ts, top[0])
                assert all(c for r in tg for c in r), f'{hex(gid)} q{q}: top tile has transparency'
                src = top
            else:
                src = bot
            g = tile_px(self.gen, self.ts, src[0])
            P = src[3]
            cols = (self.gen if P < 6 else self.ts).pal(P)
            lit = []
            for (mx, my) in ps:
                c = g[7 - my if src[2] else my][7 - mx if src[1] else mx]
                if c in idx_filter:
                    lit.append((mx, my, c))
            if not lit:
                continue
            qx, qy = (q % 2) * 8, (q // 2) * 8

            def look(mx, my, c):
                n8 = night(qx + mx, qy + my, c) if callable(night) else (night or {}).get(c)
                return tuple(cols[c]), tuple(n8 or default_night(cols[c]))
            want = {(mx, my): look(mx, my, c) for mx, my, c in lit}
            target = next((P2 for P2 in range(6, 13) if self._can_alloc(P2, want)), None)
            if target is None:
                raise RuntimeError(f'{hex(gid)}: no palette with a free slot')
            ov = [[0] * 8 for _ in range(8)]
            for (mx, my, c) in lit:
                ov[my][mx] = self.alloc(target, *want[(mx, my)])
            if src is top:
                self.ts.set_entry(gid, q, tile=top[0], pal=top[3], hf=top[1], vf=top[2])
            self.ts.set_entry(gid, 4 + q, tile=self._store(ov), pal=target, hf=False, vf=False)
            total += len(lit)
        if total and self.ts.layer(gid) == 0:
            self.ts.set_layer(gid, 1)
        return total

    def flatten(self, gid, q):
        """Merge quadrant q's top tile into its bottom tile (both must use the same palette) and clear the
        top, so glow_overlay can use the top layer. The merged pixels end up below sprites."""
        e = self.ts.entries(gid)
        bot, top = e[q], e[4 + q]
        if not top[0]:
            return
        assert bot[3] == top[3] or not bot[0], f'{hex(gid)} q{q}: layers use different palettes'

        def oriented(ent):
            t, hf, vf, _ = ent
            g = tile_px(self.gen, self.ts, t)
            return [[g[7 - j if vf else j][7 - i if hf else i] for i in range(8)] for j in range(8)]
        out = oriented(bot) if bot[0] else [[0] * 8 for _ in range(8)]
        for j, row in enumerate(oriented(top)):
            for i, c in enumerate(row):
                if c:
                    out[j][i] = c
        self.ts.set_entry(gid, q, tile=self._store(out), pal=top[3], hf=False, vf=False)
        self.ts.set_entry(gid, 4 + q, tile=0, pal=0, hf=False, vf=False)

    def solid(self, gid, quads, day8, night8=None, layer=0):
        """Replace whole quadrants (0 TL, 1 TR, 2 BL, 3 BR) of one layer with a single lit color."""
        night8 = tuple(night8 or default_night(day8))
        P2 = next(P for P in range(6, 13) if self._can_alloc(P, {0: (tuple(day8), night8)}))
        j = self.alloc(P2, tuple(day8), night8)
        t = self._store([[j] * 8 for _ in range(8)])
        for q in quads:
            self.ts.set_entry(gid, layer * 4 + q, tile=t, pal=P2, hf=False, vf=False)

    def save(self):
        self.ts.save_tiles()
        self.ts.save_mt()


# ---------------- restore helpers (need snapshot_head() first)
def head_entries(ts, gid):
    rel = os.path.relpath(ts.mt_path, N.rpath('data/tilesets'))
    hm = open(os.path.join(HEAD_DIR, rel), 'rb').read()
    o = (gid - ts.base) * 16
    return struct.unpack('<8H', hm[o:o + 16])


def restore_meta(ts, gid):
    o = (gid - ts.base) * 16
    ts.mt[o:o + 16] = struct.pack('<8H', *head_entries(ts, gid))


def head_tile(ts, lt):
    rel = os.path.relpath(ts.png_path, N.rpath('data/tilesets'))
    hi = Image.open(os.path.join(HEAD_DIR, rel))
    x, y = (lt % ts.tw) * 8, (lt // ts.tw) * 8
    return [[hi.getpixel((x + i, y + j)) & 15 for i in range(8)] for j in range(8)]


def restore_tile(ts, lt):
    ts.set_px(lt, head_tile(ts, lt))
