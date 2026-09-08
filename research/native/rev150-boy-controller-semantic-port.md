# Rev.150 — Fase C: semantic `BoyController` (port of boy_hA/hB/hC)

**Date:** 2026-09-08
**Branch:** `native-port`
**Trilha:** `[TRILHA: PORT]`
**Scope:** close "Lógica real do BOY" item from `instrucoes-agente-input-colisao-boy.md`
(Phase C step 1-2), replacing the Phase-A placeholder with a semantic port of the
verified PS2 BOY state machine.

## Objective

Swap the generic placeholder (`PlayerController`, Rev.149) for a controller whose
movement logic mirrors what the PS2 BOY handlers encode in
`src/entity/boy.c`:
`hA` (0x1C1F58), `hB` (0x1C1DD8), `hC` (0x1C1A98). The native class must consume
the Phase-A WASD input vector in the GObj ProcessNode callback (instead of a PS2
control read) and resolve motion through the existing `ClipBridge` (Phase B).

## Sources used

- `src/entity/boy.c` — decompiled `boy_hA/hB/hC`, `boy_work` stride 0x4C,
  `sub_14A0D8` walk/run discriminator call in `boy_hB`.
- USA ELF `.local/extracted/SCUS_971.13.elf` via `tools/disasm_local_range.py`.
- `research/elf/ghidra-rev078-boy-girl-vtable-vblank-correction.md` — damping
  factor 0.7f confirmed ("Tail-call movement solver with computed speed and
  damping (0.7f)").
- `research/elf/ghidra-rev060-consolidated-hb-analysis.md` — hB movement/collision
  chain: `0x103D50(entity, ..., speed, spread, damping)`, speed from
  `0x14A0D8`, rest event via `0x13FF88`.
- `research/native/rev148-playable-gap-analysis.md` — Fase C plan (semantic port
  consuming input vector, not byte-exact).
- `research/native/rev149-walkable-character-clipbridge-real-mesh.md` — baseline
  (ClipBridge two-pass walkable-room model, `PlayerController` placeholder).

## Evidence used

### hB speed tier (walk 15 / run 30)

`boy_hB` calls `sub_14A0D8()` then tail-calls the movement solver:

```asm
0x...    jal sub_14A0D8
0x...    lui $at, 0x41F0   ; 30.0f (upper)      <- run tier
0x...    mtc1 $at, $f12
         beq $v0, $zero, ...
         lui $at, 0x4170   ; 15.0f (upper)      <- walk tier
         mtc1 $at, $f12
0x...    j  sub_103D50     ; tail: move solver  (a1=2, f13=50.0, f14=damping)
```

Confirmed by Rev.091f normalization note (15.0f / 30.0f li.s expansion in hB).

`sub_14A0D8` (0x14A0D8, disassembled this session):
- `lw $v0, -0x6e08($gp)` (ICO entity global `ENTITY_ICO_PTR`);
- zero → return 0 (run tier);
- else `ld $v0, 0x18($v1)` with `$v1 = entity+0x164`,
  `dsrl32 $v0,$v0,4` then `andi $v0,$v0,1` → **bit 36** of the 64-bit flag word
  at `ICO_entity+0x17C`. Set → walking (15.0), clear → running (30.0).

### Movement solver algebra

`sub_103D50` (0x103D50) is a candidate-sweep shell (iterates the entity list at
`gp-0x6C0` with count `gp-0x6DB4`, filtering by the `a1` type) that calls the
per-candidate math `0x1034B8`. In `0x1034B8` the request is integrated as a
damped approach:

```asm
0x1035F8  jal   0x117C20      ; separation computation
0x103600  sub.s $f20, $f23, $f0   ; f20 = target_speed - separation
...
0x103610  mul.s $f12, $f20, $f22  ; f12 = (target-speed delta) * damping
```

i.e. `move_amount = (speed - separation) * damping`. With `damping = 0.7`
(FLOAT_GPCONST gp-0x771C read by `boy_hB`), a full-speed request lands at
`speed * 0.7` after the first frame and converges geometrically. The native port
mirrors this with a velocity blend `vel += (target - vel) * 0.7`.

### hA two-path + interaction convergence

`boy_hA`:
- `wk->active` selects Path A (anim+physics, `sub_104508`/`sub_102850`/
  `sub_12A060` + anim_time store when `world_state_raw==0`) vs Path B
  (transform-only idle: `boy_dispCrown`, `sub_1C1250`).
- Both paths converge: if `GAMEPLAY_FLAG (gp-0x6F60) == 0x27` and
  `scene_obj+0x644 < 20.0f` and `scene_obj+0x648 != 0` and `!sub_10D180(entity)`
  → interaction callback (`sub_10CB48`/`sub_10CF88`).

### hC defaults

`boy_hC` allocates the 0x4C work area and initializes:
- `active=0`, `anim_time=0`, `config_A/config_B=20`, `range_A/range_B=300.0f`,
  `flag_mask=0x80808080`.
- 3 children (type 1 root, 0xB hitbox, 0xC shadow) — animation/model system,
  **not ported** (no native model of BOY yet, mesh not extracted).

## Implementation

`native/src/game/BoyController.{h,cpp}` replaces `PlayerController`. It keeps the
exact same GObj/ProcessNode seam (isysGObj runtime, list 1, typeMask 1, prio 0)
and ClipBridge resolution, so the demo wiring change is a rename + comment update.

Semantic mapping:

| PS2 handler | Native BoyController |
|-------------|----------------------|
| `hC` defaults (active=0, config/range/mask) | `BoyWork` mirror fields + `kDefault*` constants; verified by test |
| `hB` walk/run discriminator | |input| vs `kRunThreshold` (30.0): single key (25) → `kWalkSpeed` 15.0; W+D diagonal (≈35.4) → `kRunSpeed` 30.0 |
| `hB` damped solver `(speed-sep)*damping` | `vel += (target - vel) * kDamping` (0.7), then `ClipBridge::move` |
| `hB` rest detection → `sub_13FF88` | active drops to Path B when vel≈0 (residual damping decays ~0.3/frame); no PS2 event emission |
| `hA` Path A active / Path B idle | `wantsActive` from move intent + velocity; `animTime` advances on Path A |
| `hA` world_state 0x27 gate + <20.0 + id + !busy | `worldState()/setDistanceSensor()/setInteractionId()/setBusy()` + `setInteractionHandler()` stub (no Girl entity on native) |

Constants lifted verbatim: `kWalkSpeed=15`, `kRunSpeed=30`, `kDamping=0.7`,
`kInteractionRange=20`, `kWorldStateGameplay=0x27`, hC defaults
`config=20`, `range=300`, `flagMask=0x80808080`.

## Tests

`tests/boy_controller_test.cpp` (renamed from `player_controller_test`) covers:

1. hC defaults: inactive, anim_time 0, config/range/flag_mask == PS2 constants.
2. Walk tier: `setMove(25,0)` → x after 1 frame ≈ `15*0.7` (10.5), second
   frame still < 30 and growing (damping approach).
3. Run tier: `setMove(40,0)` → x ≈ `30*0.7` (21.0) after 1 frame.
4. Damping decay to idle: no-input updates blend velocity toward 0, Path B
   reached within 40 frames, further update moves nothing (kEps).
5. Wall clamp: `setMove(600,0)` against x=200 wall keeps x<200 (ClipBridge).
6. Interaction gate: fired once with id when ws==0x27, sensor<20, id!=0,
   !busy; NOT when ws=0x10, sensor=80, or busy=true.
7. Spawn reject + shutdown semantics (from Rev.149 test).

## Build/test evidence

- `native/build-test` + `ICO_ENABLE_OPENGL=OFF`: `ctest` **25/25 PASS**.
- `native/build`: `ico_native` 600 frames on real p1 mesh
  (`--p2o assets/170_st00a_p1.p2o`): spawn `(-50,100,1000)` ok, grid 36x41
  blocked=483, stable 2 draw calls/15185 tris.
- `scripts/pre-push-check.sh` (Debug, asserts enabled): **PASS 25/25**.

## Confirmed

- hB speed tiers 15.0/30.0 selected by `sub_14A0D8` (bit 36 of 64-bit flags at
  ICO entity +0x17C).
- Damping 0.7f in the `(speed-sep)*damping` solver algebra.
- hA two-path active/idle + world_state 0x27 interaction convergence.
- hC work-area defaults.
- Native gate PASS with the semantic bridge in Position.

## Probable

- That the PS2 analog input maps stick magnitude to the walk/run discriminator
  bit (sub_14A0D8 reads an entity flag, not the analog value directly; the
  mapping for the native input tier is a semantic equivalent, documented in the
  header).

## Possible (not yet wired)

- The rest event (`sub_13FF88` arg 6) is detected in spirit (active→idle
  transition) but not emitted as a sound/animation event; BOY has no native
  model.
- The interaction handler is a stub seam; no Girl entity/free-play interaction
  exists on native yet.

## Unknown / blocked

- BOY mesh not extracted → placeholder box still renders movement (Phase C step 2:
  "manter o placeholder visualmente, mas com a lógica de movimento real por trás").
- PCSX2 session comparison for boy_hA/B/C state sequences (Phase C step 3,
  report item) requires instrumented probes at 0x1C1A98/0x1C1DD8/0x1C1F58 in a
  future gameplay session.

## Discarded

- Byte-exact port of `sub_103D50`/`sub_1034B8` (candidate sweep + separation
  math, dependent on PS2 entity list/allocator) — out of scope for the semantic
  bridge; the algebra is replicated, not the implementation.

## Next minimum test

- Long demo run with directional changes (walk↔run transitions) monitoring
  damping approach and wall slide; then a PCSX2 capture with probes on
  `boy_hA/hB/hC` for state-sequence comparison.

## Conservative verdict

Phase C steps 1-2 done at the semantic-bridge level: movement logic now follows
the verified PS2 BOY handlers (speed tier, damping, two-path, interaction gate)
instead of a hardcoded step vector. Not byte-exact, no PCSX2 comparison yet —
owed on a future runtime session.