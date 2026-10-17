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
    python generate_color_variations.py 25 172 26-27            # National Pokédex numbers / ranges
    python generate_color_variations.py --backs pikachu        # also do back sprite
    python generate_color_variations.py --overrides            # every overridden species
    python generate_color_variations.py --existing             # refresh every existing sheet
    python generate_color_variations.py --out previews bulbasaur
    python generate_color_variations.py "vaporeon<>green"      # suggest an entry that shifts AWAY from green
    python generate_color_variations.py "dragonite=blue"       # suggest an entry that shifts TOWARD blue
    python generate_color_variations.py vaporeon?              # suggest an entry that avoids the shiny's colors

Outputs: tech/color_variation_previews/<name>_front_variations.png
         (a 8×8 grid showing all 64 personality buckets, plus the shiny in the
         corner for comparison).

Suggestions ("name<>color" / "name=color"):
    Writes tech/color_variation_suggestions/<name>_<away|toward>_<color>.txt with a
    ready-to-paste override entry, a gentler and a bolder alternative, what to expect,
    and how to adjust it, plus a preview sheet with the entry already applied.
    Colors: red orange yellow lime green teal cyan azure blue indigo violet purple
    magenta pink rose (and gold amber crimson scarlet brown ...), or the tones
    gray white black. "<>" = away from, "=" = toward, "~" is a shell-safe alias for "<>".
    Several colors to avoid: "vaporeon<>green,yellow". In a command prompt, put "<>" in
    quotes (or use ~), otherwise cmd treats it as redirection.
    No direction at all: "vaporeon?" (same as "vaporeon<>shiny") treats the colors the shiny
    changes as the zones to avoid and aims for good variety everywhere else.

Requirements: Pillow.
"""
from __future__ import annotations

import argparse
import ast
import datetime
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
        row = {name: 0 for name in fields[1:]}  # C zero-fills omitted fields
        species = None
        seen = set()
        pos = 0  # next positional field index; C continues after the last named one
        for value in values:
            named = re.fullmatch(r"\.(\w+)\s*=\s*(.+)", value, flags=re.S)
            if named:
                name, expr = named.group(1), named.group(2).strip()
                if name not in fields:
                    raise ValueError(f"unknown field '.{name}' in override entry: {{{entry.strip()}}}")
                pos = fields.index(name)
            else:
                if pos >= len(fields):
                    raise ValueError(f"too many values in override entry: {{{entry.strip()}}}")
                name, expr = fields[pos], value
            if name in seen:
                # agbcc rejects this too ("field already initialized").
                raise ValueError(f"field '{name}' set twice in override entry: {{{entry.strip()}}}")
            seen.add(name)
            pos += 1
            if name == fields[0]:
                species = expr
            else:
                row[name] = _eval_c_expr(expr, defines)
        if species is None or not species.startswith("SPECIES_"):
            raise ValueError(f"unexpected override entry: {{{entry.strip()}}}")
        # The game uses the first entry for a species, so ignore later duplicates.
        overrides.setdefault(species[len("SPECIES_"):].lower(), row)
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
BAND_FEATHER = _define("BAND_FEATHER")


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


def color_hue256(r: int, g: int, b: int) -> int:
    """Mirror of GetColorHue256(): hue in sine-table units, or -1 if near-gray."""
    mx, mn = max(r, g, b), min(r, g, b)
    delta = mx - mn
    if delta < COLOR_VARIATION_CHROMA_THRESHOLD:
        return -1
    if mx == r:
        hue = _c_div((g - b) * 256, 6 * delta)
    elif mx == g:
        hue = _c_div((2 * delta + b - r) * 256, 6 * delta)
    else:
        hue = _c_div((4 * delta + r - g) * 256, 6 * delta)
    return hue & 0xFF


def band_darken_amount(darken: int, signed_step: int) -> int:
    """Mirror of GetBandDarkenAmount(): only the two extreme steps on one side."""
    amount = 0
    if darken > 0 and signed_step >= 2:
        amount = _c_div(darken * (signed_step - 1), 2)
    elif darken < 0 and signed_step <= -3:
        amount = _c_div(darken * (signed_step + 2), 2)
    return min(amount, TINT_LIGHT_ONE)


def apply_hue_band(palette5: List[Tuple[int, int, int]], ov: Dict[str, int], signed_step: int) -> None:
    """Mirror of ApplyHueBand()."""
    step_shift = _c_div(ov["bandShift"] * (signed_step + 4), 7)
    step_lighten = _c_div(ov.get("bandLighten", 0) * (signed_step + 4), 7)
    step_darken = band_darken_amount(ov.get("bandDarken", 0), signed_step)
    step_gray = _c_div(ov.get("bandGray", 0) * (signed_step + 4), 7)
    if step_shift == 0 and step_lighten == 0 and step_darken == 0 and step_gray == 0:
        return
    width = ov["bandWidth"]
    for i in range(1, 16):
        hue = color_hue256(*palette5[i])
        if hue < 0:
            continue
        dist = (hue - ov["bandHue"]) & 0xFF
        if dist > 128:
            dist = 256 - dist
        if dist > width + BAND_FEATHER:
            continue
        angle, lighten, darken, desat = step_shift, step_lighten, step_darken, step_gray
        if dist > width:
            angle = _c_div(angle * (width + BAND_FEATHER - dist), BAND_FEATHER)
            lighten = _c_div(lighten * (width + BAND_FEATHER - dist), BAND_FEATHER)
            darken = _c_div(darken * (width + BAND_FEATHER - dist), BAND_FEATHER)
            desat = _c_div(desat * (width + BAND_FEATHER - dist), BAND_FEATHER)
        if angle != 0:
            palette5[i] = rotate_color(*palette5[i], hue_matrix(angle & 0xFF))
        if lighten != 0:
            r, g, b = palette5[i]
            mx = max(r, g, b)
            palette5[i] = tuple(c + _c_div((31 - c) * lighten * mx, TINT_LIGHT_ONE * 31) for c in (r, g, b))
        if darken != 0:
            r, g, b = palette5[i]
            palette5[i] = tuple(c - _c_div(c * darken, TINT_LIGHT_ONE) for c in (r, g, b))
        if desat != 0:
            r, g, b = palette5[i]
            palette5[i] = adjust_saturation(r, g, b, (TINT_LIGHT_ONE - desat) / TINT_LIGHT_ONE)


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
def rotate_color_band_capped(r, g, b, angle_index, mat, override):
    """Mirror of RotateColorBandCapped(): trims positive rotation of band colors."""
    cap = override.get("bandMaxUp", 0) if override else 0
    signed = angle_index & 0xFF
    if signed > 128:
        signed -= 256
    if cap and override.get("bandWidth") and signed > cap:
        hue = color_hue256(r, g, b)
        if hue >= 0:
            dist = (hue - override.get("bandHue", 0)) & 0xFF
            if dist > 128:
                dist = 256 - dist
            if dist <= override["bandWidth"] + BAND_FEATHER + abs(override.get("bandShift", 0)):
                return rotate_color(r, g, b, hue_matrix(cap))
    return rotate_color(r, g, b, mat)


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

    if override is not None and override.get("bandWidth"):
        apply_hue_band(out, override, signed_step)

    if angle_mag == 0 and mode == 0 and angle_bias == 0 and not any(fades) and abs(sat_mul - 1.0) <= 1e-6:
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
                    r, g, b = rotate_color_band_capped(r, g, b, angle_index, mat_pos, override)
                else:
                    r, g, b = rotate_color_band_capped(r, g, b, (256 - angle_index) % 256, mat_neg, override)
            elif not has_split:
                r, g, b = rotate_color_band_capped(r, g, b, angle_index, mat_pos, override)

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


POKEDEX_HEADER = REPO_ROOT / "include" / "constants" / "pokedex.h"


def load_national_dex() -> List[str]:
    """Species folder names in National Pokédex order; index N = dex #N."""
    text = _strip_c_comments(POKEDEX_HEADER.read_text(encoding="utf-8"))
    enum_m = re.search(r"enum\s*\{\s*(NATIONAL_DEX_NONE\b.*?)\}\s*;", text, flags=re.S)
    if not enum_m:
        raise ValueError(f"couldn't find the National Dex enum in {POKEDEX_HEADER}")
    entries = re.findall(r"\bNATIONAL_DEX_(\w+)", enum_m.group(1))
    count_m = re.search(r"#define\s+NATIONAL_DEX_COUNT\s+NATIONAL_DEX_(\w+)", text)
    count = entries.index(count_m.group(1)) if count_m else len(entries) - 1
    return [name.lower() for name in entries[:count + 1]]


