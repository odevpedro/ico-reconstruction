#!/usr/bin/env python3
"""Classify each instruction diff using structural (LCS) alignment.

Usage:
    python3 tools/diff_classifier.py              # Analyze all 37 functions
    python3 tools/diff_classifier.py --fn fn_1CE5F8  # Single function deep-diff
    python3 tools/diff_classifier.py --json       # Machine-readable output
"""
import argparse
import json
import os
import re
import subprocess
import sys
import tempfile

sys.path.insert(0, __file__.rsplit("/", 1)[0])
from ee_gcc_compile import (
    disassemble_mips64, normalize_insn, parse_asm_lines,
    extract_elf_func, compile_c_to_asm, extract_func_asm,
    align_instructions, normalize_target_insns, ABI_TO_NUM,
    CC_FLAGS, SRC_ROOT,
)
from score_all import FUNCTIONS


# ============================================================
# Class categories
# ============================================================
CLASS_ORDER = [
    "MATCH", "LABEL", "PSEUDO_OP", "FRAME", "IMMEDIATE",
    "REG_ALLOC", "INSN_SEL",
    "MISSING", "EXTRA", "MISMATCH",
]

CLASS_DESCRIPTIONS = {
    "MATCH":     "Exact match after normalization",
    "LABEL":     "Same instruction, different target label (addr vs $Lxx)",
    "PSEUDO_OP": "GAS pseudo-op expansion (beqz→beq, l.s→lwc1, etc)",
    "FRAME":     "Frame opcode diff (addiu vs subu — same encoding)",
    "IMMEDIATE": "Immediate form diff (sltiu vs sltu $1)",
    "REG_ALLOC": "Same opcode, different register allocation",
    "INSN_SEL":  "Different instruction for same op (move vs or $0)",
    "MISSING":   "Missing in generated code",
    "EXTRA":     "Extra instruction in generated code",
    "MISMATCH":  "Structural difference (different mnemonic or operands)",
}


def classify_aligned(tgt_text: str, gen_text: str, status: str) -> str:
    """Classify a single aligned pair."""
    if status == "match":
        return "MATCH"
    if status == "missing":
        return "MISSING"
    if status == "extra":
        return "EXTRA"
    # status == mismatch
    if not tgt_text or not gen_text:
        return "MISMATCH"

    t_parts = tgt_text.split(None, 1)
    g_parts = gen_text.split(None, 1)
    t_mnem = t_parts[0] if len(t_parts) >= 1 else ""
    g_mnem = g_parts[0] if len(g_parts) >= 1 else ""
    t_ops = t_parts[1] if len(t_parts) >= 2 else ""
    g_ops = g_parts[1] if len(g_parts) >= 2 else ""

    # Mnemonic match: classify operand differences
    if t_mnem == g_mnem:
        if t_mnem in ("jal", "j", "jalr", "beq", "bne", "bc1f", "bc1t",
                       "bltz", "bgtz", "blez", "bgez"):
            t_ops_list = [op.strip() for op in t_ops.split(",")]
            g_ops_list = [op.strip() for op in g_ops.split(",")]
            if (len(t_ops_list) == len(g_ops_list)
                    and t_ops_list[:-1] != g_ops_list[:-1]
                    and t_ops_list[-1] == g_ops_list[-1]):
                return "REG_ALLOC"
            return "LABEL"
        # Check register-only differences
        t_nums = re.findall(r'\b\d+\b', t_ops)
        g_nums = re.findall(r'\b\d+\b', g_ops)
        t_pat = re.sub(r'\b\d+\b', 'N', t_ops)
        g_pat = re.sub(r'\b\d+\b', 'N', g_ops)
        if t_pat == g_pat:
            return "REG_ALLOC"
        # Offset diff in same kind of instruction (e.g. lw $2, X($3) vs lw $2, Y($3))
        if re.sub(r'\b\d+\b', 'N', t_ops) == re.sub(r'\b\d+\b', 'N', g_ops):
            return "REG_ALLOC"
        return "MISMATCH"

    # PSEUDO_OP: beqz/beq, bnez/bne
    if (t_mnem, g_mnem) in (("beqz", "beq"), ("beq", "beqz"),
                              ("bnez", "bne"), ("bne", "bnez")):
        return "PSEUDO_OP"

    # l.s/lwc1, s.s/swc1
    if {t_mnem, g_mnem} <= {"lwc1", "l.s"}:
        return "PSEUDO_OP"
    if {t_mnem, g_mnem} <= {"swc1", "s.s"}:
        return "PSEUDO_OP"

    # j vs jr $31
    if t_mnem in ("j", "jr") and g_mnem in ("j", "jr"):
        t_ops_s = re.sub(r'\$', '', t_ops).strip()
        g_ops_s = re.sub(r'\$', '', g_ops).strip()
        if t_ops_s in ("31", "ra") and g_ops_s in ("31", "ra"):
            return "PSEUDO_OP"

    # FRAME: addiu $sp vs subu $sp
    if t_mnem in ("addiu", "subu", "addu") and g_mnem in ("addiu", "subu", "addu"):
        return "FRAME"

    # IMMEDIATE: sltiu vs sltu
    if {t_mnem, g_mnem} <= {"sltiu", "sltu"}:
        return "IMMEDIATE"

    # INSN_SEL: move vs or $0
    if {t_mnem, g_mnem} <= {"move", "or"}:
        return "INSN_SEL"

    return "MISMATCH"


