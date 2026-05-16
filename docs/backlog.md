# Backlog — ICO Reconstruction

> Current project state and pending work. Updated in real-time during development.
> See `docs/architecture-log.md` for historical record of implemented features.

---

## Status Summary

| Category | Count |
|----------|-------|
| Completed | 66 |
| In Progress | 0 |
| Pending | 2 |

---

## In Progress

_(none)_

---

## Pending

### [SQUAD-ARCH | rev.012 | Pending]
**Architectural Analysis E-G for ICO**

- Decision matrix for technical approach
- Squad architecture definition
- Final recommendation
- Black box identification

---

### [SQUAD-RUNTIME | Pending | Pending]
**Asset Format and Runtime Validation Expansion**

- Investigate `.gcm` file references and storage format
- Locate UI text or texture-backed text sources
- Analyze file-loading and save/load code paths
- Test numeric/value patches only on local modified BINs
- Deliverable: reproducible local-only notes and tooling that do not distribute proprietary data

---

### [SQUAD-TOOLING | Pending | Pending]
**PCSX2 Generic Runtime Logpoints Proposal**

- Keep current ICO-specific PCSX2 probe as local POC only
- Stabilize the POC across at least one more focused runtime session
- Separate generic debugger/logpoint behavior from ICO-specific target addresses
- Evaluate an upstream-friendly design for PCSX2: configurable addresses, register filters, structured logs, optional memory windows, and no mandatory pause
- Open PCSX2 issue/discussion before any PR if the design remains useful after stabilization
- Deliverable: short technical proposal that does not include game data or project-specific hardcoded addresses

---

## Completed

### [x] [SQUAD-RUNTIME | rev.046 | 2026-05-15]
Runtime Capture — a1 source resolved in 0x001D27A8

- Breakpoint em 0x001D27A8 disparou com sucesso via PCSX2 debugger
- a1 = sp = 0x00798E40 — initializer struct na stack do caller
- Caller identificado: 0x001B7A74-0x001B7A8C (jalr via descriptor+0x58)
- Descriptor = 0x002A3924 (próximo ao ROPE 0x002A3974)
- [a1+0x30] = 0 (variant inicial = 0)
- [a1+0x58] = 0 (callback está no descriptor, não no initializer)
- Initializer struct contém dados de transform/pose (posição X/Y/Z)
- Gap aberto desde Rev.037 resolvido
- Documentado em research/elf/ghidra-rev046-runtime-a1-source-resolved.md

### [x] [SQUAD-TOOLING | 2026-05-15]
decomp.me scratches regenerated + ee-gcc toolchain installed

- 6 scratches regenerated in /tmp/decompme_scratches/ (18 files: .s + .bin.hex + .meta.json each)
- Functions: cloth_dispatcher, cloth_update_callback, callback_storage, callback_register, cb48_dispatcher, cloth_payload_init
- Prebuilt ps2dev/ps2dev toolchain (GCC 15.2.0) installed to ~/ps2dev/
- EE GCC compiles R5900 MIPS64 code successfully (64-bit ELF output)
- Full build pipeline tested: assembly works, linking needs path adjustments
- Note: matching ee-gcc 2.9-991111-01 requires Sony PS2 Linux SDK (GCC 2.95.2 with R5900 patches)
- decomp.me has NO ee-gcc compiler packages — only Metrowerks mwcps2 for PS2

### [x] [SQUAD-EXTERNAL | rev.048 | 2026-05-16]
C scratch model synthesis — fixed taxonomy, ico_ptr32 rule, 9-function status matrix

- Fixed taxonomy: EXACT / NEAR-STRUCTURAL / NEAR-LOCAL / MISMATCH / BLOCKED / ASM-HOLD
- 3 EXACT matches (bit-identical C for 0x1D3D70, 0x1D3D80, 0x1D3D98)
- 5 NEAR-STRUCTURAL validated models (0x1D3DB0, 0x1D3D40, 0x1D40A0, 0x1D4358, 0x1D3BF0)
- 1 BLOCKED (dispatcher 0x1D37C8 due to jump table)
- ico_ptr32 (typedef int) confirmed across all 8 tested functions
- GCC 2.95.2 limitations documented (7 items)
- Documented in research/elf/ghidra-rev048-c-scratch-model-and-ico_ptr32.md

