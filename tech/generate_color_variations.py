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
    python generate_color_variations.py "omanyte:teal=blue"    # move only the teal part toward blue
    python generate_color_variations.py "omanyte:teal=blue<>purple"   # ...but never let it reach purple
    python generate_color_variations.py "meganium=green<>red:crimson=magenta"
                                       # rest of the sprite toward green, never red; crimson part toward magenta

Outputs: tech/color_variation_previews/<dex#>_<name>_front_variations.png
         (e.g. 025_pikachu_front_variations.png: an 8×8 grid showing all 64
         personality buckets, plus the shiny in the corner for comparison).

Suggestions ("name<>color" / "name=color"):
    Writes tech/color_variation_suggestions/<dex#>_<name>_<away|toward>_<color>.txt with a
    ready-to-paste override entry, a gentler and a bolder alternative, what to expect,
    and how to adjust it, plus a preview sheet with the entry already applied.
    Colors (around the color wheel): red scarlet orange brown amber gold golden yellow lime
    chartreuse green emerald teal turquoise aqua cyan sky azure blue indigo violet purple
    magenta fuchsia pink rose crimson. Tones: gray grey white black dark deep vivid rich
    (dark/deep = deeper and richer, vivid/rich = more saturated; both ramp from the original
    at step -4 to the full effect at step +3).
    "<>" = away from, "=" = toward, "~" is a shell-safe alias for "<>".
    Several colors to avoid: "vaporeon<>green,yellow". In a command prompt, put "<>" in
    quotes (or use ~), otherwise cmd treats it as redirection.
    No direction at all: "vaporeon?" (same as "vaporeon<>shiny") treats the colors the shiny
    changes as the zones to avoid and aims for good variety everywhere else.
    Mostly neutral sprites (cream, tan, brown, gray, white, black, like Persian or Geodude)
    get tone variations instead of hue rotation for "?" and "<>" goals: white / gray / brown
    fades and darkening, leaving out the tone the shiny moves toward.
    Choosing the color that moves: "omanyte:teal=blue" shifts only the teal part of Omanyte
    toward blue and leaves the yellow shell alone ("omanyte:yellow=blue" does the opposite).
    Without a name, "toward" moves the color already nearest the target.
    Toward one color while staying out of others: "omanyte:teal=blue<>purple" (either order;
    several colors with commas, "...<>purple,pink"; "<>shiny" avoids the shiny's colors too).
    The entry is chosen so that none of the 64 variations of the moving color land in the
    avoided zone; fields like bandMaxUp (a cap on upward drift) are used when they help.
    Broad shift and one color at once: "meganium=green<>red:crimson=magenta". The part before
    ':' steers the broad hue shift (the rest of the sprite), the part after it steers only the
    named color. Either part can use =, <> or both ("meganium<>red:crimson=magenta",
    "meganium?:crimson=magenta"). Without a ':' part, <> covers the whole sprite.

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


CONFIG_WARNINGS: List[str] = []
_C_INT_TYPES = {"u8": (8, False), "s8": (8, True), "u16": (16, False), "s16": (16, True),
                "u32": (32, False), "s32": (32, True)}


def _wrap_c_int(value: int, ctype: Optional[str]) -> int:
    """What the C compiler stores when a value doesn't fit the field's type (e.g. -4 in a u8 is 252)."""
    spec = _C_INT_TYPES.get(ctype or "")
    if not spec:
        return value
    bits, signed = spec
    v = value & ((1 << bits) - 1)
    if signed and v >= 1 << (bits - 1):
        v -= 1 << bits
    return v


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
    field_types = {n: t for t, n in re.findall(r"\b(\w+)\s+(\w+)\s*;", struct_m.group(1))}

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
                raw = _eval_c_expr(expr, defines)
                row[name] = _wrap_c_int(raw, field_types.get(name))
                if row[name] != raw:
                    CONFIG_WARNINGS.append("%s: %s = %d doesn't fit in %s; the game stores %d, so the preview now uses %d too"
                                           % (species or "?", name, raw, field_types.get(name), row[name], row[name]))
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
        if lighten > 0:
            r, g, b = palette5[i]
            mx = max(r, g, b)
            palette5[i] = tuple(c + _c_div((31 - c) * lighten * mx, TINT_LIGHT_ONE * 31) for c in (r, g, b))
        elif lighten < 0:
            # negative bandLighten: straight blend toward black
            lighten = max(lighten, -TINT_LIGHT_ONE)
            palette5[i] = tuple(c + _c_div(c * lighten, TINT_LIGHT_ONE) for c in palette5[i])
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

    forced_cap = override is not None and override.get("bandWidth") and override.get("bandMaxUp", 0) < 0
    if (angle_mag == 0 and mode == 0 and angle_bias == 0 and not any(fades) and abs(sat_mul - 1.0) <= 1e-6
            and not forced_cap):
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
    has_hue = (angle_index != 0) or bool(forced_cap)
    if override is not None and override.get("holdBias"):
        step_angle = 0 if angle_mag == 0 else (angle_mag if signed_step >= 0 else -angle_mag)
        cool_index = (angle_bias - step_angle) % 256
    else:
        cool_index = (256 - angle_index) % 256
    if has_split and cool_index != 0:
        has_hue = True

    mat_pos = hue_matrix(angle_index) if has_hue else None
    mat_neg = hue_matrix(cool_index) if (has_hue and has_split) else None

    for i in range(1, 16):
        r, g, b = out[i]
        sat = is_saturated(r, g, b)
        bucket = hue_bucket(r, g, b)

        if has_hue:
            if has_split and sat:
                if bucket == 0:
                    r, g, b = rotate_color_band_capped(r, g, b, angle_index, mat_pos, override)
                else:
                    r, g, b = rotate_color_band_capped(r, g, b, cool_index, mat_neg, override)
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
    stem = file_stem(name)

    for side, enabled in (("front", True), ("back", do_back)):
        sprite = species_dir / f"{side}.png"
        if not enabled or not sprite.exists():
            continue
        sheet = make_sheet(name, sprite, pal, shiny_pal)
        out = out_dir / f"{stem}_{side}_variations.png"
        sheet.save(out)
        written.append(out)
        legacy = out_dir / f"{name}_{side}_variations.png"
        if legacy != out and legacy.exists():
            legacy.unlink()

    return written


_DEX_PREFIX = re.compile(r"^\d{3,}_")


def existing_preview_names(out_dir: Path) -> List[Tuple[str, bool]]:
    """Species that already have a sheet in out_dir, and whether a back sheet exists.
    Accepts both "025_pikachu_..." and the older unnumbered "pikachu_..." names."""
    if not out_dir.is_dir():
        return []

    def species_of(p: Path, suffix: str) -> str:
        return _DEX_PREFIX.sub("", p.name[:-len(suffix)], count=1)

    fronts = {species_of(p, "_front_variations.png") for p in out_dir.glob("*_front_variations.png")}
    backs = {species_of(p, "_back_variations.png") for p in out_dir.glob("*_back_variations.png")}
    return [(name, name in backs) for name in sorted(fronts | backs)]


POKEDEX_HEADER = REPO_ROOT / "include" / "constants" / "pokedex.h"
_DEX_NUMBERS: Optional[Dict[str, int]] = None


def file_stem(name: str) -> str:
    """Output file name prefix: the National Pokédex number leads ("025_pikachu") so
    previews sort in dex order. Folders outside the dex keep their plain name."""
    global _DEX_NUMBERS
    if _DEX_NUMBERS is None:
        try:
            _DEX_NUMBERS = {n: i for i, n in enumerate(load_national_dex()) if i > 0}
        except (OSError, ValueError):
            _DEX_NUMBERS = {}
    num = _DEX_NUMBERS.get(name)
    return f"{num:03d}_{name}" if num else name


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
NEUTRAL_WORDS = {"gray": "gray", "grey": "gray", "white": "white", "black": "black",
                 "dark": "dark", "deep": "dark", "vivid": "vivid", "rich": "vivid"}

_HUE_NAMES = [(0, "red"), (10, "red-orange"), (21, "orange"), (32, "amber"), (43, "yellow"),
              (58, "yellow-green"), (70, "lime"), (85, "green"), (105, "spring green"),
              (118, "teal"), (128, "cyan"), (148, "azure"), (171, "blue"), (186, "indigo"),
              (200, "violet"), (213, "magenta"), (232, "pink"), (246, "crimson")]

_DEFAULT_FIELDS = ["species", "maxAngle", "angleBias", "satCap", "satMul", "tintHue", "tintSpread",
                   "tintStrength", "tintDarken", "tintLighten", "grayFade", "whiteFade", "brownFade",
                   "bandHue", "bandWidth", "bandShift", "bandLighten", "bandDarken", "bandGray",
                   "bandMaxUp", "holdBias"]
_PRESETS = {
    "away":   {"gentle": dict(vw=0.6, d0=6, vcap=10, dev=1.0), "balanced": dict(vw=1.0, d0=14, vcap=10, dev=1.0),
               "bold": dict(vw=1.6, d0=24, vcap=14, dev=0.5)},
    "toward": {"gentle": dict(vw=0.6, pull=0.60, d0=6, vcap=8, dev=1.0),
               "balanced": dict(vw=0.8, pull=0.90, d0=14, vcap=10, dev=1.0),
               "bold": dict(vw=2.6, pull=0.80, d0=26, vcap=22, dev=0.4)},
}
_COMPLEXITY_FIELDS = ["angleBias", "tintHue", "tintSpread", "tintStrength", "tintDarken", "tintLighten",
                      "grayFade", "whiteFade", "brownFade", "bandShift", "bandLighten", "bandDarken",
                      "bandGray", "bandMaxUp", "holdBias"]


def hue_name(h: float) -> str:
    h = h % 256
    best = min(_HUE_NAMES, key=lambda t: min(abs(h - t[0]), 256 - abs(h - t[0])))
    return best[1]


_LAB_CACHE: Dict[Tuple[int, int, int], Tuple[float, float]] = {}


def _lab5(c: Tuple[int, int, int]) -> Tuple[float, float]:
    """(lightness L, chroma C) of a 5-bit color in CIE Lab: how light and how colorful it LOOKS.
    Hue rotation keeps luma but not perceived lightness (yellow -> blue gets darker and denser)."""
    hit = _LAB_CACHE.get(c)
    if hit is not None:
        return hit

    def lin(v: int) -> float:
        u = ((v << 3) | (v >> 2)) / 255.0
        return u / 12.92 if u <= 0.04045 else ((u + 0.055) / 1.055) ** 2.4

    r, g, b = lin(c[0]), lin(c[1]), lin(c[2])
    x = (0.4124 * r + 0.3576 * g + 0.1805 * b) / 0.95047
    y = 0.2126 * r + 0.7152 * g + 0.0722 * b
    z = (0.0193 * r + 0.1192 * g + 0.9505 * b) / 1.08883

    def f(t: float) -> float:
        return t ** (1 / 3) if t > 0.008856 else 7.787 * t + 16 / 116

    out = (116 * f(y) - 16, math.hypot(500 * (f(x) - f(y)), 200 * (f(y) - f(z))))
    _LAB_CACHE[c] = out
    return out


def _circ(d: float) -> float:
    """Absolute circular distance for a hue difference, in sine-table units."""
    d = abs(d) % 256
    return d if d <= 128 else 256 - d


def _signed(d: float) -> float:
    d = (d + 128) % 256 - 128
    return d


def _color_list(text: str) -> List[str]:
    return [c.strip() for c in re.split(r"[,+/]", text.lower()) if c.strip()]


_GOAL_PART_RE = re.compile(r"(<>|~|=)([^<>=~]*)")
_NAME_PART_RE = re.compile(r"^([^<>=~?:\s]+)(\?)?((?:(?:<>|~|=)[^<>=~:]*)*)$")
_BAND_PART_RE = re.compile(r"^([a-z]+)((?:(?:<>|~|=)[^<>=~:]*)*)$")


def _parse_goals(text: str, token: str) -> Tuple[List[str], List[str]]:
    """'=green<>red,orange' -> (['green'], ['red', 'orange'])."""
    toward: List[str] = []
    avoid: List[str] = []
    for op, cols in _GOAL_PART_RE.findall(text):
        colors = _color_list(cols)
        if not colors:
            raise ValueError("'%s': a %s needs a color after it" % (token, "=" if op == "=" else "<>"))
        if op == "=":
            if toward:
                raise ValueError("'%s': use only one = (toward) per part, e.g. meganium=green<>red:crimson=magenta" % token)
            toward = colors
        else:
            avoid += colors
    if len(toward) > 1:
        raise ValueError("'%s': = (toward) takes one color; use <> for several colors to avoid" % token)
    return toward, avoid


def _as_goal(toward: List[str], avoid: List[str]) -> Tuple[str, List[str], List[str]]:
    """(kind, words, avoid) in the form run_suggestion() expects."""
    if toward:
        return "toward", toward, avoid
    return "away", avoid, []


def parse_directive(token: str) -> Optional[Tuple[str, str, List[str], List[str], Optional[Dict[str, List[str]]]]]:
    """Returns (species[:source], kind, colors, avoid, broad) or None for an ordinary species token.

    The part before ':' steers the broad hue shift (maxAngle / angleBias, i.e. the rest of the sprite);
    the part after ':' names one color and steers just that color (the hue band):
      'vaporeon<>green'                    -> ('vaporeon', 'away', ['green'], [], None)
      'dragonite=blue'                     -> ('dragonite', 'toward', ['blue'], [], None)
      'omanyte:teal=blue<>purple'          -> ('omanyte:teal', 'toward', ['blue'], ['purple'], None)
      'meganium=green<>red:crimson=magenta'-> ('meganium:crimson', 'toward', ['magenta'], [],
                                               {'toward': ['green'], 'avoid': ['red']})
    'name?' is 'name<>shiny'; '~' is a shell-safe alias for '<>'."""
    t = re.sub(r"\s*(<>|~|=|:|\?)\s*", r"\1", token.strip())
    if not any(ch in t for ch in "<>=~?:"):
        return None
    name_part, colon, band_part = t.partition(":")
    m = _NAME_PART_RE.match(name_part)
    if not m:
        raise ValueError("can't read '%s': write it as name<>color, name=color, name?, name:color=target, "
                         "or name=color<>color:color=target" % token)
    name, question, broad_text = m.group(1), m.group(2), m.group(3)
    b_toward, b_avoid = _parse_goals(broad_text, token)
    if question:
        b_avoid = b_avoid + ["shiny"]
    if not colon:
        if not (b_toward or b_avoid):
            raise ValueError("can't read '%s': add a goal, e.g. %s<>green, %s=blue or %s?" % (token, name, name, name))
        kind, words, avoid = _as_goal(b_toward, b_avoid)
        return name, kind, words, avoid, None
    bm = _BAND_PART_RE.match(band_part.lower())
    if not bm:
        raise ValueError("can't read '%s' after the ':'; write the color to move and its goal, e.g. crimson=magenta"
                         % band_part)
    source = bm.group(1)
    s_toward, s_avoid = _parse_goals(bm.group(2), token)
    if not (s_toward or s_avoid):
        raise ValueError("'%s': give the %s part a goal, e.g. %s:%s=magenta or %s:%s<>red"
                         % (token, source, name, source, name, source))
    kind, words, avoid = _as_goal(s_toward, s_avoid)
    broad = dict(toward=b_toward, avoid=b_avoid) if (b_toward or b_avoid) else None
    return "%s:%s" % (name, source), kind, words, avoid, broad


def split_directives(tokens: List[str]) -> Tuple[List[str], List[Tuple[str, str, List[str], List[str],
                                                                        Optional[Dict[str, List[str]]]]]]:
    """Separates suggestion directives ('name<>color', 'name=color', 'name?', 'name:src=color<>avoid',
    'name=color<>avoid:src=color') from ordinary species tokens. Spaces around the signs are fine
    ('vaporeon <> green', 'meganium = green : crimson = magenta')."""
    text = re.sub(r"\s*(<>|~|=|:)\s*", r"\1", " ".join(tokens))
    text = re.sub(r"\s+\?", "?", text)
    remaining, directives = [], []
    for tok in text.split():
        d = parse_directive(tok)
        if d is None:
            remaining.append(tok)
        else:
            directives.append(d)
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
              "tintSpread", "bandLighten", "bandGray"):
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
        self.lab0 = {i: _lab5(self.pal[i]) for i in self.idxs}
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
        items = sorted((self.base_hue[i], self.weights[i], i) for i in self.idxs if self.base_hue[i] >= 0)
        if not items:
            return []
        groups: List[List[Tuple[int, float, int]]] = [[items[0]]]
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
            cx = sum(math.cos(h / 256 * 2 * math.pi) * x for h, x, _ in g)
            cy = sum(math.sin(h / 256 * 2 * math.pi) * x for h, x, _ in g)
            center = (math.atan2(cy, cx) / (2 * math.pi) * 256) % 256
            offs = [_signed(h - center) for h, _, _ in g]
            out.append(dict(weight=w, center=int(round(center)) % 256, lo=min(offs), hi=max(offs),
                            members=[i for _, _, i in g]))
        out.sort(key=lambda c: -c["weight"])
        return out


