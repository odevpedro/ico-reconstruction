#!/usr/bin/env python3
"""Local ee-gcc 2.9 compilation + scoring pipeline.

Compiles a C function with ee-gcc 2.9-991111-01 (MIPS-III, EABI, -O2),
disassembles it, and scores against the target ELF function.

Usage:
    python3 tools/ee_gcc_compile.py path/to/file.c --fn func_name --va 0xADDRESS
"""
import argparse
import subprocess
import struct
import sys
import os
import tempfile
import re

import capstone

# === Configuration ===
SRC_ROOT = os.path.expanduser(
    "~/Documentos/repos/ico-reconstruction/src"
)
TOOLCHAIN = os.path.expanduser(
    "~/Documentos/repos/ico-reconstruction/toolchain/ee-gcc2.9-991111-01/bin/ee-gcc"
)
ELF_PATH = os.path.expanduser(
    "/home/peter/Documentos/repos/ico-reconstruction/.local/extracted/SCUS_971.13.elf"
)
CC_FLAGS = [
    "-mips3", "-mgp64", "-mabi=eabi", "-msingle-float",
    "-G0", "-O2",
] + [f"-I{SRC_ROOT}", f"-I{SRC_ROOT}/entity", f"-I{SRC_ROOT}/cloth"]

# === Helpers ===

def disassemble_mips64(data: bytes, start_va: int) -> list[dict]:
    """Disassemble MIPS64 little-endian binary data."""
    md = capstone.Cs(capstone.CS_ARCH_MIPS,
                     capstone.CS_MODE_MIPS64 + capstone.CS_MODE_LITTLE_ENDIAN)
    result = []
    for insn in md.disasm(data, start_va):
        result.append({
            "va": insn.address,
            "bytes": insn.bytes.hex(),
            "mnemonic": insn.mnemonic,
            "op_str": insn.op_str,
            "size": insn.size,
        })
    return result


def extract_elf_func(target_va: int, size: int = 0x200) -> bytes:
    """Extract raw bytes of a function from the ELF at the given VA."""
    with open(ELF_PATH, "rb") as f:
        elf_data = f.read()

    e_phoff = struct.unpack_from("<I", elf_data, 0x1C)[0]
    e_phentsize = struct.unpack_from("<H", elf_data, 0x2A)[0]
    e_phnum = struct.unpack_from("<H", elf_data, 0x2C)[0]

    for i in range(e_phnum):
        ph = elf_data[e_phoff + i * e_phentsize: e_phoff + (i + 1) * e_phentsize]
        p_type = struct.unpack_from("<I", ph, 0)[0]
        if p_type != 1:
            continue
        p_offset = struct.unpack_from("<I", ph, 4)[0]
        p_vaddr = struct.unpack_from("<I", ph, 8)[0]
        p_filesz = struct.unpack_from("<I", ph, 16)[0]
        if p_vaddr <= target_va < p_vaddr + p_filesz:
            file_off = p_offset + (target_va - p_vaddr)
            avail = min(size, p_filesz - (target_va - p_vaddr))
            return elf_data[file_off:file_off + avail]
    raise ValueError(f"VA 0x{target_va:08X} not found in any LOAD segment")


def compile_c_to_asm(c_code: str) -> str | None:
    """Compile C code with ee-gcc 2.9 and return assembly text."""
    fd, infile = tempfile.mkstemp(suffix=".c")
    os.write(fd, c_code.encode())
    os.close(fd)

    sfile = infile.rsplit(".", 1)[0] + ".s"

    try:
        result = subprocess.run(
            [TOOLCHAIN] + CC_FLAGS + ["-S", infile, "-o", sfile],
            capture_output=True, text=True, timeout=30,
        )
        if result.returncode != 0:
            print("COMPILE ERROR:", result.stderr, file=sys.stderr)
            return None

        if os.path.exists(sfile):
            with open(sfile) as f:
                return f.read()
        return result.stdout
    finally:
        os.unlink(infile)
        if os.path.exists(sfile):
            os.unlink(sfile)


def extract_func_asm(asm_text: str, func_name: str) -> str:
    """Extract assembly for a specific function from GCC output."""
    lines = []
    in_func = False
    for line in asm_text.splitlines():
        stripped = line.strip()
        # Detect function entry
        if stripped.startswith(func_name + ":"):
            in_func = True
            continue
        if in_func:
            # Next public label or function ends it
            if (stripped.startswith("\t.globl") and not stripped.endswith(func_name)) \
               or re.match(r'^[a-zA-Z_][a-zA-Z0-9_]*:', stripped):
                break
            # Skip assembler directives
            if stripped.startswith(".") and not stripped.startswith(".set"):
                continue
            lines.append(stripped)
    return "\n".join(lines)