### [x] [SQUAD-EXTERNAL | rev.049 | 2026-05-16]
Physics object type table discovered — ROPE callback lives in static table

- 31 physics object types at 0x001A48A0, stride 0x64
- ROPE entry has handlers 0x1D3B28, 0x1D3A30, 0x1D27A8
- 0x001D3A30 NOT registered via 0x0013F7A8 (confirmed by 483 runtime events)
- ROPE gap redefined: find table reader, not registration path
- Documented in research/elf/ghidra-rev049-physics-object-type-table.md

### [x] [SQUAD-ARCH | rev.050 | 2026-05-16]
Cloth system anatomy consolidated — cloth_payload_init decompiled, 0x1B76F8 identified as descriptor iteration, entry table fully mapped

- cloth_payload_init (0x1D27A8): 2 paths controlled by variant (==1 full init, !=1 quick path)
- 0x1B76F8 identified as the real descriptor iteration function (not mystery table reader)
- Entry table at 0x002A4C48 (512 entries, stride 0x4C) fully scanned
- **No entry has +0x46=0x14 (ROPE)** — cloth objects use BARREL (index 0x13)
- BARREL shares same handlers as ROPE: 0x1D3B28/+0x48, 0x1D3A30/+0x50, 0x1D27A8/+0x58
- Why 0x1D3A30 never appears in 0x13F7A8 logs: entries BARREL have +0x24=0, BARREL descriptor has +0x40=0
- Callback registration in 0x1B76F8 uses entry[+0x24] → 0x13F7A8(a3=0x13), or descriptor[+0x40] → 0x13F7A8
- Documented in research/elf/ghidra-rev050-cloth-system-anatomy.md

### [x] [SQUAD-RUNTIME | rev.051 | 2026-05-16]
Runtime session 3 — 0x1D3A30 probe + 0x0024xxxx callers investigation

- Breakpoint 0x1D3A30 added to PCSX2 instrumented build
- ~90 min gameplay across varied areas (cable car, castle, Yorda, animations)
- **1419 eventos capturados: ZERO hits at 0x1D3A30**
- 145 cloth_payload_init hits with 50/50 variant split
- 1249 callback_register hits, all a3=0x13, 10 distinct callback types, none is 0x1D3A30
- .text section corrected: 0x00100000..0x0026F5D4 (not 0x001Fxxxx)
- Callers 0x00240E58/0x00240F98 traced to functions 0x240D40/0x240EA0 (object factories + multi-callback registration)
- Both callers definitively excluded as path for ROPE callback
- Documented in research/runtime/pcsx2-recompiler-session3-2026-05-16.md

### [x] [SQUAD-ARCH | rev.052 | 2026-05-16]
Five-way consolidation — descriptor table full map, sister_callbacks, event_clear decomp, VU0 cloth

- Descriptor table at 0x002A31B8 fully mapped: 68 entries, stride 0x64, with init_fn (+0x40), handlers A/B/C
- **CORREÇÃO: 0x1D3A30 é BARREL hB (índice 19), NÃO ROPE**. ROPE (índice 20) tem handlers completamente diferentes (0x1E9630/0x1E9810/0x1E8F38)
- 12 entries com init_fn não-nulo (BOY, GIRL, ENEMY1, WOODBOX0, BGA, BIRD, QUEEN, DEVIL_GI, AP1, ATTACKCH×2, BOSS_CTR)
- sister_callback_reg (0x13F778) decompilado: especialização de 0x13F7A8 com t1=0x1800 fixo
- cloth_event_clear (0x1AE6F8) decompilado: leaf function, tabela em 0x004B3D10, stride 0x40, ~182 entries
- DVP overlays confirmados como VU0 microcode (12 entries, payloads vazios no ELF, carregados de DATA.DF)
- VU0 cloth: 20KB .vutext microcode + 63 COP2 instructions no range cloth
- Modelo híbrido EE+VU0 para física cloth; hB é event-driven, não per-frame
- Documented in research/elf/ghidra-rev052-five-way-consolidation.md

