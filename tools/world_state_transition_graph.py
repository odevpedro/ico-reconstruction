#!/usr/bin/env python3
"""
Build a complete world_state transition graph from a PCSX2 runtime JSONL log.

Reads all events, extracts world_state_load events, and builds:
- Transition counts between consecutive world_states
- Cycle deltas (dwell time per state)
- Hub/leaf/cluster analysis
- ASCII transition graph diagram
"""

import json
import sys
from collections import defaultdict

CYCLES_PER_SECOND = 294912000  # EE clock ~294.9 MHz


def load_world_state_events(log_path):
    """Extract world_state_load events with cycle and world_state_raw."""
    events = []
    with open(log_path) as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            try:
                ev = json.loads(line)
            except json.JSONDecodeError:
                continue
            if ev.get("label") != "world_state_load":
                continue
            ws = ev["info"]["world_state_raw"]
            cycle = ev.get("cycle", 0)
            ws_int = int(ws, 16)
            events.append({"ws": ws_int, "cycle": cycle, "ws_hex": ws})
    return events


def dedup_same_cycle(events):
    """If multiple world_state_load events share the same cycle, keep only the first."""
    deduped = []
    last_cycle = None
    for ev in events:
        if ev["cycle"] == last_cycle:
            continue
        deduped.append(ev)
        last_cycle = ev["cycle"]
    return deduped


def build_transitions(events):
    """Build transition counts and cycle deltas from ordered world_state events.

    Dwell time = time spent IN a state = cycle_delta from entry to exit.
    A state's dwell is the sum of all cycle gaps until the next *different* state.
    Detects and skips cycle counter resets (game reboots).
    """
    transitions = defaultdict(lambda: {"count": 0, "deltas": []})
    state_dwell = defaultdict(int)  # ws -> total cycles spent in this state

    for i in range(len(events) - 1):
        cur = events[i]
        nxt = events[i + 1]
        src = cur["ws"]
        dst = nxt["ws"]
        delta = nxt["cycle"] - cur["cycle"]

        # Detect cycle counter reset (negative delta or huge backward jump)
        if delta < 0:
            # Session boundary / reboot — skip this transition entirely
            continue

        if src != dst:
            transitions[(src, dst)]["count"] += 1
            transitions[(src, dst)]["deltas"].append(delta)

        state_dwell[src] += delta

    return transitions, state_dwell


def cycle_to_seconds(cycles):
    return cycles / CYCLES_PER_SECOND


def format_duration(cycles):
    secs = cycle_to_seconds(cycles)
    if secs < 1:
        return f"{secs*1000:.0f}ms"
    elif secs < 60:
        return f"{secs:.1f}s"
    elif secs < 3600:
        return f"{secs/60:.1f}min"
    else:
        return f"{secs/3600:.2f}hr"


def classify_states(transitions, all_states):
    """Classify states as hubs, leaves, or middle based on in/out degree."""
    in_degree = defaultdict(int)
    out_degree = defaultdict(int)
    in_count = defaultdict(int)
    out_count = defaultdict(int)

    for (src, dst), info in transitions.items():
        out_degree[src] += 1
        in_degree[dst] += 1
        out_count[src] += info["count"]
        in_count[dst] += info["count"]

    results = {}
    for ws in all_states:
        ind = in_degree.get(ws, 0)
        outd = out_degree.get(ws, 0)
        inc = in_count.get(ws, 0)
        outc = out_count.get(ws, 0)

        if ind == 0 and outd == 0:
            role = "ORPHAN"
        elif ind == 0:
            role = "ENTRY"
        elif outd == 0:
            role = "DEAD-END"
        elif inc + outc > 40:
            role = "HUB"
        elif ind == 1 and outd == 1:
            role = "LINEAR"
        else:
            role = "MIDDLE"

        results[ws] = {
            "role": role,
            "in_degree": ind,
            "out_degree": outd,
            "in_count": inc,
            "out_count": outc,
        }
    return results


