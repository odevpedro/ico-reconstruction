# Backlog — ICO Reconstruction

> Current project state and pending work. Updated in real-time during development.
> See `docs/architecture-log.md` for historical record of implemented features.

---

## Status Summary

| Category | Count |
|----------|-------|
| Completed | 31 |
| In Progress | 1 |
| Pending | 2 |

---

## In Progress

### [x] [SQUAD-RUNTIME | rev.019 | 2026-05-13]
Static Analysis Pipeline - State Resolver Caller Context

- Traced callers of 0x0013eb50 via manual MIPS disassembly
- Found parent functions 0x00199f80 (stack 0x130, entity dereference) and 0x0017bb98 (stack 0xf0, VU/floating-point)
- Identified sister function 0x0013ebe0 with similar call pattern
- Analyzed state tables: DAT_006321c0 (mode flag), DAT_006a93d0 (flat), DAT_00633ca0 (0x174-byte records)
- Clustered 146 callers of 0x0013eb50 into 14 address ranges; 0x0019xxxx has 38 callers (largest)

### [x] [SQUAD-RUNTIME | rev.020 | 2026-05-13]
UI String Context and Caller Analysis

- Mapped UI string locations: 0x005539a1 (continues), 0x005551f0 (pac_continueTag), 0x00555db6 (Continue)
- Disassembled 0x0011a520 (pac_continueTag loader) - uses continuation pattern, calls 0x001a6e28, 0x001ad768, 0x00263ff0
- Found 4 lui-pattern references to pac_continueTag: 0x00116f5c, 0x00117310, 0x0012850c, 0x001310ec
- Found 4 code references to "Continue" string: 0x0012a710, 0x0012a7a0, 0x0012a7f8, 0x00198710
- Updated Ghidra headless setup docs with working command template

### [x] [SQUAD-RUNTIME | rev.021 | 2026-05-13]
Continue Menu String Deception + Static Call Graph Deep Dive

- CRITICAL: "Continue" at 0x00555db6 is actually "ContinueAnimation:illegal Animation No." - debug string, NOT menu text
- Menu text is in TM2 textures (yesno_p*.tm2, conti_p*.tm2) - no ASCII for options
- Analyzed 0x0012a710 / 0x0012a618 - display/validation function, NOT continue menu trigger
- Analyzed 0x001d37c8 - entity state dispatcher with jump table at 0x00628fb0 (handles states 0-4)
- Analyzed 0x001d3a30 - entity update loop with ZERO static callers (vtable-only dispatch)
- Analyzed 0x00199c30 and 0x00199f80 - also zero static callers (vtable-dispatched handlers)
- 60 unique functions in 0x0019xxxx call state resolver 0x0013eb50
- All shadow references in ELF are about rendering/shadow maps, not Yorda capture
- New strategy: DATA.DF overlay analysis or memory search during gameplay, stop targeting ASCII strings

### [SQUAD-RUNTIME | rev.022 | In Progress]
Static Analysis Pipeline - Entity Structure and Call Graph

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

## Completed

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
