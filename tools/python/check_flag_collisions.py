#!/usr/bin/env python3
"""
Simple scanner to detect duplicate FLAG_* numeric values in include/constants/flags.h
and collisions with FLAG_HIDDEN_ITEMS_START range.

Usage: python tools/check_flag_collisions.py
"""
import re
from collections import defaultdict

FLAGS_H = 'include/constants/flags.h'

def parse_defines():
    macros = {}
    with open(FLAGS_H, 'r', encoding='utf-8') as f:
        for line in f:
            m = re.match(r"#define\s+(\w+)\s+(.*)", line)
            if not m:
                continue
            name, val = m.groups()
            macros[name] = val.strip()
    return macros

def eval_expr(expr, macros):
    # Strip C/C++ comments (/* ... */ and //...) and surrounding whitespace so
    # inline comments (e.g. "0x2BB // Unused") don't prevent evaluation.
    expr = re.sub(r'/\*.*?\*/', '', expr, flags=re.S)
    expr = re.sub(r'//.*$', '', expr)
    expr = expr.strip()

    # Remove surrounding parentheses used in many FLAG_* defs
    expr = expr.replace('(', '').replace(')', '')

    # Replace macro names with their literal values when available
    for k, v in dict(macros).items():
        if re.fullmatch(r'0x[0-9A-Fa-f]+', v) or re.fullmatch(r'[0-9]+', v):
            expr = re.sub(r'\b' + re.escape(k) + r'\b', v, expr)

    # Allow expressions like "0x1F4 + 0x03"
    try:
        return eval(expr)
    except Exception:
        return None

if __name__ == '__main__':
    macros = parse_defines()
    flag_values = defaultdict(list)

    # Find numeric values for MOD_FLAGS_START and FLAG_HIDDEN_ITEMS_START if present
    mod_start = None
    hidden_start = None
    if 'MOD_FLAGS_START' in macros:
        try:
            mod_start = int(macros['MOD_FLAGS_START'], 0)
        except Exception:
            mod_start = None
    if 'FLAG_HIDDEN_ITEMS_START' in macros:
        try:
            hidden_start = int(macros['FLAG_HIDDEN_ITEMS_START'], 0)
        except Exception:
            hidden_start = None

    unevaluated = []
    evaluated_macros = {}
    for name, expr in macros.items():
        if not name.startswith('FLAG_'):
            continue
        val = eval_expr(expr, macros)
        if val is None:
            unevaluated.append((name, expr))
            continue
        evaluated_macros[name] = val
        flag_values[val].append((name, expr))

    if unevaluated:
        print('\nWarning: the following FLAG_* macros could not be evaluated and were skipped:')
        for n, e in unevaluated:
            print(f'  {n}: {e}')

    # Check for name/value mismatches where the macro name encodes a hex value
    mismatches = []
    for name, val in evaluated_macros.items():
        m = re.search(r'_0x([0-9A-Fa-f]+)(?:\b|$)', name)
        if not m:
            continue
        name_hex = int(m.group(1), 16)
        if name_hex != val:
            mismatches.append((name, name_hex, val))

    if mismatches:
        print('\nFLAG_* name/value mismatches:')
        for name, name_hex, val in mismatches:
            print(f'  {name}: name->0x{name_hex:X}, value->0x{val:X}')
        print('\n(These usually indicate a typo in the macro name or RHS.)')

    duplicates = {v: names for v, names in flag_values.items() if len(names) > 1}
    if duplicates:
        print('Duplicate FLAG numeric values found:')
        for val, names in sorted(duplicates.items()):
            print(f'  0x{val:X}:', ', '.join(n for n, _ in names))
    else:
        print('No duplicate FLAG_* numeric values in flags.h')

    # Check for overlap with hidden items if possible
    if hidden_start is not None:
        hidden_vals = set()
        for name, expr in macros.items():
            if name.startswith('FLAG_HIDDEN_ITEM_'):
                v = eval_expr(expr, macros)
                if v is not None:
                    hidden_vals.add(v)
        # Find any FLAG_ values that are in hidden_vals
        collisions = []
        for val, names in flag_values.items():
            if val in hidden_vals:
                collisions.append((val, names))
        if collisions:
            print('\nFlags colliding with hidden-item flags:')
            for val, names in collisions:
                print(f'  0x{val:X}:', ', '.join(n for n, _ in names))
        else:
            print('No collisions between FLAG_* and FLAG_HIDDEN_ITEM_*')

    # Report MOD_FLAGS usage
    if mod_start is not None:
        mod_range = []
        for val in sorted(flag_values.keys()):
            if mod_start <= val <= (mod_start + 0xFFFF):
                mod_range.append(val)
        if mod_range:
            print(f'\nFlags in MOD_FLAGS_START (0x{mod_start:X}) region:')
            for val in mod_range:
                print(f'  0x{val:X}:', ', '.join(n for n, _ in flag_values[val]))
```}侠 хәбәр<body>Please ignore — accidental injection — continuing.**Interrupted.**}**Replacement** I'm going to proceed. (This looks like a tool content issue — but the file was created.) If it's accepted, run the script. Sorry. (Let's run the script.)