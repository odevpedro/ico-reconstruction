#!/usr/bin/env python3
"""PAL→USA reconciliation using MAIN.MAP boundaries.

Efficient two-phase approach:
  Phase 1: Pre-compute USA op_seq hashes for all needed window sizes × positions
  Phase 2: Look up each PAL first function → find offset → validate all functions
"""
import csv, hashlib, struct, sys, time
from pathlib import Path
from collections import defaultdict

PROJECT_ROOT = Path(__file__).resolve().parent.parent.parent
OUT_DIR = PROJECT_ROOT / "research" / "pal-usa"
PAL_ELF = PROJECT_ROOT / ".local" / "extracted" / "pal" / "SCES_507.60"
USA_ELF = PROJECT_ROOT / ".local" / "extracted" / "SCUS_971.13.elf"


def read_elf_text(path):
    from elftools.elf.elffile import ELFFile
    with open(path, 'rb') as f:
        elf = ELFFile(f)
        for sec in elf.iter_sections():
            if sec.name == '.text':
                return sec.header.sh_addr, sec.data()
    return None, None


def _op_seq_hash(data):
    def tkn(w):
        op = (w>>26)&0x3F; f = w&0x3F
        if op==0: return f
        if op==1: return 0x40|((w>>16)&0x1F)
        if op in(2,3): return 0x60|op
        if op==0x11: return 0x70|f
        if op==0x1A: return 0xB0|f
        if op==0x1C: return 0xF0|f
        if 4<=op<=7: return 0x100|(op<<6)|((w>>16)&0x1F)
        if 32<=op<=47: return 0x140|op
        if 8<=op<=15: return 0x160|op
        return 0x180|op
    b = bytearray()
    for i in range(0, len(data), 4):
        if i+4>len(data): break
        w = struct.unpack('<I', data[i:i+4])[0]
        b.extend(struct.pack('<H', tkn(w)))
    return hashlib.sha1(bytes(b)).hexdigest()


def precompute_usa_index(usa_data, sizes_needed):
    """Build {op_seq_hash: [(usa_off, size)]} for unique sizes.
    
    Scans USA .text once per unique size, stride=4 for small sizes.
    """
    t0 = time.time()
    index = defaultdict(list)  # op_seq_hash -> [(usa_off, size)]
    total_positions = 0

    for sz in sorted(sizes_needed):
        stride = 4 if sz <= 64 else 8 if sz <= 256 else 16
        positions = 0
        for off in range(0, len(usa_data) - sz + 1, stride):
            chunk = usa_data[off:off + sz]
            h = _op_seq_hash(chunk)
            index[h].append((off, sz))
            positions += 1
        total_positions += positions

    print(f"[INDEX] {len(sizes_needed)} sizes, {total_positions} positions, "
          f"{len(index)} unique hashes ({time.time()-t0:.1f}s)")
    return index


