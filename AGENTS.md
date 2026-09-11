# AGENTS.md — ico-reconstruction

## Project identity

This repository is a reverse engineering project focused on building a **native PC port** of the PlayStation 2 game **ICO**.

The project follows a staged reconstruction approach:

1. **Verified reverse engineering** — recover symbols, function boundaries, source file mapping, data structures, runtime behavior, and engine architecture
2. **Byte-exact preservation** — preserve original PS2 functions as `.s` when C reconstruction is not yet possible; `.s` is documentation and ground truth, not the final form
3. **Semantic C++ reconstruction** — progressively convert verified functions into readable C++ suitable for native compilation
4. **Platform abstraction** — replace PS2-specific systems with portable equivalents (rendering, audio, input, filesystem, timing)
5. **Native runtime** — build a PC executable that runs reconstructed game logic against modern platform services

This is not a race to produce conclusions.

The goal is to build an auditable chain of evidence that enables a native PC port.

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

## Priority Rule

When choosing between two tasks, prefer the task that moves the project closer to a native PC port.

Priority order:

1. Confirm original architecture and source file ownership.
2. Recover structs and data layouts used by gameplay systems.
3. Convert small verified functions from `.s` to C/C++ when practical.
4. Identify PS2 platform dependencies that will need native replacements.
5. Design abstraction layers for input, filesystem, rendering, audio, timing, and resource loading.
6. Preserve byte-exact `.s` only when C reconstruction is blocked or not currently efficient.

Do not treat documentation-only discoveries as the final objective. Documentation is valuable when it enables reconstruction, portability, or native runtime design.

---

## Long-Term Goal: Native PC Port

The long-term goal of this project is to make a native PC port of ICO possible.

The current decompilation/reconstruction work is the foundation for this goal, but agents must distinguish between:

- verified PS2 reconstruction;
- byte-exact `.s` preservation;
- semantic C/C++ reconstruction;
- native PC runtime work.

The `main`/`master` branch remains the conservative source of truth for decompilation and reverse engineering.

The `native-port` branch is experimental and may contain platform abstraction work, native runtime scaffolding, and PC-specific prototypes.

Do not introduce native-port experiments into `main`/`master` unless they improve the decompilation/reconstruction project itself.

---

## Required toolchain (pre-flight check)

Before attempting ANY code generation, assembly, compilation, or scoring task,
the agent MUST verify that the following tools exist on the system. If any
critical tool is missing, **stop and report it** — do not improvise substitutes.

### Critical path (assembly/scoring pipeline)

| Tool | Purpose | Expected path | Install source |
|------|---------|---------------|----------------|
| **ee-gcc 2.9-991111-01** | Assemble .s files, compile C, byte-exact verification | `toolchain/ee-gcc2.9-991111-01/bin/ee-gcc` | `https://github.com/decompme/compilers/releases` → `ee-gcc2.9-991111-01.tar.xz` |
| **USA ELF** | Target binary for all analysis | `.local/extracted/SCUS_971.13.elf` | Extract via `tools/elf-extractor/elf_extractor.py` from `.local/iso/Ico (USA).bin` (LBA=25, size=5458886) |

### Python packages (pip)

| Package | Required by | Install |
|---------|-------------|---------|
| `capstone` | `asm_source_score.py`, `ee_gcc_compile.py`, `asmdiff.py`, `decompme_submit.py`, `symbol_reconcile/reconcile.py` | `pip3 install capstone` |
| `pyelftools` | `reconcile_full_pipeline.py`, `recover_unmatched_objects.py` | `pip3 install pyelftools` |

### Runtime capture (PCSX2)

| Tool | Purpose | Expected path |
|------|---------|---------------|
| **pcsx2-qt** (instrumented fork) | Runtime gameplay capture with breakpoints | `.local/pcsx2-ico-logpoints-fork/build-runtime/bin/pcsx2-qt` |
| **USA ISO** | Game image | `.local/iso/Ico (USA).bin` |

### Analysis tools (stdlib-only, always available)

These scripts need only Python stdlib — no external dependencies:

- `tools/elf_table_dump.py` — data table extraction
- `tools/elf-symbol-scan/elf_symbol_scan.py` — symbol table scanner
- `tools/mips-prologue-scan/mips_prologue_scan.py` — function prologue detection
- `tools/elf-extractor/elf_extractor.py` — ELF extraction from BIN/CUE
- `tools/runtime-probe-analyzer/runtime_probe_analyzer.py` — JSONL log analysis
- `tools/runtime-probe-analyzer/verify_runtime_probe_log.py` — log validation
- `tools/pal_usa_reconcile/parse_main_map.py` — MAIN.MAP parser
- `tools/pal_usa_reconcile/prioritize_unmatched_objects.py` — recovery queue ranker

### Optional (for PAL→USA reconciliation)

| Tool | Purpose | Status |
|------|---------|--------|
| **PAL ELF** (SCES_507.60) | PAL version for symbol reconciliation | Requires PAL ISO |
| **PAL MAIN.MAP** | Function/object map from PAL debug symbols | Requires PAL ISO |
| **PAL SRCFILE.TXT** | Source file provenance from PAL | Requires PAL ISO |

### R5900 assembler limitations

The musl cross-assembler (`mips64el-linux-musl-as`) is available at
`/tmp/mips64el-linux-musl-cross/bin/` as a fallback but does NOT support:

- `movn` / `movz` (conditional moves, funct=0x2D/0x2C) — emit as `.word`
- `mult` with accumulator selector (rd field = ac number) — emit as `.word`
- `bbit032` / `bbit132` (R5900 bit-test branches) — emit as `.word`
- COP1 compare instructions (`c.olt.s`, etc.) — emit as `.word`

For these instructions, use `.word 0xXXXXXXXX` with the raw encoding.

### Verification command

Run this before starting any task that depends on the toolchain:

```bash
# Check critical tools
test -x toolchain/ee-gcc2.9-991111-01/bin/ee-gcc && echo "ee-gcc: OK" || echo "ee-gcc: MISSING"
test -f .local/extracted/SCUS_971.13.elf && echo "USA ELF: OK" || echo "USA ELF: MISSING"
python3 -c "import capstone; print('capstone: OK')" 2>/dev/null || echo "capstone: MISSING"
python3 -c "import elftools; print('pyelftools: OK')" 2>/dev/null || echo "pyelftools: MISSING"
```

---

## Gameplay monitor agent (runtime sessions)

When the user says **"vamos jogar"**, **"vamos para mais uma sessão"**, **"abre o jogo"**,
or any similar phrase indicating intent to play ICO while the agent monitors,
the agent MUST follow this workflow:

### Pre-flight

1. Run `./tools/run-ico-pcsx2-logpoints.sh --check` to verify PCSX2 is available.
2. Read the most recent research note to understand current probe configuration.

### Launch

3. Run `./tools/run-ico-pcsx2-logpoints.sh` — it creates a unique session ID,
   a new JSONL file, and `.local/pcsx2-logs/current-session.env`.
4. Wait 3-5 seconds for PCSX2 to start.
5. Verify PCSX2 is running: `pgrep -f pcsx2-qt`.
6. Run `python3 tools/runtime-probe-analyzer/verify_runtime_probe_log.py .local/pcsx2-logs/current-session.jsonl`
   to confirm the log is valid (required sentinels: `elf_entry_sentinel`, `isys_gobj_init`, `ios_om_main`).
7. **Only release gameplay after the validator reports `status=valid`.**

### Monitoring (passive)

8. While the user plays, the agent does NOT intervene unless asked.
9. The JSONL log grows automatically — all probes fire into it.
10. If the user asks for analysis mid-session, run the validator and report event counts.

### Post-session analysis

11. When the user says **"para"**, **"parei"**, **"analyse os logs"**, or stops playing:
    - Run the validator one final time.
    - Count total events and unique labels.
    - Identify new world_state values not seen in previous sessions.
    - Identify new dispatch slot addresses not yet mapped.
    - Identify high-frequency functions not yet decompiled.
    - Write findings to `research/elf/ghidra-revNNN-runtime-session-*.md`.
    - Update `docs/backlog.md` with new completed items.
    - Commit and push if the user requests.

### Key addresses to watch

| Address | Label | What to capture |
|---------|-------|-----------------|
| `0x1B76F8` | `initSceneGObj` | scene_id, entry_count, entity types |
| `0x13F9D0` | `_iosOmMain` | slot_index, mask bits, event count |
| `0x13F3F0` | `isysGObjProcAdd_` | a1 (GObj ptr), a3 (add/remove) |
| `0x13FC00` | `iosOmCreateDL` | a1 (slot), mask bits |
| `0x1AF948` | `world_state_load` | room_id, transition count |
| `0x166E10` | `_Clip` | collision context |

### What NOT to do

