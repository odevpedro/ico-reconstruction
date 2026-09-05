# Rev.130 — Closing hot gaps #3/#4/#5: `AllocGObjEntity`, `world_state_load`, `isysGObjProcRemoveUnlink` byte-exact

- **Date:** 2026-09-05
- **Objective:** Reconstruct the three remaining byte-exact gaps identified from
  runtime hot-path analysis (the factory chain allocator, the world-state loader,
  and the process-remove unlink helper).
- **Scope:** Byte-exact `.s` preservation. No semantic C claimed.
- **Sources used:**
  - `research/elf/rev113-scene-gobj-factory-contract.md` (allocator contract)
  - `research/elf/rev124-runtime-probe-prep-and-game-loop-scene-bridge.md`
    (world_state_load probe address)
  - `research/elf/rev128-sister-callback-reg-byte-exact.md`, Rev.129 (prior gaps)
  - `research/elf/ghidra-rev099-isysgobj-lifecycle-and-ios-thread.md` (process
    lifecycle, GObj removal)
- **Evidence used:** `.local/extracted/SCUS_971.13.elf` (USA ELF, little-endian).
  Disassembly and `.s` generated with the pipeline's own `generate_asm_source`;
  verification with `assemble_and_verify`. All sizes confirmed against the next
  function boundary (prologue scan) **including** delay slots.

## 1. New byte-exact `.s` files

| Function | VA | Size | Previous doc | Verified |
|----------|-----|------|--------------|----------|
| `AllocGObjEntity` | 0x19F310 | 0x1D0 | — | **100%** |
| `world_state_load` | 0x1AF948 | **0x80** | probe addr only | **100%** |
| `isysGObjProcRemoveUnlink` | 0x13F638 | 0x80 | — | **100%** |

> **CORRECTION (Rev.131).** The size `0x248` listed here when Rev.130 was first
> written was **wrong**: it merged two functions. The byte block that starts at
> `0x1AF948` and runs to `0x1AFB8F` actually contains:
> - `world_state_load` (0x1AF948–0x1AF9C7, **0x80 bytes**) — terminates with the
>   shared-epilogue tail-jump `j 0x13d3f8` (+ delay `addiu $sp,$sp,0xb0`);
> - `DispIcoMisc` (0x1AF9C8–0x1AFB8F, **0x1C8 bytes**) — a distinct function
>   with its own `$sp,-0x90` prologue and `jr $ra` epilogue, named by the
>   Ghidra/PAL symbol map (research/ghidra-exploration-2026-05-21.md:1541).
> The `0x1AF9C8` fingerprint of size 0x98 in `docs/symbols/usa_fingerprints.json`
> is the reconciliation scanner's first-scan cut, NOT a real function start here.
> Rev.131 re-split the ground truth and saved two byte-exact `.s`:
> `src/core/asm/world_state_load.s` (0x80) and `src/core/asm/DispIcoMisc.s`
> (0x1C8), both verified 100% by `assemble_and_verify`.

All three generated/verified via `generate_asm_source` + `assemble_and_verify`.
Branch labels are the pipeline's `loc_<va>` form; absolute jal/j targets emitted
as `.word` encodings where the assembler requires it.

## 2. `AllocGObjEntity` (0x19F310, 0x1D0 bytes) — confirms Rev.113

- Frame 0x50; args `a0(+0x1CE constant check)`, `$a1`.
- Calls `0x13A0F8` twice with sizes **0x850** and **0x35** (slot-index table).
- Copies **0x840 bytes** from static template `0x2F23F0` → freshly allocated block
  (0x840 = 0x850 minus a 0x10 frame gap).
- Branch to skip if `a0 == 0x5EB` (special object type).
- String-table callbacks `0x1A6E28` with `0x612620/0x612640/0x612658/0x612678/0x612698`
  (gp+0x4xxx group) — consistency-checks with the `0x4Bxx` arena region.
- Fills a **0x35-entry** slot-index table at `alloc+0x810`, scanning for `-1`
  sentinels (cached slot reuse); sets `alloc+0x8C` from the same template.
- Restores stack; returns allocated block in `v0`.

## 3. `world_state_load` (0x1AF948, 0x80 bytes) — real dispatcher shape

> Boundary correction (Rev.131): the function is 0x80 bytes, not the 0x248 of
> the first scan. `DispIcoMisc` (0x1AF9C8) immediately follows; see the table
> note above.

