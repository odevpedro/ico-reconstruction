# AGENTS.md — ico-reconstruction

## Project identity

This repository is a reverse engineering / digital archaeology project focused on studying the PlayStation 2 game **ICO**.

The goal is to incrementally reconstruct verified technical knowledge about:

- the PS2 ELF binary;
- MIPS code;
- Ghidra analysis;
- PCSX2 runtime behavior;
- entity/state systems;
- asset references;
- overlays;
- internal dispatch patterns;
- possible reconstruction paths.

This is not a race to produce conclusions.

The goal is to build an auditable chain of evidence.

Prefer one small confirmed fact over a large speculative theory.

---

## Core rule

Always separate:

- confirmed facts;
- probable interpretations;
- possible hypotheses;
- weak speculation;
- discarded ideas.

Never upgrade a hypothesis into a conclusion without direct evidence.

Direct evidence may include:

- byte-level verification;
- instruction-level verification;
- Ghidra xrefs;
- reproducible scripts;
- runtime breakpoints;
- memory dumps;
- patch tests;
- consistent cross-document validation.

Do not treat previous AI-generated notes as truth unless they are backed by evidence.

---

## Project context / AI context file

Before starting any substantial analysis, read the AI context file if it exists:

- `.local/key-concepts.md`
- `.local/ai-context.md`
- `key-concepts.md`

This file contains the conceptual vocabulary of the project and should be used to understand:

- terminology;
- assumptions;
- investigation style;
- current mental model;
- caution rules;
- known traps;
- preferred naming.

However, do not treat the AI context file as absolute truth.

Use it as orientation only.

When there is a conflict between the AI context file and a later validated research revision, prefer the most recent validated research note.

At the current stage, the most important validated research notes are:

```txt
research/elf/ghidra-rev037-remaining-callers-and-rope-gap.md  (latest static analysis)
research/elf/ghidra-rev025-runtime-confirmed-caller-context.md (runtime validation)
research/elf/ghidra-rev023-dispatcher-table-resolution.md     (dispatcher foundation)
```

---

## Important documents to read first

Before doing new analysis, read these files in this order if they exist:

1. `AGENTS.md`
2. `.local/key-concepts.md`
3. `.local/ai-context.md`
4. `key-concepts.md`
5. `research/elf/ghidra-rev037-remaining-callers-and-rope-gap.md`
6. `research/elf/ghidra-rev025-runtime-confirmed-caller-context.md`
7. `research/elf/ghidra-rev023-dispatcher-table-resolution.md`
8. `research/elf/ghidra-rev022-dispatcher-ground-truth.md`
9. `research/elf/ghidra-rev021-continue-menu-pivot.md`
10. `research/elf/ghidra-rev018-state-transition-dispatch.md`
11. `research/elf/ghidra-rev019-state-resolver-caller-context.md`

Use Rev.037 as the current source of truth when it contradicts earlier revisions.

---

## Current strategic status

### Confirmed dispatcher model (Rev.022-024)

The dispatcher at `0x001d37c8` is confirmed with jump table at `0x00618fb0` (5 entries):

```txt
entity/context
-> [entity + 0x800]
-> candidate_state_block_ptr
-> [candidate_state_block_ptr + 0x48]
-> candidate_state_id
-> bounds check candidate_state_id < 5
-> candidate_state_id * 4
-> jump table at 0x00618fb0
-> internal basic block handler
```

### Runtime confirmation (Rev.025)

The only static caller of the dispatcher, `0x001d3a30`, was confirmed at runtime via PCSX2 breakpoint (Rev.024 session). It reaches the dispatcher during gameplay/load.

### ROPE callback registration gap (Rev.033-037)

Static analysis of all 5 callsites of `0x0013f7a8` (callback registration) is complete. Three paths are definitively excluded for ROPE callback `0x001d3a30`. Two paths remain candidates but require runtime validation. **Static options are exhausted.**

### External tooling results (Rev.038)

- **CCC (Chaos Compiler Collection)**: No `.mdebug`/STABS/debug symbols found in ELF.
- **decomp.me scratches**: 6 function packages generated (dispatcher, ROPE callback, registration chain) for crowd-sourced decompilation matching.
- **ICO-decomp cross-reference** (`RossyDoubleUnderscore/ICO-decomp`): Dispatcher `0x001d37c8` and ROPE callback `0x001d3a30` reside in **`clothAnimation.c`** (cloth physics), not entity/AI state. Nearby symbols: `GetCloth4D`, `getCloth4D_PlaneClip`. The 5 internal state blocks are likely cloth vertex/simulation state transitions.
- **Compiler confirmed**: EE GCC 2.9-991111-01 with flags `-march=r5900 -mips3 -mgp64 -mabi=eabi -msingle-float -G0 -O2`.
- **Source tree**: ICO-decomp splat YAML reveals folder structure: `sugipon/` (cloth, physics, motion, gameplay), `omori/` (camera, AI), `fumi/` (core engine, IOS), `ito/` (bosses), `seki/` (rendering).

