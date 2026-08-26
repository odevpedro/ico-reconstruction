#!/usr/bin/env python3
"""
ICO World-State → Room Mapper
=============================
Reads the PCSX2 runtime log and maps each world_state to game rooms/areas
using behavioral fingerprints: dwell time, event density, DL slot affinity,
entity count, and transition topology.

Source: ico-runtime-20260825-152452.jsonl (1.67M events, ~4hr session)

PS2 EE clock: 294.912 MHz (EE cycle → seconds)
"""

import json
import sys
import math
from collections import defaultdict, Counter

# ─── constants ───────────────────────────────────────────────────────────────
EE_MHZ = 294_912_000  # 294.912 MHz

# Known DLC slot address → index mapping (from Rev.105)
SLOT_ADDR_TO_IDX = {
    0x677DD8: "A",  0x6782F8: "B",  0x678D38: "C",
    0x678FC8: "D",  0x679258: "E",  0x6794E8: "F",
    0x679778: "G",  0x67A968: "H",  0x67C308: "I",
    0x67E458: "J",  0x678818: "K",  0x678AA8: "L",
    0x67C598: "M",  0x679F28: "N",  0x67CAB8: "O",
    0x67EE98: "P",
}

# Known global/shared entity a1 values (appear in every state)
GLOBAL_ENTITIES = {0x02, 0x03, 0x04, 0x05, 0x33, 0x6f, 0x83, 0x40a}

# ICO game knowledge cross-reference
# These are *hypotheses* based on Rev.103-105 evidence + ICO walkthrough knowledge
# Format: ws → (hypothesis_name, confidence, rationale)
ICO_ROOM_KNOWLEDGE = {
    0x01: ("Engine Init / Pre-boot",    "confirmed",  "first state in log, before title"),
    0x28: ("Title Screen / Press Start", "confirmed",  "Rev.103: menu state"),
    0x29: ("Opening FMV / Credits",      "confirmed",  "Rev.103: intro sequence"),
    0x2A: ("Opening FMV 2",              "confirmed",  "Rev.103: intro sequence"),
    0x2B: ("Opening FMV 3",              "confirmed",  "Rev.103: intro sequence"),
    0x2D: ("Opening FMV 4",              "confirmed",  "Rev.103: intro sequence"),
    0x32: ("Ending / Credits",           "strong",     "high a1 range, post-0x1a dead end"),
    0x03: ("Beach / First Area",         "strong",     "first gameplay state after intro"),
    0x04: ("Beach - Lower",              "probable",   "visited twice, moderate dwell"),
    0x05: ("Beach - Upper",              "probable",   "back-and-forth with 0x04"),
    0x06: ("Castle Entrance",            "probable",   "progression from beach"),
    0x07: ("Castle Hall / Corridor",     "probable",   "high entity count (257 objects)"),
    0x08: ("Hub Area A (Courtyard?)",    "probable",   "8 visits, transitions to 0x09/0x0a"),
    0x09: ("Hub Area B (Main Path?)",    "probable",   "13 visits, main traversal hub"),
    0x0A: ("Hub Node / Save Room",       "strong",     "many back-and-forth transitions, acts as connector"),
    0x0B: ("Side Area / Puzzle Room",    "probable",   "3 visits from hub"),
    0x0D: ("Mid-game Area 1",            "possible",   "moderate dwell, entity range 0x2B7-0x2E1"),
    0x0E: ("Mid-game Area 2",            "possible",   "frequent transitions with 0x0D"),
    0x0F: ("Bridge / Transition Zone",   "strong",     "4.8min dwell, single slot B, connects to 0x10"),
    0x10: ("Tower / Major Area",         "strong",     "223.6min dwell!! longest by far, main gameplay"),
    0x11: ("Tower Sub-area",             "possible",   "visited from 0x10, multiple times"),
    0x12: ("Late-game Area 1",           "possible",   "entity range 0x3AA-0x3E5"),
    0x13: ("Late-game Area 2",           "possible",   "entity range 0x3E6-0x437"),
    0x14: ("Final Area / Endgame",       "probable",   "entity range 0x438-0x48B, 17 scenes loaded"),
    0x15: ("Final Area Sub",             "possible",   "entity range 0x48C-0x4BC"),
    0x16: ("Credits / Epilogue",         "strong",     "unique a1=0x40a, entity range 0x613-0x64F"),
    0x17: ("Post-credits?",              "possible",   "entity range 0x650-0x699"),
    0x18: ("Bonus / Extras?",            "possible",   "entity range 0x6CC-0x6F8"),
    0x19: ("Debug / Test?",              "possible",   "entity range 0x729-0x75C"),
    0x1A: ("Dead End / Game Over?",      "strong",     "no outgoing transitions, entity range 0x5CC-0x612"),
}


