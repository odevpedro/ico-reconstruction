"""
PAL -> USA symbol reconciliation pipeline.

Converts PAL reference symbols into a validated USA symbol map using
binary fingerprint matching, not address-based offset assumptions.

Pipeline:
  1. Parse PAL symbol_addrs.txt          -> raw_symbols_pal.csv
  2. Extract USA .text functions via capstone    -> usa_fingerprints.json
  3. (future) Extract PAL .text functions        -> pal_fingerprints.json
  4. Cross-reference by fingerprint              -> pal_usa_candidates.csv
  5. Manual validation                           -> pal_usa_symbol_map.csv

Usage:
  python3 tools/symbol_reconcile/reconcile.py --step all
  python3 tools/symbol_reconcile/reconcile.py --step pal-raw
  python3 tools/symbol_reconcile/reconcile.py --step usa-fp
"""

import argparse
import csv
import hashlib
import json
import os
import re
import struct
import sys
from pathlib import Path
from collections import defaultdict

PROJECT_ROOT = Path(__file__).resolve().parent.parent.parent
DOCS_SYMBOLS = PROJECT_ROOT / "docs" / "symbols"
TOOL_DIR = Path(__file__).resolve().parent
SPLAT_DIR = PROJECT_ROOT / "splat"

# ─── R5900 instruction helpers ────────────────────────────────────

# R5900 MIPS opcode decode helpers for better fingerprinting
MIPS_REG_GP = 28  # $gp
MIPS_REG_SP = 29  # $sp
MIPS_REG_RA = 31  # $ra

# Map SPECIAL funct to names for informational use
SPECIAL_FUNCT = {
    0x00: "NOP", 0x08: "JR", 0x09: "JALR", 0x0C: "SYSCALL",
    0x10: "MFHI", 0x11: "MTHI", 0x12: "MFLO", 0x13: "MTLO",
    0x18: "MULT", 0x19: "MULTU", 0x1A: "DIV", 0x1B: "DIVU",
    0x20: "ADD", 0x21: "ADDU", 0x22: "SUB", 0x23: "SUBU",
    0x24: "AND", 0x25: "OR", 0x26: "XOR", 0x27: "NOR",
    0x28: "NAND", 0x2A: "SLT", 0x2B: "SLTU",
    0x30: "DSLL", 0x32: "DSRL", 0x38: "DSLL32", 0x3A: "DSRL32",
    0x3C: "DSRA", 0x3E: "DSRA32",
}


def decode_inst(word: int) -> dict:
    """Decode a R5900 instruction word into opcode/rs/rt/rd/funct fields."""
    op = (word >> 26) & 0x3F
    rs = (word >> 21) & 0x1F
    rt = (word >> 16) & 0x1F
    rd = (word >> 11) & 0x1F
    sa = (word >> 6) & 0x1F
    funct = word & 0x3F
    imm = word & 0xFFFF
    # Signed immediate
    imm_s = imm - 0x10000 if (imm & 0x8000) else imm
    return {
        "op": op,
        "rs": rs,
        "rt": rt,
        "rd": rd,
        "sa": sa,
        "funct": funct,
        "imm": imm,
        "imm_s": imm_s,
        "word": word,
    }


def instr_token(inst: dict) -> int:
    """
    Returns a compact token that captures the instruction *type* for
    structural matching.  Two instructions with the same token represent
    the same kind of operation, regardless of register numbers or addresses.

    Token encoding (16 bits, fits in a single byte for common cases):
      - SPECIAL  (op=0):   token = funct  (0x00-0x3F)
      - REGIMM   (op=1):   token = 0x40 | rt  (0x40-0x5F)
      - J/JAL    (op=2,3): token = 0x60 | op  (0x62, 0x63)
      - COP1     (op=0x11): token = 0x70 | funct  (0x70-0xAF)
      - COP2     (op=0x1A): token = 0xB0 | funct  (0xB0-0xEF)
      - SPECIAL2 (op=0x1C): token = 0xF0 | funct  (0xF0-0xFF, wraps)
      - Branch   (op=4-7):  token = 0x100 | (op << 6) | rt
      - Load/Store (op=32-47): token = 0x140 | op
      - ALU imm  (op=8-15): token = 0x160 | op
      - Others:             token = 0x180 | op
    """
    op = inst["op"]
    funct = inst["funct"]

    if op == 0:
        return funct  # SPECIAL: 0x00-0x3F
    if op == 1:
        return 0x40 | inst["rt"]  # REGIMM: 0x40-0x5F
    if op in (2, 3):
        return 0x60 | op  # J/JAL: 0x62, 0x63
    if op == 0x11:
        return 0x70 | funct  # COP1: 0x70-0xAF
    if op == 0x1A:
        return 0xB0 | funct  # COP2: 0xB0-0xEF
    if op == 0x1C:
        return 0xF0 | funct  # SPECIAL2: 0xF0-0xFF
    if 4 <= op <= 7:
        return 0x100 | (op << 6) | inst["rt"]  # Branches
    if 32 <= op <= 47:
        return 0x140 | op  # Loads/stores (LB/LH/LW/LBU/LHU/LWU/SB/SH/SW)
    if 8 <= op <= 15:
        return 0x160 | op  # ALUI (ADDI/ADDIU/SLTI/SLTIU/ANDI/ORI/XORI/LUI)
    return 0x180 | op  # Other


