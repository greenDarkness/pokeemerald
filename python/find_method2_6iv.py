"""
Find all Method 2 PIDs that produce near-perfect IVs in Gen 3.

Method 2 LCRNG sequence (5 consecutive calls from initial seed s0):
  s1 = LCRNG(s0) -> PID_low  = s1 >> 16
  s2 = LCRNG(s1) -> PID_high = s2 >> 16
  s3 = LCRNG(s2) -> (vBlank skip)
  s4 = LCRNG(s3) -> IV1 = s4 >> 16  (HP, ATK, DEF from bits 0-14)
  s5 = LCRNG(s4) -> IV2 = s5 >> 16  (SPD, SPATK, SPDEF from bits 0-14)

Accepted IVs:
  HP:    30 or 31
  ATK:   0, 1, 30, or 31
  DEF:   30 or 31
  SPD:   30 or 31
  SPATK: 30 or 31
  SPDEF: 30 or 31
"""
from itertools import product

MULT = 0x41C64E6D
ADD  = 0x6073
RMULT = 0xEEB9EB65
RADD  = 0x0A3561A1
MASK = 0xFFFFFFFF

NATURES = [
    "Hardy",   "Lonely",  "Brave",   "Adamant", "Naughty",
    "Bold",    "Docile",  "Relaxed", "Impish",  "Lax",
    "Timid",   "Hasty",   "Serious", "Jolly",   "Naive",
    "Modest",  "Mild",    "Quiet",   "Bashful", "Rash",
    "Calm",    "Gentle",  "Sassy",   "Careful", "Quirky",
]

STAT_NAMES = ["HP", "ATK", "DEF", "SPD", "SPATK", "SPDEF"]

def lcrng(seed):
    return (seed * MULT + ADD) & MASK

def lcrng_prev(seed):
    return (seed * RMULT + RADD) & MASK

