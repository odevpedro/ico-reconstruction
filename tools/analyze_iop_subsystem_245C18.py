#!/usr/bin/env python3
"""
Analysis of the IOP communication subsystem: 0x245C00-0x250000.

SCUS_971.13.elf — ICO USA PS2 (R5900, MIPS64 little-endian)

Uses Ghidra-verified labels from research/ghidra-exploration-2026-05-21.md.
"""

import struct
import sys
from collections import defaultdict, Counter

try:
    from capstone import Cs, CS_ARCH_MIPS, CS_MODE_MIPS64, CS_MODE_LITTLE_ENDIAN, CS_GRP_BRANCH_RELATIVE, CS_GRP_JUMP
    from capstone.mips import *
except ImportError:
    print("ERROR: pip3 install capstone"); sys.exit(1)

try:
    from elftools.elf.elffile import ELFFile
except ImportError:
    print("ERROR: pip3 install pyelftools"); sys.exit(1)

ELF_PATH = "/home/hoper/Documentos/repos/ico-reconstruction/.local/extracted/SCUS_971.13.elf"
RANGE_START = 0x245C00
RANGE_END   = 0x250000

# ── Ghidra-verified labels (from ghidra-exploration-2026-05-21.md) ────────────
LABELS = {
    # SIF RPC layer
    0x245C18: "sceSifInitRpc",
    0x245DE0: "_sceRpcGetPacket",
    0x245ED8: "_sceRpcGetFPacket2",
    0x245F18: "_request_end",
    0x245FD0: "_request_rdata",        # Ghidra name (PAL: sceSifSetIopAddr)
    0x246030: "sceSifGetOtherData",
    0x2461D8: "_request_bind",
    0x246288: "sceSifBindRpc",
    0x246458: "sceSifCallRpc",        # CENTRAL IOP RPC CALL
    0x246688: "sceSifSetRpcQueue",
    0x246720: "sceSifRegisterRpc",
    0x2467F0: "sceSifRemoveRpc",
    0x246888: "sceSifRemoveRpcQueue",
    0x246918: "sceSifGetNextRequest",
    0x246970: "sceSifExecRequest",
    # sceFs I/O layer
    0x246B38: "sceSifRpcLoop",
    0x246B78: "_sceFsIobSemaMK",
    0x246BD8: "new_iob",
    0x246C60: "get_iob",
    0x247088: "_sceFsSemInit",
    0x247108: "_sceFsSigSema",
    0x247118: "sceFsInit",
    0x2472B8: "_fs_version",
    0x247348: "sceFsReset",
    0x247380: "sceOpen",
    0x247608: "sceClose",
    0x247788: "sceLseek",
    0x2479C0: "sceRead",
    0x247C30: "sceWrite",
    0x247EF0: "sceIoctl",
    0x248240: "sceIoctl2",
    0x2485C0: "sceCdStRead",          # Ghidra confirmed
    0x2485E0: "sceMkdir",
    0x2487B0: "sceFormat",
    0x248A20: "sceAddDrv",
    0x248B60: "sceDopen",
    0x248C00: "sceDclose",
    0x248D68: "sceDread",
    0x248EC0: "sceGetstat",
    0x249060: "sceChstat",
    0x2492A0: "sceRename",
    0x2494B0: "sceSync",
    0x249648: "sceMount",
    0x2498D8: "sceLseek64",
    0x249B10: "sceDevctl",
    0x249D40: "sceSymlink",
    0x249F20: "sceReadlink",
    0x24A0E8: "sceSifInitIopHeap",
    0x24A348: "_lf_bind",
    0x24A4D8: "sceSifLoadFileReset",
    0x24A510: "_sceSifLoadModuleBuffer",
    0x24A758: "_sceSifLoadModule",
    0x24A980: "sceSifLoadModule",
    0x24A9C0: "_sceSifLoadElfPart",
    0x24AB10: "sceSifGetIopAddr",
    0x24AC00: "sceSifSetIopAddr",
    0x24ACE0: "sceSifResetIop",
    0x24AE10: "sceSifIsAliveIop",
    0x24AE38: "sceSifSyncIop",
    0x24AE80: "sceSifRebootIop",
    0x24AF90: "_DumpTLB",
    0x24B1D0: "kExpandScratchPad",
    0x24B300: "SetTLBHandler",
    0x24B3F8: "InitTLBFunctions",
    0x24B478: "iSetTLBEntry",
    0x24B498: "iGetTLBEntry",
    0x24B4A8: "ProbeTLBEntry",
    # SIF command layer
    0x24B8D8: "sceSifInitCmd",
    0x24BB58: "sceSifExitCmd",
    0x24BB90: "sceSifSetCmdBuffer",
    0x24BC18: "_sceSifSendCmd",
    0x24BDD0: "_sceSifCmdIntrHdlr",
    # CDVD layer
    0x24BFD0: "sceCdDelayThread",
    0x24C038: "sceCdCallback",
    0x24C090: "_sceCd_cd_callback",
    0x24C130: "_Cdvd_cbLoop",
    0x24C1F0: "sceCdInitEeCB",
    0x24C2C8: "_sceCd_cd_read_intr",
    0x24C368: "cmd_sem_init",
    0x24C400: "cdvd_exit",
    0x24C480: "sceCdPOffCallback",
    0x24C530: "PowerOffCB",
    0x24C6B8: "sceCdSearchFile",
    0x24C9B8: "_sceCd_ncmd_prechk",
    0x24CB28: "sceCdNcmdDiskReady",
    0x24CBC0: "sceCdSync",
    0x24CC60: "sceCdSyncS",
    0x24CCD0: "_sceCd_scmd_prechk",
    0x24CE40: "sceCdInit",
    0x24D120: "sceCdDiskReady",
    0x24D318: "sceCdMmode",
    0x24D3E0: "sceCdRead",
    0x24D5C4: "sceCdReadIOPm",
    0x24D718: "sceCdGetDiskType",
    0x24D7B0: "sceCdGetError",
    0x24D848: "sceCdStatus",
    0x24D900: "sceCdBreak",
    0x24D9B8: "sceCdStInit",
    0x24D9E8: "sceCdStStart",
    0x24DA20: "sceCdStSeekF",
    0x24DA50: "sceCdStSeek",
    0x24DA80: "sceCdStStop",
    0x24DAB8: "sceCdStRead",
    0x24DCE8: "sceCdStStat",
    0x24DD30: "sceCdStream",
    0x24DE98: "_send_to_iop",
    # Pad layer
    0x24DFC8: "scePadInit",
    0x24E108: "scePadInit2",
    0x24E1A8: "scePadEnd",
    0x24E228: "scePadPortOpen",
    0x24E428: "scePadPortClose",
    0x24E4C8: "_padGetState",
    0x24E53C: "scePadGetState",
    0x24E58C: "scePadGetReqState",
    0x24E60C: "scePadSetMainMode",
    0x24E6A8: "_padSetReqState",
    0x24E6BC: "scePadStateIntToStr",
    0x24E724: "scePadReqIntToStr",
    0x24E7AC: "scePadInfoAct",
    0x24E8CC: "scePadInfoComb",
    0x24E9EC: "scePadInfoMode",
    0x24EB10: "scePadSetMainMode",
    0x24EBC8: "scePadSetActDirect",
    0x24EC80: "scePadSetActAlign",
    0x24ED6C: "scePadGetInfoMode",
    0x24EE10: "scePadSetButtonInfo",
    0x24EED4: "_padUtilSendRpc",
    0x24EF34: "scePadGetFrameCount",
    0x24EF8C: "scePadRead",
    0x24EFD0: "scePadSetVrefParam",
    0x24F0A0: "scePadGetPortMax",
    0x24F108: "scePadGetSlotMax",
    0x24F170: "scePadInfoPressMode",
    0x24F1D8: "scePadEnterPressMode",
    0x24F240: "sceMcInit",
    0x24F3F8: "_lmcGetClientPtr",
    0x24F428: "sceMcChangeThreadPriority",
    0x24F4E0: "sceMcGetSlotMax",
    0x24F5A0: "sceMcOpen",
    0x24F6D8: "sceMcMkdir",
    0x24F710: "sceMcClose",
    0x24F7C8: "sceMcSeek",
    0x24F8A0: "mceIntrReadFixAlign",
    0x24F930: "sceMcRead",
    0x24FA50: "sceMcWrite",
    0x24FBD0: "mcHearAlarm",
    0x24FBF8: "mcDelayThread",
    0x24FC40: "sceMcSync",
    0x24FD78: "sceMcGetInfo",
    0x24FF00: "sceMcGetDir",
}

