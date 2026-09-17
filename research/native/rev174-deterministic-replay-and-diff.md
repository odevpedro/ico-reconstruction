# Rev.174 — deterministic replay machinery + state-diff classifier (PORT, no runtime required)

Date: 2026-09-17
Objective: build all runtime-independent pieces of the deterministic replay
oracle (docs/PROMPT-replay-deterministico-diff-estado.md) so the PCSX2 golden
can later drop into a ready-made harness.
Scope: `native/src/replay/*`, `tools/replay_diff.py`, CTest, CI, static door
catalog note. No PCSX2 instrumentation (that stays runtime-dependent and
user-controlled under `.local/`).

## Deliverables

| Piece | File(s) | Role |
|-------|---------|------|
| Replay format v1 | `native/src/replay/ReplayFile.{h,cpp}` | text grammar `VERSION/ISO_SHA1/SAVE_STATE/WORLD_STATE_INIT/FRAME_REF/F<frame> P<hex8>/E<frame> WORLD <ws> <scene>`; `#` comments; strict-poly fail |
| Fixture | `native/replays/sample-sala-0x0F.replay` | 12 frames, pads F3..F7 = 0x01 (+x), world event E0, synthetic (not from PCSX2) |
| State digest | `native/src/replay/StateDigest.{h,cpp}` | FNV-1a 64 (golden vector `'a'` pinned), canonical little-endian field layout with two format-marker u32s, `formatDigestLine` = `D<frame> <hex16> scene=.. gobj=.. active=.. boy=x,y,z pad=mx,mz` |
| Runner | `native/src/replay/ReplayRunner.{h,cpp}` | headless deterministic execute: IsysGObjRuntime(0x140,0x500) + KanbanSceneLoader(verified tables) + synth ClipBridge room + BoyController spawn, per-frame pad→move→digest; optional `--capture` file |
| CLI | `native/src/main.cpp` (headless `#else`) | `--replay <file> [--capture <out>]`; digest lines to stdout when no capture |
| Diff | `tools/replay_diff.py` | stdlib-only align-by-frame classifier: POSITION_DRIFT / WORLD_STATE_DIVERGE / GOBJ_MISMATCH / ACTIVE_TOGGLE / INPUT_DIVERGE / PROBE_MISSING / UNCLASSIFIED; `--eps`, `--allow-class`, `--allow`, `--selftest` (8 checks) |
| Tests | `native/tests/replay_file_test.cpp`, `state_digest_test.cpp`, `replay_runner_test.cpp` | round-trip + malformed, FNV/digest contract, reproducibility (double capture byte-identical) + movement sensitivity |

## Grammar fixes found by the always-on tests

- `parseFrameAndPayload` rejected single-digit frames (`F0`, 2 chars) under a
  `size >= 3` guard — the fixture sample would not parse. Guard now `size >= 2`.
- World-event lines are produced as one token `E<frame> WORLD <ws> <scene>`
  (matching `ReplayFile::save`); the parser expected `E <frame> ...`. Now parses
  `F<frame-num>` from the keyword directly.

## NDEBUG hazard (root cause of "tests passed without validating")

The three new tests initially used `assert()`, and this build target is
`Release` (`-DNDEBUG`) — every `assert` compiled out, so `replay_file_test`
"passed" while the sample file actually failed to parse (observed via the CLI
error `line 11: bad frame token 'F0'`). The tests were rewritten to an
always-on `CHECK(c)` (fprintf + abort). The project-wide implication stands:
**all other native tests that rely on `assert()` are no-ops under the CI
Release build** (this predates Rev.173; `BoyController`/`verified_scene` etc.
historically validated in Debug/`build-assert` trees). Fixing CI to a
with-assert configuration is a separate, project-wide decision, not made here.

## Verification

- `cmake -S native -B build -DICO_ENABLE_OPENGL=OFF -DCMAKE_BUILD_TYPE=Release`
  → clean; `ctest --test-dir build`: **32/32 passed** (29 baseline + 3 replay).
- CLI: `ico_native --replay native/replays/sample-sala-0x0F.replay` → 12 digest
  lines; double `--capture` byte-identical; `replay_diff` on the identical pair
  → `OK: 12`, exit 0.
- `replay_diff --selftest` → 8/8 PASS.
- Negative end-to-end: flip `F3` pad 0x01→0x02 → classifier reports
  POSITION_DRIFT(boy) on F3..F11, exit 1.
- Digest sanity: `gobj=26` (25 verified scene host GObjs + 1 BoyController GObj),
  boy moves at walk tier (+10.5 x on first pad frame), determinism holds across
  runs.

## Confirmed / probable / unknown