def resolve_species_tokens(tokens: List[str]) -> List[str]:
    """Turns user input into species folder names. Accepts names, National
    Pokédex numbers (25, 025, #25) and number ranges (25-26)."""
    dex: Optional[List[str]] = None
    names: List[str] = []
    for token in tokens:
        token = token.strip().lower().lstrip("#")
        m = re.fullmatch(r"(\d+)(?:-#?(\d+))?", token)
        if not m:
            names.append(token)
            continue
        if dex is None:
            dex = load_national_dex()
        lo = int(m.group(1))
        hi = int(m.group(2)) if m.group(2) else lo
        if lo > hi:
            lo, hi = hi, lo
        for num in range(lo, hi + 1):
            if 1 <= num < len(dex):
                names.append(dex[num])
            else:
                raise ValueError(f"#{num} is not a National Pokédex number (1-{len(dex) - 1})")
    return names


# ---------------------------------------------------------------------------
# Suggestion helper
#
#   "vaporeon<>green"   shift Vaporeon AWAY from green
#   "dragonite=blue"    shift Dragonite TOWARD blue
#
# Looks at the sprite's real palette, tries a few thousand candidate override
# entries through the same variation math the game uses (apply_variation), and
# writes a text file with a ready-to-paste entry, two alternatives, what to
# expect, and how to adjust it. Also renders a preview sheet with the entry
# already applied.
# ---------------------------------------------------------------------------
SUGGEST_DIR = Path(__file__).resolve().parent / "color_variation_suggestions"
_SUGGEST_KEY = "__suggest__"

# word -> (hue in sine-table units, half-width of the zone to avoid)
HUE_WORDS: Dict[str, Tuple[int, int]] = {
    "red": (0, 16), "scarlet": (3, 12), "crimson": (250, 12),
    "orange": (21, 12), "amber": (28, 10), "gold": (32, 10), "golden": (32, 10),
    "brown": (22, 14),
    "yellow": (43, 12), "lime": (62, 14), "chartreuse": (62, 14),
    "green": (85, 24), "emerald": (95, 16), "teal": (112, 14), "turquoise": (118, 14),
    "aqua": (128, 16), "cyan": (128, 16), "sky": (148, 14), "azure": (150, 14),
    "blue": (171, 22), "indigo": (188, 12), "violet": (198, 16), "purple": (200, 16),
    "magenta": (220, 16), "fuchsia": (224, 16), "pink": (240, 14), "rose": (245, 12),
}
# tone words: handled with fades / darken / lighten rather than a hue
NEUTRAL_WORDS = {"gray": "gray", "grey": "gray", "white": "white", "black": "black"}

_HUE_NAMES = [(0, "red"), (10, "red-orange"), (21, "orange"), (32, "amber"), (43, "yellow"),
              (58, "yellow-green"), (70, "lime"), (85, "green"), (105, "spring green"),
              (118, "teal"), (128, "cyan"), (148, "azure"), (171, "blue"), (186, "indigo"),
              (200, "violet"), (213, "magenta"), (232, "pink"), (246, "crimson")]

_DIRECTIVE_RE = re.compile(r"^(?P<sp>[^<>=~?]+?)\s*(?P<op><>|=|~)\s*(?P<col>[^<>=~]+?)$")

_DEFAULT_FIELDS = ["species", "maxAngle", "angleBias", "satCap", "satMul", "tintHue", "tintSpread",
                   "tintStrength", "tintDarken", "tintLighten", "grayFade", "whiteFade", "brownFade",
                   "bandHue", "bandWidth", "bandShift", "bandLighten", "bandDarken", "bandGray",
                   "bandMaxUp"]
_PRESETS = {
    "away":   {"gentle": dict(vw=0.6, d0=6),  "balanced": dict(vw=1.0, d0=14), "bold": dict(vw=1.6, d0=24)},
    "toward": {"gentle": dict(vw=0.8, pull=0.60), "balanced": dict(vw=0.8, pull=0.92),
               "bold": dict(vw=0.8, pull=1.00)},
}
_COMPLEXITY_FIELDS = ["angleBias", "tintHue", "tintSpread", "tintStrength", "tintDarken", "tintLighten",
                      "grayFade", "whiteFade", "brownFade", "bandShift", "bandLighten", "bandDarken",
                      "bandGray", "bandMaxUp"]


def hue_name(h: float) -> str:
    h = h % 256
    best = min(_HUE_NAMES, key=lambda t: min(abs(h - t[0]), 256 - abs(h - t[0])))
    return best[1]


def _circ(d: float) -> float:
    """Absolute circular distance for a hue difference, in sine-table units."""
    d = abs(d) % 256
    return d if d <= 128 else 256 - d


def _signed(d: float) -> float:
    d = (d + 128) % 256 - 128
    return d


def parse_directive(token: str) -> Optional[Tuple[str, str, List[str]]]:
    """'vaporeon<>green' -> ('vaporeon', 'away', ['green']);  'dragonite=blue' -> (..., 'toward', ...).
    '~' is a shell-safe alias for '<>'. Returns None for ordinary species tokens."""
    q = re.match(r"^([^<>=~?\s]+)\?$", token.strip())
    if q:                                  # "vaporeon?"  ==  "vaporeon<>shiny"
        return q.group(1), "away", ["shiny"]
    m = _DIRECTIVE_RE.match(token.strip())
    if not m:
        return None
    goal = "away" if m.group("op") in ("<>", "~") else "toward"
    colors = [c.strip() for c in re.split(r"[,+/]", m.group("col").lower()) if c.strip()]
    return m.group("sp").strip(), goal, colors


def split_directives(tokens: List[str]) -> Tuple[List[str], List[Tuple[str, str, List[str]]]]:
    """Separates 'name<>color' / 'name=color' directives from ordinary species tokens.
    Also accepts the spaced forms ('vaporeon <> green', 'dragonite = blue')."""
    text = " ".join(tokens)
    pattern = re.compile(r"([^\s<>=~?]+)\s*(<>|=|~)\s*([^\s<>=~?]+)")
    directives = []
    for m in pattern.finditer(text):
        d = parse_directive(m.group(0))
        if d:
            directives.append(d)
    text = pattern.sub(" ", text)
    question = re.compile(r"([^\s<>=~?]+)\?")
    for m in question.finditer(text):
        d = parse_directive(m.group(0))
        if d:
            directives.append(d)
    remaining = question.sub(" ", text).split()
    for tok in remaining:
        if any(ch in tok for ch in "<>=~?"):
            raise ValueError("can't read '%s': write it as name<>color (away from), name=color (toward) "
                             "or name? (away from the shiny), e.g. vaporeon<>green" % tok)
    return remaining, directives


