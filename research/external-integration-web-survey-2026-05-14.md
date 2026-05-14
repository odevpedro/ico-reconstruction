# External Integration Web Survey

## Date

2026-05-14

## Objective

Search for public internet resources that may help the ICO reconstruction project in its current phase, especially resources not already emphasized by `possible_ressources.md`.

This is a research/report note only. It does not validate any new ICO binary fact.

## Scope

Included:

- public PS2 reverse-engineering and decompilation tools;
- public ICO / Team Ico adjacent projects;
- PCSX2 debugger/runtime resources;
- R5900 / Emotion Engine documentation and toolchain references;
- assessment of whether each source can help the current ROPE callback/runtime gap.

Excluded:

- `DATA.DF` extraction;
- `.gcm` formats;
- texture or asset extraction;
- gameplay naming;
- copyrighted game data;
- broad ISO scanning.

## Sources Used

| Source | URL | Use |
|---|---|---|
| Existing local resource note | `possible_ressources.md` | Baseline list to avoid only repeating known resources |
| Project instructions | `AGENTS.md` | Scope, caution rules, current phase |
| Rev.037 | `research/elf/ghidra-rev037-remaining-callers-and-rope-gap.md` | Current static-analysis endpoint |
| Rev.025 | `research/elf/ghidra-rev025-runtime-confirmed-caller-context.md` | Runtime-confirmed callback context |
| ICO-decomp | https://github.com/rossydoubleunderscore/ico-decomp | Directly related public ICO matching project |
| SOTC decompilation project | https://github.com/Fantaskink/SOTC | Team Ico adjacent PS2 matching workflow |
| PCSX2 debugger docs | https://pcsx2.net/docs/advanced/debugger/ | Runtime debugger, symbol loading, expressions |
| PCSX2 patch docs | https://pcsx2.net/docs/advanced/writing-patches/ | PNACH patch format and patch safety |
| Ghidra Emotion Engine Reloaded | https://github.com/chaoticgd/ghidra-emotionengine-reloaded | Ghidra PS2 support |
| CCC | https://github.com/chaoticgd/ccc | PS2 debug-symbol parser |
| decomp.me PS2 platform | https://decomp.me/platform/ps2 | Public PS2 scratch ecosystem |
| m2c | https://github.com/matt-kempster/m2c | MIPS-to-C matching-oriented decompiler |
| spimdisasm | https://github.com/Decompollaborate/spimdisasm | Matching MIPS disassembly/splitting |
| rabbitizer | https://github.com/Decompollaborate/rabbitizer | MIPS instruction decoder |
| objdiff | https://github.com/encounter/objdiff | Local object diffing for decompilation |
| decomp-permuter | https://github.com/simonlindholm/decomp-permuter | Late-stage matching aid |
| ps2toolchain | https://github.com/ps2dev/ps2toolchain | Modern PS2 homebrew toolchain reference |
| PS2Tek EE docs | https://israpps.github.io/ps2tek/PS2/EE/ | R5900 / EE reference pages |
| PS2 Linux docs index | https://darrenrainey.github.io/PS2-Linux-Website/docs.html | EE/VU/GS manual index |
| Binutils R5900 patch thread | https://sourceware.org/pipermail/binutils/2012-November/079351.html | R5900 toolchain caveats |

## Evidence Used

This note uses only public web metadata and documentation text. It does not use:

- extracted game bytes;
- private repository contents;
- undocumented Discord claims;
- unverifiable social posts as technical evidence.

## Executive Summary

The strongest external help not yet emphasized in the local resource note is not another ICO-specific discovery. It is tooling/process integration:

1. **PCSX2 `.sym` files and debugger analysis settings** can make the next runtime session much less manual.
2. **spimdisasm + rabbitizer** can provide an independent matching-disassembly pipeline to cross-check Ghidra output for the dispatcher, callback registration chain, and unresolved runtime paths.
3. **objdiff / decomp.me / decomp-permuter** are useful if the project moves from annotated analysis to matching small functions.
4. **SOTC decomp** is likely the best process reference for a Team Ico adjacent PS2 project, even though it targets a Shadow of the Colossus preview build, not ICO.
5. **R5900 toolchain references** are useful for avoiding wrong assumptions about vanilla MIPS, especially around R5900-specific instructions, FPU behavior, and binutils flags.

The direct ICO-decomp repository remains the most relevant public project, but public metadata alone does not show whether it already names or matches the functions around `0x001d37c8`, `0x001d3a30`, or `0x0013f7a8`.

## Findings

### 1. PCSX2 symbol files are a practical next-step multiplier