- Frame 0xB0; reads world_state from `gp-0x6F60`, immediately does
  `mult $v1,$v1,$a0` with `$a0=0x194` (stride 0x194) indexing table `0x5F2FB8`
  at offset **+0x154** → if nonzero, `jalr` (per-state init function for the
  new room).
- Then `0x166028` (`MakeCollisionDependGObjList`) and `0x1AE3E8` (scene apply).
- Clears state at `0x274EC0+0x14/+0x18` and tail-jumps to the shared epilogue
  `0x13D3F8` (`iosThreadDestroy`) with delay `addiu $sp,$sp,0xb0`.
- Boundary: the function's own `jr $ra` is implicit at `0x13D3F8` (shared tail);
  the `0x1AFB84` `jr $ra` seen in the first scan belongs to `DispIcoMisc`.

**Notable:** `world_state_load` does **not** contain the world_state transitions
enumerated in Rev.125/126 runtime sessions — those come from the caller
(kanban/scene loader), consistent with the state table being a per-room
head/entry pointer set before this call.

## 4. `isysGObjProcRemoveUnlink` (0x13F638, 0x80 bytes)

- Branch-heavy small helper performing the `ProcessNode` priority-list unlink:
  delay-slot idioms compact the branch conditions.
- Confirms the process-list head/tail pointers at `PPN+0x2C` and `PPN+0x30`, and
  the node+0x08/+0x0C next/prev words — consistent with Rev.099's process-node
  layout (`+0x08` next, `+0x0C` prev; head/tail at process base `+0x2C/+0x30`).
- Also shows a null-node error path pointing to string `0x557B48` via `0x1A6E28`.

## 5. Correction to Rev.099: `isysGObjKindTableAdd` is 0xE0, not 0xDC

While checking `0x13E648` (a `CreateGObj` callee), Rev.099's documented size
(220B / 0xDC) does not match the on-disk function — the framesize/delay-slot scan
shows the real `jr $ra` at 0x13E720 with delay `addiu $sp,$sp,0x50` at 0x13E724
(next function at 0x13E728): **actual size 0xE0 (224 B)**. The existing
`src/core/asm/isysGObjKindTableAdd.s` verifies **100%** at 0xE0. The 0xDC figure
was an off-by-one that excluded the delay slot. `isysGObjLinkObjDL` (0x13F130)
already verified 100% at its documented size.

## 6. Confirmed / probable / unknown

**Confirmed:**
- 3 hot-path functions now have byte-exact `.s` (world_state_load 0x80 re-split in
  Rev.131); `world_state_load`, `AllocGObjEntity`, `isysGObjProcRemoveUnlink` all
  100% via `assemble_and_verify`.
- `world_state_load` dispatches per-room init via table 0x5F2FB8 stride 0x194, +0x154.
- `AllocGObjEntity` matches Rev.113's allocator contract (0x850/template 0x2F23F0).
- `isysGObjProcRemoveUnlink` matches Rev.099 process-node list layout.
- `isysGObjKindTableAdd` actual size 0xE0 (corrected from Rev.099's 0xDC);
  existing `.s` is byte-exact at 0xE0.
- `DispIcoMisc` (0x1AF9C8) is a distinct function immediately after
  `world_state_load`; named by the Ghidra/PAL symbol map (Rev.131).

**Probable:**
- `world_state_load` is the per-room dispatch head; the state-transition sequence
  observed at runtime happens before this function (in the scene-loader caller).
- `AllocGObjEntity`'s 0x5EB skip is a special-case object type.

**Unknown:**
- Identity of `0x13F130`/`0x13E648` as used specifically by `CreateGObj` (names
  confirm via Rev.099 as kind-table-add / link-obj-dl, but the role of the direct
  `0x13F130` call inside `CreateGObj` is not fully priced).
- Exact layout of the `0x5F2FB8` per-room table beyond the `+0x154` callback.

## 7. Next minimum test

Full pipeline pass (`asm_source_score.py --all --no-save`) to confirm the three
new `.s` are picked up; then verify the new count (704 tracked + 3 new = 707).

## Conservative verdict

The five hot-path gaps are now **all closed as byte-exact `.s`**: sister_callback_reg
(Rev.128), CreateGObj/CreateGObj_v (Rev.129), AllocGObjEntity, world_state_load,
and isysGObjProcRemoveUnlink (this Rev.130). No semantic C is claimed; these are
verified ground-truth artifacts.