def _shiny_zones(sprite: "_Sprite") -> List[Tuple[int, int]]:
    """Hues the shiny changes colors to (the places the normal variations shouldn't wander into)."""
    if not sprite.shiny:
        return []
    found = []
    for i in sprite.idxs:
        if sprite.weights[i] < 0.02:
            continue
        sh = color_hue256(*sprite.shiny[i])
        h0 = sprite.base_hue[i]
        if sh < 0:
            continue
        dist = 99 if h0 < 0 else _circ(sh - h0)
        if dist >= 6:
            # a small shiny shift (yellow -> orange) gets a narrow zone so the original isn't inside it
            found.append((sh, int(max(6, min(14, round(0.6 * dist))))))
    zones: List[Tuple[int, int]] = []
    for h, w in sorted(found):
        for k, (z, zw) in enumerate(zones):
            if _circ(h - z) < 10:
                zones[k] = (z, max(zw, w))
                break
        else:
            zones.append((h, w))
    return zones[:4]


def _override_fields() -> List[str]:
    try:
        source = _strip_c_comments(C_SOURCE.read_text(encoding="utf-8"))
        struct_m = re.search(r"struct\s+ColorVariationOverride\s*\{(.*?)\}\s*;", source, flags=re.S)
        return re.findall(r"\b\w+\s+(\w+)\s*;", struct_m.group(1))
    except Exception:  # noqa: BLE001
        return list(_DEFAULT_FIELDS)


def _new_row(**kw) -> Dict[str, int]:
    row = {f: 0 for f in _override_fields()[1:]}
    row.update(maxAngle=COLOR_VARIATION_MAX_ANGLE, satCap=FP_SCALE * 2, satMul=FP_SCALE)
    for k, v in kw.items():
        if k not in row:
            raise KeyError(f"override field '{k}' doesn't exist in the C struct (is the script in sync?)")
        row[k] = int(v)
    for k in ("angleBias", "bandShift", "maxAngle", "grayFade", "whiteFade", "brownFade", "bandDarken",
              "tintSpread"):
        if k in row:
            row[k] = max(-127, min(127, row[k]))
    return row


# ---- sprite analysis -------------------------------------------------------
class _Sprite:
    def __init__(self, name: str):
        d = GFX_ROOT / name
        if not d.is_dir():
            raise FileNotFoundError(f"No graphics directory for '{name}': {d}")
        self.name = name
        self.dir = d
        self.pal = load_jasc_pal(d / "normal.pal")
        shiny = d / "shiny.pal"
        self.shiny = load_jasc_pal(shiny) if shiny.exists() else None
        self.front = d / "front.png"
        img = load_indexed_sprite(self.front).crop((0, 0, 64, 64))
        counts = [0] * 16
        for v in img.tobytes():
            if v < 16:
                counts[v] += 1
        total = sum(counts[1:]) or 1
        self.weights = [0.0] + [c / total for c in counts[1:]]
        self.idxs = [i for i in range(1, 16) if self.weights[i] > 0.003]
        self.base_hue = {i: color_hue256(*self.pal[i]) for i in self.idxs}
        chroma_w = sum(self.weights[i] for i in self.idxs if self.base_hue[i] >= 0)
        self.chroma_share = chroma_w
        self.norm = chroma_w if chroma_w > 0.05 else (sum(self.weights[i] for i in self.idxs) or 1.0)
        self.clusters = self._clusters()
        self.dominant = max((i for i in self.idxs if self.base_hue[i] >= 0),
                            key=lambda i: self.weights[i], default=None)
        # colors of the biggest hue cluster count fully when judging "toward"; the rest count a little
        self.core = {i: 1.0 for i in self.idxs}
        if self.clusters:
            c0 = self.clusters[0]
            reach = max(abs(c0["lo"]), abs(c0["hi"])) + 2
            for i in self.idxs:
                h = self.base_hue[i]
                self.core[i] = 1.0 if (h >= 0 and _circ(h - c0["center"]) <= reach) else 0.3

    def _clusters(self) -> List[Dict[str, float]]:
        items = sorted((self.base_hue[i], self.weights[i]) for i in self.idxs if self.base_hue[i] >= 0)
        if not items:
            return []
        groups: List[List[Tuple[int, float]]] = [[items[0]]]
        for it in items[1:]:
            if it[0] - groups[-1][-1][0] > 14:
                groups.append([it])
            else:
                groups[-1].append(it)
        if len(groups) > 1 and (items[0][0] + 256 - items[-1][0]) <= 14:
            groups[0] = groups.pop() + groups[0]
        out = []
        for g in groups:
            w = sum(x[1] for x in g)
            cx = sum(math.cos(h / 256 * 2 * math.pi) * x for h, x in g)
            cy = sum(math.sin(h / 256 * 2 * math.pi) * x for h, x in g)
            center = (math.atan2(cy, cx) / (2 * math.pi) * 256) % 256
            offs = [_signed(h - center) for h, _ in g]
            out.append(dict(weight=w, center=int(round(center)) % 256, lo=min(offs), hi=max(offs)))
        out.sort(key=lambda c: -c["weight"])
        return out


def _band_for(cluster: Dict[str, float]) -> Tuple[int, int]:
    half = max(abs(cluster["lo"]), abs(cluster["hi"]))
    return int(cluster["center"]), int(max(6, min(16, math.ceil(half) + 3)))


# ---- simulation and metrics --------------------------------------------------
def _simulate(sprite: _Sprite, row: Dict[str, int]) -> List[List[Tuple[int, int, int]]]:
    OVERRIDES[_SUGGEST_KEY] = row
    try:
        return [apply_variation(sprite.pal, s, _SUGGEST_KEY) for s in range(64)]
    finally:
        OVERRIDES.pop(_SUGGEST_KEY, None)


def _std(vals: List[float]) -> float:
    n = len(vals)
    if n < 2:
        return 0.0
    m = sum(vals) / n
    return math.sqrt(sum((v - m) ** 2 for v in vals) / n)


def _circ_std(hues: List[int]) -> float:
    if len(hues) < 8:
        return 0.0
    c = sum(math.cos(h / 256 * 2 * math.pi) for h in hues) / len(hues)
    s = sum(math.sin(h / 256 * 2 * math.pi) for h in hues) / len(hues)
    r = max(1e-6, min(1.0, math.hypot(c, s)))
    return min(40.0, math.sqrt(-2 * math.log(r)) * 256 / (2 * math.pi))


