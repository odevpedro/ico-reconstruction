#!/usr/bin/env python3
"""p2/p1 PS2O face-region analysis using the Rev.151-aligned record layout.

Re-validates Rev.144's boundary-edge + Rev.151's UV-edge-spread metrics on the
CORRECTED column alignment (spine = u16[2] a, UV = u16[4] m, material = u16[7]
f) to decide whether the p1 cascade-a rule is also valid for the p2 wall family
or whether a per-file discriminator is still needed.

Usage:
  python3 tools/ps2o_family_analysis.py [path.p2o ...]
"""

import collections
import math
import struct
import sys

STRIDE = 16


def rd16(data, off):
    return struct.unpack_from("<H", data, off)[0]


def rd32(data, off):
    return struct.unpack_from("<f", data, off)[0]


def parse_verts_uvs(data):
    off = 0x20
    positions = []
    while off + STRIDE <= len(data):
        w = rd32(data, off + 12)
        if w != 1.0:
            break
        positions.append((rd32(data, off), rd32(data, off + 4), rd32(data, off + 8)))
        off += STRIDE
    nv = len(positions)
    uv_entries = []
    while off + STRIDE <= len(data):
        u0, v0, t0, t1 = rd32(data, off), rd32(data, off + 4), rd32(data, off + 8), rd32(data, off + 12)
        if (t0 != 0.0 and t0 != -0.0) or (t1 != 0.0 and t1 != -0.0):
            break
        uv_entries.append((u0, v0))
        off += STRIDE
    return positions, uv_entries


def is_strip_header(data, at, size):
    if at + STRIDE > size:
        return False
    n = rd16(data, at)
    if n < 2 or n > 64:
        return False
    for k in range(1, 8):
        if rd16(data, at + 2 * k) != 0xFFFF:
            return False
    return True


