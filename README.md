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
| Disassembly | To be defined during environment setup |
| Archive/data exploration | `tools/data-df-index` for metadata-only `DATA.DF` structural triage |
| Validation | `tools/verify-local-copy`, reproducible notes, metadata reports, emulator/debugger evidence where applicable |
| CI/CD | Not configured yet |
| Tests | No automated suite yet; synthetic fixture used for local tool validation |

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

The project is currently in the **planning and architecture phase**.

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
│   ├── legal-boundaries.md       # Legal and content boundaries
│   ├── research-methodology.md   # Research evidence standards
│   └── tooling-plan.md           # Planned local tooling workflow
├── research/
│   ├── data-df/                  # Metadata-only DATA.DF observations
│   ├── dvp/                      # Metadata-only DVP overlay observations
│   ├── elf/                      # Metadata-only ELF observations
│   ├── exe-refs/                 # Metadata-only executable reference observations
│   ├── README.md                 # Research note organization
│   └── iso-layout/               # Metadata-only disc layout observations
├── tests/
│   └── fixtures/                 # Non-copyrighted parser/tooling fixtures
└── tools/
    ├── data-df-index/            # Metadata-only DATA.DF triage tool
    ├── dvp-index/                # Metadata-only DVP overlay indexer
    ├── elf-index/                # Metadata-only ELF32 indexer
    ├── exe-ref-index/            # Metadata-only executable reference scanner
    ├── iso-index/                # Metadata-only ISO9660/BIN/CUE indexer
    ├── README.md                 # Script and tooling conventions
    └── verify-local-copy/        # Metadata-only local input verifier
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
| [`tools/elf-symbol-scan/README.md`](./tools/elf-symbol-scan/README.md) | Metadata-only ELF symbol table scanner |
| [`research/elf/ico-usa-scus-97113-symbol-table-analysis.md`](./research/elf/ico-usa-scus-97113-symbol-table-analysis.md) | Confirmed ELF symbol table analysis (stripped) |
| [`tools/mips-prologue-scan/README.md`](./tools/mips-prologue-scan/README.md) | Metadata-only MIPS function prologue scanner |
| [`research/elf/ico-usa-scus-97113-mips-prologue-scan.md`](./research/elf/ico-usa-scus-97113-mips-prologue-scan.md) | Confirmed MIPS function prologue scan (3991 functions) |
| [`tools/function-ref-correlator/README.md`](./tools/function-ref-correlator/README.md) | Metadata-only function reference correlator |
| [`research/elf/ico-usa-scus-97113-function-reference-correlation.md`](./research/elf/ico-usa-scus-97113-function-reference-correlation.md) | Confirmed function reference correlation (7 functions) |
| [`tools/mips-call-graph/README.md`](./tools/mips-call-graph/README.md) | Metadata-only MIPS call graph analyzer |
| [`research/elf/ico-usa-scus-97113-call-graph-analysis.md`](./research/elf/ico-usa-scus-97113-call-graph-analysis.md) | Confirmed call graph analysis (13 callers, 15 calls) |

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
[ ] rev.011 - Environment setup for extraction and disassembly
[ ] pending - Environment setup for extraction and disassembly
[ ] pending - First visible proof of concept against a user-owned copy
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