def _measure(sprite: _Sprite, row: Dict[str, int], goal: Dict) -> Dict[str, float]:
    outs = _simulate(sprite, row)
    norm = sprite.norm
    zones = goal.get("zones", [])
    target = goal.get("target")
    zone = near = gain = close = shiny_clash = drift = chroma_loss = 0.0
    hue_spread = light_var = sat_var = 0.0
    maxdrift = 0.0
    spread_w = 0.0
    all_w = sum(sprite.weights[i] for i in sprite.idxs) or 1.0
    shiny_idx = []
    if sprite.shiny:
        for i in sprite.idxs:
            sh = color_hue256(*sprite.shiny[i])
            if sh >= 0 and (sprite.base_hue[i] < 0 or _circ(sh - sprite.base_hue[i]) > 12):
                shiny_idx.append((i, sh))
    shiny_w = sum(sprite.weights[i] for i, _ in shiny_idx)
    for i in sprite.idxs:
        w = sprite.weights[i]
        hues, vals, chromas = [], [], []
        h0 = sprite.base_hue[i]
        c0 = max(sprite.pal[i]) - min(sprite.pal[i])
        for s in range(64):
            r, g, b = outs[s][i]
            h = color_hue256(r, g, b)
            vals.append(max(r, g, b) * 8 / 255.0)
            chromas.append(max(r, g, b) - min(r, g, b))
            if h >= 0:
                hues.append(h)
                if zones:
                    pz = max((max(0.0, 1 - _circ(h - T) / W) for T, W in zones), default=0.0)
                    zone += w * pz / 64
                    # safety margin: how close to the edge of a zone (including inside it)
                    edge = min(_circ(h - T) - W for T, W in zones)
                    near += w * max(0.0, 1 - max(0.0, edge) / 10.0) / 64
                    if h0 >= 0:
                        gain += w * (min(_circ(h - T) for T, W in zones)
                                     - min(_circ(h0 - T) for T, W in zones)) / 64
                if target is not None:
                    close += w * sprite.core[i] * max(0.0, 1 - _circ(h - target) / 64) / 64
                if h0 >= 0:
                    dd = _circ(h - h0)
                    drift += w * dd / 64
                    if w >= 0.03:
                        maxdrift = max(maxdrift, dd)
        light_var += w * _std(vals)
        sat_var += w * _std(chromas)
        if h0 >= 0 and c0 > 0:
            chroma_loss += w * sum(max(0.0, (c0 - c) / c0) for c in chromas) / 64
        if len(hues) >= 8:
            hue_spread += w * _circ_std(hues)
            spread_w += w
    for i, sh in shiny_idx:
        w = sprite.weights[i]
        for s in range(64):
            h = color_hue256(*outs[s][i])
            if h >= 0:
                d = _circ(h - sh)
                if d < 10:
                    shiny_clash += w * (1 - d / 10) / 64
    complexity = sum(1 for f in _COMPLEXITY_FIELDS if row.get(f, 0) != 0)
    complexity += (row.get("satMul", FP_SCALE) != FP_SCALE) + (row.get("satCap", FP_SCALE * 2) != FP_SCALE * 2)
    return dict(zone=zone / norm, near=near / norm, gain=gain / norm, close=close / norm,
                loss=chroma_loss / norm,
                drift=drift / norm, maxdrift=maxdrift,
                hue_spread=hue_spread / (spread_w or 1.0), light_var=light_var / all_w,
                sat_var=sat_var / all_w, shiny=(shiny_clash / shiny_w) if shiny_w else 0.0,
                complexity=complexity)


def _score(m: Dict[str, float], goal: Dict, preset: Dict, ref: Dict) -> float:
    variety = (min(m["hue_spread"], 10) / 10 + 0.25 * min(m["light_var"], 0.12) / 0.12
               + 0.15 * min(m["sat_var"], 2.5) / 2.5)
    cx = 0.10 * m["complexity"]
    if goal["kind"] == "away":
        return (-40 * m["zone"] - 8 * m["near"] + 0.10 * max(-10.0, min(m["gain"], 6.0))
                - 0.3 * max(0.0, -m["gain"]) - 6 * m["loss"]
                + preset["vw"] * variety - 0.12 * max(0, m["drift"] - preset["d0"])
                - 0.08 * max(0, m["maxdrift"] - (preset["d0"] + 14)) - cx - 0.8 * m["shiny"])
    span = ref["cmax"] - ref["c0"]
    rel = max(0.0, min(1.2, (m["close"] - ref["c0"]) / span)) if span > 1e-6 else 0.0
    return (-7 * abs(rel - preset["pull"]) + preset["vw"] * variety * 0.5 - 6 * m["loss"]
            - cx - 1.0 * m["shiny"])


# ---- candidate generation -----------------------------------------------------
def _rows_rotation(goal: Dict) -> List[Dict[str, int]]:
    angles = [4, 6, 8, 10, 14, 18, 24, 32]
    biases = list(range(-126, 127, 6)) if goal["kind"] == "toward" else list(range(-60, 61, 4))
    return [_new_row(maxAngle=a, angleBias=b) for a in angles for b in biases]


def _rows_fades() -> List[Dict[str, int]]:
    rows = []
    for kind in ("whiteFade", "grayFade", "brownFade"):
        for sign in (1, -1):
            for mag in (24, 40):
                for a in (8, 14, 20):
                    for b in (-16, -8, 0, 8, 16):
                        rows.append(_new_row(maxAngle=a, angleBias=b, **{kind: sign * mag}))
    return rows


def _rows_band(sprite: _Sprite, goal: Dict) -> List[Dict[str, int]]:
    rows = []
    for cl in sprite.clusters[:2]:
        if cl["weight"] < 0.06:
            continue
        bh, bw = _band_for(cl)
        if goal["kind"] == "toward":
            # the band ramps from the original color (step -4) to full shift (step +3), so
            # never shift further than it takes to reach the target (plus a little)
            need = _signed(goal["target"] - cl["center"])
            top = int(abs(need) * 1.15)
            sign = 1 if need >= 0 else -1
            shifts = sorted({sign * v for v in range(5, max(6, top) + 1, 5)} | {int(need)})
        else:
            shifts = list(range(-80, 81, 10))
        for shift in shifts:
            for a in (0, 6, 12):
                for b in (-6, 0, 6):
                    for cap in (0, 4):
                        if cap and a == 0:
                            continue
                        rows.append(_new_row(maxAngle=a, angleBias=b, bandHue=bh, bandWidth=bw,
                                             bandShift=shift, bandMaxUp=cap))
    return rows


def _rows_band_fade(sprite: _Sprite) -> List[Dict[str, int]]:
    rows = []
    for cl in sprite.clusters[:1]:
        bh, bw = _band_for(cl)
        for kind in ("whiteFade", "grayFade", "brownFade"):
            for sign in (1, -1):
                for shift in range(-60, 61, 12):
                    for a in (6, 12):
                        rows.append(_new_row(maxAngle=a, bandHue=bh, bandWidth=bw, bandShift=shift,
                                             **{kind: sign * 24}))
    return rows


def _rows_tint() -> List[Dict[str, int]]:
    rows = []
    for hue in range(0, 256, 8):
        for spread in (0, 4, 7):
            for strength in (3, 5):
                for a in (0, 10):
                    rows.append(_new_row(maxAngle=a, tintHue=hue, tintSpread=spread, tintStrength=strength))
    return rows


def _style_variants(row: Dict[str, int]) -> List[Dict[str, int]]:
    out = []
    if row["tintStrength"]:
        for dk, lt in ((28, 0), (0, 28), (24, 24)):
            v = dict(row)
            v.update(tintDarken=dk, tintLighten=lt)
            out.append(v)
    return out


def _pick(evaluated, goal, ref):
    result = {}
    for name, preset in _PRESETS[goal["kind"]].items():
        best = max(evaluated, key=lambda rm: (round(_score(rm[1], goal, preset, ref), 4), -rm[1]["complexity"]))
        result[name] = dict(row=best[0], metrics=best[1])
    return result