def _choose_core(sprite: _Sprite, kind: str, target: Optional[int], source: Optional[str]):
    """Decides which colors of the sprite the entry should move.
    Returns (core: {palette index: bool} | None, cluster | None). With a named source color ("omanyte:teal=blue")
    that color moves. For a plain "toward" goal the color already nearest the target moves (so teal becomes
    blue while a far-away yellow shell stays put), unless it's tiny next to the sprite's main color, in which
    case the main color moves. For "away" goals everything stays eligible."""
    clusters = [c for c in sprite.clusters if c["weight"] >= 0.04]
    if not clusters:
        return None, None
    if source:
        t = HUE_WORDS[source][0]
        best = min(clusters, key=lambda c: _circ(c["center"] - t))
        if _circ(best["center"] - t) > 36:
            have = ", ".join("%s (%.0f%%)" % (hue_name(c["center"]), c["weight"] * 100) for c in clusters)
            raise ValueError("this sprite has no %s to move; its colors are: %s" % (source, have))
        cl = best
    elif kind == "toward" and target is not None:
        pool = [c for c in clusters if c["weight"] >= 0.06] or clusters
        nearest = min(pool, key=lambda c: _circ(c["center"] - target))
        cl = nearest if nearest["weight"] >= 0.4 * clusters[0]["weight"] else clusters[0]
    else:
        return None, None
    members = set(cl["members"])
    return {i: (i in members) for i in sprite.idxs}, cl


