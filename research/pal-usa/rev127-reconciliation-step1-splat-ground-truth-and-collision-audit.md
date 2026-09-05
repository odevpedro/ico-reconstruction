# Rev.127 — Reconciliation step 1: regenerate splat ground truth + collision audit

- **Date:** 2026-09-05
- **Objective:** Apply the verified PAL→USA reconciliation as splat/Ghidra ground truth
  and audit what still blocks the wider (object-range) mapping.
- **Scope:** Static reconciliation only; no new decompilation, no runtime.
- **Sources used:**
  - `tools/symbol_reconcile/reconcile.py --step splat`
  - `research/pal-usa/pal_usa_reconciliation_summary.md` (2026-09-03)
  - `research/pal-usa/pal_usa_function_map_candidates.csv`
  - `docs/symbols/pal_usa_symbol_map.csv`
- **Evidence used:** regeneration output; per-VA collision audit of MATCH rows.

## 1. What was done

Re-ran `reconcile.py --step splat` from the current `pal_usa_symbol_map.csv`
(592 `verified` PAL→USA rows → **542 unique VAs**).

- `docs/symbols/symbol_addrs_usa.txt` — 542 entries (unchanged content vs the
  previously committed copy).
- `docs/symbols/ghidra_labels.txt` — 542 labels.
- `splat/SCUS_971.13.verified-symbols.yaml` — **regenerated**; compared to the stale
  committed copy it now includes verified symbols that were absent before, e.g.:
  - `UnitRotation` (0x7178)
  - `ACTGame_SetMotionPlaySpeedRatio_Exec` (0x48F88), `ACTGameView_Check` (0x4AC28)
  - `ClipWallAdjustPos` (0x62800), `DrawGObjWallCollision` (0x661F0)
  - `GetChainLength` (0x87650), `GetChainNodeID` (0xC6108)
  - `scpLinkBGAtoLayoutedTargetSkeltonWithLocalRotationFlag` (0x7B328)
  - `actConte14_12` (0x10A968)
  - the `__text__` source-less block grew from 392 → 405 symbols
  - fixed the machine-specific `target_path` (peter → hoper)

The `docs/symbols/*` files are gitignored; only the versioned `splat/*.yaml` changed.

## 2. The two reconciliation pipelines (state)

| Pipeline | Artifact | Coverage | Confidence |
|----------|----------|---------:|------------|
| fingerprint (`symbol_reconcile`) | `pal_usa_symbol_map.csv` | 592 verified | conservative, high |
| object-range (`pal_usa_reconcile`) | `pal_usa_function_map_candidates.csv` | 1413 MATCH | HIGH/MEDIUM mixed |

The fingerprint map (592 verified) is the clean ground-truth source for splat.
The object-range map (1413 MATCH) is broad but **not yet safe to promote**.

## 3. Collision audit — why 1413 MATCH are not yet splat-ready

A per-VA dedup of the 1413 `MATCH` rows shows **259 duplicate `usa_va` values**.
Several PAL symbols collapse onto a single USA address, indicating over-assignment
(offset-derived matches); e.g. `geometryManager.o`:

```
GetRootQuaternion                 0x00102BF0 -> 0x00102C04
UpdateRootMatrix                  0x00102BF8 -> 0x00102C04
SetRootBaseQuaternion             0x00102C00 -> 0x00102C04
SetRootQuaternion                 0x00102C10 -> 0x00102C04
SetRootMatrixWithTransOffsetByDObj 0x00102C90 -> 0x00102C04
```

Five PAL symbols all land at USA `0x00102C04` — these are not five distinct USA
functions. Promoting the whole MATCH set to splat would inject wrong names, against
the project rule (never upgrade a hypothesis into a conclusion without evidence).

## 4. What is confirmed

- The 592 verified fingerprint matches are applied as splat/Ghidra ground truth; the
  versioned YAML is regenerated and current.
- The object-range 1413 MATCH set has 259 VA collisions that must be curated before
  any of it can be promoted.

## 5. What is probable / possible / unknown

- **Probable:** most 259 collisions are offset-derivation artifacts (MEDIUM confidence
  ranges with an incorrect object offset), not genuine overloads.
- **Possible:** a fraction of the collisions are real "same function, several names"
  cases (AI-glue wrappers) — these are legitimate aliases worth keeping.
- **Unknown:** which collisions are artifacts vs real aliases; resolution requires
  per-object inspection against the USA ELF symbol table.

## 6. Discarded

- Promoting all 1413 MATCH straight into `symbol_addrs_usa.txt` — rejected due to the
  259 collisions.

## 7. Next minimum test (reconciliation)

For one HIGH object (e.g. `geometryManager.o`), resolve its collisions against the USA
ELF symbol table and decide artifact-vs-alias per hit; if resolved cleanly, add an
allow-list of curated extras to the splat generator.

## 8. Conservative verdict

Step 1 delivered: splat/YAML ground truth refreshed from the verified fingerprint map
(542 unique VAs), plus an audit proving the wider object-range map needs collision
curation before promotion. No speculative names were injected into ground truth.
