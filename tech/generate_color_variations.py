#!/usr/bin/env python3
"""Generate preview sheets of every Individual Color Variation for one or more
Pokémon front sprites.

Mirrors the algorithm in `src/pokemon_color_variation.c`. The per-species
overrides and tuning constants are read directly from that file (and
`include/pokemon_color_variation.h`) every run, so edits there show up here
without any extra syncing.

Usage
-----
    python generate_color_variations.py pikachu pichu raichu
    python generate_color_variations.py --backs pikachu        # also do back sprite
    python generate_color_variations.py --overrides            # every overridden species
    python generate_color_variations.py --existing             # refresh every existing sheet
    python generate_color_variations.py --out previews bulbasaur

Outputs: tech/color_variation_previews/<name>_front_variations.png
         (a 8×8 grid showing all 64 personality buckets, plus the shiny in the
         corner for comparison).

Requirements: Pillow.
"""
from __future__ import annotations

import argparse
import ast
import math
import os
import re
import sys
from pathlib import Path
from typing import Dict, List, Optional, Tuple

try:
    from PIL import Image
except ImportError:
    sys.stderr.write("This tool needs Pillow. Install with: pip install Pillow\n")
    sys.exit(1)

REPO_ROOT = Path(__file__).resolve().parent.parent
GFX_ROOT = REPO_ROOT / "graphics" / "pokemon"
DEFAULT_OUT = Path(__file__).resolve().parent / "color_variation_previews"
C_SOURCE = REPO_ROOT / "src" / "pokemon_color_variation.c"
C_HEADER = REPO_ROOT / "include" / "pokemon_color_variation.h"


# ---------------------------------------------------------------------------
# Read constants and per-species overrides straight from the C source, so the
# preview always matches the game (no second copy to keep in sync).
# ---------------------------------------------------------------------------
def _strip_c_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", " ", text, flags=re.S)
    return re.sub(r"//[^\n]*", "", text)


def _parse_defines(text: str) -> Dict[str, str]:
    defines = {}
    for m in re.finditer(r"^[ \t]*#define[ \t]+(\w+)[ \t]+([^\n]+)$", text, flags=re.M):
        defines[m.group(1)] = m.group(2).strip()
    return defines


def _c_div(a: int, b: int) -> int:
    q = abs(a) // abs(b)
    return q if (a >= 0) == (b > 0) else -q


_BIN_OPS = {
    ast.Add: lambda a, b: a + b,
    ast.Sub: lambda a, b: a - b,
    ast.Mult: lambda a, b: a * b,
    ast.Div: _c_div,
    ast.FloorDiv: _c_div,
    ast.Mod: lambda a, b: a - b * _c_div(a, b),
    ast.LShift: lambda a, b: a << b,
    ast.RShift: lambda a, b: a >> b,
    ast.BitOr: lambda a, b: a | b,
    ast.BitAnd: lambda a, b: a & b,
}


def _eval_c_expr(expr: str, defines: Dict[str, str], depth: int = 0) -> int:
    """Evaluates a simple C integer constant expression (numbers, #defines,
    + - * / % << >> | & and parentheses)."""
    if depth > 20:
        raise ValueError(f"#define recursion too deep in '{expr}'")
    expr = re.sub(r"\b(0x[0-9a-fA-F]+|\d+)[uUlL]+\b", r"\1", expr.strip())

    def node_value(node: ast.AST) -> int:
        if isinstance(node, ast.Expression):
            return node_value(node.body)
        if isinstance(node, ast.Constant) and isinstance(node.value, int):
            return node.value
        if isinstance(node, ast.UnaryOp) and isinstance(node.op, (ast.USub, ast.UAdd)):
            v = node_value(node.operand)
            return -v if isinstance(node.op, ast.USub) else v
        if isinstance(node, ast.BinOp) and type(node.op) in _BIN_OPS:
            return _BIN_OPS[type(node.op)](node_value(node.left), node_value(node.right))
        if isinstance(node, ast.Name):
            if node.id not in defines:
                raise ValueError(f"unknown identifier '{node.id}'")
            return _eval_c_expr(defines[node.id], defines, depth + 1)
        raise ValueError(f"unsupported expression '{expr}'")

    return node_value(ast.parse(expr, mode="eval"))


def _split_top_level(text: str) -> List[str]:
    parts, depth, cur = [], 0, ""
    for ch in text:
        if ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
        if ch == "," and depth == 0:
            parts.append(cur)
            cur = ""
        else:
            cur += ch
    if cur.strip():
        parts.append(cur)
    return [p.strip() for p in parts if p.strip()]


