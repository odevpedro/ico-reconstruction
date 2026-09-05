# Rev.131 — `world_state_load` boundary correction + `DispIcoMisc` byte-exact, and the native `WorldStateLoader` semantic bridge

**Date:** 2026-09-05
**Branch:** native-port

## Objective

1. Separate the two functions that Rev.130 accidentally merged into one
   `world_state_load.s` of size 0x248.
2. Add the missing second function as its own byte-exact `.s`.
3. Add a host-side semantic reconstruction of `world_state_load` (0x001AF948)
   on the native-port as the room-dispatch bridge, with CTest coverage.

## Scope

- Ground truth split of 0x1AF948–0x1AFB8F.
- New `src/core/asm/DispIcoMisc.s`.
- Corrected `src/core/asm/world_state_load.s` (0x80).
- Native semantic bridge `native/src/engine/WorldStateLoader.{h,cpp}` +
  `native/tests/world_state_loader_test.cpp` + CMake wiring.

## Sources used

- `.local/extracted/SCUS_971.13.elf` (USA ELF, little-endian).
- `src/core/asm/world_state_load.s` (Rev.130, before this split).
- `research/elf/rev130-hot-gaps-3-4-5-byte-exact.md`.
- `research/ghidra-exploration-2026-05-21.md` (line 1541: `0x001AF9C8` = `DispIcoMisc`).
- `docs/symbols/usa_fingerprints.json` (0x1AF9C8 entry, size 0x98 — shown below to
  be a scanner artifact, not a function boundary).
- `research/pal-usa/pal_usa_function_map_candidates.csv` (line 1333:
  `0x001AFB58` = `la_save_game_memory_card_check` — an internal offset of this
  block, not a function start; `NO_USA_FP`).

## Evidence used

- Instruction-level disassembly (little-endian MIPS64) of the 0x1AF948 region.
- `assemble_and_verify` on both split functions.
- Full scoring pipeline run (612/612, 0 failures).
- CTest execution (18/18 pass).

## Findings

### 1. The 0x248 block is two functions

Rev.130 declared `world_state_load` = 0x1AF948, 0x248 bytes. That block runs to
0x1AFB8F but contains **two functions**:

| Function | VA range | Size | Prologue | Epilogue |
|----------|----------|------|----------|----------|
| `world_state_load` | 0x1AF948–0x1AF9C7 | **0x80** | (frame 0xB0, set up by caller) | tail-jump `j 0x13d3f8` + `addiu $sp,$sp,0xb0` |
| `DispIcoMisc` | 0x1AF9C8–0x1AFB8F | **0x1C8** | `addiu $sp,$sp,-0x90` + `sd $ra,0x60` + 4 `swc1` f20–f23 | `jr $ra` + `addiu $sp,$sp,0x90` |

The first function ends with a **tail-jump** to the shared epilogue at
`0x13D3F8` (`iosThreadDestroy`). Its own `jr $ra` never appears in the .text —
the return path is the shared tail. The `jr $ra` at 0x1AFB84 and its delay
belong to `DispIcoMisc`.

The `docs/symbols/usa_fingerprints.json` entry at `0x1AF9C8` with size 0x98 and
38 instructions is the reconciliation scanner's first-scan cut (prologue
detection) and must not be treated as a true function start/size here.

### 2. `world_state_load` (0x80) — confirmed dispatcher shape

```
lw    $v1, -0x6f60($gp)              ; world_state
addiu $a0, $zero, 0x194              ; stride
mult  $v1, $v1, $a0                  ; state * 0x194
addu  $v0, table_base, $v1           ; 0x5F2FB8 + state*0x194
lw    $v0, 0x154($v0)                ; per-room init_fn
beqz  $v0, +8 ; jalr $v0 (if nonzero)
jal   0x00166028                     ; MakeCollisionDependGObjList
jal   0x001AE3E8                     ; scene apply
sw    $zero, 0x18(+0x274EC0)         ; clear room-load flag
sw    $zero, 0x14(+0x274EC0)         ; clear room-load flag
j     0x13D3F8                       ; shared epilogue
addiu $sp, $sp, 0xb0
```

Table base `0x5F2FB8`, per-entry stride `0x194`, init_fn at entry offset
`+0x154`. Flags cleared at `0x274ED4` and `0x274ED8`.

