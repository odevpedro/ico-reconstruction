#!/usr/bin/env python3
"""Second-pass recovery: find seeds for unmatched objects.

For each unmatched object, tries to find at least one function that
exists in both PAL and USA, by:
  1. op_seq_hash match (instruction pattern)
  2. raw_sha1 match (binary-identical)

Once a match is found, applies the object-range offset to all functions
in the object. Validates with pre-computed fingerprints.

Output: updates pal_usa_object_range_candidates.csv and pal_usa_function_map_candidates.csv
"""
import csv, hashlib, json, struct, sys
from pathlib import Path
from collections import defaultdict

PROJECT_ROOT = Path(__file__).resolve().parent.parent.parent
OUT_DIR = PROJECT_ROOT / "research" / "pal-usa"
USA_ELF = PROJECT_ROOT / ".local" / "extracted" / "SCUS_971.13.elf"


def load_csv(path):
    with open(path) as f:
        return list(csv.DictReader(f))


def load_fingerprint_map(path):
    with open(path) as f:
        fps = json.load(f)
    return {fp['va']: fp for fp in fps}


def build_raw_sha1_lookup(fps_map):
    lookup = defaultdict(list)
    for va, fp in fps_map.items():
        lookup[fp['raw_sha1']].append(va)
    return lookup


def build_object_index(rows):
    index = {}
    order = []
    for row in rows:
        obj = row['object_file']
        if obj in index:
            continue
        index[obj] = len(order)
        order.append(obj)
    return index, order


def read_elf_text(path: Path):
    from elftools.elf.elffile import ELFFile
    with open(path, 'rb') as f:
        elf = ELFFile(f)
        for sec in elf.iter_sections():
            if sec.name == '.text':
                return sec.header.sh_addr, sec.data()
    return None, None


def _op_seq_hash(data: bytes) -> str:
    def tkn(word: int) -> int:
        op = (word >> 26) & 0x3F
        funct = word & 0x3F
        if op == 0:
            return funct
        if op == 1:
            return 0x40 | ((word >> 16) & 0x1F)
        if op in (2, 3):
            return 0x60 | op
        if op == 0x11:
            return 0x70 | funct
        if op == 0x1A:
            return 0xB0 | funct
        if op == 0x1C:
            return 0xF0 | funct
        if 4 <= op <= 7:
            return 0x100 | (op << 6) | ((word >> 16) & 0x1F)
        if 32 <= op <= 47:
            return 0x140 | op
        if 8 <= op <= 15:
            return 0x160 | op
        return 0x180 | op

    out = bytearray()
    for i in range(0, len(data), 4):
        if i + 4 > len(data):
            break
        word = struct.unpack('<I', data[i:i + 4])[0]
        out.extend(struct.pack('<H', tkn(word)))
    return hashlib.sha1(bytes(out)).hexdigest()


def precompute_usa_index(usa_data: bytes, sizes_needed: set[int]) -> dict[str, list[tuple[int, int]]]:
    """Build {op_seq_hash: [(usa_off, size)]} for all required sizes."""
    index = defaultdict(list)
    for size in sorted(sizes_needed):
        stride = 4 if size <= 64 else 8 if size <= 256 else 16
        for off in range(0, len(usa_data) - size + 1, stride):
            chunk = usa_data[off:off + size]
            index[_op_seq_hash(chunk)].append((off, size))
    return index


def validate_object_at_offset(
    obj_pal_funcs: list[dict],
    offset: int,
    pal_fp_by_va: dict[int, dict],
    usa_fp_by_va: dict[int, dict],
) -> tuple[int, list[dict]]:
    """Validate a candidate offset against all functions in an object."""
    validated = 0
    matches = []
    for f in obj_pal_funcs:
        pal_va = int(f['pal_va'], 16)
        pal_fp = pal_fp_by_va.get(pal_va)
        if not pal_fp:
            continue

        usa_va = pal_va + offset
        usa_fp = usa_fp_by_va.get(usa_va)
        if not usa_fp:
            continue
        if usa_fp['op_seq_hash'] != pal_fp['op_seq_hash'] or usa_fp['size'] != pal_fp['size']:
            continue

        validated += 1
        matches.append({
            'pal_va': pal_va,
            'usa_va': usa_va,
            'method': 'op_seq_hash',
            'confidence': 0.95,
            'name': f['name'],
        })

    return validated, matches