- Do not open PCSX2 manually — always use `run-ico-pcsx2-logpoints.sh`.
- Do not modify probe addresses without documenting in a research note.
- Do not commit log files (they are in `.local/` which is gitignored).
- Do not interrupt gameplay unless the user asks.

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
research/elf/rev109-isysgobj-abi-consolidation.md  (canonical 32-bit GObj/ProcessNode ABI; four 8-entry head/tail tables; 32-bit mask-loop distinction; semantic C bridge)
research/elf/rev131-worldstate-boundary-dispicomisc-and-native-bridge.md  (CANONICAL BOUNDARY of world_state_load=0x80 + DispIcoMisc=0x1C8 split; byte-exact .s; native WorldStateLoader semantic bridge + CTest)
research/elf/rev130-hot-gaps-3-4-5-byte-exact.md  (all 5 hot-path gaps closed byte-exact: sister_callback_reg, CreateGObj, AllocGObjEntity, world_state_load, isysGObjProcRemoveUnlink; allocator contract + world_state dispatch table 0x5F2FB8)
research/elf/ghidra-rev099-isysgobj-lifecycle-and-ios-thread.md       (full isysGObj* lifecycle: init→alloc→add→dispatch→remove; ios/thread.c=thread creation; 36 byte-exact .s sources)
research/elf/ghidra-rev098-isysgobj-process-registration-and-dispatch.md  (isysGObjProcAdd_=488B central registration; _iosOmMain=17 slots matching runtime data; initSceneGObj=2088B connects descriptor table to isysGObj*)
research/elf/ghidra-rev097-isysgobj-clip-girlbrain-consolidation.md  (ARCHITECTURAL CORRECTION: _Clip is collision not dispatcher; isysGObj* is real game object system)
research/ghidra-exploration-2026-05-21.md  (2886 symbol import reveals real names: _Clip, execBombGeo, ItemGeo, _clipW*)
research/elf/ghidra-rev096-halfword-runtime-session-analysis.md  (offline Rev.095 review: main caller still dominates, second caller absent in this capture, fast path still inferred)
research/elf/ghidra-rev094-halfword-runtime-second-caller.md  (second halfword caller runtime-confirmed, fast path still needs direct probe)
research/elf/ghidra-rev093b-halfword-entry-runtime-validation.md  (halfword writer active in hot dispatch path)
research/elf/ghidra-rev093-three-investigations.md  (mask_set=ShockRequestBox_RequestCancel, dispatch table=compile-time .data)
research/elf/ghidra-rev089-runtime-session-rev086-worldstate-gp-m49b4.md  (world_state transitions, gp_m49B4=current entity work area, 6 room values mapped)
research/elf/ghidra-rev086-static-analysis-vtables-enveffect-cbroutine4-vblank.md  (final static analysis — behavior_fn, env_effect type matrix, cb_routine4, VBlank)
research/elf/ghidra-rev077-final-static-analysis.md                      (descriptor table, entry table, scene loader, VU0 queue, debug table, GP map)
research/elf/ghidra-rev075-init-fn-callback-dispatch-and-asm-handler-consolidation.md  (entity types, init_fn groups, callback masks)
research/elf/ghidra-rev073-main-loop-dispatch-chain-and-callback-corrected-masks.md  (12-step main loop, 17-slot dispatch)
research/elf/ghidra-rev102-isysgobj-girlbrain-ebrain-correction.md  (CORRECTION: GirlBrain real range = 0x0016xxxx, not 0x0019xxxx; eBrain/Generator in 0x0019xxxx; 15 new byte-exact .s files; 88 total)
research/elf/ghidra-rev103-isysgobj-runtime-session-yorda-bridge-save.md  (Runtime session: 86K events, 13 world_states, 8 BSS dispatch slots mapped, per-room thread assignment confirmed)
research/elf/ghidra-rev104-extended-runtime-session-dl-slots.md  (755K events, 20 world_states, 12 DL slots, a2/t0 register mapping, ws=0x0F dominant)
research/elf/rev162-pal-usa-fingerprint-validation.md  (PAL→USA reconciliation closed: 404 matches, 367 byte-verified USA VAs agree 100%; 8 formerly-ASM-ERR + 4 stubs verified byte-exact; 27/27 CTest)
research/elf/rev163-static-inventory-and-byteexact.md  (397 named functions without .s; 326 at 8..0x200 B; .s generated for 4 decompilation targets via gen_byteexact_asm.py)
research/elf/rev164-new-targets-byteexact-and-getenemydeflife-semantic.md  (617 pipeline byte-exact; GetEnemyDefLife semantic bridge + CTest; life += 0.5f corrected; 733 total .s)
research/elf/rev165-three-semantic-bridges.md  (3 more semantic bridges: holdRope, subEnemyCollision, girlForceFieldGeo; 4 semantic bridges total; 8192.0f constant correction; GFFG rounding pipeline confirmed, invented output write removed; 27/28 CTest)
research/elf/rev166-delegables-three-semantic-bridges.md  (subEnemyCollision delegables CLOSED: fn_14A100/fn_15BCC8/fn_203AA0 byte-exact .s + semantics + CTest; bit29/bit27 select table; VBlank 0x274EC0 countdown; dead mult stores; 736 total .s; 7 semantic bridges)
research/elf/rev167-first-inventory-semantic-bridges.md  (Rev.163 inventory first batch: actEnemyFlagOnDead/AP1JumpReq/actSt04bEne1Chk byte-exact .s + semantics + CTest; AP1 mask -2 clears bit0 not bit1; actSt04bEne1Chk 32-bit adjacent cells; 0x13FF88 shared sink path; 739 .s; 10 semantic bridges)
```

---

## Important documents to read first

Before doing new analysis, read these files in this order if they exist:

1. `AGENTS.md`
2. `.local/key-concepts.md`
3. `.local/ai-context.md`
4. `key-concepts.md`
5. `research/elf/rev109-isysgobj-abi-consolidation.md`
6. `research/elf/ghidra-rev099-isysgobj-lifecycle-and-ios-thread.md`
7. `research/elf/ghidra-rev098-isysgobj-process-registration-and-dispatch.md`
8. `research/elf/ghidra-rev097-isysgobj-clip-girlbrain-consolidation.md`
9. `research/ghidra-exploration-2026-05-21.md`
10. `research/elf/ghidra-rev096-halfword-runtime-session-analysis.md`
11. `research/elf/ghidra-rev094-halfword-runtime-second-caller.md`
12. `research/elf/ghidra-rev093b-halfword-entry-runtime-validation.md`
13. `research/elf/ghidra-rev093-three-investigations.md`
14. `research/elf/ghidra-rev089-runtime-session-rev086-worldstate-gp-m49b4.md`
15. `research/elf/ghidra-rev088-barrel-rope-woodbox0-decompilation.md`
16. `research/elf/ghidra-rev087-enemy1-boy-decompilation.md`
17. `research/elf/ghidra-rev086-static-analysis-vtables-enveffect-cbroutine4-vblank.md`
18. `research/elf/ghidra-rev085-death-validation-and-next-session-plan.md`
19. `research/elf/ghidra-rev084-runtime-validation-extended-session.md`
20. `research/elf/ghidra-rev077-final-static-analysis.md`
21. `research/elf/ghidra-rev073-main-loop-dispatch-chain-and-callback-corrected-masks.md`
22. `research/elf/ghidra-rev039-cloth-domain-correction.md`
23. `research/ico-decomp-cross-reference-2026-05-14.md`
24. `research/elf/ghidra-rev037-remaining-callers-and-rope-gap.md`
25. `research/elf/ghidra-rev025-runtime-confirmed-caller-context.md`
26. `research/external/sotc-tooling-relevance-survey.md`
27. `research/external/ico-rabbitizer-spimdisasm-dispatcher-check.md`
28. `research/external/ico-splat-promoted-ranges-experiment.md`
29. `research/elf/ghidra-rev102-isysgobj-girlbrain-ebrain-correction.md`
30. `research/elf/ghidra-rev103-isysgobj-runtime-session-yorda-bridge-save.md`
31. `research/elf/ghidra-rev104-extended-runtime-session-dl-slots.md`
32. `research/elf/ghidra-rev105-extended-session-25-worldstates-20-dl-slots.md`
33. `research/elf/rev142-p2o-vertex-layout-and-face-record-structure.md` (native-port, P1: PS2O geometry decode)
34. `research/native/rev154-verified-scene-tables.md` (native-port: 68 descriptors + 97 scene ranges drive 25 host GObjs for scene 0x0F)
35. `research/native/rev156-gifpacket-bridge-fidelity-and-p2-family-closed.md` (native-port: GifPacketBridge captures prim/path + viewport origin + host half-offset hook; p2 decodes via the same canonical Rev.151 rule — no discriminator needed; `tools/ps2o_family_analysis.py`)
36. `research/native/rev155-per-gobj-visual-composition.md` (native-port: GObjAttachmentStore — renderer iterates active isysGObj lists and draws each GObj's composition; boy via BoxMarker; round-robin GObj↔mesh is HOST heuristic pending PCSX2 capture)

Use Rev.039 and the ICO-decomp cross-reference as the current source of truth
for the domain of `0x001d37c8` / `0x001d3a30` when they contradict earlier
entity/gameplay interpretations. Use Rev.037 for the static registration-gap
analysis unless a later validated note supersedes it.

---

## Current strategic status

### Architectural correction (Rev.097): `isysGObj*` is the real game object system

The Ghidra exploration (Rev.096 + 5 headless scripts) revealed that the project's
central architectural assumption since Rev.062 was incorrect:

- **`_Clip` (0x166E10)** is NOT the main entity dispatcher — it is a collision/clipping
  function within `DispCollisionPC`, with only 2 static callers and 4 active `_clipW*`
  callbacks in its 17-slot configuration table (0x282690).
- **`isysGObj*` (0x13DDA0-0x141D18, 30 functions)** is the true game object processing
  system, managing callback registration, object allocation, traversal, and dispatch.
- The runtime PCSX2 slot distribution (15 active slots, 42.2M events) belongs to
  `_iosOmMain` (0x13F9D0), which has exactly 17 slots = 8 mask slots + 9 type slots.

### `isysGObj*` system architecture (Rev.098-099)

The game object lifecycle is now fully traced:

```
initSceneGObj (0x1B76F8, 2088B) — connects descriptor table (0x2A31B8, 68 entries)
                                     to isysGObj* via entry table (0x2A4C48, 3600 entries;
                                     valid descIdx<68 run is contiguous idx 0..3590; Rev.154)
  ↓
isysGObjInit (0x13DDA0) — zeros head/tail tables (0x281A70/0x281A90, 8 DLs)
  ↓
isysGObjAlloc (0x13E4D0) — allocates GObj array (stride 0x174, count at gp-0x4C4C)
  ↓
isysGObjAdd (0x13E8D8) — adds GObj to type-based display list
  ↓
isysGObjProcAdd_ (0x13F3F0, 488B) — central process/callback registration
  |                                  stride 0x94, priority-sorted linked list
  |                                  alloc class: ios/thread.c (thread control block)
  ↓
_iosOmMain (0x13F9D0, 534B) — 17-slot dispatcher
  ├── Pass 1: mask slots 0-7 (gp-0x6724 bitmask)
  └── Pass 2: type slots 0x13-0x1B (9 types)
  ↓
iosOmCreateDL (0x13FC00, 264B) — per-GObj display list dispatcher
  |                              32-slot table at 0x281AB0
  ↓