def build_ascii_graph(transitions, all_states, dwell, classification, events):
    """Build an ASCII representation of the transition graph."""
    lines = []
    lines.append("=" * 78)
    lines.append("  WORLD_STATE TRANSITION GRAPH — ICO Runtime (PCSX2)")
    lines.append("=" * 78)
    lines.append("")

    # Sort transitions by count descending
    sorted_trans = sorted(transitions.items(), key=lambda x: -x[1]["count"])

    # --- Main path ---
    lines.append("─── ALL TRANSITIONS (sorted by frequency) ───")
    lines.append("")
    lines.append(f"  {'FROM':>8s}  ──▶  {'TO':>8s}  {'COUNT':>6s}  {'AVG DWELL':>12s}  {'VISUAL':s}")
    lines.append(f"  {'─'*8:s}       {'─'*8:s}  {'─'*6:s}  {'─'*12:s}  {'─'*30:s}")

    max_count = max(t["count"] for t in transitions.values()) if transitions else 1

    for (src, dst), info in sorted_trans:
        avg_delta = sum(info["deltas"]) / len(info["deltas"]) if info["deltas"] else 0
        bar_len = int(30 * info["count"] / max_count)
        bar = "█" * bar_len
        avg_str = format_duration(avg_delta)
        lines.append(
            f"  0x{src:02x}       →  0x{dst:02x}     {info['count']:>5d}   {avg_str:>12s}  {bar}"
        )
    lines.append("")

    # --- Dwell time per state ---
    lines.append("─── DWELL TIME PER STATE ───")
    lines.append("")
    lines.append(f"  {'STATE':>8s}  {'OCCURRENCES':>12s}  {'TOTAL DWELL':>14s}  {'ROLE':>10s}")
    lines.append(f"  {'─'*8:s}  {'─'*12:s}  {'─'*14:s}  {'─'*10:s}")

    # Count occurrences of each state in the event list
    ws_occurrences = defaultdict(int)
    for ev in events:
        ws_occurrences[ev["ws"]] += 1

    total_cycle = events[-1]["cycle"] - events[0]["cycle"]

    for ws in sorted(all_states):
        occ = ws_occurrences.get(ws, 0)
        total_dwell_cycles = dwell.get(ws, 0)
        role = classification[ws]["role"]
        total_str = format_duration(total_dwell_cycles) if total_dwell_cycles > 0 else "—"
        pct = (total_dwell_cycles / total_cycle * 100) if total_cycle > 0 and total_dwell_cycles > 0 else 0
        lines.append(
            f"  0x{ws:02x}       {occ:>11d}  {total_str:>14s}  {role:>10s}  ({pct:.1f}%)"
        )
    lines.append("")

    # --- Classification summary ---
    lines.append("─── STATE CLASSIFICATION ───")
    lines.append("")

    hubs = [ws for ws in all_states if classification[ws]["role"] == "HUB"]
    entries = [ws for ws in all_states if classification[ws]["role"] == "ENTRY"]
    dead_ends = [ws for ws in all_states if classification[ws]["role"] == "DEAD-END"]
    orphans = [ws for ws in all_states if classification[ws]["role"] == "ORPHAN"]
    linears = [ws for ws in all_states if classification[ws]["role"] == "LINEAR"]
    middles = [ws for ws in all_states if classification[ws]["role"] == "MIDDLE"]

    if entries:
        lines.append(f"  ENTRY POINTS (no incoming transitions):")
        for ws in entries:
            c = classification[ws]
            lines.append(f"    0x{ws:02x}  →  outgoing: {c['out_degree']} unique targets, {c['out_count']} total transitions")
        lines.append("")

    if dead_ends:
        lines.append(f"  DEAD ENDS (no outgoing transitions):")
        for ws in dead_ends:
            c = classification[ws]
            lines.append(f"    0x{ws:02x}  ←  incoming: {c['in_degree']} unique sources, {c['in_count']} total transitions")
        lines.append("")

    if orphans:
        lines.append(f"  ORPHANS (no transitions at all):")
        for ws in orphans:
            lines.append(f"    0x{ws:02x}")
        lines.append("")

    if hubs:
        lines.append(f"  HUBS (high-frequency nodes):")
        for ws in hubs:
            c = classification[ws]
            lines.append(
                f"    0x{ws:02x}  ← {c['in_count']:>5d} in/{c['out_count']:>5d} out  "
                f"({c['in_degree']} src / {c['out_degree']} dst)"
            )
        lines.append("")

    if linears:
        lines.append(f"  LINEAR (1-in, 1-out — corridor states):")
        for ws in linears:
            c = classification[ws]
            lines.append(
                f"    0x{ws:02x}  ← {c['in_count']:>5d} in / {c['out_count']:>5d} out"
            )
        lines.append("")

    if middles:
        lines.append(f"  MIDDLE (mixed in/out):")
        for ws in middles:
            c = classification[ws]
            lines.append(
                f"    0x{ws:02x}  ← {c['in_count']:>5d} in/{c['out_count']:>5d} out  "
                f"({c['in_degree']} src / {c['out_degree']} dst)"
            )
        lines.append("")

    # --- ASCII diagram ---
    lines.append("─── ASCII TRANSITION DIAGRAM ───")
    lines.append("")
    lines.append("  (thick = high frequency, thin = low frequency)")
    lines.append("")

    # Build adjacency list for diagram
    adj = defaultdict(list)
    for (src, dst), info in sorted_trans:
        adj[src].append((dst, info["count"]))

    # Find clusters by connected components
    visited = set()
    components = []

    def bfs(start):
        comp = set()
        queue = [start]
        while queue:
            n = queue.pop(0)
            if n in comp:
                continue
            comp.add(n)
            for dst, _ in adj.get(n, []):
                if dst not in comp:
                    queue.append(dst)
            # Also follow reverse edges
            for (s, d), _ in transitions.items():
                if d == n and s not in comp:
                    queue.append(s)
        return comp

    for ws in sorted(all_states):
        if ws not in visited:
            comp = bfs(ws)
            visited |= comp
            components.append(sorted(comp))

    for ci, comp in enumerate(components):
        if len(components) > 1:
            lines.append(f"  ┌─── Cluster {ci + 1} ───┐")
        for ws in comp:
            c = classification[ws]
            occ = ws_occurrences.get(ws, 0)
            total_dwell_cycles = dwell.get(ws, 0)
            dwell_str = format_duration(total_dwell_cycles) if total_dwell_cycles > 0 else "—"
            role_sym = {"HUB": "◆", "ENTRY": "▶", "DEAD-END": "■", "ORPHAN": "○",
                        "LINEAR": "●", "MIDDLE": "◇"}.get(c["role"], "?")
            lines.append(f"  │ {role_sym} 0x{ws:02x} [{c['role']:>8s}] occ={occ:>4d} dwell={dwell_str}")

            out_edges = adj.get(ws, [])
            if out_edges:
                for dst, cnt in out_edges:
                    thickness = "━" if cnt > 10 else ("─" if cnt > 2 else "·")
                    arrow = "▶" if cnt > 10 else ("→" if cnt > 2 else "›")
                    lines.append(f"  │   {thickness}{thickness}{thickness}{thickness}{arrow} 0x{dst:02x} ({cnt}x)")
            else:
                lines.append(f"  │   ■ (dead end)")
        if len(components) > 1:
            lines.append(f"  └{'─' * 30}┘")
        lines.append("")

    # --- Linearized main path ---
    lines.append("─── MAIN PATH (linearized) ───")
    lines.append("")

    # Trace the main path by following highest-frequency transitions
    start_state = entries[0] if entries else events[0]["ws"]
    path = [start_state]
    path_set = {start_state}
    max_steps = len(all_states) * 2

    current = start_state
    for _ in range(max_steps):
        out = adj.get(current, [])
        if not out:
            break
        # Pick highest-frequency unvisited target
        best = max(out, key=lambda x: x[1])
        if best[0] in path_set and best[0] != start_state:
            # Try next best
            candidates = [o for o in out if o[0] not in path_set]
            if not candidates:
                break
            best = max(candidates, key=lambda x: x[1])
        path.append(best[0])
        path_set.add(best[0])
        current = best[0]

    path_str = " → ".join(f"0x{s:02x}" for s in path)
    lines.append(f"  {path_str}")
    lines.append("")

    # --- Clusters by transition frequency ---
    lines.append("─── CLUSTERS (strongly connected via high-frequency transitions) ───")
    lines.append("")

    # Find bidirectional pairs (A↔B both have transitions)
    bidir = []
    seen_pairs = set()
    for (a, b), info in transitions.items():
        rev = transitions.get((b, a))
        if rev and (min(a, b), max(a, b)) not in seen_pairs:
            seen_pairs.add((min(a, b), max(a, b)))
            bidir.append((a, b, info["count"], rev["count"]))

    if bidir:
        lines.append("  Bidirectional loops (A ↔ B):")
        for a, b, cnt_ab, cnt_ba in sorted(bidir, key=lambda x: -(x[2] + x[3])):
            lines.append(f"    0x{a:02x} ↔ 0x{b:02x}  ({cnt_ab} forward, {cnt_ba} backward)")
        lines.append("")

    return "\n".join(lines)


