#!/usr/bin/env python3
"""Static cross-reference: runtime room-role handlers -> ELF descriptor rows.

Maps every handler address in native/src/game/GeneratedRoomRoleTables.h to the
descriptor (0x2A31B8, stride 0x64) whose +0x48/+0x50/+0x58/+0x60 field
contains exactly that virtual address. Also compares per-scene static entry
payload descriptor counts against the runtime roleCounts.

The descriptor footprint:
    +0x40  init/cb40    +0x44  gate
    +0x48  hA           +0x50  hB (runtime primary handler)
    +0x58  hC           +0x5C  hD (runtime secondary handler)
    +0x60  vtable
Fields are resolved by matching the byte value, not assumed from the index.

No runtime capture is required. Sources:
  - USA ELF            .local/extracted/SCUS_971.13.elf
  - runtime rosters    native/src/game/GeneratedRoomRoleTables.h (Rev.159)
"""

import os
import re
import struct
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(SCRIPT_DIR, ".."))
ELF_PATH = os.path.join(REPO, ".local", "extracted", "SCUS_971.13.elf")
ROLES_PATH = os.path.join(REPO, "native", "src", "game", "GeneratedRoomRoleTables.h")

DESCRIPTOR_BASE = 0x2A31B8
DESCRIPTOR_STRIDE = 0x64
DESCRIPTOR_COUNT = 68
ENTRY_BASE = 0x2A4C48
ENTRY_STRIDE = 0x4C
DISPATCH_BASE = 0x5F2FB8
DISPATCH_STRIDE = 0x194


def load_elf(path):
    sys.path.insert(0, os.path.join(SCRIPT_DIR, "..", "tools"))
    from extract_scene_tables import ElfReader

    return ElfReader(path)


def vo(data, va):
    return data.offset(va)


def u32(elf, off):
    return struct.unpack_from("<I", elf.data, off)[0]


def descriptor(elf, idx):
    base = vo(elf, DESCRIPTOR_BASE + idx * DESCRIPTOR_STRIDE)
    name = elf.data[base : base + 16].split(b"\0")[0].decode("ascii", "replace")
    return {
        "name": name,
        "init40": u32(elf, base + 0x40),
        "gate44": u32(elf, base + 0x44),
        "hA48": u32(elf, base + 0x48),
        "hB50": u32(elf, base + 0x50),
        "hC58": u32(elf, base + 0x58),
        "hD5C": u32(elf, base + 0x5C),
        "vt60": u32(elf, base + 0x60),
    }


def scene_payload(elf, sc):
    """Return {(descriptor_name, descriptor_index): count} for a scene range."""
    base = vo(elf, DISPATCH_BASE + sc * DISPATCH_STRIDE)
    s = u32(elf, base + 0x128)
    e = u32(elf, base + 0x12C)
    cnt = {}
    for idx in range(s, e):
        row = vo(elf, ENTRY_BASE + idx * ENTRY_STRIDE)
        di = elf.data[row + 0x46]
        if di < DESCRIPTOR_COUNT:
            d = descriptor(elf, di)
            key = (d["name"], di)
            cnt[key] = cnt.get(key, 0) + 1
    return s, e, cnt


def resolve_handler(elf, descriptors, addr):
    """Find descriptor + field whose value is exactly addr."""
    for idx, d in enumerate(descriptors):
        for label, val in (
            ("hA+0x48", d["hA48"]),
            ("hB+0x50", d["hB50"]),
            ("hC+0x58", d["hC58"]),
            ("hD+0x5C", d["hD5C"]),
            ("vt+0x60", d["vt60"]),
            ("init+0x40", d["init40"]),
        ):
            if val == addr:
                return d["name"], idx, label
    return None, None, None


def parse_rosters(path):
    text = open(path).read()
    rosters = {}
    plans = {}
    # match each array: constexpr VerifiedRoomRole kVerifiedRoomRolesSceneXX[] = { ... };
    for m in re.finditer(
        r"kVerifiedRoomRolesScene([0-9A-Fa-f]{2})\[\]\s*=\s*\{(.*?)\};", text, re.S
    ):
        sc = int(m.group(1), 16)
        roles = []
        for r in re.finditer(r"\{0x([0-9A-Fa-f]{8})u,\s*\"([^\"]+)\",\s*(\d+)u?\}", m.group(2)):
            roles.append((int(r.group(1), 16), r.group(2), int(r.group(3))))
        rosters[sc] = roles
    for m in re.finditer(
        r"\{0x(?:[0-9A-Fa-f]{2})u,\s*kVerifiedRoomRolesScene([0-9A-Fa-f]{2}),\s*(\d+)u?\}",
        text,
    ):
        plans[int(m.group(1), 16)] = int(m.group(2))
    return rosters, plans


def main():
    elf = load_elf(ELF_PATH)
    descriptors = [descriptor(elf, i) for i in range(DESCRIPTOR_COUNT)]
    rosters, plans = parse_rosters(ROLES_PATH)
    resolve_errors = 0
    stats = {"total": 0, "exact": 0, "payload_absent": 0, "count_mismatch": 0, "hD": 0}

    print(f"Descriptor table: {DESCRIPTOR_COUNT} rows at 0x{DESCRIPTOR_BASE:X}")
    print(f"Runtime plans:    {len(plans)} scenes\n")
    print(f"{'sc':>3} {'field':>8}  {'desc':>16} {'desc#':>3}  {'roleCnt':>7} {'static':>6}  match")
    print("-" * 78)
    for sc in sorted(rosters):
        s, e, payload = scene_payload(elf, sc)
        print(f"\n=== scene 0x{sc:02X}  ([{s},{e}) entry idx) static payload "
              f"{sum(payload.values())} rows ===")
        for addr, name, cnt in rosters[sc]:
            dn, di, field = resolve_handler(elf, descriptors, addr)
            static = payload.get((dn, di), 0) if dn else None
            match = ""
            stats["total"] += 1
            if not field:
                resolve_errors += 1
                match = "UNRESOLVED"
            elif "hD" in field:
                stats["hD"] += 1
                match = "n/a"
            elif static == cnt:
                stats["exact"] += 1
                match = "p"
            elif not static:
                stats["payload_absent"] += 1
                match = "-"
            else:
                stats["count_mismatch"] += 1
                match = "x"
            print(f"0x{sc:02X} {field or '?':>8}  {dn or '?':>16} {str(di) if di is not None else '-':>3}  "
                  f"{cnt:>7} {str(static) if static is not None else '   -':>6}  {match}  {name}")
    print(f"\n=== summary (handler roles) ===")
    print(f"  total roles:        {stats['total']}")
    print(f"  exact match:        {stats['exact']}")
    print(f"  payload_absent:     {stats['payload_absent']}  (descriptor NOT in static payload)")
    print(f"  count mismatch:     {stats['count_mismatch']}  (descriptor present, count differs)")
    print(f"  hD secondary:       {stats['hD']}")
    print(f"  UNRESOLVED:         {resolve_errors}")


if __name__ == "__main__":
    main()