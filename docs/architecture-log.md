# Architecture Log — ICO Reconstruction

> Historical record of implemented features and internal flows.
> Never delete previous entries. Always add new ones at the end.
> Required header format per entry:
> Squad responsible | Revision | Status

---

# Feature: Project Retarget to ICO Reconstruction

> Squad responsible: SQUAD-ARCH
> Revision: rev.002
> Session: 2026-05-12
> Status: Stable

## Summary
The project was reoriented from a generic OpenGOAL-inspired reconstruction to a specific effort targeting **ICO** for PlayStation 2. OpenGOAL remains as a methodological reference for reconstruction, incremental validation, tooling, and modern runtime, but is no longer treated as a transferable architecture.

## Main Flow

### 1. Entry Point
The PO defined ICO as the main technical target because it is an early-generation PS2 title with a more contained scope than many late PS2 AAA games.

### 2. Input Validation
Documentation was revised to remove the previous target and introduce ICO-specific premises: ISO/ELF, assets, rooms, camera, actor system, collision, animation, Yorda companion AI, events, and rendering.

### 3. Application Orchestration
The backlog maintains analysis A-D as entry point, followed by E-G, proof of concept, and squad detailing.

### 4. Business Rules
All future analysis must distinguish observable technical evidence from inference. Decisions not validated against ICO ISO or binary must be marked as "no validated reference".

### 5. Persistence / Integrations
Updated `backlog.md`, `decisoes-iniciais.md`, `prompt-A-D.md`, `prompt-E-G.md`, `fases-2-4.md`, and this log.

### 6. Final Response
The official project state is now **ICO Reconstruction**, with OpenGOAL maintained only as methodological comparison.

## Alternative Flows and Errors
If ICO ISO triage reveals prohibitive coupling, absence of intervention points, or formats too opaque, the flow recommends executing a minimal PoC before detailing all squads.

## Key Technical Decisions
- Treat ICO as the primary and sole target of initial analysis.
- Prioritize empirical evidence from binary, assets, and ISO layout.
- Separate port/reconstruction from modding and emulation.
- Use OpenGOAL as process reference, not as reusable technical base.

# Feature: Initial Public README

> Squad responsible: SQUAD-ARCH
> Revision: rev.003
> Session: 2026-05-12
> Status: Stable

## Summary
An initial README was created to publish the project on GitHub and attract community collaboration without confusing the technical scope with game distribution, assets, or binaries.

## Main Flow

### 1. Entry Point
The PO requested a public project presentation to find help from the community.

### 2. Input Validation
The README was structured to explain the reconstruction goal, initial project state, legal boundaries, and areas where collaborators can help.

### 3. Application Orchestration
The `README.md` file becomes the public entry point. Operational files continue to handle backlog, decisions, prompts, and flow records.

### 4. Business Rules
The README reinforces that the repository must not contain ISO, binaries, assets, proprietary code, or extracted game data.

### 5. Persistence / Integrations
Updated `README.md`, `backlog.md`, `prompt-A-D.md`, `prompt-E-G.md`, and `system-feature-flows.md`.

### 6. Final Response
The project now has an appropriate initial public description for GitHub, focusing on technical collaboration and documented research.

## Alternative Flows and Errors
If future tools are added, the README must be revised to explain requirements, usage, and input limits without including proprietary data.

## Key Technical Decisions
- The README must explicitly declare that OpenGOAL is methodological inspiration, not technical base.
- Collaboration must prioritize research, documentation, tooling, and technical triage.
- Any future work must require user-supplied local copy when depending on game data.

# Feature: README Merge with Public Template

> Squad responsible: SQUAD-ARCH
> Revision: rev.004
> Session: 2026-05-12
> Status: Stable

## Summary
The public README was merged with the template structure from `/home/peter/Documentos/repos/claude-config/README-template.md`, preserving written content and adapting the format for a research/documentation project.

## Main Flow

### 1. Entry Point
The PO requested that the `claude-config` directory content be considered and merged into the current README, ignoring the database portion unrelated to ICO Reconstruction.

### 2. Input Validation
Consulted `README-template.md`, `CLAUDE.md`, `backlog-template.md`, and `system-feature-flows-template.md`. The data model template was ignored as it does not apply to the project.