- Breakpoint 0x1D3A30 added to PCSX2 instrumented build
- ~90 min gameplay across varied areas (cable car, castle, Yorda, animations)
- **1419 eventos capturados: ZERO hits at 0x1D3A30**
- 145 cloth_payload_init hits with 50/50 variant split
- 1249 callback_register hits, all a3=0x13, 10 distinct callback types, none is 0x1D3A30
- .text section corrected: 0x00100000..0x0026F5D4 (not 0x001Fxxxx)
- Callers 0x00240E58/0x00240F98 traced to functions 0x240D40/0x240EA0 (object factories + multi-callback registration)
- Both callers definitively excluded as path for ROPE callback
- Documented in research/runtime/pcsx2-recompiler-session3-2026-05-16.md

### [x] [SQUAD-ARCH | 2026-05-16]
Cloth struct model committed — first C source files in repo

- src/types.h, src/cloth/structs.h, src/cloth/accessors.c, src/cloth/near_matches.c
- docs/data-model.md: full data model with ADRs
- docs/system-feature-flows.md: cloth dispatch flow + type table init
- README.md updated with new structure and findings

### [x] [SQUAD-TOOLING | 2026-05-15]
SDK/library recognition — PS2 SDK functions identified in USA .text

- Cross-referenced 959 PAL SDK symbols against USA binary via ICO-decomp
- Confirmed **183 libkernl EE kernel functions** at exact same addresses (threading, interrupts, DMA, SIF, semaphores, cache)
- **Key negative finding**: libc, libm, libgcc, libsndn2, libpad, libmc, libmpeg, libipu, libscf are **NOT in USA .text** — USA .text is 108K smaller than PAL, matching the size of these libraries
- USA likely uses inlined libc or different SDK linking strategy
- Documented in research/external/sdk-library-recognition.md

### [x] [SQUAD-TOOLING | 2026-05-15]
Full cloth cluster splat promotion — 22 functions isolated

- Comprehensive splat YAML created (splat/SCUS_971.13.cloth-full.yaml)
- All 22 spimdisasm-detected cloth-domain functions promoted (0x1d27a8-0x1d45b0)
- 30 asm segments, 100% coverage, zero errors
- Makefile with EE GCC toolchain requirements (splat/Makefile)
- Documented in research/external/ico-splat-cloth-full-promotion.md

### [x] [SQUAD-TOOLING | 2026-05-15]
External splat tooling experiments (5 notes)

- SOTC tooling relevance survey (research/external/sotc-tooling-relevance-survey.md)
- Rabbitizer/spimdisasm independent anchor validation (research/external/ico-rabbitizer-spimdisasm-dispatcher-check.md)
- Minimal splat experiment confirmed viable (research/external/ico-splat-minimal-experiment.md)
- Promoted verified ranges — 4 functions isolated (research/external/ico-splat-promoted-ranges-experiment.md)
- Adjacent promoted ranges — 3 more functions (research/external/ico-splat-adjacent-promoted-ranges-experiment.md)

### [x] [SQUAD-RUNTIME | rev.045 | 2026-05-15]
Runtime plan for tomorrow — checkpoint before PCSX2 capture

- Static analysis exhausted; a1 source for 0x001d27a8 requires runtime breakpoint
- Priority breakpoints defined: 0x001d27a8 (a0, a1, [a1+0x30]), 0x0013f7a8 (a1 when a3==0x13), 0x001d37c8 (state_id distribution)
- Runtime capture automation plan documented (research/runtime-capture-automation-plan.md)
- Documented in research/elf/ghidra-rev045-runtime-plan-for-tomorrow.md