def _band_for(cluster: Dict[str, float]) -> Tuple[int, int]:
    half = max(abs(cluster["lo"]), abs(cluster["hi"]))
    return int(cluster["center"]), int(max(8, min(16, math.ceil(half) + 3)))


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
    core = goal.get("core")                      # {palette index: True/False} or None (everything counts)
    zone = near = gain = close = shiny_clash = drift = chroma_dev = light_dev = 0.0
    chroma_over = 0.0
    cell_c = [0.0] * 64                          # per variation: weighted colorfulness ratio of the colored pixels
    cell_w = 0.0
    hue_spread = light_var = sat_var = 0.0
    maxdrift = drift_nc = maxdrift_nc = 0.0
    core_w = sum(sprite.weights[i] for i in sprite.idxs if core and core.get(i)) or 1.0
    nc_w = sum(sprite.weights[i] for i in sprite.idxs if core and not core.get(i)
               and sprite.base_hue[i] >= 0) or 1.0
    spread_w = 0.0
    all_w = sum(sprite.weights[i] for i in sprite.idxs) or 1.0
    shiny_idx = []
    if sprite.shiny:
        for i in sprite.idxs:
            sh = color_hue256(*sprite.shiny[i])
            if sh >= 0 and (sprite.base_hue[i] < 0 or _circ(sh - sprite.base_hue[i]) > 12):
                shiny_idx.append((i, sh))
    shiny_w = sum(sprite.weights[i] for i, _ in shiny_idx)
    shiny_hue = {i: sh for i, sh in shiny_idx if sprite.base_hue[i] >= 0}
    nc_shiny = bzone = close_nc = 0.0
    bzones = goal.get("broad_zones", [])
    btarget = goal.get("broad_target")
    for i in sprite.idxs:
        w = sprite.weights[i]
        hues, vals, chromas = [], [], []
        h0 = sprite.base_hue[i]
        c0 = max(sprite.pal[i]) - min(sprite.pal[i])
        in_core = core is None or bool(core.get(i))
        for s in range(64):
            r, g, b = outs[s][i]
            h = color_hue256(r, g, b)
            vals.append(max(r, g, b) * 8 / 255.0)
            chromas.append(max(r, g, b) - min(r, g, b))
            if h >= 0:
                hues.append(h)
                if zones and in_core:
                    pz = max((max(0.0, 1 - _circ(h - T) / W) for T, W in zones), default=0.0)
                    zone += w * pz / 64
                    # safety margin: how close to the edge of a zone (including inside it)
                    edge = min(_circ(h - T) - W for T, W in zones)
                    near += w * max(0.0, 1 - max(0.0, edge) / 10.0) / 64
                    if h0 >= 0:
                        gain += w * (min(_circ(h - T) for T, W in zones)
                                     - min(_circ(h0 - T) for T, W in zones)) / 64
                if not in_core and h0 >= 0 and bzones and not any(_circ(h0 - T) < W for T, W in bzones):
                    # the broad goal's "never reach X" holds for the colors that aren't the moving one
                    bzone += w * max((max(0.0, 1 - _circ(h - T) / W) for T, W in bzones), default=0.0) / 64
                if not in_core and h0 >= 0 and btarget is not None:
                    close_nc += w * max(0.0, 1 - _circ(h - btarget) / 64) / 64
                if not in_core and h0 >= 0 and i in shiny_hue and btarget is None:
                    # a color that isn't meant to move shouldn't wander toward what the shiny makes of it
                    d0 = _circ(h0 - shiny_hue[i])
                    nc_shiny += w * max(0.0, d0 - _circ(h - shiny_hue[i]) - 4) / d0 / 64
                if target is not None:
                    cw = (1.0 if in_core else 0.0) if core is not None else sprite.core[i]
                    close += w * cw * max(0.0, 1 - _circ(h - target) / 64) / 64
                if h0 >= 0:
                    dd = _circ(h - h0)
                    drift += w * dd / 64
                    if w >= 0.03:
                        maxdrift = max(maxdrift, dd)
                    if not in_core:
                        drift_nc += w * dd / 64
                        if w >= 0.03:
                            maxdrift_nc = max(maxdrift_nc, dd)
        light_var += w * _std(vals)
        sat_var += w * _std(chromas)
        L0, C0 = sprite.lab0[i]
        if h0 >= 0 and C0 >= 12:
            cell_w += w
            for s in range(64):
                Ls, Cs = _lab5(outs[s][i])
                ratio = Cs / C0
                # Oversaturation reads as garish much sooner than a little dullness reads as washed out,
                # so the two sides are tracked (and weighted) separately.
                chroma_dev += w * max(0.0, 1.0 - ratio - 0.15) / 64
                chroma_over += w * max(0.0, ratio - 1.0 - 0.10) / 64
                cell_c[s] += w * min(ratio, 3.0)
                light_dev += w * max(0.0, abs(Ls - L0) - 8.0) / 30.0 / 64
        elif h0 >= 0 and not (row.get("tintStrength") and c0 < TINT_MAX_CHROMA):
            # Muted colors (grayish purples, dusty browns): a ratio would explode, so allow a fixed
            # amount of added colorfulness instead. Tint targets are meant to gain color, so skip them.
            for s in range(64):
                Cs = _lab5(outs[s][i])[1]
                chroma_over += w * max(0.0, Cs - C0 - 6.0) / 12.0 / 64
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
    hot = (sum(1 for c in cell_c if c / cell_w > 1.3) / 64) if cell_w else 0.0
    return dict(zone=zone / norm, near=near / norm, gain=gain / norm,
                close=close / (core_w if core is not None else norm),
                cdev=chroma_dev / norm, cover=chroma_over / norm, hot=hot, ldev=light_dev / norm,
                drift=drift / norm, maxdrift=maxdrift,
                drift_nc=drift_nc / nc_w, maxdrift_nc=maxdrift_nc, nc_shiny=nc_shiny / nc_w,
                bzone=bzone / nc_w, close_nc=close_nc / nc_w,
                hue_spread=hue_spread / (spread_w or 1.0), light_var=light_var / all_w,
                sat_var=sat_var / all_w, shiny=(shiny_clash / shiny_w) if shiny_w else 0.0,
                complexity=complexity)


def _color_penalty(m: Dict[str, float], preset: Dict) -> float:
    """Perceived-color deviation. Dullness and lightness shifts are relaxed for the MORE VARIANCE
    preset, but oversaturation never is: it's what makes a variation look garish."""
    dev = preset.get("dev", 1.0)
    return dev * (4 * m["cdev"] + 3 * m["ldev"]) + max(1.0, dev) * (9 * m["cover"] + 3 * m["hot"])


def _score(m: Dict[str, float], goal: Dict, preset: Dict, ref: Dict) -> float:
    vcap = preset.get("vcap", 10)
    variety = (min(m["hue_spread"], vcap) / vcap + 0.25 * min(m["light_var"], 0.12) / 0.12
               + 0.15 * min(m["sat_var"], 2.5) / 2.5)
    cx = 0.10 * m["complexity"]
    # broad goal (the part before ':'): the colors that aren't the moving one
    broad = -40 * m.get("bzone", 0.0)
    steered = goal.get("broad_target") is not None
    if steered:
        bspan = ref.get("ncmax", 0.0) - ref.get("nc0", 0.0)
        brel = max(0.0, min(1.2, (m["close_nc"] - ref["nc0"]) / bspan)) if bspan > 1e-6 else 0.0
        broad -= 7 * abs(brel - preset.get("pull", 0.9))
    keep = 0.0 if steered else 1.0               # a broad target asks the rest of the sprite to drift
    if goal["kind"] == "away":
        return (-40 * m["zone"] - 8 * m["near"] + 0.10 * max(-10.0, min(m["gain"], 6.0))
                - 0.3 * max(0.0, -m["gain"]) - _color_penalty(m, preset)
                + preset["vw"] * variety - keep * 0.12 * max(0, m["drift"] - preset["d0"])
                - keep * 0.08 * max(0, m["maxdrift"] - (preset["d0"] + 14)) - cx - 0.8 * m["shiny"] + broad)
    span = ref["cmax"] - ref["c0"]
    rel = max(0.0, min(1.2, (m["close"] - ref["c0"]) / span)) if span > 1e-6 else 0.0
    return (-7 * abs(rel - preset["pull"]) + preset["vw"] * variety * 0.5
            - 40 * m["zone"] - 8 * m["near"]
            - _color_penalty(m, preset)
            - keep * 0.12 * max(0, m["drift_nc"] - preset["d0"])
            - keep * 0.08 * max(0, m["maxdrift_nc"] - (preset["d0"] + 14)) - cx - 1.0 * m["shiny"]
            - 8 * m.get("nc_shiny", 0.0) + broad)


# ---- candidate generation -----------------------------------------------------
def _rows_rotation(goal: Dict) -> List[Dict[str, int]]:
    angles = [4, 6, 8, 10, 14, 18, 24, 32]
    biases = list(range(-126, 127, 6)) if goal["kind"] == "toward" else list(range(-60, 61, 4))
    # holdBias keeps the bias pointing one way in split modes, so a "toward" lean doesn't reverse for 3/8 of
    # the variations
    holds = (0, 1) if goal["kind"] == "toward" else (0,)
    return [_new_row(maxAngle=a, angleBias=b, holdBias=h) for a in angles for b in biases for h in holds
            if not (h and b == 0)]


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
    chosen = goal.get("core_cluster")
    for cl in ([chosen] if chosen else sprite.clusters[:2]):
        if cl["weight"] < 0.04:
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
        caps = [0, 2, 4, 6, 8] if goal.get("zones") else [0, 4]
        if goal["kind"] == "toward" and sign < 0:
            # a negative cap pushes the band colors down on every step, so even step -4 (where the band
            # shift is zero) leaves the original color, without rotating the rest of the sprite with it
            caps += [-8, -16, -24]
        for shift in shifts:
            for a in (0, 6, 12, 18, 24):
                for b in (-6, 0, 6):
                    for cap in caps:
                        if cap and a == 0:
                            continue
                        rows.append(_new_row(maxAngle=a, angleBias=b, bandHue=bh, bandWidth=bw,
                                             bandShift=shift, bandMaxUp=cap))
    return rows