iosOmExeEachGObj (0x13FD10) — linked-list walker, fires N events per slot
```

Key structures:
- **GObj**: stride 0x174, fields at +0x28 (user data), +0x34 (next), +0x48 (callback),
  +0x4C (slot mask), +0x50 (type bits)
- **Process node (TCB)**: stride 0x94, fields at +0x1C (callback), +0x14 (priority),
  +0x10 (type mask), +0x18 (active flag)
- **DL heads 0x281AB0 / tails 0x281AD0**: 8 entries each; the separate
  `iosOmCreateDL` loop scans a 32-bit control mask
- **Thread table 0x6A6F30**: indexed by thread_id, stores process ptrs

### Canonical ABI and native bridge (Rev.109)

Rev.109 converts the verified `isysGObj*` layout into an auditable, portable
boundary without replacing the byte-exact `.s` ground truth:

- `src/core/gobj_abi.h` defines `IcoGObj` (`0x174`) and `IcoProcessNode`
  (`0x94`) using fixed-width fields and compile-time size/offset assertions.
- `src/core/isysgobj_semantic.c` reconstructs first-free allocation, ordered
  and head insertion, unlink, recycling, and DL-table init as semantic C. It
  is explicitly not claimed byte-exact.
- `0x281A70/0x281A90` are 8 primary-list heads/tails.
- `0x281AB0/0x281AD0` are 8 DL-list heads/tails.
- The `iosOmCreateDL` loop scans a 32-bit mask. This does not prove a physical
  32-entry head table; bits 8-31 remaining inactive is a probable invariant.
- On `native-port`, the ABI feeds a contiguous `GObjPool`, `ProcessNodePool`,
  and `IsysGObjRuntime` with add/remove/reuse, priority-ordered process
  registration, attached-process dispatch, callback mocks, invariants, and
  CTest coverage (5 tests). This is not a playable port.

### GirlBrain / eBrain correction (Rev.102)

**CORRECTION: Rev.097 misidentified `0x00191B70-0x0019C040` as "GirlBrain AI".**
Ghidra symbols verified via PAL→USA reconciliation show:

| Range | Actual identity | Verified functions |
|-------|----------------|-------------------|
| `0x0016xxxx` | **GirlBrain** (real) | `girlBrainMain_PositionUpdate`, `subGirlBrain_PulledUp`, `_girlBrainHide_MakeHidePoint`, `girlBrainHide_GoalTurn`, `girlBrainRunawaySearchPoint`, `girlBrainRunawayMoveByWay`, `subGirlBrain_Idle/Hesitate/Busy` |
| `0x00190xxx` | **eBrain** (entry AI) | `eBrainProcess`, `eBrainGetTargetGeneratorFromLabel`, `eBrainGetTarget`, `eBrainInit`, `eBrainStatusSet`, `eBrainSendMes`, `eBrainGetTargetGeneratorFromLabelStage` |
| `0x00191xxx-0x00193xxx` | **Generator/Enemy** | `CallEnemy`, `GeneratorDL`, `_MoveGV`, etc. (not yet reconstructed) |
| `0x00194xxx` | **Geometry utils** | Misc geometry helpers |

8 speculative eBrain functions (`eBrainGetStatus` through `eBrainTargetGenerator` at `0x191D20-0x192380`)
are kept as byte-exact `.s` even without Ghidra symbol verification.

### Byte-exact reconstruction status (Rev.167 — 739 of 739 .s verified byte-exact)

> **CORRECTION (Rev.130).** The Rev.116f count of 684/701 has been superseded.
> Six hot-path gaps were reconstructed byte-exact since Rev.116f and now the
> repository on disk holds **709 `.s` files** (Rev.128-130 added: sister_callback_reg,
> CreateGObj, CreateGObj_v, AllocGObjEntity, world_state_load,
> isysGObjProcRemoveUnlink). All counts below were re-verified against the USA ELF
> (`.local/extracted/SCUS_971.13.elf`) with the exact `assemble_and_verify`
> method (size = assembled length, target VA).
>
> **CORRECTION (Rev.131).** `world_state_load.s` (Rev.130, size 0x248) actually
> merged two functions. It was re-split into `world_state_load.s` (0x1AF948,
> 0x80, ends in the shared tail-jump `j 0x13d3f8`) plus a new
> `DispIcoMisc.s` (0x1AF9C8, 0x1C8, named by the Ghidra/PAL map). On-disk total
> is now **710 `.s` files** (entity 658, cloth 6, core 46). Both split `.s`
> verify 100% via `assemble_and_verify`.
>
> **CORRECTION (Rev.157).** The 4 "Divergent `.word`-only" files — `boyAI_sub_1435A0`,
> `eBrainProcess`, `girlBrain_sub_16F618`, `girlBrain_sub_16F620` — are now
> **byte-exact** through ee-gcc 2.9. Root cause was NOT a toolchain limitation:
> the ee-as injects nop padding into short (≤5 instruction) **backward** branches,
> which broke any `.s` that used in-text labels for those branches. The regenerated
> `.s` emit every branch/jal/mult/COP1 as raw `.word` and assemble byte-exact
> (sizes 0x130/0x258). See `research/elf/rev157-branch-padding-rootcause-and-4-byteexact.md`.
>
> **CORRECTION (Rev.162).** The 4 ASM-ERR files (`boyAI_sub_1562D4`, `1562DC`,
> `1562E0`, `1562E8`) were verified byte-exact via `.word 0x7ba80020` (ee-gcc 2.9
> encodes `ld.b $w0,-0x58($0)` identically). The 4 trivial stubs (`isysGObjActiveLink`,
> `isysGObjActiveDlLink`, `isysGObjProcPause`, `boyAI_sub_14BB08`) were verified
> byte-exact by assemble_and_verify at target VA. The C semantic signatures were
> corrected to match (void→u32, u32→void), and all 27 CTest pass.
>
> **CORRECTION (Rev.164).** Core `.s` grew 46→64 (Rev.162 added 8 core fixes);
> entity `.s` grew 658→663 (Rev.164 added GetEnemyDefLife + 4 new decompilation
> targets).
>
> **CORRECTION (Rev.166).** Core `.s` grew 64→67 (3 new `fn_14A100`, `fn_15BCC8`,
> `fn_203AA0` — the subEnemyCollision delegables, all byte-exact). Total on-disk:
> **736 `.s` files** (entity 663, cloth 6, core 67). All 736 verified byte-exact.
>
> **CORRECTION (Rev.167).** Entity `.s` grew 663→666 (3 new named functions
> from the Rev.163 inventory: `actEnemyFlagOnDead` 0x15D5F0, `AP1JumpReq`
> 0x1AE3B0, `actSt04bEne1Chk` 0x203A10, all byte-exact). Total on-disk:
> **739 `.s` files** (entity 666, cloth 6, core 67). All 739 verified byte-exact.

| Step | Count | Method |
|------|-------|--------|
| Pipeline functions | 617 | `asm_source_score.py --all` → 617/617 byte-exact (0 failures) |
| Other `.s` (outside `TARGET_FUNCTIONS`) | 122 | byte-exact via `assemble_and_verify` at target VA |
| **Total byte-exact `.s`** | **739** / 739 (100%) | verified against USA ELF |

Not byte-exact / not verified: **0** (all 739 verified). The former ASM-ERR and
trivial stub categories are closed as of Rev.162.
`girlBrain_sub_16F618`, `girlBrain_sub_16F620`) are now byte-exact through
ee-gcc 2.9 (Rev.157); the `.word` fallback there is a generator choice (backward
branch padding avoidance), not a toolchain necessity.

`asm_source_score.py --all` remains the authoritative pipeline for the 612
`TARGET_FUNCTIONS`. The 4 Rev.157 files are byte-exact via
`assemble_and_verify` at target VA (sizes 0x130/0x258) and sit outside the
automated scoring pipeline because the pipeline emits labels for in-range
branches, which the ee-as backward padding would corrupt.

Plus entity/cloth functions as byte-exact C source (`.c` files).
Plus 10 semantic C bridges with CTest coverage: `getEnemyDefLife`, `holdRope`,
`subEnemyCollision`, `girlForceFieldGeo`, the 3 subEnemyCollision delegables
(`fn_14A100`, `fn_15BCC8`, `fn_203AA0` — Rev.166), and the 3 Rev.167
inventoried handlers (`actEnemyFlagOnDead`, `AP1JumpReq`, `actSt04bEne1Chk`).

Files in `src/entity/asm/` (663), `src/cloth/asm/` (6), `src/core/asm/` (67).

### Verified facts (Rev.038-099)

- **Descriptor table** at 0x2A31B8: 68 entity types, stride 0x64. Only 12/68 have init_fn.
- **Entry table** at 0x2A4C48: 3600 entries (512 understates), stride 0x4C.
  Maps scene objects to descriptors. Valid `descIdx<68` run is contiguous
  idx 0..3590 (Rev.154). Field slice `+0x48` bits[16:14] = per-entry listId,
  `+0x47` low 5 bits = gobjType, `+0x40` = u16 processArgument_40,
  descriptor `+0x44` = gate (0 skips GObj creation).
- **initSceneGObj** (0x1B76F8, 2088B): connects entry table to isysGObj* system.
- **Two independent entity systems**: callback_register (52 scene objects via 28 init_fn)
  and live dispatch (8 core entities, 20 ctx/frame).
- **17-slot dispatch table** at 0x282690 = compile-time `.data` for `_Clip` collision config.
  Only 4 active `_clipW*` callbacks (slots 0,4,8,12). NOT the runtime slot source.
- **`_iosOmMain` has 17 slots** (8 mask + 9 type) matching runtime slot distribution.
- **Mask system** (FUN_0x13ed40) = ShockRequestBox_RequestCancel, loading-only, bit 0 only.
- **Halfword table** at 0x006AB080 = 32×32 spatial hash grid for collision clipping.
  Active at 0x166BB0 (hot path). Second caller at 0x0016828C runtime-reachable.
  Single-cell fast path at 0x166DFC still inferred.
- **Scene loader** in `kanban.c` (GP=0x27A7A8): 21-stage jump table at 0x616FD0,
  only 7 unique handlers. Functions: `kanbanReqAdd`, `kanbanExec`, `initSceneGObj`,
  `la_load_processing`, `la_switching_stage`, `HotInitSceneObjects`, `MoveNextStage_Clear`.
- **Main loop**: `vblankHandler` (0x1BDE48) → `ACTGame` (0x1A63E0) →
  `backStageProcessMain` (0x1A05D0) / `stage_ApplyData` (0x1A2A1D8) / `kanbanExec`.
- **Thread system** from `ios/thread.c`: TCB stride 0x94, priority-sorted ready queue,
  thread table at 0x6A6F30, counter at gp-0x6740.

### Confirmed module structure (from Ghidra symbols)

| Module | Path | GP | Evidence |
|--------|------|----|----------|
| Core engine | `src/fumi/` | — | vblank, IO, CDVD, heap strings |
| Item/Physics | `src/sugipon/item.c` | — | Assertion "src/item.c":434 |
| Field collision | `src/sugipon/fieldCollision.c` | — | Assertion line 533 |
| Scene loading | `src/sugipon/kanban.c` | 0x27A7A8 | GP + kanban* names |
| Cloth physics | `src/sugipon/clothAnimation.c` | — | execBombGeo, ItemGeo, GetCloth4D |
| BOY handler | `src/omori/boy.c` | — | String "boy.c" |
| ENEMY1 handler | `src/omori/enemy1.c` | — | String "enemy1.c" |
| GirlBrain AI | `src/omori/` | — | eBrain*, GirlBrain* names |
| Camera/Render | `src/omori/` + `src/seki/` | — | isysGObjMoveCameraDL |

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

### 2026-05-21 correction — Ghidra deep exploration: isysGObj*, _Clip collision system, main loop

The Ghidra headless exploration (5 Java scripts) revealed the true architecture of several previously misunderstood systems.

**Correction: `_Clip` is NOT the main entity dispatcher**

The `_Clip` function at `0x00166E10` is a collision/clipping function. It has only 2 static callers, both from `DispCollisionPC` at `0x00166A10`. The 17-slot dispatch table at `0x282690` is the clipping configuration table, not a general entity dispatcher. The runtime slot distribution (15 active slots in PCSX2 capture) does NOT correspond to `_Clip`'s active callbacks (only 4 active: slots 0/4/8/12 with real callbacks).

**The actual entity dispatcher: `isysGObj*` system (30 functions)**

The `isysGObj*` / `iosOm*` family at `0x0013DDA0-0x00141D18` is the true game object processing system:

| Function | Address | Role |
|----------|---------|------|
| `isysGObjProcAdd_` | `0x0013F3F0` | Callback registration (488 bytes) |
| `iosOmExeEachGObj` | `0x0013FD10` | Object traversal dispatcher |
| `isysGObjInit` | `0x0013DDA0` | Init system |
| `isysGObjAlloc` | `0x0013E4D0` | Object allocation |
| `isysGObjRemove` | `0x0013E548` | Object removal |
| `iosOmCreateDL` | `0x0013FC00` | DL/slot creation |

The earlier "main_dispatcher" / "callback_register" / "mask_set" nomenclature should be understood as parts of this `isysGObj*` system, not standalone entity dispatch.

**Correction: execBombGeo 5-state jump table targets confirmed**

The jump table at `0x00618FB0` contains these 5 targets:
- `0x001D3818` (state_0)
- `0x001D3844` (state_1)
- `0x001D391C` (state_2)
- `0x001D39E0` (state_3)
- `0x001D3A10` (state_4)

**GirlBrain state machine discovered (corrected by Rev.102)**

**NOTE: Rev.102 corrected the range.** The 30+ symbols at `0x00191B70-0x0019C040` are NOT GirlBrain — they are eBrain (entry AI) at `0x00190xxx`, Generator/Enemy at `0x00191xxx-0x00193xxx`, and Geometry utils at `0x00194xxx`. The real GirlBrain functions are at `0x0016xxxx`.

Rev.097 functions included: `eBrainSystemInit` (now `eBrainGetTargetGeneratorFromLabelStage`), `eBrainGetStatus`, `eBrainSetFlag`, `eBrainMovePos`, `eBrainMotionSe`, `eBrainPursuit`, `eBrainAvoid`, `eBrainReturnInit`, `eBrainTargetGenerator` variants.

**Main loop call graph confirmed**

The main loop chain: `vblankHandler` (0x1BDE48) → `ACTGame` (0x1A63E0) → `backStageProcessMain` (0x1A05D0) / `stage_ApplyData` (0x1A2A1D8) / `scene loader` (0x1B76F8-0x1B81A8) / `kanbanExec` (0x1B05A8)

Scene loader confirmed in `kanban.c` with GP=0x27A7A8 containing functions: `kanbanReqAdd`, `kanbanInit`, `kanbanReqAllDel`, `kanbanExec`, `la_load_processing`, `la_switching_stage`, `initSceneGObj`, `HotInitSceneObjects`, `MoveNextStage_Clear`.

**Runtime slot distribution does NOT match _Clip dispatch**

The PCSX2-measured 15-slot distribution (42.2M events) cannot correspond to the `_Clip` dispatch table which has only 4 active callback entries (slots 0, 4, 8, 12 with `_clipW*` functions; slots 1-3, 5-7, 9-11, 13-15 have no callback). The runtime slot data likely belongs to the `isysGObj*` processing system (via `iosOmCreateDL` / `iosOmExeEachGObj`), not the collision pipeline.

**C source file origin from Ghidra symbols**

Ghidra strings reveal real source file paths:
- `src/sugipon/item.c` — BARREL/ROPE physics (hC assertions at line 434)
- `src/sugipon/fieldCollision.c` — init_fn assertions (line 533)
- `src/sugipon/kanban.c` — scene loader (GP=0x27A7A8)
- `src/omori/boy.c` — BOY handler
- `src/omori/enemy1.c` — ENEMY1 handler
- `src/fumi/` — core engine (vblank, IO, CDVD)
- `src/seki/` — rendering pipeline

**.s assembly files renamed to match real symbols**

- `cb_routine2.s` → `ItemGeo.s`
- `cloth_dispatcher.s` → `execBombGeo.s`
- `fn_1D3DD8.s` → `ReviveAllCarryableItems.s`
- `fn_1D2550.s` → `HoldItem.s`
- `sub_1D2650.s` → `avoidInsideOfWall.s`
- `sub_1C1C48.s` → `synchronizeMotionOutputOriginForGirl.s`
- `sub_1C1EA8.s` → `boy_dispCrown.s`

C source declarations and comments updated to match. Research notes for the 5 Ghidra exploration passes written (ghidra-exploration, deep-exploration, dispatch-table, final-exploration, full-system).

---

### 2026-05-21 correction — Ghidra symbol import reveals real function names (original)

The 2886 PAL→USA reconciled symbols were loaded into Ghidra headless and
revealed the actual function names for key addresses. Several speculative
project names are incorrect:

| Address | Speculative name | Real symbol name |
|---------|-----------------|------------------|
| `0x00166E10` | "main_dispatcher" | `_Clip` |
| `0x00166BB0` | "halfword writer entry" | (inline in `_Clip`) |
| `0x00166DFC` | "halfword fast path" | (inline in `_Clip`) |
| `0x00167020` | "dispatch_point" | (inline in `FUN_00166FD0`) |
| `0x001D37C8` | "cloth_dispatcher" | `execBombGeo` |
| `0x001D3A30` | "cb_routine2" | `ItemGeo` |
| `0x0013F7A8` | "callback_register" | `FUN_0013f7a8` |
| `0x0013ED40` | "mask_set" | `FUN_0013ed40` |
| `0x00168DA8` | "slot 0 callback" | `_clipWDebug` |
| `0x00168ED0` | "slot 3 callback" | `_clipW` |
| `0x001692F0` | "slot 7 callback" | `_clipWR` |
| `0x00169440` | "slot 15 callback" | `_clipWField` |

**What this means:**
- The "main_dispatcher" / "live dispatch" system at `0x00166E10` is actually
  `_Clip` — a collision/clipping function, not a general entity dispatcher.
- The 17-slot dispatch table at `0x282690` contains `_clipW*` functions
  (clipping variants: debug, wall, wall-ref, wall-field). The table has a
  repeating 16-byte structure with valid callback addresses at offsets
  +0x0C, +0x1C, +0x2C, +0x3C; slots 0,4,8,12,16 contain flag values
  (`0x00000001`), slots 1-2,5-6,9-10,13-14 are `0x00000000`.
- `execBombGeo` at `0x001D37C8` is a geometry function with a 5-state jump
  table — likely cloth physics for bomb/explosion geometry, consistent with
  the earlier cloth-domain identification.
- `ItemGeo` at `0x001D3A30` is a geometry function for items.
- Halfword probe targets (`0x166BB0`, `0x166DFC`, `0x167020`) are inline blocks
  within larger functions, not separate function entries.

---

## Current confirmed dispatcher model

> **Note:** This section describes the `execBombGeo` (0x001D37C8) cloth geometry
> dispatcher with 5 internal states. This is a **geometry/physics subsystem**,
> not the main entity dispatcher. The main game object system is `isysGObj*`
> (see [Current strategic status](#current-strategic-status)).

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

The static analysis phase (Rev.001-037), runtime validation phase (Rev.064-075),
tee-gcc scoring pipeline (Rev.090-091), Ghidra headless exploration
(Rev.096-097), isysGObj* lifecycle analysis (Rev.098-099), and canonical
GObj/ProcessNode ABI bridge (Rev.109) are complete.

### Required runtime-first reconstruction sequence

For work informed by a gameplay session, use this order:

1. **Runtime capture first.** Launch and validate the instrumented PCSX2
   session, then let the JSONL probes capture the observed execution.
2. **Analyze the captured evidence.** Identify new world states, object/process
   activity, dispatch slots, callsites, and other reproducible runtime facts.
3. **Decompile/reconstruct from those facts.** Use the runtime observations to
   choose and constrain the next static-analysis or byte-exact reconstruction
   target. Keep confirmed behavior distinct from hypotheses.
4. **Feed validated results into the native port.** Only after the original
   behavior and data contract are sufficiently evidenced should a semantic
   native implementation or platform abstraction be added.

During an active gameplay session, do not treat unrelated native-port work as
the next priority merely because the capture is running. The capture is the
primary source of new evidence; native-port work follows the validated
reconstruction it enables. The user may explicitly request an exception.

### Current score status (Rev.167 — 739 of 739 .s verified byte-exact + 10 semantic bridges)

See the "Byte-exact reconstruction status" section above for the authoritative
counts. Summary: **739 of 739 `.s` verified byte-exact against the USA ELF**
(100%). The former ASM-ERR (4) and trivial stub (4) categories are closed as
of Rev.162. Core `.s` grew 46→64 (Rev.162), 64→67 (Rev.166); entity `.s` grew
658→663 (Rev.164), 663→666 (Rev.167). Rev.164 added the first gameplay-domain
semantic C bridge (`ico_semantic_getEnemyDefLife`) with CTest coverage;
Rev.165 added three more (`ico_semantic_holdRope`,
`ico_semantic_subEnemyCollision`, `ico_semantic_girlForceFieldGeo`); Rev.166
closed the subEnemyCollision delegables (`ico_semantic_fun14A100`,
`ico_semantic_fun15BCC8`, `ico_semantic_fun203AA0` — byte-exact `.s` +
semantics + CTest); Rev.167 added the first Rev.163-inventory batch
(`ico_semantic_actEnemyFlagOnDead`, `ico_semantic_AP1JumpReq`,
`ico_semantic_actSt04bEne1Chk`). Pipeline TARGET_FUNCTIONS is now 617/617
byte-exact. CTest: 27/28 (only headless `opengl_backend` fails — the
pre-existing baseline).

| Step | Count | Method |
|------|-------|--------|
| Pipeline functions | 617 | `asm_source_score.py --all` |
| Other `.s` (outside `TARGET_FUNCTIONS`) | 122 | `assemble_and_verify` at target VA |
| **Total .s files** | **739 / 739** | verified byte-exact |

Plus entity/cloth functions as byte-exact C source (`.c` files).
Plus 10 semantic C bridges with CTest coverage (see the score-status section).

### MAIN.MAP / recovery-pass update (2026-05-22)

- `MAIN.MAP` now provides 4368 function records grouped into 208 objects.
- Range reconciliation currently validates 415 functions across 65 objects with LOW/MEDIUM confidence.
- 143 objects had no initial seed; the new whole-text `op_seq` recovery pass recovered 62 of them, adding 168 validated functions.
- 81 objects remain unrecovered and need either a second anchor function or a source-file join from `SRCFILE.TXT`.
- `SRCFILE.TXT` provenance is now joined into the symbol map for 199 rows, and `main_map_functions_source.csv` carries 61 source files.
- The current final summary note is [`research/pal-usa/pal_usa_final_summary_2026-05-22.md`](/home/peter/Documentos/repos/ico-reconstruction/research/pal-usa/pal_usa_final_summary_2026-05-22.md).

### Scoring pipeline (Path B — assembly)

- `tools/asm_source_score.py`: converts target ELF function → GCC-style .s source → assembles with ee-gcc → verifies byte-exact match (no LCS/normalizer needed)
- Byte-level verification: compares .text section bytes against target ELF at declared VA. Zero tolerance.

### EE assembler constraints discovered

- **Register names**: numeric only (`$s0` → `$16`, `$ra` → `$31`). ABI names rejected.
- **Float registers**: MUST keep `$fN` prefix (no `$f12` → `$12`).
- **COP1 compares** (`c.olt.s`, etc.): unsupported by EE assembler → emit raw bytes via `.word`.
- **R5900 `mult $acN`**: `$ac3` → `$3` (rd field encodes accumulator).
- **R5900 `bbit032`**: unsupported → emit raw bytes via `.word`.
- **Very short backward branches**: the ee-as injects nop padding when a branch
  target label lies within ≤5 instructions *before* the branch (observed for
  `bne`, `beq`, `bnel`, `beqz`, `b`, `blez`, ...), regardless of
  `.set noreorder/nomacro/noat` and of label naming (symbolic vs `1:` numeric).
  dist ≥ 6 assembles clean. Fix: emit such branches as `.word` (Rev.157 root
  cause of the 4 former "divergent .word-only" files; see
  `research/elf/rev157-branch-padding-rootcause-and-4-byteexact.md`).
- **Branch labels**: GAS numeric local labels (`1:`/`1f`/`1b`) required. `.L` prefixed labels create `BFD_RELOC_16_PCREL_S2` relocations that fail.
- **`.set noreorder`/`.set nomacro` required**: prevents assembler from expanding pseudo-ops.
- **`.set noat`**: required for functions using `$at` (`$1`).
- **Branch targets outside function range**: emit `.word` with raw bytes (assembler can't compute PC-relative offset for undefined symbols).
- **`b` single-operand format**: unconditional branch `b target_addr` has no comma, unlike conditional branches `beq $r1,$r2,target`.

### Compiler flags (archived — no longer relevant for asm)

The old C-based compiler flag investigation is archived. All 26 asm functions bypass the C compiler entirely.

### Runtime-verified slot distribution (Rev.105 — 20 DL slots)

| Slot | Address | a2/t0 Index | Count | % | World State |
|------|---------|-------------|-------|---|-------------|
| B | 0x6782F8 | 0x1A | 454,221 | 61.4% | 0x09, 0x0E, 0x0F |
| NEW-4 | 0x678AA8 | 0x1D | 52,332 | 7.1% | 0x11 only |
| E | 0x679258 | 0x20 | 52,315 | 7.1% | — |
| D | 0x678FC8 | 0x1F | 35,679 | 4.8% | 0x10 |
| NEW-1 | 0x678818 | 0x1C | 28,267 | 3.8% | 0x0D only |
| F | 0x6794E8 | 0x21 | 26,926 | 3.6% | — |
| NEW-2 | 0x67EE98 | 0x44 | 25,872 | 3.5% | 0x0B only |
| G | 0x679778 | 0x22 | 18,753 | 2.5% | — |
| I | 0x67C308 | 0x33 | 12,940 | 1.8% | — |
| NEW-3 | 0x67C598 | 0x34 | 7,248 | 1.0% | 0x13 only |
| C | 0x678D38 | 0x1E | 6,845 | 0.9% | — |
| NEW-5 | 0x679F28 | 0x25 | 5,293 | 0.7% | 0x12 only |
| NEW-6 | 0x67CAB8 | 0x36 | 3,798 | 0.5% | 0x14 only |
| A | 0x677DD8 | 0x18 | 1,567 | 0.2% | — |
| J | 0x67E458 | 0x40 | 461 | 0.1% | — |
| H | 0x67A968 | 0x29 | 150 | <0.1% | — |

**Key findings (Rev.105):**
- **25 unique world_states** mapped (up from 20 in Rev.104)
- **20 DL slot addresses** in BSS (up from 16)
- **Each new world_state (0x10-0x14) has a unique DL slot** — per-area dispatch pattern
- **ws=0x0F** dominates: 429,907 ios_om_main events (58.1%), single slot B dispatch
- **Anomalous a2 values** (0x31C383B0, 0x31CA06F0) — possible overlay slots
- **352 .s functions** all at 100% byte-exact match

### Current objectives

1. ~~Runtime: probe mask bit 0 during cutscene transitions~~ **DONE (Rev.085)**
2. ~~Runtime: probe gp+0x6F60 (world_state)~~ **DONE (Rev.089)**
3. ~~Runtime: probe halfword table writers~~ **DONE (Rev.093b/Rev.094): active hot-path rasterizer; fast path still needs direct probe**
4. ~~Runtime: probe gp-0x49B4~~ **DONE (Rev.089): current entity work area ptr**
5. ~~Investigate slot 0 callback 0x168DA8~~ **DONE (Rev.084)**
6. ~~Investigate env effect table~~ **DONE (Rev.086)**
7. ~~Study cb_routine4 pattern~~ **DONE (Rev.086)**
8. ~~Analyze vtable dispatch~~ **DONE (Rev.086)**
9. ~~Decompile ENEMY1/BOY/BARREL/ROPE/WOODBOX0~~ **DONE (Rev.087-088)**
10. ~~Populate historia.md~~ **DONE (Rev.089)**
11. ~~Download and install ee-gcc 2.9-991111-01 locally~~ **DONE (Rev.090)**
12. ~~**Fix fn_1CE5F8 decompilation matching** — frame 0x40→0x30, delay slot nops via asm barriers.~~ **DONE (Rev.090): 72.37% structural match**
13. ~~**Batch remaining 6 decompiled handlers** through the local scoring pipeline~~ **DONE (Rev.091): all 37 functions compile and score; 0 compile errors (was 28)**
14. ~~Fix 28 compile errors across entity/cloth .c files~~ **DONE (Rev.091): C89 compat — u64 typedef, compound literals, declaration ordering, void→correct return types**
15. ~~**Improve boy_hC scoring** via li expansion + ori→addiu normalizer + constant fixes~~ **DONE (Rev.091g): 62.62% → 76.64%**
16. ~~**Convert all 26 LOW functions to byte-exact .s assembly (Path B)**~~ **DONE (Rev.092): all 38 functions at 100%**
17. ~~**Update docs backlog, AGENTS.md for Path B milestone**~~ **DONE (2026-05-18)**
18. ~~**Investigate mask_set (0x13ED40)**~~ **DONE (Rev.093): ShockRequestBox_RequestCancel, loading-only, bit 0 only**
19. ~~**Investigate dispatch table population (0x282690)**~~ **DONE (Rev.093): compile-time .data, no runtime populator**
20. ~~**Probe halfword writer at function entry (0x166BB0)** — move probe from SH to entry point~~ **DONE (Rev.093b/Rev.094): active hot-path rasterizer; second caller observed**
21. **Directly probe halfword fast path (0x166DFC)** — offline analysis strengthened the inference; direct runtime probe still required
22. **Instrument second halfword caller (0x0016828C/0x00168294)** — capture caller-side state, final counter, and `world_state_raw`; absent in Rev.095 capture
23. Fix dispatch_point slot index capture for next runtime session
24. Reposition world_state_load probe to capture room init_fn
25. Deploy memory watchpoint on VBlank counter 0x274EC0
26. ~~**Load 2886 verified symbols into Ghidra** — splat YAML + headless script applied all labels~~ **DONE (2026-05-21)**
27. ~~**Rev.097: Ghidra exploration — isysGObj* correction, _Clip=não é dispatcher, GirlBrain discovery**~~ **DONE (2026-05-21)**
28. ~~**Rev.098: isysGObjProcAdd_ = 488B central registration; _iosOmMain = 17 slots matching runtime; initSceneGObj = 2088B bridge**~~ **DONE (2026-05-21)**
29. ~~**Rev.099: full isysGObj* lifecycle (init→alloc→add→dispatch→remove); ios/thread.c = thread system; 36 .s byte-exact for core**~~ **DONE (2026-05-22)**
30. ~~**Update AGENTS.md for Rev.097-099 architectural correction** — incorporating isysGObj* system, _Clip correction, ios/thread.c~~ **DONE (2026-05-22)**
31. ~~**Update backlog.md, docs/system-feature-flows.md, docs/data-model.md** for the corrected architecture~~ **DONE (2026-05-22)**
32. ~~**Rev.100: GObj struct header, dispatch doc, 4 GirlBrain .s, BSS discovery**~~ **DONE (2026-05-22)**
33. ~~**Rev.101: +4 GirlBrain .s (11 total), label cleanup, entity-state-blocks.md, initSceneGObj analysis**~~ **DONE (2026-05-22)**
34. ~~**Rev.102: GirlBrain/eBrain range correction (0x0016xxxx=GirlBrain, 0x0019xxxx=eBrain/Generator); 15 new .s (10 GirlBrain + 7 eBrain); 8 speculative eBrain preserved; 6 stale .s files removed; YAML rewritten with correct USA file offsets**~~ **DONE (2026-05-22)**
35. ~~**Rev.105: Extended session — 25 world_states, 20 DL slots, 352 functions at 100%, physics table fully covered**~~ **DONE (2026-08-25)**
36. ~~**Rev.109: canonical `IcoGObj`/`IcoProcessNode` ABI, 8-vs-32 table correction, and semantic C core**~~ **DONE (2026-08-26)**
37. ~~**Native bridge: contiguous GObj pool, ordered lists, remove/reuse, callback mocks, and CTest coverage**~~ **DONE on `native-port` (2026-08-26)**
38. ~~**Native engine: ProcessNode/TCB pool, priority registration, removal, attached dispatch**~~ **DONE on `native-port` (2026-08-26)**
39. ~~**Native engine: iosOmCreateDL slot dispatch and type-based routing** — `dispatchTypeSlots()` (type slots 19-27 per GObj), `dispatchAll()` = `dispatchActiveLists` + `dispatchTypeSlots`, `slot_dispatch_test.cpp` with full coverage. Fixed critical `assert(runtime.initialize(...))` UB where NDEBUG stripped the init call in Release mode causing SIGSEGV.~~ **DONE (2026-08-26)**
40. ~~**Native engine: RenderBackend abstraction** — PS2 GIF/GS rendering pipeline modeled as `RenderBackend` interface with textures, render targets, blend/depth/alpha test, sprites (flat + gouraud), primitives, indexed draw, render passes per display list, double-buffer swap. `Matrix4x4` math library. `Rev.110` research note mapping ~200 rendering functions across 12 modules. `render_backend_test` with 7 tests (all passing).~~ **DONE (2026-08-26)**
41. ~~**Native engine: rendering pipeline** — TM2 texture loading (PSMCT32/CT24/CT16/PSMT8/PSMT4, CLUT, GS page swizzle), GIF command buffer (tag parsing, state tracking, quad accumulation), GIF executor (bridges command buffer to RenderBackend), OpenGL backend (GLX windowing, GL 3.3 core/fallback, batch renderer, FBO, shaders, test mode). 47 new tests across 4 files. All 10 test targets pass. Fixed PSMT4 nibble order.~~ **DONE (2026-08-26)**
42. ~~**Native engine: `GifPacket.*` bridge to GIF command-buffer model (Rev.156)** — `GifPacketBridge` now captures `prim`/path (`currentPrim()`/`currentPath()`), honors `setDrawEnvironment` viewport origin x/y, and applies an explicit host-side half-offset (`setHalfOffset`) to every `*Offset` emit variant + `draw2DUVStripG` (default 0 = identity). Byte-exact `0x8000` in XYZ2 packing verified to appear in BOTH base and Offset `.s` (`gif_MakeSprite`, `gif_Draw2DStripG`, `gif_Draw2DUVStripG`, `gif_SpriteSensitive`) — so NOT a verified Offset discriminator; kept as documented host hook, not a byte claim. Also closed the Rev.151 "p2 needs a discriminator" item: `tools/ps2o_family_analysis.py` + fixture test prove p2 flags u16[0]=0/1 decode with the SAME canonical Rev.151 rule (p2: 3,562 headers, 15,005 records, 4,844/4,844 UV coverage, bnd_ratio 0.126 = Rev.144 M-A).~~ **DONE (2026-09-09)**
43. ~~**Rev.128-130: close all 5 hot-path byte-exact gaps** — `sister_callback_reg` (0x13F778), `CreateGObj`/`CreateGObj_v` (0x240D40/0x240EA0), `AllocGObjEntity` (0x19F310), `world_state_load` (0x1AF948), `isysGObjProcRemoveUnlink` (0x13F638); corrected Rev.099 size of `isysGObjKindTableAdd` (0xDC→0xE0); 687/709 `.s` byte-exact.~~ **DONE (2026-09-05)**
44. ~~**Rev.131: `world_state_load` boundary correction** (0x248 merged two functions → `world_state_load` 0x80 + `DispIcoMisc` 0x1C8, both byte-exact) — plus native `WorldStateLoader` semantic bridge with `world_state_loader_test` CTest; 688/710 `.s` byte-exact.~~ **DONE (2026-09-05)**
45. ~~**Rev.134: `moveImage`/CopyTexture plumbing** — `RenderBackend::copyTexture`, `RenderCmd.copy`, executor bridge, no-op stub overrides, `test_move_image_guards`; 20/20 CTest.~~ **DONE (2026-09-05)**
46. ~~**Rev.135 (native-port, P1): first visible milestone** — `ICO_ENABLE_OPENGL` default ON; `ico_native` opens a real GLX 640x448 window and renders 3 animated quads at 60 fps through the semantic GIF pipeline (GifPacketBridge→executor→OpenGLBackend); screenshot evidence in `research/native/ico-native-first-window-2026-09-05.png`. Fixed two rendering-blocking GL bugs: `glCreateShader` vs `glCreateProgram`, and GL context current to the GLX window (not the X window) for `glXSwapBuffers`. 21/21 CTest.~~ **DONE (2026-09-05)**
47. ~~**Rev.142: PS2O (`.p2o`) geometry decode** — vertex positions = 4 floats LE `(x,y,z,1.0)` from +0x20 (16 B/vertex, Y-up, floor at Y≈0); face data = 16 B records `[c,t,0,a,1,b,0,0]` in strips framed by `0xFFFF` at the `c` position; **global parsing rule now CLOSED** (detect `0xFFFF` at `c`, then consume 16 B records while valid — NOT fixed stride): 27 strips / 76 non-degenerate triangles recovered from full p2 scan into `/tmp/st00a_p2_faces.obj`; two strip types (0x00 = long triangle strips a==b, 0x01 = short quads/fans a≠b); `OBJH`/`null` tags + [offset,count] stream table in minimal object; `+0x1c` field and multi-submesh delimitation (p1 count=3) still open. TYPE 0x01 SEMANTICS is the next step for the static-mesh milestone (asset → geometry → render).~~ **DONE (2026-09-06)**
48. **Rev.143 (native-port, P1 — texturized room render):** `ico_native` auto-loads `.p2o` mesh (`170_st00a_p1.p2o`: 13,877 verts, 15,161 triangles, 3 submeshes, 13,070 UVs, 15,161 material indices) and `.tm2` texture (`st0_a.tm2` 256×256) from `native/assets/`. TM2 texture pipeline integrated into `runMeshDemo`: UV mapping via `triVertUVs`, texture binding through `OpenGLBackend`. Material index per strip (`f` = u16[7]) confirmed. Material↔TM2 by name (7 materials). Extraction of `.p2o`/`.tm2` from PAL `.DF` containers via `dfdatas_unpack.py`. `run_native_demo.sh` for one-command demo. 21/22 CTest (only `opengl_backend_test` segfaults due to headless context). 95% pass rate.~~ **DONE (2026-09-06)**
49. ~~**Rev.154: verified scene tables drive real GObj creation** — `tools/extract_scene_tables.py` extracts the 68 entity descriptors (0x2A31B8), 97 verified world-dispatch scene ranges (tiling [41,3453) from 0x5F2FB8), and the scene-0x0F entry slice (29 entries) into `native/src/game/GeneratedSceneTables.h`; `KanbanSceneLoader::applyVerifiedSceneTables()` feeds them into the original requestScene/execute flow with per-entry listId/gobjType/flag_44 and descriptor gate_44; **`initSceneGObj` now creates 25 host GObjs for scene 0x0F** (29 payload − 4 gate-0 descriptors DYNAMICMOTIONDAT/STAGESETTING), entry-table correction 512→3600 (valid run idx 0..3590); `verified_scene_test` + demo run confirmed GObjs>0 with BoyController stable; 26/27 CTest.~~ **DONE (2026-09-09)**
50. ~~**Rev.151: p1 face/UV decode correction (byte-validated)** — the record is NOT `[c,t,0,a,1,b,0,0]`; the strip is a 16 B header `[N, 0xFFFF x7]` where N = number of record rows that follow (literal count — 4636 headers carry 3 records, 2352 carry 4, ..., count x N reproduces the record total exactly). Each record is `[1, 0, a, s, m, b, u, f]`: pos=`u16[2]` (max 7792), **UV index=`u16[4]` (max 13069 = NUV-1, EXACT full coverage 13070/13070)**, material=`u16[7]` (EXACTLY 7 values 0-6, matching the 7 names), stream id=`u16[3]` (const per strip, NOT material). Tris = cascade (r0,r1,r2)(r1,r2,r3)... = N-2 per strip. Face region: 0x6A6A0 → ends before OBJH (0x101620). Result: 7877 strips / 15007 tris / mean UV edge spread 0.1645; runs in `ico_native` at 15019 real tris with correct brick texture. **The old "UV = k + offset[f]" heuristic is WRONG — replaced by the direct u16[4] UV index (Rev.151) in `Ps2oMesh.cpp`; p2 wall family (type 0x00/0x01 quads/fans) still needs its own discriminator.**~~ **DONE (2026-09-08)**
50. ~~**Rev.155 (native-port, P1 — per-GObj visual composition):** — **Passo 1 do plano de sequência do usuário entregue.** Novo `GObjAttachmentStore` host-side (`native/src/engine/GObjAttachment.{h,cpp}`) liga cada GObj à sua composição visual (mesh path, material names, Matrix4x4 transform) sem crescer o ABI byte-exact `IcoGObj` 0x174; um GObj pode ser dono de vários meshes (28 assets / 25 GObjs, round-robin HOST pareio `m_sceneGObjs[i % size]` — **heurística explícita, não reconstrução byte-verified do vínculo GObj↔modelo**). `KanbanSceneLoader::attachBoundAssetsToGObjs(0x0F)` → 28 anexos; accessibility `attachmentStore()`/`sceneGObjHandle()`. **O loop de render em `main.cpp` agora itera as listas primárias do runtime isysGObj (`head(listId)`→`next`) e desenha o que cada GObj ativo comanda** (fallback para `stripBatches` global apenas sem store). O boy virou **BoxMarker attachment GObj-ownered**: `BoyController` atualiza `m->transform` todo frame e o renderer lê o marker (`drawBoxMarker`), sem hardcode da posição. `gobj_attachment_test` novo (28 anexos, dedup por (handle,meshPath), detach/find coherence, re-init limpa). CTest 27/28 (só `opengl_backend` headless). Demo `--frames 1 --shot` verificado: `requestScene/execute` ok, 25 host GObjs, 28 anexos, PPM com sala texturizada + 98 px do box do boy. Nota: `research/native/rev155-per-gobj-visual-composition.md`. ROUND-ROBIN GObj↔modelo é provisional — um capture PCSX2 real o substitui (Passo 2).~~ **DONE (2026-09-09)**
50. ~~**Rev.156: `GifPacket.*` bridge fidelity + p2 family closed** — `GifPacketBridge` captures `prim`/path (`currentPrim()`/`currentPath()`), honors `setDrawEnvironment` viewport origin x/y, exposes documented host half-offset hook (`setHalfOffset`) applied to all `*Offset` emit variants + `draw2DUVStripG` (default 0 = identity); byte-exact `0x8000` verified in BOTH base and Offset `.s` (`gif_MakeSprite`, `gif_Draw2DStripG`, `gif_Draw2DUVStripG`, `gif_SpriteSensitive`, `gif_MakePoint2DOffset`, `gif_EndPacket`) — NOT a verified Offset discriminator, kept explicit/host-only. `tools/ps2o_family_analysis.py` + `test_two_strip_family_unified` fixture prove p2 flags u16[0]=0/1 decode with the SAME canonical Rev.151 rule (p2: 3,562 headers / 15,005 records, max a 4,714 < nv 6,213, UV 4,844/4,844, f∈0..5, flag0=590/flag1=2,972, bnd_ratio 0.126 = Rev.144 M-A) — **Rev.151 open item CLOSED, no decoder change**. 27/27 CTest (only headless `opengl_backend` excluded). Nota: `research/native/rev156-gifpacket-bridge-fidelity-and-p2-family-closed.md`.~~ **DONE (2026-09-09)**
51. ~~**Rev.158 (native-port, P2 — runtime GObj↔processCallback binding):** captura PCSX2 "gaiola da Yorda" (13 world_state_load, 94K eventos) correlaciona `isys_gobj_proc_add` (0x13F3F0) por sala: **29 callbacks nomeados resolvidos via `TARGET_FUNCTIONS`** (boy_hB 0x1C1DD8, girl_hB 0x1D17F8, boy_init, enemy1_hB/hD/init, generator_hB, bird_hB, torch_hB, flag_hB, chain_hB, seffect_hB, ap1_hB, woodbox0_hB, ItemGeo, type6/15/22/24/36/39/55/60_hB, subGirlBrain_Idle/Hesitate/Busy/PulledUp). **`a3` distingue handler principal (1) de layer/init (0)** — corrreção da leitura "add/remove". Piscina GObj é REUSADA entre salas (mesma casa muda de dono: ex 0x831C58 type6_hB→torch_hB→seffect_hB); vínculo por sala, não por init único. `type60_hB` (0x23D518) é o handler mais frequente (5/sala; 9 no seg11). Nota: `research/native/rev158-runtime-gobj-handler-binding.md`. Host ainda usa round-robin — próximo passo é ligar estes handlers aos meshes por sala.~~ **DONE (2026-09-09)**
52. ~~**Native engine: re-link GObj→mesh por sala (Passo 1-3, Rev.159)**~~ — `applyVerifiedRoomRolePlans`/`hasRoomRolePlan`/`gobjHandlerRole`; `attachBoundAssetsToGObjs` role-tags 23 host GObjs from the verified 26-slot/8-handler 0x2B repertoire (runtime, not descriptors); `execute()` auto-relinks every scene transition; `initSceneGObj` releases prior-room GObjs (pool reuse). `tools/extract_room_role_tables.py` + `GeneratedRoomRoleTables.h` (11 scenes, 240 slots). Verified payload extended 29→54 (0x0F+0x2B). Nota: `research/native/rev159-verified-room-role-binding.md`. **DONE (2026-09-09)**

---

### Native runtime rendering status (2026-09-06)

| Component | Status | Detail |
|-----------|--------|--------|
| `ico_native` | ✅ Working | GLX 640×448, GL 3.3 core, 60fps |
| PS2O mesh decode | ✅ Working | 13,877 verts, 15,161 tris, 3 submeshes |
| TM2 texture loading | ✅ Working | `Tm2Parser` → `Tm2Converter` → `OpenGLBackend` |
| UV mapping | ✅ Working | `triVertUVs` from material-specific offset |
| Material indices | ✅ Working | `f` = u16[7] per strip, 7 materials |
| Auto-load | ✅ Working | `--p2o` defaults to `native/assets/170_st00a_p1.p2o` |
| Infinite loop | ✅ Working | `--frames 0` runs continuously |
| CTest | ✅ 27/28 | Only `opengl_backend_test` segfaults (headless), rest pass |
| Extraction pipeline | ✅ Working | `dfdatas_unpack.py` on PAL ISO → `.p2o` + `.tm2` |
| Demo runner | ✅ Working | `run_native_demo.sh` |
| Per-GObj composition | ✅ Working (Rev.155) | render loop iterates active isysGObj lists, draws each GObj's attachments; boy via BoxMarker attachment |

> **Rev.151 NOTE.** The rendering-status table above predates Rev.151.
> Current p1 figures (after the face/UV decode correction): **13,877 verts,
> 13,070 UVs, 7,877 strips, 15,007 tris, 7 materials**. UV mapping uses the
> record's own `u16[4]` index (NOT a material-offset heuristic). See the PS2O
> face-record format block below.

> **Rev.154 NOTE.** `KanbanSceneLoader` now consumes byte-verified USA scene
> tables (`tools/extract_scene_tables.py` → `GeneratedSceneTables.h`): 68
> descriptors, 97 scene ranges, and the scene-0x0F slice of 29 entries →
> **25 host GObjs** created at load (4 gate-0 descriptors). Entry table is
> 3600 rows (valid `descIdx<68` run idx 0..3590), not 512. CTest baseline
> raised to **26/27** by `verified_scene_test`; Rev.155 then added
> `gobj_attachment_test` → **27/28**.

> **Rev.155 NOTE (Passo 1 — per-GObj composition).** The render loop is no
> longer driven by the global `stripBatches`/`SceneAssetStore`. It walks the
> ACTIVE primary isysGObj lists (`head(listId)`→`next`) and, per GObj, draws
> the strip batches of the meshes that GObj owns through the host
> `GObjAttachmentStore` (`KanbanSceneLoader::attachBoundAssetsToGObjs(0x0F)`
> → 28 attachments over 25 host GObjs). Mesh meshes stay in `native/assets/`;
> the boy is a GObj-owned **BoxMarker** attachment whose transform the
> `BoyController` updates each frame (no hardcoded player box). The
> GObj↔mesh pairing is an explicit HOST round-robin heuristic, NOT a
> byte-verified reconstruction — a future PCSX2 capture that binds each GObj
> to its model (Passo 2) will replace it.

---

## PS2O (.p2o) face-record format — CANONICAL (Rev.151, byte-validated)

Use this as ground truth when decoding p1-room face data. It supersedes the
Rev.142/143 `[c,t,0,a,1,b,0,0]` + `0xFFFF`-frame description, which was WRONG
for positions and UVs.

**Layout (all little-endian):**

```
header:  PS2O | u32 payload-16 | u32 submesh_count | 0x18 "SUM\0" | u32 @0x1c
verts:   records at +0x20, 16 B each = 4 floats (x,y,z,1.0); array ends at
         the first record whose w != 1.0.  (p1: 13,877 verts, nv=13877)
