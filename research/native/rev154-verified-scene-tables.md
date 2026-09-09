# Rev.154 — Verified scene tables drive real GObj creation (25 host GObjs for scene 0x0F)

- **Date:** 2026-09-09
- **Branch:** native-port
- **Objective:** Feed byte-verified USA ELF scene tables into the semantic
  `KanbanSceneLoader` so `initSceneGObj()` actually creates host GObjs (front 2
  of AGENTS.md), then verify GObjs > 0 in the demo with the BoyController
  process stable on the same `gobjRuntime`.
- **Milestone:** scene 0x0F (native demo room, stage st00a) now creates **25
  host GObjs** from the verified descriptor/entry/range tables; 26/27 CTest
  (only the documented headless `opengl_backend` segfault fails).

---

## Scope

- `tools/extract_scene_tables.py` — new byte-level extractor (USA ELF →
  `GeneratedSceneTables.h` + per-scene CSV).
- `native/src/game/GeneratedSceneTables.h` — generated constexpr tables (68
  descriptors, 97 scene ranges, 29 payload entries for scene 0x0F).
- `native/src/game/KanbanSceneLoader.h/.cpp` — `applyVerifiedSceneTables()`,
  `gate_44`, per-entry `listId`/`gobjType`/`flag_44`, `sceneGObjCount()`,
  `kSceneEntryCount` 512 → 3600, `kSceneEntryListIdUnknown`.
- `native/src/main.cpp` — wiring of the verified tables into the demo load path
  and corrected GObj-count log.
- `native/tests/verified_scene_test.cpp` + CMake target (new).

## Evidence used

- `src/entity/asm/initSceneGObj.s` (byte-exact) — descriptor/entry field
  layout, descriptor+0x44 gate, entry+0x48 listId bits, entry+0x47 gobjType.
- USA ELF `.local/extracted/SCUS_971.13.elf` — descriptor table 0x2A31B8
  (stride 0x64, exactly 68 entries ending at 0x2A38C0), entry table 0x2A4C48
  (stride 0x4C, valid descIdx<68 contiguous run idx 0..3590), world dispatch
  table 0x5F2FB8 (stride 0x194).
- `cmake --build native/build` for every affected target (0 errors).
- `ctest --test-dir native/build` → **26/27 pass** (only pre-existing headless
  `opengl_backend` segfault; baseline was 25/26).
- Live run: `./build/ico_native --scene st00a --frames 60` (X11 display
  available) — clean shutdown, stable 60 fps, 0 errors.

---

## Findings

### Entry-table size correction (Rev.154)

AGENTS.md's "512 entries" is an understatement. The valid `descIdx < 68` run in
the entry table is **contiguous from idx 0 through 3590** (first invalid at
3591). The native loader now owns `kSceneEntryCount = 3600` so scene 0x0F's
slice (idx 847..875) is addressable. This was already proven by the runtime
captures (higher scene indexes) but not reflected in the loader.

### Verified extraction tool

`tools/extract_scene_tables.py` reads the USA ELF and emits:
- `kVerifiedSceneDescriptors[68]` — each `{descriptorIndex, gate_44,
  processCallback_40, processCallback_58, vtable_60, name}`.
  Names match the ICO-decomp identity (BOY, GIRL, ENEMY1, SOBJ, BIRD,
  GIRLFORCEFIELD, DYNAMICMOTIONDAT, STAGESETTING, ...).
- `kVerifiedSceneRanges[97]` — verified tiling of `[41,3453)` from the world
  dispatch table `0x5F2FB8 + scene*0x194`, using `+0x128` start / `+0x12C` end
  (scenes 0x01..0x64). Slot order is not scene-id order (e.g. scene 0x10 =
  [790,847) precedes 0x0F = [847,876)).
- `kVerifiedScenePayload` — entry records `{entryIndex, descriptorIndex,
  listId, gobjType, processArgument_40, userData_30, flag_44,
  processCallback_24}` for a requested scene (CPU-sorted by entry index). p1
  restore note: `listId` is per-entry here; the descriptor table's own list id
  is not used by the entry path (matches Rev.112).

**Scenes 0x69-0x7F are excluded** — they have `start=35` and overlap the base
scene slice; they are a separate base-append semantic, not per-entry ranges.

### `applyVerifiedSceneTables()` (semantic C++)

