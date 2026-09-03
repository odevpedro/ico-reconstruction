# Rev.112 — Native bridge: world_state→slot dispatch binding (from Rev.111 runtime)

**Date:** 2026-09-03

---

## Objective

Convert the Rev.111 runtime discovery (each world_state has one dominant DL
dispatch slot) into a semantic, auditable native-engine data contract, following
the established pattern of `rev109-isysgobj-abi-consolidation` (ABI header +
semantic bridge + CTest coverage).

## Scope

- Add a header-only `WorldStateSlotMap` to the native engine that documents the
  per-world_state slot binding.
- Add a CTest that asserts the confirmed bindings and the zone-sharing groups.
- Do NOT claim byte-exactness; the mapping is derived from measured runtime
  percentages, not from a recovered `.data` table.

## Sources

- Rev.111 research note (runtime capture `ico-runtime-20260825-152452.jsonl`)
- `native/src/engine/IsysGObjRuntime.{h,cpp}` — existing slot dispatch model
- `native/src/engine/GObj.h` — GObj/ProcessNode aliases and constants
- `native/tests/slot_dispatch_test.cpp` — existing test style

## Evidence

### Confirmed (from Rev.111)

Primary slot per world_state (frequency >= 86%):

| ws | slotIndex (a2) | bssAddress | shareGroup | freq |
|----|----------------|------------|-----------|------|
| 0x05 | 0x18 | 0x00677dd8 | 1 | 91.4% |
| 0x0F | 0x1A | 0x006782f8 | 2 | 100% |
| 0x09 | 0x1A | 0x006782f8 | 2 | 97.8% |
| 0x0E | 0x1A | 0x006782f8 | 2 | 98.0% |
| 0x0D | 0x1C | 0x00678818 | 3 | 98.7% |
| 0x06 | 0x1E | 0x00678d38 | 4 | 98.1% |
| 0x07 | 0x1F | 0x00678fc8 | 5 | 98.5% |
| 0x08 | 0x20 | 0x00679258 | 6 | 98.8% |
| 0x0A | 0x21 | 0x006794e8 | 7 | 97.6% |
| 0x03 | 0x21 | 0x006794e8 | 7 | 97.1% |
| 0x2B | 0x21 | 0x006794e8 | 7 | 94.6% |
| 0x01 | 0x22 | 0x00679778 | 8 | 92.7% |
| 0x04 | 0x33 | 0x0067c308 | 9 | 99.2% |
| 0x2D | 0x40 | 0x0067e458 | 10 | 86.3% |
| 0x28 | 0x29 | 0x0067a968 | 11 | 56.6% |
| 0x0B | 0x44 | 0x0067ee98 | 12 | 99.2% |

### Zone sharing

- Slot `0x1A` serves world_states `0x0F, 0x09, 0x0E` → shareGroup 2.
- Slot `0x21` serves world_states `0x0A, 0x03, 0x2B` → shareGroup 7.
- Zones with a single mapped world_state get a unique shareGroup.

## What is confirmed

- The per-world_state slot index and its BSS dispatch-slot address are stable
  runtime observations (Rev.111).
- Several world_states share the same slot index → the slot is a zone-level
  dispatch resource, not strictly 1:1 with world_state.

## What is probable

- The slot index maps to a per-list dispatch bit / list id in the native
  `IsysGObjRuntime` model (via `activeMask`).
- The shareGroup field is an interpretation introduced by this revision, not an
  observed field.

## What is unknown

- The exact `.data`/`.bss` table that the original binary uses to derive the
  slot from a world_state.
- World_states 0x00, 0x02, 0x0C — no confirmed dominant slot; treated as unmapped.

## What is discarded

- The notion that the slot distribution is uniform across rooms (Rev.111 showed
  per-room dominant slots).

## Next minimum test

- Wire `WorldStateSlotMap` into the scene loader (`KanbanSceneLoader`) so that a
  scene request selects the active dispatch slot before GObj creation, then
  extend `scene_loader_test`.

## Conservative verdict

The world_state→slot binding is a confirmed runtime data contract. The new
`WorldStateSlotMap` header documents it as a semantic, auditable constant table
with CTest coverage asserting the confirmed bindings and zone groups. It is not
byte-exact and does not claim to recover the original table format.

## Files

- `native/src/engine/WorldStateSlotMap.h` (new)
- `native/tests/world_state_slot_test.cpp` (new)
- `native/CMakeLists.txt` (register `world_state_slot_test`)
