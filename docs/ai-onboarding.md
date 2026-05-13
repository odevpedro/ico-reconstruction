# AI Onboarding Guide

> Direction for a newly arrived AI agent working on ICO Reconstruction.
> Read this before changing files, creating tools, or proposing a new investigation path.

## First Rule

This is a documentation-first reverse engineering research project. Do not commit or generate proprietary ICO binaries, extracted assets, copyrighted data, or copied decompiled source.

Use only metadata, offsets, checksums, local-only reports, original notes, and tools that operate on the user's own local copy.

## Required Reading Order

Read these files in order:

1. `README.md`
   - Public project scope, current architecture, tool index, roadmap, and legal boundaries.
2. `docs/ai-onboarding.md`
   - This file. It defines the shortest safe path for a new AI.
3. `.local/key-concepts.md`
   - Local, ignored, high-context working memory. This currently has the freshest investigation state, including mod4/mod5/mod6 and Ghidra notes.
4. `docs/backlog.md`
   - Current work status, active runtime task, pending items, and revision signatures.
5. `docs/architecture-log.md`
   - Historical technical decisions. Append-only: never delete older entries.
6. `docs/research-methodology.md`
   - Evidence labels and documentation standard.
7. `docs/legal-boundaries.md`
   - What must never be committed.
8. `docs/tooling-plan.md`
   - Tooling direction and boundaries.
9. `tools/README.md`
   - Existing tools. Check this before creating a new tool.
10. `research/README.md`
    - Research note organization.

After this, read topic-specific research notes under `research/` and tool READMEs under `tools/` only as needed.

## Current Local Context

The current active local input is the user-owned ICO USA BIN/CUE under:

```text
/home/peter/Imagens/Ico (USA)/
```

Known important local files:

```text
.local/extracted/SCUS_971.13.elf
.local/reports/
.local/ghidra/
.local/key-concepts.md
```

The original executable metadata repeatedly used by tools:

```text
source-name: SCUS_971.13
LBA: 25
size: 5481608
sector-size: 2352
data-offset: 24
ELF load base: 0x00100000
entry point: 0x00100008
```

The large data archive:

```text
source-name: DFDATAS/DATA.DF
LBA: 2898
size: 539367424
sector-size: 2352
data-offset: 24
```

## Current Investigation State

The latest validated gameplay fact:

- When Ico dies in normal gameplay, the game shows a `Continue` menu with `Yes` and `No`.
- Treat this as the target UI. Do not assume the target is a `Game Over` screen.

Negative tests already performed:

- `mod4`: patched ASCII `%s? Yes:O No:X` to `%s? Ode:O No:X`; death menu still showed `Yes`.
- `mod5`: swapped `TEX2/yesno_p*.tm2` references to `TEX2/cont2_p*.tm2`; death menu still showed `Yes/No`.
- `mod6`: swapped `yesno`, `conti`, and `cont2` texture reference families more aggressively; death menu still showed `Yes/No`.

Current inference:

- Stop blind string/texture-name swaps for the death menu.
- The next useful step is runtime validation with PCSX2 breakpoints or logs, especially around the functions below.

Current candidate addresses:

```text
pac_continueTag VA: 0x005551f0
refs: 0x0011a570, 0x0011a57c
candidate function: 0x0011a520
caller: 0x0011a668
callsite: 0x0011a794

%s.tm2 VA: 0x00631e00
refs: 0x0012d230, 0x0012d238
candidate function: 0x0012d218
caller: 0x0012fd50
callsite: 0x0012fd58
```

## Logs and Reports

Project-generated JSON reports are in:

```text
.local/reports/
```

PCSX2 Flatpak log directory found on this machine:

```text
/home/peter/.var/app/net.pcsx2.PCSX2/config/PCSX2/logs/
```

Current PCSX2 log file found:

```text
/home/peter/.var/app/net.pcsx2.PCSX2/config/PCSX2/logs/emulog.txt
```

After the user finishes a manual PCSX2 test, inspect:

1. `.local/reports/` for project scanner outputs.
2. `/home/peter/.var/app/net.pcsx2.PCSX2/config/PCSX2/logs/emulog.txt` for emulator messages.
3. `.local/key-concepts.md` for the latest user-validated conclusions.

## Tooling Rule

Before creating a new tool:

1. Read `README.md` tool list.
2. Read `tools/README.md`.
3. Check whether one of these already covers the need:
   - `exe-ref-index`
   - `mips-immediate-scanner`
   - `function-ref-correlator`
   - `mips-call-graph`
   - `elf-index`
   - `elf-extractor`
   - `elf-replacer`
4. Extend an existing tool only when the need is clearly adjacent.
5. Create a new tool only when no existing tool fits and document why.

## How To Continue From Here

Recommended next step:

1. Ask the user to run the manual PCSX2 breakpoint/log validation if needed.
2. Prioritize breakpoint validation at `0x0011a520` and `0x0011a794`.
3. If those trigger when the `Continue` menu appears, record register/argument state.
4. Only after runtime evidence, decide whether to patch code, patch data, or switch fronts.

Secondary path:

- Test `0x0012d218` or `0x0012fd58` to understand generic `%s.tm2` texture path behavior.

Do not return to blind TM2 name swapping unless new runtime evidence says the target asset family is known.

## PCSX2 Breakpoint Reminder

Local PCSX2 installation found:

```text
Flatpak net.pcsx2.PCSX2 v2.6.3
```

Debugger path:

```text
Tools -> Show Advanced Settings
Debug -> Open Debugger
R5900 layout
Breakpoints dock/tab
```

Use execute breakpoints for main ELF addresses. The first two addresses to test are:

```text
0x0011a520
0x0011a794
```

Portuguese UI observed from local screenshots:

```text
Configuracoes do PCSX2 -> Depurar -> Interface do Usuario -> Janela do Debugger -> Mostrar na Inicializacao
```

The screenshots under `prints/` show the PCSX2 debug settings, not the debugger window itself.

If that path opens only `Janela do Registro`, use the official CLI debugger flag:

```bash
flatpak run net.pcsx2.PCSX2 -debugger -- "/home/peter/Imagens/Ico (USA)/Ico (USA)-mod6.bin"
```

## Reporting To The User

The user wants progress percentage in chat after each completed task, not inside `.local/key-concepts.md`.

Update `.local/key-concepts.md` when a relevant new result appears. Follow the existing revision style.
