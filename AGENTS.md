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
research/elf/ghidra-rev070-callers-of-166028-and-rodata-init-table.md  (callers of 0x166028: main loop 0x101C80, scene init 0x1AF4A0, entry iter 0x1B76F8; 404-byte stride entity table; debug table at 0x613E00 with ClothInfo/CollisionOldProc strings and 0x168650)
research/elf/ghidra-rev069-vu0-ringbuffer-packet-builder-halfword-table-population.md  (VU0 packet builder 0x1D43F8, kick stub 0x117C40, 0x6AB080 writers, alternates constants)
research/elf/ghidra-rev067-consolidated-live-dispatch-model.md        (live dispatch consolidated model — slot table, callbacks, callers, alternate impl)
research/elf/ghidra-rev066-static-live-dispatch-callsite-map.md       (cold paths, GP slots, dispatch point confirmed)
research/elf/ghidra-rev064-cold-paths-and-live-dispatch.md            (live dispatch at 0x00166E10, cold paths, struct map)
research/elf/ghidra-rev039-cloth-domain-correction.md                 (cloth domain for 0x001d37c8/0x001d3a30)
research/elf/ghidra-rev037-remaining-callers-and-rope-gap.md          (static registration gap)
research/elf/ghidra-rev025-runtime-confirmed-caller-context.md         (runtime validation)
research/external/ico-splat-promoted-ranges-experiment.md              (verified splat ranges)
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
12. `research/ico-decomp-cross-reference-2026-05-14.md`
13. `research/elf/ghidra-rev039-cloth-domain-correction.md`
14. `research/external/sotc-tooling-relevance-survey.md`
15. `research/external/ico-rabbitizer-spimdisasm-dispatcher-check.md`
16. `research/external/ico-splat-minimal-experiment.md`
17. `research/external/ico-splat-promoted-ranges-experiment.md`
18. `research/external/ico-splat-adjacent-promoted-ranges-experiment.md`

Use Rev.039 and the ICO-decomp cross-reference as the current source of truth
for the domain of `0x001d37c8` / `0x001d3a30` when they contradict earlier
entity/gameplay interpretations. Use Rev.037 for the static registration-gap
analysis unless a later validated note supersedes it.

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

### External tooling front (2026-05-15)

A separate tooling investigation was opened to evaluate whether the public
`Fantaskink/SOTC` decompilation project can help this project from a process
standpoint.

Current findings:

- `research/external/sotc-tooling-relevance-survey.md` confirms SOTC is useful
  mainly as a **tooling/process reference**, not as semantic evidence for ICO.
- SOTC tooling patterns worth testing for ICO: `splat64[mips]`,
  Rabbitizer/spimdisasm decoding, Ninja build graph generation, map/first-diff
  workflow, SDK/library segmentation, and decomp.me compiler packaging.
- SOTC uses `ee-gcc2.96`; do **not** treat that as ICO's compiler. ICO remains
  provisionally tied to EE GCC `2.9-991111-01` and the flags above.
- `research/external/ico-rabbitizer-spimdisasm-dispatcher-check.md` independently
  revalidated the dispatcher/callback anchors with Rabbitizer:
  `0x001d3800 == sll $v1,$v1,2`, `0x001d3b04 -> 0x001d37c8`, and jump table
  `0x00618fb0` contains the five expected `.text` targets.
- `research/external/ico-splat-minimal-experiment.md` confirms `splat64[mips]`
  can split ICO USA `.text`, generate anchors such as `func_001D37C8` and
  `func_0013F7A8`, create `jtbl_00618FB0_main_text`, and carry the full ELF
  layout as defined segments when non-text/DVP regions are treated as `databin`.
- `research/external/ico-splat-promoted-ranges-experiment.md` confirms selected
  verified ranges can be promoted into separate asm files:
  `0x0013f3f0`, `0x0013f7a8`, `0x001d37c8`, `0x001d3a30`. It also corrected two
  naive range ends: `0x0013f3f0` must include the return path through
  `0x0013f630` plus padding to `0x0013f638`, and `0x0013f7a8` must include
  `jr $ra` / delay slot through `0x0013f7d4` (end marker `0x0013f7d8`).