# Register name mapping: ABI names → numbers
ABI_TO_NUM = {
    "$zero": "$0", "$at": "$1", "$v0": "$2", "$v1": "$3",
    "$a0": "$4", "$a1": "$5", "$a2": "$6", "$a3": "$7",
    "$t0": "$8", "$t1": "$9", "$t2": "$10", "$t3": "$11",
    "$t4": "$12", "$t5": "$13", "$t6": "$14", "$t7": "$15",
    "$s0": "$16", "$s1": "$17", "$s2": "$18", "$s3": "$19",
    "$s4": "$20", "$s5": "$21", "$s6": "$22", "$s7": "$23",
    "$t8": "$24", "$t9": "$25", "$k0": "$26", "$k1": "$27",
    "$gp": "$28", "$sp": "$29", "$fp": "$30", "$ra": "$31",
    "$0": "$zero",
}

NUM_TO_ABI = {v: k for k, v in ABI_TO_NUM.items()}

# CP1 register names
FREG_ABI = {f"${i}": f"$f{i}" for i in range(32)}

LOCAL_BRANCH_MNEMS = {
    "beq", "bne", "beql", "bnel",
    "bc1f", "bc1t", "bc1fl", "bc1tl",
    "bltz", "bgez", "bgtz", "blez",
    "j",
}


def normalize_insn(line: str) -> str:
    """Normalize MIPS instruction for structural comparison."""
    line = re.sub(r'\s*#.*$', '', line).strip()
    if not line:
        return ""
    parts = line.split(None, 1)
    if len(parts) < 2:
        return parts[0]
    mnem, ops = parts

    # 1. ABI register names → numeric ($a0 → $4, $sp → $29)
    for abi, num in ABI_TO_NUM.items():
        ops = ops.replace(abi, num)

    # 2. All hex immediates ($0x15c or 0x15c) → decimal
    ops = re.sub(r'\$?0x([0-9a-fA-F]+)', lambda m: str(int(m.group(1), 16)), ops)

    # 2b. External symbolic targets emitted from prototypes use address-like
    # names in this repo (sub_1CF998, func_001D37C8, fn_1CE5F8).
    # Normalize those to the
    # same decimal target format Capstone emits for original ELF calls.
    ops = re.sub(
        r'\b(?:sub|func|fn)_([0-9a-fA-F]{6,8})\b',
        lambda m: str(int(m.group(1), 16)),
        ops,
    )

    # 3. Strip leading $ from all bare register numbers
    ops = re.sub(r'\$(\d+)', r'\1', ops)

    # 4. Normalize zero offset: 0($reg) or $0($reg) → ($reg)
    ops = re.sub(r'(?<=[\s,(])0\(', '(', ops)
    ops = re.sub(r'(?<=[\s,(])\$0\(', '(', ops)

    # 5. Normalize comma spacing: always comma+space
    ops = re.sub(r',\s*', ', ', ops)

    # 6. Collapse spacing
    ops = re.sub(r'\s+', ' ', ops).strip()

    # 7. Normalize mnemonics
    if mnem == 'j' and (ops == '$31' or ops == '$ra' or ops == '31' or ops == 'ra'):
        mnem = 'jr'

    # 8. sltu $rd, $rs, 1 → sltiu $rd, $rs, 1
    # GCC outputs sltu with immediate 1 as a macro, GAS assembles it as sltiu.
    if mnem == 'sltu':
        parts = ops.split(',')
        if len(parts) == 3 and parts[2].strip() in ('1', '$1'):
            mnem = 'sltiu'

    # 8b. slt $rd, $rs, N → slti $rd, $rs, N
    # GAS accepts slt with an immediate as a macro for slti.
    if mnem == 'slt':
        parts = [part.strip() for part in ops.split(',')]
        if len(parts) == 3:
            try:
                int(parts[2], 0)
            except ValueError:
                pass
            else:
                mnem = 'slti'

    # 9. subu $rd, $rs, N → addiu $rd, $rs, -N
    # GCC outputs subu/addu as pseudo-ops for stack frame; GAS expands to addiu.
    if mnem == 'subu':
        parts = ops.split(',')
        if len(parts) == 3 and parts[0].strip() == parts[1].strip():
            mnem = 'addiu'
            # Negate the immediate
            imm_str = parts[2].strip()
            try:
                imm = int(imm_str)
                ops = f"{parts[0].strip()}, {parts[1].strip()}, {-imm}"
            except ValueError:
                pass  # can't parse, leave as-is

    # 10. addu $rd, $rs, N → addiu $rd, $rs, N (same macro pattern)
    if mnem == 'addu':
        parts = ops.split(',')
        if len(parts) == 3 and parts[0].strip() == parts[1].strip():
            mnem = 'addiu'

    # 11. beqz $rs, target → beq $rs, $zero, target (pseudo-op)
    if mnem == 'beqz':
        ops_parts = ops.split(',', 1)
        if len(ops_parts) == 2:
            mnem = 'beq'
            ops = f"{ops_parts[0].strip()}, $zero, {ops_parts[1].strip()}"

    # 12. l.s → lwc1 (alias in GAS)
    if mnem == 'l.s':
        mnem = 'lwc1'

    # 13. s.s → swc1
    if mnem == 's.s':
        mnem = 'swc1'

    # 14. bnez $rs, target → bne $rs, $zero, target
    if mnem == 'bnez':
        ops_parts = ops.split(',', 1)
        if len(ops_parts) == 2:
            mnem = 'bne'
            ops = f"{ops_parts[0].strip()}, $zero, {ops_parts[1].strip()}"

    # 15. li $rd, imm -> addiu $rd, $zero, imm for small constants.
    if mnem == 'li':
        parts = [part.strip() for part in ops.split(',')]
        if len(parts) == 2:
            try:
                imm = int(parts[1], 0)
            except ValueError:
                imm = None
            if imm is not None and -0x8000 <= imm <= 0x7fff:
                mnem = 'addiu'
                ops = f"{parts[0]}, $zero, {imm}"

    # 16. GAS may print dsrl rd, rs, 33 where Capstone prints dsrl32 rd, rs, 1.
    if mnem == 'dsrl':
        parts = [part.strip() for part in ops.split(',')]
        if len(parts) == 3:
            try:
                shift = int(parts[2], 0)
            except ValueError:
                shift = None
            if shift is not None and shift >= 32:
                mnem = 'dsrl32'
                ops = f"{parts[0]}, {parts[1]}, {shift - 32}"

    return f"{mnem} {ops}"


