# Tooling Plan

This document defines the intended direction for local research utilities.

Current implemented tool:

- `tools/verify-local-copy/` - metadata-only verifier for local user-owned inputs
- `tools/iso-index/` - metadata-only ISO9660/BIN/CUE indexer
- `tools/elf-index/` - metadata-only ELF32 indexer
- `tools/data-df-index/` - metadata-only `DATA.DF` structural triage tool
- `tools/dvp-index/` - metadata-only `.DVP.*` overlay metadata indexer
- `tools/exe-ref-index/` - metadata-only exact executable reference scanner
- `tools/mips-immediate-scanner/` - metadata-only MIPS split-immediate pattern scanner
- `tools/elf-symbol-scan/` - metadata-only ELF symbol table scanner
- `tools/mips-prologue-scan/` - metadata-only MIPS function prologue scanner
- `tools/function-ref-correlator/` - metadata-only function reference correlator
- `tools/mips-call-graph/` - metadata-only MIPS call graph analyzer
- `tools/elf-extractor/` - local-only ELF extractor for disassembler import
- `tools/ghidra-import/` - local Ghidra guide/script generator
- `tools/elf-replacer/` - local-only ELF replacement experiment tool for modified BIN testing

## Initial Goals

The first tools should help contributors inspect a local user-owned copy without committing proprietary output.

Planned areas:

- verify local input and record checksums - started with `tools/verify-local-copy/`
- inspect ISO9660/BIN/CUE disc layout - started with `tools/iso-index/`
- identify the main executable - started with `tools/iso-index/`
- inspect ELF header, program headers, section headers, and symbol-table presence - started with `tools/elf-index/`
- inspect `DATA.DF` aggregate structure without extraction - started with `tools/data-df-index/`
- scan targeted `DATA.DF` windows around candidate offsets - started with `tools/data-df-index --target-offset`
- correlate `.DVP.*` overlay metadata with ELF load ranges and `DATA.DF` size context - started with `tools/dvp-index/`
- scan exact strings and constants in the executable - started with `tools/exe-ref-index/`
  - scan MIPS split-immediate patterns for known virtual addresses - started with `tools/mips-immediate-scanner/`
- collect file names, offsets, sizes, and basic metadata
- generate local reports ignored by git
- support future synthetic parser fixtures
- keep binary mutation experiments local under user-owned modified BIN paths, currently `/home/peter/Imagens/Ico (USA)/`

## Non-Goals

Initial tools should not:

- ship game data
- extract assets into versioned folders
- redistribute patched binaries
- imply that a source port or reconstructed runtime already exists

## Suggested Structure

Future tooling can be organized like this:

```text
tools/
  verify-local-copy/
  iso-index/
  elf-index/
  data-df-index/
  dvp-index/
  exe-ref-index/
  mips-immediate-scanner/
  mips-prologue-scan/
  function-ref-correlator/
  mips-call-graph/
  elf-extractor/
  ghidra-import/
  elf-replacer/
  elf-notes/
tests/
  fixtures/
```

Each tool should include its own README with required inputs, expected local outputs, legal limits, and reproducible examples.

## Testing Direction

Tests should use synthetic fixtures or metadata-only samples. Any test depending on a real local copy should be optional and must not require committing copyrighted files.

## Environment Setup (Next Phase)

After completing the metadata-only tooling pipeline, the next phase involves setting up a proper disassembly and emulation environment.

### Disassembler

Recommended: **Ghidra** (free, open source, excellent MIPS support)

Alternative: **radare2** (lightweight, scriptable)

### Emulator

Recommended: **PCSX2** (for runtime validation)

### Workflow

1. Import ELF into disassembler using extracted executable
2. Map known function addresses from our analysis
3. Analyze key functions (start with 0x00132630)
4. Validate findings with emulator breakpoints

### Known Targets for Analysis

| Function | Purpose |
|----------|---------|
| `0x00132630` | Most-called, likely file I/O utility |
| `0x001321c8` | References DFDATAS strings |
| `0x0019fb34` | Data loading function |

All analysis remains metadata-only - disassembly notes are documentation, not extracted code.