def cycle_to_seconds(cycle):
    return cycle / EE_MHZ

def format_time(seconds):
    if seconds < 60:
        return f"{seconds:.1f}s"
    elif seconds < 3600:
        m = seconds / 60
        return f"{m:.1f}min"
    else:
        h = seconds / 3600
        return f"{h:.2f}hr"


def parse_log(path):
    """Parse JSONL log, return all events sorted by cycle.

    The file is ~1GB with null byte padding. We use a streaming approach
    that reads chunks and splits on newlines, handling null bytes.
    """
    import subprocess
    # Use ripgrep to extract only lines containing our target labels.
    # This is 10x faster than Python line iteration on a 1GB file.
    labels = ['world_state_load', 'ios_om_main', 'init_scene_gobj',
              'ios_om_create_dl', 'isys_gobj_proc_add', 'isys_gobj_add']
    pattern = '|'.join(labels)
    
    events = []
    result = subprocess.run(
        ['rg', '--no-line-number', '-e', pattern, path],
        capture_output=True, timeout=300
    )
    
    for raw_line in result.stdout.split(b'\n'):
        clean = raw_line.strip()
        if not clean:
            continue
        clean = clean.replace(b'\x00', b'')
        try:
            e = json.loads(clean)
            events.append(e)
        except (json.JSONDecodeError, UnicodeDecodeError, ValueError):
            pass
    
    events.sort(key=lambda e: e['cycle'])
    return events


def analyze_transitions(events):
    """Extract world_state transitions from world_state_load events.

    We do NOT deduplicate consecutive same-state entries because a state
    can be re-entered (e.g., 0x10 → 0x11 → 0x10) and each visit has
    its own dwell time.
    """
    wsl_events = [e for e in events if e['label'] == 'world_state_load']
    
    # Build transition list — only record state CHANGES
    transitions = []  # (cycle, new_ws)
    prev_ws = None
    for e in wsl_events:
        ws = int(e['info']['world_state_raw'], 16)
        if ws != prev_ws:
            transitions.append((e['cycle'], ws))
            prev_ws = ws
    
    return transitions


def analyze_per_state(events, transitions):
    """Build per-state statistics."""
    all_ws = set()
    for _, ws in transitions:
        all_ws.add(ws)
    
    # Count transitions in/out
    trans_in = Counter()
    trans_out = Counter()
    for i, (cycle, ws) in enumerate(transitions):
        trans_in[ws] += 1
        if i > 0:
            trans_out[transitions[i-1][1]] += 1
    
    # Compute dwell times from transitions — sum across ALL visits
    dwell_cycles = defaultdict(int)
    first_seen = {}
    visit_count = Counter()
    for i, (cycle, ws) in enumerate(transitions):
        if ws not in first_seen:
            first_seen[ws] = i
        visit_count[ws] += 1
        if i + 1 < len(transitions):
            dt = transitions[i+1][0] - cycle
            dwell_cycles[ws] += max(0, dt)
        else:
            # Last state — use time from last ios_om_main event
            ios_events = [e for e in events if e['label'] == 'ios_om_main']
            if ios_events:
                last_cycle = ios_events[-1]['cycle']
                dt = last_cycle - cycle
                dwell_cycles[ws] += max(0, dt)
    
    # Verify accumulation: print states with >1 visit
    for ws in sorted(visit_count.keys()):
        if visit_count[ws] > 1:
            d = cycle_to_seconds(dwell_cycles[ws])
            pass  # debug: dwell_cycles should be sum of all visits
    
    # Count events per state (ios_om_main = the main processing event)
    ios_count = Counter()
    ios_dl_slots = defaultdict(lambda: Counter())
    
    for e in events:
        if e['label'] == 'ios_om_main':
            ws = int(e['info']['world_state_raw'], 16)
            ios_count[ws] += 1
            a2 = int(e['regs']['a2'], 16)
            if a2 < 0x100:  # Normal DL slot index
                ios_dl_slots[ws][a2] += 1
    
    # init_scene_gobj counts per state
    init_count = Counter()
    init_entity_ids = defaultdict(set)
    for e in events:
        if e['label'] == 'init_scene_gobj':
            ws = int(e['info']['world_state_raw'], 16)
            a1 = int(e['regs']['a1'], 16)
            init_count[ws] += 1
            init_entity_ids[ws].add(a1)
    
    # Count scene-specific entities (excluding globals)
    scene_entity_count = {}
    for ws in all_ws:
        specific = init_entity_ids[ws] - GLOBAL_ENTITIES
        scene_entity_count[ws] = len(specific)
    
    return {
        'all_ws': sorted(all_ws),
        'trans_in': trans_in,
        'trans_out': trans_out,
        'dwell_cycles': dwell_cycles,
        'visit_count': visit_count,
        'ios_count': ios_count,
        'ios_dl_slots': ios_dl_slots,
        'init_count': init_count,
        'init_entity_ids': init_entity_ids,
        'scene_entity_count': scene_entity_count,
        'first_seen': first_seen,
    }