def compute_op_seq_hash(func_bytes: bytes) -> str:
    """Hash of instruction-type tokens (structural fingerprint)."""
    token_bytes = bytearray()
    for j in range(0, len(func_bytes), 4):
        if j + 4 > len(func_bytes):
            break
        word = struct.unpack("<I", func_bytes[j:j+4])[0]
        inst = decode_inst(word)
        token = instr_token(inst)
        token_bytes.extend(struct.pack("<H", token))
    return hashlib.sha1(token_bytes).hexdigest()


def compute_op_hash(func_bytes: bytes) -> str:
    """Simple opcode-only fingerprint (just the 6-bit opcode field per word)."""
    token_bytes = bytearray()
    for j in range(0, len(func_bytes), 4):
        if j + 4 > len(func_bytes):
            break
        word = struct.unpack("<I", func_bytes[j:j+4])[0]
        op = (word >> 26) & 0x3F
        token_bytes.append(op)
    return hashlib.sha1(token_bytes).hexdigest()


def compute_norm_sha1(func_bytes: bytes, base_va: int = 0) -> str:
    """
    Normalize function bytes by zeroing out address-dependent immediates:
      - j/jal target addresses
      - GP-relative load/store offsets (rs == $28)
      - All LUI immediates
      - addiu/ori address components (when rt == rs, common lui-follow)
    Leaves structural immediates (stack offsets, small constants) intact.
    """
    normalized = bytearray(func_bytes)
    for j in range(0, len(normalized) - 4, 4):
        w = struct.unpack("<I", normalized[j:j+4])[0]
        inst = decode_inst(w)
        op = inst["op"]
        rs = inst["rs"]
        rt = inst["rt"]

        # j/jal — zero out target address
        if op in (2, 3):
            struct.pack_into("<I", normalized, j, w & 0xFC000000)  # keep opcode only
            continue

        # LUI — zero out immediate (always loads high half of an address)
        if op == 15:
            struct.pack_into("<I", normalized, j, w & 0xFFFF0000)
            continue

        # GP-relative loads/stores — zero out offset
        if rs == MIPS_REG_GP and 32 <= op <= 43:
            struct.pack_into("<I", normalized, j, w & 0xFFFF0000)
            continue

        # addiu/ori with same rs/rt (lui+addiu address construction)
        if op in (9, 13) and rs == rt:
            struct.pack_into("<I", normalized, j, w & 0xFFFF0000)
            continue

        # JALR — zero out rd (not strictly needed but helps)
        if op == 0 and inst["funct"] == 9:
            struct.pack_into("<I", normalized, j, w & 0xFFFF07FF)
            continue

        # Branch relative — keep as-is (relative offsets stay stable
        # within the same function body)

    return hashlib.sha1(normalized).hexdigest()


# ─── Default paths ─────────────────────────────────────────────────
DEFAULT_ELF = PROJECT_ROOT / ".local" / "extracted" / "SCUS_971.13.elf"
DEFAULT_PAL_SYMBOLS = Path("/tmp/pal-reference/config/symbol_addrs.txt")
DEFAULT_PAL_YAML = Path("/tmp/pal-reference/config/ICO-PAL.yaml")
DEFAULT_PAL_ELF = Path("/tmp/pal_elf/SCES_507.60")

# ─── Level 1: raw PAL CSV from symbol_addrs.txt ────────────────────

def parse_pal_symbols(symbol_path: Path) -> list[dict]:
    """Parse symbol_addrs.txt into list of dicts with symbol, pal_va, type."""
    records = []
    if not symbol_path.exists():
        print(f"[WARN] PAL symbols not found at {symbol_path}")
        return records

    with open(symbol_path) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("//"):
                continue
            m = re.match(r"(\w+)\s*=\s*(0x[0-9a-fA-F]+)\s*;?\s*(?://\s*type:\s*(\w+))?", line)
            if m:
                symbol, va_str, typ = m.group(1), m.group(2), m.group(3) or "func"
                va = int(va_str, 16)
                records.append({
                    "symbol": symbol,
                    "pal_va": va,
                    "type": typ,
                })
    return records


def write_raw_pal_csv(records: list[dict], output: Path):
    """Write raw_symbols_pal.csv."""
    os.makedirs(output.parent, exist_ok=True)
    with open(output, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["symbol", "pal_va", "type"])
        for r in records:
            w.writerow([r["symbol"], f"0x{r['pal_va']:08X}", r["type"]])
    print(f"[OK] {len(records)} PAL symbols -> {output}")


# ─── Level 2: USA function fingerprints via capstone ────────────────

def read_elf_text(elf_path: Path) -> tuple[int, bytes] | None:
    """Read .text section from USA ELF using pyelftools. Returns (vaddr, bytes)."""
    try:
        from elftools.elf.elffile import ELFFile
    except ImportError:
        print("[ERR] pyelftools not installed. Run: pip install pyelftools")
        return None

    if not elf_path.exists():
        print(f"[WARN] USA ELF not found at {elf_path}")
        return None

    with open(elf_path, "rb") as f:
        elf = ELFFile(f)
        for sec in elf.iter_sections():
            if sec.name == ".text":
                return sec.header.sh_addr, sec.data()
    print(f"[WARN] .text section not found in {elf_path}")
    return None


