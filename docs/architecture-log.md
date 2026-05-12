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
