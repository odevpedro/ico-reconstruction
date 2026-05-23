#!/usr/bin/env python3
"""Reconcile PAL→USA function addresses by .o file ranges.

Strategy:
  1. Parse MAIN.MAP to group PAL functions by .o file (in link order).
  2. Load seeds: 592+ verified PAL→USA matches.
  3. For each .o file with >=1 seed, compute the USA range offset.
  4. Map ALL functions in that .o file by relative order.
  5. Validate using pre-computed fingerprints (op_seq_hash).
  6. Classify confidence: HIGH / MEDIUM / LOW / INVALID.

Outputs in research/pal-usa/:
  pal_usa_verified_seeds.csv
  pal_usa_object_range_candidates.csv
  pal_usa_function_map_candidates.csv
  unmatched_objects.csv
  pal_usa_reconciliation_summary.md
"""
import csv, json, sys
from pathlib import Path
from collections import defaultdict

PROJECT_ROOT = Path(__file__).resolve().parent.parent.parent
PAL_DIR = PROJECT_ROOT / ".local" / "extracted" / "pal"
OUT_DIR = PROJECT_ROOT / "research" / "pal-usa"


def load_main_map_functions(path: Path) -> list[dict]:
    funcs = []
    with open(path) as f:
        for row in csv.DictReader(f):
            funcs.append({
                'pal_va': int(row['pal_va'], 16),
                'name': row['name'],
                'object_file': row['object_file'],
            })
    return funcs


def group_by_object(funcs: list[dict]) -> dict[str, list[dict]]:
    grouped = defaultdict(list)
    for f in funcs:
        grouped[f['object_file']].append(f)
    for obj in grouped:
        grouped[obj].sort(key=lambda x: x['pal_va'])
    return dict(grouped)


RELIABLE_METHODS = {'raw_sha1', 'op_seq_hash'}


def load_seeds(path: Path) -> dict[int, dict]:
    """Returns {pal_va: {usa_va, symbol, confidence, match_method}}
    
    Only includes reliable seeds: raw_sha1 (binary-identical) or 
    op_seq_hash (instruction pattern). Excludes same_va which has 
    systematic false positives (seeds at same PAL/USA address but 
    different function bodies).
    """
    seeds = {}
    with open(path) as f:
        for row in csv.DictReader(f):
            if row['status'] != 'verified' or not row['usa_va']:
                continue
            method = row['match_method']
            if method not in RELIABLE_METHODS:
                continue
            pal_va = int(row['pal_va'], 16)
            seeds[pal_va] = {
                'usa_va': int(row['usa_va'], 16),
                'symbol': row['symbol'],
                'confidence': float(row['confidence']),
                'match_method': method,
            }
    return seeds


def load_fingerprint_map(path: Path) -> dict[int, dict]:
    """Returns {va: fp_dict}"""
    with open(path) as f:
        fps = json.load(f)
    return {fp['va']: fp for fp in fps}