### [x] [SQUAD-EXTERNAL | rev.044 | 2026-05-14]
Staged Callback / Storage Path for 0x001d27a8

- No staged callback path found that explains 0x001d27a8(a0, a1)
- 0x0013f3f0 → node+0x1c → 0x0013fb70 passes only a0
- +0x48 dispatchers in 0x0013fc00 both prepare only a0
- Static options for a1 origin exhausted — next step is runtime breakpoint
- Compiler confirmed: EE GCC 2.9-991111-01, flags: -march=r5900 -mips3 -mgp64 -mabi=eabi -msingle-float -G0 -O2
- Documented in research/elf/ghidra-rev044-staged-callback-path-001d27a8.md

### [x] [SQUAD-EXTERNAL | rev.043 | 2026-05-14]
Cloth Initializer Argument Source

- 0x001d27a8 consumes a0 (context) and a1 (second structure pointer)
- [a1+0x30] copied to [payload+0x04] at 0x001d2858
- Known +0x48 dispatcher only prepares a0, not a1
- No direct jal to 0x001d27a8 — reached via callback dispatch
- Documented in research/elf/ghidra-rev043-cloth-initializer-arg-source.md

### [x] [SQUAD-EXTERNAL | rev.042 | 2026-05-14]
Cloth Variant Field Writers

- Confirmed writer of variant/mode field [payload+0x04]: 0x001d2858 (copies [initializer_arg+0x30])
- Documented in research/elf/ghidra-rev042-cloth-variant-field-writers.md

### [x] [SQUAD-EXTERNAL | rev.041 | 2026-05-14]
Cloth Variant Table 0x004d4188

- Mapped as cloth-domain table of 8 entries, stride 0x14
- Documented in research/elf/ghidra-rev041-cloth-variant-table-004d4188.md

### [x] [SQUAD-EXTERNAL | 2026-05-14]
ICO-decomp Cross-Reference

- RossyDoubleUnderscore/ICO-decomp cross-reference (5792 symbols, 1174 subsegments)
- Critical discovery: dispatcher in sugipon/src/clothAnimation.c (cloth physics)
- "ROPE" in ICO-decomp refers to gameplay ropes — different from our .data descriptor
- Documented in research/ico-decomp-cross-reference-2026-05-14.md

### [x] [SQUAD-EXTERNAL | rev.040 | 2026-05-14]
Static cloth domain reinterpretation

### [x] [SQUAD-EXTERNAL | rev.039 | 2026-05-14]
Cloth domain correction — dispatcher/callback reclassified as cloth physics

### [x] [SQUAD-ARCH | rev.001 | 2026-05-12]
Initial strategic planning and prompt workflow

### [x] [SQUAD-ARCH | rev.002 | 2026-05-12]
Project retargeted to ICO Reconstruction

### [x] [SQUAD-ARCH | rev.003 | 2026-05-12]
Public README created for community collaboration

### [x] [SQUAD-ARCH | rev.004 | 2026-05-12]
README merged with repository template structure

### [x] [SQUAD-ARCH | rev.005 | 2026-05-12]
ICO wallpaper added to README

### [x] [SQUAD-ARCH | rev.006 | 2026-05-12]
Minimal GitHub folder structure added

### [x] [SQUAD-ARCH | rev.006.1 | 2026-05-12]
Documentation cleanup and base research guides

### [x] [SQUAD-ARCH | rev.007 | 2026-05-12]
Architectural Analysis A-D for ICO

- Feasibility assessment by subsystem
- Identified validatable items without game binary
- Identified items requiring empirical testing
- Deliverable: `docs/architectural-analysis-a-d.md`

### [x] [SQUAD-TOOLING | rev.007.1 | 2026-05-12]
Local metadata tooling foundation

- Added local binary/output `.gitignore` safety rules
- Added GitHub issue templates for research, tooling, legal/content concerns, and subsystem mapping
- Added `tools/verify-local-copy/` metadata-only verifier
- Added a synthetic metadata fixture for validation

