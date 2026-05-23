# PAL→USA Final Summary

Date: 2026-05-22 23:49

## Objective

Close the PAL→USA reconciliation pass around `MAIN.MAP`, object ranges, and the new whole-text `op_seq` recovery pass.

## Scope

- PAL symbol table and `MAIN.MAP` parsing.
- Object-range reconciliation by `.o` file.
- Whole-text recovery for objects with no initial seed.
- Current counts from the generated CSVs in `research/pal-usa/`.

## Sources Used

- `.local/extracted/pal/MAIN.MAP`
- `.local/extracted/pal/SRCFILE.TXT`
- `docs/symbols/pal_usa_symbol_map.csv`
- `docs/symbols/pal_fingerprints.json`
- `docs/symbols/usa_fingerprints.json`
- `research/pal-usa/main_map_functions.csv`
- `research/pal-usa/main_map_objects.csv`
- `research/pal-usa/pal_usa_object_range_candidates.csv`
- `research/pal-usa/pal_usa_verified_seeds.csv`
- `research/pal-usa/recovery_log.csv`

## Evidence Used

- Linker-map ordering from `MAIN.MAP`.
- Function fingerprints from the PAL and USA `.text` sections.
- Whole-text `op_seq_hash` scanning on the USA side for objects without seeds.
- Function-level validation against predicted USA addresses.
- Source-listing evidence from `SRCFILE.TXT` for future source-file joins.

## Byte-Level Findings

| Item | Value | Notes |
|------|-------|-------|
| MAIN.MAP functions | 4368 | Parsed from the PAL map |
| MAIN.MAP objects | 208 | Object files grouped from the map |
| PAL fingerprints | 4357 | Functions with usable PAL fingerprints |
| Verified seeds | 362 | Reliable seeds from `raw_sha1` / `op_seq_hash` |
| Object-range matches | 415 | Functions validated in the range pass |
| Range objects with confidence | 65 | 30 MEDIUM, 35 LOW |
| Range objects with no seed | 143 | Initial recovery target set |
| Recovery-pass recovered objects | 62 | From the 143 no-seed objects |
| Recovery-pass validated functions | 168 | Additional validated functions |
| Remaining unrecovered objects | 81 | Still need another anchor or a source join |
| `SRCFILE.TXT` provenance rows | 199 | 152 verified + 47 candidate rows enriched, 17 distinct source files |
| `MAIN.MAP` source rows | 61 | `main_map_functions_source.csv`, 16 distinct source files |

## Main Tables

### Object-Range Pass

| Confidence | Objects | Functions Mapped |
|------------|---------|------------------|
| HIGH | 0 | 0 |
| MEDIUM | 30 | 0 |
| LOW | 35 | 415 |
| INVALID | 0 | 0 |
| NONE | 143 | 0 |

### Recovery Pass

| Result | Count |
|--------|-------|
| Recovered objects | 62 |
| Recovered functions | 168 |
| Still unrecovered objects | 81 |

### Symbol Map Snapshot

| Status | Count |
|--------|-------|
| verified | 592 |
| candidate | 2611 |
| unmatched | 1333 |

## Confirmed

- `MAIN.MAP` is useful as an object-order and range source.
- The range pass is not enough by itself; many objects need a second recovery step.
- A whole-text `op_seq_hash` search can recover objects that have no initial seed.
- The recovery pass is not speculative: it validated `168` functions across `62` objects.
- `SRCFILE.TXT` is useful as a provenance source: `199` symbol-map rows now have a source file across `17` distinct sources, and `61` `MAIN.MAP` functions now carry source provenance across `16` distinct sources.

## Probable

- The remaining `81` objects are either still missing a usable anchor or need a different matching strategy.
- `SRCFILE.TXT` is the right place to mine source-file provenance for the USA-side symbols, but it still needs a clean join into the final CSVs.

## Possible

- Some unresolved objects may be recoverable by trying more than one anchor function per object, not just the first available one.
- A source-file join from `SRCFILE.TXT` may disambiguate objects with repeated prologues or weak first-function anchors.

## Unknown

- Whether every remaining `NONE` object is truly PAL-only.
- Whether the unresolved set can be closed without a manual review of the remaining candidate offsets.

## Discarded

- Pure fuzzy matching as a primary reconciliation method.
- Treating `same_va` as high-confidence evidence by itself.
- Treating previous AI notes as authoritative without direct validation.

## Next Minimum Test

Run a second recovery pass on the `81` unresolved objects using:

1. more than one anchor function per object, and
2. a direct `SRCFILE.TXT` join for USA-side provenance.

## Conservative Verdict

The `MAIN.MAP` discovery was materially useful. It turned the project from a mostly fuzzy symbol recovery effort into a repeatable object-range pipeline with a working recovery pass. The result is not a full reconciliation yet, but it is no longer blind: `415` functions are validated in the range pass, and `168` more were recovered from the previously seedless set.
