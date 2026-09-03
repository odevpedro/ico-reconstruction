# Rev.114 — Faithful `_iosOmMain` dispatch gate on the native engine (native-port)

**Date:** 2026-09-03

---

## Objective

Convert `_iosOmMain` (0x0013F9D0) into true semantic C on the native engine,
using the field/table structure confirmed in Rev.113, instead of the older
ungated dispatch model.

## Scope

- `native/src/engine/IsysGObjRuntime.cpp` only:
  - `dispatchActiveLists()` = **Pass 1** of `_iosOmMain`
  - `dispatchTypeSlots()` = **Pass 2** of `_iosOmMain`
- Update the two dispatch unit tests to enable the confirmed gates.
- Do NOT touch `dispatchList`/`dispatchProcesses` (generic single-list / single
  process helpers, not part of `_iosOmMain`).

## Sources

- `src/core/asm/_iosOmMain.s` — byte-exact ground truth (0x0013F9D0).
- `research/elf/ghidra-rev113-isysgobj-semantic-list-kind-link.md` — confirmed
  dispatch-core structure.
- `native/src/engine/IsysGObjRuntime.{h,cpp}` — existing dispatch model.

## Confirmed structure being implemented (from the .s)

Both passes iterate the mask (`gp-0x6724` → native `m_activeMask`) over primary
lists and, **for each GObj, skip it unless both `+0x16C` and `+0x170` are
non-zero**:

| Pass | Walk | Callback fired | Gate |
|------|------|----------------|------|
| 1 (`dispatchActiveLists`) | primary list via `+0x10` | per-GObj callback (orig. `+0x28` user_data, a0=GObj) | `state_16c && state_170` |
| 2 (`dispatchTypeSlots`) | process chain via `+0x2C` | process callback (orig. `+0x1C`, a0=GObj) | `state_16c && state_170`; process `priority==typeSlot(0x13..0x1B)` && `active(+0x18)!=0` |

## Change

- `dispatchActiveLists`: replaced the bare `dispatchList(bit)` delegation (no
  gate) with a gated in-place walk that mirrors Pass 1.
- `dispatchTypeSlots`: added `state_170` to the existing `state_16c` gate.
- Tests: `set state_170 = 1` wherever a GObj is expected to dispatch, matching
  the confirmed ABI (a default-allocated GObj has `state_170 = 0` and is
  therefore dispatch-inert).

## Confirmed vs inferred

- **Confirmed:** two-gate (`+0x16C && +0x170`) requirement for both passes;
  pass-1 callback is the GObj `+0x28` value jalr'd with a0=GObj; pass-2 uses
  process `+0x14` as the type selector and `+0x18` for active.
- **Inferred (host model):** pass-1 callback is the `setCallback()` host entry
  rather than a raw `+0x28` function pointer (the native pool stores host
  callbacks separately and keeps ABI fields 32-bit).
  The pass-2 "thread" path (process `+0x10 == 0`, `ios/thread.c`, helpers
  `0x13D8A0`/`0x13D928`/`0x13F6B8`) remains reserved and unmodeled here.

## Verification

- Rebuilt `native/build` with `make` (no errors).
- `ctest` : **13/13 passed**, including `slot_dispatch` and `isysgobj_runtime`
  with the updated gates.

## Unknown

- The pass-2 thread path internals (process with type_mask==0) — lives in
  `ios/thread.c` and is not part of this conversion.

## Discarded

- Do not gate `dispatchList`/`dispatchProcesses` (generic helpers) — they model
  `iosOmExeEachGObj`-style single traversal, which the .s does not gate at the
  future host-callback boundary.

## Next minimum test

Extend `slot_dispatch_test` to exercise the pass-2 thread path once `ios/thread`
process scheduling is recovered, or add a test that a GObj with only one of the
two gates set does not dispatch.

## Conservative verdict

Rev.114 makes the native `_iosOmMain` implementation faithful to the confirmed
byte-exact `.s` two-gate behavior on both passes, with all existing tests
passing. The thread/`ios` path remains explicitly reserved.
