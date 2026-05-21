# Rev.096 - Halfword runtime session analysis: offline review of Rev.095 capture

**Date:** 2026-05-19

---

## Objective

Analyze the existing Rev.095 halfword capture without using the emulator, and
extract the strongest confirmed facts about:

- the dominant caller path into `0x00166BB0`;
- the inferred single-cell fast path at `0x00166DFC`;
- whether the second caller at `0x0016828C` appears in this session;
- how `world_state_raw` is distributed in the capture.

---

## Scope

- Runtime capture: `.local/runtime-captures/ico-probe-rev095/events.jsonl`
- Analyzer: `tools/analyze_halfword_log.py`
- Supplemental filters: `rg -c` and `rg -o` over the same capture
- Prior baseline: `research/elf/ghidra-rev093b-halfword-entry-runtime-validation.md`
- Follow-up context: `research/elf/ghidra-rev094-halfword-runtime-second-caller.md`

This note is strictly offline. No emulator session was used while writing it.

---

## Sources Used

| Source | Use |
|--------|-----|
| `.local/runtime-captures/ico-probe-rev095/events.jsonl` | Raw runtime event source |
| `tools/analyze_halfword_log.py` | Reproducible grouping and summary |
| `rg -c` / `rg -o` on the same capture | Fast cross-checks for zero-hit and world-state counts |
| Rev.093b / Rev.094 notes | Prior runtime baseline |

---

## Evidence Used

### Capture totals

| Label | Hits |
|-------|-----:|
| `halfword_entry` | 61,504,387 |
| `halfword_write_A` | 12,501,059 |
| `halfword_write_B` | 2,155,561 |
| `world_state_load` | 38 |

### Return addresses

| Return address | Hits | Meaning |
|----------------|-----:|---------|
| `0x00167014` | 76,155,123 | Main hot-path return after `jal 0x00166BB0` at `0x0016700C` |
| `0x00168294` | 5,884 | Second direct caller return site |
| `0x001AF948` | 38 | `world_state_load` probe label |

### Entry callsites

| Callsite | Hits | Meaning |
|----------|-----:|---------|
| `0x0016700C` | 61,498,503 | Dominant direct caller of `0x00166BB0` |
| `0x0016828C` | 5,884 | Rare second direct caller |

### Counter versus observed writes

| Final counter | Observed A/B writes | Invocations |
|--------------:|--------------------:|------------:|
| 0 | 0 | 42,780,141 |
| 1 | 0 | 8,233,666 |
| 1 | 1 | 7,099,680 |
| 2 | 2 | 2,792,932 |
| 3 | 3 | 561,580 |
| 4 | 4 | 18,384 |
| 5 | 5 | 4,018 |
| 6 | 6 | 2,629 |
| 11 | 11 | 3,257 |
| 14 | 14 | 2 |
| 15 | 15 | 50 |
| 17 | 17 | 1,473 |
| 18 | 18 | 1,331 |
| 22 | 22 | 1,801 |
| 23 | 23 | 1,331 |
| 24 | 24 | 283 |

### Top cell coordinates

| Cell `(row,col)` | Hits |
|------------------|-----:|
| `(0,0)` | 1,534,992 |
| `(0,1)` | 930,043 |
| `(1,0)` | 744,720 |
| `(1,1)` | 588,836 |
| `(2,0)` | 484,359 |
| `(0,2)` | 476,636 |
| `(2,1)` | 475,182 |
| `(3,0)` | 439,895 |
| `(3,1)` | 340,738 |
| `(4,4)` | 288,160 |

### Top context addresses

| `a0` | Total entries | With writes | Write events |
|------|--------------:|------------:|-------------:|
| `0x0063aed0` | 5,771,334 | 455,470 | 534,195 |
| `0x0063ae30` | 5,262,199 | 267,702 | 325,923 |
| `0x0063aa90` | 4,269,005 | 1,291,188 | 1,999,408 |
| `0x0063ab30` | 3,530,470 | 1,082,570 | 1,675,132 |
| `0x0063aa40` | 3,477,951 | 1,064,350 | 1,574,520 |
| `0x0063af00` | 3,149,707 | 478,585 | 559,152 |
| `0x0063a9a0` | 2,431,577 | 766,734 | 1,169,914 |
| `0x0063ac30` | 2,073,543 | 0 | 0 |
| `0x0063c650` | 2,056,108 | 0 | 0 |
| `0x0063c5d0` | 1,966,748 | 0 | 0 |

