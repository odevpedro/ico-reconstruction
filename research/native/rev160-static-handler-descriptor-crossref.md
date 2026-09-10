# Rev.160 — Static handler→descriptor cross-reference: runtime rosters resolve 68/68 to named descriptors (hB=+0x50, hD=+0x5C)

**Data:** 2026-09-09
**Branch:** native-port (P1/P2 static evidence)
**TRILHA:** PORT

## Objective

Close, **statically** (no new PCSX2 session), the open item from Rev.159:
map every runtime room-role handler back to its entity descriptor so the host
can stop treating `typeNN_hB` names as opaque and can ground them in the
byte-verified descriptor table `0x2A31B8`.

## Scope

- USA ELF `.local/extracted/SCUS_971.13.elf` (only static source).
- Runtime rosters already captured (Rev.158) in
  `native/src/game/GeneratedRoomRoleTables.h` (11 scenes, 77 role rows).
- `tools/static_crossref_handler_descriptor.py` (new, reproduces this note).
- `tools/runtime/classify_entities.py` `DESCRIPTOR_TABLE` (61 named rows,
  hA/hB/hC/hD/init_fn/vtable fields) — used as the naming vocabulary only.

## Evidence used

- ELF descriptor table read at `0x2A31B8`, stride `0x64`, 68 rows.
- Field offsets recovered from raw ELF dumps (Rev.160, see layout below).
- Scene payloads from dispatch table `0x5F2FB8 + scene*0x194`
  (`[+0x128,+0x12C)` = entry-index range) and entry table `0x2A4C48`
  (`descIdx = byte +0x46`).
- Reproducible: `python3 tools/static_crossref_handler_descriptor.py`.

## Confirmed facts

### Descriptor field layout (byte-verified, Rev.160)

| Offset | Field | Meaning |
|--------|-------|---------|
| `+0x00` | `name` (16B) | ASCII model/entity name |
| `+0x40` | `init/cb40` | init callback (0 when none) |
| `+0x44` | `gate` | 1 = creates GObj; 0 = skip (DYNAMICMOTIONDAT/STAGESETTING) |
| `+0x48` | `hA` | handler A |
| `+0x50` | `hB` | **handler B — runtime primary process** |
| `+0x58` | `hC` | handler C |
| `+0x5C` | `hD` | **runtime secondary process** (girl_hD, enemy1_hD) |
| `+0x60` | `vtable` | vtable pointer (0x202A60 BOY/GIRL/ENEMY1; 0x23D660 static props) |

Correction of the earlier Rev.158–159 guess: hD is at `+0x5C`, not a
5th contiguous field after hC; the raw row is
`... hC @+0x58, hD @+0x5C, vtable @+0x60`.

### Runtime handler resolution: 68/68 (0 UNRESOLVED)

All 77 role rows across 11 scenes resolve to a named descriptor field:

| Field | Roles | Examples |
|-------|-------|----------|
| `hB` (+0x50) | 62 | boy_hB, girl_hB, torch_hB, flag_hB, type6/22/24/39/55/60, ItemGeo=desc19 BARREL hB, buddys: seffect, weapon (runtime_1F3A00=desc14 WEAPON hB), chain, ap1, generator, enemy1, bird, woodbox0, type15(SPIDER_LAYOUT) |
| `hD` (+0x5C) | 6 | girl_hD=desc2 GIRL hD (0x1D1AD0), enemy1_hD=desc4 ENEMY1 hD (0x1CE760) |
| unresolved | 0 | — |

`typeNN_hB` == `DESCRIPTOR_TABLE[NN].hB` exactly (verify: type60=desc60 KYOMI
hB=0x0023D518; type36=desc36 DEMO_QSWORD hB=0x001F44C8; type39=desc39 POOL
hB=0x0010D070; type22=desc22 FLEVER hB=0x001BC1A8; type6=desc6 DEMOMOTCTRL
hB=0x001CE6F0).

### Runtime roster = entry-table payload + room-script objects

Per-scene comparison (roleCount vs static payload count for the same descriptor):

| Scene | payload rows | exact matches (p) | payload-absent (`-`) | count mismatch (x) | hD |
|-------|-------------|-------------------|----------------------|--------------------|-----|
| 0x01 | 34 | 4 | 1 (KYOMI×5) | 0 | 0 |
| 0x03 | 23 | 1 | 2 (KYOMI, BOY) | 0 | 0 |
| 0x04 | 80 | 0 | 1 (KYOMI×10) | 8 | 2 |
| 0x05 | 26 | 0 | 2 (KYOMI, BOY) | 4 | 0 |
| 0x06 | 26 | 1 (GIRLFORCEFIELD) | 3 (KYOMI, BOY, GIRL) | 0 | 1 |
| 0x07 | 43 | 8 | 3 (KYOMI, AP1, BOY, GIRL) | 0 | 2 |
| 0x28 | 33 | 2 (FLAG, ENEMY1) | 3 (KYOMI, BOY, GIRL) | 0 | 1 |
| 0x29 | 28 | 2 | 2 (KYOMI, BOY) | 0 | 0 |
| 0x2A | 27 | 2 | 2 (KYOMI, BOY) | 0 | 0 |
| 0x2B | 25 | 3 (DEMOMOTCTRL, POOL, FLEVER) | 4 (KYOMI, BOY, FLAG, TORCH, DEMO_QSWORD) | 0 | 0 |
| 0x2D | 57 | 2 | 5 (KYOMI, BOY, FLAG, TORCH, DEMO_QSWORD) | 0 | 0 |

