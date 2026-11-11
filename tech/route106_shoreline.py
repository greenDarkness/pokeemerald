"""Add a sand->shallow->ocean shoreline band to the Route 106 cave-beach island.

Works at half-tile (quadrant) resolution: sand shrinks by one quadrant where it touches ocean, ocean grows a one-quadrant
shallow rim around sand, and each touched tile is re-picked from the General shoreline metatiles by its 2x2 pattern.
Tiles on Mr. Briney's boat path are never modified.

  python tech/route106_shoreline.py            # dry run
  python tech/route106_shoreline.py --apply    # writes Route106 map.bin and Dewford ocean-behavior copies
"""
import json
import os
import re
import struct
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
os.chdir(ROOT)

MAP = 'data/layouts/Route106/map.bin'
W, H = 81, 20
SCOPE = (20, 10, 72, 20)  # x0, y0, x1, y1 (exclusive): the cave-beach island and its surrounding water
GEN_ATTR = 'data/tilesets/primary/general/metatile_attributes.bin'
SEC_DIR = 'data/tilesets/secondary/dewford/'
MB_OCEAN_WATER = 0x15
MB_SHALLOW_WATER = 0x17

SAND = {0x124, 0x11C, 0x13D, 0x13E}
OCEAN = {0x170}
MIX_L = {0x11B, 0x123}  # ocean left, sand right
MIX_R = {0x11D, 0x125}  # sand left, ocean right
# 2x2 water rock touching the beach: its water half is treated as ocean and its sand half as sand, so the rock's
# center lands in the shallows; the rock's top layer is laid over the resulting shoreline tiles.
ROCK_WATER = {(27, 14), (27, 15)}
ROCK_SAND = {(28, 14), (28, 15)}
# Half-sand tile right next to Briney's lane: the band wraps onto its top-left corner (NE sand corner over shallows).
CORNER = {(71, 18)}

# quadrant patterns TL TR BL BR -> General metatile
INNER = {'HHHS': 0x195, 'HHSS': 0x196, 'HHSH': 0x197, 'HSHS': 0x19D, 'HHHH': 0x19E, 'SHSH': 0x19F,
         'HSHH': 0x1A5, 'SSHH': 0x1A6, 'SHHH': 0x1A7, 'HSSS': 0x185, 'SHSS': 0x186, 'SSHS': 0x18D, 'SSSH': 0x18E}
OUTER = {'OOOH': 0x1AD, 'OOHH': 0x1AE, 'OOHO': 0x1AF, 'OHOH': 0x1B5, 'HOHO': 0x1B7, 'OHOO': 0x1BD,
         'HHOO': 0x1BE, 'HOOO': 0x1BF, 'OHHH': 0x19B, 'HOHH': 0x19C, 'HHOH': 0x1A3, 'HHHO': 0x1A4}


def briney_path():
    """Tiles of Route 106 crossed by Mr. Briney's boat (Route 104 <-> Dewford), from the movement scripts."""
    def moves(path, label):
        out, on = [], False
        for line in open(path, encoding='utf-8', errors='replace'):
            if line.startswith(label + ':'):
                on = True
                continue
            if on:
                s = line.strip()
                if s.startswith('step_end'):
                    break
                m = re.match(r'walk_(?:fast_|faster_|slow_)?(up|down|left|right)', s)
                if m:
                    out.append(m.group(1))
        return out
    step = {'up': (0, -1), 'down': (0, 1), 'left': (-1, 0), 'right': (1, 0)}
    tiles = set()
    # Route 104 sits 160 rows above Route 106; Dewford's origin is (60, 20) in Route 106 coords.
    for (x, y), path, labels in [
        ((12, 54 - 160), 'data/maps/Route104/scripts.inc', ['Route104_Movement_SailToDewford']),
        ((12, 54 - 160), 'data/maps/Route104/scripts.inc',
         ['Route104_Movement_SailToDewfordBeforeDadCalls', 'Route104_Movement_SailToDewfordAfterDadCalls']),
        ((60 + 12, 20 + 8), 'data/maps/DewfordTown/scripts.inc', ['DewfordTown_Movement_SailToPetalburg'])]:
        tiles.add((x, y))
        for lab in labels:
            for d in moves(path, lab):
                x, y = x + step[d][0], y + step[d][1]
                tiles.add((x, y))
    return {(x, y) for x, y in tiles if 0 <= x < W and 0 <= y < H}


