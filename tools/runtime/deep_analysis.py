#!/usr/bin/env python3
"""
Deep analysis of Rev.108 runtime session data.

Cross-references all CSVs to produce:
1. Entity work area → descriptor table mapping
2. World state → entity type matrix
3. Dispatch pattern analysis
4. Temporal analysis (room visit duration, transition frequency)
5. Entity lifecycle analysis (creation, binding, destruction)
6. DL slot allocation patterns

Usage:
    python3 tools/runtime/deep_analysis.py [--csv-dir research/runtime/rev108]
"""

import csv
import sys
import os
from collections import defaultdict, Counter
from pathlib import Path


def read_csv(path):
    """Read a CSV file and return list of dicts."""
    rows = []
    with open(path, 'r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            rows.append(row)
    return rows


def parse_int(s):
    """Convert a decimal or 0x-prefixed integer string to int."""
    if not s or s == '?':
        return None
    try:
        return int(s, 0)
    except ValueError:
        return None


def region(v):
    """Identify memory region for a pointer."""
    if v is None:
        return "UNKNOWN"
    if v == 0:
        return "NULL"
    if v < 0x100000:
        return "ELF"
    if v < 0x280000:
        return ".data"
    if v < 0x6A0000:
        return "BSS"
    if v < 0x800000:
        return "BSS-ext"
    if v < 0x2000000:
        return "Heap-early"
    if v < 0x10000000:
        return "Heap"
    return "Heap-hi"


def analyze_entity_workareas(csv_dir):
    """Analyze entity work area pointers and their properties."""
    print("=" * 80)
    print("SECTION 1: ENTITY WORK AREA ANALYSIS")
    print("=" * 80)

    ew = read_csv(os.path.join(csv_dir, "entity_workarea_pointers.csv"))

    total_events = sum(int(r['event_count']) for r in ew)
    print(f"\nTotal entity-related ios_om_main events: {total_events:,}")
    print(f"Unique entity work areas: {len(ew)}")

    # Region classification
    regions = Counter()
    for r in ew:
        addr = parse_int(r['entity_ptr'])
        regions[region(addr)] += 1

    print("\n--- Memory Region Distribution ---")
    for reg, count in regions.most_common():
        print(f"  {reg:15s}: {count:3d} entities")

    # Top entities by event count
    print("\n--- Top 15 Entity Work Areas by Event Count ---")
    print(f"  {'Address':12s} {'Events':>10s} {'ws#':>3s} {'DL#':>3s} {'GObj#':>4s}  World States")
    for r in sorted(ew, key=lambda x: int(x['event_count']), reverse=True)[:15]:
        ws_list = r.get('world_states', '').strip('"')
        print(f"  {r['entity_ptr']:12s} {int(r['event_count']):>10,} {r['unique_ws']:>3s} {r['unique_dl_slots']:>3s} {r['unique_gobjs']:>4s}  {ws_list}")

    # Single-world-state entities (strong room affinity)
    print("\n--- Single World State Entities (strong room affinity) ---")
    single_ws = [r for r in ew if int(r['unique_ws']) == 1]
    print(f"  Count: {len(single_ws)}/{len(ew)} ({100*len(single_ws)/len(ew):.1f}%)")
    for r in sorted(single_ws, key=lambda x: int(x['event_count']), reverse=True)[:20]:
        print(f"  {r['entity_ptr']:12s} {int(r['event_count']):>8,} events → ws={r['world_states']}")

    # Multi-world-state entities (persistent / shared)
    multi_ws = [r for r in ew if int(r['unique_ws']) >= 4]
    print(f"\n--- High-Flexibility Entities (≥4 world states) ---")
    print(f"  Count: {len(multi_ws)}")
    for r in sorted(multi_ws, key=lambda x: int(x['event_count']), reverse=True):
        ws_list = r.get('world_states', '').strip('"')
        print(f"  {r['entity_ptr']:12s} {int(r['event_count']):>8,} events, {r['unique_ws']} ws, {r['unique_gobjs']} gobjs → {ws_list}")

    return ew


def analyze_entity_gobj_binding(csv_dir):
    """Analyze entity↔GObj binding patterns."""
    print("\n" + "=" * 80)
    print("SECTION 2: ENTITY ↔ GOBJ BINDING ANALYSIS")
    print("=" * 80)

    bindings = read_csv(os.path.join(csv_dir, "entity_gobj_binding.csv"))

    total = sum(int(r['event_count']) for r in bindings)
    print(f"\nTotal bound events: {total:,}")
    print(f"Unique bindings: {len(bindings)}")

    # Entity → multiple GObjs
    entity_to_gobjs = defaultdict(list)
    for r in bindings:
        entity_to_gobjs[r['entity_ptr']].append((r['gobj_ptr'], int(r['event_count'])))

    print("\n--- Entities with Most GObjs (multi-instance) ---")
    multi = sorted(entity_to_gobjs.items(), key=lambda x: len(x[1]), reverse=True)
    for ent, gobj_list in multi[:20]:
        total_ent_events = sum(e for _, e in gobj_list)
        gobj_addrs = sorted([g for g, _ in gobj_list])
        print(f"  Entity {ent}: {len(gobj_list)} GObjs, {total_ent_events:>10,} events")
        for gobj, cnt in sorted(gobj_list, key=lambda x: x[1], reverse=True)[:5]:
            print(f"    GObj {gobj} → {cnt:>8,} events")

    # Dominant GObj per entity
    print("\n--- Dominant GObj Binding (top 95% of events per entity) ---")
    dominant = []
    for ent, gobj_list in entity_to_gobjs.items():
        total_ent = sum(e for _, e in gobj_list)
        if len(gobj_list) > 1:
            for gobj, cnt in gobj_list:
                if cnt / total_ent > 0.90:
                    dominant.append((ent, gobj, cnt, total_ent, 100*cnt/total_ent))
    for ent, gobj, cnt, total, pct in sorted(dominant, key=lambda x: x[2], reverse=True)[:20]:
        print(f"  {ent} → {gobj}: {cnt:>10,}/{total:>10,} ({pct:.1f}%)")

    # GObj address ranges
    gobj_regions = Counter()
    for r in bindings:
        addr = parse_int(r['gobj_ptr'])
        gobj_regions[region(addr)] += 1

    print("\n--- GObj Memory Region Distribution ---")
    for reg, count in gobj_regions.most_common():
        print(f"  {reg:15s}: {count:3d} bindings")

    return bindings


def analyze_world_states(csv_dir):
    """Analyze world state transitions and room topology."""
    print("\n" + "=" * 80)
    print("SECTION 3: WORLD STATE / ROOM TOPOLOGY")
    print("=" * 80)

    timeline = read_csv(os.path.join(csv_dir, "world_state_timeline.csv"))
    hot = read_csv(os.path.join(csv_dir, "hot_paths_by_world_state.csv"))

    # Transition graph
    transitions = defaultdict(int)
    transition_durations = defaultdict(list)
    for r in timeline:
        fr = r['from_ws']
        to = r['to_ws']
        delta = parse_int(r.get('delta_cycles', '0'))
        transitions[(fr, to)] += 1
        if delta is not None and delta > 0:
            transition_durations[(fr, to)].append(delta)

    print("\n--- World State Transition Graph ---")
    print(f"  Total transitions: {len(timeline)}")
    print(f"  Unique directed edges: {len(transitions)}")
    print()

    # Adjacency list
    adj = defaultdict(set)
    for (fr, to), cnt in transitions.items():
        adj[fr].add(to)

    # Visit frequency
    visit_count = Counter()
    for r in timeline:
        visit_count[r['to_ws']] += 1

    print(f"  {'ws':>12s} {'events':>8s} {'visits':>7s} {'neighbors':>9s}  Adjacent to")
    for r in sorted(hot, key=lambda x: int(x['total_events']), reverse=True):
        ws = r['world_state']
        events = int(r['total_events'])
        visits = visit_count.get(ws, 0)
        neighbors = sorted(adj.get(ws, set()), key=lambda x: parse_int(x) if parse_int(x) is not None else 0)
        n_str = ', '.join(neighbors[:8])
        print(f"  {ws:>12s} {events:>8,} {visits:>7d} {len(neighbors):>9d}  {n_str}")

    # Room duration estimates (cycle deltas between transitions)
    print("\n--- Room Duration Estimates (PS2 cycles, ~300MHz) ---")
    ws_duration = defaultdict(list)
    for r in timeline:
        delta = parse_int(r.get('delta_cycles', '0'))
        if delta is not None and delta > 0:
            ws_duration[r['from_ws']].append(delta)

    print(f"  {'ws':>12s} {'min_cycles':>15s} {'max_cycles':>15s} {'avg_cycles':>15s} {'est_seconds':>12s}")
    for ws in sorted(ws_duration.keys(), key=lambda x: parse_int(x) if parse_int(x) else 0):
        vals = ws_duration[ws]
        mn, mx, avg = min(vals), max(vals), sum(vals) / len(vals)
        est_s = avg / 300_000_000  # ~300MHz EE
        print(f"  {ws:>12s} {mn:>15,} {mx:>15,} {avg:>15,.0f} {est_s:>12.1f}")

    return timeline, hot


def analyze_dl_slots(csv_dir):
    """Analyze DL slot allocation and world state affinity."""
    print("\n" + "=" * 80)
    print("SECTION 4: DL SLOT ALLOCATION PATTERNS")
    print("=" * 80)

    matrix = read_csv(os.path.join(csv_dir, "dispatch_slot_ws_matrix.csv"))
    slots = read_csv(os.path.join(csv_dir, "callback_hits_by_dl_slot.csv"))

    # Get world state columns from matrix
    ws_cols = [c for c in matrix[0].keys() if c != 'dl_slot']

    # The CSV column is historical: these values are the a2/t0 dispatch type
    # values captured at _iosOmMain, not proven memory addresses.
    print("\n--- Dispatch Type Values ---")
    print(f"  {'value':>12s} {'total':>10s} {'ws#':>3s} {'shape':>20s}")
    for r in sorted(slots, key=lambda x: int(x['total_ios_om_main']), reverse=True):
        slot_addr = r['dl_slot']
        total = int(r['total_ios_om_main'])
        ws_count = int(r['unique_ws'])
        addr_int = parse_int(slot_addr)

        # Classify the captured value shape without asserting pointer semantics.
        if addr_int is not None:
            if addr_int >= 0x10000:
                slot_type = "pointer-like/outlier"
            else:
                slot_type = "small type/index"
        else:
            slot_type = "PARSE-ERROR"

        print(f"  {slot_addr:>12s} {total:>10,} {ws_count:>3d} {slot_type:>20s}")

    # Primary dispatch type per world state.
    print("\n--- Primary Dispatch Type per World State ---")
    ws_primary = {}
    for col in ws_cols:
        candidates = [
            (r['dl_slot'], int(r.get(col, '0')))
            for r in matrix
            if int(r.get(col, '0')) > 0
        ]
        if candidates:
            ws_primary[col] = max(candidates, key=lambda item: item[1])

    for ws in sorted(ws_primary.keys(), key=lambda x: parse_int(x) if parse_int(x) else 0):
        slot, cnt = ws_primary[ws]
        print(f"  {ws:>12s} → slot {slot:>12s} ({cnt:>8,} events)")

    # Exclusive slots (used by only 1 world state)
    print("\n--- Exclusive Dispatch Types (observed in one world state) ---")
    for r in matrix:
        slot = r['dl_slot']
        active_ws = [(col, int(r.get(col, '0'))) for col in ws_cols if int(r.get(col, '0')) > 0]
        if len(active_ws) == 1 and active_ws[0][1] > 100:
            ws_name, cnt = active_ws[0]
            print(f"  slot {slot:>12s} ← ws {ws_name:>12s} ({cnt:>8,} events)")

    return matrix, slots


def analyze_temporal(csv_dir):
    """Temporal analysis of session: visit order, revisit patterns."""
    print("\n" + "=" * 80)
    print("SECTION 5: TEMPORAL SESSION ANALYSIS")
    print("=" * 80)

    timeline = read_csv(os.path.join(csv_dir, "world_state_timeline.csv"))

    # Reconstruct visit sequence
    visit_order = []
    for i, r in enumerate(timeline):
        if i == 0:
            visit_order.append(r['from_ws'])
        visit_order.append(r['to_ws'])

    # Count revisits
    revisit_count = Counter()
    seen = set()
    for ws in visit_order:
        if ws in seen:
            revisit_count[ws] += 1
        seen.add(ws)

    # First visit vs revisit patterns
    print("\n--- Visit Sequence (first 40 transitions) ---")
    for i, ws in enumerate(visit_order[:40]):
        marker = " *" if revisit_count.get(ws, 0) > 0 and visit_order.index(ws) < i else ""
        print(f"  [{i:3d}] ws={ws}{marker}")

    # Back-and-forth patterns
    print("\n--- Back-and-Forth Patterns ---")
    bf = Counter()
    for i in range(len(visit_order) - 1):
        a, b = visit_order[i], visit_order[i + 1]
        if i + 2 < len(visit_order) and visit_order[i + 2] == a:
            bf[(a, b)] += 1

    for (a, b), cnt in bf.most_common(20):
        if cnt >= 2:
            print(f"  {a} ↔ {b}: {cnt} oscillations")

    # Total unique rooms visited
    unique_ws = set(visit_order)
    print(f"\n--- Session Summary ---")
    print(f"  Total transitions: {len(timeline)}")
    print(f"  Unique world states visited: {len(unique_ws)}")
    print(f"  World states: {', '.join(sorted(unique_ws, key=lambda x: parse_int(x) if parse_int(x) else 0))}")

    return visit_order


def analyze_init_scene(csv_dir):
    """Analyze initSceneGObj frequency per world state."""
    print("\n" + "=" * 80)
    print("SECTION 6: initSceneGObj FREQUENCY ANALYSIS")
    print("=" * 80)

    init = read_csv(os.path.join(csv_dir, "init_scene_by_world_state.csv"))

    total = sum(int(r['init_scene_count']) for r in init)
    print(f"\nTotal initSceneGObj calls: {total:,}")
    print(f"World states with scene loads: {len(init)}")

    print("\n--- initSceneGObj per World State ---")
    print(f"  {'ws':>12s} {'count':>8s} {'pct':>6s}  {'bar'}")
    for r in sorted(init, key=lambda x: int(x['init_scene_count']), reverse=True):
        cnt = int(r['init_scene_count'])
        pct = 100 * cnt / total
        bar = '#' * int(pct / 2)
        print(f"  {r['world_state']:>12s} {cnt:>8,} {pct:>5.1f}%  {bar}")

    return init


def cross_reference_all(csv_dir):
    """Final cross-reference: combine all data sources."""
    print("\n" + "=" * 80)
    print("SECTION 7: CROSS-REFERENCE SYNTHESIS")
    print("=" * 80)

    ew = read_csv(os.path.join(csv_dir, "entity_workarea_pointers.csv"))
    gobj = read_csv(os.path.join(csv_dir, "entity_gobj_binding.csv"))
    hot = read_csv(os.path.join(csv_dir, "hot_paths_by_world_state.csv"))

    # Build entity → world state → gobj map
    ew_ws_map = {}
    for r in ew:
        addr = r['entity_ptr']
        ws_list = r.get('world_states', '').strip('"').split(',')
        ew_ws_map[addr] = set(ws_list)

    gobj_map = defaultdict(list)
    for r in gobj:
        gobj_map[r['entity_ptr']].append(r['gobj_ptr'])

    # Cross-reference: world_state → entity types → GObj count
    ws_entities = defaultdict(list)
    for r in ew:
        ws_list = r.get('world_states', '').strip('"').split(',')
        for ws in ws_list:
            ws = ws.strip()
            if ws:
                ws_entities[ws].append(r)

    print("\n--- World State → Entity Inventory ---")
    for ws in sorted(ws_entities.keys(), key=lambda x: parse_int(x) if parse_int(x) else 0):
        entities = ws_entities[ws]
        total_events = sum(int(e['event_count']) for e in entities)
        total_gobjs = sum(int(e['unique_gobjs']) for e in entities)
        print(f"\n  World State {ws}: {len(entities)} entities, {total_events:,} events, {total_gobjs} GObjs")

        # Top entities in this ws
        for e in sorted(entities, key=lambda x: int(x['event_count']), reverse=True)[:5]:
            gobjs = gobj_map.get(e['entity_ptr'], [])
            gobj_str = f"({len(gobjs)} GObjs)" if gobjs else "(0 GObjs)"
            print(f"    {e['entity_ptr']:12s} {int(e['event_count']):>8,} events {gobj_str}")

    # Memory map of entity work areas
    print("\n--- Entity Work Area Memory Map ---")
    addrs = sorted([parse_int(r['entity_ptr']) for r in ew])
    regions = defaultdict(list)
    for a in addrs:
        regions[region(a)].append(a)

    for reg in sorted(regions.keys()):
        addrs_in = regions[reg]
        print(f"\n  {reg} ({len(addrs_in)} entities):")
        for a in addrs_in:
            print(f"    0x{a:08X}")


def main():
    csv_dir = sys.argv[1] if len(sys.argv) > 1 else "research/runtime/rev108"

    print("ICO PS2 Runtime Deep Analysis — Rev.108 Session Data")
    print(f"CSV directory: {csv_dir}")
    print()

    analyze_entity_workareas(csv_dir)
    analyze_entity_gobj_binding(csv_dir)
    analyze_world_states(csv_dir)
    analyze_dl_slots(csv_dir)
    analyze_temporal(csv_dir)
    analyze_init_scene(csv_dir)
    cross_reference_all(csv_dir)

    print("\n" + "=" * 80)
    print("ANALYSIS COMPLETE")
    print("=" * 80)


if __name__ == '__main__':
    main()