def load_color_variation_config():
    """Returns (defines, overrides) parsed from the C header/source.
    overrides maps a lowercase species folder name to a dict of struct fields."""
    header = _strip_c_comments(C_HEADER.read_text(encoding="utf-8"))
    source = _strip_c_comments(C_SOURCE.read_text(encoding="utf-8"))
    defines = _parse_defines(header)
    defines.update(_parse_defines(source))

    struct_m = re.search(r"struct\s+ColorVariationOverride\s*\{(.*?)\}\s*;", source, flags=re.S)
    if not struct_m:
        raise ValueError(f"couldn't find struct ColorVariationOverride in {C_SOURCE}")
    fields = re.findall(r"\b\w+\s+(\w+)\s*;", struct_m.group(1))

    table_m = re.search(r"sColorVariationOverrides\s*\[\s*\]\s*=\s*\{(.*?)\}\s*;", source, flags=re.S)
    if not table_m:
        raise ValueError(f"couldn't find sColorVariationOverrides[] in {C_SOURCE}")

    overrides: Dict[str, Dict[str, int]] = {}
    for entry in re.findall(r"\{([^{}]*)\}", table_m.group(1)):
        values = _split_top_level(entry)
        species = values[0]
        if not species.startswith("SPECIES_"):
            raise ValueError(f"unexpected override entry: {{{entry.strip()}}}")
        row = {name: 0 for name in fields[1:]}  # C zero-fills omitted fields
        for name, expr in zip(fields[1:], values[1:]):
            row[name] = _eval_c_expr(expr, defines)
        overrides[species[len("SPECIES_"):].lower()] = row
    return defines, overrides


DEFINES, OVERRIDES = load_color_variation_config()


def _define(name: str) -> int:
    return _eval_c_expr(name, DEFINES)


FP_SCALE = _define("FP_SCALE")
COLOR_VARIATION_MAX_ANGLE = _define("COLOR_VARIATION_MAX_ANGLE")  # sine-table units (256 = 360°)
COLOR_VARIATION_SAT_MUTED = _define("COLOR_VARIATION_SAT_MUTED") / FP_SCALE
COLOR_VARIATION_SAT_VIVID = _define("COLOR_VARIATION_SAT_VIVID") / FP_SCALE
COLOR_VARIATION_CHROMA_THRESHOLD = _define("COLOR_VARIATION_CHROMA_THRESHOLD")
TINT_FULL_GRAY = _define("TINT_FULL_GRAY")
TINT_FADE_START = _define("TINT_FADE_START")
TINT_MAX_CHROMA = _define("TINT_MAX_CHROMA")
TINT_LIGHT_ONE = _define("TINT_LIGHT_ONE")
SEPIA_R = _define("SEPIA_R")
SEPIA_G = _define("SEPIA_G")
SEPIA_B = _define("SEPIA_B")


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


def _fp_cos(angle: int) -> int:
    """Mirror of Cos(angle, FP_SCALE) using the game's 256-entry sine table."""
    table_val = round(math.cos((angle % 256) / 256.0 * 2.0 * math.pi) * 256)
    return (FP_SCALE * table_val) >> 8


def apply_neutral_tint(palette5: List[Tuple[int, int, int]], ov: Dict[str, int],
                       signed_step: int) -> None:
    """Mirror of ApplyNeutralTint(): lightens/darkens and tints gray colors."""
    angle = (ov["tintHue"] + signed_step * ov["tintSpread"]) % 256
    dirs = (_fp_cos(angle), _fp_cos(angle - 85), _fp_cos(angle + 85))
    light = -ov["tintDarken"] + _c_div((ov["tintDarken"] + ov["tintLighten"]) * (signed_step + 4), 7)
    light_mag = min(abs(light), TINT_LIGHT_ONE)
    tint_scale = TINT_LIGHT_ONE - light_mag
    for i in range(1, 16):
        r, g, b = palette5[i]
        if max(r, g, b) - min(r, g, b) >= TINT_MAX_CHROMA:
            continue
        gray = (r + g + b) // 3
        weight = min(gray, TINT_FULL_GRAY)
        if light > 0:
            lift = light_mag * weight
            r, g, b = (c + _c_div((31 - c) * lift, TINT_LIGHT_ONE * TINT_FULL_GRAY) for c in (r, g, b))
        elif light < 0:
            drop = light_mag
            if gray > TINT_FADE_START:
                drop = _c_div(drop * (31 - gray), 31 - TINT_FADE_START)
            r, g, b = (c - _c_div(c * drop, TINT_LIGHT_ONE) for c in (r, g, b))

        gray = (r + g + b) // 3
        weight = min(gray, TINT_FULL_GRAY)
        if gray > TINT_FADE_START:
            weight = _c_div(weight * (31 - gray), 31 - TINT_FADE_START)
        amount = _c_div(ov["tintStrength"] * weight * tint_scale, TINT_LIGHT_ONE)
        palette5[i] = tuple(to5(c + _c_div(d * amount, FP_SCALE * TINT_FULL_GRAY))
                            for c, d in zip((r, g, b), dirs))