### World-state values

| `world_state_raw` | Hits |
|-------------------|-----:|
| `0x00000013` | 8 |
| `0x00000015` | 6 |
| `0x00000014` | 6 |
| `0x0000000A` | 5 |
| `0x00000016` | 3 |
| `0x00000012` | 3 |
| `0x00000009` | 2 |
| `0x00000032` | 1 |
| `0x00000017` | 1 |
| `0x0000000B` | 1 |
| `0x00000008` | 1 |
| `0x00000001` | 1 |

Supplemental zero-hit checks on the same capture:

- `0x0016828C`: 0 occurrences in the sampled capture text
- `0x00166DFC`: 0 occurrences in the sampled capture text

---

## Byte-Level or Instruction-Level Findings

No new byte-level decoding was required for this note. The runtime evidence
maps to already verified instruction sites from the previous notes:

| Address | Status | Notes |
|---------|--------|-------|
| `0x0016700C` | confirmed | Main caller into `0x00166BB0` |
| `0x00167014` | confirmed | Main return site |
| `0x0016828C` | confirmed runtime in Rev.094, absent here | Rare second caller |
| `0x00168294` | confirmed runtime in Rev.094, absent here | Second return site |
| `0x00166DFC` | inferred | Single-cell fast path; still not directly probed here |
| `0x00166E00` | inferred | Counter update after the fast path |

---

## What Is Confirmed

1. The dominant caller path is still `0x0016700C -> 0x00166BB0 -> 0x00167014`.
2. The Rev.095 capture is very large: 61,504,387 `halfword_entry` hits.
3. `halfword_write_A` and `halfword_write_B` are both active at scale.
4. The capture is concentrated in a small `world_state_raw` cluster:
   mostly `0x13`, `0x14`, `0x15`, and `0x0A`.
5. The top `a0` contexts are the familiar `0x0063....` work-area cluster.
6. The `counter` / write pairing is strong enough to preserve the earlier
   fast-path hypothesis, but not enough to convert it into a direct probe.

---

## What Is Probable

- The `counter=1` / zero-write bucket is still the best indirect evidence for
  the single-cell path at `0x00166DFC`.
- The second caller `0x0016828C` is likely state- or route-dependent, because
  it did not appear in this capture despite appearing in Rev.094.
- The capture stayed in a narrow gameplay region or loop rather than exploring
  a broad set of state transitions.

---

## What Is Possible

- The `0x0016828C` caller may require a different `world_state_raw` value or a
  different `a0` context cluster than the one captured here.
- The `counter=1` / one-write bucket may represent a separate short path from
  the zero-write bucket rather than a single unified fast path.
- Some of the non-`0x0063....` contexts may belong to a different system path
  that still feeds the same writer.

---

## What Is Unknown

- Direct hit count for `0x00166DFC`.
- Why `0x0016828C` was absent in this particular capture.
- Whether the `counter=1` / zero-write bucket and `counter=1` / one-write
  bucket are semantically distinct or just different coverage artifacts.
- Which exact player action or room transition would maximize the second caller.

---

## What Is Discarded

| Discarded idea | Reason |
|----------------|--------|
| `0x0016828C` is a stable, always-on caller in this session | Zero hits in the sampled Rev.095 capture |
| `0x00166DFC` was directly observed in this session | Zero hits in the sampled Rev.095 capture |
| The Rev.095 capture is too small to be useful | It still contains 61.5M `halfword_entry` hits |

---

## Next Minimum Test

The next live session should still probe:

1. `0x00166DFC` directly.
2. `0x0016828C` and `0x00168294`.
3. `0x00167014` for the main caller return.
4. `world_state_raw` alongside those events.

For route selection, prioritize the same world-state cluster that dominated
this capture, then deliberately branch into a different state cluster to try
to force the rare caller.

---