def _search(sprite: _Sprite, goal: Dict) -> Dict[str, Dict]:
    """Returns {'gentle'|'balanced'|'bold': {row, metrics}} for a hue goal. Tries plain
    rotation + bias, one-sided fades, a hue band (alone and with a fade) and, for mostly
    gray sprites, a tint, then keeps whatever scores best (extra fields cost points)."""
    toward = goal["kind"] == "toward"
    neutral = sprite.chroma_share < 0.35
    evaluated = [(r, _measure(sprite, r, goal)) for r in _rows_rotation(goal)]
    ref = {"c0": 0.0, "cmax": max(m["close"] for _, m in evaluated)}
    if toward:
        ref["c0"] = _measure(sprite, _new_row(maxAngle=0), goal)["close"]
    presets = _PRESETS[goal["kind"]]
    balanced = max(evaluated, key=lambda rm: _score(rm[1], goal, presets["balanced"], ref))
    # a "toward" goal always compares rotation against a hue band; an "away" goal only reaches
    # for fades / bands / tints when plain rotation can't keep clear of the zone
    needs_more = True   # richer families only win when they score clearly better (complexity is penalized)
    if needs_more:
        if not toward:
            evaluated += [(r, _measure(sprite, r, goal)) for r in _rows_fades()]
        evaluated += [(r, _measure(sprite, r, goal)) for r in _rows_band(sprite, goal)]
        if not toward:
            evaluated += [(r, _measure(sprite, r, goal)) for r in _rows_band_fade(sprite)]
        if neutral and toward:
            evaluated += [(r, _measure(sprite, r, goal)) for r in _rows_tint()]
        ref["cmax"] = max(m["close"] for _, m in evaluated)
        ranked = sorted(evaluated, key=lambda rm: -_score(rm[1], goal, presets["balanced"], ref))
        seen, extra = set(), []
        for r, _ in ranked:
            key = tuple(sorted(r.items()))
            if r["tintStrength"] and key not in seen and len(seen) < 10:
                seen.add(key)
                extra += [(v, _measure(sprite, v, goal)) for v in _style_variants(r)]
        evaluated += extra
    result = _pick(evaluated, goal, ref)
    result["_ref"] = ref
    return result


# ---- tone goals (gray / white / black / brown) -------------------------------------
def _tone_rows(sprite: _Sprite, goal: Dict) -> Dict[str, Dict]:
    word, kind = goal["word"], goal["kind"]
    cl = sprite.clusters[0] if sprite.clusters and sprite.clusters[0]["weight"] >= 0.06 else None
    band = dict(bandHue=_band_for(cl)[0], bandWidth=_band_for(cl)[1]) if cl else {}
    neutral = (1 - sprite.chroma_share) > 0.15
    if kind == "toward":
        if word == "gray":
            specs = {"gentle": dict(satMul=1024, bandGray=24), "balanced": dict(satMul=880, bandGray=40),
                     "bold": dict(satMul=720, bandGray=56)}
        elif word == "white":
            specs = {"gentle": dict(bandLighten=16, tintLighten=16 if neutral else 0),
                     "balanced": dict(bandLighten=28, tintLighten=28 if neutral else 0),
                     "bold": dict(bandLighten=44, tintLighten=44 if neutral else 0)}
        elif word == "black":
            specs = {"gentle": dict(bandDarken=-24, tintDarken=24 if neutral else 0),
                     "balanced": dict(bandDarken=-40, tintDarken=40 if neutral else 0),
                     "bold": dict(bandDarken=-64, tintDarken=52 if neutral else 0)}
        else:
            specs = {"gentle": dict(brownFade=-20), "balanced": dict(brownFade=-32, satMul=940),
                     "bold": dict(brownFade=-48, satMul=880)}
        rows = {}
        for name, spec in specs.items():
            spec = dict(spec)
            if any(k.startswith("band") for k in spec) and not band:
                spec = {k: v for k, v in spec.items() if not k.startswith("band")}
            if "bandGray" in spec or "bandLighten" in spec or "bandDarken" in spec:
                spec.update(band)
            rows[name] = _new_row(**spec)
    else:
        if word == "gray":
            rows = {"gentle": _new_row(satMul=1100), "balanced": _new_row(satMul=1180),
                    "bold": _new_row(satMul=1229)}
        else:  # keep away from white / black: the default entry never lightens or darkens colored pixels
            rows = {n: _new_row() for n in ("gentle", "balanced", "bold")}
    return {n: dict(row=r, metrics=_measure(sprite, r, goal)) for n, r in rows.items()}


# ---- entry formatting ----------------------------------------------------------------
def _fmt_value(field: str, v: int) -> str:
    if field == "maxAngle" and v == COLOR_VARIATION_MAX_ANGLE:
        return "COLOR_VARIATION_MAX_ANGLE"
    if field == "satCap":
        return "FP_SCALE * 2" if v == FP_SCALE * 2 else ("FP_SCALE" if v == FP_SCALE else str(v))
    if field == "satMul" and v == FP_SCALE:
        return "FP_SCALE"
    return str(v)


def _format_entry(name: str, row: Dict[str, int]) -> str:
    fields = _override_fields()[1:]
    vals = [_fmt_value(f, row.get(f, 0)) for f in fields]
    last_nonzero = max((i for i, f in enumerate(fields) if row.get(f, 0) != 0
                        and not (f == "satMul" and row[f] == FP_SCALE)
                        and not (f == "satCap" and row[f] == FP_SCALE * 2)
                        and not (f == "maxAngle" and row[f] == COLOR_VARIATION_MAX_ANGLE)), default=-1)
    keep = max(fields.index("satMul") + 1, last_nonzero + 1)
    return "{ SPECIES_%s, %s }," % (name.upper(), ", ".join(vals[:keep]))


def _format_named(name: str, row: Dict[str, int]) -> str:
    fields = _override_fields()[1:]
    parts = ["    .species = SPECIES_%s" % name.upper()]
    for f in fields:
        v = row.get(f, 0)
        if f in ("maxAngle", "satCap", "satMul") or v != 0:
            parts.append("    .%s = %s" % (f, _fmt_value(f, v)))
    return "{\n" + ",\n".join(parts) + ",\n},"


# ---- report ---------------------------------------------------------------------------
_STEPS = list(range(-4, 4))
_SPLIT_MODES = (3, 6, 7)


def _dominant_table(sprite: _Sprite, row: Dict[str, int]) -> List[str]:
    i = sprite.dominant
    if i is None:
        i = max(sprite.idxs, key=lambda k: sprite.weights[k], default=None)
        if i is None:
            return ["  (no pixels to analyze)"]
    outs = _simulate(sprite, row)
    now = ("%s, hue %d" % (hue_name(sprite.base_hue[i]), sprite.base_hue[i])) if sprite.base_hue[i] >= 0 \
        else "gray/white/black (a tint is needed to give it a hue)"
    lines = ["  Main color (palette entry #%d, %.0f%% of the sprite, now %s):"
             % (i, sprite.weights[i] * 100, now),
             "    step                " + "".join("%9s" % ("%+d" % s if s else "0") for s in _STEPS)]
    for label, modes in (("normal modes", (0, 1, 2, 4, 5)), ("split modes ", _SPLIT_MODES)):
        cells = []
        for s in _STEPS:
            hs = []
            for mode in modes:
                shift = mode * 8 + (s + 4)
                h = color_hue256(*outs[shift][i])
                if h >= 0:
                    hs.append(h)
            cells.append("%9s" % (("%d-%d" % (min(hs), max(hs))) if hs and min(hs) != max(hs)
                                  else (str(hs[0]) if hs else "gray")))
        lines.append("    hue, %s  " % label + "".join(cells))
    return lines