### 3. `DispIcoMisc` (0x1C8) — byte-exact, newly extracted

- Prologue saves 4 floats (f20–f23) and 4 s-regs; frame 0x90.
- Calls a batch of helper functions: 0x129e28, 0x1ebda0, 0x1e8a60, 0x1cb928,
  0x10a550, 0x1e0978, 0x1686d8, 0x1eba40, 0x104f20, 0x105278, 0x118678,
  0x1118b8, 0x111fa8, 0x1f4be0, 0x1183f0, 0x118388, 0x1d4a58, 0x10f630,
  0x105268, 0x115fb8.
- Contains two nested loops marching x/y from -0xA to +0xA (and y up to +0xA)
  building float grid coords (`mtc1`/`cvt.s.w`/`mul.s`), consistent with a
  display/misc diagnostic grid.
- Named `DispIcoMisc` by the Ghidra symbol map (research/ghidra-exploration-2026-05-21.md:1541),
  where 0x1AF9C8 sits between `ExecIcoMisc` (0x1AF190) and `kanbanReqAdd` (0x1AFB98).

The PAL symbol at `0x001AFB58` (`la_save_game_memory_card_check`,
`layout_action.o`) is an **internal offset** inside `DispIcoMisc` — no USA
fingerprint matches it (`NO_USA_FP`), consistent with the 259-VA collision
audit of Rev.127.

### 4. Native semantic bridge (`WorldStateLoader`)

`native/src/engine/WorldStateLoader.{h,cpp}` reconstructs the dispatch
contract on the host: a `WorldStateDescriptor` table replaces the PS2
`0x5F2FB8`/stride-0x194 layout; `loadWorldState(state)` invokes the room
init_fn (if set), then the two followed host callbacks
(`MakeCollisionDependGObjList` + scene-apply mocks), then clears the two
room-load flag words. Not byte-exact — a semantic boundary.

`native/tests/world_state_loader_test.cpp` covers: reject-null-init, dispatch
of a registered room init_fn, no-op for a state with no init_fn, out-of-range
state (still runs the followed calls, matching the original's unconditional
tail work), static assertions on the documented constants, and post-shutdown
inertness.

### 5. Verification

- `world_state_load` (0x1AF948, 0x80): `assemble_and_verify` → 100%.
- `DispIcoMisc` (0x1AF9C8, 0x1C8): `assemble_and_verify` → 100%.
- Pipeline `asm_source_score.py --all --no-save`: 612/612, 0 failures.
- CTest: 18/18 pass (incl. new `world_state_loader`, `world_state_slot`,
  `scene_loader`, all isysgobj tests).

## What is confirmed

- `world_state_load` is 0x80 bytes and ends in a tail-jump to the shared
  epilogue `0x13D3F8`; it dispatches `table[state*0x194+0x154]` if non-null.
- `DispIcoMisc` (0x1AF9C8–0x1AFB8F, 0x1C8) is a separate function; both are 100%
  byte-exact as `.s`.
- `0x1AFB58` in the PAL map is an internal offset, not a function start.
- Native bridge + test pass on the host.

## What is probable

- `DispIcoMisc` renders a debug/misc grid (float double-loop).
- `0x001AE3E8` is the "scene apply" (`gamesys`-family) call, consistent with
  Rev.130's reading.

## What is unknown

- The exact host semantics of `MakeCollisionDependGObjList` (0x00166028) and the
  scene-apply (0x001AE3E8) — kept as injected mock callbacks.
- The per-room init_fn set actually installed in the `0x5F2FB8` table (needs the
  next runtime session to observe `jalr` targets per world_state).

## What is discarded

- Rev.130's `world_state_load` = 0x248 (merged boundary).
- The `usa_fingerprints.json` size-0x98 cut at 0x1AF9C8 as a function boundary.

## Next minimum test

- Next gameplay session: probe the `jalr` target register at 0x001AF96C to record
  the real per-worth-state init_fn addresses and install them in the native
  `WorldStateLoader` table.

## Conservative verdict

`world_state_load` ground truth is corrected to a clean 0x80 byte-exact `.s`,
`DispIcoMisc` is recovered as a new byte-exact `.s`, and the native port gains
a tested semantic bridge for per-room dispatch. Counts: core/asm = 46,
total `.s` = 710, pipeline 612/612.