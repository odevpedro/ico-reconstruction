# Rev.133 — Hot-gap semantic bridges on the native port (GObjFactory, GObjEntityAllocator)

- **Date:** 2026-09-05
- **Branches:** `main` (ground-truth `.s` unchanged), `native-port` (bridge code + tests)
- **Author of record:** agent session (todo #2 closure)

## Objective

Finish the native-port bridge layer for the three remaining verified hot-gap
functions so their contracts are reproducible on the host:

- `CreateGObj` (USA `0x00240D40`, 0x160) and `CreateGObj_v` (USA `0x00240EA0`,
  0x128) → `GObjFactory`.
- `AllocGObjEntity` (USA `0x0019F310`, 0x850) → `GObjEntityAllocator`.

This note documents the confirmed byte-level contract each bridge implements,
the host design, and the CTest evidence.

## Scope

- Decode the exact argument/register contract of the three `.s` sources.
- Emit `native/src/engine/GObjFactory.{h,cpp}` and
  `native/src/engine/GObjEntityAllocator.{h,cpp}` as semantic bridges.
- Extend `native/tests/isysgobj_semantic_test.c` (unlink + sister) and add
  `gobj_factory_test.cpp` + `gobj_entity_allocator_test.cpp`.
- Wire both into `native/CMakeLists.txt` and run the full suite.

No `.s` file was modified on `main`. The `.s` sources remain byte-exact
ground truth (Rev.128-131 status unchanged).

## Sources used

- `src/core/asm/CreateGObj.s` (byte-exact, Rev.129)
- `src/core/asm/CreateGObj_v.s` (byte-exact, Rev.129)
- `src/core/asm/AllocGObjEntity.s` (byte-exact, Rev.130)
- `src/core/asm/isysGObjKindTableAdd.s`, `src/core/asm/sister_callback_reg.s`,
  `src/core/asm/isysGObjProcRemoveUnlink.s`
- `src/core/isysgobj_semantic.c` / `src/core/gobj_abi.h` (semantic layer)
- `research/elf/rev129-create-gobj-factories-byte-exact.md`,
  `research/elf/rev130-hot-gaps-3-4-5-byte-exact.md`,
  `research/elf/rev131-worldstate-boundary-dispicomisc-and-native-bridge.md`
- `native/src/engine/WorldStateLoader.{h,cpp}` (bridge template)

## Evidence — CreateGObj (0x240D40)

Instruction-level trace from `CreateGObj.s` (all cited at the listed offsets
within the function):

| Step | Instruction evidence (register role) | Meaning |
|------|--------------------------------------|---------|
| trigger gate | `beqz $8,+0x00240d88` then `lw $21,0x40($2)` | `t0` gates the `+0x40` load; trigger = `0` when gate off |
| alloc | `lw $4,0x60($2)`; `move $5,$0`; `move $6,$0`; `jal 0x_13E8D8` | `isysGObjAdd(desc+0x60, 0, 0)` |
| factory words | `sw $7,0xc($20)`; `sw $7,8($20)`; `sw $6,0x16c($20)`; `sw $0,0x164($20)`; `sw $6,4($20)` | `+0x0C=-1, +0x08=-1, +0x16C=1, +0x164=0, +0x04=1` |
| registry | `lw $8,-0x4d58($28)`; base `0x712CC0`; `sw $20,0($3)`; `sw $8,-0x4d58($28)` (sister delay slot) | `table[counter]=gobj; counter++` |
| sisters | 3x `jal 0x_13F778` codes 0x16/0x17/0x18 | `(gobj, cb, 1, type)` per cbA(+0x5C)/cbB(+0x50)/cbC(+0x4C) |
| DL link | `move $5,$19`(+0x48); `move $6,$0`; `move $7,$22`(a3); `ori $8,$8,0xffff`; `jal 0x_13F130` | `LinkObjDL(gobj, +0x48, type_id 0, a3, 0xFFFFFFFF)` |
| wrap | `beqz $21`; `move $5,$21`; `addiu $7,$0,0x13`; `addiu $8,$0,0x1800`; `jal 0x_13F7A8` | if trigger != 0: `(gobj, trigger, 0, 0x13, 0x1800)` |
| tail | `sw $23,8($20)`; `jal 0x_13E648` | `+0x08 = a2`; `KindTableAdd(gobj, a1)` |
| return | `move $2,$20` | returns the gobj |

## Evidence — CreateGObj_v (0x240EA0)

Register form tail from `CreateGObj_v.s`:

| Register | Role |
|----------|------|
| a0 | user_data for `isysGObjAdd` |
| a1 / a2 / a3 | cbA / cbB / cbC (sister type ids 0x16/0x17/0x18) |
| t1 (`$19`) | DL callback (`move $5,$19` before `0x_13F130`) |
| t2 (`$20`) | DL sort key (`move $7,$20` → a3 of `0x_13F130`) |
| t0 (`$22`) | trigger: `beqz $22` gate AND wrapper callback (`move $5,$22`) |

Differs from CreateGObj: **no** `+0x08` write, **no** `KindTableAdd`. The
trigger register doubles as the gate and the `0x_13F7A8` a1 argument.

## Evidence — AllocGObjEntity (0x19F310)

| Step | Instruction evidence | Meaning |
|------|----------------------|---------|
| block alloc | `addiu $7,$0,0x1ce`; `lw $4,-0x68cc($28)`; `addiu $5,$0,0x850`; `addiu $6,$19,0x2620`; `jal 0x_13A0F8` | `heapAlloc(ctx gp-0x68CC, 0x850, tag 0x612620, line 0x1CE)` |
| template copy | `lui $2,0x2f; addiu $4,$2,0x23f0`; loop `loc_0019f35c` (66 x 0x20) + `ld/sd` tail | copy 0x850 from `0x2F23F0` (copyrighted, never embedded) |
| type init | `addiu $2,$0,0x5eb; beq $17,$2`; `jal 0x_1A27F8` | only when type != 0x5EB |
| debug | `jal 0x_1A6E28` at `0x2640` | `DebugPrint(0x612640)` |
| cond gate | `lw $2,0x820($16); bnel` / `lw $2,0x8c($16); beql` | `+0x820 != 0 || +0x8C != 0` → `0x_19EF58(block,arg)` |
| debug | `0x2658` | `DebugPrint(0x612658)` |
| slot init | `jal 0x_19E648` | always, `(block, arg)` |
| debug | `0x2678` | `DebugPrint(0x612678)` |
| list gate | `lw $2,0x8c($16); beqz` | only when `+0x8C != 0` |
| list init | `jal 0x_19EF10` | `0x_19EF10(block)` |
| table alloc | `lw $4,-0x68e0($28)`; `addiu $5,$0,0x35`; `addiu $7,$0,0x1b7`; `sw $2,0x810($16)` | `heapAlloc(ctx gp-0x68E0, 0x35, tag, line 0x1B7)`; store at `+0x810` |
| fill loop | `loc_0019f440`..`loc_0019f49c` | see below |
| return print | `0x2698` at `loc_0019f4b8` | `DebugPrint(0x612698)` |

Fill loop (per entry i, table has 0x35 entries):

- `listBase = *(block+0x8C)` (reloaded every iteration).
- If `*(listBase + 0) == -1`: value `-1`.
- Else walk `cursor = listBase + j*0x40` (j = 0, 1, ...):
  - `*(cursor+0x04) == i` → value `j`, stop.
  - else `cursor = listBase + (j+1)*0x40`; if `*(cursor+0) == -1` → value `-1`, stop.
- `*(block+0x810)` reloaded each iteration; stored as a byte (`sb`) at `slotTable + i`.

The final record of a chain carries link `-1` and its id is never examined —
the `-1` link word terminates the scan.

## Host bridge design

Both bridges follow the `WorldStateLoader` pattern (`bool initialize(...) /
void shutdown() / bool isInitialized()`) and are explicitly **semantic, not
byte-exact**; host pointers/handles replace PS2 absolute addresses.

`GObjFactory`:

- `createGObj(desc, a1, a2, a3, t0)` and `createGObjV(userData, cbA, cbB,
  cbC, dlCallback, dlSortKey, trigger)`.
- Reuses the existing semantic functions `ico_semantic_isysGObjAdd`,
  `_isysGObjLinkObjDL`, `_isysGObjKindTableAdd`, `_sisterCallbackReg`, and the
  two host hooks (`IcoSemanticProcAddFn` for `0x_13F3F0`, a 5-arg wrap hook
  for `0x_13F7A8`).
- Registry `0x712CC0` / counter `gp-0x4D58` mirrored as a handle vector
  (`registeredCount()` / `registeredHandle(index)`).
- Factory words written in the confirmed order; the `+0x08 = a2` write and
  `KindTableAdd(gobj, a1)` are CreateGObj-only.

`GObjEntityAllocator`:

- Hooks: `HeapAllocFn(context, size, tag, line)`, `TranslateFn` (EE→host,
  `Ps2Memory::translate` pattern), `DebugPrintFn`, and four init hooks
  (`0x_1A27F8` type, `0x_19EF58` cond, `0x_19E648` slot, `0x_19EF10` list).
- `templateSource` is a host-supplied pointer; the 0x850 bytes are copied but
  never stored in the repository (copyright).
- `allocEntity(type, arg)` reproduces the complete confirmed sequence,
  including the per-iteration `+0x8C` / `+0x810` reloads and the byte stores
  into the slot table.

Files added/changed on `native-port`:

- `native/src/engine/GObjFactory.h`, `GObjFactory.cpp` (new)
- `native/src/engine/GObjEntityAllocator.h`, `GObjEntityAllocator.cpp` (new)
- `native/tests/gobj_factory_test.cpp`, `gobj_entity_allocator_test.cpp` (new)
- `native/tests/isysgobj_semantic_test.c` (extended: unlink + sister; removed
  the former dead block after `return 0;`)
- `native/CMakeLists.txt` (two sources in `ICO_ENGINE_SOURCES`; two new
  `add_test` targets)

## Evidence — verification

| Test | Checks | Result |
|------|--------|--------|
| `isysgobj_semantic_test` | unlink relink + parent head/tail repair (head/middle/tail), sister argument shuffle + `a2&0xff` + fixed `t1=0x1800` + null-hook short-circuit | pass |
| `gobj_factory_test` | factory words, registry handles, 3x sister arg exactness, DL link (callback/mask/list id/sort key), kind-table chain, trigger gate off/zero, `_v` differences (+0x08 untouched, no kind add), wrap constant tail `(…, 0x13, 0x1800)`, shutdown inert | pass |
| `gobj_entity_allocator_test` | template copy, both heap calls (context/size/tag/line), debug order `[0x612640, 0x612658, 0x612678, 0x612698]`, type==0x5EB skip, cond OR-gate, empty-list all 0xFF, listBase==0 skips cond/list/table, flag-gated cond, shutdown inert | pass |
| full `ctest` | **20/20 tests passed** (was 18 before this revision) | pass |

## What is confirmed

- The exact register/id mapping and step order for both `CreateGObj` forms
  (including the `0x_13F130` sort-key source: a3 for `CreateGObj`, t2 for
  `CreateGObj_v`; and the `0x_13F7A8` gate/callback reuse in `_v`).
- `AllocGObjEntity` allocation contexts, lines, tags, debug message addresses,
  init-hook gating, and the slot-table fill algorithm (stride 0x40, `+0x04`
  id match, link `-1` sentinel, per-entry `+0x8C`/`+0x810` reloads, byte
  stores, 0x35 entries).
- The three bridges reproduce every confirmed observable side effect in the
  CTest suite.

## What is probable

- `0x_13F7A8` (`isysGObjProcAdd_Wrapper`) and `0x_13F3F0`
  (`isysGObjProcAdd_`) will eventually get host semantics; today they are
  hooks so the argument contract stays observable.
- The `0x712CC0`/`gp-0x4D58` counter is the "entity creation order" registry
  consumed elsewhere in scene setup.

## What is possible / unknown

- The exact meaning of the `0x5EB` type tag and the `+0x820`/`+0x8C`/
  `+0x810` entity block fields is still reverse-engineering territory; the
  bridge only preserves the confirmed access pattern.
- Whether the slot table entries are per-"capture point" indices awaiting a
  runtime probe.
- The `type >= 0x44` (out-of-range kind) edge of `isysGObjKindTableAdd`
  clears `type_next` in the semantic C but not in the `.s`; cosmetic, not
  exercised by these factories (CreateGObj passes player/entity types).

## What is discarded

- Any attempt to embed or ship the 0x2F23F0 template bytes (copyright) — the
  host bridge copies from a caller-supplied source instead.
- Claiming the bridges are byte-exact; they are explicitly semantic.

## Next minimum test

- Runtime probe (`0x_13F3F0` / `0x_13F7A8`) during a scene load to record the
  first two `CreateGObj` invocations and confirm the wrapper tail
  `(…, 0x13, 0x1800)` live, then compare against `gobj_factory_test`.
- Optionally bind `AllocGObjEntity` into the scene-loader bridge once the
  entity block field meanings are recovered.

## Conservative verdict

`GObjFactory` and `GObjEntityAllocator` lock the confirmed byte-level
contracts of the three remaining hot gaps into auditable host code with
test coverage, without altering `main`'s ground-truth `.s`. This closes the
todo #2 bridge phase; the next step is live runtime validation, not more
bridge scaffolding.