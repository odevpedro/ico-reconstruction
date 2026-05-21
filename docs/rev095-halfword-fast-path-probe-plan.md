# Rev.095 — Halfword Fast Path and Second Caller Probe Plan

## Objective

Directly validate the remaining Rev.094 halfword questions:

- whether the inferred single-cell fast path at `0x00166DFC` accounts for the high-volume `counter=1` / zero-A-B-write bucket;
- what caller-side state activates the second direct caller at `0x0016828C`;
- what final counter values are visible immediately after the main and second callers return from `0x00166BB0`.

## Updated Context

Offline review of the Rev.095 capture strengthened the split between the
dominant caller path and the rare second caller:

- the main caller at `0x0016700C` remains dominant;
- the second caller at `0x0016828C` did not appear in Rev.095;
- the `counter=1` / zero-write bucket still remains the best indirect signal
  for `0x00166DFC`;
- the observed `world_state_raw` cluster is narrow and should be used to pick
  the next live route deliberately rather than randomly.

## Problem

Rev.093b resolved the original activation problem: `0x00166BB0` is active in the hot dispatch path. Rev.094 extended that result and confirmed the second direct caller at `0x0016828C` is runtime-reachable.

The remaining unknowns are narrower:

1. The hit count at `0x00166DFC` is still inferred, not directly probed.
2. The semantic role and activation condition of `0x0016828C` are unknown.
3. `world_state_raw` is not yet tied directly to second-caller activation.

## Probe Config

### Probe 1: Fast Path (0x00166DFC)

```c
if (pc == 0x00166DFC) {
    // label: "halfword_fast_path"
    // Directly count the single-cell store path.
}
```

Registers to capture:
- `v0` = halfword value being stored
- `gp-0x4BC4` = counter at `0x00633D2C`
- `gp-0x4F80` = halfword table base at `0x006AB080`
- `a0`, `a1`, `a2`, `t0`, `ra`, `cycle`

### Probe 2: Second Caller Entry (0x0016828C)

```c
if (pc == 0x0016828C) {
    // label: "halfword_second_caller_entry"
    // Capture caller-side state before jal 0x00166BB0.
}
```

### Probe 3: Second Caller Return (0x00168294)

```c
if (pc == 0x00168294) {
    // label: "halfword_second_caller_return"
    // Capture final counter and selected context after writer returns.
}
```

### Probe 4: Main Caller Return (0x00167014)

```c
if (pc == 0x00167014) {
    // label: "halfword_exit_main"
    // Capture final counter after the dominant caller returns.
}
```

## Shared Fields to Capture

- `a0`, `a1`, `a2`, `a3`
- `v0`, `v1`
- `s0`, `s1`, `s2`
- `sp`, `ra`
- `gp-0x4BC4` (`0x00633D2C`, final halfword count)
- `gp+0x6F60` (`world_state_raw`)
- `cycle`

After the capture, run `tools/analyze_halfword_log.py` and inspect:

- `entry_callsites`
- `entry_callsite_vs_final_counter_and_writes`
- `entry_world_state_raw`
- `world_state_raw`

## Expected Results

| Scenario | Interpretation |
|----------|----------------|
| `halfword_fast_path` count approximately matches the `counter=1` / zero-A-B-write bucket | Confirms `0x00166DFC` as the single-cell path |
| `halfword_second_caller_entry` and return share specific `world_state_raw` values | Activation is state/room correlated |
| Second-caller final counters differ strongly from main-caller counters | Caller may serve a distinct dispatch/helper pass |
| Second-caller appears only with non-`0x0063....` contexts | Supports the Rev.094 possibility that it handles a different context cluster |

## Test Areas

Use the same or broader route as Rev.094 so the second caller is likely to recur.
If time is limited, prioritize the segment that produced `world_state_raw`
values `0x09`, `0x0A`, `0x11`, `0x12`, `0x13`, `0x14`, `0x15`, and `0x32`.

Preferred runtime capture order:

1. `0x00166DFC` fast-path probe.
2. `0x0016828C` entry and `0x00168294` return probes.
3. `0x00167014` main return probe.
4. `world_state_load` labels whenever the state changes.

## Conservative Verdict

Do not rename the gameplay semantics of the halfword grid from this probe alone. The next proof target is only mechanical: direct fast-path hit count and second-caller activation context.

## Session Criteria

Treat the next session as successful if any one of these happens:

- `0x00166DFC` appears directly and the captured count explains the
  `counter=1` / zero-write bucket;
- `0x0016828C` reappears together with a stable `world_state_raw` cluster;
- the main and second callers can be compared on the same session with
  separate return-address counts.

If none of these happen, keep the conclusion conservative:

- the fast path remains inferred;
- the second caller remains state-dependent;
- the route should be revised before changing any semantic labels.