def _nc_center(sprite: _Sprite, core: Optional[Dict[int, bool]]) -> Optional[float]:
    """Weighted average hue of the colored pixels outside the moving color."""
    cx = cy = 0.0
    for i in sprite.idxs:
        h = sprite.base_hue[i]
        if h < 0 or (core and core.get(i)):
            continue
        cx += sprite.weights[i] * math.cos(h / 256 * 2 * math.pi)
        cy += sprite.weights[i] * math.sin(h / 256 * 2 * math.pi)
    if abs(cx) + abs(cy) < 1e-9:
        return None
    return (math.atan2(cy, cx) / (2 * math.pi) * 256) % 256


def _rows_combined(sprite: _Sprite, goal: Dict) -> List[Dict[str, int]]:
    """Broad goal + band goal ('meganium=green<>red:crimson=magenta'): angleBias carries the rest of the sprite
    toward the broad target, the band takes the moving color to its own target, and bandMaxUp keeps the
    moving color from being dragged along by the bias."""
    cl = goal.get("core_cluster")
    if cl is None:
        return []
    bh, bw = _band_for(cl)
    btarget = goal.get("broad_target")
    center = _nc_center(sprite, goal.get("core"))
    if btarget is not None and center is not None:
        need_nc = _signed(btarget - center)
        bsign = 1 if need_nc >= 0 else -1
        top = min(126, int(abs(need_nc) * 1.15))
        biases = sorted({0} | {bsign * v for v in range(6, max(7, top) + 1, 6)} | {int(round(need_nc))})
    else:
        biases = [-12, -6, 0, 6, 12]
    rows = []
    for b in biases:
        if goal["kind"] == "toward":
            needs = {int(round(_signed(goal["target"] - cl["center"]))),
                     int(round(_signed(goal["target"] - cl["center"] - b)))}
            shifts = {0}
            for need in needs:
                sign = 1 if need >= 0 else -1
                top = min(126, int(abs(need) * 1.15))
                shifts |= {sign * v for v in range(8, max(9, top) + 1, 8)} | {max(-126, min(126, need))}
        else:
            shifts = set(range(-60, 61, 15))
        for shift in sorted(shifts):
            for a in (0, 6, 12, 18):
                for cap in (0, 4, -8, -16, -24, -32):
                    if cap and a == 0 and b == 0:
                        continue
                    for hold in ((0, 1) if b else (0,)):
                        rows.append(_new_row(maxAngle=a, angleBias=b, bandHue=bh, bandWidth=bw,
                                             bandShift=shift, bandMaxUp=cap, holdBias=hold))
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


def _compensations(row: Dict[str, int]) -> List[Dict[str, int]]:
    """Variants of a candidate that win back lightness / colorfulness a hue change shifts, in either
    direction (negative bandLighten darkens, negative bandGray saturates), and that calm the vivid modes."""
    out = []
    for sat in (940, 880, 820, 760):
        v = dict(row)
        v["satMul"] = sat
        out.append(v)
    # satCap limits only the vivid modes (2, 4, 5, 7), the usual source of the most garish cells
    for cap in (FP_SCALE, 1126):
        for sat in (FP_SCALE, 940):
            v = dict(row)
            v.update(satCap=cap, satMul=sat)
            out.append(v)
    if row["bandWidth"]:
        for lighten in (-16, 0, 16, 28):
            for gray in (-24, 0, 24):
                for sat in (FP_SCALE, 880):
                    if (lighten, gray, sat) == (0, 0, FP_SCALE):
                        continue
                    v = dict(row)
                    v.update(bandLighten=lighten, bandGray=gray, satMul=sat)
                    out.append(v)
    return out


def _cmax(evaluated) -> float:
    feasible = [m["close"] for _, m in evaluated if m.get("maxdrift_nc", 0) <= 40]
    return max(feasible or [m["close"] for _, m in evaluated])


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
    ref = {"c0": 0.0, "cmax": _cmax(evaluated), "nc0": 0.0, "ncmax": 0.0}
    if toward or goal.get("broad_target") is not None:
        still = _measure(sprite, _new_row(maxAngle=0), goal)
        ref["c0"] = still["close"] if toward else 0.0
        ref["nc0"] = still["close_nc"]
    ref["ncmax"] = max(m["close_nc"] for _, m in evaluated)
    presets = _PRESETS[goal["kind"]]
    balanced = max(evaluated, key=lambda rm: _score(rm[1], goal, presets["balanced"], ref))
    # a "toward" goal always compares rotation against a hue band; an "away" goal only reaches
    # for fades / bands / tints when plain rotation can't keep clear of the zone
    needs_more = True   # richer families only win when they score clearly better (complexity is penalized)
    if needs_more:
        if not toward:
            evaluated += [(r, _measure(sprite, r, goal)) for r in _rows_fades()]
        evaluated += [(r, _measure(sprite, r, goal)) for r in _rows_band(sprite, goal)]
        if goal.get("combined"):
            evaluated += [(r, _measure(sprite, r, goal)) for r in _rows_combined(sprite, goal)]
        if not toward:
            evaluated += [(r, _measure(sprite, r, goal)) for r in _rows_band_fade(sprite)]
        if neutral and toward:
            evaluated += [(r, _measure(sprite, r, goal)) for r in _rows_tint()]
        ref["cmax"] = _cmax(evaluated)
        ref["ncmax"] = max(m["close_nc"] for _, m in evaluated)
        ranked = sorted(evaluated, key=lambda rm: -_score(rm[1], goal, presets["balanced"], ref))
        seen, extra = set(), []
        for r, _ in ranked:
            key = tuple(sorted(r.items()))
            if r["tintStrength"] and key not in seen and len(seen) < 10:
                seen.add(key)
                extra += [(v, _measure(sprite, v, goal)) for v in _style_variants(r)]
        evaluated += extra
    # second pass: take the best few and try to repair perceived lightness / colorfulness
    ranked = sorted(evaluated, key=lambda rm: -_score(rm[1], goal, presets["balanced"], ref))
    seen = set()
    for r, _ in ranked[:40]:
        key = tuple(sorted(r.items()))
        if key in seen or len(seen) >= 12:
            continue
        seen.add(key)
        evaluated += [(v, _measure(sprite, v, goal)) for v in _compensations(r)]
    result = _pick(evaluated, goal, ref)
    result["_ref"] = ref
    return result


# ---- mostly-neutral sprites (cream, tan, brown, gray, white, black) -----------------
# Hue rotation turns earth tones into greens and oranges, so these sprites vary in tone instead: white /
# gray / brown fades and darkening, with whichever tone the shiny moves toward left out.
# A sprite this neutral always gets tone variations for "?" / "<>" goals ...
NEUTRAL_SPRITE_SHARE = 0.8
# ... and one this neutral (counting richer tans/ochres as earth tones too) gets them when the hue search
# would have to swing its main color this far (sine-table units, ~22 degrees) to escape the shiny.
NEUTRAL_SPRITE_LOOSE_SHARE = 0.7
NEUTRAL_LOOSE_TAN_CHROMA = 64
NEUTRAL_MAX_HUE_DRIFT = 16
_TONES = ("white", "gray", "brown", "black")
_LABF_CACHE: Dict[Tuple[int, int, int], Tuple[float, float, float]] = {}


def _lab5_full(c: Tuple[int, int, int]) -> Tuple[float, float, float]:
    """CIE Lab (L, a, b) of a 5-bit color."""
    hit = _LABF_CACHE.get(c)
    if hit is not None:
        return hit

    def lin(v: int) -> float:
        u = ((v << 3) | (v >> 2)) / 255.0
        return u / 12.92 if u <= 0.04045 else ((u + 0.055) / 1.055) ** 2.4

    def f(t: float) -> float:
        return t ** (1 / 3) if t > 0.008856 else 7.787 * t + 16 / 116

    r, g, b = lin(c[0]), lin(c[1]), lin(c[2])
    fx = f((0.4124 * r + 0.3576 * g + 0.1805 * b) / 0.95047)
    fy = f(0.2126 * r + 0.7152 * g + 0.0722 * b)
    fz = f((0.0193 * r + 0.1192 * g + 0.9505 * b) / 1.08883)
    out = (116 * fy - 16, 500 * (fx - fy), 200 * (fy - fz))
    _LABF_CACHE[c] = out
    return out


def _delta_e(p: Tuple[float, float, float], q: Tuple[float, float, float]) -> float:
    return math.sqrt((p[0] - q[0]) ** 2 + (p[1] - q[1]) ** 2 + (p[2] - q[2]) ** 2)


