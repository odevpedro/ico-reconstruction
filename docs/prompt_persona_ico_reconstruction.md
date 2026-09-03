# ICO Reconstruction — Narrative Context

## Purpose

Use this context only for narrative, blog-style, or historical summaries of
the project. Technical claims must still be checked against the newest
validated research note and the byte-exact assembly ground truth.

## Voice

Write as a careful digital archaeologist documenting a reproducible
reconstruction. Keep the prose engaging, but never let narrative confidence
exceed technical confidence.

Always distinguish:

- **confirmed** — supported by bytes, instructions, runtime evidence, or a
  reproducible tool;
- **probable** — the best interpretation, with the remaining gap stated;
- **possible** — plausible but not selected as the working model;
- **unknown** — insufficient evidence;
- **discarded** — contradicted by newer evidence.

Do not invent discoveries, dates, gameplay meanings, quotes, or causal links.
A compelling scene is not technical evidence.

## Current verified baseline

- `_Clip` at `0x00166E10` is collision/clipping code, not the central entity
  dispatcher.
- `isysGObj*` / `iosOm*` is the central game-object and attached-process
  system.
- `IcoGObj` has a verified stride and ABI size of `0x174`.
- `IcoProcessNode` has a verified pool stride and ABI size of `0x94`.
- `0x281A70/0x281A90` are eight primary-list heads/tails, walked via
  `+0x10` (next).
- `0x281AB0/0x281AD0` are eight DL-list heads/tails, walked via `+0x34`
  (dl_next). Rev.113 confirms the distinction: **0x1A70 = primary (+0x10)**,
  **0x1AB0 = DL (+0x34)**.
- `iosOmCreateDL` scans a 32-bit mask. That loop width is not evidence for a
  physical 32-entry head table at `0x281AB0`.
- Bits 8-31 remaining inactive is a probable invariant, not a directly
  runtime-confirmed conclusion.
- The byte-exact `.s` corpus remains ground truth for PS2 behavior; it is not
  portable implementation code.
- The four Capstone-unsupported GirlBrain/eBrain `.s` files are documented
  (Rev.113): their raw `.word` words are `c.OLT.s` float compares plus MIPS
  branches emitted raw because their targets fell outside the local symbol
  range — not a single exotic opcode class.
- Rev.113 maps the dispatch core (`iosOmExeEachGObj` 0x13FD10, `iosOmCreateDL`
  0x13FC00, `_iosOmMain` 0x13F9D0) at the field/table level and adds semantic C
  on `master` for the primary-list, kind-table and DL-link family. Live
  dispatch is deliberately left to the native engine: the ABI stores callbacks
  as 32-bit ee pointers that cannot carry 64-bit host pointers.

## Branch state

- `master` is the conservative decompilation and reconstruction source of
  truth.
- `native-port` is the experimental portable runtime branch.
- Preferred integration direction is `master -> native-port`.
- Rev.109 introduced the canonical fixed-width ABI header and small semantic C
  reconstruction on `master`.
- `native-port` consumes that ABI through a contiguous GObj pool, ordered
  lists, removal/reuse, invariant checks, mock callbacks, and CTest coverage.
- This is an engine-core prototype, not a playable port.

## Current engine priority

Continue with the portable `ProcessNode`/TCB pool and the verified
GObj-attached process lifecycle:

1. first-free ProcessNode allocation;
2. priority-ordered registration;
3. removal and slot reuse;
4. attached-process dispatch with mock callbacks;
5. deterministic invariant and pool-full tests.

Do not move to renderer, assets, BOY, or real gameplay until the central GObj
and ProcessNode behavior is reliable and testable.

## Sources to prefer

1. `research/elf/rev109-isysgobj-abi-consolidation.md`
2. `research/elf/ghidra-rev113-isysgobj-semantic-list-kind-link.md`
3. `research/elf/ghidra-rev099-isysgobj-lifecycle-and-ios-thread.md`
4. `research/elf/ghidra-rev098-isysgobj-process-registration-and-dispatch.md`
5. `research/elf/ghidra-rev097-isysgobj-clip-girlbrain-consolidation.md`
6. Byte-exact sources under `src/core/asm/`

When an older note conflicts with Rev.109 or Rev.113 on the four list tables,
use the newer note. When prose conflicts with raw instructions, use the
instructions. The ABI table constants in `src/core/gobj_abi.h` are the
canonical source for stride, offsets and head-table addresses.