| Item | Finding |
|---|---|
| Source | PCSX2 debugger documentation |
| Confirmed public fact | PCSX2 debugger can load external text `.sym` files. Syntax supports `address name[:size]`; if size is omitted or `1`, the entry is treated as a label. |
| Current project fit | High |
| Why this matters | The project already has stable neutral names for `candidate_state_dispatcher`, `candidate_state_jump_table`, `state_0_block` through `state_4_block`, `callback_registration_0013f7a8`, and ROPE-adjacent functions. Loading these into PCSX2 would reduce runtime transcription errors. |
| Not currently emphasized | `possible_ressources.md` mentions PCSX2/runtime generally, but not a concrete `.sym` workflow. |

Recommended use:

- Generate a local-only symbol file, not committed if it contains uncertain names.
- Use neutral names only.
- Include at minimum:

| Address | Suggested `.sym` name |
|---:|---|
| `0x001d37c8` | `candidate_state_dispatcher` |
| `0x00618fb0` | `candidate_state_jump_table` |
| `0x001d3818` | `state_0_block` |
| `0x001d3844` | `state_1_block` |
| `0x001d391c` | `state_2_block` |
| `0x001d39e0` | `state_3_block` |
| `0x001d3a10` | `state_4_block` |
| `0x001d3a30` | `rope_record_callback_001d3a30` |
| `0x0013f7a8` | `callback_registration_0013f7a8` |
| `0x00201e70` | `registration_runtime_candidate_00201e70` |

Conservative verdict:

This is the lowest-risk immediate improvement for the next runtime session. It does not require changing project conclusions or touching game data.

### 2. PCSX2 debugger expressions can encode the Rev.037 runtime test

| Item | Finding |
|---|---|
| Source | PCSX2 debugger documentation |
| Confirmed public fact | PCSX2 debugger fields accept expressions involving registers, memory dereferences, comparisons, and symbols. |
| Current project fit | High |
| Why this matters | Rev.037's next runtime test is specifically conditional: break at `0x0013f7a8`, capture `a1` when `a3 == 0x13`. |

Possible workflow:

| Runtime question | PCSX2-oriented setup |
|---|---|
| Which callback is being registered? | Break at `0x0013f7a8`; inspect `a1`, `a3`, `a0`, return address. |
| Only care about callback type `0x13`? | Use a condition expression equivalent to `a3 == 13h` if accepted by the breakpoint dialog. |
| Which state IDs appear? | Break at `0x001d37c8` or around table load; inspect `[ [a0+0x15c] + 0x800 ]` and `[state_block+0x48]`. |
| Which callsite reached registration? | Inspect `ra` at `0x0013f7a8` and map it back to the five Rev.037 callsites. |

Confidence:

High for the debugger capability; medium for exact UI syntax until tested in the installed PCSX2 build.

Next minimum test:

Create a local runtime checklist that maps each Rev.037 open question to one PCSX2 breakpoint plus the registers/memory fields to capture.

### 3. spimdisasm + rabbitizer can independently check Ghidra instruction output

| Tool | Public capability | Project relevance |
|---|---|---|
| `rabbitizer` | MIPS instruction decoder API; simple per-word instruction decoding; matching assembly goal | Useful for byte-level spot checks of disputed instructions like the earlier `sll $3,$3,2` correction. |
| `spimdisasm` | Matching MIPS disassembler; section/data splitting; function detection; hi/lo pairing; symbol naming | Useful as an independent disassembly pipeline for the dispatcher and callback chain. |

Why this matters:

The project rules explicitly say Ghidra is useful but not authoritative. The project has already hit one Ghidra/human transcription correction around `0x001d3800`. A separate R5900-aware disassembly path gives a reproducible second opinion.

Recommended narrow application:

| Target | Test |
|---|---|
| `0x001d37c8..0x001d3a2c` | Re-disassemble dispatcher and internal state blocks with a non-Ghidra tool. |
| `0x001d3a30` | Re-disassemble runtime-confirmed caller and compare direct calls / offset usage. |
| `0x0013f7a8` and five callsites | Verify callsite instructions, delay slots, and argument setup. |

Conservative verdict:

High value for auditability. This does not prove semantics, but it improves the evidence chain from "Ghidra said" to "raw bytes decoded consistently by two paths."

### 4. objdiff is useful only if the project starts compiling candidate functions

| Item | Finding |
|---|---|
| Source | `encounter/objdiff` |
| Confirmed public fact | objdiff compares object files, supports MIPS including PS2, can highlight register/value differences, and has progress-reporting integration. |
| Current project fit | Medium |
| Why not immediate | The current project is still documenting verified behavior and runtime paths, not yet maintaining recompilable translation units. |

Recommended trigger for adoption:

Use objdiff only after at least one function has:

- an extracted assembly target;
- a candidate C implementation;
- a known compiler profile;
- a repeatable local build command.