# ---------------------------------------------------------------------------
def reconcile_object(
    obj_name: str,
    obj_pal_funcs: list[dict],
    pal_fp_by_va: dict,
    usa_fp_by_va: dict,
    seeds: dict[int, dict],
) -> dict:
    """Reconcile a single .o file. Returns result dict."""
    pal_start = obj_pal_funcs[0]['pal_va']
    pal_end = obj_pal_funcs[-1]['pal_va']
    n_funcs = len(obj_pal_funcs)

    pal_fps_in_order = []
    for f in obj_pal_funcs:
        fp = pal_fp_by_va.get(f['pal_va'])
        if fp is None:
            # Find nearest next fingerprint
            all_pal = sorted(pal_fp_by_va.keys())
            idx = bisect_left(all_pal, f['pal_va'])
            fp = pal_fp_by_va.get(all_pal[idx]) if idx < len(all_pal) else None
        pal_fps_in_order.append(fp)

    # Find seeds within this object
    obj_seed_infos = []
    for i, f in enumerate(obj_pal_funcs):
        if f['pal_va'] in seeds:
            obj_seed_infos.append((i, f, seeds[f['pal_va']]))

    result = {
        'object_file': obj_name,
        'pal_start': pal_start,
        'pal_end': pal_end,
        'function_count': n_funcs,
        'seed_count': len(obj_seed_infos),
        'offset_used': 0,
        'functions_mapped': 0,
        'confidence': 'NONE',
        'notes': 'No seeds for this object',
        'function_results': [],
    }

    if not obj_seed_infos:
        return result

    # Compute offsets from each seed
    offsets = [(seed_info['usa_va'] - f['pal_va'], i, f, seed_info)
               for i, f, seed_info in obj_seed_infos]
    unique_offsets = set(o[0] for o in offsets)

    if len(unique_offsets) > 1:
        # Check if the offset difference matches a known pattern
        offset_counts = defaultdict(int)
        for o, _, _, _ in offsets:
            offset_counts[o] += 1
        best_offset = max(offset_counts, key=offset_counts.get)
        result['offset_used'] = best_offset
        result['notes'] = (f'Inconsistent seed offsets: {len(unique_offsets)} unique. '
                           f'Using offset=0x{best_offset:08X} (mode, {offset_counts[best_offset]}/{len(obj_seed_infos)}). '
                           f'Discarded: ')
        discarded = [o for o in unique_offsets if o != best_offset]
        result['notes'] += ', '.join(f'0x{o:08X}' for o in discarded)
        result['confidence'] = 'LOW'
    else:
        best_offset = list(unique_offsets)[0]
        result['offset_used'] = best_offset
        result['notes'] = f'Consistent offset=0x{best_offset:08X} from {len(obj_seed_infos)} seeds'

    # Map each function: infer USA VA, validate via pre-computed fingerprint
    validated_count = 0
    seq_mismatch_count = 0
    no_fp_count = 0

    for i, f in enumerate(obj_pal_funcs):
        pal_va = f['pal_va']
        usa_va = pal_va + best_offset
        func_name = f['name']
        pal_fp = pal_fps_in_order[i]

        # Look up USA fingerprint at inferred address
        usa_fp = usa_fp_by_va.get(usa_va)

        if pal_fp is None:
            result['function_results'].append({
                'pal_va': f'0x{pal_va:08X}',
                'usa_va': f'0x{usa_va:08X}',
                'name': func_name,
                'status': 'NO_FP',
                'notes': 'No PAL fingerprint',
            })
            no_fp_count += 1
            continue

        if usa_fp is None:
            # Try searching nearby
            all_usa = sorted(usa_fp_by_va.keys())
            idx = bisect_left(all_usa, usa_va)
            candidates = []
            for j in range(max(0, idx-5), min(len(all_usa), idx+5)):
                candidates.append((all_usa[j], abs(all_usa[j] - usa_va)))
            candidates.sort(key=lambda x: x[1])
            found = False
            for cand_va, dist in candidates:
                if dist > 128:
                    break
                cand_fp = usa_fp_by_va[cand_va]
                if cand_fp['op_seq_hash'] == pal_fp['op_seq_hash'] and cand_fp['size'] == pal_fp['size']:
                    usa_fp = cand_fp
                    usa_va = cand_va
                    found = True
                    break

        if usa_fp is None:
            result['function_results'].append({
                'pal_va': f'0x{pal_va:08X}',
                'usa_va': f'0x{usa_va:08X}',
                'name': func_name,
                'status': 'NO_USA_FP',
                'notes': 'No USA fingerprint at inferred address',
            })
            no_fp_count += 1
            continue

        # Validate: compare op_seq_hash AND size
        if pal_fp['op_seq_hash'] == usa_fp['op_seq_hash'] and pal_fp['size'] == usa_fp['size']:
            validated_count += 1
            result['function_results'].append({
                'pal_va': f'0x{pal_va:08X}',
                'usa_va': f'0x{usa_va:08X}',
                'name': func_name,
                'status': 'MATCH',
                'notes': f'op_seq+size match (offset=0x{best_offset:08X})',
            })
        elif pal_fp['op_seq_hash'] == usa_fp['op_seq_hash']:
            validated_count += 1
            result['function_results'].append({
                'pal_va': f'0x{pal_va:08X}',
                'usa_va': f'0x{usa_va:08X}',
                'name': func_name,
                'status': 'MATCH_SZ',
                'notes': f'op_seq match, sz diff: PAL={pal_fp["size"]} USA={usa_fp["size"]}',
            })
        elif pal_fp['size'] == usa_fp['size'] and pal_fp['raw_sha1'] == usa_fp['raw_sha1']:
            validated_count += 1
            result['function_results'].append({
                'pal_va': f'0x{pal_va:08X}',
                'usa_va': f'0x{usa_va:08X}',
                'name': func_name,
                'status': 'MATCH_BIN',
                'notes': 'Binary-identical (same offset)',
            })
        else:
            seq_mismatch_count += 1
            result['function_results'].append({
                'pal_va': f'0x{pal_va:08X}',
                'usa_va': f'0x{usa_va:08X}',
                'name': func_name,
                'status': 'SEQ_MISMATCH',
                'notes': f'op_seq differs (sz PAL={pal_fp["size"]} USA={usa_fp["size"]})',
            })

    result['functions_mapped'] = validated_count

    # Classify confidence
    effective_total = n_funcs - no_fp_count
    if effective_total <= 0:
        effective_total = n_funcs

    ratio = validated_count / effective_total if effective_total > 0 else 0

    if ratio >= 1.0 and n_funcs > 0:
        result['confidence'] = 'HIGH'
        result['notes'] = (f'All {validated_count}/{n_funcs} functions validated; '
                           f'offset=0x{best_offset:08X}')
    elif ratio >= 0.5:
        result['confidence'] = 'MEDIUM'
        result['notes'] = (f'{validated_count}/{n_funcs} validated ({seq_mismatch_count} seq, '
                           f'{no_fp_count} no_fp); offset=0x{best_offset:08X}')
    elif validated_count > 0:
        result['confidence'] = 'LOW'
        result['notes'] = (f'{validated_count}/{n_funcs} validated ({seq_mismatch_count} seq, '
                           f'{no_fp_count} no_fp); offset=0x{best_offset:08X}')
    elif len(unique_offsets) > 1:
        result['confidence'] = 'INVALID'
        result['notes'] = (f'Inconsistent seed offsets: {len(unique_offsets)} unique')
    else:
        result['confidence'] = 'LOW'
        result['notes'] = f'Seeds consistent ({best_offset}) but 0/{n_funcs} validated'

    return result