def _is_asm_instruction_line(stripped: str) -> bool:
    if not stripped or stripped.endswith(":") or stripped.startswith("."):
        return False
    return True


def _normalize_local_branch_target(insn: str, resolver) -> str:
    parts = insn.split(None, 1)
    if len(parts) < 2:
        return insn

    mnem, ops = parts
    if mnem not in LOCAL_BRANCH_MNEMS:
        return insn

    operands = [op.strip() for op in ops.split(",")]
    if not operands:
        return insn

    target = operands[-1]
    resolved = resolver(target)
    if resolved is None:
        return insn

    operands[-1] = resolved
    return f"{mnem} {', '.join(operands)}"


def parse_asm_lines(asm: str, resolve_local_labels: bool = True) -> list[str]:
    """Parse assembly into (mnemonic + operands) strings, stripping labels."""
    label_to_idx = {}
    if resolve_local_labels:
        idx = 0
        for raw in asm.splitlines():
            stripped = raw.strip()
            if stripped.endswith(":"):
                label = stripped[:-1]
                label_to_idx[label] = idx
                label_to_idx[label.lstrip("$")] = idx
                continue
            if _is_asm_instruction_line(stripped) and normalize_insn(stripped):
                idx += 1

    def resolve_label(target: str) -> str | None:
        if not resolve_local_labels:
            return None
        key = target.strip()
        if key in label_to_idx:
            return f"@{label_to_idx[key]}"
        key = key.lstrip("$")
        if key in label_to_idx:
            return f"@{label_to_idx[key]}"
        return None

    lines = []
    for line in asm.splitlines():
        line = line.strip()
        if not _is_asm_instruction_line(line):
            continue
        norm = normalize_insn(line)
        if norm:
            norm = _normalize_local_branch_target(norm, resolve_label)
            lines.append(norm)
    return lines


