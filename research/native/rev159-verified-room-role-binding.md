# Rev.159 — Per-room runtime-verified GObj↔handler role binding (Passo 1-3)

- Date: 2026-09-09
- Branch: native-port
- Objective: substitute the Rev.155 blind round-robin `m_sceneGObjs[i % size]`
  asset pairing with a `GObjHandle→handler` map per room whose handler
  multiset comes from the runtime-verified repertoire (Rev.158), and re-link
  it on every world-state (scene) transition, not only at initial load.
- Sources used: Rev.158 runtime capture
  (`.local/pcsx2-logs/session-extracted-20260909-gaiola.jsonl`), Rev.154/155
  scene tables and attachment seam, USA ELF entry/descriptor table reads.
- Evidence used: `isys_gobj_proc_add` events resolved to `TARGET_FUNCTIONS`
  names via `asm_source_score` (a1 = pool address, a3 = 1 for the main
  handler vs 0 for layer/init); distinct pool-address counts per (sceneId,
  handler); ELF payload reads for scenes 0x0F and 0x2B.

## What changed

| Step | File | Change |
|------|------|--------|
| Tool | `tools/extract_room_role_tables.py` | NEW. Reads an instrumented PCSX2 JSONL (`runtime_probe_analyzer` event format), resolves `isys_gobj_proc_add` targets against `TARGET_FUNCTIONS`, counts DISTINCT pool addresses per (sceneId, handler — the honest measure, event counts double-count re-registrations), and emits `native/src/game/GeneratedRoomRoleTables.h`. |
| Tables | `native/src/game/GeneratedRoomRoleTables.h` | NEW, generated. 11 captured scenes, 240 role slots. `VerifiedRoomRole{handlerAddr,handlerName,roleCount}`, `VerifiedRoomRolePlan{sceneId,roles,roleCount}`, `kVerifiedRoomRolePlans[]` (namespace `ico::engine`). |
| Tables | `native/src/game/GeneratedSceneTables.h` | REGENERATED with `--scene 0x0F 0x2B`: payload count 29 → **54** (scene 0x0F slice + scene 0x2B slice [2151,2176)). CSVs under `.local/extracted_scene_tables/`. |
| Loader | `native/src/game/KanbanSceneLoader.{h,cpp}` | `applyVerifiedRoomRolePlans()` copies plans; `hasRoomRolePlan(sceneId)`; `gobjHandlerRole(i)` exposes the GObjHandle→handler binding; `attachBoundAssetsToGObjs()` now tags the scene GObjs from the room's expanded role slots; `relinkAttachmentsForCurrentScene()` clears + re-pairs; `execute()` calls it automatically after every `initSceneGObj` (Passo 2). `initSceneGObj()` now RELEASES the previous room's GObjs first (pool reuse matches Rev.158: a pool address changes owner between rooms). |
| Manifest | `native/assets/scene/stgst00a.manifest` | Added `scene 0x2B` block reusing the host piece library as HOST TEST DATA (does not claim these compose original room 0x2B). |
| Tests | `native/tests/gobj_attachment_test.cpp` | Passo 3: scene 0x2B → 23 GObjs, 26-slot plan / 8 handlers, deterministic multiset assert (flag×5, torch×5, type36×5, type60×5, type6×3 tags over 23 GObjs), `execute()` relink re-pairs. |
| Tests | `native/tests/verified_scene_test.cpp` | Payload count 29 → 54; scene 0x2B creates 23 GObjs; per-descriptor counts (BGA 13, SOBJ 4); render sanity moved before the 0x2B section (current scene changed). |

## Verified facts (new)

- **Scene 0x2B on the USA ELF:** dispatch range [2151,2176), 25 payload rows;
  descriptor gate-0 skips DYNAMICMOTIONDAT (desc 44) and STAGESETTING
  (desc 54) → **23 host GObjs**; per-descriptor: SOBJ 4, FLEVER 1,
  DEMOMOTCTRL 3, PARTICLE 1, POOL 1, BGA 13.
- **Scene 0x2B runtime repertoire (Rev.158 capture):** 26 slots / 8 handlers —
  flag_hB ×5, torch_hB ×5, type36_hB ×5, type60_hB ×5, type6_hB ×3,
  type39_hB ×1, type22_hB ×1, boy_hB ×1.
- **Descriptors do NOT provide the handler map.** For 0x2B, `processCallback_40`/
  `procCallback_58` (0x203EE8 / 0x20F688) do not match any repertoire handler.
  The room's arrived runtime `isys_gobj_proc_add` repertoire is the verified
  target; the remaining steps (which handler binds which model; asset label →
  handler) stay explicit HOST heuristics.
- **Honest role metric:** distinct pool-address counts, not event counts
  (a single room re-registers handlers repeatedly during play; 0x04/0x05
  captured twice with identical repertoires cross-validates).
- **GObj pool reuse:** host `initSceneGObj` now releases the previous scene's
  GObjs; active-count invariant is per-scene (23 for 0x2B), not cumulative.

## What is probable / possible / unknown

- Probable: the per-room handler multiset from the runtime capture reflects
  the original room's primary processing GObjs; scaling it over the host's
  created GObjs preserves the room's handler proportions.
- Possible: a subset of role slots maps to fixed models (torch_hB → torch
  pieces, boy_hB → boy) that a future capture could confirm.
- Unknown: which specific asset each handler drew; GObj pool addresses in the
  capture do not map to host handles.
- Discarded: using `processCallback_40/58` as the handler map (mismatch
  above); event-count repoid (overcounts re-registrations).

## Next minimum test

- Run `gobj_attachment_test` (28 GObjs-composed, 23 role-tagged, relink
  verified) and `verified_scene_test` (0x0F + 0x2B) — both pass.
- CTest: 27/28 (only `opengl_backend` headless segfault, known baseline).
- Demo: `ico_native --frames 2 --shot` runs, 25 host GObjs / 28 attachments /
  15019 real triangles.

## Conservative verdict

The room's GObj↔handler map is now driven by runtime-verified per-room
repertoires and re-linked on every scene transition; asset→GObj pairing
remains a documentable host heuristic within the tagged set. No new
byte-exact `.s` claimed. No modified original binary.