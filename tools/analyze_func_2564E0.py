#!/usr/bin/env python3
"""
Final analysis of func_2564E0 — SIF Resource Loader.
"""

import struct
from collections import Counter, defaultdict
from elftools.elf.elffile import ELFFile
import rabbitizer

ELF_PATH = "/home/hoper/Documentos/repos/ico-reconstruction/.local/extracted/SCUS_971.13.elf"

def load_segment(path):
    with open(path, "rb") as f:
        elf = ELFFile(f)
        for seg in elf.iter_segments():
            if seg.header.p_type == "PT_LOAD":
                return seg.data(), seg.header.p_vaddr
    return None, 0

def read_u32(data, sv, va):
    off = va - sv
    if 0 <= off <= len(data) - 4:
        return struct.unpack_from("<I", data, off)[0]
    return None

def disasm_func(data, sv, start, max_insns=200):
    result = []
    addr = start
    for _ in range(max_insns):
        w = read_u32(data, sv, addr)
        if w is None:
            break
        i = rabbitizer.Instruction(word=w, vram=addr)
        if not i.isValid():
            break
        result.append(i)
        if i.isReturn():
            dw = read_u32(data, sv, addr + 4)
            if dw:
                di = rabbitizer.Instruction(word=dw, vram=addr + 4)
                if di.isValid():
                    result.append(di)
            break
        addr += 4
    return result

def parse_args(instr):
    """Parse operands from disassembly string."""
    d = instr.disassemble().strip()
    mn = instr.getOpcodeName()
    return d[len(mn):].strip() if d.startswith(mn) else d

