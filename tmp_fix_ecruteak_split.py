#!/usr/bin/env python3
"""
Split Ecruteak secondary tileset into two variants:
- ecruteak_city      : overflow from johto_general (for Route37)
- ecruteak_city_nw   : overflow from johto_north_west (for Ecruteak City)
"""

from pathlib import Path
from PIL import Image
import struct
import shutil

ROOT_EMR = Path("/mnt/c/Users/drfor/Documents/Porymap/Decomps/pokeemerald")
ROOT_SEQ = Path("/mnt/c/Users/drfor/Documents/Porymap/Decomps/pokesequel-master")

SEQ_PRI = ROOT_SEQ / "data/tilesets/primary"
SEQ_SEC = ROOT_SEQ / "data/tilesets/secondary/ecruteak_city"
EMR_SEC = ROOT_EMR / "data/tilesets/secondary"

METATILE_BYTES = 16
ATTR_BYTES = 2


def pal_to_gbapal(pal_text: str) -> bytes:
    lines = pal_text.splitlines()
    colors = []
    i = 0
    while i < len(lines):
        line = lines[i].strip()
        if line in ("JASC-PAL", "0100"):
            i += 1
            continue
        try:
            count = int(line)
            i += 1
            for _ in range(count):
                if i >= len(lines):
                    break
                parts = lines[i].strip().split()
                if len(parts) >= 3:
                    r, g, b = int(parts[0]), int(parts[1]), int(parts[2])
                    gba = ((r >> 3) & 0x1F) | (((g >> 3) & 0x1F) << 5) | (((b >> 3) & 0x1F) << 10)
                    colors.append(gba)
                i += 1
            break
        except ValueError:
            i += 1
    while len(colors) < 16:
        colors.append(0)
    return struct.pack("<" + "H" * 16, *colors[:16])


def build_variant(dst_name: str, pri_name: str) -> None:
    dst = EMR_SEC / dst_name
    (dst / "palettes").mkdir(parents=True, exist_ok=True)

    pri_tiles = Image.open(SEQ_PRI / pri_name / "tiles.png").convert("P")
    sec_tiles = Image.open(SEQ_SEC / "tiles.png").convert("P")
    overflow = pri_tiles.crop((0, 256, 128, 320)).convert("P")

    merged = Image.new("P", (128, 64 + sec_tiles.height))
    merged.putpalette(sec_tiles.getpalette())
    overflow.putpalette(sec_tiles.getpalette())
    merged.paste(overflow, (0, 0))
    merged.paste(sec_tiles, (0, 64))
    merged.save(dst / "tiles.png")

    pri_meta = (SEQ_PRI / pri_name / "metatiles.bin").read_bytes()
    sec_meta = (SEQ_SEC / "metatiles.bin").read_bytes()
    overflow_meta = pri_meta[512 * METATILE_BYTES:640 * METATILE_BYTES]
    (dst / "metatiles.bin").write_bytes(overflow_meta + sec_meta)

    pri_attr = (SEQ_PRI / pri_name / "metatile_attributes.bin").read_bytes()
    sec_attr = (SEQ_SEC / "metatile_attributes.bin").read_bytes()
    overflow_attr = pri_attr[512 * ATTR_BYTES:640 * ATTR_BYTES]
    (dst / "metatile_attributes.bin").write_bytes(overflow_attr + sec_attr)

    # Copy sec palettes, then force palette 06 from associated primary.
    for i in range(16):
        idx = f"{i:02d}"
        sec_pal = SEQ_SEC / "palettes" / f"{idx}.pal"
        dst_pal = dst / "palettes" / f"{idx}.pal"
        if sec_pal.exists():
            shutil.copy2(sec_pal, dst_pal)

    pri06 = SEQ_PRI / pri_name / "palettes/06.pal"
    text06 = pri06.read_text(errors="replace")
    (dst / "palettes/06.pal").write_text(text06)

    for i in range(16):
        idx = f"{i:02d}"
        p = dst / "palettes" / f"{idx}.pal"
        if p.exists():
            (dst / "palettes" / f"{idx}.gbapal").write_bytes(pal_to_gbapal(p.read_text(errors="replace")))

    print(f"[OK] {dst_name} built with primary {pri_name}")


def main() -> None:
    build_variant("ecruteak_city", "johto_general")
    build_variant("ecruteak_city_nw", "johto_north_west")


if __name__ == "__main__":
    main()