def bisect_left(a, x):
    lo, hi = 0, len(a)
    while lo < hi:
        mid = (lo + hi) // 2
        if a[mid] < x:
            lo = mid + 1
        else:
            hi = mid
    return lo


# ---------------------------------------------------------------------------
def main():
    OUT_DIR.mkdir(parents=True, exist_ok=True)

    func_csv = OUT_DIR / "main_map_functions.csv"
    if not func_csv.exists():
        print("[ERR] Run parse_main_map.py first")
        sys.exit(1)

    all_funcs = load_main_map_functions(func_csv)
    funcs_by_obj = group_by_object(all_funcs)
    print(f"[LOAD] {len(funcs_by_obj)} objects, {len(all_funcs)} functions")

    seeds = load_seeds(PROJECT_ROOT / "docs" / "symbols" / "pal_usa_symbol_map.csv")
    print(f"[SEED] {len(seeds)} verified PAL→USA matches")

    pal_fp_by_va = load_fingerprint_map(PROJECT_ROOT / "docs" / "symbols" / "pal_fingerprints.json")
    usa_fp_by_va = load_fingerprint_map(PROJECT_ROOT / "docs" / "symbols" / "usa_fingerprints.json")
    print(f"[FP] {len(pal_fp_by_va)} PAL, {len(usa_fp_by_va)} USA fingerprints")

    # Remove non-object entries from funcs_by_obj (__text__, .plt, etc.)
    valid_objects = {k: v for k, v in funcs_by_obj.items()
                     if not k.startswith('__') and k != '.plt'}

    # Reconcile each object
    results = []
    all_func_results = []

    for obj_name in sorted(valid_objects, key=lambda n: valid_objects[n][0]['pal_va']):
        obj_funcs = valid_objects[obj_name]
        r = reconcile_object(obj_name, obj_funcs, pal_fp_by_va, usa_fp_by_va, seeds)
        results.append(r)
        all_func_results.extend(r['function_results'])

    # --- Output: object range candidates ---
    range_csv = OUT_DIR / "pal_usa_object_range_candidates.csv"
    with open(range_csv, 'w', newline='') as f:
        w = csv.writer(f)
        w.writerow([
            'object_file', 'pal_start', 'pal_end', 'function_count',
            'seed_count', 'offset_used',
            'functions_mapped', 'confidence', 'notes',
        ])
        for r in sorted(results, key=lambda x: x['pal_start']):
            w.writerow([
                r['object_file'],
                f'0x{r["pal_start"]:08X}',
                f'0x{r["pal_end"]:08X}',
                r['function_count'],
                r['seed_count'],
                f'0x{r["offset_used"]:08X}',
                r['functions_mapped'],
                r['confidence'],
                r['notes'],
            ])
    print(f"[OK] Object range candidates -> {range_csv}")

    # --- Output: function map candidates ---
    func_out = OUT_DIR / "pal_usa_function_map_candidates.csv"
    with open(func_out, 'w', newline='') as f:
        w = csv.writer(f)
        w.writerow(['pal_va', 'usa_va', 'name', 'object_file', 'status', 'notes'])
        # Build pal_va -> object lookup
        pal_to_obj = {f['pal_va']: f['object_file'] for f in all_funcs}
        for r in all_func_results:
            obj = pal_to_obj.get(int(r['pal_va'], 16), '')
            w.writerow([
                r['pal_va'], r['usa_va'], r['name'],
                obj, r['status'], r['notes'],
            ])
    print(f"[OK] Function map candidates -> {func_out}")

    # --- Output: unmatched objects ---
    unmatched = [r for r in results if r['confidence'] == 'NONE']
    unmatched_csv = OUT_DIR / "unmatched_objects.csv"
    with open(unmatched_csv, 'w', newline='') as f:
        w = csv.writer(f)
        w.writerow(['object_file', 'pal_start', 'pal_end', 'function_count', 'notes'])
        for r in sorted(unmatched, key=lambda x: x['pal_start']):
            w.writerow([
                r['object_file'],
                f'0x{r["pal_start"]:08X}',
                f'0x{r["pal_end"]:08X}',
                r['function_count'],
                r['notes'],
            ])
    print(f"[OK] Unmatched objects -> {unmatched_csv}")

    # --- Output: verified seeds ---
    seed_out = OUT_DIR / "pal_usa_verified_seeds.csv"
    with open(seed_out, 'w', newline='') as f:
        w = csv.writer(f)
        w.writerow(['symbol', 'pal_va', 'usa_va', 'match_method', 'confidence'])
        for pal_va, s in sorted(seeds.items()):
            w.writerow([
                s['symbol'],
                f'0x{pal_va:08X}',
                f'0x{s["usa_va"]:08X}',
                s['match_method'],
                f'{s["confidence"]:.2f}',
            ])
    print(f"[OK] Verified seeds -> {seed_out}")

    # --- Summary stats ---
    high = [r for r in results if r['confidence'] == 'HIGH']
    medium = [r for r in results if r['confidence'] == 'MEDIUM']
    low = [r for r in results if r['confidence'] == 'LOW']
    invalid = [r for r in results if r['confidence'] == 'INVALID']
    none = unmatched

    high_funcs = sum(r['functions_mapped'] for r in high)
    medium_funcs = sum(r['functions_mapped'] for r in medium)
    low_funcs = sum(r['functions_mapped'] for r in low)
    total_mapped = high_funcs + medium_funcs + low_funcs
    total_with_seeds = len(results) - len(none)

    print()
    print("=" * 60)
    print("RECONCILIATION SUMMARY")
    print("=" * 60)
    print(f"  Objects with seeds: {total_with_seeds}/{len(results)}")
    print(f"  HIGH confidence:   {len(high):3d} objects, {high_funcs:5d} functions")
    print(f"  MEDIUM confidence: {len(medium):3d} objects, {medium_funcs:5d} functions")
    print(f"  LOW confidence:   {len(low):3d} objects, {low_funcs:5d} functions")
    print(f"  INVALID:          {len(invalid):3d} objects")
    print(f"  Unmatched:        {len(none):3d} objects (no seeds)")
    print(f"  TOTAL mapped:     {total_mapped:5d} functions")

    # Breakdown by match type
    match_types = defaultdict(int)
    for r in all_func_results:
        if r['status'] in ('MATCH', 'MATCH_SZ', 'MATCH_BIN'):
            match_types[r['status']] += 1
    print(f"  MATCH (op_seq+size): {match_types.get('MATCH', 0)}")
    print(f"  MATCH (op_seq only): {match_types.get('MATCH_SZ', 0)}")
    print(f"  MATCH (binary):      {match_types.get('MATCH_BIN', 0)}")

    # --- Summary markdown ---
    import datetime
    summary = OUT_DIR / "pal_usa_reconciliation_summary.md"
    with open(summary, 'w') as f:
        f.write("# PAL→USA Reconciliation Summary\n\n")
        f.write(f"Generated: {datetime.datetime.now().strftime('%Y-%m-%d %H:%M')}\n\n")
        f.write("## Inputs\n\n")
        f.write(f"- MAIN.MAP: {len(all_funcs)} functions in {len(valid_objects)} objects\n")
        f.write(f"- Verified seeds: {len(seeds)} PAL→USA matches\n")
        f.write(f"- PAL fingerprints: {len(pal_fp_by_va)}\n")
        f.write(f"- USA fingerprints: {len(usa_fp_by_va)}\n\n")
        f.write("## Results\n\n")
        f.write(f"| Confidence | Objects | Functions Mapped |\n")
        f.write(f"|------------|---------|------------------|\n")
        f.write(f"| HIGH       | {len(high)} | {high_funcs} |\n")
        f.write(f"| MEDIUM     | {len(medium)} | {medium_funcs} |\n")
        f.write(f"| LOW        | {len(low)} | {low_funcs} |\n")
        f.write(f"| INVALID    | {len(invalid)} | — |\n")
        f.write(f"| No Seeds   | {len(none)} | — |\n")
        f.write(f"| **Total**  | **{len(results)}** | **{total_mapped}** |\n\n")

        f.write(f"### Match Quality\n\n")
        f.write(f"- op_seq+size: {match_types.get('MATCH', 0)}\n")
        f.write(f"- op_seq only: {match_types.get('MATCH_SZ', 0)}\n")
        f.write(f"- binary identical: {match_types.get('MATCH_BIN', 0)}\n\n")

        if invalid:
            f.write("## Invalid Objects\n\n")
            f.write("| Object | PAL Range | Notes |\n")
            f.write("|--------|-----------|-------|\n")
            for r in results:
                if r['confidence'] == 'INVALID':
                    f.write(f"| {r['object_file']} | 0x{r['pal_start']:08X}-0x{r['pal_end']:08X} | {r['notes']} |\n")
            f.write("\n")

        # Top HIGH/MEDIUM objects
        f.write("## Top Objects\n\n")
        f.write("| Confidence | Object | Mapped | Total | Ratio | Offset |\n")
        f.write("|------------|--------|--------|-------|-------|--------|\n")
        for r in sorted(results, key=lambda x: -x['functions_mapped']):
            if r['confidence'] in ('HIGH', 'MEDIUM') or r['functions_mapped'] > 0:
                ratio = r['functions_mapped'] / r['function_count'] if r['function_count'] > 0 else 0
                f.write(f"| {r['confidence']:10s} | {r['object_file']:30s} | "
                        f"{r['functions_mapped']:4d}/{r['function_count']:4d} | "
                        f"{ratio:.2f} | 0x{r['offset_used']:08X} |\n")

        f.write("\n## Next Steps\n\n")
        f.write("1. Apply HIGH confidence objects as ground truth\n")
        f.write("2. For MEDIUM objects without seq mismatches, promote to HIGH\n")
        f.write("3. For MEDIUM objects with seq mismatches, inspect PAL→USA differences\n")
        f.write("4. For LOW objects with consistent seed offset, investigate why validation fails\n")
        f.write("5. For unmatched objects, find seeds via: byte-exact match of 1st function\n")
        f.write("6. Generate `symbol_addrs_usa.txt` for splat/Ghidra after review\n")
        f.write("7. Cross-check vs `SRCFILE.TXT` for source-line validation\n")

    print(f"[OK] Summary -> {summary}")


if __name__ == '__main__':
    main()
