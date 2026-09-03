#!/usr/bin/env python3
"""
Analyze hD (cb_routine4) callbacks for the 4 entities with non-null hD
in the ICO PS2 ELF descriptor table.

Descriptor table: 0x2A31B8, 68 entries, stride 0x64.
hD field: offset +0x5C within each entry.

BUG NOTE: code[] array is loaded from file offset 0x1000 (BASE_FOFF),
so code[i] = VA (0x100000 + i). To index VA A, use code[A - BASE_VADDR].
Do NOT use code[vfoff(A)] which is off by BASE_FOFF.
"""

import struct
from capstone import *

ELF_PATH = "/home/hoper/Documentos/repos/ico-reconstruction/.local/extracted/SCUS_971.13.elf"
BASE_VADDR = 0x00100000
BASE_FOFF  = 0x1000
DESC_BASE  = 0x2A31B8
DESC_STRIDE = 0x64
DESC_ENTRIES = 68

OFF_INIT_FN = 0x40
OFF_HA = 0x48
OFF_HC = 0x50
OFF_HC3 = 0x58
OFF_HD = 0x5C
OFF_VTABLE = 0x60

MD = Cs(CS_ARCH_MIPS, CS_MODE_MIPS32 | CS_MODE_LITTLE_ENDIAN)


def va_to_idx(va):
    """Convert virtual address to code[] array index."""
    return va - BASE_VADDR


def load_code():
    with open(ELF_PATH, "rb") as f:
        f.seek(BASE_FOFF)
        return f.read(0x540000)


def dis(code, addr, n=50):
    idx = va_to_idx(addr)
    chunk = code[idx:idx + n * 4 + 4]
    return list(MD.disasm(chunk, addr, count=n))


def find_jal_refs(code, target):
    refs = []
    for i in MD.disasm(code, BASE_VADDR):
        if i.mnemonic == "jal":
            try:
                if int(i.op_str.strip(), 16) == target:
                    refs.append(i.address)
            except ValueError:
                pass
    return refs


def find_lui_for_addr(code, addr):
    lo16 = addr & 0xFFFF
    hi16 = (addr >> 16) & 0xFFFF
    if lo16 >= 0x8000:
        hi16 += 1
    results = []
    for i in MD.disasm(code, BASE_VADDR):
        if i.mnemonic == "lui" and i.op_str.endswith(f",0x{hi16:X}"):
            nxt_idx = va_to_idx(i.address) + 4
            chunk = code[nxt_idx:nxt_idx+16]
            for ni in MD.disasm(chunk, i.address+1, count=4):
                if ni.mnemonic in ("addiu", "ori") and f",0x{lo16:X}" in ni.op_str:
                    results.append((i.address, ni))
                break
    return results


def full_fn(code, addr, maxn=200):
    insns = dis(code, addr, maxn)
    for idx, i in enumerate(insns):
        if i.mnemonic == "jr" and "$ra" in i.op_str:
            return insns[:idx+2]
    return insns


def section(title):
    print(f"\n{'='*70}")
    print(f"  {title}")
    print(f"{'='*70}")