### 3. Application Orchestration
The README received adapted template sections: badges, stack and architecture, folder structure, local setup, tests, documentation, status, contribution, and license.

### 4. Business Rules
No existing section was removed. Database, backend API, ORM, authentication, and endpoints content was excluded as it does not represent the project.

### 5. Persistence / Integrations
Updated `README.md`, `backlog.md`, `prompt-A-D.md`, `prompt-E-G.md`, and `system-feature-flows.md`.

### 6. Final Response
The README now follows a more complete public structure without inventing a non-existent application, database, or runtime.

## Alternative Flows and Errors
If the project gains executable tools, the setup and tests section must be revised with real commands.

## Key Technical Decisions
- Preserve all previous README content entirely.
- Adapt the template for documentation and research, not for API/backend.
- Explicitly declare there is no database, ORM, authentication, or web service in the current state.

# Feature: ICO Wallpaper in README

> Squad responsible: SQUAD-ARCH
> Revision: rev.005
> Session: 2026-05-12
> Status: Stable

## Summary
An ICO wallpaper was added to the top of the README to improve the public presentation of the repository on GitHub.

## Main Flow

### 1. Entry Point
The PO informed the wallpaper was at `/home/peter/Músicas/` as a WebP file.

### 2. Input Validation
The file `/home/peter/Músicas/ico.webp` was identified as a 1080x607 WebP image.

### 3. Application Orchestration
The image was copied to `assets/ico-wallpaper.webp` and referenced at the top of `README.md` with a relative path.

### 4. Business Rules
The README continues declaring that the repository must not contain ISO, binaries, extracted game assets, or proprietary data. The image is treated as a public presentation asset provided by the PO.

### 5. Persistence / Integrations
Updated `README.md`, `backlog.md`, `prompt-A-D.md`, `prompt-E-G.md`, and `system-feature-flows.md`; created file `assets/ico-wallpaper.webp`.

### 6. Final Response
The README now displays the ICO wallpaper at the top when rendered on GitHub.

## Alternative Flows and Errors
If the image file needs to be replaced with original artwork or material with explicit license, simply swap `assets/ico-wallpaper.webp` while keeping the same path in the README.

## Key Technical Decisions
- Store the image inside the repository to avoid local path dependency.
- Use relative path `./assets/ico-wallpaper.webp` for GitHub compatibility.

# Feature: Minimal Folder Structure for GitHub

> Squad responsible: SQUAD-ARCH
> Revision: rev.006
> Session: 2026-05-12
> Status: Stable

## Summary
A minimal directory structure was created to publish the project on GitHub without suggesting a functional implementation already exists.

## Main Flow

### 1. Entry Point
The PO requested a minimal folder structure to upload the project to GitHub.

### 2. Input Validation
The current structure contained only planning Markdown files and the `assets/` folder with the wallpaper.

### 3. Application Orchestration
Empty versionable directories were created with `.gitkeep`: `docs/`, `research/`, `tools/`, `tests/`, `tests/fixtures/`, and `.github/ISSUE_TEMPLATE/`.

### 4. Business Rules
The structure does not include code, binaries, extracted game assets, ISO, proprietary fixtures, or speculative tools.

### 5. Persistence / Integrations
Updated `README.md`, `backlog.md`, `prompt-A-D.md`, `prompt-E-G.md`, and `system-feature-flows.md`.

### 6. Final Response
The repository now has a minimal base for public organization: documentation, research, future tooling, future tests, and issue templates.

## Alternative Flows and Errors
If real tools are added, the `tools/` folder must receive a specific README with commands and legal limits.

## Key Technical Decisions
- Use `.gitkeep` to version still-empty directories.
- Keep the structure small to avoid promising non-existent modules.
- Reserve `tests/fixtures/` only for synthetic or non-copyrighted fixtures.

# Feature: Documentation Cleanup and Base Research Guides

> Squad responsible: SQUAD-ARCH
> Revision: rev.006.1
> Session: 2026-05-12
> Status: Stable