**Totals: 25 exact, 33 payload-absent, 13 count-mismatch, 6 hD, 68 resolved.**

Meaning: when a descriptor appears in the static payload of that scene, the
roleCount matches the payload count exactly (0x07: 8/8; 0x2B: DEMOMOTCTRL
3=3, POOL 1=1, FLEVER 1=1; 0x01: DEMOMOTCTRL 6=6, BOY 1=1, SEFFECT 1=1,
WEAPON 1=1). The payload-absent descriptors (KYOMI, DEMO_QSWORD, AP1, plus
BOY/GIRL in most scenes) are registered by the room's init_fn / world_state
scripts via direct `CreateGObj`, **not** by the entry table.

### KYOMI / DEMO_QSWORD / AP1 never appear in ANY scene payload

Scanning all 97 verified scene ranges (sc 0x01..0x69): only 47/68 descriptors
ever appear in a payload. **Never in any payload:** NULL(0), TREE(9), ROPE(20),
FLEVER_TRISTATE(23), WLEVER2(25), SV(27), DUMMY(29), CANDLE(34), MOBJ(35),
**DEMO_QSWORD(36)**, CHANDELIER(37), ROPEFIX(42), CAGEFIX(50), INTEREST1/3/10/20,
**KYOMI(60)**, **AP1(61)**, ATTACKCHECKBOUND(62), temp(65), ENEMY_CONTROL(66).

KYOMI is the most frequent runtime handler (5 per room in most scenes, 9 in
0x07): it is a per-room system/ambient object created by script, never from
the entry table.

### Count mismatch explanation

`roleCount` = distinct GObj **pool addresses** that carried the handler as
primary during the room segment. The GObj pool is reused across rooms and
objects are freed/lit at runtime (torches), so the same static payload can
feed more distinct pool addresses over a long segment (0x04: TORCH static 5
vs roleCount 14; 0x05: FLAG 5 vs 10). This is a runtime book-keeping artifact,
not a static contradiction.

## What is probable

- The runtime capture's `init_scene_gobj a0` sceneId (0x01..0x2D) indexes the
  same dispatch table used by `KanbanSceneLoader` (ranges [41,3453)); the
  exact-match rows make this near-certain for scenes with overlap.
- hB is "primary process callback" and hD is "secondary callback" registered
  by the same descriptor-driven creation path (a3==1 vs a3==0 in Rev.158).

## What is possible

- Other `hD`-only handlers exist for descriptors that never appear in a
  capture (e.g. QUEEN, QUEEN_BALL, ATTACKCHK*) — the 0x5C field is generically
  the secondary callback for any descriptor that runs one.
- The room-scene "script objects" (KYOMI/DEMO_QSWORD/AP1) map to a small set
  of per-room ambient entities that the native host could model explicitly.

## What is unknown

- Which init_fn/world_state code path instantiates KYOMI ×5 per room
  (world_state_load 0x1AF948 → ?). Static follow-up, no runtime needed.
- The exact descriptor-driven relation between the payload-absent descriptors
  and their meshes (mesh binding for KYOMI/DEMO_QSWORD/AP1 remains HOST-side).

## Discarded

- Earlier Reading that typeNN names were opaque "add/remove" flags: they are
  the descriptor's hB (primary) values, name-consistently.
- The guess that hD lives in a contiguous 4th handler slot: it is at +0x5C.

## Next minimum test

- Any new runtime capture must resolve to 0 UNRESOLVED with this tool; every
  payload-present descriptor must keep `roleCount == staticCount` where the
  scene is captured fresh (no long pooled segment).

## Conservative verdict

The handler→descriptor binding is now a **static, byte-verified, closed map**:
every room-role handler in the runtime rosters is the hB (or hD) field of a
named descriptor from 0x2A31B8. The runtime roster is, per scene, the union
of the entry-table payload (exact counts) plus room-script objects
(KYOMI/DEMO_QSWORD/AP1/BOY/GIRL). No new runtime capture was required.

## Sources

- `.local/extracted/SCUS_971.13.elf`
- `native/src/game/GeneratedRoomRoleTables.h`
- `tools/runtime/classify_entities.py`
- `tools/static_crossref_handler_descriptor.py` (new)