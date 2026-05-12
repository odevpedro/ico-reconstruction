# Architectural Analysis A-D

> Revision: rev.007
> Status: Completed
> Evidence baseline: documentation-only, no ICO ISO or executable inspected in this repository

## Summary

This analysis defines the first feasibility map for ICO Reconstruction. It separates what can be planned from public technical knowledge and project constraints from what must be validated empirically against a local user-owned copy of ICO.

The current conclusion is conservative: the project is feasible as a staged research and tooling effort, but not yet feasible as a reconstruction or source-port implementation. The next milestone should be a reproducible environment and a metadata-only local proof of concept.

## A. Feasibility by Subsystem

| Subsystem | Initial Feasibility | Evidence Level | Notes |
|-----------|---------------------|----------------|-------|
| Repository process | High | Confirmed | Documentation workflow, backlog, architecture log, and research rules are already present. |
| Legal/content boundary | High | Confirmed | Repository rules prohibit ISO, binaries, extracted assets, proprietary source, and copyrighted game data. |
| ISO layout inspection | High | Hypothesis | PS2 discs can be inspected locally, but ICO-specific layout is not validated here yet. |
| Main executable identification | High | Hypothesis | Expected to be discoverable from local disc metadata and ISO contents, but the executable has not been inspected. |
| ELF metadata analysis | Medium | Hypothesis | Symbol availability, section structure, overlays, and build details are unknown until empirical analysis. |
| MIPS disassembly | Medium | Hypothesis | Technically viable as research, but reconstruction complexity depends on code organization, compiler patterns, and runtime coupling. |
| File table/archive mapping | Medium | Unknown | Likely central to asset access, but no ICO-specific archive format has been confirmed in this repository. |
| Texture format exploration | Medium | Unknown | Viability depends on whether formats are standard PS2/GIF/GS-adjacent, custom packed data, or engine-specific containers. |
| Model format exploration | Medium-Low | Unknown | 3D data may involve custom formats, DMA/VIF/VU pipelines, skeletal data, and engine-specific packing. |
| Animation data | Medium-Low | Unknown | Requires model format progress plus runtime behavior mapping. |
| Collision data | Medium | Unknown | Potentially easier to reason about structurally than animation, but format and binding to rooms are unknown. |
| Room/scene data | Medium | Unknown | High-value target because ICO is environment-driven, but depends on archive/file table understanding. |
| Camera and events | Medium-Low | Unknown | Likely encoded in scripts, triggers, room data, or executable logic; validation requires runtime and data correlation. |
| Actor/object system | Low-Medium | Unknown | Requires executable mapping and runtime observation before meaningful reconstruction. |
| Yorda behavior | Low | Unknown | High semantic complexity and likely intertwined with animation, navigation, events, and gameplay state. |
| Shadow enemy behavior | Low | Unknown | Similar dependency on actors, animation, combat state, and scripted encounters. |
| Rendering pipeline | Low | Unknown | PS2 GS/VIF/VU behavior makes accurate reconstruction complex without significant runtime analysis. |
| Audio/cutscene packaging | Medium-Low | Unknown | Packaging may be identifiable early, but meaningful replacement/reconstruction requires format and playback mapping. |
| First local metadata PoC | High | Inferred | A non-distributable tool that verifies a local copy and emits hashes/file metadata is the safest first visible milestone. |

## B. Validation Without Game Binary

The following work can proceed before inspecting a local copy:

- define research note format and evidence levels
- document legal boundaries and contribution rules
- create issue templates for research observations and tooling proposals
- define local output folders that must be ignored by git
- specify the expected shape of first tools without implementing proprietary assumptions
- collect public references about PS2 ELF, MIPS, disc structure, and reverse engineering methodology
- design synthetic fixtures for parser tests
- define subsystem ownership for future squads

Useful deliverables at this stage:

- `docs/tooling-plan.md` refinements
- `docs/research-methodology.md` refinements
- `research/*/README.md` subsystem note templates
- `.gitignore` rules for local extracted output and reports
- issue templates under `.github/ISSUE_TEMPLATE/`

## C. Validation Requiring Empirical Testing

The following items require a local user-owned copy and must not result in committed proprietary data:

| Question | Required Local Evidence | Safe Repository Artifact |
|----------|--------------------------|--------------------------|
| Which disc/version is being studied? | Disc identifier, file metadata, checksums | Hashes, version notes, no copied files |
| What is the ISO file layout? | Local ISO directory table and file sizes | Metadata-only report or documented file tree |
| Which file is the main executable? | Local disc metadata and executable detection | File name, size, checksum, offsets |
| Does the executable contain useful symbols? | Local ELF inspection | Summary of symbol/section presence |
| Are assets stored as loose files or archives? | Local file layout and magic/header checks | Format notes, names, offsets, sizes |
| Is there a central file table? | Correlation between executable references and disc files | Structural description and hypotheses |
| Which formats are easiest for the first mutation PoC? | Local inspection of candidate files | Candidate list with risk notes |
| Can a local modified copy boot? | Emulator/debugger test on user machine | Reproduction steps and observed behavior |

Empirical reports should classify findings as Confirmed, Inferred, Hypothesis, or Unknown according to `docs/research-methodology.md`.

## D. Recommended First Technical Path

The first implementation path should avoid gameplay reconstruction. The safest route is metadata and observability:

1. Add `.gitignore` entries for local game inputs, extracted output, generated reports, emulator state, and temporary binary work.
2. Create a local-copy verifier that accepts a path outside the repository and emits checksums plus basic file metadata.
3. Create an ISO index report that records names, sizes, offsets, and candidate executable files without copying contents.
4. Add optional ELF metadata extraction for the detected executable, again outputting only metadata.
5. Record the first empirical observations under `research/` with evidence labels.
6. Choose a first visible PoC only after the ISO and ELF metadata are understood.

Recommended first PoC target:

```text
user-owned local ICO copy
    -> metadata-only verifier
    -> ISO index report
    -> executable candidate report
    -> documented observations
```

This creates progress without crossing the legal boundary or making unsupported claims about reconstructability.

## Risks and Black Boxes

| Risk | Impact | Current Mitigation |
|------|--------|--------------------|
| Unknown ICO-specific file formats | Blocks asset tooling and subsystem mapping | Start with metadata and structural notes before parsers |
| MIPS/VU reconstruction complexity | Blocks faithful runtime reconstruction | Delay source reconstruction until executable regions are mapped |
| Proprietary data leakage | Legal and project-health risk | Keep only metadata, hashes, scripts, and original notes in git |
| Speculative architecture | Wasted implementation effort | Require evidence labels and validation notes |
| Version differences between discs | Reproducibility issues | Track disc identifier and checksums in local reports |
| Emulator/debugger variance | Validation ambiguity | Document exact tool, version, settings, and observation method when used |

## Immediate Next Steps

1. Done in `rev.007.1`: add `.gitignore` rules for local-only binary inputs and generated reports.
2. Done in `rev.007.1`: add GitHub issue templates for research observations and tooling proposals.
3. Done in `rev.007.1`: implement the first metadata-only local verification tool.
4. Start `SQUAD-RUNTIME` environment setup planning.
5. Use empirical findings from local metadata tooling to drive `rev.008`.

## Decision

Proceed to environment setup before deeper subsystem claims.

`rev.008` should not try to finalize a squad architecture from speculation alone. It should define the decision matrix and squad model, but mark all executable, asset, and runtime assumptions as pending empirical validation until local metadata tooling exists.
