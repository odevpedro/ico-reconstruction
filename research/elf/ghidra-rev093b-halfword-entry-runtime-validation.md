# Rev.093b - Halfword writer entry runtime validation

**Date:** 2026-05-19

---

## Objective

Validate the Rev.093 minimum test for the halfword spatial hash writer:

- move the probe from the halfword `sh` stores to function entry `0x00166BB0`;
- determine whether the writer is dead, scene-load only, early-exiting, or active before callback dispatch;
- characterize the observed write paths with runtime evidence.

---

## Scope

- Runtime capture: `.local/runtime-captures/ico-probe-rev093b/events.jsonl.gz`
- Static disassembly of `0x00166BB0..0x00166E10`
- Static disassembly of caller region `0x00166E10..0x00167040`
- Existing Rev.071, Rev.073, Rev.076, and Rev.093 halfword-table model

This note does not rename the gameplay semantics of the grid. It only validates
the writer's activation and its location in the dispatch pipeline.

---

## Sources Used

| Source | Use |
|--------|-----|
| `AGENTS.md` | Current project constraints and Rev.093 status |
| `research/elf/ghidra-rev093-three-investigations.md` | Previous unresolved halfword writer status |
| `research/elf/ghidra-rev073-main-loop-dispatch-chain-and-callback-corrected-masks.md` | Callback consumer model |
| `.local/runtime-captures/ico-probe-rev093b/events.jsonl.gz` | Runtime hit counts and probe registers |
| `tools/asm_source_score.py` | ELF byte extraction and MIPS little-endian disassembly helper |
| `tools/analyze_halfword_log.py` | Reproducible offline grouping of Rev.093b probe events |

---

## Evidence Used

### Runtime event totals

The Rev.093b capture contains roughly 24.75M probe events. One streaming JSON
parse encountered a malformed/truncated line near the end of the gzip stream, so
the safest totals here are from regex label counts over the compressed log.

Reproducible command:

```sh
python3 tools/analyze_halfword_log.py .local/runtime-captures/ico-probe-rev093b/events.jsonl.gz --top 20
```

| Label | PC | Hits | Share of labeled events |
|-------|----|-----:|------------------------:|
| `halfword_entry` | `0x00166BB0` | 20,153,859 | 81.42% |
| `halfword_write_A` | `0x00166D1C` | 3,921,188 | 15.84% |
| `halfword_write_B` | `0x00166D78` | 677,707 | 2.74% |
| `world_state_load` | `0x001AF948` | 23 | <0.01% |

Important probe-address detail:

| Probe label | Probe PC | Actual store reached | Why this still confirms write |
|-------------|----------|----------------------|-------------------------------|
| `halfword_write_A` | `0x00166D1C` | `0x00166D38` | No branch between setup and `sh` |
| `halfword_write_B` | `0x00166D78` | `0x00166D94` | No branch between setup and `sh` |

### Counter values captured at probed PCs

The counter is `gp-0x4BC4` = `0x00633D2C`. At function entry it is the value
left by the previous invocation, because `0x00166BDC` resets it to zero inside
the function before any new writes.

| Captured counter | Hits |
|------------------|-----:|
| `0x00000000` | 17,520,056 |
| `0x00000001` | 5,864,015 |
| `0x00000002` | 1,153,870 |
| `0x00000003` | 210,626 |
| `0x00000004` | 4,096 |
| `0x00000005` | 90 |

The writer commonly records 0-2 cells, sometimes 3, rarely 4-5 in this session.

### Offline invocation grouping

The compressed log was streamed in chronological order and grouped as:

```txt
halfword_entry
  zero or more write_A/write_B setup hits
next halfword_entry counter = previous invocation's final counter
```

This recovers per-invocation behavior without new emulation.

| Final counter seen at next entry | Observed A/B writes before next entry | Invocations | Interpretation |
|---------------------------------:|--------------------------------------:|------------:|----------------|
| 0 | 0 | 14,289,843 | No valid cell recorded |
| 1 | 0 | 2,633,803 | Strongly inferred fast path (`0x00166DFC`) |
| 1 | 1 | 2,076,342 | One loop-path cell |
| 2 | 2 | 943,244 | Two loop-path cells |
| 3 | 3 | 206,530 | Three loop-path cells |
| 4 | 4 | 4,006 | Four loop-path cells |
| 5 | 5 | 90 | Five loop-path cells |

Summary:

| Category | Invocations | Share |
|----------|------------:|------:|
| No recorded cell | 14,289,843 | ~70.9% |
| Inferred fast path only | 2,633,803 | ~13.1% |
| Loop A/B writes observed | 3,230,212 | ~16.0% |

This makes `0x00166DFC` the likely high-volume single-cell path in this capture,
even though Rev.093b did not include a direct probe at that PC.