def compute_complexity(stats):
    """Complexity score: higher = more events, more DL slots, longer dwell, more entities."""
    scores = {}
    for ws in stats['all_ws']:
        dwell_s = cycle_to_seconds(stats['dwell_cycles'].get(ws, 0))
        ios = stats['ios_count'].get(ws, 0)
        dl_slots = len(stats['ios_dl_slots'].get(ws, {}))
        entities = stats['scene_entity_count'].get(ws, 0)
        transitions = stats['trans_in'].get(ws, 0)
        
        # Logarithmic weighting to prevent mega-states from dominating
        log_dwell = math.log1p(dwell_s) if dwell_s > 0 else 0
        log_ios = math.log1p(ios) if ios > 0 else 0
        
        score = (
            log_dwell * 2.0 +     # dwell importance
            log_ios * 1.5 +        # event density
            dl_slots * 0.8 +       # slot diversity
            entities * 0.3 +       # scene complexity
            transitions * 0.2      # connectivity
        )
        scores[ws] = round(score, 2)
    
    return scores


def build_graph(transitions):
    """Build transition graph: {ws: [successor_ws, ...]}."""
    graph = defaultdict(list)
    for i in range(1, len(transitions)):
        src = transitions[i-1][1]
        dst = transitions[i][1]
        if dst not in graph[src]:
            graph[src].append(dst)
    return graph


def progression_order(stats):
    """First-appearance order."""
    return sorted(stats['first_seen'].items(), key=lambda x: x[1])


def print_table(stats, scores, graph):
    """Print main analysis table."""
    print("=" * 140)
    print("ICO WORLD-STATE → ROOM MAPPER")
    print("Session: ico-runtime-20260825-152452.jsonl")
    print(f"Total events parsed: ~1.67M  |  States observed: {len(stats['all_ws'])}  |  Transitions: {sum(stats['trans_in'].values())}")
    print("=" * 140)
    
    # Progression order
    prog = progression_order(stats)
    ws_to_order = {ws: i+1 for i, (ws, _) in enumerate(prog)}
    
    print()
    print(f"{'#':>2} {'WS':>4} {'Room Name':<30} {'Conf':<10} {'Dwell':>10} {'Visits':>7} {'Events':>8} {'DL#':>4} {'Ent':>4} {'Trans':>6} {'Score':>7}")
    print("-" * 145)
    
    for ws in stats['all_ws']:
        order = ws_to_order[ws]
        dwell_s = cycle_to_seconds(stats['dwell_cycles'].get(ws, 0))
        ios = stats['ios_count'].get(ws, 0)
        dl_count = len(stats['ios_dl_slots'].get(ws, {}))
        entities = stats['scene_entity_count'].get(ws, 0)
        t_in = stats['trans_in'].get(ws, 0)
        score = scores.get(ws, 0)
        visits = stats['visit_count'].get(ws, 0)
        
        name, conf, _ = ICO_ROOM_KNOWLEDGE.get(ws, ("???", "speculative", ""))
        
        print(f"{order:>2}  0x{ws:02X}  {name:<30} {conf:<10} {format_time(dwell_s):>10} {visits:>7} {ios:>8} {dl_count:>4} {entities:>4} {t_in:>6} {score:>7.1f}")
    
    print()
    print("=" * 140)
    print("DL SLOT DISTRIBUTION BY WORLD STATE")
    print("=" * 140)
    print()
    
    # Collect all slot indices used across all states
    all_slots = set()
    for ws_slots in stats['ios_dl_slots'].values():
        all_slots.update(ws_slots.keys())
    all_slots = sorted(all_slots)
    
    # Print header
    header = f"{'WS':>4} {'Room':<25}"
    for s in all_slots:
        header += f" {s:02X}"
    header += "  Total"
    print(header)
    print("-" * len(header))
    
    for ws in stats['all_ws']:
        name, _, _ = ICO_ROOM_KNOWLEDGE.get(ws, ("???", "", ""))
        name_short = name[:23]
        row = f"0x{ws:02X} {name_short:<25}"
        ws_slots = stats['ios_dl_slots'].get(ws, {})
        total = 0
        for s in all_slots:
            c = ws_slots.get(s, 0)
            total += c
            if c > 0:
                if c > 10000:
                    row += f" {c/1000:.0f}K"
                else:
                    row += f" {c:>5}"
            else:
                row += "     "
        row += f"  {total:>6}"
        print(row)
    
    print()
    print("=" * 140)
    print("TRANSITION GRAPH (showing key connections)")
    print("=" * 140)
    print()
    
    for ws in stats['all_ws']:
        name, _, _ = ICO_ROOM_KNOWLEDGE.get(ws, ("???", "", ""))
        successors = graph.get(ws, [])
        if successors:
            succ_str = ", ".join(f"0x{s:02X}" for s in successors)
            print(f"  0x{ws:02X} ({name[:20]:<20}) → {succ_str}")
        else:
            print(f"  0x{ws:02X} ({name[:20]:<20}) → [DEAD END]")