- Confirmed: format grammar self-consistency (parse↔save), digest contract
  (reference LE layout + FNV golden), reproducibility, sensitivity, classifier
  behavior, CI steps run locally.
- Probable: the byte layout is identical between the native runner and a future
  PCSX2-side recorder must reproduce EXACTLY (big-endian would break it); the
  runner's scene/digest semantics are host-side (synth room, verified scene
  tables), not yet compared against the emulator.
- Unknown: PCSX2 determinism of y/position across vblank; epsilon magnitude for
  native-vs-emu position comparison; whether st05b's door snaps (needs the GL
  `--room/--pair` demo or a runtime capture).
- Discarded: door-snap verification in this revision (GL-gated; deferred).

## Door catalog static update (low priority)

Static scan of `native/assets/scene/rooms/*.manifest` for door pieces — 11 rooms
have them (st00a/st02a/st03t/st04b/st05b/st06a/st13b/st13c/st18a/st22a +
st04b-style st05b). **st05b confirmed**: `door_monyo.p2o` + `st05b_p1..p4.p2o` +
4 `umi` (mar) pieces; Rev.172 listed st05b as untested — now catalogued with a
door + real interior p1. Snap verification (door AABB center vs 5-corner
standable floor) still requires the GL demo or the PCSX2 golden. No code change.

## Next minimum steps

1. PCSX2-side recorder producing the same digest grammar (runtime-dependent).
2. First golden replay from the Rev.155-priority scene (sala 0x0F):
   `recorder → .replay file → native --replay → replay_diff gold vs native`.
3. ~~Decide the with-assert CI configuration~~ **DONE (2026-09-17, follow-up).**
   CI now builds headless `Debug` so every test's `assert()` validates on each
   push. Unblocked by fixing the one stale assertion
   (`multi_room_transition_test`: door presence now scanned across the whole
   bundle instead of the first entry) — after the fix the FULL suite passes
   32/32 with asserts ON (Debug). Golden digest verified byte-identical
   Debug==Release, so the replay CI steps are unchanged by the build type.
4. Door snap for st05b via the GL pair demo when a display session is available.

## Follow-up 2026-09-17 — world events are CONSUMED; golden pinned

Same revision, same day: the runner now consumes `E<frame> WORLD <ws> <scene>`
events mid-replay (runtime-independent conclusion of the format contract).

- `ReplayFile::eventAtFrame(data, frame)` added; `estimatedFrameCount` counts
  events too (a pads-gap replay with an event tail is not truncated).
- `ReplayRunner` checks `eventAtFrame` before each frame: on hit it runs
  `requestScene(sceneId) + execute()` (real resident-set swap), mirrors
  `worldState` into BoyController, and re-grounds the boy via the Rev.170
  contract (`setBridge` + walkable `spawn` at the current x/z — no teleport).
- Fixture `native/replays/sample-sala-0x0F-to-0x2B.replay`: pads F3..F7 drive
  +x and a single `E5 WORLD 0x2B 0x2B` swaps the resident scene on frame 5.
- Verified CLI timeline (12 frames): `scene=15 gobj=26` on F0..F4 →
  `scene=43 gobj=24` on F5..F11. `gobj 26→24` is exactly the st02a resident
  set shrinking (25→23 host GObjs + the boy; Rev.159/170 numbers). Position is
  continuous across the swap frame. Determinism holds (two runs identical).
- `native/replays/sample-sala-0x0F.digest.expected`: committed 12-line digest
  golden for the base fixture, compared in CI against fresh `--replay` stdout
  (pins the digest contract across machines/compilers; regenerate deliberately,
  never silently).
- CI reproducibility step now covers both fixtures (single- and multi-room);
  new golden-diff step compares stdout against the pinned file.

Corrections from this pass:
- **Pad-magnitude doc drift.** The grammar comment said "magnitude 25 (walk
  tier)"; the real constant is `BoyController::kWalkSpeed = 15.0f`. Comments in
  `ReplayFile.h` and both fixtures fixed to 15. Digest `pad=` fields and
  position deltas were always correct (15-driven), only the prose was wrong.
- **NDEBUG hazard, closed:** the with-assert (Debug) run of the FULL suite
  surfaced `multi_room_transition_test.cpp:75` aborting — its assertion
  `firstA->meshPath.find("door") != npos || firstB->meshPath.find("door") != npos`
  no longer held once role-based attachment ordering (Rev.159/171) stopped
  guaranteeing the FIRST bundle entry is the door. Fixed by scanning the whole
  bundle for a door piece (intent preserved, order-independent). After the fix
  the entire suite is healthy under asserts: **32/32 Debug (asserts ON)** and
  **32/32 Release**. CI now builds `Debug` so the whole suite validates on
  every push instead of under NDEBUG. The three replay tests are always-on
  CHECK regardless.