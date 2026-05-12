# Tools

This directory is reserved for future local research utilities.

Current tools:

- `verify-local-copy/` - metadata-only verifier for local user-owned inputs
- `iso-index/` - metadata-only ISO9660/BIN/CUE indexer
- `elf-index/` - metadata-only ELF32 indexer

Tools should:

- operate on a local user-owned copy of ICO
- avoid writing proprietary data into the repository
- produce deterministic metadata or reports where possible
- document all required inputs and generated outputs
- include tests using synthetic fixtures when practical

See `docs/tooling-plan.md` for the current tooling direction.