### Contexts most associated with writes

The `a0` captured at `halfword_entry` is the writer input context. The highest
write-producing contexts in this session were:

| `a0` context | Entry hits | Invocations with A/B writes | A/B write events |
|--------------|-----------:|----------------------------:|-----------------:|
| `0x0063AA90` | 1,686,512 | 414,691 | 641,475 |
| `0x0063AA40` | 1,336,472 | 343,947 | 517,881 |
| `0x0063AB30` | 1,224,742 | 320,524 | 490,887 |
| `0x0063A9A0` | 1,029,825 | 276,536 | 442,140 |
| `0x0063B090` | 784,081 | 196,070 | 244,806 |

These addresses identify good candidates for a later memory-structure dump.
They do not by themselves prove entity names or gameplay semantics.

### Observed grid coordinates at write probes

Rows and columns are captured from the writer setup probes:

| Row (`a2`) | Hits |
|------------|-----:|
| `0x00` | 1,039,280 |
| `0x03` | 774,084 |
| `0x01` | 729,765 |
| `0x02` | 612,933 |
| `0x04` | 522,551 |
| `0x05` | 288,362 |
| `0x07` | 193,826 |
| `0x06` | 170,118 |
| `0x08..0x11` | observed, lower counts |

| Column (`t0`) | Hits |
|---------------|-----:|
| `0x00` | 1,539,953 |
| `0x01` | 1,169,134 |
| `0x02` | 682,932 |
| `0x03` | 588,080 |
| `0x04` | 278,386 |
| `0x05` | 73,602 |
| `0x06..0x0B` | observed, lower counts |

Observed rows stay within `0..17` and observed columns within `0..11` for the
sampled session. This is consistent with the instruction-level 32x32 bounds
checks, but it does not prove the full possible range is limited to those values.

### World-state probe context

The same capture contains 23 `world_state_load` events:

| `world_state_raw` | Hits |
|-------------------|-----:|
| `0x00000014` | 10 |
| `0x00000015` | 8 |
| `0x00000013` | 4 |
| `0x00000001` | 1 |

`room_init_fn` was captured as `0x00000000` in these events. This confirms the
probe as placed did not yet recover a room init function pointer.

---

## Byte-Level or Instruction-Level Findings

### Writer function `0x00166BB0`

Key instructions:

| Address | Instruction | Finding |
|---------|-------------|---------|
| `0x00166BB0` | `lw v1,-0x4BC0(gp)` | Loads secondary/current dispatch data pointer |
| `0x00166BB4` | `lwc1 f2,0(a0)` | Reads context coordinate |
| `0x00166BB8` | `lw v0,0x20(v1)` | Loads reference/origin pointer |
| `0x00166BDC` | `sw zero,-0x4BC4(gp)` | Resets halfword counter every invocation |
| `0x00166BFC` | `sra t0,a3,9` | Converts one coordinate to grid cell by `/512` |
| `0x00166C0C` | `sra a2,a1,9` | Converts another coordinate to grid cell by `/512` |
| `0x00166D18` | `lw a1,-0x4BC4(gp)` | Loads write counter before path A |
| `0x00166D38` | `sh a0,0(v0)` | Stores `(row << 5) + col` for path A |
| `0x00166D3C` | `sw a1,-0x4BC4(gp)` | Increments counter after path A |
| `0x00166D74` | `lw a1,-0x4BC4(gp)` | Loads write counter before path B |
| `0x00166D94` | `sh a0,0(v0)` | Stores `(row << 5) + col` for path B |
| `0x00166D98` | `sw a1,-0x4BC4(gp)` | Increments counter after path B |
| `0x00166DFC` | `sh v0,-0x4F80(a0)` | Fast path single-cell store to `0x006AB080[0]` |
| `0x00166E00` | `sw v1,-0x4BC4(gp)` | Sets counter to 1 after fast path |

The function is a line/segment rasterizer over a bounded 32x32 grid. The writer
encodes a cell as:

```txt
halfword = (row << 5) + col
```

### Caller in `0x00166E10`

The dispatcher calls the writer immediately before the callback `jalr`:

| Address | Instruction | Finding |
|---------|-------------|---------|
| `0x0016700C` | `jal 0x00166BB0` | Builds halfword table for current context |
| `0x00167010` | `move a0,s1` | Passes current context to writer |
| `0x00167014` | `lw v1,0xC0(sp)` | Return address observed in runtime logs |
| `0x00167020` | `jalr v1` | Calls selected slot callback |
| `0x00167024` | `move a2,s0` | Passes loop index/variant to callback |

This places the halfword rebuild in the hot dispatch path immediately before
callback execution, not only in scene loading.

### Direct callers of `0x00166BB0`

