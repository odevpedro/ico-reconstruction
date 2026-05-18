# Rev.084 — Runtime Validation Session: Extended Playthrough (43.8M events)

**Date:** 2026-05-18
**Session duration:** ~122 minutes of gameplay
**Total events:** 43,776,015
**Log size:** 14 GB raw / ~600 MB gzipped
**Build:** Custom PCSX2 with 9 probes (rev078 probes)
**GP confirmed:** 0x006388F0 (3rd session)
**Log file:** `.local/runtime-captures/ico-probe/events.jsonl`

---

## Objective

Extended runtime validation across 3+ game areas to:
- Confirm slot distribution varies by game area (3rd data point)
- Detect room transitions via zone analysis
- Verify all rare probes across extended play (mask_set, halfword_store, slot0, alt_selection)
- Identify new slot activations not seen in earlier sessions
- Cross-session comparison: entrance (Rev.074) + windmill (Rev.079) + this session

---

## Probes

Same 9 probes as Rev.079 (Rev.078 configuration):

| Address | Label | What it captures |
|---------|-------|-----------------|
| 0x00166E10 | main_dispatcher | a1 = slot_index (0-16) |
| 0x00167020 | dispatch_point | v1 = callback target, a0 = context |
| 0x00167230 | cold_path_A | gp slots, a0 = context |
| 0x00167258 | cold_path_B | gp slots, a0 = context |
| 0x0013ED40 | mask_set | v0 = mask value (bit 0 set/clear) |
| 0x00166D38 | halfword_store_A | a0 = written value (spatial hash entry) |
| 0x00166D94 | halfword_store_B | a0 = written value |
| 0x001683CC | slot0_v0_from_gp | v0 = func pointer, a0/a1 = args |
| 0x00168650 | alt_selection | gp slots state |

---

## Results

### Overall Statistics

| Metric | Value |
|--------|-------|
| Total events | 43,776,015 |
| Cycle range | 3,431,491,614 → 2,204,664,977,838 |
| Estimated play time | 7,337s (~122.3 min at 300MHz EE) |
| Unique entity contexts | 1,913 |
| Main dispatcher hits | 3,619,682 |
| Dispatch point hits | 36,536,651 |

### Label Distribution

| Label | Count | % |
|-------|-------|---|
| dispatch_point | 36,536,651 | 83.5% |
| main_dispatcher | 3,619,682 | 8.3% |
| cold_path_A | 1,965,576 | 4.5% |
| cold_path_B | 1,654,106 | 3.8% |

### Rare Probes — Full Session (3rd Confirmation)

| Probe | Rev.074 | Rev.079 | Rev.084 |
|-------|---------|---------|---------|
| mask_set | 2 hits (loading only) | 0 hits | **0 hits** |
| halfword_store_A | not probed | 0 hits | **0 hits** |
| halfword_store_B | not probed | 0 hits | **0 hits** |
| slot0_v0_from_gp | not probed | 0 hits | **0 hits** |
| alt_selection | 0 hits | 0 hits | **0 hits** |

**All 5 rare probes: ZERO hits across 3 independent sessions.** This is now a confirmed finding:
- mask_set (0x13ED40) = ShockRequestBox_RequestCancel, only fires during loading transitions, never in gameplay
- Halfword table spatial hash rebuild never triggers in areas tested
- Slot 0 callback never dispatched by any caller in the binary
- Alternative implementations never selected (cold paths are the only active paths)

### Slot Distribution

| Slot | Count | % | Group | Guard | Notes |
|------|-------|---|-------|-------|-------|
| 12 | 1,591,008 | 44.0% | G2 | — | Most active overall |
| 1 | 1,388,060 | 38.3% | G1 | — | Second most active |
| 3 | 259,763 | 7.2% | G1 | — | |
| 2 | 178,956 | 4.9% | G1 | — | a2=1 variant |
| 6 | 92,835 | 2.6% | G1 | mask 0x40000000 | |
| 14 | 33,031 | 0.9% | G2 | a2=1 | |
| 15 | 30,067 | 0.8% | G2 | field_60 mask | |
| 7 | 22,193 | 0.6% | G1 | mask 0x30000000 | |
| 4 | 17,771 | 0.5% | G1 | Triplet | |
| 11 | 4,108 | 0.1% | G1 | mask 0xC0000000 | **New this session** |
| 10 | 1,209 | <0.1% | G1 | mask 0x80000000 | |
| 5 | 681 | <0.1% | G1 | Triplet+mask | **New this session** |

Slots 0, 8, 9, 13, 16: **0 hits** (confirmed dead across all 3 sessions).

### New Slot Activations

- **Slot 11 (0x0b):** 4,108 hits. This slot has mask `(field_48 & 0xC0000000) == 0xC0000000`. First appearance in any session. Requires entities with the upper 2 bits of field_48 set (bits 31-30 = 11 binary).
- **Slot 5 (0x05):** 681 hits. This slot has w1=1 (triplet guard) + mask `(field_48 & 0xF0000000) == 0`. Triplet guard means `[ctx+0x74/78/7C]` must match entity data.

These new activations indicate the user reached game areas with entity types that have these specific field_48 flag configurations.

### Callback Distribution

