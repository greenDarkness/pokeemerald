#!/usr/bin/env python3
"""For each Johto map (list in tmp_johto_maps.txt), copy script + flag fields
from pokesequel's map.json into pokeemerald's map.json.

Matches events by (x, y, graphics_id) to be order-independent. Warps matched by
(x, y, dest_map). Bg_events matched by (x, y).
"""
import json
import sys
from pathlib import Path

PE_ROOT = Path(r"D:\projects\pokeemerald")
PS_ROOT = Path(r"D:\projects\pokesequel")
LIST = PE_ROOT / "tmp_johto_maps.txt"

# Pre-collect known PE flag names so we don't reference undefined ones.
# Exclude johto_stubs.h since map_events.s does not include it.
def _collect_known_flags():
    flags = set(["0"])
    for hdr in (PE_ROOT / "include" / "constants").rglob("*.h"):
        if hdr.name == "johto_stubs.h":
            continue
        try:
            for line in hdr.read_text(encoding="utf-8", errors="ignore").splitlines():
                ls = line.strip()
                if ls.startswith("#define FLAG_"):
                    parts = ls.split()
                    if len(parts) >= 2:
                        flags.add(parts[1])
        except Exception:
            pass
    return flags

KNOWN_FLAGS = _collect_known_flags()

# Pre-collect all defined asm labels (look for `Label::` in .inc/.s files).
def _collect_known_labels():
    import re
    pat = re.compile(r"^([A-Za-z_][A-Za-z_0-9]*)::", re.MULTILINE)
    labels = set(["NULL"])
    for root in ("data", "asm"):
        for f in (PE_ROOT / root).rglob("*"):
            if not f.is_file() or f.suffix not in (".inc", ".s"):
                continue
            try:
                txt = f.read_text(encoding="utf-8", errors="ignore")
            except Exception:
                continue
            labels.update(pat.findall(txt))
    return labels

KNOWN_LABELS = _collect_known_labels()
print(f"[debug] known flags: {len(KNOWN_FLAGS)}, known labels: {len(KNOWN_LABELS)}")

def load(p):
    return json.loads(p.read_text(encoding="utf-8"))

def save(p, obj):
    # match prevailing 2-space indent + trailing newline
    p.write_text(json.dumps(obj, indent=2) + "\n", encoding="utf-8")

def merge_objects(pe_objs, ps_objs):
    """Match by (x, y, graphics_id). Update script + flag from ps."""
    if not pe_objs or not ps_objs:
        return 0
    ps_by_key = {}
    for o in ps_objs:
        key = (o.get("x"), o.get("y"), o.get("graphics_id"))
        ps_by_key.setdefault(key, []).append(o)
    changes = 0
    for o in pe_objs:
        key = (o.get("x"), o.get("y"), o.get("graphics_id"))
        cands = ps_by_key.get(key)
        if not cands:
            continue
        src = cands.pop(0)
        if "script" in src and src.get("script") not in (None, "NULL") and src.get("script") != o.get("script") and src.get("script") in KNOWN_LABELS:
            o["script"] = src["script"]; changes += 1
        # only copy flag if pokeemerald knows about it (avoid undefined refs)
        sf = src.get("flag")
        if sf and sf != o.get("flag") and sf in KNOWN_FLAGS:
            o["flag"] = sf; changes += 1
    return changes

def merge_warps(pe_warps, ps_warps):
    if not pe_warps or not ps_warps:
        return 0
    ps_by_key = {}
    for w in ps_warps:
        key = (w.get("x"), w.get("y"), w.get("dest_map"))
        ps_by_key.setdefault(key, []).append(w)
    changes = 0
    for w in pe_warps:
        key = (w.get("x"), w.get("y"), w.get("dest_map"))
        cands = ps_by_key.get(key)
        if not cands:
            continue
        src = cands.pop(0)
        # dest_warp_id may differ legitimately; only copy if pe has 0 and ps has non-zero
        if "dest_warp_id" in src and w.get("dest_warp_id") in (0, "0", None) and src.get("dest_warp_id") not in (0, "0", None):
            w["dest_warp_id"] = src["dest_warp_id"]; changes += 1
    return changes

def merge_bg(pe_bg, ps_bg):
    if not pe_bg or not ps_bg:
        return 0
    ps_by_key = {}
    for b in ps_bg:
        key = (b.get("x"), b.get("y"), b.get("type"))
        ps_by_key.setdefault(key, []).append(b)
    changes = 0
    for b in pe_bg:
        key = (b.get("x"), b.get("y"), b.get("type"))
        cands = ps_by_key.get(key)
        if not cands:
            continue
        src = cands.pop(0)
        sscript = src.get("script")
        if sscript and sscript != "NULL" and sscript != b.get("script") and sscript in KNOWN_LABELS:
            b["script"] = sscript; changes += 1
        sflag = src.get("flag")
        if sflag and sflag != b.get("flag") and sflag in KNOWN_FLAGS:
            b["flag"] = sflag; changes += 1
    return changes

def main():
    maps = [m.strip() for m in LIST.read_text().splitlines() if m.strip()]
    total_changes = 0
    changed_files = []
    skipped = []
    for name in maps:
        pe_json = PE_ROOT / "data" / "maps" / name / "map.json"
        ps_json = PS_ROOT / "data" / "maps" / name / "map.json"
        if not pe_json.exists() or not ps_json.exists():
            skipped.append(name)
            continue
        pe = load(pe_json)
        ps = load(ps_json)
        changes = 0
        changes += merge_objects(pe.get("object_events"), ps.get("object_events"))
        changes += merge_warps(pe.get("warp_events"), ps.get("warp_events"))
        changes += merge_bg(pe.get("bg_events"), ps.get("bg_events"))
        # also script header field on map itself
        if ps.get("script") and pe.get("script") in (None, "NULL") and ps.get("script") != "NULL":
            pe["script"] = ps["script"]; changes += 1
        if changes:
            save(pe_json, pe)
            total_changes += changes
            changed_files.append((name, changes))
    print(f"Files changed: {len(changed_files)} / {len(maps)}  ({len(skipped)} skipped, missing map.json)")
    print(f"Total field updates: {total_changes}")
    if skipped:
        print("Skipped:", ", ".join(skipped[:20]), "..." if len(skipped) > 20 else "")
    print("Sample of changed files:")
    for n, c in changed_files[:20]:
        print(f"  {n}: {c}")

if __name__ == "__main__":
    main()
