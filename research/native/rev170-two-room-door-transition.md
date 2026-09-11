# Rev.170 — Two-room door transition: real resident-set swap (st00a 0x0F ↔ st02a 0x2B)

- **Date:** 2026-09-10
- **Branch:** `native-port`
- **TRILHA:** PORT
- **Supersedes/extends:** Rev.168 (`RoomTransitions` host component), Rev.159 (verified room-role binding), Rev.154 (verified scene tables), Rev.155 (per-GObj composition)

## Objective

Make the door demo cross a **real kanban seam**: `requestScene(0x2B)` must load a
different room's asset bundle (geometry, collision, sky, camera, GObj composition)
instead of re-executing the same room. Also turn the host asset pipeline into a
two-room data-driven model (`--room <primary> --pair <companion>`) with per-room
ClipBridge / camera fit / sky / door zones.

Secondary hardening discovered during verification: fix room collision fallback to
never use a decorative first piece, and snap door zones to a point the player can
actually stand on (5-corner predicate).

## Evidence (byte/runtime verified)

### Two-room resident sets

| Room | Store | Piece assets | Scenes bound | Host GObjs (verified_scene) |
|------|-------|--------------|--------------|------------------------------|
| st00a (primary) | shared store | 33 | 0x0F | 25 |
| st02a (companion) | same store (parseRoom → 0x2B) | 30 | 0x2B | 23 |

Confirmed in log (run 3):

```
loader bound scene 0x0F (st00a): 33 assets (first=0str01_s2)
loader bound scene 0x2B (st02a): 30 assets (first=02a_flare1)
scene 0x0F semantic load via requestScene/execute: ok (currentSceneId=0x0F, 25 host GObjs)
DOOR OPEN st00a -> st02a, spawn(20.7199,-44.5774)
  transition execute: ok (scene 0x2B, 23 host GObjs)
door zone st02a at (-210,25) -> scene 0x0F      (reverse zone reinstalled post-swap)
```

### Render evidence of the real swap (frames, run 3)

| Phase | Draw calls | Triangles | Room on screen |
|-------|-----------|-----------|----------------|
| Pre-door-open (frames 2 … ~47) | 68 | 29,462 | st00a |
| Post-transition (frames ~50 … 260) | 54 | 34,195 | st02a |

The change in composition (68→54 calls, 29,462→34,195 tris) persists through the
remainder of the run, so the screenshot `/tmp/rev170-swap3.ppm` is the st02a view
after the swap.

### Door zones

| Room | Piece | Zone center (snapped) | Radius | Target scene | Post-swap spawn |
|------|-------|----------------------|--------|--------------|-----------------|
| st00a | `door.p2o` | (16.6671, 15.2856) | 40 | 0x2B (st02a) | (20.7199, -44.5774) |
| st02a | `2a_door1.p2o` | (-210, 25) **unsnapped** | 40 | 0x0F (st00a) | (-167.764, -17.6158) |

`setOpenDelay(1.0f)`; spawn = door center + normalize(door − room center)×60.
One-way demo this round: the st02a zone did **not** snap — no 5-corner-standable
point within 150 units of the door on the fallback collision (see Blocked/Open).

### ClipBridge fallback correction (Rev.170 hardening)

Before: room with no loadable `_p1` used its **first** piece. st02a's first piece is
`02a_flare1.p2o` → useless 8×7 grid, blocked=7 (Rev.169 player could barely move and
the door never fired). After: fallback = **largest successfully-parsed mesh** of the
room.

| Room piece | Parses? | Verts / Tris / Submeshes | Collision grid |
|------------|---------|--------------------------|----------------|
| `st00a_p1.p2o` | yes | 13,877 / 15,007 / 3 | 36×41, blocked=201 |
| `st02a_p2.p2o` | yes | 17,393 / 21,586 / 58 | 73×72, blocked=4143 (fallback chosen) |
| `st02a_p1.p2o` | **no** | — | — (p2-family; Open item) |
| `st02a_p3.p2o` | **no** | — | — |

The same "largest loaded" fallback now also drives the **non-macro camera fit** and
the **spawn anchor** when `_p1` is missing.

## What is confirmed

- Door demo now performs a **real resident-set swap** through `KanbanSceneLoader`
  (kanban seam), swapping scene, GObj composition, sky, camera and collision.
- Reverse door zone is reinstalled after crossing (correct timing: at entry, not at
  startup).
- Multi-scene load in one shared `SceneAssetStore` works (`parseRoom` merges only its
  own asset records; boot-time `parse()` reset semantics preserved).

## What is probable

- st02a's true interior is `st02a_p1.p2o` (67 OBJH submeshes seen during file probe),
  which currently fails `loadPs2oMeshFromFile` because its face region does not start
  with the Rev.151 `[N, 0xFFFF, 0xFFFF, …]` strip-header pattern (probe: face region
  at 0x1150 begins `16 16 18 ff 0e 0e 0e ff …`). Two possibilities: a 16-bit-index /
  per-submesh variant of the p1 cascade format, or the p2 wall-family (Rev.151 "open")
  layout.

## What is unknown

- Whether `st02a_p1.p2o` loaded under any earlier rev (Rev.169 log, same failure:
  `failed to load ... st02a_p1.p2o` in `/tmp/rev169c-st02a.log` at line 32) — the
  failure is pre-existing, not a regression of this rev.
- The exact encode of the st02a_p1 face region.
- The texture-position complaint from Rev.169 (boy/mesh offsets under the round-robin
  GObj↔mesh pairing) is **not** resolved in this rev; backlog item.

## What is discarded

- The 8×7 flare-grid collision for st02a (wrong piece, useless grid).
- Treating the first room piece as a fit/floor fallback.

## Next minimum test

1. Run `./build/ico_native --room st00a --pair st02a --frames 260 --teleport 16.6671,15.2856 --shot /tmp/rev170-swap3.ppm` → expect `DOOR OPEN st00a -> st02a` + `transition execute: ok (scene 0x2B, 23 host GObjs)` + frame stats changing 68/29,462 → 54/34,195 (already green).
2. Optionally decode `st02a_p1.p2o` (p1-variant or p2-family) so st02a gets its real
   walkable floor and the door works in both directions.

## Conservative verdict

Rev.170 delivers the Port item: a data-driven two-room demo where a door piece
triggers a genuine `requestScene(0x2B)` exchange producing different geometry,
collision, sky and GObj composition. GObj↔mesh binding remains the Rev.155
host heuristic (round-robin / role-tagged), not byte-verified reconstruction. The
st02a one-way limitation is caused by the unresolved `st02a_p1.p2o` decode, a
separate open item.