---

## Known corrections

The instruction at:

```txt
0x001d3800
```

is:

```asm
sll $3,$3,2
```

not:

```asm
sll $3,$0,2
```

The real jump table is:

```txt
0x00618fb0
```

not:

```txt
0x00628fb0
```

The previous invalid values found at `0x00628fb0` were caused by reading the wrong address.

The Rev.021 targets such as:

```txt
0x001f2148
0x001d2538
0x001d2540
0x00105f00
```

should not be treated as validated state handlers unless later evidence proves otherwise.

Rev.023 rejected them as the current dispatcher targets.

### Rev.038 correction

The dispatcher `0x001d37c8` and its 5 internal state blocks are **cloth animation state transitions** (verified via ICO-decomp symbol names: `clothAnimation.c` range), not entity/gameplay state management. All earlier speculative names (Yorda, capture, menu, etc.) are incorrect for these functions. The 5 states likely represent cloth vertex simulation phases (e.g., idle, wind, collision, constraint solve, post-process).

---

## Current confirmed dispatcher model

Use this provisional terminology:

```txt
candidate_state_block_ptr = [entity + 0x800]
candidate_state_id        = [candidate_state_block_ptr + 0x48]
candidate_state_dispatcher = 0x001d37c8
candidate_state_jump_table = 0x00618fb0
```

Use neutral names for state blocks:

```txt
state_0_block = 0x001d3818
state_1_block = 0x001d3844
state_2_block = 0x001d391c
state_3_block = 0x001d39e0
state_4_block = 0x001d3a10
```

Do not call these definitively:

- Yorda state;
- capture state;
- continue state;
- menu state;
- death state;
- final state;
- animation state;
- AI state.

Those names require evidence.

---

## Current priority

The static analysis phase (Rev.001-037) is complete. The verified chain covers:

1. Dispatcher model (`0x001d37c8` + 5 state blocks)
2. ROPE callback (`0x001d3a30` confirmed at runtime)
3. Callback registration chain (`0x0013f7a8` -> `0x0013f3f0` -> node+0x1c)
4. 5 callsites of registration function mapped; 3 excluded for ROPE
5. Compiler identified: EE GCC (Sony fork for R5900)

The static analysis phase (Rev.001-037) is complete. The verified chain covers:

1. Dispatcher model (`0x001d37c8` + 5 state blocks)
2. ROPE callback (`0x001d3a30` confirmed at runtime)
3. Callback registration chain (`0x0013f7a8` -> `0x0013f3f0` -> node+0x1c)
4. 5 callsites of registration function mapped; 3 excluded for ROPE
5. Compiler identified: EE GCC (Sony fork for R5900)

The project has entered a new phase: **External Integration**.

```txt
Phase 2 — External Integration
```

Objective:

1. Submit scratches to decomp.me for crowd-sourced matching (Rev.038 in progress)
2. Cross-reference findings with ICO-decomp project (if feasible)
3. Validate compiler flags through PS2 dev community knowledge
4. Prepare runtime breakpoints for the unresolved ROPE gap

The next file should be:

```txt
research/elf/ghidra-rev038-decompme-scratches.md
```

Only create a different file if explicitly instructed.

---

## What not to investigate yet

Unless explicitly asked, do not investigate:

- `DATA.DF`;
- `.gcm`;
- Yorda strings;
- capture strings;
- shadow strings;
- Continue menu;
- TM2 textures;
- overlay extraction;
- unrelated asset formats;
- broad ISO scanning;
- speculative gameplay naming.

These are important, but they are not the current priority.

The current priority is understanding the confirmed dispatcher and its five internal blocks.

---

## Documentation discipline

When creating a new research note, write only to the requested file.

Do not freely edit:

- `docs/backlog.md`;
- `docs/architecture-log.md`;
- high-level project summaries;
- older revision notes.

Do not update high-level docs until the result is reviewed, except that `README.md`
should be updated when a result is mature and relevant enough to change the
public project summary.