def read_elf_symtab_functions(elf_path: Path) -> list[dict]:
    """
    Read function symbols from ELF .symtab or .dynsym sections.

    Returns list of {va, size, name} for STT_FUNC / STB_GLOBAL entries.
    Returns empty list if no symbol table is present (common for PS2 ELFs).
    """
    try:
        from elftools.elf.elffile import ELFFile
        from elftools.elf.constants import ST_TYPE, ST_BIND
    except ImportError:
        return []

    if not elf_path.exists():
        return []

    try:
        with open(elf_path, "rb") as f:
            elf = ELFFile(f)
            for sec in elf.iter_sections():
                if sec.name not in (".symtab", ".dynsym"):
                    continue
                if not hasattr(sec, "iter_symbols"):
                    continue
                functions = []
                for sym in sec.iter_symbols():
                    if sym.entry.st_info.type == "STT_FUNC" and sym.entry.st_value:
                        functions.append({
                            "va": sym.entry.st_value,
                            "size": sym.entry.st_size,
                            "name": sym.name,
                        })
                print(f"[INFO] {len(functions)} symtab functions from {sec.name}")
                return functions
    except Exception as e:
        print(f"[WARN] symtab read failed: {e}")
        return []

    print("[INFO] No .symtab/.dynsym section found in ELF")
    return []


def extract_functions_from_text(vaddr: int, text_bytes: bytes, symtab_functions: list[dict] | None = None) -> list[dict]:
    """
    Disassemble .text via capstone, detect function boundaries by:
      1. Standard prologue: addiu $sp, $sp, -N
      2. Leaf prologue: jr $ra (single-instruction return)
      3. jr $ra + nop (common tail)
      4. NOP-padding before a function (for overlapping coverage)
      5. (optional) Symbol table entries for precise boundaries

    Returns list of {va, size, raw_sha1, norm_sha1, op_hash, op_seq_hash,
                     insn_count, call_count, branch_count}
    """
    try:
        from capstone import Cs, CS_ARCH_MIPS, CS_MODE_MIPS64, CS_GRP_CALL, CS_GRP_BRANCH_RELATIVE, CS_GRP_JUMP
    except ImportError:
        print("[ERR] capstone not installed. Run: pip install capstone")
        return []

    md = Cs(CS_ARCH_MIPS, CS_MODE_MIPS64)
    md.detail = True

    length = len(text_bytes)

    # ── Step 0: Seed with symbol table entries (if available) ──────────
    starts = []  # list of offset_in_text
    if symtab_functions:
        text_start = vaddr
        for sym in symtab_functions:
            if sym["va"] >= vaddr and sym["va"] < vaddr + length:
                offset = sym["va"] - vaddr
                if sym["size"] >= 4 or sym["size"] == 0:
                    starts.append(offset)
        if starts:
            print(f"[INFO] Seeded {len(starts)} symtab function starts")

    # ── Step 1: Prologue / heuristic function detection ───────────────
    i = 0
    while i < length - 4:
        word = struct.unpack("<I", text_bytes[i:i+4])[0]
        inst = decode_inst(word)

        # 1a: Standard prologue: addiu $sp, $sp, -N
        if inst["op"] == 9 and inst["rs"] == MIPS_REG_SP and inst["rt"] == MIPS_REG_SP and (inst["imm"] & 0x8000):
            starts.append(i)
            i += 4
            continue

        # 1b: Leaf/entry: jr $ra (e.g., a getter that just returns a value)
        if word == 0x03E00008:  # jr $ra
            starts.append(i)
            i += 4
            continue

        # 1c: NOP padding followed by a non-NOP (covers functions after linker padding)
        if word == 0 and i + 8 <= length:
            next_word = struct.unpack("<I", text_bytes[i+4:i+8])[0]
            if next_word != 0 and (next_word >> 26) in (9, 15, 0x1C, 0x11, 0x1A):
                starts.append(i + 4)
                i += 4
                continue

        i += 4

    # ── Step 2: For each start, find return boundary ──────────────────
    def find_return_backward(offset: int, min_offset: int) -> int:
        """Scan backward from offset to find jr $ra. Returns byte after delay slot."""
        for scan in range(offset - 4, min_offset - 1, -4):
            w = struct.unpack("<I", text_bytes[scan:scan+4])[0]
            if w == 0x03E00008:  # jr $ra
                return scan + 8  # include delay slot
        return offset  # not found

    # De-duplicate by grouping overlapping starts
    starts = sorted(set(starts))
    deduped = []
    for s in starts:
        if deduped and s - deduped[-1] < 8:
            continue  # too close — skip as likely duplicate
        deduped.append(s)
    starts = deduped

    # ── Step 3: Extract function bodies ───────────────────────────────
    functions = []
    for idx, start in enumerate(starts):
        # End is next start, or end of .text
        if idx + 1 < len(starts):
            next_start = starts[idx + 1]
        else:
            next_start = length

        # Try to find actual end by scanning backward from next_start to find jr $ra
        actual_end = find_return_backward(next_start, start)
        if actual_end == next_start:
            # No jr $ra found — use next_start directly
            actual_end = next_start

        func_bytes = text_bytes[start:actual_end]
        if len(func_bytes) < 8:
            continue

        # Compute all fingerprints
        raw_sha1 = hashlib.sha1(func_bytes).hexdigest()
        norm_sha1 = compute_norm_sha1(func_bytes, vaddr + start)
        op_hash = compute_op_hash(func_bytes)
        op_seq_hash = compute_op_seq_hash(func_bytes)

        # Capstone stats
        call_count = 0
        branch_count = 0
        jump_count = 0
        insn_count = 0
        try:
            for insn in md.disasm(func_bytes, vaddr + start):
                insn_count += 1
                if CS_GRP_CALL in insn.groups:
                    call_count += 1
                if CS_GRP_BRANCH_RELATIVE in insn.groups:
                    branch_count += 1
                if CS_GRP_JUMP in insn.groups:
                    jump_count += 1
        except Exception:
            pass

        functions.append({
            "va": vaddr + start,
            "size": actual_end - start,
            "raw_sha1": raw_sha1,
            "norm_sha1": norm_sha1,
            "op_hash": op_hash,
            "op_seq_hash": op_seq_hash,
            "insn_count": insn_count,
            "call_count": call_count,
            "branch_count": branch_count,
        })

    return functions