# boyAI / entity handler functions to cross-reference
BOYAI = {
    0x1C1A98: "boy_hC",
    0x1C1C48: "synchronizeMotionOutputOriginForGirl",
    0x1C1EA8: "boy_dispCrown",
    0x1C1F58: "boy_hA",
    0x1C20A8: "boy_float_accum",
    0x1C2170: "boy_activate",
    0x153478: "boy_init",
    0x164440: "enemy1_init",
    0x1CE220: "enemy1_hC",
    0x1CE3C0: "enemy1_hB",
}

def load_segments(path):
    segs = []
    with open(path, "rb") as f:
        elf = ELFFile(f)
        for seg in elf.iter_segments():
            if seg.header.p_type == "PT_LOAD":
                segs.append((seg.data(), seg.header.p_vaddr))
    return segs

def read_bytes(segs, va, size):
    for data, vaddr in segs:
        off = va - vaddr
        if 0 <= off < len(data):
            return data[off:off+size]
    return None

def read_u32(segs, va):
    d = read_bytes(segs, va, 4)
    return struct.unpack("<I", d)[0] if d else None

def disasm_func(segs, start, max_insns=500):
    md = Cs(CS_ARCH_MIPS, CS_MODE_MIPS64 | CS_MODE_LITTLE_ENDIAN)
    md.detail = True
    code = read_bytes(segs, start, max_insns * 4)
    if not code:
        return []
    insns = []
    for insn in md.disasm(code, start, count=max_insns):
        insns.append(insn)
        if insn.mnemonic == "jr" and insn.operands:
            if insn.operands[0].type == 2:
                r = insn.reg_name(insn.operands[0].reg)
                if r in ("ra", "$31"):
                    # include delay slot
                    dc = read_bytes(segs, insn.address + 4, 4)
                    if dc:
                        for di in md.disasm(dc, insn.address + 4, count=1):
                            insns.append(di)
                    break
    return insns

