# Rev.109 — isysGObj ABI consolidation

**Date:** 2026-08-26
**Objective:** Consolidate an auditable 32-bit ABI for the verified
`isysGObj*` core and resolve the apparent 8-list versus 32-slot divergence.
**Scope:** Static evidence only. No PCSX2 session or new runtime log was used.

## Sources used

- `src/core/asm/isysGObjAlloc.s`
- `src/core/asm/isysGObjAdd.s`
- `src/core/asm/isysGObjAddHead.s`
- `src/core/asm/isysGObjAddAfterGObj.s`
- `src/core/asm/isysGObjAddBeforeGObj.s`
- `src/core/asm/isysGObjInit.s`
- `src/core/asm/isysGObjRemove.s`
- `src/core/asm/isysGObjRemoveAll.s`
- `src/core/asm/isysGObjDlInit.s`
- `src/core/asm/isysGObjProcessAlloc.s`
- `src/core/asm/isysGObjProcAdd_.s`
- `src/core/asm/iosOmCreateDL.s`
- `src/core/asm/iosOmExeEachGObj.s`
- Rev.098–100 notes, used as orientation and checked against the assembly above

## Evidence and ABI findings

`isysGObjAlloc` multiplies the requested count by `0x174` and advances its
initialization cursor by `0x174`. `isysGObjProcessAlloc` and
`isysGObjProcAdd_` do the equivalent operation with `0x94`. These instruction
patterns directly establish the two ABI sizes.

The new canonical header, `src/core/gobj_abi.h`, uses only fixed-width integer
members. Original EE pointers are represented by `ico_ptr32`, never by a host
pointer. `_Static_assert`/`static_assert` checks cover both total sizes and all
offsets named as confirmed.

One important naming caveat remains at `GObj+0x28`: `isysGObjAdd` stores its
first argument there, while `_iosOmMain` later executes that word with `jalr`.
The field is retained as `user_data` to preserve the established API-facing
name, but its comment records the directly observed callback use. The exact
source-level type is therefore not claimed as settled.

## The four adjacent tables

| Address | Entry count | Entry size | Role | Evidence | Confidence |
|---|---:|---:|---|---|---|
| `0x281A70` | 8 | 4 | Primary GObj-list heads | `isysGObjInit` clears exactly 8 words; insertion/unlink code indexes it by `GObj+0x18`; `_iosOmMain` and `iosOmExeEachGObj` load heads from it | CONFIRMED |
| `0x281A90` | 8 | 4 | Primary GObj-list tails | `isysGObjInit` clears exactly 8 words; ordered insert and unlink repair tails here; starts exactly 32 bytes after `0x281A70` | CONFIRMED |
| `0x281AB0` | 8 | 4 | DL GObj-list heads | `isysGObjDlInit` clears exactly 8 words; helpers around `0x13EE60`/`0x13EFD0` index it and link through `GObj+0x34/+0x38`; starts after the primary-tail array | CONFIRMED |
| `0x281AD0` | 8 | 4 | DL GObj-list tails | `isysGObjDlInit` clears exactly 8 words; the same helpers update tail entries; starts exactly 32 bytes after `0x281AB0` | CONFIRMED |

## Resolution of 8 entries versus 32 bits

The previous “32-entry table at `0x281AB0`” reading combined two different
facts:

1. `isysGObjDlInit` physically initializes **8 head pointers** at `0x281AB0`
   and **8 tail pointers** at `0x281AD0`.
2. `iosOmCreateDL` scans **32 bits** of the word at `gp-0x6724`.

The second fact does not establish 32 physical head entries. The loop tests a
mask bit before it calculates and dereferences `0x281AB0 + index * 4`. Since
`0x281AD0` is independently proven to be the tail array, treating
`0x281AB0[8]` as another head would alias `0x281AD0[0]` and violate the link
helpers' layout.

Conservative model:

- the physical primary and DL list tables each contain 8 heads and 8 tails;
- the control mask and its scan are 32 bits wide;
- valid DL list indices are `0..7` in the reconstructed model;
- bits `8..31` must remain zero before `iosOmCreateDL` dereferences the table.

The first two bullets are **CONFIRMED** by instructions. The last two are a
**PROBABLE invariant**: the original code has no local bounds check in the
bit-set helper or in the link helpers, so static evidence alone cannot prove
that no external caller ever violates it. A high bit would cause the original
loop to read the adjacent tail/unknown region as if it were a head pointer.

## Confirmed offsets

