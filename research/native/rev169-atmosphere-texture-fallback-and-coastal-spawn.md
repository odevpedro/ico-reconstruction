# Rev.169 — Atmosphere, shared-texture fallback and coastal-room spawn (native-port, P1/P2)

**Date:** 2026-09-10
**Branch:** `native-port`
**Objective:** Close the user-visible gaps from the Rev.168 live demo session
(black sky in st00a/st02a, stock textures missing per-room, and the st02a
coastal room being viewer-only because the spawn probe found no floor at the
p1 bbox centre).

## Scope

- Native-port presentation only (`native/src/main.cpp`, `native/src/engine/ClipBridge.*`,
  `native/tests/clip_bridge_test.cpp`, `native/CMakeLists.txt`).
- No reconstruction claims: everything below is HOST-side rendering/spawn
  behaviour, explicitly separate from byte-verified `.s`/semantic work.

## Sources

- `native/src/main.cpp` — runSceneDemo sky/clear/texture/spawn code paths.
- `native/src/engine/ClipBridge.{h,cpp}` — walkable-grid ground truth for spawning.
- `native/assets/scene/rooms/catalog.json` (40 extracted rooms) + room manifests.
- Runtime logs: `st02a` (5 genuinely-missing textures), the st00a/st17a headless runs.
- Live session feedback (user): "cadê o céu / o mar", "tem muita textura pra
  carregar ainda" (2026-09-10).

## Findings

### Textures

1. `assets/scene/texture/` holds 20 shared stage tiles; each room dir holds its
   own `.tm2` set (st00a 19, st02a 51, st17a 43, st19a 55, st09a 41).
2. In `--room st02a` exactly **5 material textures are genuinely absent on disk**
   and do not exist in any other bundle: `a01_d, a02_d, abe2, abe3, block1`.
   They are baked into the p2o material-name tables but were never extracted
   from `STGST02A.DF`. A textual fallback cannot synthesize them; closing that
   gap requires re-extracting `STGST02A.DF` (future PASSOSS).
3. All other room/stage materials resolve from either the room dir or the
   shared bundle.

### Sky/atmosphere

1. `loadRoomSkyColors()` samples the room's own `sky.tm2` (or a `*sky*.tm2`)
   zenith/horizon and drives the existing `drawSkyGradient()` backdrop.
2. st00a/st02a have **no sky texture** => `hasSky=false` => previous clear
   colour `rgb(10,12,18)` = near-black void for a daylight coast.
3. st17a/st19a DO carry `sky.tm2` (verified: st17a gradient top (140,139,158),
   horizon (203,199,203)).

### st02a spawn failure root cause

1. The `_p1` piece of st02a rasterizes a **tiny clip grid (8x7 cells)** with
   every centre/corner probe landing on cells with `samples==0` (no up-facing
   floor triangle) => `player.spawn()` failed => room was viewer-only.
2. The p1 bbox XZ centre ((−15513, 0)) is inside a courtyard/void, not a floor.

## Changes

### 1. `native/src/engine/ClipBridge.{h,cpp}` — `bestFloorPoint()`

New public query: scans the walkable grid and returns the cell whose floor is
the **highest**, qualified by (a) `samples>0`, (b) not wall-blocked, (c) at
least one walkable 4-neighbour (excludes 1-cell wall tips from being spawn
anchors). Returns false only when no cell qualifies. Documented as Rev.169
spawn fallback.

### 2. `native/src/main.cpp` — spawn fallback (st02a playable)

After `player.spawn(spawnX, spawnZ, …)` fails, re-anchor on
`clip.bestFloorPoint(...)`; the log line distinguishes the anchor
(`[room center]` vs `[highest walkable floor]`).

Result (headless): `st02a player spawn at (750,1075) [highest walkable floor] -> ok pos=(750,45,1075)`.
Previously: `failed pos=(0,0,0)` (viewer-only).

### 3. `native/src/main.cpp` — shared-texture fallback

`texForName()` now resolves each material against (1) the room texture dir,
then (2) the shared `assets/scene/texture/` bundle, and logs each genuinely
missing name once (`room dir + shared scene/texture`). Reduces per-room
"missing texture" noise and fixes stock tiles (st0_a/torch/window…) shared
with scene mode.

### 4. `native/src/main.cpp` — daylight placeholder sky

When the room has no `sky.tm2`, the backdrop now renders a **host daylight
haze** (pale-blue zenith, pale horizon) instead of the near-black clear, and
the frame clear uses the horizon colour. Verified st17a still uses its real
sampled sky (gradient log: top (140,139,158) horizon (203,199,203)).

**Explicitly host presentation, NOT a reconstruction claim**: the original
room could tint its backdrop differently (fog/storm/night). Reversible by
removing the placeholder block in `runSceneDemo`.

### 5. Tests

`native/tests/clip_bridge_test.cpp`:
- `bestFloorPoint()` on the synthetic room returns the elevated tier (fx/fz in
  [100,300], y=100), never the pedestal wall-tip or the wall band.
- Empty bridge: `bestFloorPoint()` fails safely.

## Verification

| Scenario | Result |
|----------|--------|
| `--room st02a --frames 3` | spawn ok via highest-floor fallback; 5 genuinely-missing textures logged once each |
| `--scene pieces --frames 3` (st00a) | regression: `[room center]` spawn unchanged, no missing textures |
| `--room st17a --frames 3` | spawn ok at room centre; real `sky.tm2` gradient loaded |
| CTest | **28/29** (only headless `opengl_backend` SEGFAULT — pre-existing baseline) |
| Build | clean, no warnings from touched files |

## Confirmed / Probable / Possible / Unknown / Discarded

- **Confirmed:** st02a spawn root cause = no floor samples at the p1 centre
  (grid 8x7, all probes `samples==0`); 5 st02a textures absent on disk; st17a
  has a real sky texture; st00a/st02a do not.
- **Probable:** other coastal/outdoor rooms without `sky.tm2` benefit from the
  daylight placeholder the same way.
- **Possible:** `a01_d/a02_d/abe2/abe3/block1` address walls/glass in the st02a
  coastal structures; recovering them needs a `STGST02A.DF` re-extraction pass.
- **Unknown:** the original per-room backdrop tint for rooms without sky.tm2.
- **Discarded:** clamping the fit-macro camera to the playable cluster (Rev.168
  experiment) — the map is large by nature and the user explicitly wants the
  whole coast; reverted before this revision.

## Next minimum test

Reload the st02a demo live (`--room st02a`): sea vista with daylight sky and a
spawned/walkable boy. Optionally re-extract `STGST02A.DF` for the 5 missing
textures.

## Conservative verdict

Rev.169 is a host-side presentation and spawn-completion revision. It does not
change any reconstruction artifact; it makes the already-extracted 40-room
world presentable (sky per room + shared texture resolution) and turns the
previously viewer-only st02a coast into a walkable room. All claims above are
either byte-logged or reproducible via `--room/--scene` headless runs.