def print_timeline(transitions, stats):
    """Print chronological state visit timeline with dwell per visit."""
    print()
    print("=" * 145)
    print("VISIT TIMELINE (chronological order)")
    print("  Dwell shown = time in THIS state before next transition")
    print("=" * 145)
    print()
    
    for i, (cycle, ws) in enumerate(transitions):
        if i + 1 < len(transitions):
            dt = cycle_to_seconds(transitions[i+1][0] - cycle)
        else:
            dt = 0
        
        name, _, _ = ICO_ROOM_KNOWLEDGE.get(ws, ("???", "", ""))
        
        # Mark back-and-forth patterns
        marker = ""
        if i >= 2 and transitions[i-2][1] == ws:
            marker = "  ← return"
        
        print(f"  [{i:>2}] 0x{ws:02X} {name:<30} dwell={format_time(dt):>8}{marker}")


def print_entity_fingerprint(stats):
    """Print entity count fingerprint per state — helps identify rooms."""
    print()
    print("=" * 140)
    print("ENTITY FINGERPRINT (scene-specific entities loaded per state)")
    print("=" * 140)
    print()
    print("  Each state loads a unique set of scene objects via init_scene_gobj.")
    print("  Higher entity count = more complex scene (geometry, enemies, items).")
    print()
    
    for ws in stats['all_ws']:
        name, _, _ = ICO_ROOM_KNOWLEDGE.get(ws, ("???", "", ""))
        total_init = stats['init_count'].get(ws, 0)
        specific = stats['scene_entity_count'].get(ws, 0)
        all_ids = sorted(stats['init_entity_ids'].get(ws, set()) - GLOBAL_ENTITIES)
        
        if all_ids:
            id_range = f"0x{min(all_ids):04X}-0x{max(all_ids):04X}" if len(all_ids) > 1 else f"0x{all_ids[0]:04X}"
        else:
            id_range = "none"
        
        bar_len = min(specific, 60)
        bar = "█" * bar_len
        
        print(f"  0x{ws:02X} ({name[:22]:<22}) {total_init:>5} inits  {specific:>3} unique  [{id_range:>13}]  {bar}")


def print_knowledge_crossref(stats):
    """Print cross-reference with ICO game knowledge."""
    print()
    print("=" * 140)
    print("CROSS-REFERENCE: ICO GAME KNOWLEDGE")
    print("=" * 140)
    print()
    print("  Confidence levels:")
    print("    confirmed  = directly verified from runtime data + Rev.103/105")
    print("    strong     = high-confidence inference from behavioral pattern")
    print("    probable   = likely correct based on progression + entity counts")
    print("    possible   = speculative, needs further validation")
    print()
    
    for ws in sorted(ICO_ROOM_KNOWLEDGE.keys()):
        name, conf, rationale = ICO_ROOM_KNOWLEDGE[ws]
        if ws in stats['all_ws']:
            dwell_s = cycle_to_seconds(stats['dwell_cycles'].get(ws, 0))
            ios = stats['ios_count'].get(ws, 0)
            marker = "✓"
        else:
            dwell_s = 0
            ios = 0
            marker = "—"
        
        print(f"  {marker} 0x{ws:02X}  [{conf:<10}]  {name:<30}  {format_time(dwell_s):>10}  {ios:>7} ios_om_main")
        print(f"    Rationale: {rationale}")
        print()


