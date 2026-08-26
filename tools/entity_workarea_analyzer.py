#!/usr/bin/env python3
"""
entity_workarea_analyzer.py - Map entity work area structure from PCSX2 runtime log.

The entity work area pointer is loaded from GP-0x6E08/GP-0x6E0C.
This script analyzes captured entity pointer values, their DL slot bindings,
world_state affinity, and register-level context.

Usage:
    python3 tools/entity_workarea_analyzer.py [.local/pcsx2-logs/<session>.jsonl]
"""

import json
import sys
from collections import defaultdict, Counter


def hx(s):
    return int(s, 16)


def region(v):
    if v == 0:        return "NULL"
    if v < 0x100000:  return "ELF"
    if v < 0x280000:  return ".data"
    if v < 0x6A0000:  return "BSS"
    if v < 0x800000:  return "BSS-ext"
    if v < 0x2000000: return "Heap"
    return "HeapHi"


def main():
    logfile = (sys.argv[1] if len(sys.argv) > 1
               else ".local/pcsx2-logs/ico-runtime-20260825-152452.jsonl")
    N = 500000

    all_ev = []
    with open(logfile) as fh:
        for line in fh:
            line = line.strip()
            if not line:
                continue
            try:
                all_ev.append(json.loads(line))
            except json.JSONDecodeError:
                pass
    ev = all_ev[-N:] if len(all_ev) > N else all_ev
    print(f"Log: {logfile}")
    print(f"Total: {len(all_ev):,} events | Analyzed: {len(ev):,} (last {N})")
    print()

    # -- Collect data --
    ew_cnt = Counter()
    ew_by_ws = defaultdict(Counter)
    ew_ws = defaultdict(set)
    ew_fc = {}
    ew_lc = {}
    ew_regs = defaultdict(list)
    ew_dl = defaultdict(Counter)
    ew_a2 = defaultdict(set)
    ew_gobj = defaultdict(set)
    gobj_a0 = Counter()
    gobj_a2 = Counter()
    ws_trans = []
    lws = None

    for e in ev:
        lbl = e.get("label", "")
        rg = e.get("regs", {})
        inf = e.get("info", {})
        cy = e.get("cycle", 0)
        ws = hx(inf.get("world_state_raw", "0x0"))
        ew = hx(inf.get("current_entity_work", "0x0"))

        if ws != lws and lws is not None:
            ws_trans.append((cy, lws, ws))
        lws = ws

        if lbl == "ios_om_main":
            if ew:
                ew_cnt[ew] += 1
                ew_by_ws[ws][ew] += 1
                ew_ws[ew].add(ws)
                ew_fc.setdefault(ew, cy)
                ew_lc[ew] = cy
                ew_dl[ew][rg.get("a1", "0x0")] += 1
                ew_a2[ew].add(rg.get("a2", "0x0"))
                ew_gobj[ew].add(inf.get("current_gobj", "0x0"))
                if len(ew_regs[ew]) < 8:
                    ew_regs[ew].append({"cy": cy, "ws": ws, "r": dict(rg)})
        elif lbl == "isys_gobj_proc_add":
            a0v = hx(rg.get("a0", "0x0"))
            if a0v > 0x200000:
                gobj_a0[a0v] += 1
            a2v = hx(rg.get("a2", "0x0"))
            if a2v > 0x100000:
                gobj_a2[a2v] += 1

    tot = len([e for e in ev if e.get("label") == "ios_om_main"])
    with_e = sum(1 for e in ev
                 if e.get("label") == "ios_om_main"
                 and hx(e["info"].get("current_entity_work", "0x0")))

    W = 78
    print("=" * W)
    print("  ENTITY WORK AREA STRUCTURE - PCSX2 Runtime Analysis")
    print("=" * W)
    gp = 0x6388f0
    print(f"  ios_om_main events:           {tot:>12,}")
    print(f"    with non-zero entity_work:  {with_e:>12,} ({100*with_e/max(tot,1):.1f}%)")
    print(f"  Unique entity work ptrs:      {len(ew_cnt):>12,}")
    print(f"  GP register (stable):         0x{gp:08x}")
    print(f"  GP-0x6E08 source address:     0x{gp - 0x6E08:08x}")
    print(f"  GP-0x6E0C source address:     0x{gp - 0x6E0C:08x}")
    print()

    # Top 10
    print("-" * W)
    print("  TOP 10 MOST ACTIVE ENTITY WORK AREAS")
    print("-" * W)
    print(f"  {'#':<3} {'Ptr':<14} {'Count':>10} {'%':>6} {'#DL':>3} {'#WS':>3}")
    print("-" * W)
    t10 = ew_cnt.most_common(10)
    for r, (e, c) in enumerate(t10, 1):
        print(f"  {r:<3} 0x{e:08x}   {c:>10,} {100*c/max(with_e,1):>5.1f}%  {len(ew_dl[e]):>3}  {len(ew_ws[e]):>3}")
    print()

    # DL slot binding
    print("-" * W)
    print("  ENTITY -> DL SLOT BINDING")
    print("-" * W)
    print(f"  {'Entity':<14} {'Primary DL':<14} {'a2':<10} {'Count':>10}")
    print("-" * W)
    for e, c in ew_cnt.most_common():
        sa, sc = ew_dl[e].most_common(1)[0]
        a2s = "/".join(sorted(ew_a2[e])[:3])
        print(f"  0x{e:08x}  {sa:<14} {a2s:<10} {sc:>10,}")
    print()

    # Entity per world state
    print("-" * W)
    print("  ENTITY MAP BY WORLD STATE")
    print("-" * W)
    for w in sorted(ew_by_ws):
        d = ew_by_ws[w]
        t = sum(d.values())
        items = ", ".join(f"0x{x:08x}({v:,})" for x, v in d.most_common())
        print(f"  ws=0x{w:02x}  [{t:>8,} ev, {len(d)} ent]  {items}")
    print()

    # Entity world state affinity
    print("-" * W)
    print("  PER-ENTITY WORLD STATE AFFINITY")
    print("-" * W)
    for e, c in ew_cnt.most_common():
        wd = Counter()
        for w, ec in ew_by_ws.items():
            if e in ec:
                wd[w] = ec[e]
        wl = ", ".join(f"0x{w:02x}({v:,})" for w, v in wd.most_common())
        print(f"  0x{e:08x}  ({c:>10,})  {wl}")
    print()

    # Detailed registers for top 3
    print("-" * W)
    print("  DETAILED REGISTER CONTEXT - TOP 3 ENTITIES")
    print("-" * W)
    for rank, (e, c) in enumerate(t10[:3], 1):
        print(f"\n  Entity #{rank}: 0x{e:08x} ({c:,} events)")
        dur = ew_lc[e] - ew_fc[e]
        print(f"    Region:   {region(e)}")
        print(f"    Duration: {dur:>15,} cycles ({dur/294912000:.2f}s @294.9MHz)")
        snaps = ew_regs[e]
        if snaps:
            s0 = snaps[0]
            print(f"    First hit: cycle={s0['cy']:,} ws=0x{s0['ws']:02x}")
            print(f"    Registers:")
            for rg in ["a0","a1","a2","a3","v0","v1","t0","t1","t2","s0","s1","s2","sp","gp"]:
                print(f"      {rg:<4} = {s0['r'].get(rg, '?')}")
            vary = []
            for rg in ["a0","a1","a2","a3","t0","t1","t2"]:
                vals = set(s["r"].get(rg) for s in snaps)
                if len(vals) > 1:
                    vary.append(rg)
            if vary:
                print(f"    Varying: {', '.join(vary)}")
                for rg in vary:
                    vc = Counter(s["r"].get(rg, "?") for s in snaps)
                    vs = ", ".join(f"{v}({n})" for v, n in vc.most_common(3))
                    print(f"      {rg:<4}: {vs}")
    print()

    # GObj proc_add patterns
    print("-" * W)
    print("  isys_gobj_proc_add PATTERNS")
    print("-" * W)
    print(f"  a0 (GObj ptr) top 15:")
    for v, c in gobj_a0.most_common(15):
        print(f"    0x{v:08x}  {c:>7,}  {region(v)}")
    print(f"  a2 (entry/descriptor) top 15:")
    for v, c in gobj_a2.most_common(15):
        print(f"    0x{v:08x}  {c:>7,}  {region(v)}")
    print()

    # WS transitions
    print("-" * W)
    print(f"  WORLD STATE TRANSITIONS ({len(ws_trans)} total)")
    print("-" * W)
    for cy, ow, nw in ws_trans[:30]:
        print(f"    cycle={cy:>15,}  0x{ow:02x} -> 0x{nw:02x}")
    print()
    print("=" * W)
    print("  DONE")
    print("=" * W)


if __name__ == "__main__":
    main()