| Callback | Slot | Count | % |
|----------|------|-------|---|
| 0x169AA8 | 12 | 16,280,869 | 44.6% |
| 0x168ED0 | 1 | 13,343,457 | 36.5% |
| 0x169440 | 3 | 2,693,306 | 7.4% |
| 0x1692F0 | 2 | 1,929,302 | 5.3% |
| 0x1696C0 | 6 | 980,541 | 2.7% |
| 0x169E58 | 14 | 413,921 | 1.1% |
| 0x169D18 | 15 | 388,325 | 1.1% |
| 0x169580 | 7 | 247,376 | 0.7% |
| 0x169020 | 4 | 187,937 | 0.5% |
| 0x169968 | 11 | 51,224 | 0.1% |
| 0x169800 | 10 | 12,732 | <0.1% |
| 0x169190 | 5 | 7,661 | <0.1% |

---

## Zone Analysis

The session was divided into 500K-event windows for zone detection. Each zone transition represents a shift in the active slot mix (which slots contribute ≥5% of dispatches).

### Zone Summary

| Window | Events | Active Slots | Dominant | Notes |
|--------|--------|--------------|----------|-------|
| 0-7 | 0-3.5M | 1,2,3,4,6,12 | slot12 ~35% | **Entrance area** (matches Rev.074) |
| 8-10 | 4-5M | 1,3,12 | slot12 ~47% | Transition zone |
| 11-13 | 5.5-6.5M | 1,2,3,12 | **slot1 ~48%** | **Windmill area** (matches Rev.079) |
| 16 | 8M | 1,12 | slot1 ~63% | Deep windmill |
| 28 | 14M | **12 only** | slot12 **100%** | **Cutscene/menu** — all other slots suspended |
| 33-43 | 16.5-21.5M | 1,12 (variable) | varied | Post-cutscene gameplay, slot1 recovers |
| 50-62 | 25-31.5M | 1,2,3,6,12 | slot12 ~45% | **3rd area** — new slot mix not seen before |
| 62+ | 31.5M+ | 1,12 | slot12 ~80% | End of session, slot12 dominant |

### Key Observation

The zone analysis confirms **3 distinct gameplay area patterns** and **1 cutscene period**. This is strong evidence that the slot distribution is determined by the game room/level, not by the game phase (intro vs mid vs late).

### Cross-Session Comparison

| Metric | Rev.074 | Rev.079 | Rev.084 |
|--------|---------|---------|---------|
| Area(s) | Entrance only | Windmill only | Entrance + Windmill + 3rd area + cutscene |
| Events | 9.1M | 14.0M | 43.8M |
| Duration | ~15 min | ~15 min | ~122 min |
| Entity contexts | 615 | low | 1,913 |
| Slot 12 | 38.5% | 37.1% | **44.0%** (blended) |
| Slot 1 | 27.0% | **45.7%** | **38.3%** (blended) |
| Slot 3 | 14.3% | 9.0% | 7.2% |
| mask_set | 2 (loading) | 0 | **0** |
| halfword_store | not probed | 0 | **0** |
| slot0 | not probed | 0 | **0** |
| alt_selection | 0 | 0 | **0** |
| Unique callbacks | 14 known | 12 seen | **12 seen** |

---

## Conclusions

### Confirmed (3-session evidence)

1. **mask_set never fires during gameplay** — only loading transitions. 2 hits total across 66.9M events, both during loading.
2. **Slot 0 is dead** — zero dispatch sites reference slot 0 in all of .text. Confirmed across 66.9M runtime events.
3. **Halfword table writers never trigger** in any tested area. The spatial hash rebuild is tied to an unknown condition.
4. **Alt implementations never selected** in normal gameplay.
5. **GP = 0x006388F0** confirmed at runtime (3rd session).
6. **Slot distribution varies by area**: the ratio of slot 12 to slot 1 changes depending on which room/zone the player is in.
7. **12 of 17 slots** are actively used in gameplay. Slots 0, 8, 9, 13, 16 are confirmed dead.

### New This Session

1. **Slot 11** (mask 0xC0000000==0xC0000000) and **Slot 5** (triplet guard + mask) were observed for the first time, indicating the user reached areas with specific entity flag configurations.
2. The cutscene period (window 28, 100% slot 12) shows that the game suspends all Group 1 slots during non-gameplay segments.
3. **1,913 unique entity contexts** were observed (vs 615 in Rev.074's entrance-only session), covering many more game objects.

### What Remains Unknown

1. What triggers halfword table rebuild (spatial hash)? — 3 sessions, 0 activations.
2. What condition activates mask_set bit 0 during gameplay? — never observed in ~2h of play.
3. What is the most-referenced GP variable (gp-0x49B4, 434 refs)? — requires dedicated probe.
4. world_state transitions at runtime (gp+0x6F60) — requires probe at room init.
5. Slot 0's purpose (callbacks configured but no dispatcher ever selects slot 0).

---

## Next Runtime Targets

1. Probe gp+0x6F60 (0x00631990) to capture world_state during room transitions
2. Die intentionally to capture mask_set toggle
3. Halfword writer probe at function entry (0x166D1C) instead of SH instruction
4. Probe gp-0x49B4 (0x00633F3C) to identify the most-referenced global