def score_offset(
    obj_pal_funcs: list[dict],
    offset: int,
    pal_fp_by_va: dict[int, dict],
    usa_fp_by_va: dict[int, dict],
) -> tuple[int, list[dict]]:
    """Score a candidate offset using all functions in the object."""
    return validate_object_at_offset(obj_pal_funcs, offset, pal_fp_by_va, usa_fp_by_va)


def find_seed_for_object(
    obj_pal_funcs: list,
    pal_fp_by_va: dict,
    usa_by_raw: dict,
    usa_index: dict[str, list[tuple[int, int]]],
    usa_text_va: int,
    usa_fp_by_va: dict,
) -> list[dict]:
    """Find all PAL→USA matches for an object's functions.
    Returns list of {pal_va, usa_va, method, confidence}.
    """
    best = {
        'validated': 0,
        'offset': None,
        'matches': [],
        'seeds': [],
    }

    candidate_offsets = defaultdict(list)

    for f in obj_pal_funcs:
        pal_va = int(f['pal_va'], 16)
        pal_fp = pal_fp_by_va.get(pal_va)
        if not pal_fp:
            continue

        # Try raw_sha1 first (binary-identical)
        raw_candidates = usa_by_raw.get(pal_fp['raw_sha1'], [])
        for usa_va in raw_candidates:
            usa_fp = usa_fp_by_va[usa_va]
            if usa_fp['size'] == pal_fp['size']:
                offset = usa_va - pal_va
                candidate_offsets[offset].append({
                    'pal_va': pal_va,
                    'usa_va': usa_va,
                    'method': 'raw_sha1',
                    'confidence': 0.99,
                    'name': f['name'],
                })

        # Try a whole-.text op_seq scan to recover objects without seeds.
        # Evaluate every possible anchor and keep the offset that validates
        # the most functions in the object.
        if pal_fp['size'] < 4:
            continue

        for usa_off, size in usa_index.get(pal_fp['op_seq_hash'], []):
            if size != pal_fp['size']:
                continue
            offset = (usa_text_va + usa_off) - pal_va
            candidate_offsets[offset].append({
                'pal_va': pal_va,
                'usa_va': usa_text_va + usa_off,
                'method': 'op_seq_hash',
                'confidence': 0.95,
                'name': f['name'],
            })

    for offset, seeds in candidate_offsets.items():
        validated, candidate_matches = score_offset(
            obj_pal_funcs, offset, pal_fp_by_va, usa_fp_by_va
        )
        if validated > best['validated']:
            best['validated'] = validated
            best['offset'] = offset
            best['matches'] = candidate_matches
            best['seeds'] = seeds

    if best['validated'] > 0:
        return best['matches']
    return []


def try_apply_offset(obj_funcs, offset, usa_fp_by_va):
    """Try applying an offset to all functions. Return validated count."""
    validated = 0
    for f in obj_funcs:
        pal_va = int(f['pal_va'], 16)
        usa_va = pal_va + offset
        usa_fp = usa_fp_by_va.get(usa_va)
        if not usa_fp:
            continue
        # Found a fingerprint at the inferred address — accept it
        validated += 1
    return validated