def main(apply):
    blk = bytearray(open(MAP, 'rb').read())
    get = lambda x, y: struct.unpack_from('<H', blk, (y * W + x) * 2)[0]
    path = briney_path()
    x0, y0, x1, y1 = SCOPE
    if any(get(x, y) & 0x3FF in INNER.values() for y in range(y0, y1) for x in range(x0, x1)):
        sys.exit('Shoreline already applied; restore the map and Dewford tileset with git checkout first.')

    def kind(x, y):
        if not (0 <= x < W and 0 <= y < H):
            return 'X'
        if (x, y) in ROCK_WATER:
            return 'O'
        if (x, y) in ROCK_SAND:
            return 'S'
        v = get(x, y)
        t, col = v & 0x3FF, (v >> 10) & 3
        if col and t not in MIX_L | MIX_R:
            return 'X'
        if t in SAND:
            return 'S'
        if t in OCEAN:
            return 'O'
        # half sand/half ocean edges become full sand, unless the water side is Briney's lane
        if t in MIX_L:
            return 'X' if (x - 1, y) in path else 'S'
        if t in MIX_R:
            return 'X' if (x + 1, y) in path else 'S'
        return 'X'

    q = {}
    for y in range(-1, H + 1):
        for x in range(-1, W + 1):
            k = kind(x, y)
            for j in range(2):
                for i in range(2):
                    q[(2 * x + i, 2 * y + j)] = k

    def near(qx, qy, k):
        return any(q.get((qx + i, qy + j)) == k for i in (-1, 0, 1) for j in (-1, 0, 1))

    changes = {}
    missing = []
    for y in range(y0, y1):
        for x in range(x0, x1):
            if (x, y) in path:
                continue
            k = kind(x, y)
            if k not in 'SO':
                continue
            pat = ''
            for j in range(2):
                for i in range(2):
                    qx, qy = 2 * x + i, 2 * y + j
                    if k == 'S':
                        pat += 'H' if near(qx, qy, 'O') else 'S'
                    else:
                        pat += 'H' if near(qx, qy, 'S') else 'O'
            if pat in ('SSSS', 'OOOO'):
                if get(x, y) & 0x3FF in MIX_L | MIX_R:
                    changes[(x, y)] = ('SSSS', 0x124, 3)
                continue
            table = INNER if k == 'S' else OUTER
            if pat not in table:
                missing.append(((x, y), pat))
                continue
            changes[(x, y)] = (pat, table[pat], 3 if k == 'S' else 1)

    for m in missing:
        print('NO METATILE FOR', m)
    print(len(changes), 'tiles to change; path tiles', len(path))

    gen_attr = open(GEN_ATTR, 'rb').read()
    gen_mt = open('data/tilesets/primary/general/metatiles.bin', 'rb').read()
    sec_mt = bytearray(open(SEC_DIR + 'metatiles.bin', 'rb').read())
    sec_attr = bytearray(open(SEC_DIR + 'metatile_attributes.bin', 'rb').read())

    def entries(g):
        src, o = (gen_mt, g * 16) if g < 0x200 else (sec_mt, (g - 0x200) * 16)
        return list(struct.unpack_from('<8H', src, o))

    def attr(g):
        return struct.unpack_from('<H', gen_attr, g * 2)[0] if g < 0x200 else \
            struct.unpack_from('<H', sec_attr, (g - 0x200) * 2)[0]

    def add_metatile(ents, a, why):
        """Return a Dewford metatile with these entries/attributes, appending one if none matches."""
        raw = struct.pack('<8H', *ents)
        for i in range(len(sec_attr) // 2):
            if sec_mt[i * 16:i * 16 + 16] == raw and struct.unpack_from('<H', sec_attr, i * 2)[0] == a:
                return 0x200 + i
        new = 0x200 + len(sec_attr) // 2
        assert new < 0x3FF  # 0x3FF is MAPGRID_UNDEFINED
        sec_mt.extend(raw)
        sec_attr.extend(struct.pack('<H', a))
        print('%#x: %s' % (new, why))
        return new

    for (x, y), (pat, g, e) in changes.items():
        orig = get(x, y)
        if (x, y) in ROCK_WATER | ROCK_SAND:
            # shoreline tile underneath, rock's top layer on top; keeps the rock's attributes and collision
            ents, rock = entries(g), entries(orig & 0x3FF)
            for k in range(4, 8):
                if rock[k] & 0x3FF:
                    assert ents[k] & 0x3FF in (0, rock[k] & 0x3FF), ((x, y), k)
                    ents[k] = rock[k]
            g = add_metatile(ents, attr(orig & 0x3FF), 'rock in shallows at %d,%d (%s)' % (x, y, pat))
            struct.pack_into('<H', blk, (y * W + x) * 2, g | (orig & 0xFC00))
            continue
        if e == 1 and gen_attr[g * 2] == MB_SHALLOW_WATER:
            # outer-band tiles must be ocean behavior so the band is fishable/surfable
            g = add_metatile(entries(g), (attr(g) & 0xFF00) | MB_OCEAN_WATER, 'ocean copy of %#x' % g)
        struct.pack_into('<H', blk, (y * W + x) * 2, g | (e << 12))

    for x, y in CORNER:
        orig = get(x, y)
        assert orig & 0x3FF == 0x11D, (x, y, hex(orig))
        corner, edge = entries(0x197), entries(0x11D)
        ents = [corner[0], edge[1], corner[2], edge[3], 0, 0, corner[6], 0]
        g = add_metatile(ents, attr(0x11D), 'NE sand corner over shallows at %d,%d' % (x, y))
        struct.pack_into('<H', blk, (y * W + x) * 2, g | (orig & 0xFC00))

    if apply:
        open(MAP, 'wb').write(blk)
        open(SEC_DIR + 'metatiles.bin', 'wb').write(sec_mt)
        open(SEC_DIR + 'metatile_attributes.bin', 'wb').write(sec_attr)
        print('applied')
    else:
        print('dry run; pass --apply to write')


if __name__ == '__main__':
    main('--apply' in sys.argv)