Best candidate functions later:

| Address | Why |
|---:|---|
| `0x001d37c8` | Small, bounded dispatcher with known jump table and state blocks. |
| `0x001d3a30` | Runtime-confirmed callback wrapper; structurally meaningful. |
| `0x0013f7a8` | Central registration function, but likely more connected/risky. |

Conservative verdict:

Do not make objdiff a current blocker. Keep it as a Phase 2/3 matching tool.

### 5. decomp-permuter is late-stage only

| Item | Finding |
|---|---|
| Source | `simonlindholm/decomp-permuter` |
| Confirmed public fact | The tool permutes C code to better match target assembly and supports MIPS workflows, especially when remaining differences are register allocation or small codegen issues. |
| Current project fit | Low now, higher later |

Why this matters:

It is tempting to use a permuter early, but the tool's own documentation frames it as most useful when a function is already close to matching. The ICO project is not there yet for the dispatcher chain.

Conservative verdict:

Do not use this for semantic discovery. Use it only after manual analysis and compiler selection have produced near-matching C.

### 6. SOTC decomp is a strong methodology reference, not direct evidence

| Item | Finding |
|---|---|
| Source | `Fantaskink/SOTC` |
| Confirmed public fact | Public in-progress decompilation targeting the Shadow of the Colossus preview version main ELF `SCPS_150.97` and `KERNEL.XFF`; Linux/WSL2 workflow; includes config, source, scripts, docs, and build setup. |
| Current project fit | Medium |
| Why this matters | SOTC is Team Ico adjacent and PS2/MIPS based. Its repository structure and build scripts may be a better process template than unrelated PS2 projects. |

Limits:

- SOTC is not ICO.
- The repository targets a preview version, not the retail ICO USA `SCUS_971.13`.
- No names or structures should be copied as evidence for ICO without binary validation.

Useful comparison targets:

| Area | Use |
|---|---|
| Build setup | See how they manage compiler scripts and checksums. |
| Config files | Compare how function boundaries and sections are represented. |
| Contribution guide | Use as a model if `ico-reconstruction` later invites external matching help. |

Conservative verdict:

Good methodology source. Weak-to-no direct semantic evidence for current ROPE findings.

### 7. ICO-decomp remains the only direct public project found

| Item | Finding |
|---|---|
| Source | `RossyDoubleUnderscore/ICO-decomp` |
| Confirmed public fact | Public repository named `ICO-decomp`, targeting 1:1 matching for PS2 ICO; public page shows CC0 license, C/Python/BitBake/Shell mix, 14 commits, and folders including config/scripts/include/iso. |
| Current project fit | Very high |
| Unknown | Whether it has symbols, function labels, or matching work near `0x001d37c8`, `0x001d3a30`, or `0x0013f7a8`. |

Recommended next check:

Clone or inspect the repository locally and search for:

```txt
001d37c8
001d3a30
0013f7a8
00618fb0
ROPE
SCUS_971.13
SCES_507.60
```

Important caution:

If the target version differs, address mapping may not be direct. Treat any names or matches as candidate cross-reference only.

Conservative verdict:

Highest external collaboration value. Public page alone does not resolve the current gap.

### 8. R5900-specific documentation prevents false vanilla-MIPS assumptions

| Source | Use |
|---|---|
| PS2Tek EE pages | Quick reference for EE/R5900 components, registers, instruction decoding, COP0/COP1, timers. |
| PS2 Linux docs index | Links to EE, GS, VU, SPU2, optimization, and TX79 manuals. |
| Binutils R5900 patch thread | Documents practical R5900 quirks and toolchain options. |

Important R5900 caveats from public sources:

| Topic | Relevance |
|---|---|
| MMI instructions | Ghidra or generic MIPS tools can mislead if EE-specific opcodes are not handled. |
| VU0 macro mode / COP2 | Relevant to render/animation functions; less relevant to current callback chain unless encountered. |
| FPU differences | Avoid overinterpreting floating-point code through vanilla MIPS assumptions. |
| `-march=r5900`, `-mtune=r5900`, `-mhard-ldsd` | Relevant when testing compilation/matching. |

Conservative verdict:

Use these sources as technical guardrails, not as ICO-specific evidence.

## Priority Ranking

| Priority | Resource / action | Why |
|---:|---|---|
| 1 | PCSX2 `.sym` file for current verified labels | Immediate runtime quality improvement; low risk. |
| 2 | Conditional PCSX2 breakpoint checklist for Rev.037 | Directly targets the unresolved ROPE registration gap. |
| 3 | Inspect ICO-decomp locally for matching addresses/names | Only direct public ICO project found. |
| 4 | Run spimdisasm/rabbitizer against the dispatcher and registration chain | Independent audit trail for instruction-level findings. |
| 5 | Study SOTC decomp build/config structure | Best Team Ico adjacent workflow reference. |
| 6 | Keep objdiff ready for first compile-backed function | Useful after build/matching infrastructure exists. |
| 7 | Keep decomp-permuter for late-stage matching only | Not a discovery tool; useful only near a match. |