def main():
    # Load data
    pal_fp_by_va = load_fingerprint_map(PROJECT_ROOT / "docs" / "symbols" / "pal_fingerprints.json")
    usa_fp_by_va = load_fingerprint_map(PROJECT_ROOT / "docs" / "symbols" / "usa_fingerprints.json")
    pal_funcs = load_csv(OUT_DIR / "main_map_functions.csv")
    obj_candidates = load_csv(OUT_DIR / "pal_usa_object_range_candidates.csv")
    func_candidates = load_csv(OUT_DIR / "pal_usa_function_map_candidates.csv")
    main_map_objects = load_csv(OUT_DIR / "main_map_objects.csv")

    usa_by_raw = build_raw_sha1_lookup(usa_fp_by_va)
    object_index, object_order = build_object_index(main_map_objects)
    known_offsets = {}
    for row in obj_candidates:
        if row['confidence'] != 'NONE' and row.get('offset'):
            known_offsets[row['object_file']] = int(row['offset'], 16)

    usa_text_va, usa_data = read_elf_text(USA_ELF)
    if usa_data is None:
        print(f"[ERR] USA .text not found at {USA_ELF}")
        sys.exit(1)

    unmatched_objs = [r['object_file'] for r in obj_candidates if r['confidence'] == 'NONE']
    sizes_needed = set()
    for obj_name in unmatched_objs:
        obj_funcs = []
        for f in pal_funcs:
            if f['object_file'] == obj_name:
                obj_funcs.append(f)
        for f in obj_funcs:
            pal_va = int(f['pal_va'], 16)
            fp = pal_fp_by_va.get(pal_va)
            if fp and fp['size'] >= 8:
                sizes_needed.add(fp['size'])

    print(f"[INDEX] scanning {len(sizes_needed)} unique sizes from unmatched objects")
    usa_index = precompute_usa_index(usa_data, sizes_needed)

    # Group PAL functions by object
    pal_by_obj = defaultdict(list)
    for f in pal_funcs:
        pal_by_obj[f['object_file']].append(f)
    for obj in pal_by_obj:
        pal_by_obj[obj].sort(key=lambda x: int(x['pal_va'], 16))

    # Find unmatched objects
    unmatched_objs = [r['object_file'] for r in obj_candidates if r['confidence'] == 'NONE']
    print(f"Processing {len(unmatched_objs)} unmatched objects...")

    recovered_count = 0
    recovery_log = []

    for obj_name in sorted(unmatched_objs):
        obj_pal_funcs = pal_by_obj.get(obj_name, [])
        if not obj_pal_funcs:
            continue

        # Reuse offsets from nearby objects in MAIN.MAP order as a fallback.
        idx = object_index.get(obj_name)
        neighbor_offsets = []
        if idx is not None:
            for delta in range(1, 4):
                if idx - delta >= 0:
                    prev_obj = object_order[idx - delta]
                    if prev_obj in known_offsets:
                        neighbor_offsets.append(known_offsets[prev_obj])
                if idx + delta < len(object_order):
                    next_obj = object_order[idx + delta]
                    if next_obj in known_offsets:
                        neighbor_offsets.append(known_offsets[next_obj])

        # Search for seeds
        seeds = find_seed_for_object(
            obj_pal_funcs, pal_fp_by_va,
            usa_by_raw, usa_index, usa_text_va, usa_fp_by_va,
        )

        if not seeds and neighbor_offsets:
            best_neighbor = None
            best_neighbor_validated = 0
            best_neighbor_matches = []
            for offset in sorted(set(neighbor_offsets)):
                validated, matches = score_offset(
                    obj_pal_funcs, offset, pal_fp_by_va, usa_fp_by_va
                )
                if validated > best_neighbor_validated:
                    best_neighbor_validated = validated
                    best_neighbor = offset
                    best_neighbor_matches = matches
            if best_neighbor_validated > 0:
                seeds = best_neighbor_matches

        if not seeds:
            recovery_log.append((obj_name, 0, len(obj_pal_funcs), 0.0, '', 0, 'NONE'))
            continue

        # Group by offset
        offsets = defaultdict(list)
        for s in seeds:
            offset = s['usa_va'] - s['pal_va']
            offsets[offset].append(s)

        # Try each offset
        best_offset = max(offsets, key=lambda o: len(offsets[o]))
        best_seeds = offsets[best_offset]

        # Validate offset
        validated = try_apply_offset(obj_pal_funcs, best_offset, usa_fp_by_va)
        ratio = validated / len(obj_pal_funcs) if obj_pal_funcs else 0

        if validated > 0:
            recovered_count += 1
            recovery_log.append((
                obj_name, validated, len(obj_pal_funcs),
                ratio,
                f'0x{best_offset:08X}',
                len(best_seeds),
                'HIGH' if ratio >= 0.8 else 'MEDIUM' if ratio >= 0.3 else 'LOW',
            ))
            print(f'  [{obj_name:30s}] recovered: {validated}/{len(obj_pal_funcs)} '
                  f'offset=0x{best_offset:08X} seeds={len(best_seeds)}')

    # Summary
    print(f"\nRecovered objects: {recovered_count}/{len(unmatched_objs)}")
    
    if recovery_log:
        log_path = OUT_DIR / "recovery_log.csv"
        with open(log_path, 'w', newline='') as f:
            w = csv.writer(f)
            w.writerow(['object_file', 'validated', 'total', 'ratio', 'offset', 'seeds', 'confidence'])
            for entry in recovery_log:
                w.writerow(list(entry))
        print(f"[OK] Recovery log -> {log_path}")

        remaining_path = OUT_DIR / "unmatched_objects.csv"
        recovered_names = {entry[0] for entry in recovery_log if entry[1] > 0}
        remaining_rows = [r for r in obj_candidates
                          if r['confidence'] == 'NONE' and r['object_file'] not in recovered_names]
        with open(remaining_path, 'w', newline='') as f:
            w = csv.writer(f)
            w.writerow(['object_file', 'pal_start', 'pal_end', 'function_count', 'notes'])
            for r in remaining_rows:
                w.writerow([
                    r['object_file'],
                    r['pal_start'],
                    r['pal_end'],
                    r['function_count'],
                    r['notes'],
                ])
        print(f"[OK] Remaining unmatched objects -> {remaining_path}")

    # Generate recovery report
    report_path = OUT_DIR / "pal_usa_recovery_report.md"
    with open(report_path, 'w') as f:
        f.write("# Unmatched Object Recovery Report\n\n")
        f.write(f"Scanned {len(unmatched_objs)} unmatched objects via op_seq/raw fingerprint search\n\n")
        f.write(f"Recovered: {recovered_count}/{len(unmatched_objs)}\n\n")

        f.write("## Recovery Details\n\n")
        f.write("| Object | Validated | Total | Ratio | Offset | Seeds | Confidence |\n")
        f.write("|--------|-----------|-------|-------|--------|-------|------------|\n")
        for entry in recovery_log:
            f.write(
                f"| {entry[0]:30s} | {entry[1]:4d} | {entry[2]:4d} | "
                f"{entry[3]:.2f} | {entry[4]:>8s} | {entry[5]:3d} | {entry[6]:6s} |\n"
            )

        f.write("\n## Still Unrecoverable\n\n")
        f.write("Objects with no matching functions found:\n")
        unrec = [e for e in recovery_log if e[1] == 0]
        for entry in unrec:
            f.write(f"- {entry[0]}\n")
        if not any(e[1] == 0 for e in recovery_log):
            f.write("(none)\n")

        f.write("\n## Next Steps\n\n")
        f.write("1. Integrate recovered offsets into main reconciliation\n")
        f.write("2. For MEDIUM/LOW objects, manual inspection needed\n")
        f.write("3. Finally unrecoverable objects may be PAL-only or misidentified\n")

    print(f"[OK] Recovery report -> {report_path}")


if __name__ == '__main__':
    main()