### `IcoGObj` (`0x174` bytes)

| Offset | Field | Evidence |
|---:|---|---|
| `0x00` | `self` | free-slot scans test zero; add stores self |
| `0x0C` | `type` | kind-table add/remove selector |
| `0x10` / `0x14` | `next` / `prev` | primary-list insertion and unlink |
| `0x18` | `list_id` | indexes `0x281A70/0x281A90` |
| `0x1C` | `sort_key` | ordered insertion comparisons |
| `0x28` | `user_data` (source type unresolved) | stored by add; executed by `_iosOmMain` |
| `0x2C` / `0x30` | process head / tail | `isysGObjProcAdd_` insertion |
| `0x34` / `0x38` | DL next / previous | DL helpers and `iosOmCreateDL` traversal |
| `0x3C` | type-chain next | kind-table add/remove |
| `0x40` / `0x44` | DL list id / sort key | DL helpers |
| `0x48` | `callback` | DL link helpers and `iosOmCreateDL` `jalr` |
| `0x4C` | `slot_mask` | per-object bit test in `iosOmCreateDL` |
| `0x50` | `type_mask` | AND filter in `iosOmCreateDL` |
| `0x15C` | `state_15c` | cleared by alloc and add |
| `0x16C` / `0x170` | dispatch gates | tested by `_iosOmMain`/`iosOmCreateDL` |

### `IcoProcessNode` (`0x94` bytes)

| Offset | Field | Evidence |
|---:|---|---|
| `0x00` | `self` | free-slot scan and allocation marker |
| `0x04` | `parent` | registration stores owning GObj |
| `0x08` / `0x0C` | `next` / `prev` | process-list insertion and traversal |
| `0x10` | `type_mask` | registration and dispatch branch |
| `0x14` | `priority` | ordered insertion and dispatch comparison |
| `0x18` | `active` | dispatch gate |
| `0x1C` | `callback` | callback store and `jalr` |
| `0x24` | embedded TCB area | passed to the `ios/thread.c` routines |

The word at `+0x20` remains `unknown_020`. The `0x70` bytes from `+0x24` to
`+0x93` are preserved as `tcb_area`; no field names inside that area are
promoted by this revision.

## Semantic C reconstruction

`src/core/isysgobj_semantic.c` provides a deliberately small, host-comparable
model for:

- DL table initialization;
- contiguous pool initialization;
- first-free allocation;
- ordered insertion and head insertion;
- primary-list unlink and slot recycling.

The original stores EE addresses. The semantic model instead uses
`slot_index + 1` handles so the ABI structs remain 32-bit on a 64-bit host and
zero still means null/free. This is an explicit portability inference, not a
claim about original bytes. Attached-process teardown and the separate
kind-table chain are outside this minimum semantic core.

## Confidence classification

### CONFIRMED

- `IcoGObj` stride/size is `0x174`.
- `IcoProcessNode` pool stride/size is `0x94`.
- all offsets guarded by assertions in `gobj_abi.h` are instruction-backed.
- each of the four table addresses begins an 8-word array with the role shown.
- `iosOmCreateDL` scans a 32-bit mask and conditionally indexes DL heads.

### PROBABLE

- bits 8–31 are intentionally required to remain zero.
- the original source module for all small GObj routines was `gobj.c`; the
  allocator's class string confirms this for allocation, while adjacent
  routines are assigned by module continuity.

### POSSIBLE

- `GObj+0x28` had a source-level union or context-dependent typedef explaining
  its “user data” and direct-call uses.

### UNKNOWN

- source-level names for padding and for most of `GObj+0x5C..+0x15B`;
- internal layout of `IcoProcessNode.tcb_area`;
- whether defensive validation existed only at higher-level callers.

### Discarded

- **Discarded:** `0x281AB0` is a standalone physical array of 32 head pointers.
  It conflicts with the confirmed tail array beginning at `0x281AD0`.
- **Discarded:** the 32-iteration loop proves 32 DL list objects. It proves only
  the width of the scanned mask and loop bound.

## Next minimum test

Compile the ABI header as both C11 and C++ and exercise the portable pool/list
model with empty, full, remove and reuse cases. After merging into
`native-port`, make the native runtime consume the same ABI definitions and
run those cases through CTest.

## Conservative verdict

The ABI sizes and the two 8-head/8-tail table pairs are sufficiently supported
for a portable core. The 32-bit loop is retained as a control-mask property,
not promoted into a 32-entry storage claim. No new runtime capture is required
for this bridge step.