if __name__ == "__main__":
    if len(sys.argv) < 2:
        log_path = "/home/hoper/Documentos/repos/ico-reconstruction/.local/pcsx2-logs/ico-runtime-20260825-152452.jsonl"
    else:
        log_path = sys.argv[1]

    print(f"Reading log: {log_path}")
    raw_events = load_world_state_events(log_path)
    print(f"  Raw world_state_load events: {len(raw_events)}")

    events = dedup_same_cycle(raw_events)
    print(f"  After dedup same-cycle: {len(events)}")

    # Detect session boundaries (cycle counter resets)
    sessions = []
    session_start = 0
    for i in range(1, len(events)):
        if events[i]["cycle"] < events[i - 1]["cycle"]:
            sessions.append((session_start, i))
            session_start = i
    sessions.append((session_start, len(events)))
    print(f"  Session boundaries: {len(sessions)} segments (reboots: {len(sessions) - 1})")
    for si, (start, end) in enumerate(sessions):
        first_cycle = events[start]["cycle"]
        last_cycle = events[end - 1]["cycle"]
        dur = last_cycle - first_cycle
        print(f"    Segment {si + 1}: events {start}-{end-1}, "
              f"{end-start} loads, {format_duration(dur)}")
    print()

    all_states = sorted(set(ev["ws"] for ev in events))
    print(f"  Unique world_states: {len(all_states)}")
    print(f"  States: {', '.join(f'0x{s:02x}' for s in all_states)}")
    print()

    transitions, dwell = build_transitions(events)

    print(f"  Unique transitions: {len(transitions)}")
    print()

    classification = classify_states(transitions, all_states)
    output = build_ascii_graph(transitions, all_states, dwell, classification, events)
    print(output)
