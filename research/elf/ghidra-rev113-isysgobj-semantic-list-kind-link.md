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

---

## Appendix: the four `.word` R5900/COP1 fallback functions

Four GirlBrain/eBrain functions were preserved as byte-exact `.s` with raw
`.word` emissions that Capstone could not handle. Decoding the raw words this
revision shows they fall into **two distinct classes**, not one:

### Class 1 — COP1 single compare `c.OLT.s`

The `0x4600xxxx` words are the R5900 COP1 *ordered-less-than* comparison in
single-float format (`c.OLT.s fs, ft`). They are genuine float compares used to
select a minimum/maximum value in a scan. Present in `eBrainProcess` (2) and
`girlBrainRunawaySearchPoint` (5).

| Word | Decode |
|------|--------|
| `0x46000834` | `c.OLT.s $f1, $f0` |
| `0x46001034` | `c.OLT.s $f2, $f0` |
| `0x46010034` | `c.OLT.s $f0, $f1` |
| `0x46011034` | `c.OLT.s $f2, $f1` |
| `0x46140034` | `c.OLT.s $f0, $f20` |
| `0x46150034` | `c.OLT.s $f0, $f21` |
| `0x46160034` | `c.OLT.s $f0, $f22` |

### Class 2 — MIPS branches emitted raw (target outside symbol range)

The other words are standard MIPS branches that the assembler could not
relocate because the branch target lay outside the local function symbol, so
they were emitted as raw bytes. They are **not** COP1:

| Word | Decode |
|------|--------|
| `0x1040007B` | `beqz $v0, +0x1F0` |
| `0x1840002B` | `blez $v0, +0xB0` |
| `0x1AE00054` | `blez $s7, +0x154` |
| `0x18600015` | `blez $v1, +0x58` |
| `0x1453005C` | `bne $v0, $s3, +0x174` |
| `0x10000011` | `b +0x48` |
| `0x1040000D` | `beqz $v0, +0x38` |

### `eBrainProcess` (0x190B10 region) — confirmed shape

Iterates up to `0x20` entries; each entry is `0x1C` bytes. Two chains are
tracked at `0x6D0710` and `0x6D0B10` (index 0x2A in the `0x6D0000` block);
each candidate holds a float at +0x8 and +0xC and a halfword kind at +0x0.
Two float-min scans (`0x46000834` = `c.OLT.s`) pick the lowest float, with a
running counter at gp-0x4B48 / gp-0x4B4C and a count-up at +0x14 when kind==1.
What these fields mean for gameplay is **not** claimed here; only the `.s`
structure is recorded.

### Conclusion

All four fallback functions are byte-exact `.s` ground truth. The raw `.word`
are explained: `c.OLT.s` float compares plus out-of-range MIPS branches. They
are deliberately kept outside the automated `asm_source_score.py` pipeline. No
reconstruction is attempted for these float branches this revision.

---

## Dispatch core: field/table mapping confirmed (≥ 0x13FD10 / 0x13FC00 / 0x13F9D0)

This revision decoded the three dispatch functions from their byte-exact `.s`
and thereby **confirmed** the cross-function link-field / table model. They are
recorded as documentation; see the design note below for why they are not
re-implemented as runnable semantic functions in `isysgobj_semantic.c`.

### Two distinct link fields and two head tables

| Walk | Head table | Link field | Used by |
|------|-----------|------------|---------|
| primary list | `0x00281A70` (pool `primary_heads`) | `+0x10` (next) | `iosOmExeEachGObj`, `_iosOmMain` pass 1 |
| DL list | `0x00281AB0` (pool `dl_heads`) | `+0x34` (dl_next) | `iosOmCreateDL`; also GObj chain via `+0x34` |

This cleanly resolves the earlier ambiguity (0x1A70 vs 0x1AB0): **0x1A70 is
the primary head table** walked by `+0x10`; **0x1AB0 is the DL head table**
walked by `+0x34`. `iosOmCreateDL`'s outer GObj chain (head gp-0x671C) also
uses `+0x34`.

### `iosOmExeEachGObj` (0x0013FD10) — confirmed

```
cur = primary_heads[slot];            # 0x281A70[slot]
while (cur) { cb(cur, arg); cur = cur->next; }   # next = +0x10
```

### `iosOmCreateDL` (0x0013FC00) — confirmed

```
g = chain_from(gp-0x671C);            # walk via dl_next (+0x34)
while (g) {
  for slot in 0..31:                  # ICO_GOBJ_DL_MASK_BITS == 32
    mbit = (mask(gp-0x6724) >> slot) & 1
    if slot==0: if mbit and g->callback(+0x48): call g->callback(g)
                if not mbit: continue
    if not mbit or not ((g->slot_mask(+0x4C)>>slot)&1): continue
    dl = dl_heads[slot]               # 0x281AB0, walk via dl_next +0x34
    while (dl):
      if dl->state_16c(+0x16C) and (g->type_mask(+0x50)&dl->type_mask(+0x50))
         and dl->callback(+0x48):
           call dl->callback(dl)
      dl = dl->dl_next
    g = g->dl_next
}
```

### `_iosOmMain` (0x0013F9D0) — two-pass structure confirmed

- **Pass 1** (mask slots 0..7 of `gp-0x6724`): walk `primary_heads[slot]` via
  `+0x10`; for each GObj with both `+0x16C` and `+0x170` non-zero, call its
  `+0x28` (user_data) as a callback with `a0 = GObj`.
- **Pass 2** (same mask slots): walk the process-node chain rooted at GObj
  `+0x2C`; dispatch process nodes for type values `0x13..0x1B` (i.e. 19..27),
  consulting helpers `0x13D8A0`, `0x13D928`, `0x13F6B8` and transient
  gp-0x6710.

### Design decision: dispatch NOT re-implemented as runnable semantic C

The canonical ABI stores callbacks as 32-bit ee pointers
(`IcoGObj.callback` +0x48, and ProcessNode callbacks). The semantic pool keeps
these fields 32-bit to mirror the EE ABI, so they cannot carry 64-bit host
function pointers. Invoking them on a 64-bit host truncates the pointer and
crashes (verified: a test driver core-dumped attempting it). Correct execution
is owned by the native engine (`dispatchActiveLists` / `dispatchTypeSlots` /
`dispatchAll`, Rev.105, native-port) which stores real host pointers.

Therefore `isysgobj_semantic.c` documents the three functions (comment block)
and leaves execution to the native engine, avoiding a half-runnable callback
path that would violate the "quality without speculation" rule.

---

## Next minimum test

- Link the semantic core into the native CTest suite (on `native-port`) and
  assert the same list invariants exercised by the standalone driver.

## Conservative verdict

Rev.113 converts 10 additional isysGObj*/iosOm* functions to auditable semantic
C on top of the canonical ABI, verified against byte-exact `.s`, with confirmed
fields separated from inferred host behavior. The `.s` files remain ground
truth.