def _zone_hits(sprite: _Sprite, row: Dict[str, int], zones: List[Tuple[int, int]]) -> int:
    """Number of the 64 variations where more than 5% of the sprite's pixels land in a zone."""
    outs = _simulate(sprite, row)
    hits = 0
    for s in range(64):
        w = 0.0
        for i in sprite.idxs:
            h = color_hue256(*outs[s][i])
            if h >= 0 and any(_circ(h - T) < W for T, W in zones):
                w += sprite.weights[i]
        if w > 0.05:
            hits += 1
    return hits


def _tone_change(sprite: _Sprite, row: Dict[str, int]) -> Tuple[float, float]:
    """Average % change in saturation and brightness across the 64 variations."""
    outs = _simulate(sprite, row)
    sat_b = sum(sprite.weights[i] * (max(sprite.pal[i]) - min(sprite.pal[i])) for i in sprite.idxs)
    val_b = sum(sprite.weights[i] * max(sprite.pal[i]) for i in sprite.idxs)
    sat_a = sum(sprite.weights[i] * (max(c[i]) - min(c[i])) for c in outs for i in sprite.idxs) / 64
    val_a = sum(sprite.weights[i] * max(c[i]) for c in outs for i in sprite.idxs) / 64
    return ((sat_a / sat_b - 1) * 100 if sat_b else 0.0, (val_a / val_b - 1) * 100 if val_b else 0.0)


def _describe_fields(sprite: _Sprite, row: Dict[str, int], goal: Dict) -> List[str]:
    base = sprite.dominant is not None and sprite.base_hue[sprite.dominant] or 0
    out = []
    deg = lambda u: abs(u) * 360 / 256
    if row["maxAngle"] == 0:
        out.append("maxAngle 0: no general hue rotation, so nothing wanders; all hue change comes from the other fields.")
    else:
        out.append("maxAngle %d: the hue rotation swings up to about +-%.0f degrees across the 8 steps%s."
                   % (row["maxAngle"], deg(row["maxAngle"]),
                      " (the game default)" if row["maxAngle"] == COLOR_VARIATION_MAX_ANGLE else ""))
    if row["angleBias"]:
        d = row["angleBias"]
        out.append("angleBias %+d: leans every variation about %.0f degrees along the hue wheel (the main color's hue %d becomes about %d, %s -> %s)."
                   % (d, deg(d), base, (base + d) % 256, hue_name(base), hue_name(base + d)))
    if row["satCap"] != FP_SCALE * 2 or row["satMul"] != FP_SCALE:
        out.append("satCap %d / satMul %d: saturation control (satMul %.0f%% is applied to everything)."
                   % (row["satCap"], row["satMul"], row["satMul"] * 100 / FP_SCALE))
    if row["tintStrength"]:
        out.append("tintHue %d / tintSpread %d / tintStrength %d: adds a %s-ish tint to the gray pixels, moving %d units per step."
                   % (row["tintHue"], row["tintSpread"], row["tintStrength"], hue_name(row["tintHue"]), row["tintSpread"]))
    if row["tintDarken"] or row["tintLighten"]:
        out.append("tintDarken %d / tintLighten %d: the gray pixels run from darker (step -4) to lighter (step +3)."
                   % (row["tintDarken"], row["tintLighten"]))
    for f, label in (("grayFade", "toward gray"), ("whiteFade", "toward white"), ("brownFade", "toward brown")):
        if row[f]:
            side = "steps +1..+3" if row[f] > 0 else "steps -1..-4"
            out.append("%s %+d: %s fade %s instead of rotating hue (full amount at the end of that side)."
                       % (f, row[f], side, label))
    if row["bandWidth"]:
        out.append("bandHue %d / bandWidth %d: only the colors near %s (hue %d) get the band effects below."
                   % (row["bandHue"], row["bandWidth"], hue_name(row["bandHue"]), row["bandHue"]))
        if row["bandShift"]:
            out.append("bandShift %+d: that color rotates %s about %.0f degrees, none at step -4, full at step +3."
                       % (row["bandShift"], "up the hue wheel" if row["bandShift"] > 0 else "down the hue wheel",
                          deg(row["bandShift"])))
        if row["bandLighten"]:
            out.append("bandLighten %d: that color gets paler toward step +3." % row["bandLighten"])
        if row["bandDarken"]:
            out.append("bandDarken %+d: that color darkens at the %s end (step %s full)."
                       % (row["bandDarken"], "negative" if row["bandDarken"] < 0 else "positive",
                          "-4" if row["bandDarken"] < 0 else "+3"))
        if row["bandGray"]:
            out.append("bandGray %d: that color gets duller toward step +3." % row["bandGray"])
        if row["bandMaxUp"]:
            out.append("bandMaxUp %d: that color can't be rotated more than %.0f degrees up the hue wheel, in any mode."
                       % (row["bandMaxUp"], deg(row["bandMaxUp"])))
    return out


def _adjust_rows(sprite: _Sprite, row: Dict[str, int], goal: Dict) -> List[Tuple[str, str]]:
    rows: List[Tuple[str, str]] = []
    base = sprite.base_hue[sprite.dominant] if sprite.dominant is not None else 0
    if goal.get("hue") and row["tintStrength"] and sprite.dominant is None and goal["kind"] == "toward":
        # a gray sprite gets its hue from the tint, so the tint is the knob for "how blue"
        word = goal["word"]
        rows.append(("Not %s enough" % word,
                     "tintStrength %d -> %d, or set tintHue to %d (the %s hue)" % (
                         row["tintStrength"], row["tintStrength"] + 1, goal["target"], word)))
        parts = []
        if row["tintSpread"]:
            parts.append("tintSpread %d -> %d" % (row["tintSpread"], max(0, row["tintSpread"] - 2)))
        if row["maxAngle"]:
            parts.append("maxAngle %d -> %d" % (row["maxAngle"], max(0, row["maxAngle"] - 4)))
        if parts:
            rows.append(("Tint colors drift away from %s" % word, "lower " + " and/or ".join(parts) + " (both move the hue off center)"))
    elif goal.get("hue") and (goal["kind"] == "toward" or goal.get("zones")):
        ref_hue = goal["target"] if goal["kind"] == "toward" else goal["zones"][0][0]
        to_sign = 1 if _signed(ref_hue - base) >= 0 else -1      # direction that heads toward the goal color
        lean = to_sign if goal["kind"] == "toward" else -to_sign   # direction we want to move
        field = "bandShift" if (row["bandWidth"] and row["bandShift"]) else "angleBias"
        cur = row[field]
        up = hue_name(base + 24)
        down = hue_name(base - 24)
        rows.append(("(for this sprite)", "a HIGHER %s turns the main color toward %s, a LOWER one toward %s" % (field, up, down)))
        word = goal["word"] if goal["word"] != "shiny" else "shiny-like"
        if goal["kind"] == "away":
            rows.append(("Still seeing %s-ish cells" % word, "%s %+d -> %+d (or %+d)" % (field, cur, cur + 4 * lean, cur + 8 * lean)))
            rows.append(("Moved too far from the original color", "%s %+d -> %+d" % (field, cur, cur - 4 * lean)))
        else:
            rows.append(("Not %s enough" % word, "%s %+d -> %+d (or %+d)" % (field, cur, cur + 6 * lean, cur + 12 * lean)))
            rows.append(("Gone too far toward %s" % word, "%s %+d -> %+d" % (field, cur, cur - 6 * lean)))
    if row["maxAngle"] > 0:
        rows.append(("Variants too similar to each other",
                     "raise maxAngle %d -> %d (1 unit is about 1.4 degrees)" % (row["maxAngle"], row["maxAngle"] + 3)))
        rows.append(("Variants stray too far from the original",
                     "lower maxAngle %d -> %d" % (row["maxAngle"], max(2, row["maxAngle"] - 3))))
    if row["bandWidth"]:
        rows.append(("Part of the color is left behind (shading, edges)",
                     "raise bandWidth %d -> %d" % (row["bandWidth"], row["bandWidth"] + 3)))
        rows.append(("Another part of the sprite changed too",
                     "lower bandWidth %d -> %d" % (row["bandWidth"], max(3, row["bandWidth"] - 3))))
    for f, more, less in (("bandLighten", "paler", "less pale"), ("bandDarken", "darker", "less dark"),
                          ("bandGray", "duller", "less dull"), ("grayFade", "grayer", "less gray"),
                          ("whiteFade", "paler", "less pale"), ("brownFade", "browner", "less brown")):
        v = row[f]
        if v:
            sign = 1 if v > 0 else -1
            rows.append(("%s: want it %s / %s" % (f, more, less),
                         "%s %+d -> %+d / %+d" % (f, v, v + 8 * sign, v - 8 * sign)))
    if row["bandMaxUp"]:
        rows.append(("A color still drifts too far up the hue wheel",
                     "lower bandMaxUp %d -> %d" % (row["bandMaxUp"], max(1, row["bandMaxUp"] - 2))))
    for f, more, less in (("tintDarken", "darker dark end", "lighter dark end"),
                          ("tintLighten", "lighter light end", "less light")):
        if row[f]:
            rows.append(("%s: want a %s / %s" % (f, more, less),
                         "%s %d -> %d / %d" % (f, row[f], row[f] + 8, max(0, row[f] - 8))))
    if row["tintStrength"]:
        rows.append(("Gray parts too plain / too colored",
                     "tintStrength %d -> %d / %d" % (row["tintStrength"], row["tintStrength"] + 1, max(1, row["tintStrength"] - 1))))
        rows.append(("Tint hues too alike / too different",
                     "tintSpread %d -> %d / %d" % (row["tintSpread"], row["tintSpread"] + 2, max(0, row["tintSpread"] - 2))))
    if row["satMul"] != FP_SCALE:
        rows.append(("Colors too washed out / too loud",
                     "satMul %d -> %d / %d" % (row["satMul"], row["satMul"] + 100, row["satMul"] - 100)))
    rows.append(("Want the same idea but milder / stronger", "use the GENTLER or BOLDER line above, then fine tune"))
    rows.append(("Shiny comes too close", "compare the red-framed cell; nudge the field above that fixes the main problem"))
    return rows


