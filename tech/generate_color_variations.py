#!/usr/bin/env python3
"""Generate preview sheets of every Individual Color Variation for one or more
Pokémon front sprites.

Mirrors the algorithm in `src/pokemon_color_variation.c` including the
per-species overrides (e.g. Pikachu line → brown bias).

Usage
-----
    python generate_color_variations.py pikachu pichu raichu
    python generate_color_variations.py --backs pikachu        # also do back sprite
    python generate_color_variations.py --out previews bulbasaur

Outputs: tech/color_variation_previews/<name>_front_variations.png
         (a 8×8 grid showing all 64 personality buckets, plus the shiny in the
         corner for comparison).

Requirements: Pillow.
"""
from __future__ import annotations

import argparse
import math
import os
import sys
from pathlib import Path
from typing import List, Optional, Tuple

try:
    from PIL import Image
except ImportError:
    sys.stderr.write("This tool needs Pillow. Install with: pip install Pillow\n")
    sys.exit(1)

REPO_ROOT = Path(__file__).resolve().parent.parent
GFX_ROOT = REPO_ROOT / "graphics" / "pokemon"
DEFAULT_OUT = Path(__file__).resolve().parent / "color_variation_previews"

# ---------------------------------------------------------------------------
# Constants — must mirror include/pokemon_color_variation.h
# ---------------------------------------------------------------------------
COLOR_VARIATION_MAX_ANGLE = 14            # sine-table units (256 = 360°)
COLOR_VARIATION_SAT_MUTED = 922 / 1024.0
COLOR_VARIATION_SAT_VIVID = 1229 / 1024.0
COLOR_VARIATION_CHROMA_THRESHOLD = 4

# Per-species overrides — must mirror sColorVariationOverrides in
# src/pokemon_color_variation.c.
# Tuple: (maxAngle, angleBias, satCap / 1024.0, satMul / 1024.0)
OVERRIDES = {
    "pichu":   (6, -6, 900 / 1024.0, 750 / 1024.0),
    "pikachu": (6, -6, 900 / 1024.0, 750 / 1024.0),
    "raichu":  (6, -6, 900 / 1024.0, 750 / 1024.0),
}


# ---------------------------------------------------------------------------
# 5-bit RGB helpers (GBA palette colors are 5 bits per channel)
# ---------------------------------------------------------------------------
def to5(v: int) -> int:
    return max(0, min(31, v))


def rgb8_to_rgb5(r: int, g: int, b: int) -> Tuple[int, int, int]:
    return (r >> 3, g >> 3, b >> 3)


def rgb5_to_rgb8(r: int, g: int, b: int) -> Tuple[int, int, int]:
    # Replicate top bits to fill 8-bit channel — matches GBA hardware behavior
    return ((r << 3) | (r >> 2), (g << 3) | (g >> 2), (b << 3) | (b >> 2))


# ---------------------------------------------------------------------------
# Hue rotation (circulant matrix around (1,1,1)) — matches the C code's
# fixed-point Sin/Cos lookups but expressed in floating point for clarity.
# ---------------------------------------------------------------------------
def hue_matrix(angle_index: int) -> Tuple[float, float, float]:
    radians = (angle_index / 256.0) * 2.0 * math.pi
    cos_a = math.cos(radians)
    sin_a = math.sin(radians)
    one_minus_cos_div_3 = (1.0 - cos_a) / 3.0
    sqrt_term = math.sqrt(1.0 / 3.0) * sin_a
    mA = cos_a + one_minus_cos_div_3
    mB = one_minus_cos_div_3 + sqrt_term
    mC = one_minus_cos_div_3 - sqrt_term
    return (mA, mB, mC)


def rotate_color(r: int, g: int, b: int, mat) -> Tuple[int, int, int]:
    mA, mB, mC = mat
    nr = round(mA * r + mC * g + mB * b)
    ng = round(mB * r + mA * g + mC * b)
    nb = round(mC * r + mB * g + mA * b)
    return (to5(nr), to5(ng), to5(nb))


def adjust_saturation(r: int, g: int, b: int, factor: float) -> Tuple[int, int, int]:
    gray = (r + g + b) // 3
    nr = round(gray + (r - gray) * factor)
    ng = round(gray + (g - gray) * factor)
    nb = round(gray + (b - gray) * factor)
    return (to5(nr), to5(ng), to5(nb))


def is_saturated(r: int, g: int, b: int) -> bool:
    return (max(r, g, b) - min(r, g, b)) >= COLOR_VARIATION_CHROMA_THRESHOLD