def compile_and_align(filepath: str, fn_name: str, va: int, size: int) -> dict:
    """Compile a function and align its instructions with the target."""
    with open(filepath) as f:
        c_code = f.read()

    asm = compile_c_to_asm(c_code)
    if asm is None:
        return {"fn": fn_name, "error": "COMPILE_ERROR"}

    func_asm = extract_func_asm(asm, fn_name)
    if not func_asm.strip():
        return {"fn": fn_name, "error": f"FUNCTION_NOT_FOUND"}

    target_bytes = extract_elf_func(va, size)
    target_insns = disassemble_mips64(target_bytes, va)

    gen_lines = parse_asm_lines(func_asm)
    tgt_lines = normalize_target_insns(target_insns)

    while tgt_lines and tgt_lines[-1] == 'nop':
        tgt_lines.pop()
    while gen_lines and gen_lines[-1] == 'nop':
        gen_lines.pop()

    aligned = align_instructions(tgt_lines, gen_lines)

    # Classify each aligned pair
    classified = []
    for tgt_i, gen_i, tgt_text, gen_text, status in aligned:
        cls = classify_aligned(tgt_text, gen_text, status)
        classified.append({
            "tgt_i": tgt_i,
            "gen_i": gen_i,
            "tgt": tgt_text,
            "gen": gen_text,
            "status": status,
            "class": cls,
        })

    # Score
    score = 0
    max_score = len(tgt_lines) * 100
    for c in classified:
        if c["class"] == "MATCH":
            score += 100
        elif c["class"] in ("MISSING", "EXTRA"):
            pass
        else:
            tgt_mnem = c["tgt"].split()[0] if c["tgt"] and c["tgt"] != "(missing)" else ""
            gen_mnem = c["gen"].split()[0] if c["gen"] and c["gen"] != "(missing)" else ""
            if tgt_mnem and tgt_mnem == gen_mnem:
                score += 50

    # Aggregate counts
    counts = {c: 0 for c in CLASS_ORDER}
    for c in classified:
        counts[c["class"]] = counts.get(c["class"], 0) + 1

    match_count = counts["MATCH"]

    return {
        "fn": fn_name,
        "va": va,
        "size": size,
        "file": filepath,
        "tgt_count": len(tgt_lines),
        "gen_count": len(gen_lines),
        "score": score,
        "max_score": max_score,
        "pct": score / max_score * 100 if max_score else 0,
        "match_count": match_count,
        "aligned": classified,
        "counts": counts,
        "error": None,
    }


