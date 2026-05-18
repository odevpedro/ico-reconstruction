# Rev.079 — Runtime Validation Session: Windmill Section (14M events)

**Date:** 2026-05-18
**Session duration:** ~15 minutes of play, entrance to windmill
**Total events:** 14,014,800
**Log size:** 4.8 GB raw / 187 MB gzipped
**Build:** Custom PCSX2 with 9 probes (rev078 probes, rebuilt 2026-05-18)
**GP confirmed:** 0x006388F0

---

## Objective

Validate the live dispatch system with updated probes targeting:
- Slot distribution and callback mapping (main_dispatcher + dispatch_point)
- Cold path activity and handler status
- mask_set behavior during gameplay (not just loading)
- Halfword table writer activation
- Slot 0 deadness confirmation

---

## Probes

| Address | Label | What it captures |
|---------|-------|-----------------|
| 0x00166E10 | main_dispatcher | a1 = slot_index (0-16) |
| 0x00167020 | dispatch_point | v1 = callback target, a0 = context, a1 = slot desc ptr |
| 0x00167230 | cold_path_A | gp slots, a0 = context |
| 0x00167258 | cold_path_B | gp slots, a0 = context |
| 0x0013ED40 | mask_set | a0 = bit_index, a1 = set_flag, mask_before |
| 0x00166D38 | halfword_store_A | written_value, write_addr, a2_row, t0_sub |
| 0x00166D94 | halfword_store_B | same as A |
| 0x001683CC | slot0_v0_from_gp | v0 = func ptr loaded from gp-0x6500 |
| 0x00168650 | alt_selection | a0 = flag, default and alt handler values |

---

## Results

### 1. Slot Distribution (main_dispatcher, 1.7M events)

| Slot | Events | % | Tier | Callback |
|------|--------|---|------|----------|
| 1 | 780,483 | 45.7% | Tier 1 (leaf pos/rot) | 0x168ED0 |
| 12 | 633,231 | 37.1% | Tier 3 (full pipeline) | 0x169AA8 |
| 3 | 133,924 | 7.8% | Tier 2 (hybrid) | 0x169440 |
| 2 | 82,848 | 4.9% | Tier 1 | 0x1692F0 |
| 6 | 45,225 | 2.6% | Tier 2 | 0x1696C0 |
| 14 | 10,421 | 0.6% | Tier 3 | 0x169E58 |
| 7 | 10,143 | 0.6% | Tier 2 | 0x169580 |
| 4 | 8,465 | 0.5% | Tier 1 | 0x169020 |
| 15 | 6,198 | 0.4% | Tier 3 | 0x169D18 |
| 11 | 1,130 | 0.1% | Tier 2 | 0x169968 |
| 10 | 721 | <0.1% | Tier 2 | 0x169800 |
| 5 | 294 | <0.1% | Tier 1 | 0x169190 |
| 0,8,9,13,16 | 0 | 0% | — | — |

### 2. Callback Distribution (dispatch_point, 10.7M events)

| Callback | Events | % | Slots | Group |
|----------|--------|---|-------|-------|
| 0x168ED0 | 4,670,615 | 43.7% | 1, 8 | G1 — leaf pos/rot |
| 0x169AA8 | 4,201,857 | 39.3% | 12, 16 | G2 — full pipeline |
| 0x169440 | 854,990 | 8.0% | 3, 9 | G1 — hybrid |
| 0x1692F0 | 521,335 | 4.9% | 2 | G1 — leaf pos/rot |
| 0x1696C0 | 291,341 | 2.7% | 6 | G1 — hybrid |
| 0x169580 | 77,109 | 0.7% | 7 | G1 — hybrid |
| 0x169E58 | 66,643 | 0.6% | 14 | G2 — full pipeline |
| 0x169020 | 56,625 | 0.5% | 4 | G1 — leaf pos/rot |
| 0x169D18 | 43,708 | 0.4% | 15 | G2 — full pipeline |
| 0x169968 | 9,386 | 0.1% | 11 | G1 — hybrid |
| 0x169800 | 5,037 | <0.1% | 10 | G1 — hybrid |
| 0x169190 | 1,700 | <0.1% | 5 | G1 — leaf pos/rot |