uvs:     immediately after positions, 16 B each = 4 floats (u,v,0,0);
         ends when trailing floats != 0.          (p1: 13,070 entries, NUV)
faces:   immediately after the UV region (p1: 0x6A6A0). 16 B rows:
    STRIP HEADER row (8 x u16):  [ N, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
                                   0xFFFF, 0xFFFF, 0xFFFF ]
      N = literal number of record rows that follow this header.
      Validate by counting: for EVERY header value in p1, count x N equals
      the observed record total exactly (4636x3, 2352x4, 142x5, 330x6,
      73x7, 133x8, ... up to 32). N in [2,64] is a safe acceptance rule;
      rows that are not `[N, ffff x7]` mark the END of the face region
      (p1: ends at 0x101580, right before OBJH tags at 0x101620).
    RECORD row (8 x u16):        [ 1, 0, a, s, m, b, u6, f ]
      a  = u16[2] vertex-position index            (p1 max 7792 < nv)
      s  = u16[3] stream id, CONST per strip       (p1 max 6083, 5349
           distinct; does NOT index materials)       ← DO NOT use as pos!
      m  = u16[4] UV index into the UV array       (p1 max 13069 = NUV-1,
           EXACT full coverage: all 13070 UVs referenced) ← UV source
      b  = u16[5] mirror of a (== a on most rows)
      u6 = u16[6] CONST per strip, 10 distinct values 0..9; coincides with
           legacy tex=1/tex=2 counts at 2154/168 but CANNOT be material
           (10 != 7 names)                            ← DO NOT use as mat!
      f  = u16[7] material index, EXACTLY 7 const  (p1 values 0..6 == the 7
           embedded material names)                  ← the material source