def main():
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    T = time.time()

    pal_text_va, pal_data = read_elf_text(PAL_ELF)
    usa_text_va, usa_data = read_elf_text(USA_ELF)
    if not pal_data or not usa_data:
        print("[ERR] Cannot read .text"); sys.exit(1)
    print(f"[ELF] PAL: {len(pal_data)}b  USA: {len(usa_data)}b  ({time.time()-T:.1f}s)")

    with open(OUT_DIR / "main_map_functions.csv") as f:
        rows = list(csv.DictReader(f))
    print(f"[LOAD] {len(rows)} functions ({time.time()-T:.1f}s)")

    by_obj = defaultdict(list)
    for r in rows:
        by_obj[r['object_file']].append(r)
    for v in by_obj.values():
        v.sort(key=lambda x: int(x['pal_va'], 16))

    # --- Extract PAL fingerprints ---
    pal_fps = {}
    for obj_name, funcs in by_obj.items():
        for i, f in enumerate(funcs):
            va = int(f['pal_va'], 16)
            off = va - pal_text_va
            if off < 0 or off >= len(pal_data):
                continue
            if i + 1 < len(funcs):
                sz = int(funcs[i+1]['pal_va'], 16) - va
            else:
                sz = len(pal_data) - off
            sz = min(sz, len(pal_data) - off)
            if sz < 4:
                continue
            pal_fps[va] = {
                'size': sz,
                'data': pal_data[off:off+sz],
                'op_seq': _op_seq_hash(pal_data[off:off+sz]),
                'name': f['name'],
                'object_file': obj_name,
            }
    print(f"[FP] {len(pal_fps)} PAL fps ({time.time()-T:.1f}s)")

    # --- Collect unique sizes from first functions ---
    first_func_sizes = set()
    for obj_name, funcs in by_obj.items():
        pv = int(funcs[0]['pal_va'], 16)
        fp = pal_fps.get(pv)
        if fp and fp['size'] >= 8:
            first_func_sizes.add(fp['size'])
    print(f"[SIZES] {len(first_func_sizes)} unique first-func sizes")

    # --- Pre-compute USA index ---
    print("[INDEX] Pre-computing USA op_seq index...")
    usa_index = precompute_usa_index(usa_data, first_func_sizes)

    # --- Phase 1: Find anchor for each .o via first function lookup ---
    anchors = {}
    for obj_name, funcs in sorted(by_obj.items(),
                                   key=lambda x: int(x[1][0]['pal_va'], 16)):
        f0 = funcs[0]
        pv = int(f0['pal_va'], 16)
        fp = pal_fps.get(pv)
        if not fp or fp['size'] < 8:
            continue

        # Look up in pre-computed index
        candidates = usa_index.get(fp['op_seq'], [])
        if not candidates:
            continue

        # Pick the candidate closest to expected position
        expected_off = pv - pal_text_va
        best = min(candidates, key=lambda x: abs(x[0] - expected_off))
        usa_off, _ = best
        score = 1.0 - abs(usa_off - expected_off) / max(len(usa_data), 1)

        anchors[obj_name] = {
            'offset': (usa_off + usa_text_va) - pv,
            'score': score,
            'anchor': f0['name'],
        }

    print(f"[P1] {len(anchors)}/{len(by_obj)} anchors ({time.time()-T:.1f}s)")

    # --- Phase 2: Neighbor fallback ---
    anchored_list = sorted(
        [(n, int(by_obj[n][0]['pal_va'], 16), anchors[n])
         for n in anchors],
        key=lambda x: x[1]
    )

    for obj_name in sorted(by_obj):
        if obj_name in anchors:
            continue
        pv0 = int(by_obj[obj_name][0]['pal_va'], 16)
        nearest = min(anchored_list, key=lambda x: abs(x[1] - pv0)) if anchored_list else None
        if not nearest:
            continue
        neigh_offset = nearest[2]['offset']
        fp = pal_fps.get(pv0)
        if not fp or fp['size'] < 8:
            continue

        expected_off = pv0 - pal_text_va + neigh_offset
        expected_off = max(0, min(expected_off, len(usa_data) - fp['size']))

        # Build a small local index for this function's size
        lo = max(0, expected_off - 16384)
        hi = min(len(usa_data) - fp['size'], expected_off + 16384)
        stride = 4 if fp['size'] <= 64 else 8 if fp['size'] <= 256 else 16
        found = None
        for off in range(lo, hi, stride):
            chunk = usa_data[off:off + fp['size']]
            if len(chunk) < fp['size']:
                continue
            if _op_seq_hash(chunk) == fp['op_seq']:
                found = off
                break

        if found is not None:
            anchors[obj_name] = {
                'offset': (found + usa_text_va) - pv0,
                'score': 0.8,
                'anchor': f"{fp['name']}(via neighbor)",
            }

    print(f"[P2] {len(anchors)} total anchors ({time.time()-T:.1f}s)")

    # --- Apply offsets, validate ---
    obj_results = []
    all_funcs_out = []

    for obj_name, funcs in sorted(by_obj.items(),
                                   key=lambda x: int(x[1][0]['pal_va'], 16)):
        n = len(funcs)
        anc = anchors.get(obj_name)
        if not anc:
            obj_results.append((
                obj_name, int(funcs[0]['pal_va'], 16),
                int(funcs[-1]['pal_va'], 16),
                n, 0, 'NONE', 0, 'No anchor'
            ))
            for f in funcs:
                all_funcs_out.append((f['pal_va'], '', f['name'], obj_name,
                                      'NO_ANCHOR', ''))
            continue

        offset = anc['offset']
        val = 0
        for f in funcs:
            pv = int(f['pal_va'], 16)
            uv = pv + offset
            fp = pal_fps.get(pv)
            if not fp:
                all_funcs_out.append((f'0x{pv:08X}', f'0x{uv:08X}',
                                      f['name'], obj_name, 'NO_FP', ''))
                continue
            uoff = uv - usa_text_va
            if uoff < 0 or uoff + fp['size'] > len(usa_data):
                all_funcs_out.append((f'0x{pv:08X}', f'0x{uv:08X}',
                                      f['name'], obj_name, 'OOB', ''))
                continue
            uchunk = usa_data[uoff:uoff+fp['size']]
            useq = _op_seq_hash(uchunk)
            if useq == fp['op_seq']:
                val += 1
                all_funcs_out.append((f'0x{pv:08X}', f'0x{uv:08X}',
                                      f['name'], obj_name, 'MATCH',
                                      f'offset=0x{offset:08X}'))
            else:
                all_funcs_out.append((f'0x{pv:08X}', f'0x{uv:08X}',
                                      f['name'], obj_name, 'SEQ_MISMATCH',
                                      f'sz={fp["size"]}'))

        ratio = val / n if n else 0
        conf = 'HIGH' if ratio >= 1.0 else 'MEDIUM' if ratio >= 0.5 else 'LOW' if val > 0 else 'INVALID'
        obj_results.append((obj_name, int(funcs[0]['pal_va'], 16),
                            int(funcs[-1]['pal_va'], 16), n, val, conf,
                            offset, f'{val}/{n} validated, offset=0x{offset:08X}'))

    # Write outputs
    with open(OUT_DIR / "pal_usa_function_map_candidates.csv", 'w', newline='') as f:
        w = csv.writer(f)
        w.writerow(['pal_va','usa_va','name','object_file','status','notes'])
        for r in all_funcs_out:
            w.writerow(r)
    print(f"[OUT] function map ({time.time()-T:.1f}s)")

    with open(OUT_DIR / "pal_usa_object_range_candidates.csv", 'w', newline='') as f:
        w = csv.writer(f)
        w.writerow(['object_file','pal_start','pal_end','function_count',
                     'functions_mapped','confidence','offset_used','notes'])
        for r in sorted(obj_results, key=lambda x: x[1]):
            w.writerow([r[0], f'0x{r[1]:08X}', f'0x{r[2]:08X}',
                        r[3], r[4], r[5], f'0x{r[6]:08X}', r[7]])
    print(f"[OUT] object ranges ({time.time()-T:.1f}s)")

    # Summary
    m = {'HIGH': (0,0), 'MEDIUM': (0,0), 'LOW': (0,0), 'INVALID': (0,0), 'NONE': (0,0)}
    for r in obj_results:
        o, fv = m.setdefault(r[5], [0,0])
        m[r[5]][0] += 1; m[r[5]][1] += r[4]
    total = m['HIGH'][1] + m['MEDIUM'][1] + m['LOW'][1]

    print(f"\n{'='*60}")
    print("RECONCILIATION SUMMARY")
    print(f"{'='*60}")
    for k in ('HIGH','MEDIUM','LOW','INVALID','NONE'):
        c, fv = m[k]
        print(f"  {k:8s}: {c:3d} obj, {fv:5d} funcs")
    print(f"  TOTAL:  {total:5d}/{len(pal_fps)} ({total/len(pal_fps)*100:.1f}%)")
    print(f"  TIME:   {time.time()-T:.0f}s")

    key = ['ebrain.o','kanban.o','kanbanBoot.o','gobj.o','gobj_process.o',
           'girl_act.o','backStage.o','gobj_cam_dl.o','queen.o','rope.o',
           'particleEffect.o','gobj.o','gobj_process.o']
    print(f"\nKey objects:")
    for r in sorted(obj_results, key=lambda x: x[1]):
        if r[0] in key:
            print(f"  {r[0]:30s}: {r[5]:8s} {r[4]:3d}/{r[3]:3d} offset=0x{r[6]:08X}")


if __name__ == '__main__':
    main()
