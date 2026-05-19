# Rev.094 - Halfword runtime session: second caller observed

**Date:** 2026-05-19

---

## Objective

Analyze the Rev.094 runtime capture after the Rev.093b halfword writer findings,
with focus on:

- whether the inferred fast path pattern repeats;
- whether the second static caller of `0x00166BB0` appears at runtime;
- whether longer grid traces occur during extended play;
- whether new `world_state_raw` values appear.

---

## Scope

- Runtime capture: `.local/runtime-captures/ico-probe-rev094/events.jsonl.gz`
- Offline summary: `/tmp/ico-rev094-halfword-summary.txt`
- Tool: `tools/analyze_halfword_log.py`
- Prior baseline: `research/elf/ghidra-rev093b-halfword-entry-runtime-validation.md`

The capture was initially written as a raw `events.jsonl` file, then compressed
to `events.jsonl.gz`. `gzip -t` passed after compression.

The embedded JSON `"session"` field still says `ico_rev093b`; this is a probe
label compiled into the PCSX2 logpoint build. The file path and analysis context
are Rev.094.

---

## Sources Used

| Source | Use |
|--------|-----|
| `.local/runtime-captures/ico-probe-rev094/events.jsonl.gz` | Runtime event source |
| `tools/analyze_halfword_log.py` | Reproducible event grouping |
| `research/elf/ghidra-rev093b-halfword-entry-runtime-validation.md` | Baseline comparison |
| Static disassembly around `0x0016700C` and `0x0016828C` | Caller interpretation |

Reproducible command:

```sh
python3 tools/analyze_halfword_log.py .local/runtime-captures/ico-probe-rev094/events.jsonl.gz --top 30
```

---

## Evidence Used

### Capture size

| Item | Value |
|------|------:|
| Raw line count before compression | 72,787,591 |
| Compressed file size | ~1.4 GB |
| Gzip integrity | Passed |

### Event totals

| Label | Hits |
|-------|-----:|
| `halfword_entry` | 59,285,635 |
| `halfword_write_A` | 12,009,348 |
| `halfword_write_B` | 2,154,870 |
| `world_state_load` | 62 |

### Return addresses

| Return address | Hits | Meaning |
|----------------|-----:|---------|
| `0x00167014` | 73,435,596 | Main hot dispatch caller after `jal 0x00166BB0` at `0x0016700C` |
| `0x00168294` | 14,257 | Second static caller after `jal 0x00166BB0` at `0x0016828C` |
| `0x001AF948` | 62 | `world_state_load` probe label |

For `ra=0x00168294`, all counted events were `halfword_entry` events. This
confirms the second direct caller is runtime-reachable.

### Per-invocation grouping

The log was grouped by `halfword_entry`, then the next entry's counter was used
as the previous invocation's final counter.

| Final counter | Observed A/B writes | Invocations |
|--------------:|--------------------:|------------:|
| 0 | 0 | 41,073,482 |
| 1 | 0 | 8,006,350 |
| 1 | 1 | 6,874,880 |
| 2 | 2 | 2,773,356 |
| 3 | 3 | 539,553 |
| 4 | 4 | 10,545 |
| 5 | 5 | 2,925 |
| 6 | 6 | 1,125 |
| 23 | 23 | 726 |
| 22 | 22 | 646 |
| 15 | 15 | 443 |
| 24 | 24 | 424 |
| 26 | 26 | 2 |

The `counter=1` / zero-A-B-write bucket repeats the Rev.093b fast-path pattern
at much larger scale. The only known static path compatible with this is the
single-cell store at `0x00166DFC`, followed by `sw v1,-0x4BC4(gp)` at
`0x00166E00`.

### Top observed cells

| Cell `(row,col)` | Hits |
|------------------|-----:|
| `(0,0)` | 1,058,432 |
| `(0,1)` | 528,045 |
| `(1,0)` | 526,824 |
| `(3,3)` | 461,888 |
| `(3,1)` | 455,736 |
| `(2,0)` | 421,363 |
| `(3,0)` | 416,406 |
| `(4,1)` | 409,302 |
| `(1,1)` | 376,077 |
| `(2,3)` | 359,066 |

### Notable sequence expansion

Rev.093b observed short traces up to 5 cells. Rev.094 observed rare long traces
up to 26 cells. This confirms the rasterizer is not limited to the small
0-5-cell pattern seen in the first session.

### World-state values

| `world_state_raw` | Hits |
|-------------------|-----:|
| `0x00000015` | 19 |
| `0x00000014` | 16 |
| `0x00000013` | 10 |
| `0x00000012` | 6 |
| `0x0000000A` | 5 |
| `0x00000032` | 2 |
| `0x00000009` | 2 |
| `0x00000001` | 1 |
| `0x00000011` | 1 |