## Summary
The public documentation was aligned with the actual repository state. The obsolete future `docs/data-model.md` reference was removed, and base guides were added for research methodology, legal boundaries, tooling direction, research notes, and future tools.

## Main Flow

### 1. Entry Point
The PO requested cleanup after reviewing the next recommended steps for the project.

### 2. Input Validation
The repository was confirmed to be documentation-first, with no executable application, runtime, database, API, source port, or implemented tooling.

### 3. Application Orchestration
The README now points to existing project documents and newly added base guides. The backlog records the cleanup as an intermediate revision without consuming `rev.007`, which remains reserved for architectural analysis A-D.

### 4. Business Rules
Documentation continues to prohibit committing ISO contents, binaries, extracted assets, proprietary source, or copyrighted game data. Future tooling must operate on a local user-supplied copy.

### 5. Persistence / Integrations
Updated `README.md`, `docs/backlog.md`, and this log. Added `docs/research-methodology.md`, `docs/legal-boundaries.md`, `docs/tooling-plan.md`, `research/README.md`, and `tools/README.md`.

### 6. Final Response
The repository now has a cleaner public structure and enough base documentation to support the next architecture pass.

## Alternative Flows and Errors
If the project later introduces real executable tools, the README and `docs/tooling-plan.md` must be revised with actual setup commands, generated output paths, and test instructions.

## Key Technical Decisions
- Keep `rev.007` assigned to architectural analysis A-D.
- Replace the unused data model reference with research, legal, and tooling guides.
- Keep the repository documentation-first until empirical tooling exists.

# Feature: Architectural Analysis A-D

> Squad responsible: SQUAD-ARCH
> Revision: rev.007
> Session: 2026-05-12
> Status: Stable

## Summary
The first subsystem feasibility and validation map was documented for ICO Reconstruction. The analysis distinguishes confirmed repository state from hypotheses and unknowns that require local empirical validation against a user-owned copy.

## Main Flow

### 1. Entry Point
The project advanced from documentation cleanup into the pending `rev.007` architecture task.

### 2. Input Validation
No ISO, ELF, executable, asset, or extracted game data is present in the repository. The analysis therefore uses documentation-only evidence and marks ICO-specific technical claims as hypotheses or unknowns.

### 3. Application Orchestration
Created `docs/architectural-analysis-a-d.md` with a subsystem feasibility table, validation work that can happen without a binary, validation work that requires empirical testing, and a recommended first technical path.

### 4. Business Rules
The recommended path starts with metadata-only local verification and ISO indexing. It explicitly avoids committing proprietary data or beginning gameplay reconstruction before empirical metadata exists.

### 5. Persistence / Integrations
Updated `README.md`, `docs/backlog.md`, and this log. Added `docs/architectural-analysis-a-d.md`.

### 6. Final Response
`rev.007` is complete. The next practical project step is environment setup or metadata-only local verifier work before deeper subsystem claims.

## Alternative Flows and Errors
If empirical ISO/ELF inspection contradicts the assumptions in the A-D analysis, the report should be revised with confirmed observations and the changed evidence level.

## Key Technical Decisions
- Treat reconstruction/source-port feasibility as unproven until local ISO and executable metadata are inspected.
- Prioritize metadata-only tooling before asset extraction or mutation PoCs.
- Keep `rev.008` constrained by evidence from `rev.007` and future local tooling.

# Feature: Local Metadata Tooling Foundation

> Squad responsible: SQUAD-TOOLING
> Revision: rev.007.1
> Session: 2026-05-12
> Status: Stable

## Summary
The repository received the first practical tooling foundation: safety-focused `.gitignore` rules, GitHub issue templates, and a metadata-only verifier for local user-owned inputs.

## Main Flow

### 1. Entry Point
The PO requested the next practical steps to be done at once: ignored local outputs, issue templates, and a first metadata-only verifier.

### 2. Input Validation
No real ICO data was used. A synthetic fixture was added under `tests/fixtures/` to validate the verifier without proprietary content.

### 3. Application Orchestration
The verifier accepts a local file or directory, records file metadata and optional SHA-256 hashes, and writes a JSON report under `.local/reports/`, which is ignored by git.

### 4. Business Rules
The tool does not copy, extract, patch, decode, or redistribute game data. Reports are local by default and must be reviewed before any metadata from a real copy is committed.