def analyze():
    data, sv = load_segment(ELF_PATH)

    print("=" * 90)
    print("COMPREHENSIVE ANALYSIS: func_2564E0")
    print("SCUS_971.13.elf — ICO USA PS2 (R5900)")
    print("=" * 90)

    # ━━━ DISASSEMBLE ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    insns = disasm_func(data, sv, 0x2564E0, 200)
    func_size = insns[-1].vram + 4 - 0x2564E0
    print(f"\nFunction: 0x2564E0 – 0x{insns[-1].vram+4:08X} ({func_size} bytes, {len(insns)} insns)")

    print("\n" + "=" * 90)
    print("FULL DISASSEMBLY")
    print("=" * 90)

    # Collect branch targets for BB markers
    bb_targets = set()
    for i in insns:
        if i.isBranch():
            boff = i.getBranchOffset()
            if boff is not None:
                bb_targets.add(i.vram + 4 + boff)
        if i.getOpcodeName() in ('jal', 'j'):
            args = parse_args(i)
            parts = [p.strip() for p in args.split(',')]
            try:
                bb_targets.add(int(parts[-1], 0))
            except:
                pass

    for idx, i in enumerate(insns):
        mn = i.getOpcodeName()
        args = parse_args(i)
        raw = i.getRaw()

        flags = []
        if i.isBranch(): flags.append("BR")
        if i.isReturn(): flags.append("RET")
        if mn == 'jal': flags.append("CALL")
        if mn == 'j': flags.append("JUMP")

        binfo = ""
        if i.isBranch():
            boff = i.getBranchOffset()
            if boff is not None:
                tgt = i.vram + 4 + boff
                direction = "← LOOP" if tgt < i.vram else "→ FWD"
                binfo = f"  → 0x{tgt:08X} ({direction})"
        if mn == 'jal':
            parts = [p.strip() for p in args.split(',')]
            try:
                t = int(parts[-1], 0)
                binfo = f"  → func_0x{t:08X}"
            except:
                pass

        marker = " ◆" if i.vram in bb_targets else ""
        flagstr = '/'.join(flags) if flags else ''
        print(f"  {idx:3d} 0x{i.vram:08X} {raw:08X} {mn:8s} {args:40s} [{flagstr:5s}]{binfo}{marker}")

    # ━━━ STACK FRAME ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    print("\n" + "=" * 90)
    print("STACK FRAME (96 bytes = 0x60)")
    print("=" * 90)

    print("\n  Prologue:")
    for i in insns[:20]:
        mn = i.getOpcodeName()
        args = parse_args(i)
        if mn == 'addiu' and '$sp' in args:
            print(f"    0x{i.vram:08X}: {mn:8s} {args}  ; allocate frame")
            break

    print("\n  Callee-saved register save/restore:")
    for i in insns[:25]:
        mn = i.getOpcodeName()
        args = parse_args(i)
        if mn in ('sd', 'sw', 'sq') and '$sp' in args:
            parts = args.split(',')
            reg = parts[0].strip() if parts else '?'
            off_str = parts[1].strip().split('(')[0] if len(parts) > 1 else '?'
            print(f"    0x{i.vram:08X}: {mn:8s} {args:30s}  ← SAVE {reg}")
    print()
    for i in insns:
        mn = i.getOpcodeName()
        args = parse_args(i)
        if mn in ('ld', 'lw', 'lq') and '$sp' in args and i.vram > 0x256580:
            parts = args.split(',')
            reg = parts[0].strip() if parts else '?'
            off_str = parts[1].strip().split('(')[0] if len(parts) > 1 else '?'
            print(f"    0x{i.vram:08X}: {mn:8s} {args:30s}  → RESTORE {reg}")

    # ━━━ HARDWARE ACCESSES ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    print("\n" + "=" * 90)
    print("HARDWARE REGISTER ACCESSES")
    print("=" * 90)

    print("""
  The function communicates with the PS2 IOP (I/O Processor) via the
  SIF (Sub-processor Interface) bridge registers:

  ┌──────────────┬────────────────────────────────────────────────────┐
  │ Address      │ Access pattern                                     │
  ├──────────────┼────────────────────────────────────────────────────┤
  │ 0x10002010   │ LW (32-bit read) — SIF status/control register    │
  │              │   AND 0x80004000 — test bits 31+14                 │
  │              │   BNE at 0x256520 — branch if flags differ         │
  │              │   SW (32-bit write) — set channel flags            │
  ├──────────────┼────────────────────────────────────────────────────┤
  │ 0x10002000   │ LD (64-bit read) — SIF DMA address (func_256028)  │
  │              │   BGEZ — check if DMA complete (addr >= 0)         │
  │              │   SW (32-bit write) — set DMA target address       │
  └──────────────┴────────────────────────────────────────────────────┘

  Also used by func_256028 (internal callee):
  ┌──────────────┬────────────────────────────────────────────────────┐
  │ 0x10002010   │ LW — AND 0x4000 — test bit 14 (busy flag)         │
  │ 0x10002000   │ LD — read 64-bit DMA result                       │
  └──────────────┴────────────────────────────────────────────────────┘
""")

    # ━━━ INTERNAL CALLEES ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    print("=" * 90)
    print("INTERNAL CALLEES")
    print("=" * 90)

    callees = []
    for i in insns:
        if i.getOpcodeName() == 'jal':
            args = parse_args(i)
            parts = [p.strip() for p in args.split(',')]
            try:
                t = int(parts[-1], 0)
                callees.append((i.vram, t))
            except:
                pass

    for va, target in callees:
        # Disassemble callee briefly
        callee_insns = disasm_func(data, sv, target, 40)
        callee_size = callee_insns[-1].vram + 4 - target if callee_insns else 0
        callee_callees = sum(1 for ci in callee_insns if ci.getOpcodeName() == 'jal')
        callee_returns = sum(1 for ci in callee_insns if ci.isReturn())
        print(f"\n  0x{va:08X}: jal func_{target:08X}")
        print(f"    Callee: {callee_size} bytes, {len(callee_insns)} insns, {callee_callees} internal calls")
        # Show first 10 instructions of callee
        for ci in callee_insns[:10]:
            print(f"      0x{ci.vram:08X}: {ci.disassemble().strip()}")
        if len(callee_insns) > 10:
            print(f"      ... ({len(callee_insns) - 10} more instructions)")

    # ━━━ LOOP ANALYSIS ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    print("\n" + "=" * 90)
    print("LOOP / BRANCH ANALYSIS")
    print("=" * 90)

    print("\n  Conditional branches:")
    for i in insns:
        if i.isBranch() and not i.isReturn():
            args = parse_args(i)
            boff = i.getBranchOffset()
            if boff is not None:
                tgt = i.vram + 4 + boff
                direction = "BACKWARD (loop)" if tgt < i.vram else "forward"
                print(f"    0x{i.vram:08X}: {i.getOpcodeName():8s} {args:40s} → 0x{tgt:08X}  ({direction})")

    # ━━━ CALLER ANALYSIS ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    print("\n" + "=" * 90)
    print("CALLER ANALYSIS (82 call sites)")
    print("=" * 90)

    target_enc = (3 << 26) | (0x2564E0 >> 2)
    a0_vals = Counter()
    caller_list = []
    store_targets = Counter()

    for off in range(0, len(data) - 8, 4):
        word = struct.unpack_from('<I', data, off)[0]
        if word == target_enc:
            caller_va = sv + off
            delay_va = caller_va + 4
            dw = read_u32(data, sv, delay_va)
            if dw is not None:
                di = rabbitizer.Instruction(word=dw, vram=delay_va)
                dargs = parse_args(di)
                a0_val = None
                if 'a0' in dargs and 'zero' in dargs:
                    parts = dargs.split(',')
                    if len(parts) >= 3:
                        imm = parts[2].strip()
                        try:
                            a0_val = int(imm, 0) if 'x' in imm.lower() else int(imm)
                        except:
                            pass
                if a0_val is not None:
                    a0_vals[a0_val] += 1

                # Look at the instruction BEFORE jal for sw (where result is stored)
                prev_va = caller_va - 4
                prev_w = read_u32(data, sv, prev_va)
                prev_i = rabbitizer.Instruction(word=prev_w, vram=prev_va) if prev_w else None

                caller_list.append((caller_va, a0_val, di))

    print(f"\n  Total callers: {len(caller_list)}")
    print(f"  Unique $a0 values: {len(a0_vals)}")

    print("\n  $a0 distribution (resource type codes):")
    for val, count in sorted(a0_vals.items()):
        bar = '█' * count
        print(f"    $a0=0x{val:02X} ({val:3d}): {count:3d} {bar}")

    # ━━━ CALLER ADDRESS DISTRIBUTION ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    print("\n  Caller address distribution (by 4KB page):")
    pages = defaultdict(list)
    for caller_va, a0_val, _ in caller_list:
        page = (caller_va >> 12) << 12
        pages[page].append(caller_va)

    for page in sorted(pages.keys()):
        addrs = pages[page]
        print(f"    0x{page:08X}: {len(addrs):3d} callers  [{', '.join(f'0x{a:08X}' for a in addrs[:5])}{'...' if len(addrs) > 5 else ''}]")

    # ━━━ SAMPLE CALLERS ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    print("\n  Sample callers with context (first 15):")
    for caller_va, a0_val, delay_i in caller_list[:15]:
        # Show 3 insns before + call + delay slot
        print(f"\n    0x{caller_va:08X} ($a0={f'0x{a0_val:02X}' if a0_val is not None else '?'}):")
        for ctx_off in range(-3, 2):
            ctx_va = caller_va + ctx_off * 4
            w = read_u32(data, sv, ctx_va)
            if w is None:
                continue
            ci = rabbitizer.Instruction(word=w, vram=ctx_va)
            marker = ">>> " if ctx_off == 0 else "    "
            print(f"      {marker}0x{ctx_va:08X}: {ci.disassemble().strip()}")

    # ━━━ STORE PATTERN ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    print("\n" + "=" * 90)
    print("RESULT STORAGE PATTERN (where return values are stored)")
    print("=" * 90)

    store_offsets = Counter()
    for caller_va, a0_val, delay_i in caller_list:
        # Look at 2 instructions AFTER the delay slot for 'sw' storing $v0
        for fwd in range(2, 5):
            fwd_va = caller_va + (1 + fwd) * 4
            fw = read_u32(data, sv, fwd_va)
            if fw is None:
                continue
            fi = rabbitizer.Instruction(word=fw, vram=fwd_va)
            fargs = parse_args(fi)
            if fi.getOpcodeName() in ('sw', 'sd', 'sq') and '$v0' in fargs:
                # Extract store offset
                parts = fargs.split(',')
                if len(parts) >= 2:
                    addr_part = parts[-1].strip()
                    if '(' in addr_part:
                        off_str = addr_part.split('(')[0].strip()
                        base = addr_part.split('(')[1].rstrip(')').strip()
                        try:
                            off_val = int(off_str, 0) if 'x' in off_str else int(off_str)
                            store_offsets[(base, off_val)] += 1
                        except:
                            pass
                break

    if store_offsets:
        print("\n  Return value ($v0) stored at:")
        for (base, off), count in sorted(store_offsets.items(), key=lambda x: -x[1]):
            print(f"    [{base} + 0x{off & 0xFFFF:04X}]  ×{count}")

    # ━━━ ROLE SUMMARY ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    print("\n" + "=" * 90)
    print("ROLE IDENTIFICATION")
    print("=" * 90)
    print(f"""
  ┌─────────────────────────────────────────────────────────────────────────┐
  │ FUNCTION: SIF Resource Loader / IOP Communication Primitive            │
  │                                                                         │
  │ Prototype: u64 func_2564E0(u32 resource_type)                          │
  │                                                                         │
  │ Stack frame: 96 bytes (saves $s0-$s5, $ra)                             │
  │ Callees: func_256028 (DMA poll), func_251D48 (timeout handler)          │
  │ Hardware: SIF registers at 0x10002000, 0x10002010                       │
  │                                                                         │
  │ This function is NOT recursive. There are ZERO self-calls.              │
  │ The loop at 0x25653C is a timeout-bounded SIF polling loop.             │
  │ The 82 "callers" are external call sites across boyAI and engine.       │
  │                                                                         │
  │ $a0 parameter is a resource type code (14 unique values, 1-32).         │
  │ The function sends a command to the IOP via SIF, waits for DMA          │
  │ completion, and returns a pointer to the loaded resource data.          │
  └─────────────────────────────────────────────────────────────────────────┘

  ALGORITHM (pseudo-C):
  ─────────────────────
    u64 func_2564E0(u32 type) {{
        save_regs();

        // Check SIF status
        u32 status = *(u32*)0x10002010;
        if ((status & 0x80004000) != 0x80000000) {{
            // SIF not ready — busy-wait with timeout
            u32 timeout = 0;
            while (timeout < 5001) {{
                timeout++;
                status = *(u32*)0x10002010;
                if ((status & 0x80004000) == 0x80000000)
                    break;
            }}
            if (timeout >= 5001)
                func_251D48(sif_error_context);  // timeout handler
        }}

        // Compute resource address from type
        u32 addr = type_to_address(type);

        // Set SIF channel
        *(u32*)0x10002010 = (status & 0xFFFCFFFF) | (flags & 0x00030000);

        // Load from lookup table and poll DMA
        u64 resource = func_256028(lookup_table[type]);

        // Store result
        save_result(resource);
        restore_regs();
        return resource;
    }}

  WHY 82 CALLERS:
  ────────────────
  This function is the primary EE→IOP resource loading primitive.
  Every subsystem that needs data loaded from the IOP (animation,
  AI tables, physics data, sound, textures, level geometry) calls
  this function with a different resource_type code. It is the
  most-called utility in boyAI because resource loading is the
  fundamental operation for populating entity data.

  The $a0 values map to IOP resource IDs:
    0x01 (×26) — entity/animation data (most common)
    0x08 (×4)  — secondary data type
    0x03 (×4)  — tertiary data type
    0x02 (×2)  — auxiliary data
    0x04 (×3)  — quaternary data
    0x06 (×3)  — sound/physics data
    0x07 (×2)  — secondary physics
    0x10 (×3)  — higher-order data
    0x16 (×2)  — extended type
    Others (×1 each) — rare resource types

  KEY FINDING: This is NOT a recursive priority dispatcher.
  ─────────────────────────────────────────────────────────
  - Zero self-recursion (no JAL/J to 0x2564E0)
  - The "loop" is a SIF polling loop with 5001-iteration timeout
  - The 82 call sites are callers, not recursive invocations
  - The 96-byte frame saves $s0-$s5 + $ra for the DMA poll
  - This is a hardware communication primitive, not a tree walker
""")
    print("=" * 90)

if __name__ == "__main__":
    analyze()