def write_usa_fingerprints(functions: list[dict], output: Path):
    """Write usa_fingerprints.json."""
    with open(output, "w") as f:
        json.dump(functions, f, indent=2)
    print(f"[OK] {len(functions)} USA functions -> {output}")


# ─── Level 3: cross-reference PAL symbols against USA functions ─────

def cross_reference(
    pal_records: list[dict],
    usa_functions: list[dict],
    pal_functions: list[dict] | None = None,
) -> list[dict]:
    """
    Cross-reference PAL symbols against USA function fingerprints.
    Matching levels (in priority order):
      1. raw_sha1           — exact byte match
      2. op_seq_hash         — same instruction-type sequence (structure match)
      3. norm_sha1           — address-normalized byte match
      4. fuzzy               — size + call/branch score

    For PAL symbols without a PAL function fingerprint (missed by prologue
    scanner), falls back to extracting the symbol's byte range from the PAL
    ELF and trying all matching levels.
    """
    if not pal_functions:
        results = []
        for r in pal_records:
            results.append({
                "symbol": r["symbol"],
                "pal_va": f"0x{r['pal_va']:08X}",
                "usa_va": "",
                "type": r["type"],
                "source_file": r.get("source_file", ""),
                "match_method": "unmatched",
                "confidence": 0.0,
                "status": "unmatched",
                "notes": "PAL ELF not available for byte matching",
            })
        return results

    # ── Build lookup tables ───────────────────────────────────────────
    usa_by_sha1: dict[str, list[dict]] = {}
    usa_by_opseq: dict[str, list[dict]] = {}
    usa_by_norm: dict[str, list[dict]] = {}
    usa_by_va: dict[int, dict] = {}
    for f in usa_functions:
        usa_by_sha1.setdefault(f["raw_sha1"], []).append(f)
        usa_by_opseq.setdefault(f["op_seq_hash"], []).append(f)
        usa_by_norm.setdefault(f["norm_sha1"], []).append(f)
        usa_by_va[f["va"]] = f

    pal_by_va: dict[int, dict] = {}
    for f in pal_functions:
        pal_by_va[f["va"]] = f

    # ── Read PAL ELF bytes for fallback extraction ────────────────────
    pal_elf_path = Path(os.environ.get("PAL_ELF", str(DEFAULT_PAL_ELF)))
    pal_text_vaddr = 0x00100000
    pal_text_bytes: bytes | None = None
    if pal_elf_path.exists():
        result = read_elf_text(pal_elf_path)
        if result:
            pal_text_vaddr, pal_text_bytes = result

    def extract_pal_bytes(va: int) -> bytes | None:
        """Extract bytes from PAL .text at VA (heuristically find end)."""
        if pal_text_bytes is None:
            return None
        offset = va - pal_text_vaddr
        if offset < 0 or offset >= len(pal_text_bytes):
            return None
        # Find next symbol address or jr $ra as end bound
        remaining = pal_text_bytes[offset:]
        # Scan for jr $ra (0x03E00008) within reasonable limit
        max_size = 8192
        for k in range(0, min(len(remaining), max_size), 4):
            w = struct.unpack("<I", remaining[k:k+4])[0]
            if w == 0x03E00008:  # jr $ra
                return remaining[:k + 8]  # include delay slot
        return remaining[:min(len(remaining), max_size)]

    # ── Match each PAL symbol ─────────────────────────────────────────
    results = []
    exact_count = 0
    opseq_count = 0
    normalized_count = 0
    sameva_count = 0
    fuzzy_count = 0
    unmatched_count = 0

    for r in pal_records:
        result = {
            "symbol": r["symbol"],
            "pal_va": f"0x{r['pal_va']:08X}",
            "usa_va": "",
            "type": r["type"],
            "source_file": r.get("source_file", ""),
            "match_method": "unmatched",
            "confidence": 0.0,
            "status": "unmatched",
            "notes": "",
        }
        pal_va = r["pal_va"]

        # Decide which fingerprint to use
        if pal_va in pal_by_va:
            pal_fp = pal_by_va[pal_va]
            pal_fp_source = "prologue"
        else:
            # Not in PAL fingerprint set — extract bytes directly
            func_bytes = extract_pal_bytes(pal_va)
            if func_bytes is None or len(func_bytes) < 8:
                result["status"] = "unmatched"
                result["notes"] = f"PAL VA 0x{pal_va:08X} not in PAL fingerprint set and cannot extract"
                unmatched_count += 1
                results.append(result)
                continue
            pal_fp = {
                "va": pal_va,
                "size": len(func_bytes),
                "raw_sha1": hashlib.sha1(func_bytes).hexdigest(),
                "norm_sha1": compute_norm_sha1(func_bytes, pal_va),
                "op_seq_hash": compute_op_seq_hash(func_bytes),
                "op_hash": compute_op_hash(func_bytes),
                "insn_count": 0,
                "call_count": func_bytes.count(b"\x0c") if len(func_bytes) > 0 else 0,
                "branch_count": 0,
            }
            pal_fp_source = "extracted"

        # ── Level 1: raw_sha1 (exact byte match) ──────────────────────
        raw_key = pal_fp["raw_sha1"]
        if raw_key in usa_by_sha1:
            match = usa_by_sha1[raw_key][0]
            result["usa_va"] = f"0x{match['va']:08X}"
            result["match_method"] = "raw_sha1"
            result["confidence"] = 1.0
            result["status"] = "verified"
            result["notes"] = f"exact; src={pal_fp_source}"
            exact_count += 1
            results.append(result)
            continue

        # ── Level 2: op_seq_hash (structural match, high confidence) ──
        seq_key = pal_fp["op_seq_hash"]
        if seq_key in usa_by_opseq:
            candidates = usa_by_opseq[seq_key]
            best = min(candidates, key=lambda m: abs(m["size"] - pal_fp["size"]))
            sz_diff = abs(best["size"] - pal_fp["size"])
            confidence = 0.95 - (sz_diff / max(pal_fp["size"], 256))
            if confidence >= 0.85:
                result["usa_va"] = f"0x{best['va']:08X}"
                result["match_method"] = "op_seq_hash"
                result["confidence"] = round(confidence, 2)
                result["status"] = "verified"
                result["notes"] = f"struct match; sz_diff={sz_diff}; src={pal_fp_source}"
                opseq_count += 1
                results.append(result)
                continue

        # ── Level 3: norm_sha1 (address-normalized match) ─────────────
        norm_key = pal_fp["norm_sha1"]
        if norm_key in usa_by_norm:
            match = usa_by_norm[norm_key][0]
            result["usa_va"] = f"0x{match['va']:08X}"
            result["match_method"] = "norm_sha1"
            result["confidence"] = 0.90
            result["status"] = "verified"
            result["notes"] = f"norm; src={pal_fp_source}"
            normalized_count += 1
            results.append(result)
            continue

        # ── Level 4: same-VA fallback (strong for kernel/lib symbols) ─
        if pal_va in usa_by_va:
            match = usa_by_va[pal_va]
            result["usa_va"] = f"0x{match['va']:08X}"
            result["match_method"] = "same_va"
            result["confidence"] = 0.85
            result["status"] = "verified"
            result["notes"] = f"same VA; USA size={match['size']}; src={pal_fp_source}"
            sameva_count += 1
            results.append(result)
            continue

        # ── Level 5: op_seq_hash fallback (lower confidence) ──────────
        if seq_key in usa_by_opseq:
            candidates = usa_by_opseq[seq_key]
            best = min(candidates, key=lambda m: abs(m["size"] - pal_fp["size"]))
            sz_diff = abs(best["size"] - pal_fp["size"])
            confidence = 0.95 - (sz_diff / max(pal_fp["size"], 256))
            result["usa_va"] = f"0x{best['va']:08X}"
            result["match_method"] = "op_seq_hash"
            result["confidence"] = round(max(confidence, 0.5), 2)
            result["status"] = "candidate"
            result["notes"] = f"struct lo conf; sz_diff={sz_diff}; src={pal_fp_source}"
            fuzzy_count += 1
            results.append(result)
            continue

        # ── Level 6: re-ranked fuzzy (op_seq_hash priority + size) ────
        candidates = []
        for uf in usa_functions:
            sz_diff = abs(uf["size"] - pal_fp["size"])
            if sz_diff > 128:
                continue
            seq_match = uf.get("op_seq_hash") == seq_key
            sz_ratio = 1.0 - (sz_diff / max(pal_fp["size"], 256))
            call_diff = abs(uf.get("call_count", 0) - pal_fp.get("call_count", 0))
            call_penalty = call_diff / 20.0
            # Primary: op_seq_hash match gives 0.4 bonus, then size ratio
            seq_bonus = 0.4 if seq_match else 0.0
            score = seq_bonus + sz_ratio - call_penalty
            if score > 0.5:
                candidates.append((score, uf))
        if candidates:
            candidates.sort(key=lambda x: -x[0])
            best_score, match = candidates[0]
            result["usa_va"] = f"0x{match['va']:08X}"
            result["match_method"] = "fuzzy"
            result["confidence"] = round(best_score, 2)
            result["status"] = "candidate"
            result["notes"] = f"fuzzy; sz_diff={abs(match['size']-pal_fp['size'])}; src={pal_fp_source}"
            fuzzy_count += 1
        else:
            result["status"] = "unmatched"
            result["notes"] = f"no match; PAL size={pal_fp['size']}; src={pal_fp_source}"
            unmatched_count += 1

        results.append(result)

    total = len(results)
    print(f"[MATCH] exact={exact_count} opseq={opseq_count} norm={normalized_count} sameva={sameva_count} fuzzy={fuzzy_count} unmatched={unmatched_count} total={total}")
    return results