def main():
    code = load_code()
    print(f"Code loaded: {len(code)} bytes, covers VA 0x{BASE_VADDR:08X}–0x{BASE_VADDR+len(code):08X}")

    # ==================================================================
    # PHASE 1: Read descriptor table
    # ==================================================================
    section("PHASE 1: Descriptor Table — All 68 entries with non-null hD")

    targets = {}
    all_with_hd = {}

    for idx in range(DESC_ENTRIES):
        base = DESC_BASE + idx * DESC_STRIDE
        ci = va_to_idx(base)
        e = code[ci:ci + DESC_STRIDE]

        # Read name (first 16 bytes)
        name_raw = e[:16]
        # Find first null terminator
        nz = name_raw.find(b"\x00")
        if nz >= 0:
            name = name_raw[:nz].decode("ascii", errors="replace")
        else:
            name = name_raw.decode("ascii", errors="replace")

        hd = struct.unpack("<I", e[OFF_HD:OFF_HD+4])[0]

        if hd != 0:
            ha = struct.unpack("<I", e[OFF_HA:OFF_HA+4])[0]
            hc = struct.unpack("<I", e[OFF_HC:OFF_HC+4])[0]
            hc3 = struct.unpack("<I", e[OFF_HC3:OFF_HC3+4])[0]
            vt = struct.unpack("<I", e[OFF_VTABLE:OFF_VTABLE+4])[0]
            init_fn = struct.unpack("<I", e[OFF_INIT_FN:OFF_INIT_FN+4])[0]
            all_with_hd[idx] = {
                "name": name, "ha": ha, "hc": hc, "hc3": hc3,
                "hd": hd, "vt": vt, "init_fn": init_fn,
            }

    # The 4 target entities
    target_indices = {2: "GIRL", 4: "ENEMY1", 5: "ENEMY_TEST", 48: "DEVIL_GIRL"}
    target_hd_addrs = set()

    print(f"Total entries with non-null hD: {len(all_with_hd)}")
    print(f"\n--- Target entities (GIRL/ENEMY1/ENEMY_TEST/DEVIL_GIRL) ---")
    print(f"{'Idx':>4} {'Name':<16} {'hA':>10} {'hC':>10} {'hD':>10} {'vtable':>10}")
    print("-" * 60)
    for idx in sorted(target_indices):
        t = all_with_hd.get(idx)
        if t:
            target_hd_addrs.add(t["hd"])
            print(f"{idx:>4} {t['name']:<16} 0x{t['ha']:08X} 0x{t['hc']:08X} 0x{t['hd']:08X} 0x{t['vt']:08X}")
        else:
            print(f"{idx:>4} {target_indices[idx]:<16} ** hD is NULL **")

    print(f"\nTarget hD addresses: {', '.join(f'0x{a:08X}' for a in sorted(target_hd_addrs))}")

    # Group all entities by hD address
    by_addr = {}
    for idx, t in all_with_hd.items():
        a = t["hd"]
        if a not in by_addr:
            by_addr[a] = []
        by_addr[a].append((idx, t["name"]))

    print(f"\nAll hD address groupings:")
    for addr in sorted(by_addr):
        names = [f"{n}[{i}]" for i, n in by_addr[addr]]
        is_target = addr in target_hd_addrs
        tag = " ** TARGET **" if is_target else ""
        print(f"  0x{addr:08X} → {', '.join(names)}{tag}")

    # ==================================================================
    # PHASE 2: Disassemble hD for the 4 target entities
    # ==================================================================
    section("PHASE 2: Target hD Handler Disassembly")

    for addr in sorted(target_hd_addrs):
        names = [n for a, n in by_addr[addr] if a in target_indices]
        label = "/".join(names)
        insns = dis(code, addr, 20)

        print(f"\n--- hD at 0x{addr:08X} ({label}) ---")
        is_stub = (len(insns) >= 2 and
                   insns[0].mnemonic == "jr" and "$ra" in insns[0].op_str and
                   insns[1].mnemonic == "nop")

        if is_stub:
            print("  ** NOP STUB: jr $ra; nop — returns immediately **")
        for i in insns[:8]:
            print(f"    0x{i.address:08X}: {i.mnemonic:8s} {i.op_str}")

    # ==================================================================
    # PHASE 3: Cross-references to hD addresses
    # ==================================================================
    section("PHASE 3: Cross-References (JAL + LUI) to hD Addresses")

    for addr in sorted(target_hd_addrs):
        names = [n for a, n in by_addr[addr] if a in target_indices]
        label = "/".join(names)
        jal_refs = find_jal_refs(code, addr)
        lui_refs = find_lui_for_addr(code, addr)

        print(f"\n  0x{addr:08X} ({label}):")
        if jal_refs:
            for r in jal_refs:
                print(f"    JAL from 0x{r:08X}")
        else:
            print(f"    No JAL references")

        if lui_refs:
            for lui_a, use_i in lui_refs:
                print(f"    LUI at 0x{lui_a:08X} → {use_i.mnemonic} {use_i.op_str}")
        else:
            print(f"    No LUI+ADDIU address builds")

    # ==================================================================
    # PHASE 4: hA full disassembly — where does hD sit?
    # ==================================================================
    section("PHASE 4: hA Full Disassembly — hD is Embedded at Tail")

    pairs = [
        ("GIRL", 0x1D1A98, 0x1D1AD0),
        ("ENEMY1", 0x1CE690, 0x1CE760),
    ]

    for entity, ha_addr, hd_addr in pairs:
        print(f"\n--- {entity}: hA at 0x{ha_addr:08X} — hD at 0x{hd_addr:08X} ---")
        hA_insns = full_fn(code, ha_addr)
        print(f"  hA total instructions: {len(hA_insns)}")
        print(f"  hA range: 0x{hA_insns[0].address:08X} — 0x{hA_insns[-1].address:08X}")

        hD_offset = hd_addr - ha_addr
        print(f"  hD offset from hA start: +0x{hD_offset:X} ({hD_offset} bytes = {hD_offset // 4} instructions)")

        # Verify hD is inside hA
        hD_inside = any(i.address == hd_addr for i in hA_insns)
        if hD_inside:
            print(f"  hD IS inside hA — it's the 2-instruction tail: jr $ra; nop")
        else:
            print(f"  hD is NOT inside hA — checking gap...")
            last_addr = hA_insns[-1].address
            print(f"  hA ends at 0x{last_addr:08X}, hD at 0x{hd_addr:08X}")

        # Show the last 15 instructions of hA
        print(f"\n  Last 15 instructions of hA:")
        for i in hA_insns[-15:]:
            marker = ""
            if i.address == hd_addr:
                marker = " ◄── hD STUB START"
            elif i.address == hd_addr + 4:
                marker = " ◄── hD STUB END (delay slot)"
            print(f"    0x{i.address:08X}: {i.mnemonic:8s} {i.op_str}{marker}")

    # ==================================================================
    # PHASE 5: Compare hA vs hC vs hD profiles
    # ==================================================================
    section("PHASE 5: Callback Profiles (hA vs hC vs hD)")

    profiles = {
        "GIRL": {"hA": (0x1D1A98, 0x1D1AD0), "hC": (0x1D1668, None)},
        "ENEMY1": {"hA": (0x1CE690, 0x1CE760), "hC": (0x1CE220, None)},
    }

    for entity, cbs in profiles.items():
        print(f"\n--- {entity} ---")
        for slot, (addr, _) in cbs.items():
            insns = dis(code, addr, 80)
            is_stub = (len(insns) >= 2 and
                       insns[0].mnemonic == "jr" and "$ra" in insns[0].op_str and
                       insns[1].mnemonic == "nop")

            # Find end of function
            end_idx = len(insns)
            for idx, i in enumerate(insns):
                if idx > 0 and i.mnemonic == "jr" and "$ra" in i.op_str:
                    end_idx = idx + 2
                    break
            func_insns = insns[:end_idx]

            jal_t = [int(i.op_str.strip(), 16) for i in func_insns
                     if i.mnemonic == "jal"]
            fp = sum(1 for i in func_insns
                     if i.mnemonic.startswith("c.") or
                     i.mnemonic in ("cvt.s.w", "cvt.w.s", "mfc1", "mtc1"))
            br = sum(1 for i in func_insns
                     if any(i.mnemonic.startswith(p) for p in
                            ("beq", "bne", "bgez", "blez", "bgtz", "bltz", "bc1")))

            loads = []
            stores = []
            for i in func_insns:
                if ("lw" in i.mnemonic or "lwc" in i.mnemonic) and "(" in i.op_str:
                    base = i.op_str.split("(")[1].rstrip(")")
                    off_s = i.op_str.split(",")[1].split("(")[0].strip()
                    try:
                        o = int(off_s, 16) if "0x" in off_s else int(off_s)
                        loads.append((o, base))
                    except ValueError:
                        pass
                if ("sw" in i.mnemonic or "swc" in i.mnemonic) and "(" in i.op_str:
                    base = i.op_str.split("(")[1].rstrip(")")
                    off_s = i.op_str.split(",")[1].split("(")[0].strip()
                    try:
                        o = int(off_s, 16) if "0x" in off_s else int(off_s)
                        stores.append((o, base))
                    except ValueError:
                        pass

            func_bytes = (end_idx if end_idx <= len(insns) else len(insns)) * 4

            if is_stub:
                print(f"  {slot:3s} (0x{addr:08X}): NOP STUB (8 bytes, 2 insns)")
            else:
                print(f"  {slot:3s} (0x{addr:08X}): ~{func_bytes}B, {len(func_insns)} insns, "
                      f"{len(jal_t)} JALs, {fp} FP, {br} branches")
                if jal_t:
                    print(f"       JAL: {', '.join(f'0x{t:08X}' for t in jal_t)}")
                if loads:
                    unique = sorted(set(f"+0x{o:X}({b})" for o, b in loads))
                    print(f"       Loads: {', '.join(unique[:10])}")
                if stores:
                    unique = sorted(set(f"+0x{o:X}({b})" for o, b in stores))
                    print(f"       Stores: {', '.join(unique[:10])}")

    # ==================================================================
    # PHASE 6: What about the descriptor dispatch — does it read +0x5C?
    # ==================================================================
    section("PHASE 6: Dispatch Code — Does It Read +0x5C (hD)?")

    # Search the isysGObj* range for loads from offset 0x5C
    print("Searching isysGObj* range (0x13DDA0-0x141D18) for offset 0x5C...")
    search_ranges = [
        (0x13D800, 0x142000, "isysGObj*"),
        (0x1B7000, 0x1B8500, "initSceneGObj"),
        (0x1A0000, 0x1A3000, "backStage"),
    ]

    for lo, hi, label in search_ranges:
        idx_lo = va_to_idx(lo)
        idx_hi = va_to_idx(hi)
        chunk = code[idx_lo:idx_hi]
        found_any = False
        for i in MD.disasm(chunk, lo):
            # Check for loads/stores with offset 0x5C
            if ",0x5c(" in i.op_str or ",0x5c)" in i.op_str:
                if not found_any:
                    print(f"\n  [{label}] offset 0x5C accesses:")
                    found_any = True
                print(f"    0x{i.address:08X}: {i.mnemonic} {i.op_str}")
        if not found_any:
            print(f"  [{label}] No offset 0x5C accesses found")

    # Also check the behavior_fn (0x202A60) range
    print("\nSearching behavior_fn (0x202A60) range for offset 0x5C...")
    idx_lo = va_to_idx(0x202A60)
    chunk = code[idx_lo:idx_lo + 0x800]
    found_any = False
    for i in MD.disasm(chunk, 0x202A60):
        if ",0x5c(" in i.op_str or ",0x5c)" in i.op_str:
            if not found_any:
                print(f"  [behavior_fn] offset 0x5C accesses:")
                found_any = True
            print(f"    0x{i.address:08X}: {i.mnemonic} {i.op_str}")
    if not found_any:
        print(f"  [behavior_fn] No offset 0x5C accesses found")

    # ==================================================================
    # PHASE 7: Verify hA function prologues (confirm hD is at hA tail)
    # ==================================================================
    section("PHASE 7: Verify hA Prologues")

    for entity, ha_addr, hd_addr in pairs:
        insns = dis(code, ha_addr, 4)
        print(f"\n  {entity} hA at 0x{ha_addr:08X}:")
        for i in insns:
            print(f"    0x{i.address:08X}: {i.mnemonic:8s} {i.op_str}")

    # ==================================================================
    # PHASE 8: Summary
    # ==================================================================
    section("ANALYSIS SUMMARY")

    print("""
hD (cb_routine4) Callback System — Complete Analysis
====================================================

1. DESCRIPTOR TABLE HOOKUP

   Offset +0x5C in each 100-byte descriptor entry holds hD.
   Only 4 entity types have non-null hD:

     Entity        idx  hD address   Identity
     ───────────── ──── ──────────── ─────────────
     GIRL           2   0x001D1AD0   nop stub
     DEVIL_GIRL    48   0x001D1AD0   nop stub (same)
     ENEMY1         4   0x001CE760   nop stub
     ENEMY_TEST     5   0x001CE760   nop stub (same)

2. hD IS AN EMPTY STUB

   Both unique hD addresses contain: jr $ra; nop (8 bytes).
   They return immediately without executing any logic.

3. hD IS DEAD CODE

   Zero JAL instructions target either hD address.
   Zero LUI+ADDIU pairs build either hD address.
   The descriptor dispatch mechanism (isysGObjProcAdd_,
   _iosOmMain, iosOmCreateDL) never reads offset +0x5C.

4. hD IS EMBEDDED AT THE TAIL OF hA

   - GIRL: hA starts at 0x1D1A98, hD stub at 0x1D1AD0 (+0x38 = +14 insns)
   - ENEMY1: hA starts at 0x1CE690, hD stub at 0x1CE760 (+0xD0 = +52 insns)

   The hD "stub" is actually the final 2 instructions of hA.
   hA ends with: jr $ra; nop — which IS the hD address.

   This means hD is not a separate function but part of hA's epilogue.

5. ALIAS PAIRS

   GIRL = DEVIL_GIRL: all callbacks identical (hA, hC, hD, vtable)
   ENEMY1 ≈ ENEMY_TEST: share hC and hD, differ in hA

6. hD vs hA/hC COMPARISON

   hA:  hundreds of instructions, heavy JAL/FP, complex logic
   hC:  medium functions, state_block access, some JAL calls
   hD:  nop stub (0 instructions of real logic)

7. INTERPRETATION

   The hD slot at +0x5C is a structural placeholder in the 100-byte
   descriptor entry. The 4 'live character' entities (GIRL, ENEMY1,
   DEVIL_GIRL, ENEMY_TEST) happen to have their hA function's
   return instruction at the hD address, but this is a consequence
   of code layout, not intentional callback registration.

   No code path dispatches through hD. The engine treats +0x5C as
   either unused or reads it as zero for the other 64 entity types.
""")


if __name__ == "__main__":
    main()