def find_function_starts(segs, start, end):
    """Find function starts by detecting `addiu $sp,$sp,-N` prologues."""
    md = Cs(CS_ARCH_MIPS, CS_MODE_MIPS64 | CS_MODE_LITTLE_ENDIAN)
    md.detail = True
    code = read_bytes(segs, start, end - start)
    if not code:
        return []
    starts = []
    for insn in md.disasm(code, start, count=(end - start) // 4):
        if insn.mnemonic == "addiu" and len(insn.operands) == 3:
            r0 = insn.reg_name(insn.operands[0].reg)
            r1 = insn.reg_name(insn.operands[1].reg)
            if r0 == "sp" and r1 == "sp" and insn.operands[2].type == 1:
                imm = insn.operands[2].imm
                if imm < 0 and 0x10 <= (0x10000 + imm) <= 0x4000:
                    starts.append(insn.address)
    # Deduplicate: only keep if not within 16 bytes of previous
    deduped = []
    for s in sorted(set(starts)):
        if not deduped or s - deduped[-1] >= 16:
            deduped.append(s)
    return deduped

def label(va):
    return LABELS.get(va, f"sub_{va:08X}")

def main():
    print("=" * 100)
    print("IOP COMMUNICATION SUBSYSTEM ANALYSIS — 0x245C00-0x250000")
    print("SCUS_971.13.elf — ICO USA PS2 (R5900)")
    print("=" * 100)

    segs = load_segments(ELF_PATH)
    print(f"Loaded {len(segs)} PT_LOAD segments")

    # ── Known entries (merge prologue detection with Ghidra labels) ─────────
    print("\n" + "─" * 100)
    print("STEP 1: FUNCTION MAP (Ghidra labels + prologue detection)")
    print("─" * 100)

    # Get all known labels in range
    known = sorted(va for va in LABELS if RANGE_START <= va < RANGE_END)
    # Also detect prologues
    detected = find_function_starts(segs, RANGE_START, RANGE_END)

    # Merge: use known labels as primary, add detected ones that are close
    all_starts = set(known)
    for d in detected:
        # Check if close to an existing label
        close = False
        for k in known:
            if abs(d - k) < 16:
                close = True
                break
        if not close:
            all_starts.add(d)

    all_starts = sorted(all_starts)
    print(f"  Known labels in range: {len(known)}")
    print(f"  Prologue-detected:     {len(detected)}")
    print(f"  Merged unique starts:  {len(all_starts)}")

    # ── Analyze each function ───────────────────────────────────────────────
    print("\n" + "─" * 100)
    print("STEP 2: FUNCTION ANALYSIS")
    print("─" * 100)

    func_data = {}  # va -> analysis dict

    for start in all_starts:
        lbl = label(start)
        insns = disasm_func(segs, start)
        if not insns:
            continue

        end = insns[-1].address + 4
        size = end - start

        jals = set()
        jalr_count = 0
        struct_offs = set()
        gp_offs = set()
        syscalls = set()
        bbs = 0

        for insn in insns:
            mn = insn.mnemonic
            if mn == "jal" and insn.operands:
                jals.add(insn.operands[0].imm)
            if mn == "jalr":
                jalr_count += 1
            if mn == "syscall":
                syscalls.add(insn.address)
            if CS_GRP_BRANCH_RELATIVE in insn.groups or CS_GRP_JUMP in insn.groups:
                bbs += 1
            if insn.operands and len(insn.operands) >= 2:
                op = insn.operands[-1]
                if op.type == CS_OP_MEM:
                    base = insn.reg_name(op.mem.base) if op.mem.base else ""
                    disp = op.mem.disp
                    if base == "gp" and disp != 0:
                        gp_offs.add(disp & 0xFFFFFFFF)
                    if disp != 0 and 0 < abs(disp) < 0x8000:
                        struct_offs.add(disp & 0xFFFFFFFF)

        internal = sorted(t for t in jals if RANGE_START <= t < RANGE_END)
        external = sorted(t for t in jals if t < RANGE_START or t >= RANGE_END)

        func_data[start] = {
            "label": lbl,
            "end": end,
            "size": size,
            "insns": len(insns),
            "jals": sorted(jals),
            "internal": internal,
            "external": external,
            "jalr": jalr_count,
            "struct_offs": struct_offs,
            "gp_offs": gp_offs,
            "syscalls": sorted(syscalls),
            "bbs": bbs,
        }

        ext_names = []
        for t in external[:12]:
            ext_names.append(label(t) if t in LABELS else f"ext_{t:08X}")
        int_names = [label(t) for t in internal[:8]]

        print(f"\n  0x{start:08X} {lbl}")
        print(f"    {size:5d} bytes | {len(insns):3d} insns | {len(jals)} JAL ({len(internal)} int, {len(external)} ext) | {len(struct_offs)} struct | {len(gp_offs)} GP | {jalr_count} JALR")
        if int_names:
            print(f"    → {', '.join(int_names)}")
        if ext_names:
            more = f" +{len(external)-12} more" if len(external) > 12 else ""
            print(f"    → ext: {', '.join(ext_names)}{more}")

    # ── Cross-references from boyAI ─────────────────────────────────────────
    print("\n" + "─" * 100)
    print("STEP 3: boyAI → IOP SUBSYSTEM CROSS-REFERENCES")
    print("─" * 100)

    md = Cs(CS_ARCH_MIPS, CS_MODE_MIPS64 | CS_MODE_LITTLE_ENDIAN)
    md.detail = True

    found_refs = []
    for va, name in BOYAI.items():
        code = read_bytes(segs, va, 300 * 4)
        if not code:
            continue
        for insn in md.disasm(code, va, count=300):
            if insn.mnemonic == "jal" and insn.operands:
                target = insn.operands[0].imm
                if RANGE_START <= target < RANGE_END:
                    found_refs.append((name, va, insn.address, target, label(target)))
            if insn.mnemonic == "jr" and insn.operands:
                if insn.operands[0].type == 2:
                    r = insn.reg_name(insn.operands[0].reg)
                    if r in ("ra", "$31"):
                        break

    if found_refs:
        print(f"\n  Found {len(found_refs)} direct JAL calls from boyAI into subsystem:\n")
        for caller, caller_va, site, target, callee in found_refs:
            print(f"    {caller:30s} @ 0x{caller_va:08X}")
            print(f"      └─ JAL at 0x{site:08X} → {callee} @ 0x{target:08X}")
    else:
        print("\n  No direct JAL calls from boyAI → IOP subsystem.")
        print("  boyAI uses IOP indirectly via higher-level engine layers.")

    # Reverse: subsystem → boyAI
    rev_refs = []
    for va, fd in func_data.items():
        for t in fd["jals"]:
            if t in BOYAI:
                rev_refs.append((fd["label"], va, t, BOYAI[t]))
    if rev_refs:
        print(f"\n  Reverse: subsystem → boyAI ({len(rev_refs)} calls):")
        for caller, cva, target, callee in rev_refs:
            print(f"    {caller} @ 0x{cva:08X} → {callee} @ 0x{target:08X}")

    # ── External dependency analysis ─────────────────────────────────────────
    print("\n" + "─" * 100)
    print("STEP 4: EXTERNAL DEPENDENCIES (most-called outside functions)")
    print("─" * 100)

    ext_count = Counter()
    for fd in func_data.values():
        for t in fd["external"]:
            ext_count[t] += 1

    print(f"\n  Top external call targets (count ≥ 3):\n  {'Count':>5s}  {'Address':>10s}  {'Label'}")
    print("  " + "─" * 60)
    for t, cnt in ext_count.most_common(30):
        if cnt >= 3:
            lbl = label(t) if t in LABELS else f"ext_{t:08X}"
            print(f"  {cnt:5d}  0x{t:08X}  {lbl}")

    # ── Internal call graph (hub analysis) ───────────────────────────────────
    print("\n" + "─" * 100)
    print("STEP 5: INTERNAL CALL GRAPH — HUB ANALYSIS")
    print("─" * 100)

    called_by = defaultdict(list)
    calls_out = {}
    for va, fd in func_data.items():
        calls_out[va] = fd["internal"]
        for t in fd["internal"]:
            called_by[t].append(va)

    print(f"\n  Hub functions (called by most internal callers):\n  {'Callers':>7s}  {'Address':>10s}  {'Label'}")
    print("  " + "─" * 60)
    for t, callers in sorted(called_by.items(), key=lambda x: -len(x[1])):
        if len(callers) >= 3:
            print(f"  {len(callers):7d}  0x{t:08X}  {label(t)}")

    # ── Layer classification ─────────────────────────────────────────────────
    print("\n" + "─" * 100)
    print("STEP 6: LAYER CLASSIFICATION")
    print("─" * 100)

    layers = defaultdict(list)
    for va in sorted(func_data.keys()):
        lbl = func_data[va]["label"]
        if lbl.startswith("sceSif") or lbl.startswith("_sceRpc") or lbl.startswith("_request"):
            layers["SIF RPC (EE→IOP bridge)"].append((va, lbl))
        elif lbl.startswith("sceFs") or lbl.startswith("_sceFs") or lbl.startswith("_fs_") or lbl in ("sceOpen", "sceClose", "sceRead", "sceWrite", "sceLseek", "sceLseek64", "sceIoctl", "sceIoctl2", "sceMkdir", "sceFormat", "sceAddDrv", "sceDopen", "sceDclose", "sceDread", "sceGetstat", "sceChstat", "sceRename", "sceSync", "sceMount", "sceDevctl", "sceSymlink", "sceReadlink", "new_iob", "get_iob"):
            layers["sceFs (file system over IOP)"].append((va, lbl))
        elif lbl.startswith("sceSifLoad") or lbl.startswith("sceSifInit") or lbl.startswith("sceSifAlloc") or lbl.startswith("sceSifFree") or lbl.startswith("_sceSif"):
            layers["sceSifLoad (module/ELF loader)"].append((va, lbl))
        elif lbl.startswith("_DumpTLB") or lbl.startswith("SetTLB") or lbl.startswith("InitTLB") or lbl.startswith("iSetTLB") or lbl.startswith("iGetTLB") or lbl.startswith("ProbeTLB") or lbl.startswith("kExpandScratchPad"):
            layers["TLB / Memory management"].append((va, lbl))
        elif lbl.startswith("sceSifInitCmd") or lbl.startswith("sceSifExitCmd") or lbl.startswith("sceSifSetCmd") or lbl.startswith("_sceSifSendCmd") or lbl.startswith("_sceSifCmd"):
            layers["SIF command layer"].append((va, lbl))
        elif lbl.startswith("sceCd") or lbl.startswith("_sceCd") or lbl.startswith("_Cdvd") or lbl.startswith("cmd_sem") or lbl.startswith("cdvd_") or lbl.startswith("PowerOff"):
            layers["CDVD (disc I/O)"].append((va, lbl))
        elif lbl.startswith("scePad") or lbl.startswith("_pad"):
            layers["Pad (controller input)"].append((va, lbl))
        elif lbl.startswith("sceMc") or lbl.startswith("_lmc") or lbl.startswith("mc"):
            layers["Mc (memory card)"].append((va, lbl))
        elif lbl.startswith("_send_to_iop"):
            layers["Low-level IOP comm"].append((va, lbl))
        else:
            layers["Unclassified"].append((va, lbl))

    for layer_name, funcs in sorted(layers.items()):
        print(f"\n  ┌─ {layer_name} ({len(funcs)} functions)")
        for va, lbl in funcs[:15]:
            fd = func_data.get(va, {})
            size = fd.get("size", "?")
            ext_cnt = len(fd.get("external", []))
            print(f"  │  0x{va:08X}  {lbl:30s}  {size:>5} bytes  {ext_cnt} ext calls")
        if len(funcs) > 15:
            print(f"  │  ... +{len(funcs)-15} more")
        print(f"  └─")

    # ── Hub functions (most called from within) ──────────────────────────────
    print("\n" + "─" * 100)
    print("STEP 7: CRITICAL HUB FUNCTIONS")
    print("─" * 100)

    # sceSifCallRpc is expected to be the main hub
    print("\n  The central dispatch function is sceSifCallRpc (0x246458).")
    print("  This is the EE→IOP RPC call primitive — every IOP communication")
    print("  flows through this function.")

    # Count how many functions call sceSifCallRpc
    callrpc_callers = called_by.get(0x246458, [])
    print(f"\n  Functions that call sceSifCallRpc: {len(callrpc_callers)}")
    for c in sorted(callrpc_callers)[:20]:
        print(f"    {label(c)} @ 0x{c:08X}")

    # Count callers of sceSifBindRpc
    bindrpc_callers = called_by.get(0x246288, [])
    print(f"\n  Functions that call sceSifBindRpc: {len(bindrpc_callers)}")
    for c in sorted(bindrpc_callers)[:20]:
        print(f"    {label(c)} @ 0x{c:08X}")

    # Count callers of sceSifInitRpc
    initrpc_callers = called_by.get(0x245C18, [])
    print(f"\n  Functions that call sceSifInitRpc: {len(initrpc_callers)}")
    for c in sorted(initrpc_callers)[:20]:
        print(f"    {label(c)} @ 0x{c:08X}")

    # ── Summary ──────────────────────────────────────────────────────────────
    print("\n" + "═" * 100)
    print("SUBSYSTEM ARCHITECTURE SUMMARY")
    print("═" * 100)

    total_funcs = len(func_data)
    total_jals = sum(len(fd["jals"]) for fd in func_data.values())
    total_int = sum(len(fd["internal"]) for fd in func_data.values())
    total_ext = sum(len(fd["external"]) for fd in func_data.values())
    total_struct = sum(len(fd["struct_offs"]) for fd in func_data.values())
    total_gp = sum(len(fd["gp_offs"]) for fd in func_data.values())

    print(f"""
  Range: 0x{RANGE_START:08X}-0x{RANGE_END:08X} ({RANGE_END-RANGE_START:,} bytes)

  FUNCTIONS: {total_funcs}
  JAL TARGETS: {total_jals} total ({total_int} internal, {total_ext} external)
  STRUCT OFFSETS: {total_struct} unique
  GP-RELATIVE REFS: {total_gp} unique

  ╔═══════════════════════════════════════════════════════════════════════════════╗
  ║                     PS2 SDK I/O SUBSYSTEM LAYERS                           ║
  ╠═══════════════════════════════════════════════════════════════════════════════╣
  ║                                                                             ║
  ║  Layer 5: sceMc*    ──── Memory Card (save/load)                           ║
  ║  Layer 4: scePad*   ──── Controller (input, rumble, pressure)              ║
  ║  Layer 3: sceCd*    ──── CDVD (disc read, stream, search)                  ║
  ║  Layer 2: sceFs*    ──── File System (open/read/write/ioctl over IOP)      ║
  ║  Layer 1: sceSif*   ──── SIF RPC (EE↔IOP communication primitive)          ║
  ║                                                                             ║
  ║  ─────────────────────────────────────────────────────────────────────────  ║
  ║  All layers share the same IOP communication path:                          ║
  ║                                                                             ║
  ║    EE Game Code                                                            ║
  ║       ↓                                                                    ║
  ║    sceCdRead / scePadRead / sceMcOpen  (high-level API)                   ║
  ║       ↓                                                                    ║
  ║    sceFsRead / sceFsOpen               (file system indirection)          ║
  ║       ↓                                                                    ║
  ║    sceSifCallRpc (0x246458)            ← CENTRAL HUB                     ║
  ║       ↓                                                                    ║
  ║    SIF DMA registers (0x10002000)      (hardware bridge)                  ║
  ║       ↓                                                                    ║
  ║    IOP processor                     (runs IOP-side modules)              ║
  ║       ↓                                                                    ║
  ║    CDVD hardware / PAD hardware / MC hardware                              ║
  ║                                                                             ║
  ╠═══════════════════════════════════════════════════════════════════════════════╣
  ║  KEY FINDINGS:                                                              ║
  ║                                                                             ║
  ║  1. sceSifCallRpc (0x246458) is the single chokepoint for ALL IOP          ║
  ║     communication. Every sceCd*, scePad*, sceMc* call eventually           ║
  ║     reaches this function.                                                  ║
  ║                                                                             ║
  ║  2. sceSifInitRpc (0x245C18) initializes the SIF RPC subsystem.           ║
  ║     This must be called before any IOP communication.                       ║
  ║                                                                             ║
  ║  3. boyAI does NOT call this subsystem directly. The call path is:         ║
  ║     boyAI → engine layer (kanban.c/backstage) → sceCd*/scePad* → SIF      ║
  ║                                                                             ║
  ║  4. The subsystem is statically linked PS2 SDK code (libds2h, libcdvd,     ║
  ║     libpad, libmc) — not game-specific code. ICO embeds these stubs        ║
  ║     directly rather than loading them as IOP modules.                       ║
  ║                                                                             ║
  ║  5. The SIF command layer (sceSifInitCmd, _sceSifSendCmd) provides a       ║
  ║     separate path for SIF commands (used for DMA control, not RPC).         ║
  ║                                                                             ║
  ║  6. The TLB functions (_DumpTLB, SetTLBEntry, etc.) are EE-side kernel     ║
  ║     primitives embedded in the ELF — not part of IOP communication.         ║
  ║                                                                             ║
  ║  7. The sceFs layer provides file system abstraction over the IOP           ║
  ║     filesystem RPC server — this is how ICO loads game data.               ║
  ║                                                                             ║
  ╚═══════════════════════════════════════════════════════════════════════════════╝

  CALL STACK DEPTH (typical boyAI → hardware):

    Layer 5:  boyAI / game code
    Layer 4:  sceCdRead (0x24D3E0)           ~120 bytes
    Layer 3:  _sceCd_ncmd_prechk (0x24C9B8)  validation
    Layer 2:  sceSifCallRpc (0x246458)        CENTRAL HUB
    Layer 1:  SIF DMA (0x10002000)
    Layer 0:  IOP processor → CDVD hardware

  DATA STRUCTURES OPERATED ON:

    - SIF RPC packet: 8+ bytes (command, endpoint, data ptr, size)
    - IOB (I/O Buffer): managed by new_iob/get_iob
    - CDVD command buffer: managed by cmd_sem_init / _sceCd_ncmd_prechk
    - Pad data buffer: managed by _padGetState / _padSetReqState
    - MC client: managed by _lmcGetClientPtr
    - Semaphore: managed by _sceFsSemInit / _sceFsSigSema

  ERROR HANDLING:

    - sceCdSync/sceCdSyncS: synchronous wait with timeout
    - _sceCd_ncmd_prechk/_sceCd_scmd_prechk: pre-check validation
    - sceCdGetError: error code retrieval
    - sceCdStatus: drive status polling
    - sceCdBreak: abort pending command

  ════════════════════════════════════════════════════════════════════════════════
""")
    print("=" * 100)
    print("END OF ANALYSIS")
    print("=" * 100)


if __name__ == "__main__":
    main()