def normalize_target_insns(target_insns: list[dict]) -> list[str]:
    """Normalize target instructions and convert intra-function branches to @idx."""
    va_to_idx = {insn["va"]: idx for idx, insn in enumerate(target_insns)}

    def resolve_va(target: str) -> str | None:
        try:
            target_va = int(target, 10)
        except ValueError:
            return None
        if target_va in va_to_idx:
            return f"@{va_to_idx[target_va]}"
        return None

    lines = []
    for insn in target_insns:
        norm = normalize_insn(f"{insn['mnemonic']} {insn['op_str']}")
        if norm:
            norm = _normalize_local_branch_target(norm, resolve_va)
            lines.append(norm)
    return lines


def align_instructions(tgt: list[str], gen: list[str]) -> list[tuple]:
    """Align two instruction sequences using LCS.

    Returns list of (tgt_idx | None, gen_idx | None, tgt_insn, gen_insn, status).
    Status is 'match', 'mismatch', 'missing' (in gen), 'extra' (in gen).
    """
    n, m = len(tgt), len(gen)
    # DP table
    dp = [[0] * (m + 1) for _ in range(n + 1)]
    for i in range(1, n + 1):
        for j in range(1, m + 1):
            if tgt[i - 1] == gen[j - 1]:
                dp[i][j] = dp[i - 1][j - 1] + 1
            else:
                dp[i][j] = max(dp[i - 1][j], dp[i][j - 1])
    # Backtrack
    result = []
    i, j = n, m
    while i > 0 or j > 0:
        if i > 0 and j > 0 and tgt[i - 1] == gen[j - 1]:
            result.append((i - 1, j - 1, tgt[i - 1], gen[j - 1], 'match'))
            i -= 1
            j -= 1
        elif j > 0 and (i == 0 or dp[i][j - 1] >= dp[i - 1][j]):
            result.append((None, j - 1, '(missing)', gen[j - 1], 'extra'))
            j -= 1
        else:
            result.append((i - 1, None, tgt[i - 1], '(missing)', 'missing'))
            i -= 1
    result.reverse()
    return coalesce_missing_extra_pairs(result)


def _mnemonic(text: str) -> str:
    if not text or text == "(missing)":
        return ""
    return text.split()[0]


def coalesce_missing_extra_pairs(aligned: list[tuple]) -> list[tuple]:
    """Pair adjacent missing/extra rows that are really operand mismatches.

    LCS alignment is useful for inserted/deleted instructions, but it represents
    same-mnemonic operand differences as a missing target row plus an extra
    generated row. Coalescing those pairs restores the scorer's old partial
    credit behavior and lets the diff classifier label branch-target/register
    differences instead of reporting them as structural insertions.
    """
    result = []
    i = 0
    while i < len(aligned):
        cur = aligned[i]
        if cur[4] in ("missing", "extra"):
            j = i
            first_status = cur[4]
            while j < len(aligned) and aligned[j][4] == first_status:
                j += 1

            second_status = "extra" if first_status == "missing" else "missing"
            k = j
            while k < len(aligned) and aligned[k][4] == second_status:
                k += 1

            first_block = aligned[i:j]
            second_block = aligned[j:k]
            if first_block and len(first_block) == len(second_block):
                merged = []
                can_merge = True
                for a, b in zip(first_block, second_block):
                    missing = a if a[4] == "missing" else b
                    extra = a if a[4] == "extra" else b
                    if _mnemonic(missing[2]) != _mnemonic(extra[3]):
                        can_merge = False
                        break
                    merged.append((missing[0], extra[1], missing[2], extra[3], "mismatch"))
                if can_merge:
                    result.extend(merged)
                    i = k
                    continue

        if i + 1 >= len(aligned):
            result.append(cur)
            i += 1
            continue

        nxt = aligned[i + 1]
        cur_missing = cur[4] == "missing" and nxt[4] == "extra"
        cur_extra = cur[4] == "extra" and nxt[4] == "missing"

        if cur_missing or cur_extra:
            missing = cur if cur[4] == "missing" else nxt
            extra = cur if cur[4] == "extra" else nxt
            if _mnemonic(missing[2]) == _mnemonic(extra[3]):
                result.append((missing[0], extra[1], missing[2], extra[3], "mismatch"))
                i += 2
                continue

        result.append(cur)
        i += 1

    return result


