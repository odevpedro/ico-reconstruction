# ICO Reconstruction Evidence Index

This document is an objective index for the validated reconstruction path in
this repository.

It is not a narrative history. It is a control document that answers four
questions:

1. What sources were consulted?
2. What tools were used?
3. What artifacts were promoted?
4. What is still open?

## Scope

This index covers the currently validated work around:

- the `isysGObj*` / `iosOm*` core block in `src/core/asm/`;
- the earlier Path B byte-exact set in `src/entity/asm/` and `src/cloth/asm/`;
- the research notes that established the evidence chain for those results;
- the local tooling used to verify byte-exact assembly and runtime behavior.

It does not claim to be a full ledger of every intermediate idea, failed
attempt, or private note ever produced during the project.

## Sources Used

### Project context

- `AGENTS.md`
- `docs/research-methodology.md`
- `docs/local-logs-and-reports.md`
- local context files when present:
  - `.local/key-concepts.md`
  - `.local/ai-context.md`
  - `key-concepts.md`

### Research notes consulted for the validated path

| File | Role |
|------|------|
| `research/elf/ghidra-rev073-main-loop-dispatch-chain-and-callback-corrected-masks.md` | Main-loop dispatch model |
| `research/elf/ghidra-rev074-runtime-session-main-loop-dispatch-confirmed.md` | Runtime confirmation of the dispatch chain |
| `research/elf/ghidra-rev075-init-fn-callback-dispatch-and-asm-handler-consolidation.md` | Init/callback consolidation |
| `research/elf/ghidra-rev077-final-static-analysis.md` | Static architecture summary |
| `research/elf/ghidra-rev084-runtime-validation-extended-session.md` | Extended runtime validation |
| `research/elf/ghidra-rev085-death-validation-and-next-session-plan.md` | Validation and next-session planning |
| `research/elf/ghidra-rev086-static-analysis-vtables-enveffect-cbroutine4-vblank.md` | Vtable, env effect, callback, VBlank analysis |
| `research/elf/ghidra-rev087-enemy1-boy-decompilation.md` | BOY / ENEMY1 decompilation |
| `research/elf/ghidra-rev088-barrel-rope-woodbox0-decompilation.md` | BARREL / ROPE / WOODBOX0 decompilation |
| `research/elf/ghidra-rev089-runtime-session-rev086-worldstate-gp-m49b4.md` | world_state and gp variable work |
| `research/elf/ghidra-rev091f-boy-hb-near-exact-via-normalization.md` | Path A/C/D normalization milestone |
| `research/elf/ghidra-rev091g-boy-hc-77pct-via-li-expansion-and-constant-fix.md` | Path A/C/D normalization milestone |
| `research/elf/ghidra-rev093-three-investigations.md` | mask set and dispatch table analysis |
| `research/elf/ghidra-rev093b-halfword-entry-runtime-validation.md` | halfword entry validation |
| `research/elf/ghidra-rev094-halfword-runtime-second-caller.md` | second halfword caller validation |
| `research/elf/ghidra-rev096-halfword-runtime-session-analysis.md` | offline runtime review |
| `research/ghidra-exploration-2026-05-21.md` | symbol-import exploration and real-name reconciliation |
| `research/elf/ghidra-rev099-isysgobj-lifecycle-and-ios-thread.md` | `isysGObj*` / `iosOm*` lifecycle note |
| `research/external/sotc-tooling-relevance-survey.md` | tooling/process reference only |
| `research/external/ico-rabbitizer-spimdisasm-dispatcher-check.md` | external decoding cross-check |
| `research/external/ico-splat-promoted-ranges-experiment.md` | range promotion experiment |
| `research/external/ico-splat-adjacent-promoted-ranges-experiment.md` | adjacent range promotion experiment |

## Tools Used

### Verification and analysis

- `tools/asm_source_score.py`
- `tools/score_all.py`
- `tools/analyze_halfword_log.py` with optional recent-tail summaries via `--recent-lines` / `--recent-mb`
- Ghidra headless analysis scripts
- Rabbitizer / spimdisasm cross-checks
- PCSX2 runtime breakpoint sessions

### Assembly and validation

- local `ee-gcc 2.9-991111-01`
- byte-for-byte comparison against the extracted ELF
- promoted `.s` sources assembled and verified against target VAs

### Workflow rules

- keep confirmed facts separate from inference;
- do not upgrade a hypothesis without byte-level, instruction-level, or runtime evidence;
- treat Ghidra names as hints, not authority;
- prefer small reproducible observations over broad claims.

## Validated Commits

