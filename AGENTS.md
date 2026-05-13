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

At the current stage, the most important validated research note is:

```txt
research/elf/ghidra-rev023-dispatcher-table-resolution.md
```

---

## Important documents to read first

Before doing new analysis, read these files in this order if they exist:

1. `AGENTS.md`
2. `.local/key-concepts.md`
3. `.local/ai-context.md`
4. `key-concepts.md`
5. `research/elf/ghidra-rev023-dispatcher-table-resolution.md`
6. `research/elf/ghidra-rev022-dispatcher-ground-truth.md`
7. `research/elf/ghidra-rev021-continue-menu-pivot.md`
8. `research/elf/ghidra-rev018-state-transition-dispatch.md`
9. `research/elf/ghidra-rev019-state-resolver-caller-context.md`

Use Rev.023 as the current source of truth when it contradicts Rev.022.

---

## Current strategic status

The most important current discovery is the confirmed dispatcher at:

```txt
0x001d37c8
```

Rev.023 resolved the prior contradiction from Rev.022.

The real jump table is:

```txt
0x00618fb0
```

not:

```txt
0x00628fb0
```

The table contains 5 valid internal basic-block targets:

```txt
state 0 -> 0x001d3818
state 1 -> 0x001d3844
state 2 -> 0x001d391c
state 3 -> 0x001d39e0
state 4 -> 0x001d3a10
```

These are internal basic blocks inside `0x001d37c8`.

They are not separate function entry points.

This means the current confirmed model is:

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

The next intended research step is:

```txt
Rev.024 — Internal State Block Semantics
```

Objective:

Analyze the five internal state blocks reached by the dispatcher and classify what each state appears to do.

The next file should likely be:

```txt
research/elf/ghidra-rev024-internal-state-block-semantics.md
```

Only create a different file if explicitly instructed.

---

## Rev.024 target blocks

Analyze these internal blocks:

```txt
0x001d3818
0x001d3844
0x001d391c
0x001d39e0
0x001d3a10
```

For each block, identify:

- starting address;
- estimated ending address;
- main instructions;
- direct calls;
- indirect calls;
- memory reads;
- memory writes;
- offsets used;
- constants used;
- branches;
- whether it changes `candidate_state_id`;
- whether it accesses `candidate_state_block_ptr`;
- whether it accesses entity/context fields;
- whether it returns to the common epilogue;
- likely semantic role;
- confidence level.

Do not assign gameplay names without evidence.

---

## Preferred naming for Rev.024

Use neutral names first:

```txt
state_0_block
state_1_block
state_2_block
state_3_block
state_4_block
```

If the evidence supports cautious interpretation, use names like:

```txt
state_0_init_like
state_1_update_like
state_2_transition_like
state_3_cleanup_like
state_4_exit_like
```

Never use strong names unless directly supported by evidence.

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

## Runtime validation later

Runtime validation should come after Rev.024 unless explicitly requested earlier.

Useful runtime targets later:

```txt
breakpoint at 0x001d37c8
breakpoint at 0x001d380c
breakpoint at 0x001d3810
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
```

Useful questions:

- Which state IDs appear during gameplay?
- Which internal block is reached most often?
- Does `candidate_state_id` change?
- Who writes `[candidate_state_block_ptr + 0x48]`?
- Does the dispatcher trigger during normal gameplay, death, capture, menu, or transition?

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