Rev.094 therefore expands observed world-state coverage beyond the Rev.093b
values `0x13..0x15` plus startup `0x01`.

---

## Byte-Level or Instruction-Level Findings

No new byte-level decoding was required beyond Rev.093b. The runtime evidence
maps directly onto already verified instructions:

| Address | Status | Notes |
|---------|--------|-------|
| `0x0016700C` | confirmed | Main `jal 0x00166BB0`, dominant caller |
| `0x00167014` | confirmed | Main return address observed at scale |
| `0x0016828C` | confirmed runtime | Second direct caller of `0x00166BB0` |
| `0x00168294` | confirmed runtime | Return address for second caller |
| `0x00166DFC` | inferred | Single-cell fast path; still needs direct probe |
| `0x00166E00` | inferred | Counter set after the fast path |

| Address | Instruction role | Rev.094 relevance |
|---------|------------------|-------------------|
| `0x0016700C` | Main `jal 0x00166BB0` | Dominant observed caller |
| `0x00167014` | Main return address | 73.4M event return-address hits |
| `0x0016828C` | Second `jal 0x00166BB0` | Newly runtime-observed caller |
| `0x00168294` | Second return address | 14,257 `halfword_entry` hits |
| `0x00166DFC` | Single-cell `sh` fast path | Strongly inferred by `counter=1`, zero-A/B-write bucket |
| `0x00166E00` | Set counter to 1 after fast path | Matches inferred fast path final counter |

---

## What Is Confirmed

1. The second direct caller of `0x00166BB0` at `0x0016828C` is runtime-reachable.
2. Rev.094 produced 14,257 `halfword_entry` hits returning to `0x00168294`.
3. The main caller at `0x0016700C` remains the overwhelmingly dominant path.
4. The inferred single-cell fast path pattern repeats at large scale:
   8,006,350 invocations ended with `counter=1` and no observed A/B write.
5. The rasterizer can produce far longer traces than Rev.093b showed:
   up to 26 observed cells in Rev.094.
6. Additional world-state values appeared: `0x09`, `0x0A`, `0x11`, `0x12`,
   and `0x32`.

---

## What Is Probable

- `0x00166DFC` is an active high-volume single-cell path.
- The `0x0016828C` caller is a less common dispatch/helper path that appears
  only under some later gameplay or context condition.
- Long traces around 22-26 cells likely correspond to wider or longer spatial
  segments crossing many grid cells, not a different table format.

---

## What Is Possible

- The second caller may be associated with contexts outside the usual
  `0x0063....` work-area cluster, because sample events included addresses such
  as `0x013B4060` and `0x0139ECE0`.
- `world_state_raw=0x32` may represent a transient or non-room state; this note
  does not assign semantics to it.

---

## What Is Unknown

- Direct hit count at `0x00166DFC`; it remains inferred, not directly probed.
- The semantic role of the `0x0016828C` caller.
- Which gameplay action or room transition first activated the second caller.
- The meaning of newly observed `world_state_raw` values.

---

## What Is Discarded

| Discarded idea | Reason |
|----------------|--------|
| `0x0016828C` is static-only / unreachable | Rev.094 observed 14,257 entry hits with `ra=0x00168294` |
| The writer only produces short 0-5-cell traces | Rev.094 observed traces up to 26 cells |
| Rev.093b fast-path inference was a one-session artifact | Rev.094 repeated the same `counter=1`, zero-A/B-write pattern at 8.0M invocations |

---

## Next Minimum Test

Add direct probes for:

| Probe | Address | Purpose |
|-------|---------|---------|
| `halfword_fast_path` | `0x00166DFC` | Directly count the inferred single-cell path |
| `halfword_second_caller_entry` | `0x0016828C` | Capture caller-side state before invoking `0x00166BB0` |
| `halfword_second_caller_return` | `0x00168294` | Capture selected callback/context after the writer returns |
| `halfword_exit_main` | `0x00167014` | Capture final counter after main writer return |

The next runtime session should also log the current `world_state_raw` alongside
`halfword_entry` for the second caller, so the activation condition can be tied
to state transitions.

---

## Conservative Verdict

Rev.094 materially extends Rev.093b. The halfword writer remains confirmed as a
hot-path spatial grid rasterizer, the inferred fast path is reinforced, the
previously unobserved second direct caller is now runtime-confirmed, and the
observed raster traces expanded from short 0-5-cell cases to rare 22-26-cell
cases. The remaining minimum proof is a direct probe at `0x00166DFC` and a
caller-side probe around `0x0016828C`.
