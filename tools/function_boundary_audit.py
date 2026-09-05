#!/usr/bin/env python3
"""Audit .s function boundaries against the USA ELF (canonical method).

Detects the Rev.131 class of bug: a .s captured by scanning for the first
`jr $ra` can over-run the real function end when the function finishes with a
tail-jump (`j 0x...`) instead of `jr $31`, silently swallowing the NEXT
function into the same .s file.

Method
------
Canonical segmentation (validated on GeneratorDL, type22_hA, type24_hB,
boyAI_sub_1446E0, eBrainGetTargetGeneratorFromLabelStage, type23_hB):

1. Recover the function start VA:
     a. internal `loc_XXXXXXXX:` anchors (label VA = base + 4*insn_index);
     b. trailing 6-hex-digit in the file name (e.g. boyAI_sub_1446E0);
     c. TARGET_FUNCTIONS table in tools/asm_source_score.py.
2. Count instructions with a multi-dot mnemonic regex (`cvt.w.s`, `bgezl`,
   `c.olt.s`, ...) — the single-dot regex silently mis-slots instructions.
3. Compute the canonical end:
     end = addr(last function-level terminator among:
             `jr $31` / `jalr $31` / `j <out-of-block absolute>`)
           + 8   (jr + its delay slot)
   bounded by the next canonical prologue start from
   `.local/reports/elf_real_prologues.json` (3846 starts) or the next
   detected sibling prologue inside the block (M1 heuristic).
4. Classify each .s:
     exact      current size == canonical window;
     INFLATED   current size > canonical window (swallows next function);
     truncated  current size < canonical window (caps at mid-function);
     misfiled   no in-ELF VA / start does not match a canonical start.

The report is printed as a table and written to .local/reports/. With
`--write`, INFLATED files are truncated at their canonical end and re-verified
byte-exact against the ELF before overwriting.

Verification uses the same assemble_and_verify pipeline as the scoring tool
so truncated files are proven byte-exact (0 tolerance) before landing.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import struct
import sys
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ELF_PATH = ROOT / ".local/extracted/SCUS_971.13.elf"
SRC_ROOTS = [ROOT / "src/entity/asm", ROOT / "src/cloth/asm", ROOT / "src/core/asm"]
REPORT_DIR = ROOT / ".local/reports"
PROLOGUE_JSON = REPORT_DIR / "elf_real_prologues.json"

sys.path.insert(0, str(ROOT / "tools"))

# Multi-dot mnemonic regex: `cvt.w.s`, `c.olt.s`, `bgezl`, `.word` etc.
# A single-dot regex `\w+\.?\w*` FAILS to match `cvt.w.s` and mis-slots every
# instruction after it, producing phantom boundary inflation.
INSN_RE = re.compile(r"^\t(?:(?:\w+(?:\.\w+)+|\w+|\.word)\t)")
LOC_RE = re.compile(r"^loc_([0-9a-fA-F]{8}):\s*$")


def utc_now() -> str:
    return datetime.now(timezone.utc).replace(microsecond=0).isoformat()


def load_elf(path: Path) -> tuple[bytes, list[dict]]:
    data = path.read_bytes()
    e_phoff = struct.unpack_from("<I", data, 0x1C)[0]
    e_phentsize = struct.unpack_from("<H", data, 0x2A)[0]
    e_phnum = struct.unpack_from("<H", data, 0x2C)[0]
    segments = []
    for i in range(e_phnum):
        ph = data[e_phoff + i * e_phentsize: e_phoff + (i + 1) * e_phentsize]
        if struct.unpack_from("<I", ph, 0)[0] != 1:  # PT_LOAD
            continue
        segments.append({
            "off": struct.unpack_from("<I", ph, 4)[0],
            "vaddr": struct.unpack_from("<I", ph, 8)[0],
            "filesz": struct.unpack_from("<I", ph, 16)[0],
        })
    return data, segments


def extract(elf: bytes, segs: list[dict], va: int, size: int) -> bytes | None:
    for seg in segs:
        if seg["vaddr"] <= va < seg["vaddr"] + seg["filesz"]:
            off = seg["off"] + (va - seg["vaddr"])
            avail = min(size, seg["filesz"] - (va - seg["vaddr"]))
            return elf[off:off + avail]
    return None


def split_body_footer(path: Path) -> tuple[list[str], list[str]]:
    lines = path.read_text().splitlines()
    try:
        split = lines.index("\t.set\tmacro")
    except ValueError:
        return lines, []
    return lines[:split], lines[split:]


def count_insns(lines: list[str]) -> int:
    return sum(1 for l in lines if INSN_RE.match(l))


def recover_va(path: Path, body: list[str], elf: bytes,
               segs: list[dict]) -> int | None:
    """Recover the function start VA.

    Priority:
      1. first generated `loc_XXXXXXXX:` anchor (label = base + 4*insn_idx);
      2. trailing 6-hex-digit in the file name (sub_1435A0 => 0x1435A0);
      3. TARGET_FUNCTIONS table in tools/asm_source_score.py (by name);
      4. byte fingerprint: assemble the first 8-16 instructions and scan the
         ELF .text segment for the exact byte sequence (files are byte-exact,
         so a unique match is the real start).
    """
    slot = 0
    for l in body:
        m = LOC_RE.match(l)
        if m:
            return int(m.group(1), 16) - 4 * slot
        if INSN_RE.match(l):
            slot += 1
    m = re.search(r"([0-9a-fA-F]{6})\s*$", path.stem)
    if m:
        return int(m.group(1), 16)
    name = ""
    for l in body:
        if l.startswith(".globl\t"):
            name = l.split("\t", 1)[1].strip()
            break
    if name:
        try:
            import asm_source_score as ss
            for entry in getattr(ss, "TARGET_FUNCTIONS", []) or []:
                if entry[0] == name:
                    return entry[1]
        except Exception:
            pass
    return _recover_va_fingerprint(path, body, elf, segs)


def _recover_va_fingerprint(path: Path, body: list[str],
                            elf: bytes, segs: list[dict]) -> int | None:
    """Assemble up to 16 leading instructions and find them in the ELF once."""
    import tempfile
    import subprocess
    import asm_source_score as ss

    insn_lines = [l for l in body if INSN_RE.match(l)][:16]
    if not insn_lines:
        return None
    # Keep the loc_ label lines that precede the cut point for branch
    # resolution, and emit the rest (if any) as .word to avoid unresolved
    # forward refs corrupting the fingerprint semantics.
    prologue = []
    slot = 0
    for l in body:
        if INSN_RE.match(l):
            if slot < len(insn_lines):
                prologue.append(l)
            else:
                break
            slot += 1
        elif LOC_RE.match(l) and slot < len(insn_lines):
            prologue.append(l)

    header = ("\t.text\n\t.p2align 3\n\t.globl\tfp\n\t.ent\tfp\nfp:\n"
              "\t.set\tnoreorder\n\t.set\tnomacro\n\t.set\tnoat\n")
    footer = "\t.set\tmacro\n\t.set\treorder\n\t.end\tfp\n"
    with tempfile.NamedTemporaryFile(suffix=".s", delete=False) as f:
        f.write((header + "\n".join(prologue) + "\n" + footer).encode())
        src = f.name
    obj = src + ".o"
    try:
        r = subprocess.run([ss.EE_GCC, "-c", src, "-o", obj] + list(ss.CC_FLAGS),
                           capture_output=True)
        if r.returncode != 0:
            return None
        blob = ss.extract_text_from_o(Path(obj).read_bytes())
    finally:
        for f in (src, obj):
            if os.path.exists(f):
                os.unlink(f)
    if not blob:
        return None

    # Scan every PT_LOAD segment for an exact byte match. Branches within the
    # prologue may assemble to symbols (0) if targets were cut; only trust a
    # match found EXACTLY ONCE across the whole load image.
    hits = []
    for seg in segs:
        start = seg["off"]
        base = seg["vaddr"]
        window = seg["filesz"]
        hay = elf[start:start + window]
        pos = hay.find(blob)
        while pos != -1:
            hits.append(base + pos)
            pos = hay.find(blob, pos + 1)
    return hits[0] if len(hits) == 1 else None


def disasm_blocks(data: bytes, start_va: int):
    import capstone
    md = capstone.Cs(capstone.CS_ARCH_MIPS,
                     capstone.CS_MODE_MIPS64 + capstone.CS_MODE_LITTLE_ENDIAN)
    return [i for i in md.disasm(data, start_va)]


def is_ret(mnem: str, ops: str) -> bool:
    return mnem in ("jr", "jalr") and ops in ("$31", "$ra", "31", "ra")


def canonical_end(body: list[str], va: int, starts: list[int]) -> int | None:
    """Canonical end of the function owned by this .s.

    A .s captured by scanning for the *first* `jr $ra` can over-run the real
    function end in two ways:

      a) the function finishes with a tail-jump (`j 0x...`) whose target lies
         OUTSIDE the block [va, va + block); the old fixed 0x200000 band
         classified far in-ELF tails (e.g. `j 0x2641d8` from bossctrl_hB) as
         in-block and never recognised them as terminators;
      b) the following function is a *prologue-less leaf* (e.g. attackch62_hC:
         next function at 0x1BBEA0 starts straight with `lw $v0,0x15c($a0)`),
         so "last terminator + 8" lands inside it. The 3846-entry prologue set
         cannot bound these, but the canonical end is bounded by them anyway:
         end = min(last-ish terminator + 8, next canonical start after va).

    Rule (validated on all 12 Rev.132 truncations):
        heuristic = addr(last function-level terminator) + 8, where a
        function-level terminator is `jr/jalr $31` or an absolute `j tgt`
        with tgt outside [va, va + block);
        bound      = next canonical start > va (prologue set);
        canonical  = min(heuristic, bound) if both exist, else whichever
        exists; None when neither does.
    """
    import bisect
    block = va + 4 * count_insns(body)
    addr = va
    last = 0
    seen_insn = False
    for l in body:
        alive = va <= addr < block
        if INSN_RE.match(l):
            seen_insn = True
            s = l.strip()
            if s.startswith("jr\t") or s.startswith("jalr\t"):
                if alive and ("$31" in s or "$ra" in s or "31" in s or "ra" in s):
                    last = addr
            elif s.startswith("j\t"):
                t = s.split("\t", 1)[1].strip()
                try:
                    tgt = int(t, 0)
                except ValueError:
                    tgt = t
                if alive and isinstance(tgt, int) and (tgt < va or tgt >= block):
                    last = addr
            addr += 4
    if not seen_insn:
        return None
    cand: list[int] = []
    if last:
        cand.append(last + 8)
    i = bisect.bisect_right(starts, va)
    if i < len(starts):
        cand.append(starts[i])
    return min(cand) if cand else None


def audit_file(path: Path, elf: bytes, segs: list[dict], starts: set[int]) -> dict:
    body, footer = split_body_footer(path)
    name = ""
    for l in path.read_text().splitlines():
        if l.startswith(".globl\t"):
            name = l.split("\t", 1)[1].strip()
            break
    va = recover_va(path, body, elf, segs)
    size = count_insns(body) * 4
    row = {
        "file": str(path.relative_to(ROOT)),
        "name": name,
        "va": f"0x{va:08x}" if va else None,
        "insn_count": count_insns(body),
        "size": size,
        "verdict": "unknown",
    }
    if va is None:
        row["verdict"] = "misfiled-no-va"
        return row
    in_load = extract(elf, segs, va, 4)
    if in_load is None:
        row["verdict"] = "misfiled-not-in-elf"
        return row
    # The 3846-entry prologue set is KNOWN-incomplete (prologue-less leaf
    # functions like isysGObjProcRemoveUnlink at a verified byte-exact site
    # are missed), so membership is informational, not blocking.
    if starts and va not in starts:
        row["start_note"] = "not-in-prologue-set"

    end = canonical_end(body, va, sorted(starts))
    if end is None:
        row["verdict"] = "no-terminator"
        return row
    window = end - va
    row["canonical_size"] = window
    if size == window:
        row["verdict"] = "exact"
    elif size > window:
        row["verdict"] = "INFLATED"
    else:
        row["verdict"] = "truncated"
    return row


def truncate_file(path: Path, canonical_size: int, va: int) -> dict:
    """Truncate INFLATED .s at canonical size and verify byte-exact."""
    import asm_source_score as ss
    body, footer = split_body_footer(path)
    out = []
    slot = 0
    for l in body:
        if INSN_RE.match(l):
            a = va + 4 * slot
            slot += 1
            if a < va + canonical_size:
                out.append(l)
        elif LOC_RE.match(l):
            a = va + 4 * slot
            if a < va + canonical_size:
                out.append(l)
        else:
            out.append(l)
    newsrc = "\n".join(out) + "\n" + "\n".join(footer) + "\n"
    name = body and path.stem or path.stem
    r = ss.assemble_and_verify(newsrc, name, va, canonical_size)
    if not r["success"]:
        return {"wrote": False, "error": r.get("error")}
    path.write_text(newsrc)
    return {"wrote": True, "size": canonical_size}


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--json", action="store_true", help="write JSON report")
    ap.add_argument("--write", action="store_true",
                    help="truncate INFLATED .s at canonical end (byte-exact verified)")
    ap.add_argument("--no-prologues", action="store_true",
                    help="skip the canonical start-set membership check")
    args = ap.parse_args()

    elf, segs = load_elf(ELF_PATH)
    starts: set[int] = set()
    if not args.no_prologues:
        if PROLOGUE_JSON.exists():
            starts = {int(x, 16) for x in json.loads(PROLOGUE_JSON.read_text())}
        else:
            print(f"!! {PROLOGUE_JSON} missing; run the prologue scan first "
                  f"(or pass --no-prologues)", file=sys.stderr)

    rows = []
    for root in SRC_ROOTS:
        for p in sorted(root.glob("*.s")):
            rows.append(audit_file(p, elf, segs, starts))

    inflated = [r for r in rows if r["verdict"] in ("INFLATED",)]
    truncated = [r for r in rows if r["verdict"] == "truncated"]
    exact = [r for r in rows if r["verdict"] == "exact"]
    other = [r for r in rows if r["verdict"] not in ("INFLATED", "truncated", "exact")]

    print(f"{'FILE':72} {'SIZE':6} {'CANON':6} {'VERDICT':16} NOTE")
    print("-" * 130)
    for r in rows:
        note = ""
        if "misfiled" in r["verdict"]:
            note = "va not recovered / not a canonical start"
        elif r["verdict"] == "truncated":
            note = "window < canonical; may cap mid-function"
        print(f"{r['file']:72} {r['size']:6} {r.get('canonical_size','-'):>6} "
              f"{r['verdict']:16} {note}")

    print()
    print(f"total={len(rows)}  exact={len(exact)}  INFLATED={len(inflated)}  "
          f"truncated={len(truncated)}  other={len(other)}")

    if args.write:
        done = []
        for r in inflated:
            res = truncate_file(ROOT / r["file"], r["canonical_size"],
                                int(r["va"], 16))
            done.append({**r, **res})
            print(f"  {'WROTE' if res['wrote'] else 'FAIL '} "
                  f"{r['file']} -> {res.get('size', res.get('error'))}")
        ok = sum(1 for d in done if d["wrote"])
        print(f"  truncation: {ok}/{len(done)} written byte-exact")

    if args.json:
        REPORT_DIR.mkdir(parents=True, exist_ok=True)
        out = REPORT_DIR / "function_boundary_audit.json"
        out.write_text(json.dumps({
            "tool": "function_boundary_audit (canonical)",
            "generated_at": utc_now(),
            "elf": str(ELF_PATH),
            "prologue_set": str(PROLOGUE_JSON),
            "summary": {"total": len(rows), "exact": len(exact),
                        "inflated": len(inflated), "truncated": len(truncated),
                        "other": len(other)},
            "rows": rows,
            "write_results": [] if not args.write else done,
        }, indent=2))
        print(f"report: {out}")

    return 0 if not inflated else 2


if __name__ == "__main__":
    raise SystemExit(main())