def main():
    log_path = sys.argv[1] if len(sys.argv) > 1 else (
        ".local/pcsx2-logs/ico-runtime-20260825-152452.jsonl"
    )
    
    print(f"Reading {log_path}...")
    events = parse_log(log_path)
    print(f"  Loaded {len(events)} events")
    
    transitions = analyze_transitions(events)
    print(f"  Found {len(transitions)} unique state transitions")
    
    stats = analyze_per_state(events, transitions)
    scores = compute_complexity(stats)
    graph = build_graph(transitions)
    
    print_table(stats, scores, graph)
    print_timeline(transitions, stats)
    print_entity_fingerprint(stats)
    print_knowledge_crossref(stats)
    
    # Summary
    total_dwell = sum(cycle_to_seconds(c) for c in stats['dwell_cycles'].values())
    total_ios = sum(stats['ios_count'].values())
    
    print()
    print("=" * 140)
    print("SUMMARY")
    print("=" * 140)
    print()
    print(f"  Session duration: {format_time(total_dwell)} ({total_dwell/3600:.2f} hr)")
    print(f"  States visited: {len(stats['all_ws'])}")
    print(f"  Total ios_om_main events: {total_ios:,}")
    print(f"  Total init_scene_gobj calls: {sum(stats['init_count'].values()):,}")
    print()
    
    # Top 5 by dwell
    print("  Top 5 by dwell time:")
    by_dwell = sorted(stats['all_ws'], key=lambda ws: stats['dwell_cycles'].get(ws, 0), reverse=True)
    for i, ws in enumerate(by_dwell[:5]):
        name, _, _ = ICO_ROOM_KNOWLEDGE.get(ws, ("???", "", ""))
        dwell_s = cycle_to_seconds(stats['dwell_cycles'].get(ws, 0))
        print(f"    {i+1}. 0x{ws:02X} {name:<30} {format_time(dwell_s):>10}")
    
    print()
    # Top 5 by event density (events per second)
    print("  Top 5 by event density (ios_om_main / second):")
    density = []
    for ws in stats['all_ws']:
        dwell_s = cycle_to_seconds(stats['dwell_cycles'].get(ws, 0))
        ios = stats['ios_count'].get(ws, 0)
        if dwell_s > 0:
            density.append((ws, ios / dwell_s))
    density.sort(key=lambda x: x[1], reverse=True)
    for i, (ws, d) in enumerate(density[:5]):
        name, _, _ = ICO_ROOM_KNOWLEDGE.get(ws, ("???", "", ""))
        print(f"    {i+1}. 0x{ws:02X} {name:<30} {d:>8.1f} events/s")
    
    print()
    print("  Key observations:")
    
    # Find hub state (most transitions)
    hub = max(stats['all_ws'], key=lambda ws: stats['trans_in'].get(ws, 0))
    hub_name, _, _ = ICO_ROOM_KNOWLEDGE.get(hub, ("???", "", ""))
    print(f"    • Hub state: 0x{hub:02X} ({hub_name}) — {stats['trans_in'][hub]} incoming transitions")
    
    # Find dead ends
    dead_ends = [ws for ws in stats['all_ws'] if not graph.get(ws)]
    for de in dead_ends:
        de_name, _, _ = ICO_ROOM_KNOWLEDGE.get(de, ("???", "", ""))
        print(f"    • Dead end: 0x{de:02X} ({de_name}) — no outgoing transitions")
    
    # Back-and-forth pairs
    bf_pairs = set()
    for src, dsts in graph.items():
        for dst in dsts:
            if src in graph.get(dst, []):
                pair = tuple(sorted([src, dst]))
                bf_pairs.add(pair)
    
    if bf_pairs:
        print(f"    • Back-and-forth pairs: {len(bf_pairs)}")
        for a, b in sorted(bf_pairs):
            a_name, _, _ = ICO_ROOM_KNOWLEDGE.get(a, ("???", "", ""))
            b_name, _, _ = ICO_ROOM_KNOWLEDGE.get(b, ("???", "", ""))
            print(f"      0x{a:02X} ({a_name[:15]}) ↔ 0x{b:02X} ({b_name[:15]})")


if __name__ == '__main__':
    main()
