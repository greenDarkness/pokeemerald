"""Contact sheets for combing glass detections: each cluster of detected metatiles as a day crop beside the same
crop with detected pixels in magenta, labelled with map coords and metatile ids.

    python comb_sheet.py johto [--maps A,B]   # writes out/comb_<map>.png
"""
import argparse
import os

import glass
from glass import N, Image, OUT
from PIL import ImageDraw

S = 3


def clusters(cells):
    cells, out = set(cells), []
    while cells:
        stack, group = [cells.pop()], set()
        while stack:
            c = stack.pop()
            group.add(c)
            for dx in (-1, 0, 1):
                for dy in (-1, 0, 1):
                    n = (c[0] + dx, c[1] + dy)
                    if n in cells:
                        cells.remove(n)
                        stack.append(n)
        out.append(group)
    return out


def sheet(name, s, g):
    r = N.MetatileRenderer(N.Sources(), s.prim.name, s.sec.name, False)
    cells = {(x // 16, y // 16) for x, y in g}
    tiles = []
    for grp in sorted(clusters(cells), key=lambda c: (min(y for _, y in c), min(x for x, _ in c))):
        x0, x1 = max(0, min(x for x, _ in grp) - 1), min(s.w - 1, max(x for x, _ in grp) + 1)
        y0, y1 = max(0, min(y for _, y in grp) - 1), min(s.h - 1, max(y for _, y in grp) + 1)
        day = Image.new('RGB', ((x1 - x0 + 1) * 16, (y1 - y0 + 1) * 16))
        for my in range(y0, y1 + 1):
            for mx in range(x0, x1 + 1):
                day.paste(r.get(s.b[my][mx])[0].convert('RGB'), ((mx - x0) * 16, (my - y0) * 16))
        hl = day.copy()
        px = hl.load()
        for (x, y) in g:
            if x0 * 16 <= x < (x1 + 1) * 16 and y0 * 16 <= y < (y1 + 1) * 16:
                px[x - x0 * 16, y - y0 * 16] = (255, 0, 255)
        ids = sorted({s.b[y][x] for x, y in grp})
        label = f'({x0 + 1},{y0 + 1}) ' + ' '.join('%x' % i for i in ids[:8]) + (' ..' if len(ids) > 8 else '')
        w, h = day.width * S, day.height * S
        t = Image.new('RGB', (max(2 * w + 6, 8 * len(label)), h + 14), (30, 30, 30))
        t.paste(day.resize((w, h), Image.NEAREST), (0, 14))
        t.paste(hl.resize((w, h), Image.NEAREST), (w + 6, 14))
        ImageDraw.Draw(t).text((2, 1), label, fill=(255, 255, 0))
        tiles.append(t)
    if not tiles:
        return
    W = 1600
    rows, row, x, rh = [], [], 0, 0
    for t in tiles:
        if row and x + t.width > W:
            rows.append((row, rh))
            row, x, rh = [], 0, 0
        row.append(t)
        x += t.width + 8
        rh = max(rh, t.height)
    rows.append((row, rh))
    out = Image.new('RGB', (W, sum(h + 8 for _, h in rows)), (0, 0, 0))
    y = 0
    for row, rh in rows:
        x = 0
        for t in row:
            out.paste(t, (x, y))
            x += t.width + 8
        y += rh + 8
    out.save(os.path.join(OUT, f'comb_{name}.png'))
    print(f'{name}: {len(tiles)} clusters, {out.size}')


if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument('region')
    ap.add_argument('--maps')
    a = ap.parse_args()
    names, scans = glass.analyze(a.region)
    want = set(a.maps.split(',')) if a.maps else None
    for n in names:
        if want is None or n in want:
            sheet(n, *scans[n])
