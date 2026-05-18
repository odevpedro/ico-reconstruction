#!/usr/bin/env python3
"""Batch score all decompiled C functions against the ELF target.

Usage: python3 tools/score_all.py [--summary]
  --summary: Only print summary table, skip per-function output
"""
import argparse
import subprocess
import sys
import re

TOOL = "python3 tools/ee_gcc_compile.py"

# (file, funct_name, target_va, size)
# (file, funct_name, target_va, size_bytes)
FUNCTIONS = [
    # ---- Entity handlers ----
    ("src/entity/enemy1.c", "enemy1_init",   0x164440, 0x460),
    ("src/entity/enemy1.c", "enemy1_hC",     0x1CE220, 0x1A0),
    ("src/entity/enemy1.c", "enemy1_hB",     0x1CE3C0, 0x240),
    ("src/entity/enemy1.c", "enemy1_hA",     0x1CE690, 0x38),
    ("src/entity/enemy1.c", "fn_1CE5F8",     0x1CE5F8, 0x98),
    ("src/entity/boy.c",    "boy_init",       0x153478, 0xEC),
    ("src/entity/boy.c",    "boy_hC",         0x1C1A98, 0x1B0),
    ("src/entity/boy.c",    "sub_1C1C48",     0x1C1C48, 0x190),
    ("src/entity/boy.c",    "boy_hB",         0x1C1DD8, 0xD0),
    ("src/entity/boy.c",    "sub_1C1EA8",     0x1C1EA8, 0xB0),
    ("src/entity/boy.c",    "boy_hA",         0x1C1F58, 0x140),
    ("src/entity/boy.c",    "boy_set_state",  0x1C2098, 0x10),
    ("src/entity/boy.c",    "boy_float_accum",0x1C20A8, 0xC8),
    ("src/entity/boy.c",    "boy_activate",   0x1C2170, 0x28),
    # barrel.c (unique functions only, cloth duplicates omitted)
    ("src/entity/barrel.c", "barrel_init",   0x166028, 0x8C),
    ("src/entity/barrel.c", "barrel_hA",     0x1D2540, 0x10),
    ("src/entity/barrel.c", "barrel_hA_alt", 0x1D2548, 0x10),
    ("src/entity/barrel.c", "fn_1D2550",     0x1D2550, 0xE8),
    ("src/entity/barrel.c", "sub_1D2650",    0x1D2650, 0xE8),
    ("src/entity/barrel.c", "sub_1D2738",    0x1D2738, 0x68),
    ("src/entity/barrel.c", "barrel_hC",     0x1D27A8, 0x380),
    ("src/entity/barrel.c", "rope_hC",       0x1D3B28, 0x108),
    ("src/entity/barrel.c", "cb_routine2",   0x1D3A30, 0xE0),
    ("src/entity/barrel.c", "fn_1D3BF0",     0x1D3BF0, 0x140),
    ("src/entity/barrel.c", "fn_1D3DD8",     0x1D3DD8, 0x130),
    # woodbox0.c
    ("src/entity/woodbox0.c", "woodbox0_hC", 0x1C00C0, 0x478),
    ("src/entity/woodbox0.c", "woodbox0_hB", 0x1C0538, 0x98),
    ("src/entity/woodbox0.c", "woodbox0_hA", 0x1C05D0, 0x60),
    # entity near_matches.c (verified VAs only)
    ("src/entity/near_matches.c", "bird_hC",       0x197240, 0x200),
    ("src/entity/near_matches.c", "attackch62_hC", 0x1BBE50, 0x100),
    # ---- Cloth handlers ----
    ("src/cloth/accessors.c", "cloth_get_variant",           0x1D3D70, 0x10),
    ("src/cloth/accessors.c", "cloth_payload_field0_is_zero",0x1D3D80, 0x18),
    ("src/cloth/accessors.c", "cloth_payload_state_is_two",  0x1D3D98, 0x18),
    ("src/cloth/near_matches.c", "cloth_test_variant_field",   0x1D3DB0, 0x28),
    ("src/cloth/near_matches.c", "cloth_test_field0_or_extra", 0x1D40A0, 0x38),
    ("src/cloth/near_matches.c", "cloth_dispatcher",          0x1D37C8, 0x60),
    ("src/cloth/near_matches.c", "clothSubForceApply",        0x1D3F78, 0x80),
]


def extract_score(output: str) -> float:
    m = re.search(r'Score:\s*(\d+)/(\d+)\s*=\s*([\d.]+)%', output)
    if m:
        return float(m.group(3))
    # Check for compilation errors
    if "FAILED" in output or "COMPILE ERROR" in output:
        return -1.0
    if "PERFECT MATCH" in output:
        return 100.0
    return -2.0


def main():
    parser = argparse.ArgumentParser(description="Batch score all decompiled functions")
    parser.add_argument("--summary", action="store_true", help="Only print summary table")
    args = parser.parse_args()

    results = []
    for filepath, fn_name, va, size in FUNCTIONS:
        # Use --whole-file for files that need it (compilation errors in other funcs)
        whole = ""
        if filepath.startswith("src/entity/") and fn_name != "fn_1CE5F8":
            whole = "--whole-file"
        cmd = f"{TOOL} {whole} {filepath} --fn {fn_name} --va 0x{va:X} --size {size}"
        r = subprocess.run(cmd, shell=True, capture_output=True, text=True, timeout=120)
        output = r.stdout + r.stderr
        score = extract_score(output)

        if not args.summary:
            print(f"{'='*60}")
            print(f"Scoring: {fn_name} ({filepath})")
            print(f"{'='*60}")
            print(output)
        else:
            # Print minimal per-function output
            m = re.search(r'Target instructions: (\d+)', output)
            target_insns = int(m.group(1)) if m else 0
            m2 = re.search(r'Generated instructions: (\d+)', output)
            gen_insns = int(m2.group(1)) if m2 else 0
            print(f"  {fn_name:30s} | score={score:6.2f}% | tgt={target_insns:3d} gen={gen_insns:3d}")

        results.append((fn_name, score))

    # Summary table
    print(f"\n{'='*60}")
    print("SCORING SUMMARY")
    print(f"{'='*60}")
    print(f"{'Function':35s} {'Score':>8s} {'Status'}")
    print(f"{'-'*35} {'-'*8} {'-'*20}")
    
    for fn_name, score in results:
        if score == 100.0:
            status = "EXACT MATCH"
        elif score >= 90:
            status = "NEAR EXACT"
        elif score >= 70:
            status = "STRUCTURAL OK"
        elif score >= 50:
            status = "PARTIAL"
        elif score == -1:
            status = "COMPILE ERROR"
        elif score == -2:
            status = "NOT FOUND"
        else:
            status = "LOW"
        print(f"{fn_name:35s} {score:7.2f}%  {status}")
    
    print(f"\nTotal: {len(results)} functions")
    perfect = sum(1 for _, s in results if s == 100.0)
    partial = sum(1 for _, s in results if 0 < s < 100.0)
    errors = sum(1 for _, s in results if s < 0)
    print(f"Perfect: {perfect}, Partial: {partial}, Errors/NA: {errors}")


if __name__ == "__main__":
    main()