Validates ranges (distinct scene ids, no wrap), copies descriptor
`processCallback_40`/`gate_44`, enables each payload entry with its raw fields,
and binds every payload entry to its scene id via range membership. Written
explicitly not to claim byte-exactness at this level — it is the semantic bridge
feeding the original `requestScene()`/`execute()` (kanban.c) flow.

### `initSceneGObj` gate + list placement

- `SceneGObjDescriptor.gate_44` = descriptor `+0x44`; `0` skips GObj creation
  entirely (matches the `beqz → loc_001b7b50` path in `initSceneGObj.s`).
- Entry `listId` (`entries +0x48` bits[16:14]) wins; `kSceneEntryListIdUnknown`
  (`0xFF`) falls back to the descriptor list id — preserves the old manual
  tests' behavior.
- Entry `gobjType` (`+0x47` low 5 bits) is written into `gobj->type`.

### Scene 0x0F = 29 payload entries → 25 GObjs

The 29 entries are e.g.: 847/872/875 DYNAMICMOTIONDAT (desc 44), 848/849 NONE,
850..856 SOBJ (desc 7; lists 7×5 + list 2/1/0 at 853/854/855), 857..860 BIRD
(desc 32), 861 SOFA, 862 GIRLFORCEFIELD (desc 55), 863 SEFFECT, 864 PARTICLE,
865/869/870/871/874 BGA, 866 FLEVER, 867 POOL, 868 CAMERADUMMY, 873
STAGESETTING (desc 54). Gate-0 descriptors (44, 54) drop out: **29 − 4 = 25**.

### Test + demo verification

- `verified_scene_test`: wires the generated tables; asserts payload head
  (entry 847, desc 44, count 29), 29 enabled scene-0x0F records, **25 GObjs
  created**, pool activeCount == 25, exactly one list-0 entry (entry 855), 4
  BIRD + 1 GIRLFORCEFIELD descriptors, and 25 synthetic sprites through the
  GIF bridge.
- Demo (`ico_native --scene st00a --frames 60`):
  ```
  main: player spawn at (-50,1000) -> ok pos=(-50,100,1000)
  main: scene 0x0F semantic load via requestScene/execute: ok (currentSceneId=0x0F, 25 host GObjs)
  [render] Frame: 22 draw calls, 26361 triangles (26349 real, 12 degenerate)
  ```
  BoyController spawn + 60 fps + 0 errors for the whole run.

---

## Confirmed

- Valid entry-table descriptor-index run is contiguous 0..3590; loader now
  sized to 3600 entries.
- 68 descriptors + 97 verified scene ranges tile `[41,3453)` exactly.
- Scene 0x0F → 29 payload entries → 25 host GObjs (4 gate-0 descriptors).
- Descriptor/entry list ids are per-entry (`entries +0x48` bits[16:14]).
- CTest 26/27; demo path creates >0 GObjs with BoyController stable.

## Probable

- Scenes 0x69..0x7F (`start=35`) are a base-append layer appended to the scene
  entry slice, per the same dispatch table; not yet exercised by the native
  loader (out of the demo's demo-room scope).

## Possible / Unknown

- `processCallback_24` (`entries +0x24`) is copied into the record but not yet
  into any host process registration; the 0x115108-style registration wrapper
  is not yet bridged for these rows.
- Whether any of the 25 GObjs correspond to drawable pieces (vs pure logic
  entities like CAMERADUMMY/POOL) is not yet resolved — the render loop stays
  decoupled from the loader.

## Discarded

- "512 entries" as the entry-table size (understatement; corrected).
- Fixing `tools/elf_table_dump.py`'s wrong descriptor offsets (name+0x00,
  init+0x04, ...) — superseded by the new verified extractor.

## Next minimum test

1. Re-run `verified_scene_test` + full CTest after any loader change (expect
   26/27).
2. Load a second room (e.g. one whose dispatch row differs) through the same
   verified-table path and confirm its GObj count matches its payload size
   minus gate-0 rows.
3. Evaluate whether scene-0x0F's GObj rows should drive real per-GObj asset
   attachment (linking table entries to `.p2o` pieces by descriptor).

## Conservative verdict

The loader front is upgraded from "0 GObjs (descriptors not recovered)" to
"25 GObjs from byte-verified USA tables" with zero CTest regressions and no
BoyController destabilization. The remaining open item is per-GObj asset
attachment, not descriptor recovery.