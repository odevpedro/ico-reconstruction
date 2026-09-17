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
3. Decide the with-assert CI configuration so every existing test validates in
   CI (project-wide, needs user call).
4. Door snap for st05b via the GL pair demo when a display session is available.