## Conservative Verdict

The offline Rev.095 capture strengthens the hot-path picture but does not
resolve the remaining branch questions. The main caller is confirmed at very
high volume, the second caller remains absent in this session, and the
single-cell fast path is still an inference that needs a direct runtime probe.

---

## Addendum - 2026-05-21 Live Recapture

The current live capture at:

- `.local/runtime-captures/ico-probe-rev095-next/events.jsonl`

adds another large hot-path sample without changing the branch conclusions.

### Recent recapture slice

The latest `50 MB` slice of the log contains:

| Label | Hits |
|-------|-----:|
| `halfword_entry` | `120,820` |
| `halfword_write_A` | `24,506` |
| `halfword_write_B` | `5,286` |
| `world_state_load` | `0` |

Top PCs in that slice:

| PC | Hits | Role |
|----|-----:|------|
| `0x00166BB0` | `120,820` | Dominant writer entry |
| `0x00166D1C` | `24,506` | Write path A |
| `0x00166D78` | `5,286` | Write path B |

Top return address:

| RA | Hits | Role |
|----|-----:|------|
| `0x00167014` | `150,612` | Dominant return site |

### What this addendum confirms

1. The hot path remains stable and high-volume.
2. The writer continues to operate through the same dominant entry and return
   sites observed in Rev.093b/Rev.096.
3. The recapture still does not show direct evidence for `0x00166DFC`.
4. The recapture still does not show the second caller at `0x0016828C` or
   `0x00168294`.
5. `world_state_load` did not appear in the sampled recent slice, so this
   recapture does not refine the room-transition hypothesis.

### What remains open

- direct fast-path confirmation at `0x00166DFC`;
- state correlation for `0x0016828C` / `0x00168294`;
- any room-transition relationship that would explain when the rare caller
  becomes visible.

### Conservative reading

This recapture is useful as reinforcement, not as a revision. It increases
confidence in the dominant writer path, but it does not change the status of
the rare branches.

### Freshest tail check

A later freshness check on the live log showed an even narrower tail:

| Metric | Value |
|--------|-----:|
| `halfword_entry` | `151,531` |
| `halfword_write_A` | `0` |
| `halfword_write_B` | `0` |
| `world_state_load` | `0` |

Top observed context in that tail:

| Field | Value | Hits |
|-------|-------|-----:|
| `a0` | `0x0063c3e0` | `151,531` |
| `ra` | `0x00167014` | `151,531` |

This does not change the branch conclusions. It does confirm that the newest
tail of the capture is locked even more tightly onto the dominant entry/return
pair, with no write labels or world-state labels visible in that freshest
window.

### Structural summary of the hot path

The current live capture is still dominated by a small set of `a0` work-area
clusters. The most useful way to read the hot path is by bucket, not by a
single global interpretation.

| Bucket | Observed `a0` values | Observed behavior |
|--------|----------------------|------------------|
| Entry-only tail | `0x0063c3e0` | `halfword_entry` only; no writes in the freshest tail slice |
| Write A heavy | `0x00000000`, `0x00000001`, `0x00000061`, `0x00000062`, `0x00000063`, `0x00000064`, `0x000000E4`, `0x00000163` | Frequent `halfword_write_A` activity in the recent `50 MB` slice |
| Write B heavy | `0x00000020`, `0x00000040`, `0x000000E5`, `0x00000104` | Recurring `halfword_write_B` activity in the recent `50 MB` slice |

The write buckets are consistent with the same dominant writer entry/return
pair:

- `0x00166BB0` is the entry site;
- `0x00167014` is the return site;
- `0x00166D1C` is the A write path;
- `0x00166D78` is the B write path.

What this summary does:

- confirms that the hot path is not a single flat loop, but a small set of
  recurring `a0` contexts;
- preserves the conservative split between entry-only tails and write-bearing
  invocations;
- avoids assigning gameplay semantics to the buckets without a direct probe.

What this summary does not do:

- it does not identify the exact object or room semantics behind any `a0`
  value;
- it does not resolve `0x00166DFC`;
- it does not explain why `0x0016828C` / `0x00168294` stay absent in this
  capture.
