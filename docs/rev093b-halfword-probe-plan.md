# Rev.093b — Halfword Writer Probe Plan

## Objective

Determine why the halfword spatial hash table writers at `0x166D1C`/`0x166D78` never fire in gameplay. The previous probes were placed at the `SH` instruction (halfword store). The new probe must be at the **function entry** `0x166BB0` to detect early-exit conditions.

## Problem

Zero hits in 67.3M events across 4 game areas (entrance, windmill, 3rd area, death). All 14 callbacks read the BSS-zeroed counter (stays 0 → loop skipped). The function at `0x166C80` may:

1. Have an early-exit guard that prevents reaching the SH instructions
2. Run during scene loading (not per-frame)
3. Require a specific entity bounding box not present in tested rooms
4. Be a disconnected/dead code path

## Probe Config

### Probe 1: Function Entry (0x166BB0)

```c
// PCSX2 IcoProbeMaybeLog addition:
if (pc == 0x00166BB0) {  // function entry
    // Capture: counter value, a0 (context), any early-exit decision
    // label: "halfword_entry"
}
```

Registers to capture:
- `a0` = context pointer (first arg)
- `v0` = return value / early-exit flag
- `gp-0x4BC4` = counter at `0x00633D2C` (read before function starts)
- `cycle` = CPU cycle count

### Probe 2: Write Point A (0x166D1C)

```c
if (pc == 0x00166D1C) {  // halfword write A
    // label: "halfword_write_A"
    // Capture: a2 (row), t0 (col), counter value
}
```

### Probe 3: Write Point B (0x166D78)

```c
if (pc == 0x00166D78) {  // halfword write B
    // label: "halfword_write_B"
    // Capture: a2 (row), t0 (col), counter value
}
```

## Expected Results

| Scenario | Probe 1 | Probe 2/3 | Interpretation |
|----------|---------|-----------|----------------|
| Entry hit, writes hit | >0 | >0 | Per-frame spatial rebuild (late-game room) |
| Entry hit, writes zero | >0 | 0 | Early-exit: condition not met in this frame |
| Entry zero, writes zero | 0 | 0 | Function never reached (scene-load only or dead code) |

## Test Areas (in order)

1. **Castle interior** (throne room / main hall) — most complex geometry
2. **Water channel / sluice** (room 23) — water physics may trigger rebuild
3. **Symmetry room** (room 18/27) — mirror effect may need spatial hash
4. **Crest rooms** (19-21, 28-30) — puzzle rooms with many entities

## If Entry Is Hit But Writes Are Not

If Probe 1 fires but Probes 2/3 don't, the function has an early-exit condition. Deploy additional probes at key branch points:
- `0x166BF0` (guard check before loop)
- `0x166C24` (loop entry)

## If Entry Is Never Hit

If Probe 1 never fires in any late-game area, the function at `0x166C80`/`0x166BB0` may be:
- Scene-load only (called before per-frame dispatch starts)
- Dead code (disconnected from call graph)
- Triggered by a specific world_state or mode flag not set in tested rooms

Next step: add a scene-load probe at `0x1AF948` (world_state_load) alongside the halfword entry probe to see if they correlate.