def analyze(path):
    data = open(path, "rb").read()
    print(f"=== {path} ({len(data)} B) ===")
    if data[:4] != b"PS2O":
        print("  not a PS2O file")
        return

    positions, uv_entries = parse_verts_uvs(data)
    nv = len(positions)
    nuv = len(uv_entries)
    print(f"  positions: {nv}  (4-float w==1.0 scan)")
    print(f"  uvs:       {nuv}")

    uv_end = 0x20 + nv * STRIDE + nuv * 4
    i = (uv_end + STRIDE - 1) & ~(STRIDE - 1)
    while i + STRIDE <= len(data) and not is_strip_header(data, i, len(data)):
        i += STRIDE
    print(f"  face region starts at 0x{i:X} (uv_end 0x{uv_end:X})")

    n_headers = 0
    n_records = 0
    n_total = 0
    n_flag0 = 0
    n_flag1 = 0
    n_mixed = 0
    strip_mats = collections.Counter()
    _m_per_strip_const = 0
    _m_per_strip_var = 0
    max_m = 0
    max_a = 0
    coverage_all = set()
    flag_mat = collections.Counter()
    header_n = collections.Counter()
    n_records_per_strip = []
    flag0_mat = collections.Counter()
    flag1_mat = collections.Counter()
    strip_streams = collections.Counter()
    prev_flag = None
    strips = []  # list of dict(flag, mat, spine, ms, u6)

    ok = True
    while i + STRIDE <= len(data) and is_strip_header(data, i, len(data)):
        n = rd16(data, i)
        rec_start = i + STRIDE
        if rec_start + n * STRIDE > len(data):
            ok = False
            break
        header_n[n] += 1
        n_headers += 1
        n_total += n

        flag = rd16(data, rec_start)
        mats = []
        ms = []
        u6s = []
        spines = []
        bad = False
        for r in range(n):
            o = rec_start + r * STRIDE
            a = rd16(data, o + 4)
            s = rd16(data, o + 6)
            m = rd16(data, o + 8)
            b = rd16(data, o + 10)
            u6 = rd16(data, o + 12)
            f = rd16(data, o + 14)
            if a >= nv:
                bad = True
                break
            spines.append(a)
            ms.append(m)
            mats.append(f)
            u6s.append(u6)
            max_a = max(max_a, a)
            max_m = max(max_m, m)
            coverage_all.add(m)
            if s == 0 and r == 0:
                pass
        if bad:
            ok = False
            break
        if all(m == ms[0] for m in ms):
            _m_per_strip_const += 1
        else:
            _m_per_strip_var += 1

        sf = mats[0]
        strip_mats[sf] += 1
        flag_mat[(flag, sf)] += 1
        if flag == 0:
            n_flag0 += 1
            flag0_mat[sf] += 1
        elif flag == 1:
            n_flag1 += 1
            flag1_mat[sf] += 1
        else:
            n_mixed += 1
        n_records += n
        n_records_per_strip.append(n)
        strip_streams[rd16(data, rec_start + 6)] += 1
        strips.append({"flag": flag, "mat": sf, "spine": spines, "ms": ms, "u6": u6s})
        i = rec_start + n * STRIDE

    print(f"  headers: {n_headers}, records: {n_records}, N-histogram: "
          f"{collections.Counter({k: v for k, v in header_n.items()})}")
    print(f"  flag 0: {n_flag0}, flag 1: {n_flag1}, other: {n_mixed}")
    print(f"  materials (f): {dict(strip_mats)}")
    print(f"  flag0 -> mat: {dict(flag0_mat)}")
    print(f"  flag1 -> mat: {dict(flag1_mat)}")
    print(f"  max_a={max_a} (nv={nv}), max_m={max_m} (nuv={nuv})")
    uv_cov = len(coverage_all)
    print(f"  m distinct: {uv_cov}/{nuv} (full-coverage ratio {uv_cov}/{nuv})")
    print(f"  m constant per strip: {_m_per_strip_const}/{n_headers} ({100.0*_m_per_strip_const/n_headers:.1f}%)")
    print(f"  record-total check: n_headers * mean? total expected via sum(N): {sum(k*v for k,v in header_n.items())} vs observed {n_records}")

    # Triangulate (cascade) and compute metrics.
    tris = []
    for st in strips:
        sp = st["spine"]
        for r in range(len(sp) - 2):
            s0, s1, s2 = sp[r], sp[r + 1], sp[r + 2]
            if s0 == s1 or s1 == s2:
                continue
            tris.append((s0, s1, s2, st["mat"]))
    print(f"  cascade triangles: {len(tris)}")

    # Boundary-edge ratio (Rev.144): fraction of unique edges shared by != 2 tris.
    edge_count = collections.Counter()
    for t in tris:
        for e in ((t[0], t[1]), (t[1], t[2]), (t[2], t[0])):
            edge_count[tuple(sorted(e))] += 1
    boundary = sum(1 for c in edge_count.values() if c != 2)
    bnd_ratio = boundary / len(edge_count) if edge_count else 0.0
    print(f"  edges: {len(edge_count)}, boundary edges: {boundary}, bnd_ratio: {bnd_ratio:.3f}")

    # UV edge-spread (Rev.151): for each shared edge (both tris use it), the
    # max |delta| of the UV at the shared vertices.
    tri_uv = []
    for st in strips:
        sp = st["spine"]
        for r in range(len(sp) - 2):
            s0, s1, s2 = sp[r], sp[r + 1], sp[r + 2]
            if s0 == s1 or s1 == s2:
                continue
            tri_uv.append(((st["ms"][r], st["ms"][r + 1], st["ms"][r + 2]),
                           (st["mat"], s0, s1, s2)))
    # map (edge) -> list of (uv-index-at-vertex) per tri
    edge_uv = collections.defaultdict(list)
    for m_idx, (mv, t) in enumerate(tri_uv):
        for k in range(3):
            u = t[1 + k]
            e = tuple(sorted((t[1 + k], t[1 + (k + 1) % 3])))
            edge_uv[e].append(mv[k])
            edge_uv[e].append(mv[(k + 1) % 3])
    spreads = []
    for e, uvs in edge_uv.items():
        # uvs pairs: (u0,u1) for each face that contains edge e.
        pairs = [(uvs[i], uvs[i + 1]) for i in range(0, len(uvs), 2)]
        if len(pairs) < 2:
            continue
        for i in range(len(pairs)):
            for j in range(i + 1, len(pairs)):
                d = abs(pairs[i][0] - pairs[j][0]) + abs(pairs[i][1] - pairs[j][1])
                spreads.append(d)
    mean_spread = sum(spreads) / len(spreads) if spreads else float("nan")
    print(f"  UV edge-spread (pairs): {len(spreads)}, mean {mean_spread:.4f}")
    print(f"  flag correlation to material: {dict(flag_mat)}")


def main():
    paths = sys.argv[1:]
    if not paths:
        paths = ["native/assets/170_st00a_p1.p2o",
                 "native/assets/scene/pieces/171_st00a_p2.p2o"]
    for p in paths:
        analyze(p)
        print()


if __name__ == "__main__":
    main()