### [x] [SQUAD-TOOLING | rev.007.2 | 2026-05-12]
Local BIN/CUE disc index and initial observation

- Added `tools/iso-index/` metadata-only ISO9660/BIN/CUE indexer
- Ran metadata-only indexing against local `Ico (USA).bin` with CUE context
- Recorded confirmed safe metadata in `research/iso-layout/ico-usa-bin-cue-initial-index.md`
- Identified `SCUS_971.13` as the main executable candidate and `DFDATAS/DATA.DF` as the primary large data/archive candidate

### [x] [SQUAD-TOOLING | rev.007.3 | 2026-05-12]
Local ELF metadata index and initial observation

- Added `tools/elf-index/` metadata-only ELF32 indexer
- Ran ELF metadata indexing against embedded `SCUS_971.13`
- Recorded confirmed safe metadata in `research/elf/ico-usa-scus-97113-elf-metadata.md`
- Confirmed one `PT_LOAD` segment, 27 section headers, no symbol table, and `.DVP.overlay...` sections requiring follow-up

### [x] [SQUAD-TOOLING | rev.007.4 | 2026-05-12]
Local DATA.DF metadata triage

- Added `tools/data-df-index/` metadata-only structural triage tool
- Ran head/middle/tail sampling against embedded `DFDATAS/DATA.DF`
- Recorded confirmed safe metadata in `research/data-df/ico-usa-data-df-initial-triage.md`
- Found no simple head-window offset table candidate under current heuristic

### [x] [SQUAD-TOOLING | rev.007.5 | 2026-05-12]
Local DVP overlay metadata correlation

- Added `tools/dvp-index/` metadata-only `.DVP.*` overlay indexer
- Correlated `.DVP.ovlytab`, `.DVP.ovlystrtab`, and `.DVP.overlay...` metadata with ELF load range and `DATA.DF` size context
- Recorded confirmed safe metadata in `research/dvp/ico-usa-dvp-overlay-metadata.md`
- Identified 12 overlay table entries and 12 overlay string-table entries

### [x] [SQUAD-TOOLING | rev.007.6 | 2026-05-12]
Targeted DATA.DF scans around DVP tokens

- Extended `tools/data-df-index/` with repeatable `--target-offset` scans
- Ran targeted windows around DVP numeric tokens
- Recorded confirmed safe metadata in `research/data-df/ico-usa-data-df-dvp-targeted-scan.md`
- Found no simple local offset table or fixed-record candidates around tested DVP tokens

### [x] [SQUAD-TOOLING | rev.007.7 | 2026-05-12]
Executable reference scan for DATA.DF and DVP tokens

- Added `tools/exe-ref-index/` metadata-only exact reference scanner
- Scanned embedded `SCUS_971.13` for `DATA.DF`, `DFDATAS`, DVP terms, and DVP numeric constants
- Recorded confirmed safe metadata in `research/exe-refs/ico-usa-scus-97113-data-df-dvp-references.md`
- Confirmed direct `DATA.DF` and `DFDATAS` string references in executable data sections

### [x] [SQUAD-TOOLING | rev.007.8 | 2026-05-12]
MIPS split-immediate pattern scan for DATA.DF and DFDATAS addresses

- Added `tools/mips-immediate-scanner/` metadata-only MIPS split-immediate pattern scanner
- Scanned embedded `SCUS_971.13` for lui/addiu patterns referencing known virtual addresses
- Recorded confirmed safe metadata in `research/exe-refs/ico-usa-scus-97113-mips-immediate-patterns.md`
- Found 8 pattern matches for addresses 0x00556a10, 0x00556a20, 0x006127e8, and 0x00633b68
- DATA.DF at 0x00556a28 had no split-immediate patterns found in .text section

### [x] [SQUAD-TOOLING | rev.007.9 | 2026-05-12]
ELF symbol table analysis (stripped executable)