def _is_neutral_color(c: Tuple[int, int, int], tan_chroma: float = 56) -> bool:
    """Gray/white/black, or an earth tone: cream, beige, tan or brown. Judged in Lab: tans and creams sit at
    hue angles of about 65-110 degrees; below that only dark colors count (brown), since light ones read as
    pink or peach. tan_chroma is how colorful a tan may be before it counts as yellow / orange."""
    L, a, b = _lab5_full(c)
    chroma = math.hypot(a, b)
    if chroma <= 12:
        return True
    angle = math.degrees(math.atan2(b, a)) % 360
    if 65 <= angle <= 110:
        return chroma <= tan_chroma
    return 45 <= angle < 65 and L <= 70 and chroma <= 55


def _main_hue_drift(sprite: "_Sprite", row: Dict[str, int]) -> Tuple[float, float]:
    """(average hue change of the sprite's main color cluster across the 64 variations, in sine-table units;
    the average hue it lands on)."""
    if not sprite.clusters:
        return 0.0, 0.0
    members = [i for i in sprite.clusters[0]["members"] if sprite.base_hue[i] >= 0]
    outs = _simulate(sprite, row)
    tot = wsum = cx = cy = 0.0
    for i in members:
        w = sprite.weights[i]
        for s in range(64):
            h = color_hue256(*outs[s][i])
            if h < 0:
                continue
            tot += w * _circ(h - sprite.base_hue[i])
            wsum += w
            cx += w * math.cos(h / 256 * 2 * math.pi)
            cy += w * math.sin(h / 256 * 2 * math.pi)
    if not wsum:
        return 0.0, 0.0
    return tot / wsum, (math.atan2(cy, cx) / (2 * math.pi) * 256) % 256


def sprite_neutral_share(sprite: "_Sprite", tan_chroma: float = 56) -> float:
    """Share of the sprite's pixels (black outlines aside) that are neutral or earth-toned."""
    tot = n = 0.0
    for i in sprite.idxs:
        if max(sprite.pal[i]) <= 4:
            continue
        tot += sprite.weights[i]
        if _is_neutral_color(sprite.pal[i], tan_chroma):
            n += sprite.weights[i]
    return n / tot if tot else 1.0


def _tone_target(c: Tuple[int, int, int], tone: str, amount: int = TINT_LIGHT_ONE) -> Tuple[int, int, int]:
    """Where the engine takes a color for each tone (same math as the game); amount in 1/64ths."""
    if tone == "white":
        return apply_side_fades(*c, (0, amount, 0))
    if tone == "gray":
        return apply_side_fades(*c, (amount, 0, 0))
    if tone == "brown":
        return apply_side_fades(*c, (0, 0, amount))
    return tuple(x - _c_div(x * amount, TINT_LIGHT_ONE) for x in c)


def _shiny_changes(sprite: "_Sprite") -> List[Tuple[int, float]]:
    """(palette index, weight) of the colors the shiny visibly changes."""
    if not sprite.shiny:
        return []
    return [(i, sprite.weights[i]) for i in sprite.idxs
            if sprite.weights[i] >= 0.005 and _delta_e(_lab5_full(sprite.pal[i]), _lab5_full(sprite.shiny[i])) >= 5]


# A tone is left out when this small a dose (1/64ths) already moves the colors this far toward the shiny.
# A subtle shiny (Persian's paler cream) blocks the tone it leans toward; a strong one (Eevee's silver) only
# limits how far that tone may go, which the clash penalty in the scoring takes care of.
SHINY_TEST_DOSE = 12
SHINY_TEST_LIMIT = 0.25


def shiny_tone_closeness(sprite: "_Sprite") -> Dict[str, float]:
    """For each tone: how far (0..1) a small dose of it moves the shiny-changed colors toward the shiny."""
    changes = _shiny_changes(sprite)
    cw = sum(w for _, w in changes)
    out = {}
    for t in _TONES:
        tot = 0.0
        for i, w in changes:
            shiny = _lab5_full(sprite.shiny[i])
            d0 = _delta_e(_lab5_full(sprite.pal[i]), shiny)
            d = _delta_e(_lab5_full(_tone_target(sprite.pal[i], t, SHINY_TEST_DOSE)), shiny)
            tot += w * max(0.0, 1 - d / d0)
        out[t] = tot / cw if cw else 0.0
    return out


def shiny_blocked_tones(sprite: "_Sprite") -> List[str]:
    return [t for t, c in shiny_tone_closeness(sprite).items() if c >= SHINY_TEST_LIMIT]


def _tone_row(band: Dict[str, int], gray_mass: bool, sides: Dict[int, Tuple[str, int]], dark: Tuple[int, int],
              max_angle: int, sat_cap: int) -> Optional[Dict[str, int]]:
    """sides: {+1 / -1: (fade tone, amount)}; dark: (side, amount) for a light darkening at that side's
    extreme steps, on top of whatever fade that side has (side 0 = none)."""
    kw: Dict[str, int] = dict(maxAngle=max_angle, satCap=sat_cap)
    for side, (tone, mag) in sides.items():
        kw[{"white": "whiteFade", "gray": "grayFade", "brown": "brownFade"}[tone]] = side * mag
    side, mag = dark
    if side:
        if not band and not (gray_mass and side < 0):
            return None
        if band:
            kw.update(band, bandDarken=side * mag)
        if gray_mass and side < 0:
            kw["tintDarken"] = mag
    return _new_row(**kw)


# darkening per preset (1/64ths); kept light, since a little already reads clearly darker
_NEUTRAL_DARK = {"gentle": 4, "balanced": 6, "bold": 10}
_NEUTRAL_FADES = (12, 20, 32, 48)


def _rows_neutral(sprite: "_Sprite", tones: List[str]) -> List[Dict[str, int]]:
    cl = sprite.clusters[0] if sprite.clusters and sprite.clusters[0]["weight"] >= 0.06 else None
    band = dict(zip(("bandHue", "bandWidth"), _band_for(cl))) if cl else {}
    gray_mass = sum(sprite.weights[i] for i in sprite.idxs
                    if sprite.base_hue[i] < 0 and 4 < max(sprite.pal[i]) < 28) >= 0.15
    fades = [None] + [t for t in tones if t != "black"]
    darks = [(0, 0)] + ([(s, m) for s in (1, -1) for m in sorted(set(_NEUTRAL_DARK.values()))]
                        if "black" in tones else [])
    rows = []
    seen = set()
    for pos in fades:
        for neg in fades:
            if pos is not None and pos == neg:
                continue
            for mp in (_NEUTRAL_FADES if pos else (0,)):
                for mn in (_NEUTRAL_FADES if neg else (0,)):
                    sides = {}
                    if pos:
                        sides[1] = (pos, mp)
                    if neg:
                        sides[-1] = (neg, mn)
                    for dark in darks:
                        if not sides and not dark[0]:
                            continue
                        for a in (0, 4):
                            for cap in (FP_SCALE, 1126):
                                row = _tone_row(band, gray_mass, sides, dark, a, cap)
                                if row is None:
                                    continue
                                key = tuple(sorted(row.items()))
                                if key not in seen:
                                    seen.add(key)
                                    rows.append(row)
    return rows


def _measure_neutral(sprite: "_Sprite", row: Dict[str, int], changes: List[Tuple[int, float]]) -> Dict[str, float]:
    outs = _simulate(sprite, row)
    tw = sum(sprite.weights[i] for i in sprite.idxs) or 1.0
    amount = std_l = std_c = cover = olive = 0.0
    cell_c = [0.0] * 64
    cell_d = [0.0] * 64
    cell_w = 0.0
    for i in sprite.idxs:
        w = sprite.weights[i]
        base = _lab5_full(sprite.pal[i])
        labs = [_lab5_full(outs[s][i]) for s in range(64)]
        des = [_delta_e(base, x) for x in labs]
        amount += w * sum(des) / 64
        for s in range(64):
            cell_d[s] += w * des[s]
        if not _is_olive(base):
            olive += w * sum(1 for x in labs if _is_olive(x)) / 64
        std_l += w * _std([x[0] for x in labs])
        cs = [math.hypot(x[1], x[2]) for x in labs]
        std_c += w * _std(cs)
        c0 = math.hypot(base[1], base[2])
        if c0 >= 12:
            cell_w += w
            for s in range(64):
                cover += w * max(0.0, cs[s] / c0 - 1.10) / 64
                cell_c[s] += w * min(cs[s] / c0, 3.0)
    hot = (sum(1 for c in cell_c if c / cell_w > 1.3) / 64) if cell_w else 0.0
    # How far each variation moves toward the shiny's colors: 0 = no closer than the original, 1 = identical.
    clash_mean = clash_frac = 0.0
    cw = sum(w for _, w in changes)
    if cw:
        for s in range(64):
            closeness = 0.0
            for i, w in changes:
                d0 = _delta_e(_lab5_full(sprite.pal[i]), _lab5_full(sprite.shiny[i]))
                d = _delta_e(_lab5_full(outs[s][i]), _lab5_full(sprite.shiny[i]))
                closeness += w * max(0.0, 1 - d / d0)
            closeness /= cw
            clash_mean += closeness / 64
            if closeness > 0.4:
                clash_frac += 1 / 64
    complexity = sum(1 for f in _COMPLEXITY_FIELDS if row.get(f, 0) != 0)
    complexity += (row.get("satMul", FP_SCALE) != FP_SCALE) + (row.get("satCap", FP_SCALE * 2) != FP_SCALE * 2)
    tones = sum(1 for f in ("whiteFade", "grayFade", "brownFade", "bandDarken") if row.get(f, 0) != 0)
    tones += bool(row.get("tintDarken")) and not row.get("bandDarken")
    achromatic = sum(1 for f in ("whiteFade", "grayFade") if row.get(f, 0) != 0)
    achromatic += bool(row.get("bandDarken") or row.get("tintDarken"))
    return dict(amount=amount / tw, std_l=std_l / tw, std_c=std_c / tw, cover=cover / tw, hot=hot,
                clash=clash_mean, shiny=clash_frac, complexity=complexity, tones=tones, achromatic=achromatic, olive=olive / tw,
                coverage=sum(1 for d in cell_d if d / tw >= 3.0) / 64)


