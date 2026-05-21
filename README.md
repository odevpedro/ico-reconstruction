# ICO Reconstruction

> Community research project for documenting and progressively reconstructing ICO for PlayStation 2 as a recompilable technical base.

[![Last Commit](https://img.shields.io/github/last-commit/odevpedro/ico-reconstruction?style=flat-square)](https://github.com/odevpedro/ico-reconstruction/commits/main)
[![Project Status](https://img.shields.io/badge/status-planning%20%26%20research-blue?style=flat-square)](#current-status)

![ICO wallpaper](./assets/ico-wallpaper.webp)

---

ICO Reconstruction is an early-stage community research project focused on understanding and progressively reconstructing **ICO** for PlayStation 2 as a documented, recompilable development base.

The goal is not emulation, piracy, or redistribution of game content. The goal is to study the original PS2 release, document its architecture, build tooling around its data formats and executable code, and eventually make carefully validated technical changes possible.

## Project Goal

This project explores whether ICO can be transformed from a closed PS2 binary into a maintainable reconstruction pipeline:

- ISO and ELF analysis
- asset extraction and documentation
- disassembly and decompilation research
- runtime and subsystem mapping
- tooling for validation and experimentation
- progressive reconstruction of engine, gameplay, camera, actors, animation, collision, events, and scene data

OpenGOAL is an important methodological inspiration, but ICO is not assumed to share OpenGOAL's technical advantages. ICO likely requires a more traditional PS2 reverse engineering workflow over MIPS code, proprietary asset formats, and Team Ico/Sony Japan Studio engine structures.

## Stack & Architecture

This repository is currently documentation-first. The technical stack below describes the intended research workflow rather than a finished application.

| Layer | Current / Intended Tooling |
|-------|-----------------------------|
| Documentation | Markdown |
| Project tracking | `docs/backlog.md`, `docs/architecture-log.md`, architectural decision notes |
| Disc image inspection | `tools/iso-index` for metadata-only ISO9660/BIN/CUE indexing |
| ELF analysis | `tools/elf-index` for metadata-only ELF32 header/section/program-header indexing |
| Overlay metadata | `tools/dvp-index` for metadata-only `.DVP.*` overlay correlation |
| Executable references | `tools/exe-ref-index` for exact string/constant reference scans |
| MIPS immediate patterns | `tools/mips-immediate-scanner` for split-immediate (lui/addiu) pattern scanning |
| Symbol table analysis | `tools/elf-symbol-scan` for detecting symbol tables and dynamic linking |
| Function prologue detection | `tools/mips-prologue-scan` for MIPS function boundary detection |
| Function reference correlation | `tools/function-ref-correlator` for linking prologues with code references |
| Call graph analysis | `tools/mips-call-graph` for identifying function callers |
| ELF extraction | `tools/elf-extractor` for extracting ELF for disassembler import |
| Assembly verification | `tools/asm_source_score.py` — auto-generates `.s` from target ELF disassembly, assembles with ee-gcc, verifies byte-exact match |
| Disassembly | Environment setup with Ghidra |
| Archive/data exploration | `tools/data-df-index` for metadata-only `DATA.DF` structural triage |
| Validation | `tools/verify-local-copy`, reproducible notes, metadata reports, emulator/debugger evidence where applicable |
| CI/CD | Not configured yet |
| Tests | No automated suite yet; synthetic fixture used for local tool validation |

### Decompiled entity handlers (Rev.088)

NEAR-STRUCTURAL C decompilation of 7 entity handler sets:

| Entity | File | Functions | Status |
|--------|------|-----------|--------|
| BOY (1) | `src/entity/boy.c` | init_fn, hC, hB, hA, 5 helpers (632 lines) | NEAR-STRUCTURAL |
| ENEMY1 (4) | `src/entity/enemy1.c` | init_fn, hC, hB, hA, destructor + helpers (572 lines) | NEAR-STRUCTURAL |
| BARREL (5) / ROPE (8) | `src/entity/barrel.c` | init_fn, hA, hB (cb_routine2), hC (2 variants), 7 helpers (700+ lines) | NEAR-STRUCTURAL |
| WOODBOX0 (17) | `src/entity/woodbox0.c` | hC, hB, hA, cb_collision, work struct (220+ lines) | NEAR-STRUCTURAL |
| HB skeletons | `src/entity/hb_skeletons.c` | Common dispatch patterns | REFERENCE |
| Near matches | `src/entity/near_matches.c` | Near-identical function pairs | REFERENCE |

**Historically significant:** The function at 0x1D3A30 (cb_routine2) was
erroneously called the "ROPE callback" during Rev.001-036. Corrected in
Rev.039 via ICO-decomp cross-reference. It is BARREL's physics constraint
solver (clothAnimation.c / src/item.c domain).

Architectural approach:

```text
user-owned ICO copy
    -> extraction / inspection tools
        -> documented formats and executable regions
            -> subsystem maps
                -> small verified reconstruction targets
                    -> tooling and runtime experiments
```

The project does not currently define a database, backend API, ORM, authentication layer, or web service architecture.

## Current Status

The project has moved into **decompilation and struct modeling**. A verified structural model of the cloth physics system exists in `src/`, with 3 EXACT C matches and 5 NEAR-STRUCTURAL validated models (Rev.048). The ROPE callback is confirmed to live in a static physics object type table at `0x001A48A0` with 31 types (Rev.049), not in the dynamic callback registry. A third PCSX2 runtime session (Rev.051, ~90 min, 1419 events) with a direct probe at `0x001D3A30` confirmed **zero hits during normal gameplay**, refuting the per-frame update model for this function.

**Rev.066-067 live dispatch consolidation:** The live scene init dispatcher at `0x00166E10` has been fully mapped: a 17-entry slot table at `0x00282690` with 14 unique parametric callbacks (two templates: Group 1 for position/rotation via `0x166258`, Group 2 for orientation checks via `0x1667E0`), a runtime pointer list at `0x006AAC80` (corrected from `0x006AAC00`), cold paths as leaf fragments at `0x00167230`/`0x00167258`, and an alternate implementation at `0x00169F80`/`0x0016A058` with extra transform/matrix init. See [`research/elf/ghidra-rev067-consolidated-live-dispatch-model.md`](./research/elf/ghidra-rev067-consolidated-live-dispatch-model.md).

**Rev.069 static mop-up:** The VU0 ring-buffer packet builder at `0x1D43F8` was fully disassembled (5-entry VIF packet, 80 bytes, terminator via `t0=-1`). The VU0 kick stub `0x117C40` was identified as inline asm (4× LUI VIF codes + `J 0x3800C`). The halfword table writer at `0x6AB080` was located at `0x00166D1C`/`0x00166D78` (same function range as the dispatcher — writes `(a2 << 5) + t0`). No static caller of `0x00168650` with `a0 != 0` exists — the alternate implementation selection point remains a runtime-only question. See [`research/elf/ghidra-rev069-vu0-ringbuffer-packet-builder-halfword-table-population.md`](./research/elf/ghidra-rev069-vu0-ringbuffer-packet-builder-halfword-table-population.md).

**Rev.070 callers of 0x166028:** All 3 direct callers of the runtime pointer list builder were fully disassembled. `0x101C80` is the main game loop (128B stack) — calls `0x166028` once during scene init. `0x1AF4A0` is a scene/subsystem init (176B stack) — calls `0x166028` after an entity callback from a previously undocumented 404-byte stride entity table at `0x005F2F98`. `0x1B76F8` is the entry iteration/object creation function (304B stack, Rev.050) — calls `0x166028` at the end of every successful entry. The rodata table at `0x613E00` with `0x00168650` was identified as a debug build artifact (strings: `ClothInfo`, `CollisionOldProc`, `Skelton`, etc.) with no code references. See [`research/elf/ghidra-rev070-callers-of-166028-and-rodata-init-table.md`](./research/elf/ghidra-rev070-callers-of-166028-and-rodata-init-table.md).

**Rev.071 five-way consolidation:** (1) The 404-byte room entity table at `0x005F2F98` fully scanned — 32 rooms (NULL, logo, title, sacrifice, jail, warehouse, ico_brigde, proto, troko, chandelier, entrance, gate, gate2, grave, shadows, windmill, plaza, stone, symmetry_L, crest_L1-3, taki, sluice, underground, gondola, watertower, symmetry_R, crest_R1-3, cliff) with callback index 0x4B (75) at offset +340 for all non-NULL rooms. Code reads via base 0x005F2FB8 with pre-multiplied world_state. (2) Halfword table at `0x006AB080` confirmed as 32×32 grid rasterization encoding `(row<<5)|col`, populated by exactly 2 write points inside the dispatcher function at `0x00166D1C`/`0x00166D78`. (3) Slot table corrected: stride = 16 bytes (not 8), 17 entries × 4 fields, 14 unique callbacks across two templates (Group 1: position/rotation proximity at 0x166258, Group 2: orientation/origin matching at 0x1667E0). (4) Callback skeleton fully mapped: iterate halfword table → resolve 80-byte entity structures → run template matching. (5) Main loop 0x101C80 dispatch chain documented: 0x166028 → 0x1AF4A0 (reads 404-byte table) → dispatch, with VSync idle at 0x101F60. See [`research/elf/ghidra-rev071-404-table-room-names-callbacks-and-dispatch-system-consolidation.md`](./research/elf/ghidra-rev071-404-table-room-names-callbacks-and-dispatch-system-consolidation.md).

**Rev.072 — Room init callback system corrected:** The 404-table callback field was found to be at `row_base + 0x154` (= absolute `0x005F2F98 + 0x174`), NOT at `0x005F2F98 + 0x154`. The earlier `0x4B` value was a different field (debug/type ID). The real callback field contains **19 pre-loaded function pointers** for gameplay rooms (jail, warehouse, proto, troko, chandelier, entrance, shadows, windmill, plaza, stone, crest_L1-3, taki, sluice, gondola, watertower, crest_R1-3), with null pointers for non-gameplay rooms. The instruction at `0x1AF954` is `mult $v1,$a0` (not `and`) — confirmed as dead code (no `mflo` follows). A **descriptor table** of 68 entity types was fully mapped at `0x002A31B8` (stride 0x64, with names like BOY, GIRL, ENEMY1, BARREL, ROPE, WOODBOX0, etc.), and an **entry table** of 512 entity instances at `0x002A4C48` (stride 0x4C). The patching function at `0x00143290` processes inner structs (not the callback field). See [`research/elf/ghidra-rev072-callback-systems-descriptor-table-and-entry-table.md`](./research/elf/ghidra-rev072-callback-systems-descriptor-table-and-entry-table.md).

**Rev.073 — Main loop dispatch chain and corrected callback masks:** The full 12-step main loop pipeline at `0x00101C80` was disassembled: framebuf reset (0x1AA098) → live dispatch (0x166028) → entity registration (0x103370) → entity transform COP1 (0x104C80, 352B stack) → game systems input/timer/audio (0x1AF190) → VU0 kick conditional (0x129A78, tail-call J 0x117768 only when gp-28384==0 gameplay) → scene cleanup (0x129AA8) → callback dispatch linked-list A (0x13F9D0, two-level) → scene proc2 (0x129C90) → callback dispatch B (0x13FC00) → table clear (0x102680) → vsync wait (0x13D3F0). The callback mask model was corrected: all 14 callbacks use **bits 28-31** (upper nibble of field_48 for Group 1, field_60 for Group 2 slot 15), not bits 12-15 as previously documented. A secondary pointer table was located at `0x00633D30` (BSS) with two halves: Group 1 struct array (stride 0x50=80 bytes) and Group 2 struct array (stride 0x70=112 bytes). All callbacks follow a linked-list data flow: halfword table → pointer array → linked list of pre-multiplied byte offsets → struct pointer → mask test → template dispatch. The `mult $v0, 0x50`/`mult $v0, 0x70` instructions in all callbacks were confirmed as dead compiler scheduling artifacts (same pattern as `0x1AF954`). Context store pattern mapped: Group 1 writes to ctx+0x88, Group 2 writes to ctx+0x94 and clears ctx+0x88. See [`research/elf/ghidra-rev073-main-loop-dispatch-chain-and-callback-corrected-masks.md`](./research/elf/ghidra-rev073-main-loop-dispatch-chain-and-callback-corrected-masks.md).

**Rev.074 — Runtime session confirms main loop dispatch chain:** A PCSX2 instrumented session (~15 min, 9.1M events, 4.5GB log) validated the live dispatch model at runtime. **Slot 0 confirmed dead** (zero hits — reserved/fallback). **Slot 12 is most active** (38.7%, Group 2 orientation). Slots 8/9/13/16 never fire (w2=1 flag and triplet guard conditions not triggered). **Alternative impl paths 0x69F80/0x6A058: ZERO hits** — confirmed dead code in normal gameplay. **VU0 kick fires 59K times**, always in gameplay mode (world_state=0). **Match rate: 58%** across all callbacks. **GP = 0x006388F0 verified** at runtime. **615 unique entity contexts**, **20 live entities** per frame. **Callback registration** callers confirmed: scene loader (0x1B7AD4/0x1B7AB8) and factory functions (0x240E58/0x240F98). See [`research/elf/ghidra-rev074-runtime-session-main-loop-dispatch-confirmed.md`](./research/elf/ghidra-rev074-runtime-session-main-loop-dispatch-confirmed.md).

**Rev.075 — Init fn identification, callback dispatch, ASM handler analysis:** Three previously unknown init_fn identified: `cloth_sys` at 0x1C3760 (stride 0x50/0x1A0 parallel tables), `cloth_tramp` at 0x1F2370 (5-mode entries tail-calling 0x1D12A8), and `env_effect` at 0x17D128 (fog/BGA, resource loads). The callback dispatch system at 0x13F9D0 was fully disassembled: two-phase linked-list walk with 8-entry table at 0x281A70, typed handler IDs 0x13-0x1B, mask register at `gp+0x98DC`. A secondary dispatch at 0x13FC00 uses table 0x281AB0. Full ASM-HANDLER analysis completed: BOY (352B stack, 46 JALs), GIRL (heavy FPU animation blend), ENEMY1 (only entity using flags_0x48 for mask selection), WOODBOX0 (lookup table at 0x28A640), AP1 (data-driven from 0x623468). GP mask writer found: `mask_set()` at 0x13ED40 sets/clears bits 0 or 1; boot zero-init at 0x13DDB0; 6 callers in scene loader range. See [`research/elf/ghidra-rev075-init-fn-callback-dispatch-and-asm-handler-consolidation.md`](./research/elf/ghidra-rev075-init-fn-callback-dispatch-and-asm-handler-consolidation.md).

**Rev.076 — Post-runtime consolidation and final static analysis:** All findings consolidated: **Two independent entity systems** discovered (callback_register 52 objects vs dispatch 8 core entities; only 3 overlap). **28 init_fn classified** into 6 groups: Generic (60%), HUD/status lights (15%), UI/menu (9%), cloth/physics aux (5%), env effect subtype (3%), special one-shots (0.4%). **17-slot dispatch table** at 0x282690 fully mapped (3 callback tiers: leaf pos/rot, hybrid G1+G2, full pipeline with math+cloth). **Mask system uses only bit 0** — all 6 callers converge on bit 0 set/clear for callback gating during scene loading. **404-byte table** at 0x005F2F98 confirmed as room/stage config (not entity table). **Halfword table** at 0x6AB080 is a 32×32 spatial hash grid rebuilt per dispatch cycle. **VU0 "kick"** at 0x117C40 is a COP2 macro-mode vector clamp utility (no actual VU0 kick). **Alternate implementations** at 0x169F80/0x16A058 are VU0 DMA pipeline code (unreachable). **ICO-decomp cross-reference** confirms only mask_set matches exactly: `ShockRequestBox_RequestCancel` in fumi/ios/shockdriver.c. See [`research/elf/ghidra-rev076-post-runtime-consolidation.md`](./research/elf/ghidra-rev076-post-runtime-consolidation.md).

**Rev.077 — Final static analysis:** Descriptor table at 0x2A31B8 fully parsed — **68 entity types** with names (BOY, GIRL, ENEMY1, BARREL, ROPE, QUEEN, BIRD, etc.), init_fn, 3-4 callback routines, mass, radius, threshold, and vtable pointers. **ROPE callback registration gap finally resolved**: `0x1D3A30` is BARREL's physics constraint solver registered via descriptor table `+0x50`, not callback_register. **Entry table** at 0x2A4C48: 512 entity instance spawns with positions, classes, model IDs. **8-step scene loading state machine** at 0x1B08E4 in `kanban.c` (GP=0x27A7A8, separate compilation unit). **VU0 "kick" function** at 0x117768 is actually a **linked-list deferred processing queue** (no VU0 instructions at all). **Debug visualization table** at 0x613E00 contains 47 runtime debug entries with names like CollisionOldProc, ClothInfo, BrainBar, and only one non-null callback: `0x168650`. **Resource check** at 0x17B230 is a bitmap-based readiness check (array at 0x28A520). **Wait/yield** at 0x203AA0 uses EE kernel syscall 50 (cooperative yield) with VBlank counter at 0x274EC0. **GP data map** consolidated: 1032 unique GP offsets referenced by 9971 instructions across `.text`; the most-referenced is `-0x49B4` (434 refs). With Rev.077, **all possible static analysis without emulator access is complete** — any further progress requires runtime validation via PCSX2 breakpoints. See [`research/elf/ghidra-rev077-final-static-analysis.md`](./research/elf/ghidra-rev077-final-static-analysis.md).

**Rev.079 — Runtime session at windmill validates dispatch across areas:** A second independent PCSX2 runtime session (~15 min, 14M events, 4.8GB log) in the windmill section confirmed and expanded the Rev.074 findings. **Slot distribution inverted** (slot 1 at 45.7% vs 27.0% in the entrance section), confirming slot activation is area-dependent. **12 callbacks fully mapped** with cross-references. **mask_set (0x13ED40)** confirmed zero hits during gameplay across two sessions — only fires on loading transitions. **Slot 0 dead** in 2nd session (0/1.8M dispatches). **Halfword table writers** never triggered in windmill. **Alt selection** never called. GP=0x006388F0 reconfirmed. See [`research/elf/ghidra-rev079-runtime-validation-windmill-session.md`](./research/elf/ghidra-rev079-runtime-validation-windmill-session.md).

**Rev.084 — Extended runtime session (43.8M events, ~122 min):** A third PCSX2 runtime session covering entrance → windmill → cutscene → 3rd+ area. **Slot 5** fired for the first time (triplet guard entity). **Slot 11** observed for the first time (mask 0xC0000000). **Cutscene period** detected: 100% slot 12 only (Group 1 suspended). All rare probes ZERO across 3rd independent session: mask_set, halfword_store, slot0, alt_selection — confirmed dead code or loading-only. **1,913 unique entity contexts** observed. **Slot 0 root cause found** via static analysis: zero `addiu a1, zero, 0` + JALR sites in all .text — no code path ever dispatches to slot 0. Total project coverage: **66.9M events** across 3 independent sessions. See [`research/elf/ghidra-rev084-runtime-validation-extended-session.md`](./research/elf/ghidra-rev084-runtime-validation-extended-session.md).

**Rev.085 — Death validation:** User intentionally jumped off a cliff. **mask_set (0x13ED40) = 0 hits** even during death sequence. Death zone exhibits 100% slot 12 (Group 2 only, Group 1 suspended) — identical to cutscene behavior. Confirms mask_set is `ShockRequestBox_RequestCancel` (I/O system), not a gameplay death callback. See [`research/elf/ghidra-rev085-death-validation-and-next-session-plan.md`](./research/elf/ghidra-rev085-death-validation-and-next-session-plan.md).

**Rev.086 — Final static analysis (4 systems resolved):** (1) Descriptor `+0x60` = **behavior_fn** (shared dispatch function), not vtable — Group A at 0x202A60 (4 main character types), Group B at 0x23D660 (16 prop types). (2) **Env effect table** corrected: 395 entries × 0x30 (not 7), a type-to-type mapping matrix, NOT spatial trigger zones. (3) **cb_routine4 (+0x5C)**: no-op `jr $ra` stubs for GIRL/DEVIL_GI/ENEMY1/ENEMY_TEST only — never called at runtime. (4) **VBlank counter** 0x274EC0: no `.text` writer found — IOP-driven via SIF DMA. **GIRL = DEVIL_GI** confirmed (identical in all 5 fields). See [`research/elf/ghidra-rev086-static-analysis-vtables-enveffect-cbroutine4-vblank.md`](./research/elf/ghidra-rev086-static-analysis-vtables-enveffect-cbroutine4-vblank.md).

**Rev.092 — Path B milestone: all 26 LOW functions converted to byte-exact .s assembly (38/38 at 100%).** The C compiler register-allocation approach was fundamentally unable to reach 100% for 26 entity/low-level functions. The solution: `tools/asm_source_score.py` auto-generates `.s` assembly source from target ELF disassembly, assembles byte-exact with EE GCC 2.9-991111-01, and verifies against the ELF. All 26 functions now pass at 100% byte-level match. Combined with 12 existing EXACT-from-C functions, all **38 functions** are now byte-exact. EE assembler constraints discovered and handled: numeric-only register names, `.word` raw bytes for COP1 compares/bbit032, GAS numeric labels for branches, `.set noat` for `$at` usage, `.word` fallback for branches targeting external code. `src/{core,entity,cloth}/asm/` stores the generated `.s` files, including the byte-exact core lifecycle helpers promoted for `isysGObj*`. The old C-based scoring pipeline is archived. See [`tools/asm_source_score.py`](./tools/asm_source_score.py).

No reconstructed game code, assets, binaries, or ISO-derived copyrighted data are included in this repository.

Current repository contents are mostly operational documents:

- `docs/backlog.md` - current project state and pending work
- `docs/architecture-log.md` - historical record of implemented flows and decisions
- `docs/architectural-analysis-a-d.md` - first subsystem feasibility and validation map
- `docs/research-methodology.md` - evidence and documentation rules for research notes
- `docs/legal-boundaries.md` - repository content boundaries and contribution limits
- `docs/tooling-plan.md` - intended local tooling direction before implementation
- `tools/verify-local-copy/` - metadata-only local input verifier
- `tools/iso-index/` - metadata-only ISO9660/BIN/CUE indexer
- `tools/elf-index/` - metadata-only ELF32 indexer
- `tools/data-df-index/` - metadata-only `DATA.DF` triage tool
- `tools/dvp-index/` - metadata-only `.DVP.*` overlay indexer
- `tools/exe-ref-index/` - metadata-only exact executable reference scanner
- `tools/mips-immediate-scanner/` - metadata-only MIPS split-immediate pattern scanner
- `tools/elf-symbol-scan/` - metadata-only ELF symbol table scanner
- `tools/mips-prologue-scan/` - metadata-only MIPS function prologue scanner
- `tools/function-ref-correlator/` - metadata-only function reference correlator
- `tools/mips-call-graph/` - metadata-only MIPS call graph analyzer
- `tools/elf-extractor/` - ELF extractor for disassembler import

Current ELF research focus:

- a confirmed internal dispatcher at `0x001d37c8`, with jump table `0x00618fb0`, documented in [`research/elf/ghidra-rev023-dispatcher-table-resolution.md`](./research/elf/ghidra-rev023-dispatcher-table-resolution.md)
- a record/callback chain around the `.data` record labeled `ROPE`, documented across [`research/elf/ghidra-rev026-rope-record-table-context.md`](./research/elf/ghidra-rev026-rope-record-table-context.md) through [`research/elf/ghidra-rev035-entry-table-and-descriptor-correction.md`](./research/elf/ghidra-rev035-entry-table-and-descriptor-correction.md)
- a confirmed node callback dispatcher at `0x0013fb70`, plus a corrected descriptor-index model that keeps the exact indirect dispatcher for `ROPE +0x40 = 0x001d3a30` unresolved, documented in [`research/elf/ghidra-rev033-node-callback-dispatch-chain.md`](./research/elf/ghidra-rev033-node-callback-dispatch-chain.md) and [`research/elf/ghidra-rev035-entry-table-and-descriptor-correction.md`](./research/elf/ghidra-rev035-entry-table-and-descriptor-correction.md)
- a registration path survey confirming that `0x001b76f8` skips ROPE registration due to a zero-guard, and mapping all five static callers of `0x0013f7a8`, documented in [`research/elf/ghidra-rev036-registration-path-survey.md`](./research/elf/ghidra-rev036-registration-path-survey.md)
- a complete analysis of the three remaining callers of `0x0013f7a8`, closing three paths definitively and leaving two runtime-dependent candidates open, documented in [`research/elf/ghidra-rev037-remaining-callers-and-rope-gap.md`](./research/elf/ghidra-rev037-remaining-callers-and-rope-gap.md)
- External integration phase (Rev.038-045):
  - **CCC (Chaos Compiler Collection)** confirmed no `.mdebug`/STABS/debug symbols in the ELF
  - **decomp.me scratches** generated for 6 key functions (dispatcher, ROPE callback, registration chain)
  - **Compiler identified**: EE GCC 2.9-991111-01 (Sony fork), flags: `-march=r5900 -mips3 -mgp64 -mabi=eabi -msingle-float -G0 -O2`
  - **ICO-decomp cross-reference** (`RossyDoubleUnderscore/ICO-decomp`): dispatcher and ROPE callback reside in `clothAnimation.c` (cloth physics), not entity/AI state
  - **Parallel findings**: 5-state dispatcher likely handles cloth vertex/simulation states, not gameplay state transitions
- External tooling experiments (splat, Rabbitizer, spimdisasm):
  - **splat64[mips]** confirmed viable for ICO USA ELF: `splat/SCUS_971.13.cloth-full.yaml` promotes **22 cloth-domain functions** into individual asm files
  - **Rabbitizer/spimdisasm** independently validated dispatcher/callback instruction anchors
  - **SOTC tooling survey** confirmed useful process patterns (splat, Ninja, map/first-diff)
- Full cloth cluster documented in [`research/external/ico-splat-cloth-full-promotion.md`](./research/external/ico-splat-cloth-full-promotion.md)
- **Rev.048** — C scratch model synthesis with fixed taxonomy (EXACT/NEAR-STRUCTURAL/NEAR-LOCAL/MISMATCH/BLOCKED/ASM-HOLD), `ico_ptr32` type rule, and GCC 2.95.2 limitations documented. 3 exact matches, 5 near-structural validated models. [`research/elf/ghidra-rev048-c-scratch-model-and-ico_ptr32.md`](./research/elf/ghidra-rev048-c-scratch-model-and-ico_ptr32.md)
- **Rev.049** — Physics object type table discovered at `0x001A48A0` (stride 0x64, 31 types). ROPE entry at `0x001A4968` with handlers `0x001D3B28`, `0x001D3A30`, `0x001D27A8`. Confirms ROPE callback is NOT registered via `0x0013F7A8` — it lives statically in the type table. [`research/elf/ghidra-rev049-physics-object-type-table.md`](./research/elf/ghidra-rev049-physics-object-type-table.md)
- **Rev.050** — Cloth system anatomy consolidated. Three analyses: (1) `cloth_payload_init` (0x1D27A8) partially decompiled — variant-controlled init path, (2) `0x1B76F8` identified as the descriptor iteration function that calls `cloth_payload_init` via `descriptor+0x58`, (3) Entry table at `0x002A4C48` fully mapped (512 entries, stride 0x4C). **No entry has +0x46=0x14 (ROPE)** — all cloth objects use BARREL (index 0x13) which shares the same handlers. [`research/elf/ghidra-rev050-cloth-system-anatomy.md`](./research/elf/ghidra-rev050-cloth-system-anatomy.md)
- **src/ directory** — First verified C sources committed. Struct model with `cloth_payload`, `cloth_entity`, `cloth_context`, `physics_type_entry`. 3 exact-match accessor functions and 3 near-structural models.
- **Rev.051** — Runtime session 3 (~90 min, 1419 events): probe direto em `0x001D3A30` confirmou **zero disparos** durante gameplay normal, refutando o modelo de "update callback por frame". Investigação dos callers `0x00240E58`/`0x00240F98` revelou duas funções factory (`0x240D40`, `0x240EA0`) que registram múltiplos callbacks — ambas excluídas como caminho para o ROPE callback. Correção do segmento `.text`: `0x00100000..0x0026F5D4` (não `0x001Fxxxx`). [`research/runtime/pcsx2-recompiler-session3-2026-05-16.md`](./research/runtime/pcsx2-recompiler-session3-2026-05-16.md)
- **Rev.052** — Five-way consolidation: descriptor table full map (68 entries, stride 0x64, 13 with non-null init_fn), sister_callback_reg decompiled, cloth_event_clear decompiled, DVP overlays confirmed as VU0 microcode (no MIPS code), VU0 cloth physics (20KB microcode + 63 COP2 instructions). [`research/elf/ghidra-rev052-five-way-consolidation.md`](./research/elf/ghidra-rev052-five-way-consolidation.md)
- **Rev.053** — Handler decompilation wave 1: COP2 cloth functions (clothSubDistanceCheck at 0x1D3E80, clothSubPlaneClip at 0x1D45B0), WOODBOX0 lifecycle (286 insns constructor, 27 insns update), ENEMY1 full AI lifecycle (19+ sub-functions mapped), BOY warm/cold init paths. [`research/elf/ghidra-rev053-handler-decompilation-wave1.md`](./research/elf/ghidra-rev053-handler-decompilation-wave1.md)
- **Rev.054** — Handler decompilation wave 2: GIRL/Yorda (64B alloc, variant paths, anim blend), QUEEN boss (24B alloc, LOD scaling), BGA sprite overlay (init_fn=12 insns, no handlers), AP1 attack pattern 1 (640B alloc, 4 child slots, 7-state machine). Padrão hC/hB/hA confirmado em 7 entidades. [`research/elf/ghidra-rev054-handler-decompilation-wave2.md`](./research/elf/ghidra-rev054-handler-decompilation-wave2.md)
- **Rev.055** — COP2 cloth decompilation: clothSubDistanceCheck (5 COP2, proximity wakeup), clothSubPlaneClip (74 COP2, 4×4 transform + frustum clip), clothSubForceApply (0 COP2 — EE sin/cos only). BOY hC analyzed: 76B alloc, 5 models, 3 child entities. hB dispatcher investigation: gap unresolved — no centralized dispatch found. [`research/elf/ghidra-rev055-cop2-cloth-boy-hc-hb-dispatcher.md`](./research/elf/ghidra-rev055-cop2-cloth-boy-hc-hb-dispatcher.md)
- **Rev.056** — Handler decompilation wave 3 (final): BIRD, DEVIL_GI, ATTACKCH x2, BOSS_CTR — 12 handlers analyzed. Descriptor index correction (WOODBOX0=17, BGA=30, AP1=61). DEVIL_GI = GIRL alias. BIRD hB = 2 insns (delegate). ATTACKCH idx 63 hC = 176 insns (rotation matrix + dynamic child array). BOSS_CTR = 53-slot Queen boss arena manager. hB dispatcher diversity: 5 distinct patterns, no centralized dispatch. All 13 entries with non-null init_fn now mapped. [`research/elf/ghidra-rev056-handler-decompilation-wave3.md`](./research/elf/ghidra-rev056-handler-decompilation-wave3.md)
- **Rev.057** — C models for cloth dispatcher (0x1D37C8, 5-state FSM: guard→prepare→simulate→check→done) and clothSubForceApply (0x1D3F78, EE sin/cos force projection) added to `src/cloth/near_matches.c`. ENEMY1 hC model (80B constructor, 2x child arrays) added to `src/entity/near_matches.c`. WOODBOX0 hC marked ASM-HOLD (286 insns, 400B alloc, 384B data copy). Factory analysis summary integrated (0x240D40/0x240EA0 documented in Rev.052/session3).
- **Rev.087** — ENEMY1 entity handler fully decompiled to near-structural C. All 5 functions re-disassembled from raw ELF via capstone and written as C to `src/entity/enemy1.c`: hA (0x1CE690, 22 insns, conditional mask handler), hB (0x1CE3C0, 141 insns, per-frame AI+draw), hC (0x1CE220, 103 insns, 80B constructor), init_fn (0x164440, 278 insns, full entity init), fn_1CE5F8 (37 insns, destruction cleanup). Struct `enemy1_work` (0x50 bytes) defined with per-field documentation. Documentation updated (backlog, data-model, system-feature-flows). [`src/entity/enemy1.c`](./src/entity/enemy1.c)

These notes describe structural evidence only. They do not assign definitive gameplay names to the internal states or lifecycle slots.

## Repository Structure

```text
.
├── .github/
│   └── ISSUE_TEMPLATE/           # GitHub issue templates
├── .gitignore                    # Local binary/output safety rules
├── README.md                     # Public entry point for GitHub
├── assets/
│   └── ico-wallpaper.webp        # Public README image
├── docs/
│   ├── architectural-analysis-a-d.md # rev.007 feasibility and validation map
│   ├── architecture-log.md       # Historical record of implemented flows and decisions
│   ├── backlog.md                # Current work state and revision signatures
│   ├── data-model.md             # Reverse-engineered struct hierarchy
│   ├── legal-boundaries.md       # Legal and content boundaries
│   ├── research-methodology.md   # Research evidence standards
│   └── tooling-plan.md           # Planned local tooling workflow
├── research/
│   ├── data-df/                  # Metadata-only DATA.DF observations
│   ├── dvp/                      # Metadata-only DVP overlay observations
│   ├── elf/                      # Metadata-only ELF observations
│   ├── exe-refs/                 # Metadata-only executable reference observations
│   ├── external/                 # External tooling experiments (splat, Rabbitizer, SOTC)
│   ├── README.md                 # Research note organization
│   └── iso-layout/               # Metadata-only disc layout observations
├── splat/                        # splat64[mips] YAML config and Makefile
├── src/                          # Verified reverse-engineered sources
│   ├── types.h                   # ico_ptr32 and basic type definitions
│   ├── core/
│   │   ├── asm/                  # Byte-exact .s assembly (isysGObj* lifecycle helpers)
│   │   └── isys_process.h        # Core DL/process layout notes
│   ├── cloth/
│   │   ├── structs.h             # Cloth struct hierarchy (payload, entity, context)
│   │   ├── accessors.c           # 3 EXACT match functions
│   │   ├── asm/                  # Byte-exact .s assembly (16 cloth functions)
│   │   └── near_matches.c        # 3 NEAR-STRUCTURAL validated models
    │   └── entity/                   # Entity system structs and handlers (Rev.056)
│       ├── types.h               # 68-type enum, phy_type constants
│       ├── structs.h             # descriptor_record, entry_record, state_block
│       ├── asm/                  # Byte-exact .s assembly (10 entity functions)
│       ├── enemy1.c              # ENEMY1 full decompilation (hA/hB/hC/init_fn)
│       └── near_matches.c        # 4 NEAR-STRUCTURAL handler models (GIRL hA, QUEEN hA/init, BGA init)
├── tests/
│   └── fixtures/                 # Non-copyrighted parser/tooling fixtures
├── tools/
│   ├── asm_source_score.py       # Generate .s from ELF, assemble, verify byte-exact match
│   ├── ee_gcc_compile.py         # Old C-based scoring pipeline (archived for asm bypass)
│   ├── score_all.py              # Batch scorer for all 38 functions
│   ├── runtime-probe-analyzer/   # PCSX2 runtime event log analyzer
│   ├── data-df-index/            # Metadata-only DATA.DF triage tool
│   ├── dvp-index/                # Metadata-only DVP overlay indexer
│   ├── elf-index/                # Metadata-only ELF32 indexer
│   ├── exe-ref-index/            # Metadata-only executable reference scanner
│   ├── iso-index/                # Metadata-only ISO9660/BIN/CUE indexer
│   ├── README.md                 # Script and tooling conventions
│   └── verify-local-copy/        # Metadata-only local input verifier
```

## What This Project Is Not

This project is not:

- a PS2 emulator
- a game download
- a source port ready to build
- a mod loader
- a distribution of ICO assets, binaries, or proprietary data
- an attempt to bypass ownership of the original game

Any future tooling should require contributors to provide their own legally obtained copy of the game.

## Local Setup

There is currently no buildable source tree and no runtime to execute.

To work with the repository locally:

```bash
git clone https://github.com/odevpedro/ico-reconstruction.git
cd ico-reconstruction
```

Recommended first reading order:

1. `README.md`
2. `docs/backlog.md`
3. `docs/architecture-log.md`
4. `docs/architectural-analysis-a-d.md`
5. `docs/research-methodology.md`
6. `docs/legal-boundaries.md`
7. `docs/tooling-plan.md`

Future setup instructions will be added after the environment setup task defines reproducible tools for ISO inspection, ELF analysis, disassembly, debugging, and asset exploration.

Current metadata-only verifier:

```bash
python3 tools/verify-local-copy/verify_local_copy.py /path/to/local/user-owned/input
```

Generated reports are written to `.local/reports/`, which is ignored by git.

Current disc indexer:

```bash
python3 tools/iso-index/iso_index.py "/path/to/Ico (USA).bin" --cue "/path/to/Ico (USA).cue"
```

Current ELF indexer:

```bash
python3 tools/elf-index/elf_index.py --image "/path/to/Ico (USA).bin" --lba 25 --size 5481608 --sector-size 2352 --data-offset 24 --source-name SCUS_971.13
```

Current `DATA.DF` triage:

```bash
python3 tools/data-df-index/data_df_index.py --image "/path/to/Ico (USA).bin" --lba 2898 --size 539367424 --sector-size 2352 --data-offset 24 --source-name DFDATAS/DATA.DF
```

Current DVP overlay metadata indexer:

```bash
python3 tools/dvp-index/dvp_index.py --image "/path/to/Ico (USA).bin" --lba 25 --size 5481608 --sector-size 2352 --data-offset 24 --source-name SCUS_971.13 --data-df-size 539367424
```

Current executable reference scanner:

```bash
python3 tools/exe-ref-index/exe_ref_index.py --image "/path/to/Ico (USA).bin" --lba 25 --size 5481608 --sector-size 2352 --data-offset 24 --source-name SCUS_971.13 --query DATA.DF --query DFDATAS
```

Current MIPS split-immediate pattern scanner:

```bash
python3 tools/mips-immediate-scanner/mips_immediate_scanner.py --image "/path/to/Ico (USA).bin" --lba 25 --size 5481608 --sector-size 2352 --data-offset 24 --source-name SCUS_971.13 --target 0x00556a10 --target 0x00633b68
```

Current ELF symbol table scanner:

```bash
python3 tools/elf-symbol-scan/elf_symbol_scan.py --image "/path/to/Ico (USA).bin" --lba 25 --size 5481608 --sector-size 2352 --data-offset 24 --source-name SCUS_971.13
```

Current MIPS function prologue scanner:

```bash
python3 tools/mips-prologue-scan/mips_prologue_scan.py --image "/path/to/Ico (USA).bin" --lba 25 --size 5481608 --sector-size 2352 --data-offset 24 --source-name SCUS_971.13
```

Current function reference correlator:

```bash
python3 tools/function-ref-correlator/function_ref_correlator.py --immediate-report .local/reports/...-mips-immediate-scan.json --prologue-report .local/reports/...-mips-prologue-scan.json
```

Current MIPS call graph analyzer:

```bash
python3 tools/mips-call-graph/mips_call_graph.py --image "/path/to/Ico (USA).bin" --lba 25 --size 5481608 --sector-size 2352 --data-offset 24 --source-name SCUS_971.13 --prologue-report .local/reports/...-mips-prologue-scan.json --target 0x001321c8 --target 0x00132630
```

ELF extractor for disassembler import:

```bash
python3 tools/elf-extractor/elf_extractor.py --image "/path/to/Ico (USA).bin" --lba 25 --size 5481608 --sector-size 2352 --data-offset 24 --source-name SCUS_971.13 --output-dir .local/extracted
```

## Tests

No automated test suite exists yet because the repository currently contains planning and research documentation only.

Future tests should focus on:

- deterministic extraction outputs
- known file table parsing cases
- asset format parsing fixtures that do not contain copyrighted data
- checksum or structure validation for user-supplied local files
- regression tests for tooling behavior

## Why ICO?

ICO is an interesting PS2 reconstruction target because it is an early-generation title with a focused design, limited cast, sparse UI, contained environments, and a smaller gameplay surface than many later PS2 AAA games.

At the same time, ICO is still a complex 3D game with proprietary engine technology. Important technical risks include:

- MIPS executable reconstruction
- PS2 vector unit and rendering behavior
- proprietary model, animation, scene, and collision formats
- camera and event scripting
- Yorda companion behavior
- shadow enemy behavior
- room/scene loading
- audio and cutscene systems

The project treats these as research topics, not solved problems.

## Documentation

| Document | Purpose |
|----------|---------|
| [`docs/backlog.md`](./docs/backlog.md) | Current state, pending tasks, completed work, and revision signatures |
| [`docs/architecture-log.md`](./docs/architecture-log.md) | Historical record of implemented flows and decisions |
| [`docs/architectural-analysis-a-d.md`](./docs/architectural-analysis-a-d.md) | First subsystem feasibility and validation map |
| [`docs/research-methodology.md`](./docs/research-methodology.md) | Research evidence standards and observation format |
| [`docs/legal-boundaries.md`](./docs/legal-boundaries.md) | Content rules for legal and clean-room repository boundaries |
| [`docs/tooling-plan.md`](./docs/tooling-plan.md) | Planned local tooling direction before executable tools exist |
| [`docs/ai-onboarding.md`](./docs/ai-onboarding.md) | Required reading path and current handoff context for new AI agents |
| [`docs/local-logs-and-reports.md`](./docs/local-logs-and-reports.md) | Local project report directory and PCSX2 emulator log locations |
| [`docs/historia.md`](./docs/historia.md) | Narrative blog (first-person student persona, 10k words, 14 chapters) |
| [`docs/prompt_persona_ico_reconstruction.md`](./docs/prompt_persona_ico_reconstruction.md) | Blog persona prompt for narrative writing |
| [`research/README.md`](./research/README.md) | Organization rules for future research notes |
| [`tools/README.md`](./tools/README.md) | Scope and conventions for future local utilities |
| [`tools/verify-local-copy/README.md`](./tools/verify-local-copy/README.md) | Metadata-only local input verifier |
| [`tools/iso-index/README.md`](./tools/iso-index/README.md) | Metadata-only ISO9660/BIN/CUE indexer |
| [`research/iso-layout/ico-usa-bin-cue-initial-index.md`](./research/iso-layout/ico-usa-bin-cue-initial-index.md) | Initial confirmed metadata-only BIN/CUE layout observation |
| [`tools/elf-index/README.md`](./tools/elf-index/README.md) | Metadata-only ELF32 indexer |
| [`research/elf/ico-usa-scus-97113-elf-metadata.md`](./research/elf/ico-usa-scus-97113-elf-metadata.md) | Initial confirmed metadata-only ELF observation |
| [`tools/data-df-index/README.md`](./tools/data-df-index/README.md) | Metadata-only `DATA.DF` structural triage |
| [`research/data-df/ico-usa-data-df-initial-triage.md`](./research/data-df/ico-usa-data-df-initial-triage.md) | Initial confirmed metadata-only `DATA.DF` triage |
| [`research/data-df/ico-usa-data-df-dvp-targeted-scan.md`](./research/data-df/ico-usa-data-df-dvp-targeted-scan.md) | Targeted metadata-only `DATA.DF` scans around DVP numeric tokens |
| [`tools/dvp-index/README.md`](./tools/dvp-index/README.md) | Metadata-only `.DVP.*` overlay indexer |
| [`research/dvp/ico-usa-dvp-overlay-metadata.md`](./research/dvp/ico-usa-dvp-overlay-metadata.md) | Initial confirmed metadata-only `.DVP.*` overlay observation |
| [`tools/exe-ref-index/README.md`](./tools/exe-ref-index/README.md) | Metadata-only exact executable reference scanner |
| [`tools/mips-immediate-scanner/README.md`](./tools/mips-immediate-scanner/README.md) | Metadata-only MIPS split-immediate pattern scanner |
| [`research/exe-refs/ico-usa-scus-97113-data-df-dvp-references.md`](./research/exe-refs/ico-usa-scus-97113-data-df-dvp-references.md) | Initial confirmed metadata-only executable reference scan |
| [`research/exe-refs/ico-usa-scus-97113-mips-immediate-patterns.md`](./research/exe-refs/ico-usa-scus-97113-mips-immediate-patterns.md) | Confirmed metadata-only MIPS split-immediate pattern scan |
| [`research/exe-refs/ico-usa-continue-menu-breakpoint-candidates.md`](./research/exe-refs/ico-usa-continue-menu-breakpoint-candidates.md) | Continue menu breakpoint candidates and PCSX2 validation notes |
| [`tools/elf-symbol-scan/README.md`](./tools/elf-symbol-scan/README.md) | Metadata-only ELF symbol table scanner |
| [`research/elf/ico-usa-scus-97113-symbol-table-analysis.md`](./research/elf/ico-usa-scus-97113-symbol-table-analysis.md) | Confirmed ELF symbol table analysis (stripped) |
| [`tools/mips-prologue-scan/README.md`](./tools/mips-prologue-scan/README.md) | Metadata-only MIPS function prologue scanner |
| [`research/elf/ico-usa-scus-97113-mips-prologue-scan.md`](./research/elf/ico-usa-scus-97113-mips-prologue-scan.md) | Confirmed MIPS function prologue scan (3991 functions) |
| [`tools/function-ref-correlator/README.md`](./tools/function-ref-correlator/README.md) | Metadata-only function reference correlator |
| [`research/elf/ico-usa-scus-97113-function-reference-correlation.md`](./research/elf/ico-usa-scus-97113-function-reference-correlation.md) | Confirmed function reference correlation (7 functions) |
| [`tools/mips-call-graph/README.md`](./tools/mips-call-graph/README.md) | Metadata-only MIPS call graph analyzer |
| [`research/elf/ico-usa-scus-97113-call-graph-analysis.md`](./research/elf/ico-usa-scus-97113-call-graph-analysis.md) | Confirmed call graph analysis (13 callers, 15 calls) |
| [`research/elf/ico-usa-environment-setup-plan.md`](./research/elf/ico-usa-environment-setup-plan.md) | Environment setup plan for disassembly and emulation |
| [`research/elf/ghidra-rev023-dispatcher-table-resolution.md`](./research/elf/ghidra-rev023-dispatcher-table-resolution.md) | Confirmed dispatcher table resolution for `0x001d37c8` |
| [`research/elf/ghidra-rev033-node-callback-dispatch-chain.md`](./research/elf/ghidra-rev033-node-callback-dispatch-chain.md) | Static callback chain from record slot to `node +0x1c` dispatch |
| [`research/elf/ghidra-rev034-callback-signature-and-record-selection.md`](./research/elf/ghidra-rev034-callback-signature-and-record-selection.md) | Callback signature and record-selection analysis for the `ROPE` chain |
| [`research/elf/ghidra-rev035-entry-table-and-descriptor-correction.md`](./research/elf/ghidra-rev035-entry-table-and-descriptor-correction.md) | Entry-table and descriptor-index correction for the `ROPE` callback model |
| [`research/elf/ghidra-rev036-registration-path-survey.md`](./research/elf/ghidra-rev036-registration-path-survey.md) | Registration path survey: zero-guard in `0x001b76f8`, all five callers of `0x0013f7a8` |
| [`research/elf/ghidra-rev037-remaining-callers-and-rope-gap.md`](./research/elf/ghidra-rev037-remaining-callers-and-rope-gap.md) | Complete analysis of remaining callers of `0x0013f7a8` and the ROPE registration gap |
| [`research/elf/ghidra-rev079-runtime-validation-windmill-session.md`](./research/elf/ghidra-rev079-runtime-validation-windmill-session.md) | Runtime session at windmill (14M events): slot distribution inverted, 12 callbacks mapped, probes documented |
| [`research/runtime/pcsx2-recompiler-session3-2026-05-16.md`](./research/runtime/pcsx2-recompiler-session3-2026-05-16.md) | Runtime session 3: 0 hits at 0x1D3A30, 0x0024xxxx callers investigation |
| [`possible_ressources.md`](./possible_ressources.md) | Catalog of external projects, tools, and communities for integration |
| [`tools/elf-extractor/README.md`](./tools/elf-extractor/README.md) | ELF extractor for disassembler import |

## Initial Roadmap

1. **Architectural analysis**
   - Define feasibility by subsystem.
   - Identify what can be validated without the game binary and what requires empirical testing.

2. **Environment setup**
   - Establish reproducible tools for ISO inspection, ELF analysis, disassembly, debugging, and asset exploration.

3. **First visible proof of concept**
   - Produce a minimal, documented, non-distributable change from a user-provided copy of ICO.
   - Examples: string, texture, parameter, or simple data mutation.

4. **Subsystem mapping**
   - Map executable regions and data formats.
   - Prioritize actors, rooms, camera, collision, animation, events, and rendering.

5. **Tooling and documentation**
   - Build small tools only when they clarify repeatable workflows.
   - Document every confirmed format, function, and flow.

6. **Progressive reconstruction**
   - Reconstruct small verified pieces first.
   - Avoid large speculative rewrites until enough behavior is understood.

## Project Status

```text
[x] rev.001 - Initial strategic planning and prompt workflow
[x] rev.002 - Project retargeted to ICO Reconstruction
[x] rev.003 - Public README created for community collaboration
[x] rev.004 - README merged with repository template structure
[x] rev.005 - ICO wallpaper added to README
[x] rev.006 - Minimal GitHub folder structure added
[x] rev.006.1 - Documentation cleanup and base research guides
[x] rev.007 - Architectural analysis A-D for ICO
[x] rev.007.1 - Local metadata tooling foundation
[x] rev.007.2 - Local BIN/CUE disc index and initial observation
[x] rev.007.3 - Local ELF metadata index and initial observation
[x] rev.007.4 - Local DATA.DF metadata triage
[x] rev.007.5 - Local DVP overlay metadata correlation
[x] rev.007.6 - Targeted DATA.DF scans around DVP tokens
[x] rev.007.7 - Executable reference scan for DATA.DF and DVP tokens
[x] rev.007.8 - MIPS split-immediate pattern scan for DATA.DF and DFDATAS addresses
[x] rev.007.9 - ELF symbol table analysis (stripped executable)
[x] rev.008 - MIPS function prologue scan (3991 functions)
[x] rev.009 - Function reference correlation (7 functions identified)
[x] rev.010 - Call graph analysis (13 callers identified)
[x] rev.011 - Environment setup for disassembly and emulation (Ghidra headless with JDK 21)
[x] rev.012 - Function disassembly analysis (FUN_001b7288, FUN_001a6e28, FUN_001b0a80)
[x] rev.013 - Library calls analysis (top 25 functions identified)
[x] rev.014 - PS2 SDK function analysis (leaf functions dominant)
[x] rev.015 - First proof of concept: string extraction (camdata/*.gcm files found)
[x] rev.016 - Video/rendering functions identified (FUN_001b7288, FUN_001a6e28, FUN_001b0a80)
[x] rev.017 - Second proof of concept: string modification in ELF (NULL.gcm -> NULL0000)
[x] rev.018 - Multiple string modifications tested (title.gcm, logo.gcm, sacrifice.gcm)
[x] rev.019 - State resolver caller context analysis (0x0013eb50, entity structures, state ID mapping)
[x] rev.023 - Dispatcher table resolution (`0x001d37c8`, jump table `0x00618fb0`)
[x] rev.026-034 - Record/callback chain analysis around `ROPE`, `node +0x1c`, and `0x001d3a30`
[x] rev.035 - Entry-table and descriptor-index correction for the `ROPE` callback model
[x] rev.036 - Registration path survey: zero-guard confirmed in `0x001b76f8`, all five callers of `0x0013f7a8` mapped
[x] rev.037 - Remaining callers analyzed: three paths closed, two runtime-dependent candidates open; ROPE registration gap documented
[x] rev.038 - External integration: CCC debug symbol scan (none found), 6 decomp.me scratch packages generated, compiler flags refined, ICO-decomp cross-reference
[x] rev.039 - Cloth domain correction: dispatcher 0x001d37c8 and callback 0x001d3a30 reclassified as cloth physics, not entity/gameplay state
[x] rev.040 - Static cloth domain reinterpretation: auxiliary helpers mapped (0x001d2738, 0x001d29b8, 0x001d2bf0), 0x0013f7a8 corrected as not iosThreadStart
[x] rev.041 - Cloth variant table 0x004d4188: 8 entries stride 0x14, indexed by [state_block+0x04]
[x] rev.042 - Cloth variant field writers: 0x001d2858 confirmed, 0x001d1ad8 candidate, 0x001d390c discarded
[x] rev.043 - Cloth initializer arg source: 0x001d27a8 needs a1, [a1+0x30] origin still open
[x] rev.044 - Staged callback path: no static explanation for a1; 0x00129660 constructor-like found but excluded for ROPE static
[x] rev.045-047 - Runtime PCSX2: a1 source resolved (a1 = sp), descriptor callback model consolidated
[x] rev.048 - C scratch model synthesis: fixed taxonomy, ico_ptr32 rule, 9-function status matrix
[x] rev.049 - Physics object type table discovered (0x001A48A0, 31 types)
[x] rev.050 - Cloth system anatomy consolidated: cloth_payload_init decompiled, 0x1B76F8 identified, entry table fully mapped
[x] rev.051 - Runtime session 3: 0 hits at 0x1D3A30 (~90 min, 1419 events), 0x0024xxxx callers investigated, .text segment correction
[x] rev.052 - Five-way consolidation: descriptor table full map, sister_callbacks, event_clear decomp, VU0 cloth
[x] rev.053 - Handler decompilation wave 1: COP2 cloth, WOODBOX0, ENEMY1, BOY
[x] rev.054 - Handler decompilation wave 2: GIRL, QUEEN, BGA, AP1 — 12 functions disassembled
[x] rev.064 - Live dispatch at 0x00166E10: cold paths, struct map (400B stack, iterates desc array)
[x] rev.066 - Static live dispatch callsite map: GP slots confirmed, cold paths validated, 0x006AAC80 correction
[x] rev.067 - Consolidated dispatch model: slot table 0x282690 (17 entries, 14 parametric callbacks), runtime ptr list 0x6AAC80, alternate impl 0x169F80/0x16A058, runtime probe plan
[x] rev.069 - VU0 ring-buffer packet builder (0x1D43F8), kick stub (0x117C40), halfword table writers (0x00166D1C/0x00166D78), alternate constants, zero static callers for alternate selection
[x] rev.070 - Callers of 0x166028 (main loop 0x101C80, scene init 0x1AF4A0, entry iter 0x1B76F8), 404-byte stride entity table, rodata debug table with ClothInfo/CollisionOldProc
[x] rev.072 - Room init callback correction (offset +0x174), 19 function pointers, descriptor table (68 entries, 0x2A31B8), entry table (512 entries, 0x2A4C48), dead mult instruction
[x] rev.073 - Main loop dispatch chain (12 steps), corrected callback masks (bits 28-31), secondary pointer table (0x00633D30), struct field maps (80B/112B), linked-list flow
[x] rev.074 - Runtime session (9.1M events): slot 0 dead, slot 12 most active, alt_impl unused, VU0 kick only in gameplay, 58% match rate, 615 contexts/20 live entities
[x] rev.079 - Runtime session at windmill (14M events): slot distribution inverted (slot 1 #1 at 45.7%, slot 12 #2 at 37.1%), 12 callbacks fully mapped, mask_set 0 gameplay hits (confirmed 2nd session), slot 0 dead (2nd session), halfword writers 0 hits, alt_selection 0 hits, GP=0x6388F0 reconfirmed
[x] rev.084 - Extended runtime session (43.8M events, ~122 min): entrance→windmill→cutscene→3rd area. Slot 5+11 fired first time. Cutscene=100% slot12. All rare probes ZERO (3rd session). 1913 entity contexts. Slot 0 root cause (static: 0 dispatch sites). Total 66.9M events across 3 sessions.
[x] rev.085 - Death validation: mask_set = 0 hits during death. Death = 100% slot12 (Group 2 only). Confirms mask_set = ShockRequestBox_RequestCancel (I/O system).
[x] rev.086 - Final static analysis: +0x60 = behavior_fn (not vtable). Env effect table = 395-entry type-to-type matrix. cb_routine4 = no-op stubs. VBlank counter = IOP-driven. GIRL=DEVIL_GI confirmed.
[x] rev.092 - Path B milestone: all 26 LOW functions converted to byte-exact .s assembly via tools/asm_source_score.py. All 38 functions at 100% byte-exact match. C-based scoring pipeline archived.
[x] rev.093 - Three static investigations resolved: mask_set=ShockRequestBox_RequestCancel (I/O shock driver, loading-only, bit 0 only), dispatch table 0x282690 confirmed as compile-time .data (zero runtime writers), halfword spatial hash writers condition unknown (zero hits in 67.3M events across 4 areas).
```

## How To Contribute

Useful contributions at this stage:

- PS2 reverse engineering notes
- ICO file format observations
- ELF/disassembly analysis
- asset format identification
- documentation improvements
- reproducible tooling suggestions
- references to public PS2 decompilation/reconstruction techniques
- careful issue triage and research summaries

Please do not submit copyrighted assets, game binaries, ISO contents, or decompiled proprietary source copied from commercial material.

Suggested contribution flow:

1. Fork the repository.
2. Create a branch: `git checkout -b research/my-topic`.
3. Make a focused documentation or tooling change.
4. Update `docs/backlog.md` and `docs/architecture-log.md` when the change affects project state or technical flow.
5. Commit using Conventional Commits, for example: `docs: document initial ELF observations`.
6. Open a pull request explaining what was observed, how it was validated, and what remains uncertain.

## Suggested Research Areas

- Main ELF structure
- File table and archive formats
- Texture and model formats
- Room/scene data
- Collision data
- Actor/object system
- Animation data
- Camera and event triggers
- Yorda behavior
- Shadow enemy behavior
- Save/progression data
- Audio and cutscene packaging

## Repository Process

This project uses a documentation-first workflow.

Every meaningful contribution should update the relevant project documents:

- update `docs/backlog.md` when work starts, completes, blocks, or creates a new risk
- update `docs/architecture-log.md` when a technical flow, tool, or subsystem behavior is documented

Backlog entries use this signature format:

```text
[SQUAD-ID | STATUS | rev.XXX]
```

Common squad IDs:

- `SQUAD-ARCH`
- `SQUAD-RUNTIME`
- `SQUAD-TOOLING`
- `SQUAD-GAMEPLAY`
- `SQUAD-QA`

## Legal Boundary

This repository should contain only original research, documentation, scripts, and clean-room tooling.

Do not upload:

- ICO ISO files
- executable binaries extracted from the game
- copyrighted assets
- proprietary source code
- copyrighted text, audio, video, models, textures, or cutscenes

Any future tools should operate on a local copy supplied by the user.

## License

No license has been selected yet. Until a license is added, assume all rights are reserved for repository contents.

This does not grant rights to ICO, its assets, binaries, trademarks, audio, video, models, textures, scripts, or other proprietary game material.

## Status Summary

ICO Reconstruction is currently a feasibility and planning effort. The next useful milestone is an empirical proof of concept against a user-owned copy of ICO, followed by a subsystem-by-subsystem technical map.

---

Maintained by [@odevpedro](https://github.com/odevpedro) with a documentation-first research workflow.