def hue_bucket(r: int, g: int, b: int) -> int:
    if r >= g and r >= b:
        return 0  # warm
    return 1  # cool


# ---------------------------------------------------------------------------
# Apply variation — must mirror ApplyIndividualColorVariation()
# ---------------------------------------------------------------------------
def apply_variation(palette5: List[Tuple[int, int, int]], shift: int,
                    species_name: Optional[str]) -> List[Tuple[int, int, int]]:
    """Apply variation to a 16-color 5-bit palette. Index 0 unchanged."""
    out = list(palette5)
    override = OVERRIDES.get(species_name.lower()) if species_name else None
    if override is not None:
        max_angle, angle_bias, sat_cap, sat_mul = override
    else:
        max_angle, angle_bias, sat_cap, sat_mul = COLOR_VARIATION_MAX_ANGLE, 0, 2.0, 1.0

    hue_step = shift & 0x7
    mode = (shift >> 3) & 0x7
    signed_step = hue_step - 4
    angle_mag = (abs(signed_step) * max_angle) // 4

    if angle_mag == 0 and mode == 0 and angle_bias == 0:
        return out

    if angle_mag == 0:
        angle_index = 0
    elif signed_step >= 0:
        angle_index = angle_mag
    else:
        angle_index = (256 - angle_mag) % 256

    if angle_bias != 0:
        angle_index = (angle_index + angle_bias) % 256

    vivid_factor = min(COLOR_VARIATION_SAT_VIVID, sat_cap)

    has_split = mode in (3, 6, 7)
    has_muted = mode in (1, 6)
    has_vivid = mode in (2, 7)
    has_split_sat = mode in (4, 5)
    warm_is_vivid = (mode == 4)
    has_hue = (angle_index != 0)

    mat_pos = hue_matrix(angle_index) if has_hue else None
    mat_neg = hue_matrix((256 - angle_index) % 256) if (has_hue and has_split) else None

    for i in range(1, 16):
        r, g, b = out[i]
        sat = is_saturated(r, g, b)
        bucket = hue_bucket(r, g, b)

        if has_hue:
            if has_split and sat:
                if bucket == 0:
                    r, g, b = rotate_color(r, g, b, mat_pos)
                else:
                    r, g, b = rotate_color(r, g, b, mat_neg)
            elif not has_split:
                r, g, b = rotate_color(r, g, b, mat_pos)

        if has_muted:
            r, g, b = adjust_saturation(r, g, b, COLOR_VARIATION_SAT_MUTED)
        elif has_vivid:
            r, g, b = adjust_saturation(r, g, b, vivid_factor)
        elif has_split_sat and sat:
            if (bucket == 0) == warm_is_vivid:
                r, g, b = adjust_saturation(r, g, b, vivid_factor)
            else:
                r, g, b = adjust_saturation(r, g, b, COLOR_VARIATION_SAT_MUTED)

        if abs(sat_mul - 1.0) > 1e-6:
            r, g, b = adjust_saturation(r, g, b, sat_mul)

        out[i] = (r, g, b)
    return out


# ---------------------------------------------------------------------------
# Pal / sprite IO
# ---------------------------------------------------------------------------
def load_jasc_pal(path: Path) -> List[Tuple[int, int, int]]:
    lines = path.read_text().splitlines()
    if lines[0].strip() != "JASC-PAL":
        raise ValueError(f"{path}: not a JASC palette")
    count = int(lines[2].strip())
    colors8 = []
    for i in range(count):
        r, g, b = (int(x) for x in lines[3 + i].split())
        colors8.append((r, g, b))
    if len(colors8) < 16:
        colors8 += [(0, 0, 0)] * (16 - len(colors8))
    return [rgb8_to_rgb5(*c) for c in colors8[:16]]


def load_indexed_sprite(path: Path) -> Image.Image:
    img = Image.open(path)
    if img.mode != "P":
        # Best-effort: convert to a 16-color indexed image using the file's pal
        img = img.convert("P", palette=Image.ADAPTIVE, colors=16)
    return img


def render_with_palette(indexed: Image.Image, palette5: List[Tuple[int, int, int]]) -> Image.Image:
    out = indexed.copy()
    flat = []
    for (r, g, b) in palette5:
        r8, g8, b8 = rgb5_to_rgb8(r, g, b)
        flat += [r8, g8, b8]
    flat += [0] * (768 - len(flat))
    out.putpalette(flat)
    return out.convert("RGBA")