# gentle stays closest to the original; bold allows the widest tone spread
_NEUTRAL_PRESETS = {"gentle": dict(d0=5.0, cap=5.0), "balanced": dict(d0=8.0, cap=8.0), "bold": dict(d0=12.0, cap=12.0)}


def _score_neutral(m: Dict[str, float], preset: Dict[str, float]) -> float:
    cap = preset["cap"]
    variety = 0.5 * min(m["std_l"], cap) / cap + 0.5 * min(m["std_c"], cap) / cap
    # white / gray / black are preferred over brown whenever the shiny allows them; brown fills in when it doesn't
    return (2.0 * variety + 1.5 * m["coverage"] + 0.3 * (m["tones"] >= 2) + 0.2 * (m["tones"] >= 3)
            + 0.8 * m["achromatic"]
            - 0.25 * max(0.0, m["amount"] - preset["d0"])
            - 12 * m["shiny"] - 4 * m["clash"] - 9 * m["cover"] - 3 * m["hot"] - 0.1 * m["complexity"]
            - 15 * m["olive"])


def _is_olive(lab: Tuple[float, float, float]) -> bool:
    """A darkish yellow-to-green: what a cream or tan turns into when it's darkened without warming it.
    Low-chroma drabs (C < 22) read as gray-brown rather than olive, so they don't count."""
    chroma = math.hypot(lab[1], lab[2])
    return lab[0] < 62 and chroma >= 22 and 88 <= math.degrees(math.atan2(lab[2], lab[1])) % 360 <= 140


def _search_neutral(sprite: "_Sprite", goal: Dict) -> Dict[str, Dict]:
    avoid = shiny_blocked_tones(sprite)
    tones = [t for t in _TONES if t not in avoid]
    changes = _shiny_changes(sprite)
    evaluated = [(r, _measure_neutral(sprite, r, changes)) for r in _rows_neutral(sprite, tones)]
    if goal.get("zones"):
        # an explicit hue to avoid still counts (tone moves rarely reach one, but small rotations can)
        for r, m in evaluated:
            m["zone"] = _measure(sprite, r, goal)["zone"] if r["maxAngle"] else 0.0
    result = {}
    for name, preset in _NEUTRAL_PRESETS.items():
        dark = _NEUTRAL_DARK[name]
        pool = [rm for rm in evaluated if abs(rm[0]["bandDarken"]) in (0, dark) and rm[0]["tintDarken"] in (0, dark)]
        best = max(pool, key=lambda rm: (round(_score_neutral(rm[1], preset) - 40 * rm[1].get("zone", 0.0), 4),
                                         -rm[1]["complexity"]))
        result[name] = dict(row=best[0], metrics=best[1])
    result["_avoid_tone"] = avoid
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
            # negative bandLighten: a gradient from the original (step -4) to dark (step +3)
            specs = {"gentle": dict(bandLighten=-20, tintDarken=24 if neutral else 0),
                     "balanced": dict(bandLighten=-32, tintDarken=40 if neutral else 0),
                     "bold": dict(bandLighten=-48, tintDarken=52 if neutral else 0)}
        elif word == "dark":
            # deeper, not blacker: darken and saturate together so the color stays rich (Gengar's GSC look)
            specs = {"gentle": dict(bandLighten=-14, bandGray=-16, satCap=1126, tintDarken=16 if neutral else 0),
                     "balanced": dict(bandLighten=-24, bandGray=-32, satCap=1126, tintDarken=28 if neutral else 0),
                     "bold": dict(bandLighten=-32, bandGray=-48, satCap=1126, tintDarken=40 if neutral else 0)}
        elif word == "vivid":
            # the band ramps up to richer at step +3; satCap keeps the vivid modes from stacking on top
            specs = {"gentle": dict(bandGray=-16, satCap=1126), "balanced": dict(bandGray=-32, satCap=1126),
                     "bold": dict(bandGray=-48, satCap=FP_SCALE)}
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
            # lift the muted modes back up without letting the vivid modes stack (satCap FP_SCALE turns the
            # vivid boost off, so satMul is the most any variation gets)
            rows = {"gentle": _new_row(satMul=1080, satCap=FP_SCALE), "balanced": _new_row(satMul=1140, satCap=FP_SCALE),
                    "bold": _new_row(satMul=1140, satCap=1126)}
        elif word == "vivid":
            rows = {"gentle": _new_row(satCap=1126), "balanced": _new_row(satCap=FP_SCALE),
                    "bold": _new_row(satCap=FP_SCALE, satMul=940)}
        else:  # keep away from white / black / dark: the default entry never lightens or darkens colored pixels
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


def _focus_index(sprite: _Sprite, goal: Dict) -> Optional[int]:
    """The palette entry the report should describe: the heaviest color that is meant to move,
    otherwise the sprite's main color."""
    core = goal.get("core")
    if core and (goal.get("kind") == "toward" or goal.get("source")):
        members = [i for i, v in core.items() if v]
        if members:
            return max(members, key=lambda k: sprite.weights[k])
    return sprite.dominant