```

**Triangle construction (cascade strip):** one strip = header + N records
emits N-2 triangles using consecutive records:
`(r0,r1,r2)(r1,r2,r3)...(rN-3,rN-2,rN-1)`. Vertex k of each triple = `a` of
record r(k); the UV of that vertex = `m` of the same record row. Aligned
16-byte walking from the UV end → first `[N, ffff x7]` header finds the face
region; the walker stops at the first non-header row. Degenerate inner
triangles (equal consecutive `a`) are dropped from the flat list.

**Validated numbers (room `170_st00a_p1.p2o`):** 7,877 strips / 15,007 tris /
mean UV edge spread **0.1645** (the winning model among cascade 0.171, old
tolerant-walker 0.171, strip-window s-equal 0.189, fan-pivot 0.228). The old
loader's 15,161 tris came from a misaligned spine (u16[3]=s read as `a`).

**Material names:** located by scanning the file for path-style printable
runs (`...\texture\<name>\0`) or `?name`/`>name` on NUL boundaries,
deduped in file order == material index order. p1 yields exactly 7:
`metal, wall_dec1, st0_a, broken, wall_fuchi2, isikabe, pole` (index 0..6).
The TM2 texture for material f is `<name>.tm2`.

**What is still open (p2 wall family):** the p2 wall mesh uses a DIFFERENT
rule (type 0x00 long tri-strips with a==b vs type 0x01 short quads/fans with
a!=b per Rev.142) so the p1 cascade-a rule over-decodes p2. A per-file
family discriminator is still unresolved.

**Native implementation:** `native/src/engine/Ps2oMesh.cpp` —
`loadPs2oMesh()` parses the canonical format above; `Ps2oStrip` keeps the
spine + per-vertex UVs; `Ps2oMesh::triVertUVs` is built directly from each
record's `m`. Remove neither the row-aligned header scan nor the stop-on-
non-header rule when adding p2 support.

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

The confirmed architecture is now understood:

1. **`_Clip` (0x166E10)**: clipping/collision function within `DispCollisionPC`. Its 17-slot dispatch table (0x282690) is compile-time `.data` with only 4 active `_clipW*` callbacks.
2. **`isysGObj*` (0x13DDA0-0x141D18)**: the true game object processing system (30 functions). Callback registration, object traversal, allocation, and removal are all managed here.
3. **`execBombGeo` (0x1D37C8)**: geometry function with 5-state cloth physics jump table at 0x618FB0.
4. **GirlBrain** (0x16xxxx): 10 verified Girl AI functions for navigation, pursuit, and hide behavior. **Not** in 0x19xxxx range (that is eBrain/Generator).
5. **Main loop**: `vblankHandler` → `ACTGame` → `backStageProcessMain` / `stage_ApplyData` / `kanbanExec`.
6. **Scene loader** in `kanban.c` (GP=0x27A7A8): 8 confirmed functions including `initSceneGObj`, `HotInitSceneObjects`, `la_load_processing`.

The runtime slot distribution from PCSX2 (15 active slots, 42.2M events) likely belongs to the `isysGObj*` processing system, not the `_Clip` collision pipeline.

A secondary but high-impact front is **External Symbol Reconciliation (PAL→USA)**:

- `tools/symbol_reconcile/reconcile.py` — pipeline que converte `symbol_addrs.txt` PAL (5681 símbolos) em um mapa validado contra o ELF USA, usando fingerprint binário (não endereço)
- Outputs em `docs/symbols/`: `raw_symbols_pal.csv`, `usa_fingerprints.json`, `pal_usa_symbol_candidates.csv`
- 3991 funções USA com fingerprints (raw_sha1, norm_sha1, op_hash) extraídas
- Matching PAL→USA byte-a-byto **aguarda o ELF PAL** (obter ISO PAL do usuário)
- Quando completo: nomes de função, source_file, e agrupamento por módulo original disponíveis para toda a base de código

**Caveat: `pal_usa_symbol_map.csv` rows with `status=candidate` are fuzzy matches, NOT ground truth.** Rev.145 documented a concrete false-positive case: of the 38 `GifPacket.o` symbols, only `gif_Sprite` (0x00110948) is byte-verified same-VA; the 36 "candidate" rows pointed to unrelated USA functions (e.g. `gif_EndPacket`→0x13F9D0 = `_iosOmMain`) and were flipped to `rejected`. The GifPacket.o module IS present in USA at ~0x10FD40-0x112A30 (GIF buffer 0x4C7710), but only `gif_Sprite`'s boundary is byte-verified. Treat any `candidate` with `sz_diff>0` or conflicting verified targets as suspicious; verify boundaries with `assemble_and_verify` before reconstructing `.s`. Nota: research/elf/rev145-gifpacket-reconciliation-false-positives.md.

---

## Persistent PCSX2 gameplay sessions (validated 2026-08-25)

When the user says **"vamos para mais uma sessão"**, use the persistent local
runtime below. Do not clone or rebuild upstream PCSX2 from scratch.

Persistent paths (survive a normal reboot):

```txt
.local/pcsx2-ico-logpoints-fork/                 odevpedro/pcsx2, branch ico-logpoints
.local/pcsx2-ico-logpoints-fork/build-runtime/   validated instrumented Release build
.local/pcsx2-fork-deps/                          local runtime/build dependencies
.local/pcsx2-runtime/                            BIOS/config/memory-card state
.local/iso/Ico (USA).bin                         validated game image path
.local/pcsx2-logs/                               per-session JSONL captures
```

The `.cue` path failed image-type detection in this fork; use the `.bin` path.

Required workflow:

1. Run `./tools/run-ico-pcsx2-logpoints.sh --check`.
2. Start `./tools/run-ico-pcsx2-logpoints.sh`. It creates a unique session ID,
   a new JSONL, `.local/pcsx2-logs/current-session.env`, and the
   `current-session.jsonl` symlink.
3. Start `./tools/monitor-ico-pcsx2-logpoints.sh` to follow the current capture.
4. Before telling the user to play, run:

   ```txt
   python3 tools/runtime-probe-analyzer/verify_runtime_probe_log.py \
     .local/pcsx2-logs/current-session.jsonl
   ```

5. Only release gameplay after the validator reports `status=valid`. The
   required boot sentinels are `elf_entry_sentinel`, `isys_gobj_init`, and
   `ios_om_main`.

The 2026-08-25 validation capture contained 43,346 well-formed events from one
session, including `isys_gobj_proc_add`, `isys_gobj_add`, `init_scene_gobj`,
`world_state_load`, and the required sentinels. The instrumented fork source
currently has a deliberate local modification in
`pcsx2/x86/ix86-32/iR5900.cpp`; do not discard or reset it.

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

## Commit message discipline

Every commit that closes a revision must carry a **detailed body**, not only a
subject. The body is part of the auditable evidence chain: it must survive
independently of the research notes and let a future agent (or reviewer)
reconstruct what changed and why.

Minimum structure:

```txt
core: Rev.XXX — short headline summary [TRILHA: PORT]

