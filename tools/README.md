# Tools

This directory is reserved for future local research utilities.

Current tools:

- `verify-local-copy/` - metadata-only verifier for local user-owned inputs
- `iso-index/` - metadata-only ISO9660/BIN/CUE indexer
- `elf-index/` - metadata-only ELF32 indexer
- `data-df-index/` - metadata-only `DATA.DF` structural triage tool
- `dvp-index/` - metadata-only `.DVP.*` overlay metadata indexer
- `exe-ref-index/` - metadata-only exact executable reference scanner
- `mips-immediate-scanner/` - metadata-only MIPS split-immediate pattern scanner
- `elf-symbol-scan/` - metadata-only ELF symbol table scanner
- `mips-prologue-scan/` - metadata-only MIPS function prologue scanner
- `function-ref-correlator/` - metadata-only function reference correlator
- `mips-call-graph/` - metadata-only MIPS call graph analyzer
- `elf-extractor/` - local-only ELF extractor for disassembler import
- `ghidra-import/` - local Ghidra guide/script generator
- `elf-replacer/` - local-only ELF replacement experiment tool for modified BIN testing

Tools should:

- operate on a local user-owned copy of ICO
- avoid writing proprietary data into the repository
- produce deterministic metadata or reports where possible
- document all required inputs and generated outputs
- include tests using synthetic fixtures when practical

See `docs/tooling-plan.md` for the current tooling direction.