def side_fade_amount(fade: int, signed_step: int) -> int:
    """Mirror of GetSideFadeAmount()."""
    amount = 0
    if fade > 0 and signed_step > 0:
        amount = _c_div(fade * signed_step, 3)
    elif fade < 0 and signed_step < 0:
        amount = _c_div(fade * signed_step, 4)
    return min(amount, TINT_LIGHT_ONE)


def side_fade_amounts(ov: Optional[Dict[str, int]], signed_step: int) -> Tuple[int, int, int]:
    """Mirror of GetSideFades(): (gray, white, brown) amounts."""
    if ov is None:
        return 0, 0, 0
    return tuple(side_fade_amount(ov.get(k, 0), signed_step)
                 for k in ("grayFade", "whiteFade", "brownFade"))


def apply_side_fades(r: int, g: int, b: int, fades: Tuple[int, int, int]) -> Tuple[int, int, int]:
    """Mirror of ApplySideFades()."""
    gray_amount, white_amount, brown_amount = fades
    if gray_amount:
        r, g, b = adjust_saturation(r, g, b, (TINT_LIGHT_ONE - gray_amount) / TINT_LIGHT_ONE)
    if brown_amount:
        gray = (r + g + b) // 3
        amount = brown_amount
        sepia = (min(31, gray * SEPIA_R // TINT_LIGHT_ONE),
                 gray * SEPIA_G // TINT_LIGHT_ONE,
                 gray * SEPIA_B // TINT_LIGHT_ONE)
        if gray > TINT_FADE_START:
            amount = _c_div(amount * (31 - gray), 31 - TINT_FADE_START)
        r, g, b = (c + _c_div((s - c) * amount, TINT_LIGHT_ONE) for c, s in zip((r, g, b), sepia))
    if white_amount:
        gray = (r + g + b) // 3
        lift = white_amount * min(gray, TINT_FULL_GRAY)
        r, g, b = (c + _c_div((31 - c) * lift, TINT_LIGHT_ONE * TINT_FULL_GRAY) for c in (r, g, b))
    return r, g, b


# ---------------------------------------------------------------------------
# Apply variation — must mirror ApplyIndividualColorVariation()
# ---------------------------------------------------------------------------
def apply_variation(palette5: List[Tuple[int, int, int]], shift: int,
                    species_name: Optional[str]) -> List[Tuple[int, int, int]]:
    """Apply variation to a 16-color 5-bit palette. Index 0 unchanged."""
    out = list(palette5)
    override = OVERRIDES.get(species_name.lower()) if species_name else None
    if override is not None:
        max_angle = override["maxAngle"]
        angle_bias = override["angleBias"]
        sat_cap = override["satCap"] / FP_SCALE
        sat_mul = override["satMul"] / FP_SCALE
    else:
        max_angle, angle_bias, sat_cap, sat_mul = COLOR_VARIATION_MAX_ANGLE, 0, 2.0, 1.0

    hue_step = shift & 0x7
    mode = (shift >> 3) & 0x7
    signed_step = hue_step - 4
    angle_mag = (abs(signed_step) * max_angle) // 4

    fades = side_fade_amounts(override, signed_step)
    if any(fades):
        angle_mag = 0

    if override is not None and (override["tintStrength"] or override["tintDarken"] or override["tintLighten"]):
        apply_neutral_tint(out, override, signed_step)

    if angle_mag == 0 and mode == 0 and angle_bias == 0 and not any(fades):
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

        r, g, b = apply_side_fades(r, g, b, fades)

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


def existing_preview_names(out_dir: Path) -> List[Tuple[str, bool]]:
    """Species that already have a sheet in out_dir, and whether a back sheet exists."""
    if not out_dir.is_dir():
        return []
    fronts = {p.name[:-len("_front_variations.png")] for p in out_dir.glob("*_front_variations.png")}
    backs = {p.name[:-len("_back_variations.png")] for p in out_dir.glob("*_back_variations.png")}
    return [(name, name in backs) for name in sorted(fronts | backs)]


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
    parser.add_argument("--overrides", action="store_true",
                        help="Generate every species listed in sColorVariationOverrides[].")
    parser.add_argument("--existing", action="store_true",
                        help="Regenerate every sheet already in the output directory "
                             "(back sheets too, where one exists).")
    args = parser.parse_args()

    names: List[str] = list(args.species)
    back_names = set()
    if args.overrides:
        names += list(OVERRIDES)
    if args.existing:
        existing = existing_preview_names(args.out)
        if not existing:
            print(f"No existing previews in {args.out}")
            return 1
        for name, has_back in existing:
            names.append(name)
            if has_back:
                back_names.add(name)
    if not names:
        try:
            line = input("Enter Pokémon names (space-separated, blank = all overrides): ").strip()
        except EOFError:
            line = ""
        names = line.split() or list(OVERRIDES)
    if not names:
        parser.print_help()
        return 1

    failures = 0
    seen = set()
    for raw in names:
        name = raw.strip().lower()
        if name in seen:
            continue
        seen.add(name)
        try:
            written = process_species(name, args.backs or name in back_names, args.out)
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