### 5. Persistence / Integrations
Added `.gitignore`, issue templates under `.github/ISSUE_TEMPLATE/`, `tools/verify-local-copy/`, and `tests/fixtures/metadata-sample.txt`. Updated README, tooling docs, backlog, and this log.

### 6. Final Response
The project now has a first safe local verification tool and collaboration templates. Full runtime/disassembly environment setup remains pending.

## Alternative Flows and Errors
If a local directory contains too many files, the verifier refuses to scan beyond `--max-files` unless explicitly raised. Users may pass `--no-hash` to avoid expensive hashing.

## Key Technical Decisions
- Keep generated reports under `.local/reports/` and ignored by git.
- Use Python standard library only for the first tool.
- Treat the verifier as metadata-only groundwork, not ISO parsing or extraction.

# Feature: Local BIN/CUE Disc Index and Initial Observation

> Squad responsible: SQUAD-TOOLING
> Revision: rev.007.2
> Session: 2026-05-12
> Status: Stable

## Summary
A metadata-only ISO9660/BIN/CUE indexer was added and used against the local user-owned ICO USA BIN/CUE image. The project now has its first confirmed disc layout observation without copying or extracting proprietary files into the repository.

## Main Flow

### 1. Entry Point
The PO provided the local path `/home/peter/Downloads/Ico (USA)/` and clarified that the game is stored as CD-ROM BIN/CUE rather than a DVD ISO.

### 2. Input Validation
The local directory contains `Ico (USA).bin` and `Ico (USA).cue`. The CUE declares `TRACK 01 MODE2/2352`. The files remained outside the repository.

### 3. Application Orchestration
`tools/verify-local-copy/` generated a local metadata report with hashes. `tools/iso-index/` detected the raw 2352-byte sector layout with data offset 24, generated a local metadata-only ISO9660 index, and parsed basic ELF header metadata for the executable candidate.

### 4. Business Rules
No game content was copied, extracted, decoded, patched, or committed. Only safe metadata, file names, sizes, LBAs, hashes, and interpretation notes were recorded.

### 5. Persistence / Integrations
Added `tools/iso-index/` and `research/iso-layout/ico-usa-bin-cue-initial-index.md`. Updated README, tooling docs, backlog, and this log.

### 6. Final Response
The project now has confirmed local disc metadata: main executable candidate `SCUS_971.13`, ELF32 header metadata, primary data/archive candidate `DFDATAS/DATA.DF`, and BIN/CUE sector layout details.

## Alternative Flows and Errors
If another regional dump uses a different sector layout, `iso-index` must detect the Primary Volume Descriptor using one of its supported layouts or be extended with another layout candidate.

## Key Technical Decisions
- Treat ICO USA as a CD-ROM BIN/CUE workflow for current local analysis.
- Keep generated index JSON in `.local/reports/` and commit only reviewed metadata summaries.
- Use disc metadata to drive the next step: metadata-only ELF section/program-header inspection and `DATA.DF` structure triage.

# Feature: Local ELF Metadata Index and Initial Observation

> Squad responsible: SQUAD-TOOLING
> Revision: rev.007.3
> Session: 2026-05-12
> Status: Stable

## Summary
A metadata-only ELF32 indexer was added and used against the embedded `SCUS_971.13` executable from the local ICO USA BIN/CUE image. The project now has confirmed executable layout metadata without extracting or committing the executable.

## Main Flow

### 1. Entry Point
The previous disc index identified `SCUS_971.13` at LBA 25 with size 5,481,608 bytes.

### 2. Input Validation
The executable was read from the local BIN image using sector size 2352 and data offset 24. No executable copy was written into the repository.

### 3. Application Orchestration
`tools/elf-index/` parsed the ELF header, program headers, section headers, section names, and symbol-table presence from memory, then wrote a local JSON report under `.local/reports/`.

### 4. Business Rules
The report is metadata-only. It does not include executable bytes, disassembly, function bodies, copied proprietary source, or extracted game content.

### 5. Persistence / Integrations
Added `tools/elf-index/` and `research/elf/ico-usa-scus-97113-elf-metadata.md`. Updated README, tooling docs, backlog, and this log.

