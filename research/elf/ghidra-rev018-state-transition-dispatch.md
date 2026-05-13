# Ghidra State Transition Dispatch Analysis - rev.018

## Date
2026-05-13

## Environment
- Ghidra: 12.0.4_PUBLIC
- JDK: OpenJDK 21
- Target: `SCUS_971.13.elf` from the local ICO USA copy
- Method: headless analysis with a local `AnalyzeContinueFlow.java` script

## Scope
The earlier continue-menu string/breakpoint campaign was not isolating the death/continue flow. This pass pivots to state-transition code in the ELF, especially table-driven blocks that look like dispatch logic rather than text rendering.

## Observed State-Oriented Anchors

| Address | Ghidra Name | Body Size | Observation | Evidence |
|---------|-------------|-----------|-------------|----------|
| `0x0013c9d8` | `FUN_0013c9d8` | 88 addresses | Object/list-style callback splice with `lw`/`sw` around `0x30` and `0x34`, then indirect `jalr v0` | Confirmed |
| `0x001ae5f0` | `FUN_001ae5f0` | 264 addresses | Enum-like branch chain on `a0` values `1`, `2`, `4`, `f`; writes paired values to stack and checks a `0x4B3D10` table with `lhu` | Inferred |
| `0x0013d8e4` | `FUN_0013d8e4` | in same chain | Bounds check `sltiu v0,a0,0x101` followed by lookup into a `0x6A6F30` table via `a0 << 2` | Inferred |
| `0x0013eb50` | `FUN_0013eb50` | 140 addresses | Strong table-dispatch pattern using records of size `0x174`, globals at `-0x4c4c(gp)` / `-0x4c50(gp)`, and repeated record scanning | Confirmed |

## Interpretation

The most useful result from this pass is not a menu string. It is the shape of the code:

- `0x0013c9d8` looks like a generic object or callback list helper.
- `0x001ae5f0` looks like a small enum classifier that normalizes state into a compact set of outputs.
- `0x0013d8e4` looks like a guarded table lookup.
- `0x0013eb50` looks like a state-table or entity-table scan with fixed record stride `0x174`.

That combination is much closer to gameplay state management than to UI text, and it is the right place to look next if the goal is to understand the transition into `Continue / Yes / No` without brute-force breakpoints.

## Remaining Uncertainty

- The exact meaning of the numeric state values is still unknown.
- The `0x174` record table is not yet named, so the subject could still be menu state, scene state, entity state, or another game-system dispatcher.
- Ghidra has not yet given us stable xrefs or symbolic names for these blocks.

## Next Validation Step

Find the callers of `0x0013eb50` and `0x001ae5f0`, then tie them to runtime state changes around death/continue. If a runtime breakpoint is needed again, use these anchors instead of the earlier string-derived breakpoints.

## Notes

This revision is intentionally metadata-only:
- no extracted ELF payloads
- no copyrighted asset content
- no disassembly dump beyond the local observations needed to justify the inference

