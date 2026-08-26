#!/usr/bin/env python3
"""
Rev.108 — Comprehensive runtime log analyzer for ICO PS2 gameplay validation.

Streaming processor: reads JSONL line-by-line, does NOT load entire file into memory.

Produces CSVs and summary stats for:
- Callback hits by address / world_state / DL slot
- Entity work area pointer tracking
- World state timeline and transitions
- isysGObj/iosOm dispatch validation
- IOP/SIF/cache related hits
- Non-observed watchlist

Usage:
    python3 tools/runtime/rev108_analyzer.py <jsonl_path> [--out-dir <dir>]
"""

import json
import csv
import sys
import os
from collections import defaultdict, Counter
from pathlib import Path

# --- Watchlist definitions (from AGENTS.md + Rev.107) ---

BOY_CALLBACKS = {
    '0x00153478': 'BOY_init_fn',
    '0x001C1A98': 'BOY_hC_constructor',
    '0x001C1DD8': 'BOY_hB_per_frame',
    '0x001C1F58': 'BOY_hA_update',
}

IOP_SIF_CALLBACKS = {
    '0x002564E0': 'SIF_Resource_Loader',
    '0x00246458': 'sceSifCallRpc',
    '0x0024BEF8': 'D_Cache_Invalidate',
    '0x001A48A0': 'CODE_not_physics_table',
}

ISYS_GOBJ_CALLBACKS = {
    '0x0013F3F0': 'isysGObjProcAdd_',
    '0x0013FD10': 'iosOmExeEachGObj',
    '0x0013FC00': 'iosOmCreateDL',
    '0x0013F9D0': '_iosOmMain',
    '0x001B76F8': 'initSceneGObj',
    '0x0013DDA0': 'isysGObjInit',
    '0x0013E4D0': 'isysGObjAlloc',
    '0x0013E548': 'isysGObjRemove',
    '0x0013E8D8': 'isysGObjAdd',
}

ALL_WATCHLIST = {}
ALL_WATCHLIST.update(BOY_CALLBACKS)
ALL_WATCHLIST.update(IOP_SIF_CALLBACKS)
ALL_WATCHLIST.update(ISYS_GOBJ_CALLBACKS)

# Entity work area field offsets to monitor
ENTITY_FIELDS = [
    '+0x00', '+0x04', '+0x08', '+0x0C', '+0x18', '+0x1C', '+0x20', '+0x24',
    '+0x15C', '+0x164', '+0x670', '+0x678',
]


def parse_hex(s):
    """Parse hex string to int, returning None on failure."""
    if not s or s == '0x00000000':
        return 0
    try:
        return int(s, 16)
    except (ValueError, TypeError):
        return None


