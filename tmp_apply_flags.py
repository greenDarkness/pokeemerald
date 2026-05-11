import re

# Read flag names from diff output (ignore the offsets, just the names)
names = []
with open("tmp_flag_diff_output.txt", "r") as f:
    for line in f:
        m = re.match(r'^\s*#define\s+(FLAG_\w+)', line)
        if m:
            names.append(m.group(1))

print(f"Flag names read: {len(names)}")

# Build replacement block for offsets 0x309..0x577
# - Skip 0x329 (FLAG_HIDE_NEW_BARK_BOAT stays there)
# - Assign flag names sequentially; fill remainder with FLAG_MOD_UNUSED_*
block_lines = []
name_idx = 0
for off in range(0x309, 0x578):
    hex_off = f"0x{off:03X}"
    if off == 0x329:
        block_lines.append(f"#define FLAG_HIDE_NEW_BARK_BOAT                           (MOD_FLAGS_START + 0x329)")
    elif name_idx < len(names):
        name = names[name_idx]
        block_lines.append(f"#define {name:<45} (MOD_FLAGS_START + {hex_off})")
        name_idx += 1
    else:
        label = f"FLAG_MOD_UNUSED_{hex_off}"
        block_lines.append(f"#define {label:<45} (MOD_FLAGS_START + {hex_off})")

print(f"Names used: {name_idx}/{len(names)}  Remaining unused slots: {0x577 - 0x309 + 1 - 1 - name_idx}")

# Read flags.h
flags_path = "include/constants/flags.h"
with open(flags_path, "r", encoding="utf-8") as f:
    content = f.read()

# Replace from FLAG_MOD_UNUSED_0x309 through FLAG_MOD_UNUSED_0x577 (the whole unused block)
pattern = r'#define FLAG_MOD_UNUSED_0x309\s+\(MOD_FLAGS_START \+ 0x309\).*?#define FLAG_MOD_UNUSED_0x577\s+\(MOD_FLAGS_START \+ 0x577\)'
new_block = "\n".join(block_lines)
new_content, count = re.subn(pattern, new_block, content, flags=re.DOTALL)

if count == 0:
    print("ERROR: Pattern not found in flags.h")
    exit(1)

with open(flags_path, "w", encoding="utf-8", newline="\n") as f:
    f.write(new_content)

print(f"Done. flags.h updated ({count} replacement).")