| Commit | Summary |
|--------|---------|
| `531fdf7` | Added the `rev099` lifecycle note |
| `7c65133` | Added `isysGObjProcessAlloc` / `isysGObjProcAdd_` asm |
| `c6a5e7a` | Fixed `iosOmInit` labels and added comments |
| `5d2329e` | Added `isysGObjMove*`, `isysGObjAlloc`, `isysGObjKindTable*` asm |
| `5cd7802` | Added `isysGObjInit` / `isysGObjRemove` asm |
| `d212746` | Added `isysGObjAdd*`, `isysGObjRemoveAll`, `_iosOmMain` asm |
| `9ad073d` | Added `isysGObjAddHead` asm |
| `7d53c13` | Removed personal narrative docs from git tracking |

## Byte-Exact Artifacts

### Path B exact set already established before the `isysGObj*` work

These 12 functions were already exact via C in the first Path B cycle:

- `enemy1_hA`
- `fn_1CE5F8`
- `boy_hB`
- `boy_set_state`
- `barrel_hA`
- `barrel_hA_alt`
- `cloth_get_variant`
- `cloth_payload_field0_is_zero`
- `cloth_payload_state_is_two`
- `cloth_test_state_lt_2`
- `cloth_test_variant_field`
- `cloth_test_field0_or_extra`

### `src/core/asm/` exact set

There are currently 36 byte-exact assembly files in `src/core/asm/`.

#### Lifecycle and list management

- `isysGObjInit`
- `isysGObjAlloc`
- `isysGObjAdd`
- `isysGObjAddAfterGObj`
- `isysGObjAddBeforeGObj`
- `isysGObjAddHead`
- `isysGObjMove`
- `isysGObjMoveAfterGObj`
- `isysGObjMoveBeforeGObj`
- `isysGObjRemove`
- `isysGObjRemoveAll`
- `isysGObjKindTableAdd`
- `isysGObjKindTableRemove`

#### DL and process helpers

- `isysGObjDlInit`
- `isysGObjLinkObjDL`
- `isysGObjProcessAlloc`
- `isysGObjProcAdd_`
- `isysGObjProcAdd_Wrapper`
- `isysGObjProcAddSGOppArg`
- `isysGObjProcRemove`
- `isysGObjProcPause`
- `isysGObjProcRemoveAll`
- `isysGObjProcThreadSleep`

#### iosOm core

- `_iosOmMain`
- `iosOmInit`
- `iosOmCreateDL`
- `iosOmExeEachGObj`
- `iosOmExeEachGObjAll`
- `iosOmReturnExeEachGObj`
- `iosOmGetGObjStatus`
- `iosOmExeMail`

#### Active and camera helpers

- `isysGObjActiveLink`
- `isysGObjActiveDlLink`
- `isysGObjMoveCameraDL`
- `isysGObjLinkCameraDL`
- `isysGObjMoveCameraDLHead`

## Local-Only Narrative Files

These files are intentionally kept outside git tracking:

- `docs/historia.md`
- `docs/prompt_persona_ico_reconstruction.md`

They are local narrative and persona documents, not part of the repository
history.

## Known Gaps

### Documentation gaps

- The narrative history is not a complete ledger of every tool run or failed
  hypothesis.
- The rev099 note now mirrors the current `src/core/asm/` byte-exact set in
  its size map and addendum, but it is still a research note rather than a
  machine-generated inventory of every promoted file ever considered.
- The current index is intentionally conservative and avoids turning semantic
  guesses into final claims.

### Evidence gaps

These runtime probes and checks remain open from the broader project context:

- direct probe of `0x00166DFC` fast path;
- second halfword caller instrumentation at `0x0016828C` / `0x00168294`;
- slot-index capture for the dispatch-point path;
- world-state load capture for room initialization;
- VBlank counter watchpoint at `0x274EC0`.

### Semantic gaps

- The `isysGObj*` / `iosOm*` byte-exact set is mechanically validated.
- Individual field meanings and some slot interactions still need xref or
  runtime support before they should be promoted to strong semantic claims.

## Current Summary

Current validated state:

- the `isysGObj*` / `iosOm*` core block is byte-exact in `src/core/asm/`;
- the earlier Path B exact set remains in place;
- the project has a clean separation between validated facts, local narrative,
  and open gaps;
- the evidence chain is reproducible with local tools and the user-owned ELF
  copy.

## Chronological Appendix

Short validation path, in order:

| Date | Milestone | Result |
|------|-----------|--------|
| 2026-05-18 | Path B byte-exact assembly phase | 38/38 functions validated, with 26 promoted via `.s` and 12 exact via C |
| 2026-05-21 | `isysGObj*` / `iosOm*` block promotion | 36 byte-exact `.s` files in `src/core/asm/` |
| 2026-05-21 | Evidence index created | Sources, tools, commits, artifacts, and gaps consolidated in one factual document |
| 2026-05-21 | Documentation sync | `rev.099`, backlog, and architecture log aligned to the current validated state |
| 2026-05-21 | Static architecture follow-up | `docs/architectural-analysis-e-g.md` formalized the post-Path-B decision matrix |

This appendix is intentionally short. It records the major validated steps
without trying to enumerate every intermediate hypothesis or tool run.