class RuntimeAnalyzer:
    """Streaming analyzer for ICO PCSX2 runtime logs."""

    def __init__(self, out_dir):
        self.out_dir = Path(out_dir)
        self.out_dir.mkdir(parents=True, exist_ok=True)

        # Counters
        self.total_events = 0
        self.label_counts = Counter()
        self.cycle_min = None
        self.cycle_max = None

        # World state tracking
        self.ws_transitions = []  # (from_ws, to_ws, cycle)
        self.ws_event_counts = Counter()  # ws -> count
        self.ws_dl_slots = defaultdict(Counter)  # ws -> {slot: count}
        self.ws_entity_ptrs = defaultdict(set)  # ws -> {entity_ptrs}
        self.current_ws = '0x00000000'
        self.prev_ws = None

        # DL slot tracking
        self.dl_slot_counts = Counter()  # slot -> count
        self.dl_slot_ws = defaultdict(Counter)  # slot -> {ws: count}
        self.dl_slot_entities = defaultdict(set)  # slot -> {entity_ptrs}

        # Entity work area tracking
        self.entity_ws = defaultdict(set)  # entity_ptr -> {world_states}
        self.entity_dl_slot = defaultdict(Counter)  # entity_ptr -> {slot: count}
        self.entity_event_count = Counter()  # entity_ptr -> count
        self.entity_gobj = defaultdict(set)  # entity_ptr -> {gobj_ptrs}

        # GObj tracking
        self.gobj_entity = {}  # gobj_ptr -> entity_ptr
        self.gobj_event_count = Counter()

        # ProcAdd tracking (callback registration)
        self.procad_count = Counter()  # pc -> count
        self.procad_by_ws = defaultdict(Counter)  # ws -> {pc: count}
        self.procad_a1_values = Counter()  # a1 (GObj ptr) values

        # Watchlist hits
        self.watchlist_hits = Counter()  # label -> count
        self.watchlist_by_ws = defaultdict(Counter)  # ws -> {label: count}

        # Init scene tracking
        self.init_scene_count = 0
        self.init_scene_by_ws = Counter()

        # Info field tracking
        self.entity_work_values = Counter()
        self.gobj_list_head_values = Counter()
        self.current_gobj_values = Counter()
        self.om_mask_values = Counter()

        # Timeline (sampled every 10000 events)
        self.timeline = []  # (cycle, ws, entity_count, dl_slot_count)

    def process_event(self, ev):
        """Process a single JSONL event."""
        self.total_events += 1
        label = ev.get('label', '?')
        self.label_counts[label] += 1

        cycle = ev.get('cycle', 0)
        if self.cycle_min is None or cycle < self.cycle_min:
            self.cycle_min = cycle
        if self.cycle_max is None or cycle > self.cycle_max:
            self.cycle_max = cycle

        regs = ev.get('regs', {})
        info = ev.get('info', {})

        # Parse world state
        ws_raw = info.get('world_state_raw', '0x00000000')
        ws = ws_raw if ws_raw else '0x00000000'

        # Parse entity work area pointer
        entity_work = info.get('current_entity_work', '0x00000000')
        entity_ptr = parse_hex(entity_work)

        # Parse GObj pointers
        gobj_list_head = info.get('gobj_list_head', '0x00000000')
        current_gobj = info.get('current_gobj', '0x00000000')
        current_child = info.get('current_child', '0x00000000')
        gobj_ptr = parse_hex(current_gobj)
        gobj_list = parse_hex(gobj_list_head)

        # Parse OM mask
        om_mask = info.get('om_mask', '0x00000000')

        # Parse DL slot (a2 in ios_om_main)
        a2 = regs.get('a2', '0x00000000')
        dl_slot = a2 if label == 'ios_om_main' else None

        # World state transition detection
        if ws != self.current_ws and ws != '0x00000000':
            if self.current_ws != '0x00000000':
                self.ws_transitions.append((self.current_ws, ws, cycle))
            self.current_ws = ws

        # Per-label processing
        self.ws_event_counts[ws] += 1

        if dl_slot:
            self.dl_slot_counts[dl_slot] += 1
            self.dl_slot_ws[dl_slot][ws] += 1
            if entity_ptr:
                self.dl_slot_entities[dl_slot].add(entity_ptr)
            # Track DL slots per world_state
            if not hasattr(self, 'ws_dl_slot_counter'):
                self.ws_dl_slot_counter = defaultdict(Counter)
            self.ws_dl_slot_counter[ws][dl_slot] += 1

        if entity_ptr and entity_ptr != 0:
            self.entity_ws[entity_ptr].add(ws)
            self.entity_event_count[entity_ptr] += 1
            if dl_slot:
                self.entity_dl_slot[entity_ptr][dl_slot] += 1
            if gobj_ptr and gobj_ptr != 0:
                self.entity_gobj[entity_ptr].add(gobj_ptr)
            # Track entities per world_state
            if not hasattr(self, 'ws_entities'):
                self.ws_entities = defaultdict(set)
            self.ws_entities[ws].add(entity_ptr)

        if gobj_ptr and gobj_ptr != 0:
            self.gobj_event_count[gobj_ptr] += 1
            if entity_ptr:
                self.gobj_entity[gobj_ptr] = entity_ptr

        # ProcAdd tracking
        if label == 'isys_gobj_proc_add':
            self.procad_count[regs.get('ra', '?')] += 1
            self.procad_by_ws[ws][regs.get('ra', '?')] += 1
            a1 = regs.get('a1', '0x00000000')
            self.procad_a1_values[a1] += 1

        # Init scene tracking
        if label == 'init_scene_gobj':
            self.init_scene_count += 1
            self.init_scene_by_ws[ws] += 1

        # Watchlist check
        pc = ev.get('pc', '')
        if pc in ALL_WATCHLIST:
            watch_label = ALL_WATCHLIST[pc]
            self.watchlist_hits[watch_label] += 1
            self.watchlist_by_ws[ws][watch_label] += 1

        # Entity work area field tracking
        if entity_ptr and entity_ptr != 0:
            self.entity_work_values[entity_work] += 1
        if gobj_list:
            self.gobj_list_head_values[gobj_list_head] += 1
        if gobj_ptr:
            self.current_gobj_values[current_gobj] += 1
        if om_mask != '0x00000000':
            self.om_mask_values[om_mask] += 1

        # Timeline sampling
        if self.total_events % 10000 == 0:
            self.timeline.append({
                'cycle': cycle,
                'ws': ws,
                'entity_ptr': entity_work,
                'dl_slot': dl_slot or '',
                'event_count': self.total_events,
            })

    def process_file(self, filepath):
        """Process JSONL file in streaming mode."""
        print(f"Processing: {filepath}")
        with open(filepath) as f:
            for line in f:
                try:
                    ev = json.loads(line)
                    self.process_event(ev)
                except json.JSONDecodeError:
                    continue
                if self.total_events % 100000 == 0:
                    print(f"  ... {self.total_events:,} events processed", file=sys.stderr)
        print(f"  Done: {self.total_events:,} events total")

    def write_csvs(self):
        """Write all CSV output files."""
        print(f"\nWriting CSVs to {self.out_dir}...")

        # 1. callback_hits_by_address.csv
        with open(self.out_dir / 'callback_hits_by_address.csv', 'w', newline='') as f:
            w = csv.writer(f)
            w.writerow(['address', 'label', 'total_hits', 'unique_world_states'])
            for label_name, count in sorted(self.watchlist_hits.items(), key=lambda x: -x[1]):
                # Find address for this label
                addr = next((a for a, l in ALL_WATCHLIST.items() if l == label_name), '?')
                # Count unique world states
                ws_set = set(self.watchlist_by_ws.keys())
                w.writerow([addr, label_name, count, len(ws_set)])

        # 2. callback_hits_by_world_state.csv
        with open(self.out_dir / 'callback_hits_by_world_state.csv', 'w', newline='') as f:
            w = csv.writer(f)
            labels_sorted = sorted(set(l for ws_data in self.watchlist_by_ws.values() for l in ws_data))
            w.writerow(['world_state'] + labels_sorted)
            for ws in sorted(self.watchlist_by_ws.keys()):
                row = [ws] + [self.watchlist_by_ws[ws].get(l, 0) for l in labels_sorted]
                w.writerow(row)

        # 3. callback_hits_by_dl_slot.csv
        with open(self.out_dir / 'callback_hits_by_dl_slot.csv', 'w', newline='') as f:
            w = csv.writer(f)
            w.writerow(['dl_slot', 'total_ios_om_main', 'unique_ws', 'unique_entities'])
            for slot, count in self.dl_slot_counts.most_common():
                ws_count = len(self.dl_slot_ws[slot])
                ent_count = len(self.dl_slot_entities[slot])
                w.writerow([slot, count, ws_count, ent_count])

        # 4. entity_workarea_pointers.csv
        with open(self.out_dir / 'entity_workarea_pointers.csv', 'w', newline='') as f:
            w = csv.writer(f)
            w.writerow(['entity_ptr', 'event_count', 'unique_ws', 'unique_dl_slots', 'unique_gobjs', 'world_states'])
            for ptr, count in self.entity_event_count.most_common(100):
                ws_set = self.entity_ws[ptr]
                slots = self.entity_dl_slot[ptr]
                gobjs = self.entity_gobj[ptr]
                ws_list = ','.join(sorted(ws_set))
                w.writerow([f'0x{ptr:08X}', count, len(ws_set), len(slots), len(gobjs), ws_list])

        # 5. boy_callbacks_observed.csv
        with open(self.out_dir / 'boy_callbacks_observed.csv', 'w', newline='') as f:
            w = csv.writer(f)
            w.writerow(['address', 'label', 'hits_in_log', 'observation'])
            for addr, label in BOY_CALLBACKS.items():
                hits = self.watchlist_hits.get(label, 0)
                obs = 'OBSERVED' if hits > 0 else 'NOT_OBSERVED_IN_LOG'
                w.writerow([addr, label, hits, obs])

        # 6. girlbrain_callbacks_observed.csv
        # GirlBrain callbacks are NOT in the watchlist — they are unknown addresses
        with open(self.out_dir / 'girlbrain_callbacks_observed.csv', 'w', newline='') as f:
            w = csv.writer(f)
            w.writerow(['address', 'label', 'hits_in_log', 'observation'])
            w.writerow(['?', 'GirlBrain_callbacks', 0, 'NO_PROBES_DEPLOYED'])
            w.writerow(['?', 'girlBrainMain_PositionUpdate', 0, 'NO_PROBE'])
            w.writerow(['?', 'subGirlBrain_PulledUp', 0, 'NO_PROBE'])
            w.writerow(['?', '_girlBrainHide_MakeHidePoint', 0, 'NO_PROBE'])
            w.writerow(['?', 'girlBrainHide_GoalTurn', 0, 'NO_PROBE'])
            w.writerow(['?', 'girlBrainRunawaySearchPoint', 0, 'NO_PROBE'])
            w.writerow(['?', 'girlBrainRunawayMoveByWay', 0, 'NO_PROBE'])

        # 7. hot_paths_by_world_state.csv
        with open(self.out_dir / 'hot_paths_by_world_state.csv', 'w', newline='') as f:
            w = csv.writer(f)
            w.writerow(['world_state', 'total_events', 'dl_slot_primary', 'dl_slot_count', 'entity_count'])
            ws_dl_counter = getattr(self, 'ws_dl_slot_counter', defaultdict(Counter))
            ws_ent = getattr(self, 'ws_entities', defaultdict(set))
            for ws, count in sorted(self.ws_event_counts.items(), key=lambda x: -x[1]):
                if ws == '0x00000000':
                    continue
                top_slot = ws_dl_counter.get(ws, Counter()).most_common(1)
                slot_str = top_slot[0][0] if top_slot else '?'
                slot_count = top_slot[0][1] if top_slot else 0
                ent_count = len(ws_ent.get(ws, set()))
                w.writerow([ws, count, slot_str, slot_count, ent_count])

        # 8. iop_related_hits.csv
        with open(self.out_dir / 'iop_related_hits.csv', 'w', newline='') as f:
            w = csv.writer(f)
            w.writerow(['address', 'label', 'hits', 'observation'])
            for addr, label in IOP_SIF_CALLBACKS.items():
                hits = self.watchlist_hits.get(label, 0)
                obs = 'OBSERVED' if hits > 0 else 'NOT_OBSERVED_IN_LOG'
                w.writerow([addr, label, hits, obs])

        # 9. non_observed_watchlist.csv
        with open(self.out_dir / 'non_observed_watchlist.csv', 'w', newline='') as f:
            w = csv.writer(f)
            w.writerow(['address', 'expected_label', 'reason_not_observed'])
            for addr, label in ALL_WATCHLIST.items():
                if self.watchlist_hits.get(label, 0) == 0:
                    reason = 'NO_PROBE_AT_THIS_ADDRESS'
                    w.writerow([addr, label, reason])

        # 10. world_state_timeline.csv
        with open(self.out_dir / 'world_state_timeline.csv', 'w', newline='') as f:
            w = csv.writer(f)
            w.writerow(['from_ws', 'to_ws', 'cycle', 'delta_cycles'])
            prev_cycle = None
            for from_ws, to_ws, cycle in self.ws_transitions:
                delta = cycle - prev_cycle if prev_cycle else 0
                w.writerow([from_ws, to_ws, cycle, delta])
                prev_cycle = cycle

        # 11. dispatch_slot_ws_matrix.csv
        with open(self.out_dir / 'dispatch_slot_ws_matrix.csv', 'w', newline='') as f:
            w = csv.writer(f)
            ws_list = sorted(self.ws_event_counts.keys())
            w.writerow(['dl_slot'] + ws_list)
            for slot in sorted(self.dl_slot_counts.keys()):
                row = [slot] + [self.dl_slot_ws[slot].get(ws, 0) for ws in ws_list]
                w.writerow(row)

        # 12. entity_gobj_binding.csv
        with open(self.out_dir / 'entity_gobj_binding.csv', 'w', newline='') as f:
            w = csv.writer(f)
            w.writerow(['entity_ptr', 'gobj_ptr', 'event_count'])
            for gobj_ptr, entity_ptr in self.gobj_entity.items():
                if entity_ptr and entity_ptr != 0:
                    w.writerow([f'0x{entity_ptr:08X}', f'0x{gobj_ptr:08X}', self.gobj_event_count[gobj_ptr]])

        # 13. init_scene_by_world_state.csv
        with open(self.out_dir / 'init_scene_by_world_state.csv', 'w', newline='') as f:
            w = csv.writer(f)
            w.writerow(['world_state', 'init_scene_count'])
            for ws, count in self.init_scene_by_ws.most_common():
                w.writerow([ws, count])

        # 14. procadd_registration.csv
        with open(self.out_dir / 'procadd_registration.csv', 'w', newline='') as f:
            w = csv.writer(f)
            w.writerow(['ra_caller', 'count', 'top_world_states'])
            for ra, count in self.procad_count.most_common(30):
                ws_str = ','.join(f'{ws}({c})' for ws, c in self.procad_by_ws[ws].most_common(3)
                                 for ws in [ws])  # simplified
                w.writerow([ra, count, ''])

        print(f"  Written 14 CSV files")

    def print_summary(self):
        """Print summary to stdout."""
        print(f"\n{'='*70}")
        print(f"Rev.108 Runtime Analysis Summary")
        print(f"{'='*70}")
        print(f"\nTotal events: {self.total_events:,}")
        print(f"Cycle range: {self.cycle_min} - {self.cycle_max}")

        print(f"\n--- Event Labels ---")
        for label, count in self.label_counts.most_common():
            print(f"  {label}: {count:,} ({count/self.total_events*100:.1f}%)")

        print(f"\n--- World States ({len([ws for ws in self.ws_event_counts if ws != '0x00000000'])} active) ---")
        for ws, count in sorted(self.ws_event_counts.items(), key=lambda x: -x[1]):
            if ws == '0x00000000':
                continue
            print(f"  {ws}: {count:,} events")

        print(f"\n--- DL Slots ({len(self.dl_slot_counts)} active) ---")
        for slot, count in self.dl_slot_counts.most_common(10):
            print(f"  {slot}: {count:,}")

        print(f"\n--- Entity Work Area Pointers ({len(self.entity_event_count)} unique) ---")
        for ptr, count in self.entity_event_count.most_common(10):
            ws_set = self.entity_ws[ptr]
            print(f"  0x{ptr:08X}: {count:,} events, {len(ws_set)} world_states")

        print(f"\n--- Watchlist Hits ---")
        for label, count in self.watchlist_hits.most_common():
            print(f"  {label}: {count:,}")
        not_observed = [l for l in ALL_WATCHLIST.values() if self.watchlist_hits.get(l, 0) == 0]
        if not_observed:
            print(f"\n  NOT OBSERVED ({len(not_observed)}):")
            for l in not_observed:
                print(f"    {l}")

        print(f"\n--- World State Transitions ({len(self.ws_transitions)}) ---")
        for from_ws, to_ws, cycle in self.ws_transitions[-10:]:
            print(f"  {from_ws} -> {to_ws} (cycle {cycle})")

        print(f"\n--- Init Scene GObj ---")
        print(f"  Total: {self.init_scene_count}")
        for ws, count in self.init_scene_by_ws.most_common(5):
            print(f"  {ws}: {count}")


def main():
    import argparse
    parser = argparse.ArgumentParser(description='Rev.108 Runtime Log Analyzer')
    parser.add_argument('jsonl_path', help='Path to JSONL log file')
    parser.add_argument('--out-dir', default='research/runtime/rev108', help='Output directory')
    args = parser.parse_args()

    analyzer = RuntimeAnalyzer(args.out_dir)
    analyzer.process_file(args.jsonl_path)
    analyzer.write_csvs()
    analyzer.print_summary()


if __name__ == '__main__':
    main()