A raw `.text` scan for direct `jal 0x00166BB0` found two static callsites:

| Callsite | Return address | Runtime status in Rev.093b | Context |
|----------|----------------|----------------------------|---------|
| `0x0016700C` | `0x00167014` | 20,153,859 entry hits | Hot dispatch path before slot callback |
| `0x0016828C` | `0x00168294` | 0 observed entry hits | Later dispatch/helper path, unhit in this session |

The Rev.093b log confirms all observed entry hits returned to `0x00167014`.

---

## Hypotheses Checked

| Hypothesis from Rev.093 | Rev.093b result | Status |
|-------------------------|-----------------|--------|
| The writer is dead/disconnected | Entry and write-path probes fire millions of times | Discarded |
| The writer is scene-load only | Caller is in `0x00166E10` hot dispatch path before callback `jalr` | Discarded |
| Probes at the store path missed activation | Entry probe plus writer setup probes captured activation | Confirmed as prior coverage issue |
| Early exit commonly leaves counter at zero | Many entries have no following write and counter is reset every call | Probable |
| The table encodes 32x32 cells | Runtime rows/columns are in range; stores encode `(row << 5) + col` | Confirmed for observed session |
| The fast path matters | 2.63M invocations ended with counter 1 and no observed A/B write | Probable, needs direct PC probe |

---

## What Is Confirmed

1. `0x00166BB0` is reached heavily at runtime in the Rev.093b session.
2. The writer paths are reached: `0x00166D1C` 3,921,188 times and
   `0x00166D78` 677,707 times.
3. The previous "zero writer hits" result was a probe/session coverage issue,
   not evidence that the writer is dead.
4. The counter is reset inside the writer at `0x00166BDC` and then incremented
   at the store paths.
5. `0x00166BB0` is called from `0x0016700C`, directly before the selected slot
   callback at `0x00167020`.
6. Runtime rows/columns observed in this session are valid 32x32 grid cells.
7. A second direct static caller exists at `0x0016828C`, but was not observed in
   this runtime log.

---

## What Is Probable

- The halfword table is rebuilt per callback-context candidate, not globally once
  per frame.
- The counter captured at entry is useful as a previous-call residue / previous
  candidate result, not as the current call's final count.
- The common runtime pattern is a short segment producing 0-2 cells, with rare
  longer segments producing up to 5 cells in this session.
- The single-cell fast path at `0x00166DFC` is active and accounts for the
  `counter=1` / zero-A-B-write invocations.

---

## What Is Possible

- The writer may cover larger row/column ranges in other rooms or camera/entity
  layouts; this session only observed row `0..17` and column `0..11`.
- The unobserved static caller at `0x0016828C` may represent a less common
  dispatch/helper path that requires a different slot, mode, or entity context.
- `world_state_raw` values `0x13..0x15` may correlate with the tested runtime
  segment, but this capture does not yet tie them to specific rooms.

---

## What Is Unknown

- The exact semantic meaning of the reference/origin pointer loaded via
  `[gp-0x4BC0] + 0x20`.
- Whether every slot callback consumes the freshly rebuilt table in the same way
  under all modes.
- The room/entity condition that made prior sessions miss the write paths.
- Direct hit count for the fast path `0x00166DFC`.
- Why `room_init_fn` remains `0x00000000` in the current world-state probe.

---

## What Is Discarded

| Discarded idea | Reason |
|----------------|--------|
| `0x00166BB0` is dead code | 20.15M entry hits in Rev.093b |
| Halfword writers never run | 4.59M setup hits for write paths A/B |
| The table is scene-load only | Caller is in hot dispatcher path directly before callback `jalr` |
| Counter staying zero means no writer exists | Counter is reset every invocation and often remains zero when the candidate rasterizes no valid cell |

---

## Next Minimum Test

Add two narrow probes:

| Probe | Address | Purpose |
|-------|---------|---------|
| `halfword_fast_path` | `0x00166DFC` | Directly verify the 2.63M inferred single-cell cases |
| `halfword_exit` | immediately after callback return or after `0x00166BB0` returns at `0x00167014` | Capture final `gp-0x4BC4` per invocation |
| `halfword_second_caller` | `0x0016828C`/`0x00168294` | Determine when the second direct caller is used |

For the world-state side, move or enrich the `world_state_load` probe so it
captures the real room/init context instead of repeated `room_init_fn=0`.

---

## Conservative Verdict

Rev.093b resolves the halfword writer activation question. The writer is active
in the hot dispatch path, rebuilds a small 32x32 spatial cell list for the
current dispatch context, and stores that list in `0x006AB080` before the
selected callback consumes it. The exact semantic owner of the coordinate frame
and the room/entity reason for prior zero-hit sessions remain open.
