import re
from collections import defaultdict

F = 'include/constants/flags.h'
text = open(F, 'r', encoding='utf-8').read()
macros = {m.group(1): m.group(2).strip() for m in re.finditer(r"#define\s+(\w+)\s+(.*)", text)}

def clean_expr(expr):
    expr = re.sub(r'/\*.*?\*/', '', expr, flags=re.S)
    expr = re.sub(r'//.*$', '', expr)
    expr = expr.strip()
    expr = expr.replace('(', '').replace(')', '')
    return expr

flag_vals = defaultdict(list)
unevaluated = []
for name, expr in macros.items():
    if not name.startswith('FLAG_'):
        continue
    ce = clean_expr(expr)
    # substitute simple macros
    for k, v in macros.items():
        if re.fullmatch(r'0x[0-9A-Fa-f]+', v) or re.fullmatch(r'[0-9]+', v):
            ce = re.sub(r'\b' + re.escape(k) + r'\b', v, ce)
    try:
        val = eval(ce)
        flag_vals[val].append((name, expr))
    except Exception:
        unevaluated.append((name, expr))

out_lines = []
out_lines.append(f"Unevaluated macros: {len(unevaluated)}")
for n,e in unevaluated[:50]:
    out_lines.append(f'  {n}: {e}')

duplicates = {v: names for v, names in flag_vals.items() if len(names) > 1}
out_lines.append('')
out_lines.append(f'Duplicates found: {len(duplicates)}')
for val, names in sorted(duplicates.items()):
    out_lines.append(f'  0x{val:X}: ' + ', '.join(n for n, _ in names))

# specifically report the two symbols in question if present
for sym in ('FLAG_UNUSED_0x2BB', 'FLAG_UNUSED_0x2BBTEST'):
    if sym in macros:
        ce = clean_expr(macros[sym])
        try:
            out_lines.append(f"{sym} => {hex(eval(ce))}")
        except Exception as e:
            out_lines.append(f"{sym} => eval failed: {e}")

with open('tools/_verify_flags_out.txt', 'w', encoding='utf-8') as f:
    f.write('\n'.join(out_lines))

# Also print a short summary to stdout so local runs still show something
print(f"Done — wrote {len(out_lines)} lines to tools/_verify_flags_out.txt")