### 6. Final Response
Confirmed ELF32 little-endian metadata: entry point `0x00100008`, one `PT_LOAD` segment, 27 section headers, no symbol table, vector-unit-related sections, and `.DVP.overlay...` sections that need follow-up.

## Alternative Flows and Errors
If another regional executable has different ELF layout, run `elf-index` with the LBA and size reported by `iso-index` for that dump and record a separate regional observation.

## Key Technical Decisions
- Keep ELF reports metadata-only and local by default.
- Do not list individual symbol names; only record symbol table presence/counts.
- Use `.DVP.overlay...` section metadata to guide the next investigation into overlays and `DFDATAS/DATA.DF`.

# Feature: Local DATA.DF Metadata Triage

> Squad responsible: SQUAD-TOOLING
> Revision: rev.007.4
> Session: 2026-05-12
> Status: Stable

## Summary
A metadata-only `DATA.DF` triage tool was added and used against the embedded `DFDATAS/DATA.DF` archive candidate from the local ICO USA BIN/CUE image.

## Main Flow

### 1. Entry Point
The disc index identified `DFDATAS/DATA.DF` at LBA 2,898 with size 539,367,424 bytes.

### 2. Input Validation
The archive candidate was read from the local BIN image using sector size 2352 and data offset 24. No archive copy or internal archive contents were written into the repository.

### 3. Application Orchestration
`tools/data-df-index/` sampled head, middle, and tail windows, calculated byte-profile metadata, and searched the head window for simple monotonic 32-bit offset tables and fixed-record table candidates.

### 4. Business Rules
The report is metadata-only. It does not include raw archive bytes, extracted entries, asset payloads, file names from inside the archive, or decoded game content.

### 5. Persistence / Integrations
Added `tools/data-df-index/` and `research/data-df/ico-usa-data-df-initial-triage.md`. Updated README, tooling docs, backlog, and this log.

### 6. Final Response
The initial triage found high entropy at the start, lower entropy in the middle, mostly zero/padding at the tail, and no simple head-window offset table candidate under the current heuristic.

## Alternative Flows and Errors
If later executable or overlay analysis identifies a table offset outside the first megabyte, `data-df-index` should be extended to scan targeted offsets instead of relying on generic head-window heuristics.

## Key Technical Decisions
- Keep archive analysis metadata-only until the format is understood.
- Avoid extracting or naming internal archive entries until a clean safe representation is defined.
- Use `.DVP.*` ELF metadata and executable references to guide the next targeted `DATA.DF` pass.

# Feature: Local DVP Overlay Metadata Correlation

> Squad responsible: SQUAD-TOOLING
> Revision: rev.007.5
> Session: 2026-05-12
> Status: Stable

## Summary
A metadata-only `.DVP.*` overlay indexer was added and used against the embedded `SCUS_971.13` executable. The tool correlates DVP section metadata with the ELF load range and known `DATA.DF` size context.

## Main Flow

### 1. Entry Point
The previous ELF analysis identified `.DVP.ovlytab`, `.DVP.ovlystrtab`, and 12 `.DVP.overlay...` sections.

### 2. Input Validation
The executable was read from the local BIN image using sector size 2352 and data offset 24. `DATA.DF` was used only as size context. No executable bytes, overlay payloads, or archive contents were committed.

### 3. Application Orchestration
`tools/dvp-index/` parsed `.DVP.*` section metadata, counted overlay string-table entries, interpreted `.DVP.ovlytab` as 12-byte entries, and classified numeric values against the ELF load range and known `DATA.DF` size.

### 4. Business Rules
The report remains metadata-only. It does not include raw executable bytes, disassembly, overlay contents, extracted archive entries, or decoded assets.

### 5. Persistence / Integrations
Added `tools/dvp-index/` and `research/dvp/ico-usa-dvp-overlay-metadata.md`. Updated README, tooling docs, backlog, and this log.

### 6. Final Response
Confirmed 14 `.DVP.*` sections, 12 overlay sections, 12 overlay-table entries, and 12 overlay string-table entries. The overlay table appears to include ELF memory references, so direct `DATA.DF` offset interpretation remains unconfirmed.