def _write_report(sprite: _Sprite, goal: Dict, results: Dict[str, Dict], out_path: Path, preview: Optional[Path]) -> None:
    name = sprite.name
    bal = results["balanced"]
    row = bal["row"]
    lines: List[str] = []
    add = lines.append
    kind_text = {"away": "AWAY from", "toward": "TOWARD"}[goal["kind"]]
    add("=" * 72)
    add(" COLOR VARIATION SUGGESTION")
    add(" Pokemon : %s" % name)
    add(" Goal    : shift %s %s" % (kind_text, ", ".join(goal["words"])))
    add(" Created : %s" % datetime.datetime.now().strftime("%Y-%m-%d %H:%M"))
    add("=" * 72)
    add("")
    add("1. PASTE THIS LINE")
    add("   into sColorVariationOverrides[] in src/pokemon_color_variation.c")
    add("   (evolutions need their own lines; if the species already has a line, replace it,")
    add("   because only the first entry for a species is used)")
    add("")
    add("    " + _format_entry(name, row))
    add("")
    add("   The same entry with field names (easier to read and edit; don't mix this style")
    add("   with the positional one inside a single entry):")
    add("")
    for ln in _format_named(name, row).splitlines():
        add("    " + ln)
    add("")
    add("   Two alternatives built the same way:")
    add("")
    add("   GENTLER (stays closer to the original):")
    add("    " + _format_entry(name, results["gentle"]["row"]))
    add("   BOLDER (more change):")
    add("    " + _format_entry(name, results["bold"]["row"]))
    if preview is not None:
        add("")
        add("   A preview sheet of the recommended entry is already saved:")
        try:
            add("    " + str(preview.relative_to(REPO_ROOT)))
        except ValueError:
            add("    " + str(preview))
        add("   (green frame = original, red frame = shiny, the other 64 cells = every variation)")
    add("")
    add("2. WHAT THE SPRITE CONTAINS (from graphics/pokemon/%s/normal.pal and front.png)" % name)
    add("")
    add("    share   hue  color        shiny hue")
    cols = sorted(sprite.idxs, key=lambda i: -sprite.weights[i])
    for i in cols[:8]:
        h = sprite.base_hue[i]
        sh = color_hue256(*sprite.shiny[i]) if sprite.shiny else -1
        add("    %4.0f%%  %4s  %-12s %s" % (sprite.weights[i] * 100, h if h >= 0 else "-",
                                           hue_name(h) if h >= 0 else "gray/white/black",
                                           ("%d (%s)" % (sh, hue_name(sh))) if sh >= 0 else "-"))
    if goal.get("zones"):
        for T, W in goal["zones"]:
            add("")
            add("    Zone to avoid: %s, hue %d +- %d (about %d to %d)." % (hue_name(T), T, W, (T - W) % 256, (T + W) % 256))
    if goal.get("target") is not None:
        add("")
        add("    Target: %s, hue %d." % (hue_name(goal["target"]), goal["target"]))
    add("")
    add("3. WHAT THE RECOMMENDED ENTRY DOES")
    add("")
    for ln in _describe_fields(sprite, row, goal):
        add("    - " + ln)
    for note in goal.get("notes", []):
        add("    * " + note)
    add("")
    add("4. WHAT TO EXPECT (from the same math the game uses)")
    add("")
    if goal.get("zones"):
        before = _zone_hits(sprite, _new_row(), goal["zones"])
        after = _zone_hits(sprite, row, goal["zones"])
        add("    Variations (out of 64) with a noticeable part of the sprite inside the zone:")
        add("      default entry (no override): %d     recommended entry: %d" % (before, after))
    if goal.get("target") is not None and sprite.dominant is not None:
        i = sprite.dominant
        outs = _simulate(sprite, row)
        d_after = [_circ(color_hue256(*outs[s][i]) - goal["target"]) for s in range(64) if color_hue256(*outs[s][i]) >= 0]
        d_before = _circ(sprite.base_hue[i] - goal["target"])
        add("    Main color's average distance from the target: before %.0f degrees, after %.0f degrees."
            % (d_before * 360 / 256, (sum(d_after) / len(d_after) * 360 / 256) if d_after else 0))
    if goal.get("word_neutral"):
        sat, val = _tone_change(sprite, row)
        add("    Averaged over all 64 variations: saturation %+.0f%%, brightness %+.0f%% versus the original." % (sat, val))
    add("")
    for ln in _dominant_table(sprite, row):
        add(ln)
    add("")
    if sprite.shiny:
        m = bal["metrics"]
        add("    Shiny check: %s" % ("some variations land close to the shiny's colors; compare the red-framed cell."
                                    if m["shiny"] > 0.05 else "no variation lands close to the shiny's changed colors."))
    add("")
    add("5. HOW TO CHECK IT")
    add("")
    add("    1) Paste the line from section 1 into sColorVariationOverrides[] and save.")
    add("    2) Run:  python tech/generate_color_variations.py %s" % name)
    add("    3) Open tech/color_variation_previews/%s_front_variations.png" % name)
    add("       (the preview in section 1 shows the same thing before you paste anything).")
    add("    4) Adjust with section 6, re-run step 2, and only rebuild the ROM when you like it.")
    add("")
    add("6. HOW TO ADJUST  (position numbers are the field order in the entry; see the cheat sheet)")
    add("")
    for symptom, change in _adjust_rows(sprite, row, goal):
        add("    %-52s -> %s" % (symptom, change))
    add("")
    add("    Rules of thumb:")
    add("    - Change one field at a time, then re-run the preview.")
    add("    - maxAngle/angleBias/bandShift/bandWidth are in sine-table units (256 = 360 degrees).")
    add("    - Fades, lighten/darken/gray and tint strengths are in 1/64ths (64 = all the way).")
    add("    - Fields left off the end of an entry are 0; to set a later field you must write every")
    add("      field before it. If a species looks wildly off, count the values.")
    add("")
    add("7. NOTES")
    add("")
    add("    - These numbers were found by trying candidates through the preview math, not by eye:")
    add("      the final judge is the sheet. If it looks wrong, trust the sheet and use section 6.")
    add("    - Evolutions share colors but not entries: run this helper for each stage you want to change.")
    add("    - The helper never edits src/pokemon_color_variation.c; you paste the line yourself.")
    out_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