# ---------------------------------------------------------------------------
# Sheet generation
# ---------------------------------------------------------------------------
def make_sheet(species_name: str, sprite_path: Path, pal_path: Path,
               shiny_pal_path: Optional[Path]) -> Image.Image:
    """9x9 layout: original in the center (green border), all 64 variations
    around it, shiny in the bottom-right (red border) separated by a gap."""
    base_pal = load_jasc_pal(pal_path)
    sprite = load_indexed_sprite(sprite_path)
    sprite = sprite.crop((0, 0, 64, 64))  # only the first frame

    cell_w, cell_h = sprite.size
    cols, rows = 9, 9
    pad = 2
    sheet_w = cols * (cell_w + pad) + pad
    sheet_h = rows * (cell_h + pad) + pad

    sheet = Image.new("RGBA", (sheet_w, sheet_h), (40, 40, 40, 255))
    from PIL import ImageDraw
    draw = ImageDraw.Draw(sheet)

    center_idx = 4 * cols + 4   # row 4, col 4
    shiny_idx = rows * cols - 1 # bottom-right

    def cell_xy(idx: int) -> Tuple[int, int]:
        r, c = divmod(idx, cols)
        return (pad + c * (cell_w + pad), pad + r * (cell_h + pad))

    # Place 64 variations, skipping center and shiny cells
    var_iter = iter(range(64))
    for idx in range(rows * cols):
        if idx == center_idx or idx == shiny_idx:
            continue
        try:
            shift = next(var_iter)
        except StopIteration:
            break
        out_pal = apply_variation(base_pal, shift, species_name)
        rendered = render_with_palette(sprite, out_pal)
        sheet.paste(rendered, cell_xy(idx))

    # Original (no variation) in center, green border
    rendered = render_with_palette(sprite, base_pal)
    x, y = cell_xy(center_idx)
    sheet.paste(rendered, (x, y))
    draw.rectangle((x - 2, y - 2, x + cell_w + 1, y + cell_h + 1),
                   outline=(64, 220, 64, 255), width=2)

    # Shiny in corner, red border
    if shiny_pal_path and shiny_pal_path.exists():
        shiny_pal = load_jasc_pal(shiny_pal_path)
        rendered = render_with_palette(sprite, shiny_pal)
        x, y = cell_xy(shiny_idx)
        sheet.paste(rendered, (x, y))
        draw.rectangle((x - 2, y - 2, x + cell_w + 1, y + cell_h + 1),
                       outline=(255, 64, 64, 255), width=2)

    return sheet


def process_species(name: str, do_back: bool, out_dir: Path) -> List[Path]:
    species_dir = GFX_ROOT / name
    if not species_dir.is_dir():
        raise FileNotFoundError(f"No graphics directory for '{name}': {species_dir}")
    pal = species_dir / "normal.pal"
    shiny_pal = species_dir / "shiny.pal"
    if not pal.exists():
        raise FileNotFoundError(f"Missing palette: {pal}")

    out_dir.mkdir(parents=True, exist_ok=True)
    written: List[Path] = []

    front = species_dir / "front.png"
    if front.exists():
        sheet = make_sheet(name, front, pal, shiny_pal)
        out = out_dir / f"{name}_front_variations.png"
        sheet.save(out)
        written.append(out)

    if do_back:
        back = species_dir / "back.png"
        if back.exists():
            sheet = make_sheet(name, back, pal, shiny_pal)
            out = out_dir / f"{name}_back_variations.png"
            sheet.save(out)
            written.append(out)

    return written


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("species", nargs="*",
                        help="Pokemon folder names under graphics/pokemon/ "
                             "(e.g. pikachu pichu raichu). If omitted, you'll "
                             "be prompted interactively.")
    parser.add_argument("--backs", action="store_true",
                        help="Also generate sheets for the back sprite.")
    parser.add_argument("--out", type=Path, default=DEFAULT_OUT,
                        help=f"Output directory (default: {DEFAULT_OUT})")
    args = parser.parse_args()

    names: List[str] = list(args.species)
    if not names:
        try:
            line = input("Enter Pokémon names (space-separated): ").strip()
        except EOFError:
            line = ""
        names = line.split()
    if not names:
        parser.print_help()
        return 1

    failures = 0
    for raw in names:
        name = raw.strip().lower()
        try:
            written = process_species(name, args.backs, args.out)
            if not written:
                print(f"  {name}: no sprites found")
            else:
                for p in written:
                    print(f"  wrote {p.relative_to(REPO_ROOT)}")
        except Exception as e:  # noqa: BLE001
            failures += 1
            print(f"  {name}: ERROR — {e}")

    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