def report_summary(results: list[dict], use_json: bool = False):
    """Print summary table of all functions."""
    if use_json:
        summary = []
        for r in results:
            if r.get("error"):
                summary.append({"fn": r["fn"], "error": r["error"]})
            else:
                summary.append({
                    "fn": r["fn"],
                    "score": r["score"],
                    "max_score": r["max_score"],
                    "pct": round(r["pct"], 2),
                    "tgt_count": r["tgt_count"],
                    "gen_count": r["gen_count"],
                    "match_count": r["match_count"],
                    "counts": r["counts"],
                })
        print(json.dumps(summary, indent=2))
        return

    print()
    print("=" * 140)
    print("DIFF CLASSIFICATION — STRUCTURAL (LCS) ALIGNMENT")
    print("=" * 140)
    h = f"{'Function':30s} {'Score':>7s} {'%':>6s} {'Match/Tot':>9s} "
    for cls in CLASS_ORDER:
        if cls == "MATCH":
            continue
        h += f"{cls:>8s}"
    print(h)
    print("-" * 140)

    totals = {cls: 0 for cls in CLASS_ORDER}
    for r in results:
        if r.get("error"):
            print(f"{r['fn']:30s}  ERROR: {r['error']}")
            continue
        sc = f"{r['score']}/{r['max_score']}"
        pct = f"{r['pct']:.1f}"
        mc = r["match_count"]
        tc = r["tgt_count"]
        line = f"{r['fn']:30s} {sc:>7s} {pct:>6s} {mc:>5d}/{tc:<5d} "
        for cls in CLASS_ORDER:
            if cls == "MATCH":
                continue
            cnt = r["counts"].get(cls, 0)
            line += f"{cnt:>8d}"
            totals[cls] = totals.get(cls, 0) + cnt
        print(line)

    # Totals row
    print("-" * 140)
    total_score = sum(r.get("score", 0) for r in results if not r.get("error"))
    total_max = sum(r.get("max_score", 0) for r in results if not r.get("error"))
    total_match = sum(r.get("match_count", 0) for r in results if not r.get("error"))
    total_insns = sum(r.get("tgt_count", 0) for r in results if not r.get("error"))
    pct = total_score / total_max * 100 if total_max else 0
    line = f"{'TOTAL':30s} {total_score}/{total_max} {pct:>5.1f}% {total_match:>5d}/{total_insns:<5d} "
    for cls in CLASS_ORDER:
        if cls == "MATCH":
            continue
        line += f"{totals[cls]:>8d}"
    print(line)

    # Class descriptions
    print()
    print("=" * 60)
    print("CLASS DESCRIPTIONS")
    print("=" * 60)
    for cls in CLASS_ORDER:
        if cls == "MATCH":
            continue
        desc = CLASS_DESCRIPTIONS.get(cls, "")
        cnt = totals.get(cls, 0)
        print(f"  {cls:12s} ({cnt:4d})  {desc}")

    # Structural vs cosmetic breakdown
    cosmetic = totals.get("LABEL", 0) + totals.get("PSEUDO_OP", 0) + totals.get("FRAME", 0) + totals.get("IMMEDIATE", 0)
    structural = totals.get("REG_ALLOC", 0) + totals.get("INSN_SEL", 0) + totals.get("MISSING", 0) + totals.get("EXTRA", 0) + totals.get("MISMATCH", 0)
    total_diffs = cosmetic + structural
    print()
    print("=" * 60)
    print("BREAKDOWN")
    print("=" * 60)
    print(f"  Total instructions (all functions): {total_insns}")
    print(f"  Exact matches: {total_match}")
    print(f"  Diffs: {total_diffs}")
    print(f"    Cosmetic (normalizable): {cosmetic} ({cosmetic/max(total_diffs,1)*100:.1f}%)")
    print(f"    Structural (real):       {structural} ({structural/max(total_diffs,1)*100:.1f}%)")
    print(f"      REG_ALLOC:  {totals.get('REG_ALLOC', 0)}")
    print(f"      INSN_SEL:   {totals.get('INSN_SEL', 0)}")
    print(f"      MISSING:    {totals.get('MISSING', 0)}")
    print(f"      EXTRA:      {totals.get('EXTRA', 0)}")
    print(f"      MISMATCH:   {totals.get('MISMATCH', 0)}")

    # Fixability
    print()
    print("=" * 60)
    print("FIXABILITY ROADMAP")
    print("=" * 60)
    fixable = {
        "LABEL":     "High — can normalize in pipeline via label→address mapping",
        "PSEUDO_OP": "High — already partially handled, extend normalize_insn",
        "FRAME":     "High — already handled",
        "IMMEDIATE": "High — already handled",
        "REG_ALLOC": "Low — hard to control without C restructuring or register vars",
        "INSN_SEL":  "Medium — different GCC version preference",
        "MISSING":   "Medium — asm barriers or code restructuring",
        "EXTRA":     "Low — compiler emits redundant code",
        "MISMATCH":  "Low — diverse causes, per-function investigation needed",
    }
    for cls, fix in fixable.items():
        cnt = totals.get(cls, 0)
        if cnt:
            print(f"  {cls:12s} ({cnt:4d})  {fix}")

    # Show examples of structural diffs
    print()
    print("=" * 60)
    print("STRUCTURAL DIFF EXAMPLES")
    print("=" * 60)
    for r in results:
        if r.get("error"):
            continue
        structural_classes = ["REG_ALLOC", "INSN_SEL", "MISMATCH"]
        examples = [c for c in r["aligned"] if c["class"] in structural_classes]
        if not examples:
            continue
        print(f"\n  {r['fn']} ({r['pct']:.1f}%):")
        for c in examples[:5]:
            ti = c["tgt_i"] if c["tgt_i"] is not None else "?"
            gi = c["gen_i"] if c["gen_i"] is not None else "?"
            tgt = c["tgt"] if c["tgt"] != "(missing)" else "--"
            gen = c["gen"] if c["gen"] != "(missing)" else "--"
            print(f"    [{c['class']:10s}] T:{ti}:{tgt:30s}  G:{gi}:{gen}")


def main():
    parser = argparse.ArgumentParser(description="Classify instruction diffs (structural alignment)")
    parser.add_argument("--fn", type=str, default=None, help="Single function name")
    parser.add_argument("--json", action="store_true", help="JSON output")
    args = parser.parse_args()

    results = []
    for filepath, fn_name, va, size in FUNCTIONS:
        if args.fn and fn_name != args.fn:
            continue
        result = compile_and_align(filepath, fn_name, va, size)
        results.append(result)

    report_summary(results, use_json=args.json)


if __name__ == "__main__":
    main()
