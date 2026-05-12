# Tooling Plan

This document defines the intended direction for local research utilities.

Current implemented tool:

- `tools/verify-local-copy/` - metadata-only verifier for local user-owned inputs
- `tools/iso-index/` - metadata-only ISO9660/BIN/CUE indexer
- `tools/elf-index/` - metadata-only ELF32 indexer
- `tools/data-df-index/` - metadata-only `DATA.DF` structural triage tool
- `tools/dvp-index/` - metadata-only `.DVP.*` overlay metadata indexer

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
- collect file names, offsets, sizes, and basic metadata
- generate local reports ignored by git
- support future synthetic parser fixtures

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
  elf-notes/
tests/
  fixtures/
```

Each tool should include its own README with required inputs, expected local outputs, legal limits, and reproducible examples.

## Testing Direction

Tests should use synthetic fixtures or metadata-only samples. Any test depending on a real local copy should be optional and must not require committing copyrighted files.