## What Is Confirmed

1. Public resources exist that can help the next phase without touching assets or broad ISO data.
2. PCSX2 supports external symbols and expression-based debugger fields.
3. Decompollaborate/decompals tooling provides a non-Ghidra MIPS disassembly/matching ecosystem.
4. Public PS2 decompilation projects exist, including direct ICO and Team Ico adjacent SOTC projects.
5. R5900-specific documentation and toolchain discussions are necessary to avoid generic MIPS mistakes.

## What Is Probable

1. A small local `.sym` workflow will reduce runtime note-taking errors and make the Rev.037 breakpoint work easier.
2. ICO-decomp may contain useful build/config assumptions even if it does not contain direct labels for the project's current addresses.
3. SOTC decomp can help structure future matching work, especially scripts/checksums/config conventions.
4. spimdisasm/rabbitizer will be most valuable for narrow verification of known address ranges, not broad automated naming.

## What Is Possible

1. ICO-decomp may already have a name or split for the dispatcher/callback region in a different game version.
2. decomp.me PS2 scratches may attract feedback on specific small functions if the project publishes clean target assembly and neutral context.
3. objdiff could become useful for project progress tracking if `ico-reconstruction` shifts from research notes to matching translation units.

## What Is Unknown

1. Whether ICO-decomp's target build maps cleanly to `SCUS_971.13`.
2. Whether the installed PCSX2 build accepts the exact intended breakpoint condition syntax without adjustment.
3. Whether decomp.me has an ICO-specific compiler preset or whether a custom GCC/Sony profile is needed.
4. Whether `spimdisasm` currently handles every EE/R5900 instruction pattern present in the target functions without local configuration.

## What Is Discarded

1. General ICO game databases and cover/serial pages are not useful for the current dispatcher/callback work, beyond confirming identifiers already known.
2. Asset extraction resources are intentionally out of scope for the current phase.
3. LLM decompilation benchmark papers are not actionable for this project right now; they do not provide ICO-specific evidence or a reproducible PS2 workflow.
4. Reddit/social claims about native ports or decompilation progress are too weak to use as technical evidence unless tied to a repository or reproducible artifact.

## Recommended Next Minimum Tests

### Test A — Local PCSX2 Symbol File

Create a local-only symbol file with neutral verified labels and load it in PCSX2.

Expected benefit:

- less address transcription;
- easier screenshots/logging;
- clearer breakpoint sessions.

Success criterion:

- PCSX2 debugger displays neutral labels for `0x001d37c8`, `0x001d3a30`, `0x0013f7a8`, and the five state blocks.

### Test B — Rev.037 Runtime Breakpoint Checklist

Use PCSX2 to capture:

| Breakpoint | Condition | Capture |
|---|---|---|
| `0x0013f7a8` | `a3 == 0x13` if supported | `a0`, `a1`, `a2`, `a3`, `ra` |
| `0x001d37c8` | none initially | context pointer, state block pointer, state ID |
| `0x001d380c` / `0x001d3810` | optional | selected table entry / handler target |

Success criterion:

- A runtime note can map at least one actual registration hit back to one of the five Rev.037 callsites.

### Test C — ICO-decomp Address Cross-Reference

Inspect the public ICO-decomp repository locally and search for current project addresses and neutral concepts.

Success criterion:

- Either find a useful cross-reference, or document that the current public state does not expose matching labels for these addresses.

### Test D — Independent Disassembly Spot Check

Run a narrow disassembly pass with `spimdisasm` or direct `rabbitizer` decoding for:

```txt
0x001d37c8..0x001d3a2c
0x001d3a30..0x001d3b10
0x0013f7a8 callsites from Rev.037
```

Success criterion:

- Produce a small comparison table: raw word, Ghidra instruction, independent decoded instruction, match/mismatch.

## Conservative Verdict

The internet does contain useful help for the project, but the most useful help is infrastructure rather than a ready-made answer to the ROPE callback gap.

The immediate best move is to convert the project's verified labels into a local PCSX2 `.sym` workflow and run the Rev.037 runtime breakpoint test with better symbolization. In parallel, inspect ICO-decomp for address/name overlap and add an independent `spimdisasm` or `rabbitizer` verification pass for the dispatcher and callback-registration chain.

No external source found during this survey justifies renaming the state blocks, assigning gameplay semantics, or changing the current confirmed model.