- Added `tools/elf-symbol-scan/` metadata-only ELF symbol table scanner
- Scanned embedded `SCUS_971.13` for .symtab, .dynsym, and dynamic linking structures
- Recorded confirmed metadata in `research/elf/ico-usa-scus-97113-symbol-table-analysis.md`
- Confirmed executable is completely stripped: no .symtab, no .dynsym, no PT_DYNAMIC

### [x] [SQUAD-TOOLING | rev.008 | 2026-05-12]
MIPS function prologue scan

- Added `tools/mips-prologue-scan/` metadata-only MIPS function prologue scanner
- Scanned embedded `SCUS_971.13` for addiu $sp patterns
- Recorded confirmed metadata in `research/elf/ico-usa-scus-97113-mips-prologue-scan.md`
- Found 3,991 function prologues with 71 unique stack sizes in .text section

### [x] [SQUAD-TOOLING | rev.009 | 2026-05-12]
Function reference correlation

- Added `tools/function-ref-correlator/` metadata-only function reference correlator
- Correlated prologue and immediate scan data to identify 7 functions with DATA.DF/DFDATAS references
- Recorded confirmed metadata in `research/elf/ico-usa-scus-97113-function-reference-correlation.md`
- Identified function addresses: 0x001321c8, 0x00132630, 0x00132ff0, 0x00185ca8, 0x0019fb34, 0x001a0a38, 0x0023d468

### [x] [SQUAD-TOOLING | rev.010 | 2026-05-12]
Call graph analysis

- Added `tools/mips-call-graph/` metadata-only MIPS call graph analyzer
- Scanned for jal/jalr instructions targeting the 7 known functions
- Found 15 calls from 13 unique caller functions
- Recorded confirmed metadata in `research/elf/ico-usa-scus-97113-call-graph-analysis.md`

---

## Revision Signatures