When updating `README.md`, keep it conservative:

- summarize only validated research state;
- avoid speculative gameplay names;
- link to the newest relevant research notes;
- do not include copyrighted game data;
- do not turn unresolved hypotheses into project claims.

Do not rewrite history.

If a previous note is wrong, create a new correction note or clearly mark the contradiction in the current revision.

## Blog persona prompt maintenance

The file below is a required companion context for narrative/blog-style writing about this project:

```txt
prompt_persona_ico_reconstruction.md
```

Whenever a validated research revision changes the current understanding of the project, update this persona/blog prompt as part of the same work.

This is a project condition, not an optional cleanup task.

The prompt must stay aligned with the latest validated research while preserving the same caution rules:

- separate confirmed facts from hypotheses;
- do not turn narrative scenes into technical evidence;
- do not invent discoveries;
- prefer the newest validated research note when older notes conflict;
- keep the archaeology/digital-reconstruction tone grounded in reproducible evidence.

---

## Required output style for research notes

Every research note should include:

- title;
- date;
- objective;
- scope;
- sources used;
- evidence used;
- byte-level or instruction-level findings;
- tables for addresses and hypotheses;
- what is confirmed;
- what is probable;
- what is possible;
- what is unknown;
- what is discarded;
- next minimum test;
- conservative verdict.

Use tables when comparing:

- addresses;
- states;
- functions;
- offsets;
- hypotheses;
- confidence levels.

---

## Evidence hierarchy

Prefer evidence in this order:

1. Byte-level verification.
2. Instruction-level disassembly.
3. Runtime breakpoint / memory dump.
4. Ghidra xrefs.
5. Reproducible script output.
6. Consistency across multiple research notes.
7. Plausible interpretation.
8. Naming intuition.
9. String matches.
10. AI-generated speculation.

String matches are weak evidence unless tied to xrefs or runtime behavior.

A visible patch confirms location, not necessarily semantics.

---

## Ghidra caution

Ghidra is useful, but not authoritative.

When Ghidra output conflicts with raw bytes, trust raw bytes.

When Ghidra names something automatically, treat it as a hint, not a conclusion.

When Ghidra infers jump targets, verify the table bytes.

When a function has zero static callers, consider:

- indirect call;
- function pointer;
- vtable-like dispatch;
- jump table;
- dead code;
- overlay-related code;
- analysis artifact.

Do not assume zero static callers means high importance.

---

## Runtime validation next

The Rev.025 session confirmed dispatcher reachability. Next runtime targets for the ROPE gap:

```txt
breakpoint at 0x0013f7a8  (capture a1 when a3 == 0x13)
breakpoint at 0x001d37c8  (monitor state_id distribution)
```

Useful values to capture:

```txt
$3 before table load
$4 before jr
candidate_state_id
candidate_state_block_ptr
entity/context pointer
state block pointer
selected jump table entry
a1 at 0x0013f7a8 entry (which callback is being registered?)
```

Useful questions:

- Which state IDs appear during gameplay?
- Which internal block is reached most often?
- Does `candidate_state_id` change?
- Who writes `[candidate_state_block_ptr + 0x48]`?
- Does the dispatcher trigger during normal gameplay, death, capture, menu, or transition?

---

## Blog persona prompt maintenance

The file below is a required companion context for narrative/blog-style writing about this project:

```txt
prompt_persona_ico_reconstruction.md
```

Whenever a validated research revision changes the current understanding of the project, update this persona/blog prompt as part of the same work.

This is a project condition, not an optional cleanup task.

The prompt must stay aligned with the latest validated research while preserving the same caution rules:

- separate confirmed facts from hypotheses;
- do not turn narrative scenes into technical evidence;
- do not invent discoveries;
- prefer the newest validated research note when older notes conflict;
- keep the archaeology/digital-reconstruction tone grounded in reproducible evidence.

---

## Safety and legality

Do not include copyrighted game data in generated outputs.

Do not commit extracted proprietary binaries, ISO contents, or copyrighted assets.

Research notes should describe offsets, addresses, behavior, and methodology.

Do not distribute game files.

---

## Philosophy

This is a digital archaeology project.

Each offset is a fragment.

Each function is a buried room.

Each string is an inscription.

Each false positive is a common stone mistaken for an artifact.

The purpose is not to force a theory onto the binary.

The purpose is to let the binary constrain the theory.

Build an auditable chain of evidence.

Prefer clarity over speed.

Prefer conservative naming over exciting claims.

Prefer reproducible verification over impressive speculation.