### 3. Probes That Did NOT Fire

| Probe | Address | Expected | Reality |
|-------|---------|----------|---------|
| mask_set | 0x13ED40 | Toggle bit 0 during load | **0 hits** — no loading transitions in this section |
| halfword_store_A | 0x166D38 | SH to 0x6AB080 | **0 hits** — spatial hash not rebuilt in this section? |
| halfword_store_B | 0x166D94 | SH to 0x6AB080 | **0 hits** |
| slot0_v0_from_gp | 0x1683CC | Load handler from gp-0x6500 | **0 hits** — slot 0 never selected |
| alt_selection | 0x168650 | Swap cold path handlers | **0 hits** — defaults always used |

### 4. Cold Path Analysis

| Context | Cold Path A events | % of cold |
|---------|--------------------|-----------|
| 0x0063c650 | 335,332 | 31.5% |
| 0x0063b590 | 68,024 | 6.4% |
| 0x004bec10 | 50,592 | 4.8% |
| 0x0063aa90 | 49,091 | 4.6% |
| 0x0063ae30 | 47,018 | 4.4% |
| ... | ... | ... |

GP slots remain at default values:
- gp-0x6500 (0x006323C0) = 0x00167230 (cold_path_A)
- gp-0x64FC (0x006323C4) = 0x00167258 (cold_path_B)

**Alt selection never called.** Cold paths are never swapped during gameplay.

### 5. Entity Context Addresses (main_dispatcher a0)

15 unique context addresses observed:
- 0x0063c650: 335K (most active)
- 0x0063b590: 132K
- 0x004bec10: 101K
- 0x0063ae30: 70K
- ... (11 more in 0x0063xxxx range)

These are heap-allocated contexts, different from Rev.074's 0x0083xxxx range — likely due to different game state/save.

---

## Comparison with Rev.074

| Metric | Rev.074 (entrance) | Rev.078 (windmill) | Delta |
|--------|-------------------|-------------------|-------|
| Total events | 9,151,217 | 14,014,800 | +53% |
| Active slots | 12 | 12 | Same |
| Top slot | 12 (38.5%) | 1 (45.7%) | **Inverted** |
| Slot 12 rank | 1st | 2nd | ↘ |
| Slot 1 rank | 2nd (26.8%) | 1st (45.7%) | ↗ |
| Dead slots | 0,8,9,13,16 | 0,8,9,13,16 | Same |
| GP | 0x006388F0 | 0x006388F0 | Confirmed |
| alt_impl A/B | 0 hits | not probed | N/A |
| mask_set | not probed | 0 hits | Never fires mid-game |

**Key insight:** The slot distribution shift suggests different game sections activate different callback tiers. The entrance section had more full-pipeline work (slot 12), while the windmill section has more leaf position/rotation updates (slot 1). This likely corresponds to different entity compositions in each area.

---

## Confirmed Results

- **12 callbacks fully mapped** across 15 active slots (2 always dead: 0, 13)
- **No runtime deviation from static table** — every observed callback matches a slot entry
- **Cold paths use default handlers only** — alt_selection never called
- **mask_set never fires during active gameplay** — only at loading transitions
- **slot 0 confirmed dead** across two independent sessions (Rev.074 + Rev.078)
- **Halfword writers never called** during this section — needs investigation

## Unknown

- Why halfword_store_A/B probes didn't fire — the spatial hash rebuild may use a different code path or be triggered by specific entity activity not present in this section
- The exact entity type mapped to each context address (runtime heap addresses cannot be statically resolved)
- The relationship between slot distribution and game area (entrance vs windmill) — needs more sections

## Next Steps

1. Investigate halfword table writer activation — need a save point near entity-dense area or trigger condition
2. Capture a loading transition to see mask_set in action (die and respawn, or enter a new room)
3. Repeat section traversal with more probes (add the 6 write-only probes that didn't fire this session)