| Revision | Date | Squad | Summary |
|----------|------|-------|---------|
| rev.001 | 2026-05-12 | SQUAD-ARCH | Initial strategic planning and prompt workflow |
| rev.002 | 2026-05-12 | SQUAD-ARCH | Project retargeted to ICO Reconstruction |
| rev.003 | 2026-05-12 | SQUAD-ARCH | Public README created for community collaboration |
| rev.004 | 2026-05-12 | SQUAD-ARCH | README merged with repository template structure |
| rev.005 | 2026-05-12 | SQUAD-ARCH | ICO wallpaper added to README |
| rev.006 | 2026-05-12 | SQUAD-ARCH | Minimal GitHub folder structure added |
| rev.006.1 | 2026-05-12 | SQUAD-ARCH | Documentation cleanup and base research guides |
| rev.007 | 2026-05-12 | SQUAD-ARCH | Architectural analysis A-D for ICO |
| rev.007.1 | 2026-05-12 | SQUAD-TOOLING | Local metadata tooling foundation |
| rev.007.2 | 2026-05-12 | SQUAD-TOOLING | Local BIN/CUE disc index and initial observation |
| rev.007.3 | 2026-05-12 | SQUAD-TOOLING | Local ELF metadata index and initial observation |
| rev.007.4 | 2026-05-12 | SQUAD-TOOLING | Local DATA.DF metadata triage |
| rev.007.5 | 2026-05-12 | SQUAD-TOOLING | Local DVP overlay metadata correlation |
| rev.007.6 | 2026-05-12 | SQUAD-TOOLING | Targeted DATA.DF scans around DVP tokens |
| rev.007.7 | 2026-05-12 | SQUAD-TOOLING | Executable reference scan for DATA.DF and DVP tokens |
| rev.007.8 | 2026-05-12 | SQUAD-TOOLING | MIPS split-immediate pattern scan for DATA.DF and DFDATAS addresses |
| rev.007.9 | 2026-05-12 | SQUAD-TOOLING | ELF symbol table analysis (stripped executable) |
| rev.008 | 2026-05-12 | SQUAD-TOOLING | MIPS function prologue scan |
| rev.009 | 2026-05-12 | SQUAD-TOOLING | Function reference correlation (7 functions identified) |
| rev.010 | 2026-05-12 | SQUAD-TOOLING | Call graph analysis (13 callers identified) |
| rev.011 | 2026-05-12 | SQUAD-RUNTIME | Environment setup for disassembly (Ghidra + PCSX2) |
| rev.012 | 2026-05-12 | SQUAD-RUNTIME | Function disassembly analysis |
| rev.013 | 2026-05-12 | SQUAD-RUNTIME | Library calls analysis (top 25 functions) |
| rev.014 | 2026-05-12 | SQUAD-RUNTIME | PS2 SDK function analysis (leaf functions) |
| rev.015 | 2026-05-12 | SQUAD-TOOLING | First PoC: string extraction (81 .gcm files found) |
| rev.016 | 2026-05-12 | SQUAD-RUNTIME | Video/rendering functions identified |
| rev.017 | 2026-05-12 | SQUAD-TOOLING | Second PoC: string modification (NULL.gcm -> NULL0000) |
| rev.018 | 2026-05-12 | SQUAD-TOOLING | Multiple string modifications tested (title.gcm, logo.gcm, sacrifice.gcm) |
| rev.019 | 2026-05-13 | SQUAD-RUNTIME | Static analysis - state resolver caller context (146 callers, 14 clusters) |
| rev.020 | 2026-05-13 | SQUAD-RUNTIME | UI string context and caller analysis |
| rev.021 | 2026-05-13 | SQUAD-RUNTIME | Continue menu string deception + vtable call graph deep dive |
| rev.022 | 2026-05-13 | SQUAD-RUNTIME | Dispatcher ground truth — jump table address corrected to 0x00618fb0 |
| rev.023 | 2026-05-13 | SQUAD-RUNTIME | Dispatcher table resolution — confirmed 5-state dispatch model |
| rev.024 | 2026-05-13 | SQUAD-RUNTIME | Internal state block semantics — 5 blocks analyzed |
| rev.025 | 2026-05-13 | SQUAD-RUNTIME | Runtime-confirmed caller context (0x001d3a30) |
| rev.026 | 2026-05-13 | SQUAD-RUNTIME | ROPE record table context and descriptor structure |
| rev.027 | 2026-05-13 | SQUAD-RUNTIME | ROPE state block initializer analysis |
| rev.028 | 2026-05-13 | SQUAD-RUNTIME | State block provider contract (0x0013a0f8) |
| rev.029 | 2026-05-13 | SQUAD-RUNTIME | State block provider deeper static analysis |
| rev.030 | 2026-05-13 | SQUAD-RUNTIME | Provider caller survey — allocator pattern |
| rev.031 | 2026-05-13 | SQUAD-RUNTIME | Record callback dispatchers |
| rev.032 | 2026-05-13 | SQUAD-RUNTIME | Static callback follow-through |
| rev.033 | 2026-05-13 | SQUAD-RUNTIME | Node callback dispatch chain — store in node+0x1c |
| rev.034 | 2026-05-13 | SQUAD-RUNTIME | Callback signature and record selection |
| rev.035 | 2026-05-13 | SQUAD-RUNTIME | Entry table and descriptor correction — ROPE index fixed to 0x14 |
| rev.036 | 2026-05-13 | SQUAD-RUNTIME | Registration path survey — 5 callsites of 0x0013f7a8 |
| rev.037 | 2026-05-13 | SQUAD-RUNTIME | Remaining callers and ROPE registration gap — static options exhausted |
| rev.038 | 2026-05-13 | SQUAD-EXTERNAL | decomp.me scratch generation + CCC debug symbol scan (none found) |
| 2026-05-14 | 2026-05-14 | SQUAD-EXTERNAL | ICO-decomp cross-reference: cloth physics, source tree mapping |
| rev.039 | 2026-05-14 | SQUAD-EXTERNAL | Cloth domain correction — dispatcher/callback reclassified as cloth physics |
| rev.040 | 2026-05-14 | SQUAD-EXTERNAL | Static cloth domain reinterpretation + auxiliary helper mapping |
| rev.041 | 2026-05-14 | SQUAD-EXTERNAL | Cloth variant table 0x004d4188: 8 entries stride 0x14 |
| rev.042 | 2026-05-14 | SQUAD-EXTERNAL | Cloth variant field writers: 0x001d2858 confirmed, 0x001d1ad8 candidate, 0x001d390c discarded |
| rev.043 | 2026-05-14 | SQUAD-EXTERNAL | Cloth initializer arg source: 0x001d27a8 needs a1, [a1+0x30] origin open |
| rev.044 | 2026-05-14 | SQUAD-EXTERNAL | Staged callback path: no static explanation for a1 |
| rev.045 | 2026-05-15 | SQUAD-RUNTIME | Runtime plan for tomorrow — checkpoint before PCSX2 capture |
| rev.046 | 2026-05-15 | SQUAD-RUNTIME | Runtime capture: a1 source resolved — a1 = sp, caller = 0x001B7A74 |
| rev.047 | 2026-05-15 | SQUAD-RUNTIME | Descriptor Callback Runtime Model — modelo consolidado pós-runtime |
| 2026-05-15 | 2026-05-15 | SQUAD-TOOLING | External splat tooling experiments (SOTC survey, Rabbitizer, 3 splat experiments) |
| 2026-05-15 | 2026-05-15 | SQUAD-TOOLING | Full cloth cluster splat promotion — 22 functions isolated, YAML in splat/ |
| 2026-05-15 | 2026-05-15 | SQUAD-TOOLING | SDK/library recognition — 183 libkernl functions confirmed; libc absent from USA .text |
| 2026-05-15 | 2026-05-15 | SQUAD-TOOLING | decomp.me scratches regenerated + ee-gcc 15.2.0 toolchain installed |
| rev.048 | 2026-05-16 | SQUAD-EXTERNAL | C scratch model synthesis — fixed taxonomy, ico_ptr32 rule, 9-function status matrix |
| rev.049 | 2026-05-16 | SQUAD-EXTERNAL | Physics object type table — ROPE in static table, not dynamic registry |
| rev.050 | 2026-05-16 | SQUAD-ARCH | Cloth system anatomy — cloth_payload_init decompiled, 0x1B76F8 identified, entry table fully mapped |
| 2026-05-16 | 2026-05-16 | SQUAD-ARCH | First C source files committed (struct model + accessors + near matches) |
| 2026-05-16 | 2026-05-16 | SQUAD-ARCH | docs/data-model.md + docs/system-feature-flows.md created |
| rev.051 | 2026-05-16 | SQUAD-RUNTIME | Runtime session 3: 0 hits at 0x1D3A30 across ~90 min / 1419 events — refutes per-frame model |
| rev.052 | 2026-05-16 | SQUAD-ARCH | Five-way consolidation: descriptor map (68 entries), sister_callbacks, event_clear decomp, VU0 cloth physics |
| 2026-05-16 | 2026-05-16 | SQUAD-RUNTIME | 0x0024xxxx callers investigated: 0x240D40/0x240EA0 are object factories, excluded for ROPE callback |
| 2026-05-16 | 2026-05-16 | SQUAD-ARCH | .text end corrected: 0x0026F5D4 (not 0x001Fxxxx) |

---

## Squad Definitions

| Squad | Focus Area |
|-------|------------|
| SQUAD-ARCH | Architecture, documentation, project structure |
| SQUAD-RUNTIME | Runtime environment, extraction, disassembly |
| SQUAD-TOOLING | Scripts, parsers, validation tools |
| SQUAD-GAMEPLAY | Gameplay systems, actors, events |
| SQUAD-QA | Quality assurance, validation, testing |

---

## Operating Rules

1. When starting a task: move to "In Progress"
2. When completing: mark as `[x]` and move to "Completed"
3. When identifying new work: add to "Pending"
4. All completed items remain in "Completed" section
5. Update `docs/architecture-log.md` when documenting features