# ---- entry point -------------------------------------------------------------------
def run_suggestion(species_token: str, kind: str, words: List[str]) -> Path:
    try:
        name = resolve_species_tokens([species_token])[0]
    except ValueError as e:
        raise ValueError(str(e)) from None
    bad = [w for w in words if w not in HUE_WORDS and w not in NEUTRAL_WORDS and w != "shiny"]
    if bad:
        raise ValueError("unknown color '%s'. Try: %s, %s" % (
            bad[0], ", ".join(sorted(HUE_WORDS, key=lambda k: HUE_WORDS[k][0])), ", ".join(sorted(NEUTRAL_WORDS))))
    sprite = _Sprite(name)
    goal: Dict = dict(kind=kind, words=words)
    tone = [NEUTRAL_WORDS[w] for w in words if w in NEUTRAL_WORDS]
    if kind == "toward" and words[0] == "brown":
        tone = ["brown"]
    if kind == "toward" and len(words) > 1:
        raise ValueError("'toward' takes one color (e.g. dragonite=blue); use <> for several colors to avoid")
    goal["notes"] = []
    auto = words == ["shiny"]
    if "shiny" in words and not auto:
        raise ValueError("'shiny' can't be combined with other colors; use name? (or name<>shiny) on its own")
    if auto and kind != "away":
        raise ValueError("'shiny' only works as 'away from': use name? or name<>shiny")
    if auto:
        zones = _shiny_zones(sprite)
        goal.update(word="shiny", hue=True, zones=zones, words=["the shiny's colors"])
        if not sprite.shiny:
            goal["notes"].append("No shiny.pal was found for this species, so there was no shiny to steer away from; "
                                 "the entry just aims for good variety.")
        elif not zones:
            goal["notes"].append("This species' shiny doesn't move any color's hue by more than about 8 degrees, "
                                 "so there is no hue to steer away from; the entry just aims for good variety.")
        else:
            goal["notes"].append("The shiny's changed colors are treated as the zones to avoid: %s. Variations that "
                                 "stay clear of those hues keep the shiny special." % ", ".join(
                                     "%s (hue %d +- %d)" % (hue_name(z), z, w) for z, w in zones))
    if tone:
        goal.update(word=tone[0], word_neutral=True)
        results = _tone_rows(sprite, goal)
        neutral_share = 1 - sprite.chroma_share
        if kind == "away" and tone[0] in ("white", "black"):
            goal["notes"].append("The default entry never lightens or darkens colored pixels on its own, so it already "
                                 "stays away from %s; all three lines are the default. Avoid adding whiteFade, "
                                 "bandLighten, tintLighten (for white) or bandDarken, tintDarken (for black)." % tone[0])
        if kind == "toward" and tone[0] in ("gray", "white", "black") and neutral_share > 0.5:
            goal["notes"].append("About %.0f%% of this sprite is already gray/white/black, so a tone change will be subtle."
                                 % (neutral_share * 100))
        if kind == "toward" and tone[0] == "gray" and sprite.chroma_share < 0.05:
            goal["notes"].append("This sprite has almost no color, so there is nothing to gray out.")
        goal.setdefault("hue", None)
        if kind == "away" and len(words) > 1:
            goal["words"] = words
    else:
        w0 = words[0]
        if not auto:
            goal.update(word=w0, hue=True)
            if kind == "away":
                goal["zones"] = [HUE_WORDS[w] for w in words]
            else:
                goal["target"] = HUE_WORDS[w0][0]
        if kind == "away" and sprite.chroma_share < 0.05:
            default = _new_row()
            results = {n: dict(row=default, metrics=_measure(sprite, default, goal))
                       for n in ("gentle", "balanced", "bold")}
            goal["notes"].append("This sprite is almost colorless, so it has no hue to move away from %s; the default "
                                 "entry is already safe. To give it a color, use = (for example name=blue) to add a tint."
                                 % ", ".join(goal["words"]))
        else:
            print("  %s: trying candidate entries (a few seconds)..." % name)
            results = _search(sprite, goal)
    SUGGEST_DIR.mkdir(parents=True, exist_ok=True)
    tag = "%s_auto" % name if auto else "%s_%s_%s" % (name, kind, "-".join(words))
    preview = SUGGEST_DIR / (tag + "_preview.png")
    previous = OVERRIDES.get(name)
    OVERRIDES[name] = results["balanced"]["row"]
    try:
        sheet = make_sheet(name, sprite.front, sprite.dir / "normal.pal", sprite.dir / "shiny.pal")
        sheet.save(preview)
    except Exception:  # noqa: BLE001
        preview = None
    finally:
        if previous is None:
            OVERRIDES.pop(name, None)
        else:
            OVERRIDES[name] = previous
    out = SUGGEST_DIR / (tag + ".txt")
    _write_report(sprite, goal, results, out, preview)
    return out


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("species", nargs="*",
                        help="Pokemon folder names under graphics/pokemon/ or National "
                             "Pokédex numbers/ranges (e.g. pikachu 172 25-26), or color "
                             "suggestions like \"vaporeon<>green\" (away from) and "
                             "\"dragonite=blue\" (toward), or just \"vaporeon?\" to steer "
                             "away from the shiny's colors. If omitted, you'll be prompted "
                             "interactively.")
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

    try:
        plain_tokens, directives = split_directives(args.species)
        names: List[str] = resolve_species_tokens(plain_tokens)
    except ValueError as e:
        print(f"  ERROR — {e}")
        return 1
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
    if not names and not directives:
        try:
            line = input("Enter Pokémon names or Pokédex numbers (space-separated, blank = all overrides;\n"
                         "or name<>color / name=color for a suggestion): ").strip()
        except EOFError:
            line = ""
        try:
            plain_tokens, directives = split_directives(line.split())
            names = resolve_species_tokens(plain_tokens) or ([] if directives else list(OVERRIDES))
        except ValueError as e:
            print(f"  ERROR — {e}")
            return 1
    if not names and not directives:
        parser.print_help()
        return 1

    failures = 0
    for sp, kind, words in directives:
        try:
            out = run_suggestion(sp, kind, words)
            print(f"  wrote {out.relative_to(REPO_ROOT)}")
        except Exception as e:  # noqa: BLE001
            failures += 1
            print(f"  {sp}: ERROR — {e}")
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