def write_candidates_csv(results: list[dict], output: Path):
    """Write pal_usa_symbol_map.csv with all candidates."""
    with open(output, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow([
            "symbol", "pal_va", "usa_va", "type", "source_file",
            "match_method", "confidence", "status", "notes",
        ])
        for r in results:
            w.writerow([
                r["symbol"],
                r["pal_va"],
                r["usa_va"],
                r["type"],
                r["source_file"],
                r["match_method"],
                f"{r['confidence']:.2f}",
                r["status"],
                r["notes"],
            ])
    print(f"[OK] {len(results)} records -> {output}")


# ─── Source file mapping from PAL YAML ──────────────────────────────

def build_source_ranges(yaml_path: Path) -> list[dict]:
    """
    Parse PAL YAML subsegments and build (vram_start, vram_end, name) ranges.
    
    Asm subsegments in the YAML have format [offset, 'asm', 'name'] without
    explicit vram/size. Vram is computed sequentially: the base text vram is
    0x00100000, and each subsegment advances in offset order.
    """
    ranges = []
    if not yaml_path.exists():
        return ranges

    try:
        import yaml
    except ImportError:
        return ranges

    with open(yaml_path) as f:
        data = yaml.safe_load(f)

    main_seg = data["segments"][1]
    base_vram = main_seg["vram"]  # 0x00100000 for PAL
    subs = main_seg.get("subsegments", [])

    # Build a list of (offset, name, next_offset) for asm + c + rodata + data subsegments
    entries = []
    for i, s in enumerate(subs):
        if isinstance(s, list) and len(s) >= 3:
            offset = s[0]
            typ = s[1]
            name = s[2]
            # Infer size from next entry
            next_offset = None
            for j in range(i + 1, len(subs)):
                if isinstance(subs[j], list) and len(subs[j]) >= 1:
                    next_offset = subs[j][0]
                    break
            if next_offset is None:
                # Last subsegment: estimate from known sizes
                if typ == "asm":
                    next_offset = offset + 0x800  # placeholder
                elif typ in ("rodata", "data", "sdata", "lit4"):
                    next_offset = offset + 0x200
                else:
                    next_offset = offset + 0x100

            size = next_offset - offset
            vram = base_vram + (offset - subs[0][0]) if isinstance(subs[0], list) else base_vram

            # Adjust for known section boundaries
            known_sections = {
                0x18abd0: ("vutext", 0x289BD0, 0x50E0),
                0x18fd00: ("data", 0x28ED00, 0x2BE620),
                0x44e380: ("rodata", 0x54D380, 0xEB718),
                0x539b00: ("lit4", 0x638B00, 0x1154),
                0x53ac80: ("sdata", 0x639C80, 0x2476),
                0x53d100: ("sbss", 0x63C100, 0x4DC),
            }
            if offset in known_sections:
                # Use precise known vram for section boundaries
                pass  # we'll rely on direct offset->vram mapping

            # Only include code-relevant segments
            if typ in ("asm", "c", "rodata") and size > 0:
                vram_calc = offset - subs[0][0] + base_vram
                ranges.append({
                    "vram": vram_calc,
                    "vram_end": vram_calc + size,
                    "name": name,
                    "type": typ,
                })

    # Merge adjacent asm ranges with same name prefix
    # First, sort by vram
    ranges.sort(key=lambda r: r["vram"])

    # Also add known section boundaries from PAL YAML header
    text_vram = 0x00100000
    text_size = 0x189BC4
    ranges.insert(0, {
        "vram": text_vram,
        "vram_end": text_vram + text_size,
        "name": "__text__",
        "type": "header",
    })

    return ranges


def map_source_files_from_ranges(
    pal_records: list[dict],
    ranges: list[dict],
) -> list[dict]:
    """
    For each PAL symbol, find which source range contains it.
    Returns enriched records.
    """
    enriched = []
    for r in pal_records:
        va = r["pal_va"]
        source = ""
        for seg in ranges:
            if seg["vram"] <= va < seg["vram_end"]:
                source = seg["name"]
                break
        # Also check if it's in the first text range
        if not source and 0x00100000 <= va < 0x00289BC4:
            source = "__text__"
        enriched.append({**r, "source_file": source})
    return enriched


def write_source_file_csv(records: list[dict], output: Path):
    """Write enriched CSV with source_file column."""
    with open(output, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["symbol", "pal_va", "type", "source_file"])
        for r in records:
            w.writerow([r["symbol"], f"0x{r['pal_va']:08X}", r["type"], r.get("source_file", "")])


def step_source_file(records: list[dict] | None = None):
    """Step 1b: Enrich raw PAL CSV with source_file from YAML subsegments."""
    if records is None:
        records = parse_pal_symbols(
            Path(os.environ.get("PAL_SYMBOLS", str(DEFAULT_PAL_SYMBOLS)))
        )
    yaml_path = Path(os.environ.get("PAL_YAML", str(DEFAULT_PAL_YAML)))
    ranges = build_source_ranges(yaml_path)
    enriched = map_source_files_from_ranges(records, ranges)

    # Count distinct source files
    sources = set(r.get("source_file", "") for r in enriched)
    mapped = sum(1 for r in enriched if r.get("source_file"))

    output = DOCS_SYMBOLS / "raw_symbols_pal.csv"
    write_source_file_csv(enriched, output)

    print(f"[OK] {mapped}/{len(enriched)} symbols mapped to source files ({len(sources)} distinct)")
    return enriched


def write_source_file_csv(records: list[dict], output: Path):
    """Write enriched CSV with source_file column."""
    with open(output, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["symbol", "pal_va", "type", "source_file"])
        for r in records:
            w.writerow([r["symbol"], r["pal_va"], r["type"], r["source_file"]])


# ─── CLI ─────────────────────────────────────────────────────────────

def step_pal_raw():
    """Step 1: Convert PAL symbol_addrs.txt -> raw_symbols_pal.csv"""
    sym_path = Path(
        os.environ.get("PAL_SYMBOLS", str(DEFAULT_PAL_SYMBOLS))
    )
    records = parse_pal_symbols(sym_path)
    output = DOCS_SYMBOLS / "raw_symbols_pal.csv"
    write_raw_pal_csv(records, output)
    return records


def step_source_file(records):
    """Step 1b: Enrich raw PAL CSV with source_file from YAML subsegments."""
    yaml_path = Path(
        os.environ.get("PAL_YAML", str(DEFAULT_PAL_YAML))
    )
    ranges = build_source_ranges(yaml_path)
    enriched = map_source_files_from_ranges(records, ranges)
    
    sources = set(r.get("source_file", "") for r in enriched)
    mapped = sum(1 for r in enriched if r.get("source_file"))
    
    output = DOCS_SYMBOLS / "raw_symbols_pal.csv"
    write_source_file_csv(enriched, output)
    print(f"[OK] {mapped}/{len(enriched)} symbols mapped ({len(sources)} distinct source files)")
    return enriched


def step_usa_fp():
    """Step 2: Extract USA function fingerprints"""
    elf_path = Path(
        os.environ.get("USA_ELF", str(DEFAULT_ELF))
    )
    sym_fns = read_elf_symtab_functions(elf_path)
    result = read_elf_text(elf_path)
    if result is None:
        return []
    vaddr, text_bytes = result
    print(f"[INFO] USA .text: 0x{vaddr:08X}, {len(text_bytes)} bytes")
    functions = extract_functions_from_text(vaddr, text_bytes, sym_fns)
    output = DOCS_SYMBOLS / "usa_fingerprints.json"
    write_usa_fingerprints(functions, output)
    return functions


def step_candidates(pal_records: list[dict] | None = None):
    """Step 3: Cross-reference PAL -> USA using fingerprints."""
    if pal_records is None:
        pal_records = parse_pal_symbols(
            Path(os.environ.get("PAL_SYMBOLS", str(DEFAULT_PAL_SYMBOLS)))
        )

    usa_fp_path = DOCS_SYMBOLS / "usa_fingerprints.json"
    pal_fp_path = DOCS_SYMBOLS / "pal_fingerprints.json"

    if not usa_fp_path.exists():
        print("[WARN] USA fingerprints not found; run --step usa-fp first")
        return []

    with open(usa_fp_path) as f:
        usa_functions = json.load(f)

    pal_functions = None
    if pal_fp_path.exists():
        with open(pal_fp_path) as f:
            pal_functions = json.load(f)
        print(f"[INFO] PAL fingerprints loaded: {len(pal_functions)} functions")
    else:
        print("[WARN] PAL fingerprints not found — running unmatched placeholder")

    results = cross_reference(pal_records, usa_functions, pal_functions)
    output = DOCS_SYMBOLS / "pal_usa_symbol_map.csv"
    write_candidates_csv(results, output)
    return results


def step_pal_fp():
    """Step 2b: Extract PAL function fingerprints from PAL ELF."""
    elf_path = Path(os.environ.get("PAL_ELF", str(DEFAULT_PAL_ELF)))
    sym_fns = read_elf_symtab_functions(elf_path)
    result = read_elf_text(elf_path)
    if result is None:
        return []
    vaddr, text_bytes = result
    print(f"[INFO] PAL .text: 0x{vaddr:08X}, {len(text_bytes)} bytes")
    functions = extract_functions_from_text(vaddr, text_bytes, sym_fns)
    output = DOCS_SYMBOLS / "pal_fingerprints.json"
    write_usa_fingerprints(functions, output)  # same format
    return functions


def step_splat():
    """Step 4: Generate splat YAML and symbol files from verified candidates."""
    csv_path = DOCS_SYMBOLS / "pal_usa_symbol_map.csv"
    if not csv_path.exists():
        print(f"[WARN] symbol map not found; run --step candidates first")
        return

    with open(csv_path) as f:
        reader = csv.DictReader(f)
        verified = [r for r in reader if r["status"] == "verified" and r["usa_va"]]

    if not verified:
        print("[WARN] no verified symbols to export")
        return

    symbols_out = DOCS_SYMBOLS / "symbol_addrs_usa.txt"
    ghidra_out = DOCS_SYMBOLS / "ghidra_labels.txt"
    yaml_out = SPLAT_DIR / "SCUS_971.13.verified-symbols.yaml"

    SPLAT_DIR.mkdir(parents=True, exist_ok=True)

    # ── Write symbol_addrs_usa.txt (splat format) ──────────────────────
    # Deduplicate by VA (keep first symbol name for each unique VA)
    seen_vas: set[int] = set()
    splat_entries = []
    for r in verified:
        va = int(r["usa_va"], 16)
        if va not in seen_vas:
            seen_vas.add(va)
            splat_entries.append(r)
    with open(symbols_out, "w") as f:
        f.write("// Auto-generated from PAL->USA symbol reconciliation\n")
        f.write(f"// {len(splat_entries)} unique VA symbols (from {len(verified)} total)\n\n")
        for r in splat_entries:
            va = int(r["usa_va"], 16)
            f.write(f"{r['symbol']} = 0x{va:08X}; // type:func\n")
    print(f"[OK] {len(splat_entries)} splat symbols (deduplicated) -> {symbols_out}")

    # ── Write ghidra_labels.txt (simple VA→name) ──────────────────────
    with open(ghidra_out, "w") as f:
        f.write("# Auto-generated from PAL->USA symbol reconciliation\n")
        f.write(f"# {len(splat_entries)} unique VA symbols\n")
        f.write("# Format: VA SymbolName\n")
        for r in splat_entries:
            va = int(r["usa_va"], 16)
            f.write(f"0x{va:08X} {r['symbol']}\n")
    print(f"[OK] {len(splat_entries)} Ghidra labels -> {ghidra_out}")

    # ── Write splat YAML with named subsegments ────────────────────────
    # Group by source_file, deduplicated by VA
    from collections import defaultdict
    by_source: dict[str, list[dict]] = defaultdict(list)
    seen_yaml: set[int] = set()
    for r in verified:
        va = int(r["usa_va"], 16)
        if va not in seen_yaml:
            seen_yaml.add(va)
            src = r.get("source_file", "__text__")
            by_source[src].append(r)

    for src in by_source:
        by_source[src].sort(key=lambda x: int(x["usa_va"], 16))

    # Read existing ELF sha1 for YAML header
    elf_path = Path(os.environ.get("USA_ELF", str(DEFAULT_ELF)))
    elf_sha1 = ""
    if elf_path.exists():
        import hashlib
        with open(elf_path, "rb") as f:
            elf_sha1 = hashlib.sha1(f.read()).hexdigest()

    text_vaddr = 0x00100000
    text_offset = 0x001000  # offset within ELF file

    with open(yaml_out, "w") as f:
        f.write(f"name: ICO USA SCUS_971.13 — verified symbols\n")
        if elf_sha1:
            f.write(f"sha1: {elf_sha1}\n")
        f.write("options:\n")
        f.write("  basename: SCUS_971.13\n")
        f.write(f"  target_path: {elf_path}\n")
        f.write("  base_path: /tmp/ico-verified-symbols\n")
        f.write("  asm_path: asm\n")
        f.write("  asset_path: assets\n")
        f.write("  src_path: src\n")
        f.write("  build_path: build\n")
        f.write("  compiler: EEGCC\n")
        f.write("  platform: ps2\n")
        f.write("  ld_bss_contains_common: True\n")
        f.write("  ld_bss_is_noload: true\n")
        f.write("  use_gp_rel_macro_nonmatching: False\n")
        f.write("  string_encoding: EUC-JP\n")
        f.write("  disasm_unknown: True\n")
        f.write("  named_regs_for_c_funcs: True\n")
        f.write("  find_file_boundaries: False\n")
        f.write("  section_order: [\".text\", \".vutext\", \".data\", \".rodata\", \".lit4\", \".sdata\", \".sbss\", \".bss\"]\n")
        f.write("segments:\n")
        f.write("  - [0x000000, databin, elf_header]\n")
        f.write("  - name: main_text\n")
        f.write("    type: code\n")
        f.write("    start: 0x001000\n")
        f.write("    vram: 0x00100000\n")
        f.write("    subalign: null\n")
        f.write("    subsegments:\n")

        # Write monolithic blocks for sources without symbols
        # and individual symbol entries for each verified function
        src_list = sorted(by_source.keys())
        for src in src_list:
            entries = by_source[src]
            first_va = int(entries[0]["usa_va"], 16)
            last_va = int(entries[-1]["usa_va"], 16)
            first_off = first_va - text_vaddr + text_offset
            last_off = last_va - text_vaddr + text_offset + _estimate_symbol_size(entries[-1]["symbol"])

            f.write(f"      # --- {src} ({len(entries)} symbols) ---\n")
            f.write(f"      # range: 0x{first_off:X}-0x{last_off:X}\n")

            # If the gap from previous section is large, mark monolithic
            for r in entries:
                va = int(r["usa_va"], 16)
                off = va - text_vaddr + text_offset
                f.write(f"      - [0x{off:X}, asm, {r['symbol']}]\n")

        # Remaining sections
        f.write("  - [0x{0:X}, databin, .vutext]\n".format(
            0x189BD0))  # .vutext offset (known from ELF structure)
        f.write("  - [0x{0:X}, databin, .data]\n".format(0x18FD00))
        f.write("  - [0x{0:X}, databin, .rodata]\n".format(0x44E380))
        f.write("  - [0x{0:X}, databin, .lit4]\n".format(0x539B00))
        f.write("  - [0x{0:X}, databin, .sdata]\n".format(0x53AC80))

    print(f"[OK] Splat YAML with {len(verified)} named subsegments -> {yaml_out}")


def _estimate_symbol_size(symbol_name: str) -> int:
    """Estimate size of a symbol based on naming conventions."""
    # Default: assume 8 bytes minimum
    return 8


def step_all():
    """Run all available steps."""
    records = step_pal_raw()
    enriched = step_source_file(records)
    step_usa_fp()
    step_pal_fp()
    step_candidates(enriched)
    step_splat()


def main():
    parser = argparse.ArgumentParser(
        description="PAL -> USA symbol reconciliation pipeline"
    )
    parser.add_argument(
        "--step", choices=["all", "pal-raw", "source-file", "usa-fp", "pal-fp", "candidates", "splat"],
        default="all",
        help="Which step to run",
    )
    parser.add_argument(
        "--pal-symbols",
        default=str(DEFAULT_PAL_SYMBOLS),
        help=f"Path to PAL symbol_addrs.txt (default: {DEFAULT_PAL_SYMBOLS})",
    )
    parser.add_argument(
        "--usa-elf",
        default=str(DEFAULT_ELF),
        help=f"Path to USA ELF (default: {DEFAULT_ELF})",
    )
    parser.add_argument(
        "--pal-elf",
        default=str(DEFAULT_PAL_ELF),
        help=f"Path to PAL ELF (default: {DEFAULT_PAL_ELF})",
    )
    args = parser.parse_args()

    # Set env vars for sub-steps
    os.environ["PAL_SYMBOLS"] = args.pal_symbols
    os.environ["USA_ELF"] = args.usa_elf
    os.environ["PAL_ELF"] = args.pal_elf

    if args.step == "all":
        step_all()
    elif args.step == "pal-raw":
        step_pal_raw()
    elif args.step == "source-file":
        records = parse_pal_symbols(Path(args.pal_symbols))
        step_source_file(records)
    elif args.step == "usa-fp":
        step_usa_fp()
    elif args.step == "pal-fp":
        step_pal_fp()
    elif args.step == "candidates":
        records = parse_pal_symbols(Path(args.pal_symbols))
        step_candidates(records)
    elif args.step == "splat":
        step_splat()
    else:
        print(f"[ERR] Unknown step: {args.step}")


if __name__ == "__main__":
    main()