- `research/external/ico-splat-adjacent-promoted-ranges-experiment.md` confirms
  the next adjacent verified ranges can also be promoted cleanly:
  `0x0013fc00`, `0x001d27a8`, `0x001d3b28`. It preserves the known static gap:
  `0x0013fc00` can dispatch slot `+0x48` callbacks, but visibly prepares only
  `a0`; `0x001d27a8` consumes a meaningful `a1`, so runtime capture remains
  required to identify the real `a1` source.
- The next tooling tests should stay small and auditable: promote only verified
  functions/ranges out of monolithic asm, run SDK/library recognition, check
  compiler package availability, and prioritize runtime capture for
  `0x001d27a8`'s `a1` source.

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

### 2026-05-15 tooling correction

The independent Rabbitizer check confirms the corrected instruction and table:

```txt
0x001d3800: sll $v1,$v1,2
0x001d3b04: jal 0x001d37c8
0x00618fb0: [0x001d3818, 0x001d3844, 0x001d391c, 0x001d39e0, 0x001d3a10]
```

Rabbitizer may render jump targets with a high `func_80...` prefix depending on
formatting; convert those back to the local project VA convention before
recording conclusions.

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

1. Cloth dispatcher model (`0x001d37c8` + 5 state blocks)
2. ROPE callback (`0x001d3a30` confirmed at runtime)
3. Callback registration chain (`0x0013f7a8` -> `0x0013f3f0` -> node+0x1c)
4. 5 callsites of registration function mapped; 3 excluded for ROPE
5. Compiler identified: EE GCC (Sony fork for R5900)
6. Domain corrected to cloth physics via ICO-decomp cross-reference
7. Dispatcher/callback byte-level anchors independently checked with Rabbitizer

The project has moved into a new analysis phase: **Live Dispatch System**.

The live scene init dispatcher at `0x00166E10` has been fully mapped (Rev.064-067):

- 17-entry slot table at `0x00282690` with 14 unique parametric callbacks (Group 1: position/rotation via `0x166258`, Group 2: orientation via `0x1667E0`)
- Two cold paths (`0x00167230`/`0x00167258`) as leaf fragments tail-calling `0x00166E10`
- Alternate implementation (`0x00169F80`/`0x0016A058`) with extra transform/matrix init
- Runtime pointer list at `0x006AAC80` (corrected from `0x006AAC00`)
- All 14 callbacks iterate a runtime-populated halfword table at `0x006AB080` (BSS)

Current objectives:

1. Runtime validation: capture hits at cold paths (`0x00167230`, `0x00167258`), main body (`0x00166E10`), and dispatch point (`0x00167020`)
2. Confirm which slot indices (a1=0..16) fire during gameplay vs cutscenes vs menus
3. Check if the alternate implementation is ever reached (no static path known)
4. Map the halfword table at `0x006AB080` population mechanism — writers confirmed at `0x00166D1C`/`0x00166D78` (same function as dispatcher)
5. Understand the semantic meaning of each slot
6. Investigate the 404-byte stride entity table at `0x005F2F98` indexed by world state

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

The current priority is understanding the live dispatch system (0x00166E10 and its 17-slot table).

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

The Rev.025 session confirmed cloth dispatcher reachability. The project has since completed static analysis for the **Live Dispatch System** (0x00166E10, Rev.064-067). Next runtime targets:

```txt
breakpoint at 0x00167230  (cold path A — capture a0, gp, gp-25856 value)
breakpoint at 0x00167258  (cold path B — capture a0, gp, gp-25852 value)
breakpoint at 0x00166E10  (main body — capture a0 context, a1 slot index)
breakpoint at 0x00167020  (dispatch — capture v1 callback target, a0-a2)
breakpoint at 0x001683B4  (wrapper for slot 0 — capture a0, a1, target from GP slot)
breakpoint at 0x00169F80  (alternate impl A — check if ever reached)
breakpoint at 0x0016A058  (alternate impl B — check if ever reached)
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
a2 + t0 at 0x00166D38 (values written to 0x6AB080)
```

Useful questions:

- Which slot indices (a1=0..16) appear during gameplay?
- Are the cold path slots ever swapped to the alternate implementation?
- Which Group 1 vs Group 2 callbacks fire?
- Does the halfword table at 0x6AB080 contain entity/object type indices?
- What is the real callback distribution at 0x00167020?
- Is the VU0 kick stub (0x117C40) ever reached during gameplay?
- Which function calls 0x00168650 with a0 != 0 (alternate selection)?

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