def score_against_target(func_asm: str, target_va: int, func_size: int = 0x100,
                         format: str = 'full'):
    """Score generated assembly against target ELF function.

    Args:
        format: 'full' (default), 'compact', or 'json'
    """
    target_bytes = extract_elf_func(target_va, func_size)
    target_insns = disassemble_mips64(target_bytes, target_va)

    gen_lines = parse_asm_lines(func_asm)

    # Normalize target instructions for comparison
    tgt_lines = normalize_target_insns(target_insns)

    # Trim trailing nops from both sides (alignment padding)
    while tgt_lines and tgt_lines[-1] == 'nop':
        tgt_lines.pop()
    while gen_lines and gen_lines[-1] == 'nop':
        gen_lines.pop()

    # Align using LCS
    aligned = align_instructions(tgt_lines, gen_lines)

    # Score: each match = 100, each mnemonic-only match = 50
    score = 0
    max_score = len(tgt_lines) * 100
    for tgt_i, gen_i, tgt_text, gen_text, status in aligned:
        if status == 'match':
            score += 100
        elif status == 'missing':
            pass
        elif status == 'extra':
            pass
        else:
            # mismatched — check mnemonic
            tgt_mnem = tgt_text.split()[0] if tgt_text else ""
            gen_mnem = gen_text.split()[0] if gen_text else ""
            if tgt_mnem == gen_mnem:
                score += 50

    pct = (score / max_score * 100) if max_score > 0 else 0

    match_count = sum(1 for _, _, _, _, s in aligned if s == 'match')
    missing_count = sum(1 for _, _, _, _, s in aligned if s == 'missing')
    extra_count = sum(1 for _, _, _, _, s in aligned if s == 'extra')

    print(f"Target instructions: {len(tgt_lines)}")
    print(f"Generated instructions: {len(gen_lines)}")
    print(f"Aligned matches: {match_count}")
    print(f"Missing in generated: {missing_count}")
    print(f"Extra in generated: {extra_count}")
    print()
    print(f"Score: {score}/{max_score} = {pct:.2f}%")
    print()

    if format == 'compact':
        if match_count < min(len(tgt_lines), len(gen_lines)):
            print(f"  {missing_count} missing, {extra_count} extra")
            # Show first few structural diffs
            shown = 0
            for tgt_i, gen_i, tgt_text, gen_text, status in aligned:
                if status == 'match':
                    continue
                if status == 'missing':
                    print(f"  -{tgt_i}: {tgt_text}")
                elif status == 'extra':
                    print(f"  +{gen_i}: {gen_text}")
                else:
                    tgt_mnem = tgt_text.split()[0] if tgt_text else ""
                    gen_mnem = gen_text.split()[0] if gen_text else ""
                    if tgt_mnem == gen_mnem:
                        print(f"  ~{tgt_i}: T:{tgt_text}  G:{gen_text}  (reg/imm)")
                    else:
                        print(f"  !{tgt_i}: T:{tgt_text}  G:{gen_text}  (diff mnem)")
                shown += 1
                if shown >= 10:
                    remaining = sum(1 for _, _, _, _, s in aligned if s != 'match') - 10
                    if remaining > 0:
                        print(f"  ... +{remaining} more diffs")
                    break
        else:
            print("  PERFECT MATCH!")
        return pct

    # Differences section
    mismatches = [(tgt_i, gen_i, tgt_text, gen_text, status)
                  for tgt_i, gen_i, tgt_text, gen_text, status in aligned
                  if status != 'match']

    if mismatches:
        print("Differences (structural alignment):")
        print("-" * 70)
        for tgt_i, gen_i, tgt_text, gen_text, status in mismatches:
            idx = tgt_i if tgt_i is not None else gen_i
            if status == 'missing':
                print(f"  -{idx:3d}: | {tgt_text:30s} | (missing in gen)")
            elif status == 'extra':
                print(f"  +{idx:3d}: | (extra in gen)        | {gen_text}")
            elif status == 'mismatch':
                tgt_mnem = tgt_text.split()[0] if tgt_text else ""
                gen_mnem = gen_text.split()[0] if gen_text else ""
                if tgt_mnem == gen_mnem:
                    print(f"  ~{tgt_i:3d}: | {tgt_text:30s} | {gen_text}  (reg/imm)")
                else:
                    print(f"  !{tgt_i:3d}: | {tgt_text:30s} | {gen_text}  (diff mnem)")
        print()
    else:
        print("PERFECT MATCH!")
        print()

    # Full alignment
    print("Full alignment (target vs generated):")
    print("-" * 70)
    for tgt_i, gen_i, tgt_text, gen_text, status in aligned:
        idx = gen_i if gen_i is not None else (tgt_i if tgt_i is not None else 0)
        if status == 'match':
            marker = " "
        elif status == 'missing':
            marker = "M"
        elif status == 'extra':
            marker = "E"
        else:
            marker = "X"
        tgt_disp = f"{tgt_i:3d}:{tgt_text}" if tgt_i is not None else "   --"
        gen_disp = f"{gen_i:3d}:{gen_text}" if gen_i is not None else "   --"
        print(f"  {marker} {tgt_disp:35s} {gen_disp}")
    print()

    return pct


