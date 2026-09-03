# Rev.113 — isysGObj* semantic core: primary-list, kind-table and DL-link functions (.s → C)

**Date:** 2026-09-03

---

## Objective

Extend the canonical `IcoGObj`/`IcoProcessNode` ABI (Rev.109) semantic model
from the five base operations (`isysGObjDlInit`, `isysGObjAlloc`, `isysGObjAdd`,
`isysGObjAddHead`, `isysGObjRemove`) to the primary-list, type-kind and display
list link family that was still `.s`-only. This is decompilation truth work on
`master`, not engine work.

## Scope

- Reconstruct, as semantic C in `src/core/isysgobj_semantic.c`:
  `isysGObjAddAfterGObj`, `isysGObjAddBeforeGObj`, `isysGObjMove`,
  `isysGObjMoveBeforeGObj`, `isysGObjMoveAfterGObj`, `isysGObjKindTableAdd`,
  `isysGObjKindTableRemove`, `isysGObjLinkObjDL`, `isysGObjLinkCameraDL`,
  `isysGObjActiveLink`, plus a full rewrite of `isysGObjRemove`.
- Extend `src/core/gobj_abi.h` with the type-kind table
  (`0x6A93D0`, 0x44 heads) and the `kind_table_disabled` (gp-0x6730) flag.
- Do NOT claim byte-exactness. Each function documents confirmed (`.s`) fields
  versus inferred host-model behavior.

## Sources

- `src/core/asm/*.s` — byte-exact ground truth (one per function; several files
  also carry tail-merged adjacent bodies, noted where relevant).
- `src/core/gobj_abi.h` — canonical ABI (Rev.109).
- `src/core/isysgobj_semantic.c` — existing semantic pattern.
- `src/core/isys_process.h` — earlier structural notes (GP offsets, table VAs).

## Evidence

### Confirmed from the `.s` ground truth

| Function | .s ground truth | Confirmed behavior |
|----------|-----------------|--------------------|
| `isysGObjAddAfterGObj` | `isysGObjAddAfterGObj.s` | allocate first free slot; set user +0x28, self +0x00, +0x04/+0x08=-1, +0x15C/+0x2C/+0x30/+0x58=0; insert immediately after `ref` copying list_id/sort_key; repair tail |
| `isysGObjMoveBeforeGObj` | `isysGObjMoveBeforeGObj.s` | unlink primary (`0x0013DDF8`), then link before `ref` (list_id/sort_key copied, head repair) |
| `isysGObjMoveAfterGObj` | `isysGObjMoveAfterGObj.s` | same, but link after `ref` (tail repair) |
| `isysGObjMove` | `isysGObjMove.s` | thin: unlink `0x0013DDF8` + ordered insert `0x0013DFF0` (a tail-merged variant targets head insert `0x0013E0C0`) |
| `isysGObjKindTableAdd` | `isysGObjKindTableAdd.s` | if gp-0x6730 set: only write type +0x0C; else scan current type chain, unlink if present, store new type, and for type<0x44 append to `0x6A93D0[type]` |
| `isysGObjKindTableRemove` | `isysGObjKindTableRemove.s` | unlink from `0x6A93D0` chain for current type |
| `isysGObjLinkObjDL` | `isysGObjLinkObjDL.s` | store callback +0x48, type_bits +0x50; call DL ordered insert `0x0013EE60` |
| `isysGObjLinkCameraDL` | `isysGObjLinkCameraDL.s` | store callback +0x48, slot_mask +0x4C, type_bits +0x50; call camera-DL helper `0x001FC048` |
| `isysGObjActiveLink` | `isysGObjActiveLink.s` | empty stub: single `jr $31` |
| `isysGObjRemove` | `isysGObjRemove.s` | remove kind-table (type in [1,0x44)); unlink primary; self=0; walk process list +0x2C via `0x0013F6B8` |

### Internal helpers referenced by the `.s` (no standalone `.s`)

| VA | Role | Modeled as |
|----|------|------------|
| `0x0013DDF8` | primary-list unlink | `unlink_primary()` |
| `0x0013DFF0` | primary ordered insert | `insert_primary_sorted()` (pre-existing) |
| `0x0013E0C0` | primary head insert | `insert_primary_head()` (pre-existing) |
| `0x0013EB50` | kind-table head get | inline scan in `ico_semantic_isysGObjKindTableAdd` |
| `0x0013EBE0` | kind-table next get | inline scan |
| `0x0013E728` | kind-table unlink | `kind_table_remove()` |
| `0x0013EE60` | DL ordered insert | `insert_dl_sorted()` (fields confirmed; not byte-exact) |
| `0x0013F6B8` | process removal loop | represented as loop; internals out of scope |

## Implementation notes

- `IcoGObjSemanticPool` gained `kind_heads[0x44]` and `kind_table_disabled`
  (mirrors gp-0x6730).
- Handles remain slot_index+1 throughout (original ABI null = 0).
- `unlink_primary()` does NOT clear `self` (the caller does), matching
  `0x0013DDF8` as used by all callers.

## Verification

- `gcc -std=c99/-std=c11 -Wall -Wextra -Wpedantic -c` clean (no warnings).
- A standalone driver exercised: ordered add, MoveBefore/MoveAfter/Move,
  AddAfterGObj, kind-table add/remove, and Remove recycling. All assertions
  passed; the full list order was validated at each step via a dump helper.

## Confirmed vs inferred

- **Confirmed:** field offsets, table VAs, insertion/link/unlink field logic
  shown directly in the `.s`, kind-table gating on gp-0x6730, process-teardown
  call at `0x0013F6B8`.
- **Inferred (host model):** handles in place of EE pointers; the exact walk
  order in `kind_table_add` (kept as the inner loop); `insert_dl_sorted` as the
  target of `0x0013EE60` (field set confirmed, byte-exactness not claimed);
  camera-DL helper internals omitted.

## Unknown

- The exact contents of `isysGObjProcAdd_`/`isysGObjProcessAlloc` and the full
  `_iosOmMain`/`iosOmCreateDL` dispatch bodies (already structurally noted in
  `isys_process.c`, not byte-exact; reserved for a focused pass).

## Discarded

- Do not re-derive byte-exact asm from these semantic functions — the `.s`
  remains the ground truth.

## Next minimum test

- Link the semantic core into the native CTest suite (on `native-port`) and
  assert the same list invariants exercised by the standalone driver.

## Conservative verdict

Rev.113 converts 10 additional isysGObj*/iosOm* functions to auditable semantic
C on top of the canonical ABI, verified against byte-exact `.s`, with confirmed
fields separated from inferred host behavior. The `.s` files remain ground
truth.