## Alternative Flows and Errors
If future targeted `DATA.DF` scans validate any `.DVP.overlay...` numeric token as an archive offset, the DVP note should be revised from hypothesis to confirmed mapping for that token.

## Key Technical Decisions
- Do not treat numeric values as `DATA.DF` offsets merely because they fit inside the archive size.
- Compare DVP values against both ELF load ranges and `DATA.DF` size context.
- Use DVP numeric tokens as search seeds for targeted `DATA.DF` scans, not as confirmed table entries.

# Feature: Targeted DATA.DF Scans Around DVP Tokens

> Squad responsible: SQUAD-TOOLING
> Revision: rev.007.6
> Session: 2026-05-12
> Status: Stable

## Summary
`tools/data-df-index/` was extended with repeatable targeted offset scans and used to inspect windows around numeric tokens from `.DVP.overlay...` section names.

## Main Flow

### 1. Entry Point
The DVP overlay analysis identified numeric tokens that could be used as search seeds for `DATA.DF`.

### 2. Input Validation
The scan used local `DFDATAS/DATA.DF` metadata from the ICO USA BIN/CUE image. No archive bytes, extracted entries, or decoded game data were committed.

### 3. Application Orchestration
`data-df-index` now accepts repeatable `--target-offset` arguments and scans bounded windows around each candidate. The DVP token scan used 262,144-byte windows around six candidate offsets.

### 4. Business Rules
The report remains metadata-only and records entropy, byte-class statistics, hashes, and table-candidate counts only.

### 5. Persistence / Integrations
Updated `tools/data-df-index/`, README, tooling docs, backlog, and this log. Added `research/data-df/ico-usa-data-df-dvp-targeted-scan.md`.

### 6. Final Response
No simple local offset table or fixed-record candidates were found around the tested DVP numeric tokens. The DVP tokens remain useful search seeds, but are not confirmed `DATA.DF` offsets.

## Alternative Flows and Errors
Future scans can adjust `--target-window-bytes`, add more offsets, or scan offsets found from executable-reference analysis.

## Key Technical Decisions
- Preserve generic head/middle/tail triage while adding targeted scans.
- Treat high-entropy targeted windows as evidence against simple local table structures, not proof that the offsets are meaningless.
- Move next investigation toward executable-reference analysis instead of wider blind archive scans.

# Feature: Executable Reference Scan for DATA.DF and DVP Tokens

> Squad responsible: SQUAD-TOOLING
> Revision: rev.007.7
> Session: 2026-05-12
> Status: Stable

## Summary
A metadata-only executable reference scanner was added and used against embedded `SCUS_971.13` to search for exact strings and 32-bit constants related to `DATA.DF`, DVP metadata, and prior candidate tokens.

## Main Flow

### 1. Entry Point
Targeted `DATA.DF` scans around DVP tokens did not reveal simple local tables, so the investigation shifted to executable references.

### 2. Input Validation
The executable was read from the local BIN image using sector size 2352 and data offset 24. No executable bytes, disassembly, or arbitrary string dumps were committed.

### 3. Application Orchestration
`tools/exe-ref-index/` searches only user-supplied strings and constants, then reports match counts, file offsets, virtual addresses, and containing sections.

### 4. Business Rules
The report remains metadata-only and exact-query-only. It does not extract executable contents, recover functions, or dump arbitrary strings.

### 5. Persistence / Integrations
Added `tools/exe-ref-index/` and `research/exe-refs/ico-usa-scus-97113-data-df-dvp-references.md`. Updated README, tooling docs, backlog, and this log.

### 6. Final Response
Confirmed direct `DATA.DF` and `DFDATAS` string references in executable data sections. Tested DVP numeric tokens were not found as direct 32-bit constants.

## Alternative Flows and Errors
Constants may be formed through MIPS split-immediate instruction sequences, so absence of exact 32-bit constants is not proof that the executable does not use those values.

## Key Technical Decisions
- Search only explicit user-provided strings and constants.
- Record exact offsets and sections, not surrounding bytes or disassembly.
- Move next investigation toward MIPS immediate/reference pattern scanning.