def main():
    # Build all valid IV1 patterns (HP bits 0-4, ATK bits 5-9, DEF bits 10-14)
    hp_vals  = [30, 31]
    atk_vals = [0, 1, 30, 31]
    def_vals = [30, 31]
    valid_iv1 = set()
    for hp, atk, df in product(hp_vals, atk_vals, def_vals):
        valid_iv1.add(hp | (atk << 5) | (df << 10))

    # Build all valid IV2 patterns (SPD bits 0-4, SPATK bits 5-9, SPDEF bits 10-14)
    spd_vals   = [30, 31]
    spatk_vals = [30, 31]
    spdef_vals = [30, 31]
    valid_iv2 = set()
    for spd, spa, spd2 in product(spd_vals, spatk_vals, spdef_vals):
        valid_iv2.add(spd | (spa << 5) | (spd2 << 10))

    print(f"IV1 patterns: {len(valid_iv1)}, IV2 patterns: {len(valid_iv2)}")

    results = []

    # For each valid IV1 pattern, search all possible s4 seeds
    for iv1_bits in valid_iv1:
        for bit15 in range(2):
            iv1_high = (bit15 << 15) | iv1_bits
            for low16 in range(0x10000):
                s4 = (iv1_high << 16) | low16
                s5 = lcrng(s4)
                iv2_bits = (s5 >> 16) & 0x7FFF
                if iv2_bits not in valid_iv2:
                    continue

                # Valid IV pair found. Work backwards to derive PID.
                s3 = lcrng_prev(s4)
                s2 = lcrng_prev(s3)
                s1 = lcrng_prev(s2)

                pid_high = s2 >> 16
                pid_low  = s1 >> 16
                pid = (pid_high << 16) | pid_low

                if pid == 0:
                    continue

                iv1_val = (s4 >> 16) & 0x7FFF
                iv2_val = (s5 >> 16) & 0x7FFF
                ivs = (
                    iv1_val & 0x1F,
                    (iv1_val >> 5) & 0x1F,
                    (iv1_val >> 10) & 0x1F,
                    iv2_val & 0x1F,
                    (iv2_val >> 5) & 0x1F,
                    (iv2_val >> 10) & 0x1F,
                )

                nature  = pid % 25
                ability = pid & 1
                gender_val = pid & 0xFF
                results.append((pid, nature, ability, gender_val, ivs))

    # Deduplicate (same s4 could appear from different iv1_bits iterations - shouldn't, but safe)
    results = list({r[0]: r for r in results}.values())
    results.sort()

    print(f"\nMethod 2 PIDs with near-perfect IVs (Gen 3 LCRNG)")
    print(f"Found {len(results)} result(s)\n")
    print(f"{'PID':>12}  {'Nature':<10}  Ab  {'HP':>2} {'AT':>2} {'DF':>2} {'SP':>2} {'SA':>2} {'SD':>2}")
    print("-" * 60)
    for pid, nature, ability, gv, ivs in results:
        print(f"0x{pid:08X}  {NATURES[nature]:<10}  {ability:>2}  {ivs[0]:>2} {ivs[1]:>2} {ivs[2]:>2} {ivs[3]:>2} {ivs[4]:>2} {ivs[5]:>2}")

    # Output as C array
    if results:
        print(f"\n// C array for pokeemerald ({len(results)} entries):")
        print(f"static const u32 sMethod2_NearPerfect_PIDs[] =\n{{")
        for i, (pid, nature, ability, gv, ivs) in enumerate(results):
            comma = "," if i < len(results) - 1 else ""
            iv_str = "/".join(str(v) for v in ivs)
            print(f"    0x{pid:08X}{comma} // {NATURES[nature]}, ab{ability}, {iv_str}")
        print("};")

    # Sort: egg-safe entries (all IVs 30-31) first, then entries with ATK 0/1
    def is_egg_safe(entry):
        return entry[4][1] >= 30  # ATK >= 30

    egg_safe = [r for r in results if is_egg_safe(r)]
    egg_unsafe = [r for r in results if not is_egg_safe(r)]
    sorted_results = egg_safe + egg_unsafe
    egg_count = len(egg_safe)

    print(f"\nEgg-safe entries (ATK 30-31): {egg_count}")
    print(f"NPC-only entries (ATK 0/1):   {len(egg_unsafe)}")

    # Write .h data file for pokeemerald
    import os
    out_path = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
                            "src", "data", "wish_egg_pid_iv_table.h")
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    with open(out_path, "w", newline="\n") as f:
        f.write("// Auto-generated by python/find_method2_6iv.py\n")
        f.write("// Method 2 PID/IV pairs with near-perfect IVs\n")
        f.write("// Sorted: egg-safe (all 30-31) first, then ATK 0/1 entries\n\n")
        f.write("struct WishEggPidIvs {\n")
        f.write("    u32 pid;\n")
        f.write("    u16 iv1; // HP(0-4), ATK(5-9), DEF(10-14)\n")
        f.write("    u16 iv2; // SPD(0-4), SPATK(5-9), SPDEF(10-14)\n")
        f.write("};\n\n")
        f.write(f"#define WISH_EGG_PID_IV_EGG_COUNT {egg_count}\n")
        f.write(f"#define WISH_EGG_PID_IV_TABLE_COUNT {len(sorted_results)}\n\n")
        f.write(f"const struct WishEggPidIvs sWishEggPidIvTable[] =\n{{\n")
        for i, (pid, nature, ability, gv, ivs) in enumerate(sorted_results):
            iv1 = ivs[0] | (ivs[1] << 5) | (ivs[2] << 10)
            iv2 = ivs[3] | (ivs[4] << 5) | (ivs[5] << 10)
            comma = "," if i < len(sorted_results) - 1 else ""
            iv_str = "/".join(str(v) for v in ivs)
            marker = "" if is_egg_safe((pid, nature, ability, gv, ivs)) else " [NPC]"
            f.write(f"    {{ 0x{pid:08X}, 0x{iv1:04X}, 0x{iv2:04X} }}{comma} // {NATURES[nature]}, {iv_str}{marker}\n")
        f.write("};\n")
    print(f"\nWrote {len(sorted_results)} entries to {out_path}")

if __name__ == "__main__":
    main()