def _dominant_table(sprite: _Sprite, row: Dict[str, int], focus: Optional[int] = None) -> List[str]:
    i = focus if focus is not None else sprite.dominant
    if i is None:
        i = max(sprite.idxs, key=lambda k: sprite.weights[k], default=None)
        if i is None:
            return ["  (no pixels to analyze)"]
    outs = _simulate(sprite, row)
    now = ("%s, hue %d" % (hue_name(sprite.base_hue[i]), sprite.base_hue[i])) if sprite.base_hue[i] >= 0 \
        else "gray/white/black (a tint is needed to give it a hue)"
    label0 = "Color that moves" if (focus is not None and focus != sprite.dominant) else "Main color"
    lines = ["  %s (palette entry #%d, %.0f%% of the sprite, now %s):"
             % (label0, i, sprite.weights[i] * 100, now),
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


def _perceived_change(sprite: _Sprite, row: Dict[str, int]) -> Optional[Tuple[float, float]]:
    """(average lightness change in Lab points, average colorfulness change in %) for the sprite's
    colored pixels across the 64 variations. Returns None for sprites without colored pixels."""
    outs = _simulate(sprite, row)
    tl = tc = tw = 0.0
    for i in sprite.idxs:
        L0, C0 = sprite.lab0[i]
        if sprite.base_hue[i] < 0 or C0 < 12:
            continue
        ls = [_lab5(outs[s][i]) for s in range(64)]
        w = sprite.weights[i]
        tl += w * (sum(l for l, _ in ls) / 64 - L0)
        tc += w * (sum(c for _, c in ls) / 64 / C0 - 1.0)
        tw += w
    return (tl / tw, tc / tw * 100) if tw else None


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
    if row.get("holdBias"):
        out.append("holdBias 1: in the split modes (3, 6, 7) only the per-step rotation flips for cool colors; the "
                   "angleBias lean stays the same for every color, so no variation leans the other way.")
    if row["satCap"] != FP_SCALE * 2 or row["satMul"] != FP_SCALE:
        out.append("satCap %d / satMul %d: saturation control (satCap limits the vivid modes' boost%s; satMul %.0f%% is "
                   "applied to everything)." % (row["satCap"], row["satMul"],
                                                 ", FP_SCALE = no boost" if row["satCap"] <= FP_SCALE else "",
                                                 row["satMul"] * 100 / FP_SCALE))
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
            out.append("bandLighten %d: that color gets %s toward step +3."
                       % (row["bandLighten"], "paler" if row["bandLighten"] > 0 else "darker"))
        if row["bandDarken"]:
            out.append("bandDarken %+d: that color darkens at the %s end (step %s full)."
                       % (row["bandDarken"], "negative" if row["bandDarken"] < 0 else "positive",
                          "-4" if row["bandDarken"] < 0 else "+3"))
        if row["bandGray"]:
            out.append("bandGray %d: that color gets %s toward step +3."
                       % (row["bandGray"], "duller" if row["bandGray"] > 0 else "more saturated"))
        if row["bandMaxUp"]:
            out.append("bandMaxUp %d: that color can't be rotated more than %.0f degrees up the hue wheel, in any mode."
                       % (row["bandMaxUp"], deg(row["bandMaxUp"])))
    return out


def _adjust_rows(sprite: _Sprite, row: Dict[str, int], goal: Dict) -> List[Tuple[str, str]]:
    rows: List[Tuple[str, str]] = []
    focus = _focus_index(sprite, goal)
    base = sprite.base_hue[focus] if focus is not None else 0
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
    elif goal.get("hue") and not goal.get("neutral_sprite") and (goal["kind"] == "toward" or goal.get("zones")):
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
    if goal.get("broad_target") is not None:
        center = _nc_center(sprite, goal.get("core"))
        if center is not None:
            bt = goal["broad_target"]
            bword = hue_name(bt)
            lean = 1 if _signed(bt - center) >= 0 else -1
            cur = row["angleBias"]
            rows.append(("(rest of the sprite)", "a HIGHER angleBias turns it toward %s, a LOWER one toward %s"
                         % (hue_name(center + 24), hue_name(center - 24))))
            rows.append(("Rest not %s enough" % bword, "angleBias %+d -> %+d" % (cur, cur + 6 * lean)))
            rows.append(("Rest gone too far toward %s" % bword, "angleBias %+d -> %+d" % (cur, cur - 6 * lean)))
            if row["bandWidth"] and (row["angleBias"] > 0 or row["bandMaxUp"] < 0):
                rows.append(("The moving color follows the rest too much",
                             "make bandMaxUp (position 20) more negative: %d -> %d" % (row["bandMaxUp"], row["bandMaxUp"] - 8)))
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
            if v < 0 and f == "bandLighten":
                more, less = "darker", "less dark"
            elif v < 0 and f == "bandGray":
                more, less = "more saturated", "less saturated"
            rows.append(("%s: want it %s / %s" % (f, more, less),
                         "%s %+d -> %+d / %+d" % (f, v, v + 8 * sign, v - 8 * sign)))
    if goal.get("avoid_words"):
        names = ", ".join(goal["avoid_words"])
        sign = 1 if row["bandShift"] >= 0 else -1
        if row["bandMaxUp"]:
            rows.append(("Still drifting into %s" % names,
                         "lower bandMaxUp %d -> %d, or bandShift %+d -> %+d" % (
                             row["bandMaxUp"], max(1, row["bandMaxUp"] - 2), row["bandShift"], row["bandShift"] - 5 * sign)))
        else:
            rows.append(("Still drifting into %s" % names,
                         "lower the shift (bandShift %+d -> %+d / angleBias), or add a bandMaxUp (position 20) of 4 "
                         "to cap the upward drift" % (row["bandShift"], row["bandShift"] - 5 * sign)))
        rows.append(("Not %s enough now that %s is off limits" % (goal["word"], names),
                     "raise the shift a little (the cap and the zone limit how far it can go)"))
    elif row["bandMaxUp"]:
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
    if row["satCap"] > FP_SCALE:
        rows.append(("Variants look oversaturated / too vivid",
                     "set satCap (field 4) %s -> FP_SCALE to switch off the vivid-mode boost, then lower satMul "
                     "%d -> %d if still needed" % (_fmt_value("satCap", row["satCap"]), row["satMul"],
                                                    max(400, row["satMul"] - 80))))
    else:
        rows.append(("Variants look oversaturated / too vivid", "lower satMul %d -> %d (or %d); it scales every color's saturation"
                     % (row["satMul"], max(400, row["satMul"] - 80), max(400, row["satMul"] - 160))))
    rows.append(("Variants look washed out / too dull", "raise satMul %d -> %d (or %d)"
                 % (row["satMul"], row["satMul"] + 80, row["satMul"] + 160)))
    rows.append(("Want the same idea but milder / with more spread", "use the GENTLER or MORE VARIANCE line above, then fine tune"))
    rows.append(("Shiny comes too close", "compare the red-framed cell; nudge the field above that fixes the main problem"))
    return rows


def _broad_report(sprite: _Sprite, row: Dict[str, int], goal: Dict) -> List[str]:
    """Before/after lines for the broad goal: the colors outside the moving one."""
    core = goal.get("core") or {}
    rest = [i for i in sprite.idxs if not core.get(i) and sprite.base_hue[i] >= 0 and sprite.weights[i] >= 0.02]
    if not rest:
        return ["(no other colored pixels)"]
    lines = []
    outs = _simulate(sprite, row)
    rw = sum(sprite.weights[i] for i in rest)
    bt = goal.get("broad_target")
    if bt is not None:
        before = sum(sprite.weights[i] * _circ(sprite.base_hue[i] - bt) for i in rest) / rw
        after = sum(sprite.weights[i] * _circ(color_hue256(*outs[s][i]) - bt) for s in range(64) for i in rest
                    if color_hue256(*outs[s][i]) >= 0) / rw / 64
        lines.append("average distance from %s: before %.0f degrees, after %.0f degrees."
                     % (hue_name(bt), before * 360 / 256, after * 360 / 256))
    bz = goal.get("broad_zones") or []
    if bz:
        def hits(o):
            n = 0
            for s in range(64):
                w = sum(sprite.weights[i] for i in rest
                        if color_hue256(*o[s][i]) >= 0 and any(_circ(color_hue256(*o[s][i]) - T) < W for T, W in bz))
                n += w > 0.05
            return n
        lines.append("variations (out of 64) with a noticeable part inside the zone to avoid: default entry %d, "
                     "recommended entry %d." % (hits(_simulate(sprite, _new_row())), hits(outs)))
    return lines


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
    if goal.get("source"):
        add(" Moves   : only the %s part of the sprite" % goal["source"])
    if goal.get("avoid_words"):
        add(" Avoid   : never reach %s" % ", ".join(goal["avoid_words"]))
    if goal.get("broad_desc"):
        add(" Rest    : the rest of the sprite %s (broad hue shift)" % goal["broad_desc"])
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
    add("   Two alternatives built the same way (each tuned differently, so they aren't just scaled copies):")
    add("")
    add("   GENTLER (stays closer to the original):")
    add("    " + _format_entry(name, results["gentle"]["row"]))
    add("   MORE VARIANCE (a wider spread between individuals, a little less tidy):")
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
        add("    %4.0f%%  %4s  %-12s %-16s%s" % (sprite.weights[i] * 100, h if h >= 0 else "-",
                                               hue_name(h) if h >= 0 else "gray/white/black",
                                               ("%d (%s)" % (sh, hue_name(sh))) if sh >= 0 else "-",
                                               "  <- moves" if (goal.get("core") and goal["core"].get(i)
                                                                and (goal["kind"] == "toward" or goal.get("source"))) else ""))
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
    if goal.get("target") is not None and _focus_index(sprite, goal) is not None:
        i = _focus_index(sprite, goal)
        outs = _simulate(sprite, row)
        d_after = [_circ(color_hue256(*outs[s][i]) - goal["target"]) for s in range(64) if color_hue256(*outs[s][i]) >= 0]
        d_before = _circ(sprite.base_hue[i] - goal["target"])
        add("    The moving color's average distance from the target: before %.0f degrees, after %.0f degrees."
            % (d_before * 360 / 256, (sum(d_after) / len(d_after) * 360 / 256) if d_after else 0))
    if goal.get("combined"):
        add("    The rest of the sprite (broad goal):")
        for ln in _broad_report(sprite, row, goal):
            add("      " + ln)
    if goal.get("hue"):
        pc = _perceived_change(sprite, row)
        if pc is not None:
            add("    How the colors LOOK, averaged over all 64 variations (colored pixels only):")
            add("      colorfulness %+.0f%%, lightness %+.0f points (0 = same as the original; about +-12%% / +-8 points"
                " is hard to notice)." % (pc[1], pc[0]))
            if pc[1] > 15:
                add("      -> the variants will look more vivid than the original: set satCap (field 4) to FP_SCALE"
                    " and/or lower satMul (field 5) to calm them.")
            elif pc[1] < -15:
                add("      -> the variants will look duller than the original: raise satMul (field 5) a little.")
            if pc[0] < -10:
                add("      -> the variants will look darker (a hue change keeps the brightness number but not how light"
                    " the color looks; yellow turning blue is the classic case).")
            elif pc[0] > 10:
                add("      -> the variants will look lighter than the original.")
    if goal.get("word_neutral"):
        sat, val = _tone_change(sprite, row)
        add("    Averaged over all 64 variations: saturation %+.0f%%, brightness %+.0f%% versus the original." % (sat, val))
    add("")
    for ln in _dominant_table(sprite, row, _focus_index(sprite, goal)):
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
    add("    3) Open tech/color_variation_previews/%s_front_variations.png" % file_stem(name))
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
def run_suggestion(species_token: str, kind: str, words: List[str], avoid: Optional[List[str]] = None,
                   broad: Optional[Dict[str, List[str]]] = None) -> Path:
    avoid = avoid or []
    base_token, _, source = species_token.partition(":")
    source = source.strip().lower()
    if source and source not in HUE_WORDS:
        raise ValueError("unknown source color '%s' in '%s'. Try one of: %s" % (
            source, species_token, ", ".join(sorted(HUE_WORDS, key=lambda k: HUE_WORDS[k][0]))))
    try:
        name = resolve_species_tokens([base_token])[0]
    except ValueError as e:
        raise ValueError(str(e)) from None
    if broad:
        bad = [w for w in broad["toward"] + broad["avoid"] if w not in HUE_WORDS and w != "shiny"]
        if bad:
            raise ValueError("the part before ':' steers the broad hue shift, so it takes hues (or <>shiny), not "
                             "'%s'. Try: %s" % (bad[0], ", ".join(sorted(HUE_WORDS, key=lambda k: HUE_WORDS[k][0]))))
        if "shiny" in broad["toward"]:
            raise ValueError("'shiny' only works as 'away from' (<>shiny or name?)")
    bad = [w for w in list(words) + list(avoid) if w not in HUE_WORDS and w not in NEUTRAL_WORDS and w != "shiny"]
    if avoid and (kind != "toward" or any(w in NEUTRAL_WORDS for w in list(words) + list(avoid))
                  or (words and words[0] == "brown")):
        raise ValueError("combine a hue goal with colors to avoid like this: omanyte:teal=blue<>purple "
                         "(gray / white / black / brown can't be combined)")
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
    goal["source"] = source
    auto = words == ["shiny"]
    if source and (auto or any(w in NEUTRAL_WORDS for w in words) or (kind == "toward" and words[0] == "brown")):
        raise ValueError("name:color only works with a color goal, e.g. omanyte:teal=blue or omanyte:yellow<>green")
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
        if kind == "away" and tone[0] in ("white", "black", "dark"):
            goal["notes"].append("The default entry never lightens or darkens colored pixels on its own, so it already "
                                 "stays away from %s; all three lines are the default. Avoid adding whiteFade, "
                                 "positive bandLighten, tintLighten (for white) or bandDarken, negative bandLighten, "
                                 "tintDarken (for black / dark)." % tone[0])
        if kind == "away" and tone[0] == "vivid":
            goal["notes"].append("satCap limits the vivid modes (2, 4, 5, 7), which are where the most saturated "
                                 "variations come from; FP_SCALE turns that boost off entirely.")
        if kind == "toward" and tone[0] in ("black", "dark", "vivid"):
            goal["notes"].append("The change ramps across the hue steps: step -4 keeps the original and step +3 gets "
                                 "the full amount, so individuals spread from the original look to the %s one.%s"
                                 % (tone[0], " (To turn only a few individuals black, use bandDarken instead.)"
                                    if tone[0] != "vivid" else ""))
        if kind == "toward" and tone[0] in ("gray", "white", "black", "dark") and neutral_share > 0.5:
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
                if avoid:
                    zones = []
                    for w in avoid:
                        zones += _shiny_zones(sprite) if w == "shiny" else [HUE_WORDS[w]]
                    for z, zw in zones:
                        if _circ(goal["target"] - z) < zw:
                            raise ValueError("the target %s sits inside the zone to avoid (%s); pick a target "
                                             "outside it" % (w0, ", ".join(avoid)))
                    goal["zones"], goal["avoid_words"] = zones, list(avoid)
        if not auto:
            core, cluster = _choose_core(sprite, kind, goal.get("target"), source or None)
            goal["core"], goal["core_cluster"] = core, cluster
            if cluster is not None and (kind == "toward" or source):
                share = cluster["weight"] * 100
                what = "%s (hue %d, %.0f%% of the sprite)" % (hue_name(cluster["center"]), cluster["center"], share)
                others = [c for c in sprite.clusters if c is not cluster and c["weight"] >= 0.04]
                if kind == "toward":
                    goal["notes"].append(
                        "Only the %s %s toward %s; %s%s."
                        % (what, "moves" if source else "is the color that moves (the nearest to the target)", words[0],
                           "the rest of the sprite follows the broad goal" if broad
                           else "the sprite's other colors get just the normal variation",
                           (" (" + ", ".join("%s %.0f%%" % (hue_name(c["center"]), c["weight"] * 100) for c in others[:3]) + ")")
                           if others else ""))
                else:
                    goal["notes"].append("Only the %s is checked and moved; %s." % (
                        what, "the rest of the sprite follows the broad goal" if broad
                        else "the sprite's other colors stay put"))
                if not source and others:
                    pick = max(others, key=lambda c: c["weight"])
                    goal["notes"].append("To move a different color, name it: %s:%s=%s (for example)."
                                         % (name, hue_name(pick["center"]).split("-")[-1], words[0]) if kind == "toward"
                                         else "To check a different color, name it: %s:%s<>%s." % (
                                             name, hue_name(pick["center"]).split("-")[-1], words[0]))
        if not auto:
            if broad:
                bzones = []
                for w in broad["avoid"]:
                    bzones += _shiny_zones(sprite) if w == "shiny" else [HUE_WORDS[w]]
                goal["broad_zones"] = bzones
                goal["combined"] = True
                bdesc = []
                if broad["toward"]:
                    bt = HUE_WORDS[broad["toward"][0]][0]
                    for z, zw in bzones:
                        if _circ(bt - z) < zw:
                            raise ValueError("the broad target %s sits inside the broad zone to avoid (%s)"
                                             % (broad["toward"][0], ", ".join(broad["avoid"])))
                    goal["broad_target"] = bt
                    bdesc.append("toward %s" % broad["toward"][0])
                if broad["avoid"]:
                    bdesc.append("never %s" % ", ".join("the shiny's colors" if w == "shiny" else w
                                                         for w in broad["avoid"]))
                goal["broad_desc"] = " and ".join(bdesc)
                goal["notes"].append(
                    "The rest of the sprite follows the broad goal (%s) through maxAngle / angleBias; bandMaxUp "
                    "can hold the %s back so the broad shift doesn't drag it along." % (goal["broad_desc"],
                                                                                        source or "moving color"))
            else:
                # without a ':' part, "never X" covers the whole sprite; with one, it's only the band's
                goal["broad_zones"] = [] if source else list(goal.get("zones", []))
        if kind == "away" and sprite.chroma_share < 0.05:
            default = _new_row()
            results = {n: dict(row=default, metrics=_measure(sprite, default, goal))
                       for n in ("gentle", "balanced", "bold")}
            goal["notes"].append("This sprite is almost colorless, so it has no hue to move away from %s; the default "
                                 "entry is already safe. To give it a color, use = (for example name=blue) to add a tint."
                                 % ", ".join(goal["words"]))
        else:
            print("  %s: trying candidate entries (a few seconds)..." % name)
            share = sprite_neutral_share(sprite)
            use_tones = kind == "away" and not source and share >= NEUTRAL_SPRITE_SHARE
            results = None
            if kind == "away" and not source and not use_tones:
                loose = sprite_neutral_share(sprite, NEUTRAL_LOOSE_TAN_CHROMA)
                if loose >= NEUTRAL_SPRITE_LOOSE_SHARE:
                    results = _search(sprite, goal)
                    drift, landed = _main_hue_drift(sprite, results["balanced"]["row"])
                    if drift > NEUTRAL_MAX_HUE_DRIFT:
                        use_tones, share, results = True, loose, None
                        goal["notes"].append(
                            "Getting away from the shiny by hue would swing the main color about %.0f degrees "
                            "(to %s), so tones are used instead." % (drift * 360 / 256, hue_name(landed)))
            if use_tones:
                results = _search_neutral(sprite, goal)
                avoid_tone = results.pop("_avoid_tone")
                goal["neutral_sprite"] = True
                goal["notes"] = [n for n in goal["notes"] if not n.startswith("This species' shiny doesn't move")]
                goal["notes"].append(
                    "About %.0f%% of this sprite is neutral or earth-toned (cream, tan, brown, gray, white, black), so "
                    "the variations use tones (whiteFade, grayFade, brownFade, bandDarken / tintDarken) instead of "
                    "hue rotation, which would turn those colors green or orange." % (share * 100))
                if avoid_tone:
                    words = " and ".join(avoid_tone)
                    goal["notes"].append(
                        "Even a little %s already looks like the shiny, so %s %s left out of every line; "
                        "variations that drift toward the shiny's colors are rejected too."
                        % (words, words, "is" if len(avoid_tone) == 1 else "are"))
                elif sprite.shiny and _shiny_changes(sprite):
                    goal["notes"].append("A little of any tone stays clear of the shiny, so all four are allowed; "
                                         "each is kept mild enough that no variation drifts toward the shiny's colors.")
                goal["notes"].append("For hue variety anyway, ask with a hue instead (e.g. %s=blue)." % name)
            elif results is None:
                results = _search(sprite, goal)
    SUGGEST_DIR.mkdir(parents=True, exist_ok=True)
    stem = file_stem(name)
    tag = ("%s_auto" % stem if auto else "%s_%s_%s" % (stem, kind, "-".join(words)))
    if source:
        tag = "%s_%s_%s_%s" % (stem, source, kind, "-".join(words))
    if avoid:
        tag += "_avoid_" + "-".join(avoid)
    if broad:
        bparts = (["toward_" + broad["toward"][0]] if broad["toward"] else []) + \
                 (["avoid_" + "-".join(broad["avoid"])] if broad["avoid"] else [])
        tag = "%s_%s__%s" % (stem, "_".join(bparts), tag[len(stem) + 1:])
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


def _color_words_help() -> str:
    hues = " ".join(sorted(HUE_WORDS, key=lambda k: HUE_WORDS[k][0]))
    return ("Colors: %s. Tones: %s (dark/deep = deeper and richer, vivid/rich = more saturated). "
            "'shiny' also works as a color to avoid." % (hues, " ".join(NEUTRAL_WORDS)))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, epilog=_color_words_help(),
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("species", nargs="*",
                        help="Pokemon folder names under graphics/pokemon/ or National "
                             "Pokédex numbers/ranges (e.g. pikachu 172 25-26), or color "
                             "suggestions like \"vaporeon<>green\" (away from) and "
                             "\"dragonite=blue\" (toward), or just \"vaporeon?\" to steer "
                             "away from the shiny's colors, or \"omanyte:teal=blue<>purple\" to move only the "
                             "teal part toward blue without reaching purple, or "
                             "\"meganium=green<>red:crimson=magenta\" to steer the broad hue shift (before ':') and "
                             "one color (after ':') separately. " + _color_words_help() +
                             " If omitted, you'll be prompted interactively.")
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
    for w in CONFIG_WARNINGS:
        print(f"  WARNING — {w}")

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
                         "or name<>color / name=color for a suggestion).\n" + _color_words_help() + "\n> ").strip()
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
    for sp, kind, words, avoid, broad in directives:
        try:
            out = run_suggestion(sp, kind, words, avoid, broad)
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