- Bullet per concrete change (fix, addition, removal), each with the
  function/file/tool name and, when applicable, byte sizes or VAs
- State the ROOT CAUSE of any bug fixed (e.g. "phantom inflation = regex
  bug, not ee-gcc"), especially when a previous revision's count changed
- Name each affected artifact explicitly (e.g. "9 pipeline files re-
  aligned", "12 .s truncated & byte-exact verified")
- Include the verification evidence (e.g. "asm_source_score --all:
  612/612 direct PASS", "21/21 CTest")
```

Do not commit with an empty or one-line-only message. If a commit was pushed
with a thin message, do NOT silently amend history: either write a follow-up
commit documenting the details, or ask the user before force-pushing an
amend of a pushed commit (rewriting public history requires explicit
consent).

## Verify-before-reconstruct (avoid redoing resolved work)

Before writing ANY new reconstruction code (semantic C/C++, native bridge,
fixtures, tests) or proposing a new decompilation target, FIRST verify what
already exists:

1. **Search the repo for the target** before creating anything:
   - byte-exact `.s` files in `src/**/asm/` (e.g. `initSceneGObj.s`);
   - semantic C/C++ in `src/core/` and `native/src/`;
   - existing research notes (`research/`) covering the same address/function;
   - existing tests (`native/tests/`, `tests/`).
2. **Read the relevant notes and source before deciding** the work is new.
   A function may already be reconstructed (even if only as `.s` or a semantic
   wrapper). Reconstructing something already resolved wastes time and risks
   conflicting changes.
3. **Only create new code when there is a genuine gap** — a target with no
   existing `.s`, no semantic wrapper, and no research note documenting it.
   When a real gap exists, prefer extending an existing module over creating a
   parallel one.
4. **If you are unsure whether a gap exists, ask or state the uncertainty**
   before writing code, rather than assuming it is new work.

This rule applies to both the `main`/`master` (decompilation truth) and the
`native-port` branch. Treat "already resolved or already partially covered" as
the default assumption until proven otherwise.

## Blog persona prompt maintenance

The file below is a required companion context for narrative/blog-style writing about this project:

```txt
docs/prompt_persona_ico_reconstruction.md
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

The project has completed static analysis of all remaining systems (Rev.086). Next runtime targets (prepared as Rev.086 probes):

```txt
breakpoint at 0x00166E10  (_Clip                  — slot_index, a0 context)
breakpoint at 0x00167020  (dispatch_point         — v1 callback, a0-a2)
breakpoint at 0x00167230  (cold_path_A            — a0 context, gp slots)
breakpoint at 0x00167258  (cold_path_B            — a0 context, gp slots)
breakpoint at 0x001AF948  (world_state_load       — room transitions)
breakpoint at 0x00166600  (gp_m49B4_read          — most-referenced GP var)
```

Additional isysGObj* validation targets (after the corrected architecture):

```txt
breakpoint at 0x0013F9D0  (_iosOmMain            — validate 17-slot distribution matches runtime capture)
breakpoint at 0x0013FC00  (iosOmCreateDL          — observe 32-slot dispatch, mask bits)
breakpoint at 0x0013FD10  (iosOmExeEachGObj       — slot iteration, event counts per slot)
breakpoint at 0x0013F3F0  (isysGObjProcAdd_       — registration activity during scene load)
breakpoint at 0x001B76F8  (initSceneGObj          — observe GObj creation for scene objects)
```

Useful questions:

- Which room transitions occur and what world_state values appear?
- What is the most-referenced GP variable (gp-0x49B4, 434 refs)?
- Does the VBlank counter at 0x274EC0 show frame beats consistent with IOP timing?
- What entity contexts accompany room transitions?
- Does slot distribution correlate with specific world_state values?
- Does `_iosOmMain` slot distribution (8 mask + 9 type) match the earlier PCSX2 15-slot capture?
- What mask bits are active in `gp-0x6724` during gameplay vs cutscenes?
- How many GObjs are created by `initSceneGObj` per scene load?

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
