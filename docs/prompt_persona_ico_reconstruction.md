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
- `0x281A70/0x281A90` are eight primary-list heads/tails.
- `0x281AB0/0x281AD0` are eight DL-list heads/tails.
- `iosOmCreateDL` scans a 32-bit mask. That loop width is not evidence for a
  physical 32-entry head table at `0x281AB0`.
- Bits 8-31 remaining inactive is a probable invariant, not a directly
  runtime-confirmed conclusion.
- The byte-exact `.s` corpus remains ground truth for PS2 behavior; it is not
  portable implementation code.

## Branch state

- `master` is the conservative decompilation and reconstruction source of
  truth.
- `native-port` is the experimental portable runtime branch.
- Preferred integration direction is `master -> native-port`.
- Rev.109 introduced the canonical fixed-width ABI header and small semantic C
  reconstruction on `master`.
- `native-port` consumes that ABI through a contiguous GObj pool, ProcessNode
  pool, priority-ordered registration, attached-process dispatch, ordered
  lists, removal/reuse, invariant checks, mock callbacks, and CTest coverage
  (5 tests). This is an engine-core prototype, not a playable port.

## Current engine priority

Continue with `iosOmCreateDL` slot dispatch and type-based routing on
`native-port`. The GObj pool, ProcessNode pool, priority-ordered
registration, removal, attached-process dispatch, and callback mocks are
implemented and tested (5 CTest tests).

Do not move to renderer, assets, BOY, or real gameplay until the central GObj
and ProcessNode behavior is reliable and testable.

## Sources to prefer

1. `research/elf/rev109-isysgobj-abi-consolidation.md`
2. `research/elf/ghidra-rev099-isysgobj-lifecycle-and-ios-thread.md`
3. `research/elf/ghidra-rev098-isysgobj-process-registration-and-dispatch.md`
4. `research/elf/ghidra-rev097-isysgobj-clip-girlbrain-consolidation.md`
5. Byte-exact sources under `src/core/asm/`

When an older note conflicts with Rev.109 on the four list tables, use
Rev.109. When prose conflicts with raw instructions, use the instructions.