def compile_and_score(c_code: str, func_name: str, target_va: int, func_size: int = 0x100):
    """Full pipeline: compile → extract → score."""
    print("=== Compiling with ee-gcc 2.9-991111-01 ===")
    print(f"  Flags: {' '.join(CC_FLAGS)}")
    print(f"  Target: {func_name} @ 0x{target_va:08X} ({func_size}B)")
    print()

    asm = compile_c_to_asm(c_code)
    if asm is None:
        print("FAILED: compilation error")
        return

    func_asm = extract_func_asm(asm, func_name)
    if not func_asm.strip():
        print(f"FAILED: function '{func_name}' not found in output")
        print("Available sections:")
        for line in asm.splitlines():
            if line.strip().endswith(":") and not line.strip().startswith("."):
                print(f"  {line.strip()}")
        return

    print("=== Generated ASM ===")
    print(func_asm)
    print()

    score_against_target(func_asm, target_va, func_size)


def extract_function_body(c_code: str, func_name: str) -> str | None:
    """Extract function body + ALL preceding declarations/types."""
    lines = c_code.splitlines()
    pre_lines = []   # everything before the function definition
    func_lines = []  # the function definition itself
    func_line_idx = -1

    # Find the target function DEFINITION line
    for i, line in enumerate(lines):
        stripped = line.strip()
        if stripped.endswith(";"):
            continue  # skip forward decls
        pattern = re.compile(
            r'^(void|int|u32|ico_u32|ico_ptr32|float|static)\s+\**\s*'
            + re.escape(func_name) + r'\s*\('
        )
        if pattern.search(stripped):
            func_line_idx = i
            break

    if func_line_idx < 0:
        return None

    # Include ALL lines before the function (except other function bodies)
    brace_depth = 0
    for i in range(func_line_idx):
        stripped = lines[i].strip()
        # Skip comment-only lines (including multi-line comment continuations)
        if stripped.startswith("//") or stripped.startswith("/*") or stripped.startswith("*"):
            continue
        pre_lines.append(lines[i])

    # Extract function body by brace matching
    in_func = False
    brace_depth = 0
    saw_open = False
    for i in range(func_line_idx, len(lines)):
        stripped = lines[i].strip()
        if not in_func:
            pattern = re.compile(
                r'^(void|int|u32|ico_u32|ico_ptr32|float|static)\s+\**\s*'
                + re.escape(func_name) + r'\s*\('
            )
            if pattern.search(stripped):
                in_func = True
        if in_func:
            func_lines.append(lines[i])
            if "{" in lines[i]:
                saw_open = True
            brace_depth += lines[i].count("{") - lines[i].count("}")
            if brace_depth == 0 and saw_open:
                break

    if not func_lines:
        return None

    result = list(pre_lines)
    result.extend(func_lines)
    return "\n".join(result)


def main():
    parser = argparse.ArgumentParser(
        description="Compile C with ee-gcc 2.9 and score against ELF target")
    parser.add_argument("c_file", help="C source file path")
    parser.add_argument("--fn", required=True, help="Function name to extract")
    parser.add_argument("--va", required=True, help="Target VA (e.g. 0x1CE5F8)")
    parser.add_argument("--size", type=str, default="0x100",
                        help="Function size in bytes (e.g. 0x100, 256)")
    parser.add_argument("--whole-file", action="store_true",
                        help="Compile entire file, don't extract function")
    args = parser.parse_args()

    with open(args.c_file) as f:
        c_code = f.read()

    target_va = int(args.va, 16)
    target_size = int(args.size, 16) if isinstance(args.size, str) and args.size.startswith("0x") else int(args.size)

    if not args.whole_file:
        extracted = extract_function_body(c_code, args.fn)
        if extracted is None:
            print(f"FAILED: function '{args.fn}' not found in {args.c_file}")
            return
        compile_and_score(extracted, args.fn, target_va, target_size)
    else:
        compile_and_score(c_code, args.fn, target_va, target_size)


if __name__ == "__main__":
    main()
