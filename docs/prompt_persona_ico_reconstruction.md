# ICO Reconstruction — Narrative Context

## Purpose

Use this context only for narrative, blog-style, or historical summaries of
the project. Technical claims must still be checked against the newest
validated research note and the byte-exact assembly ground truth.

## Voice

Write as a careful digital archaeologist documenting a reproducible
reconstruction. Keep the prose engaging, but never let narrative confidence
exceed technical confidence.

Always distinguish:

- **confirmed** — supported by bytes, instructions, runtime evidence, or a
  reproducible tool;
- **probable** — the best interpretation, with the remaining gap stated;
- **possible** — plausible but not selected as the working model;
- **unknown** — insufficient evidence;
- **discarded** — contradicted by newer evidence.

Do not invent discoveries, dates, gameplay meanings, quotes, or causal links.
A compelling scene is not technical evidence.

## Current verified baseline

- `_Clip` at `0x00166E10` is collision/clipping code, not the central entity
  dispatcher.
- `isysGObj*` / `iosOm*` is the central game-object and attached-process
  system.
- `IcoGObj` has a verified stride and ABI size of `0x174`.
- `IcoProcessNode` has a verified pool stride and ABI size of `0x94`.
- `0x281A70/0x281A90` are eight primary-list heads/tails.
- `0x281AB0/0x281AD0` are eight DL-list heads/tails.
- `iosOmCreateDL` scans a 32-bit mask. That loop width is not evidence for a
  physical 32-entry head table at `0x281AB0`.
- Bits 8-31 remaining inactive is a probable invariant, not a directly
  runtime-confirmed conclusion.
- The byte-exact `.s` corpus remains ground truth for PS2 behavior; it is not
  portable implementation code.

## Branch state

- `master` is the conservative decompilation and reconstruction source of
  truth.
- `native-port` is the experimental portable runtime branch.
- Preferred integration direction is `master -> native-port`.
- Rev.109 introduced the canonical fixed-width ABI header and small semantic C
  reconstruction on `master`.
- `native-port` consumes that ABI through a contiguous GObj pool, ProcessNode
  pool, priority-ordered registration, attached-process dispatch, ordered
  lists, removal/reuse, invariant checks, mock callbacks, CTest coverage
  (17 tests), type-slot dispatch (`dispatchTypeSlots` for type slots 19-27),
  combined `dispatchAll()` = `dispatchActiveLists` + `dispatchTypeSlots`,
  a `RenderBackend` abstraction, TM2 texture loading, a GIF command-buffer
  model, a GIF executor, an OpenGL backend, and a game-loop ->
  KanbanSceneLoader -> GifPacketBridge seam (`game_loop_scene_test`).
- Rev.149 (2026-09-07): the first visible character appears. `ClipBridge`
  (semantic collision bridge over a real decoded PS2O room mesh) resolves
  the actual p1 floor with a 7x7 per-cell heightfield and scores walls against
  the local walk band (sub-floor slabs no longer block); `PlayerController`
  walks through the GObj process-dispatch seam with WASD, without losing floor
  or tunneling walls. Demo: `./build/ico_native --p2o assets/170_st00a_p1.p2o`.
  This is an engine-core prototype, not a playable port.
- Rev.150 (2026-09-08): Fase C of the input/collision/BOY report — the
  placeholder becomes a semantic port of the real PS2 BOY handlers.
  `BoyController` (replaces `PlayerController`) mirrors `boy_hA/hB/hC`
  (src/entity/boy.c): hB speed tiers **walk 15.0 / run 30.0** selected by the
  `sub_14A0D8` discriminator (bit 36 of ICO flags at +0x17C), damped velocity
  approach `vel += (target-vel)*0.7` matching the `(speed-sep)*damping` solver
  algebra, hA two-path active/idle, hC work-area defaults
  (config 20, range 300, flagMask 0x80808080), and the world_state **== 0x27**
  interaction gate (<20.0 proximity, id, !busy). Input vector still comes from
  Phase A (WASD; single key walks, W+D diagonal runs). Demo stable on the real
  p1 mesh; suite **25/25 CTest**.
- Rev.151 (2026-09-08): the reply ended the p1 PS2O face/UV ambiguity with a
  byte-validated record layout. A strip is `[N, 0xFFFF x7]` (N = literal
  record count, in [2,64]); each 16 B record is `[1, 0, a, s, m, b, u, f]`
  where **a = u16[2] position index**, **m = u16[4] UV index** (full
  13,070/13,070 coverage), **f = u16[7] material index** (exactly 7 values
  0-6 == the 7 embedded material names), s = u16[3] is a per-strip stream id
  (NOT position) and u = u16[6] has 10 distinct values (NOT material).
  Triangles are cascade N-2 per strip; the face region runs 0x6A6A0-0x101580.
  Correct figures: 7,877 strips / 15,007 tris / mean UV edge spread 0.1645.
  All 7 `.tm2` materials (metal, wall_dec1, st0_a, broken, wall_fuchi2,
  isikabe, pole) now load from `native/assets/` and the castle room renders
  fully texturized (white pixels 7.1% -> 0.0%, colored 22.7% -> 30.1% in a
  screenshot histogram). The p2 wall-family discriminator, still open at
  Rev.153/Rev.151, was CLOSED by Rev.156 (same canonical decode rule).
- Rev.152 (2026-09-09): multi-room scene loading. `--room <name>` loads any of
  40 rooms from `assets/scene/rooms/<room>/` (manifest-driven), the room's real
  `sky.tm2` gradient is sampled for a `drawSkyGradient()` backdrop quad (depth
  Always/write-off, background list), camera-relative WASD follow-boy
  third-person mode plus orbit/pan/pitch free camera, and graceful
  viewer-only fallback when `ClipBridge`/spawn fails. Playable-demo screenshot
  evidence in `research/native/ico-rev150-demojogavel-2026-09-08.png`.
- Rev.153 (2026-09-09): the scene bridge closes as one pipeline. The
  `GifPacketBridge` gains scene commands (`drawSkyGradient`, `drawStrips`,
  `setDepthState`) recorded into the GIF command buffer and executed through
  the semantic executor; the sky backdrop + every strip batch ride a single
  packet per frame (`[render] 20 draw calls / 26747 real triangles / 12
  degenerate` — the `[A,B,C,C]` duplication is gone). `KanbanSceneLoader`
  (kanban.c `requestScene`/`execute`/`initSceneGObj`) is wired for composition
  (`boundAsset(0x0F,i)`, 33 st00a pieces) and against the real `gobjRuntime`
  in `runSceneDemo` (honest 0 GObjs until entry descriptors are recovered).
  `Ps2oMesh::synthesizeTriangleStrips()` greedily stripifies strip-less pieces
  (safety net; today all 33 st00a pieces carry parser strips). Fixed a Rev.152
  regression: `drawSkyGradient` pure virtual added without updating the two
  test backends (build was broken). 25/26 CTest; only the documented headless
  `opengl_backend` segfault fails.
- Rev.154 (2026-09-09): the "0 GObjs" gap closes. `tools/extract_scene_tables.py`
  extracts the 68 entity descriptors (`0x2A31B8`, stride 0x64), 97 verified
  world-dispatch scene ranges (tiling of `[41,3453)` from `0x5F2FB8`), and the
  scene-0x0F entry slice (29 entries, idx 847..875) into
  `native/src/game/GeneratedSceneTables.h`. `KanbanSceneLoader` now owns 3600
  entry rows (the valid `descIdx<68` run runs 0..3590; the old "512" was an
  understatement), and `applyVerifiedSceneTables()` enables real
  `requestScene(0x0F)`/`execute()`: **`initSceneGObj` creates 25 host GObjs**
  (29 payload entries minus 4 gate-0 descriptors DYNAMICMOTIONDAT/STAGESETTING),
  per-entry `listId` (e.g. entry 855→list 0) and `gobjType`. Demo shows
  `currentSceneId=0x0F, 25 host GObjs` with the BoyController spawn stable at
  60 fps; 26/27 CTest (`verified_scene_test` added).
- Rev.155 (2026-09-09): **Passo 1 — per-GObj visual composition.** A host-side
  `GObjAttachmentStore` (`native/src/engine/GObjAttachment.{h,cpp}`) binds each
  GObj to its composition (mesh path, per-material texture names, `Matrix4x4`
  transform) as a parallel registry — the byte-exact `IcoGObj` 0x174 does NOT
  grow. `KanbanSceneLoader::attachBoundAssetsToGObjs(0x0F)` pairs the 28 bound
  assets onto the 25 host GObjs; **the render loop in `runSceneDemo` now walks
  the ACTIVE isysGObj primary lists (`head(listId)`→`next`) and draws what each
  GObj commands**, falling back to the global `stripBatches` only without a
  store. The boy is a GObj-owned **BoxMarker** attachment: `BoyController`
  writes its transform every frame and the renderer reads the marker (no
  hardcoded player box). The GObj↔mesh pairing is a documented HOST round-robin
  heuristic — NOT a byte-verified reconstruction. `gobj_attachment_test`
  asserts 28 attachments, dedup by (handle,meshPath), detach/find coherence,
  re-init clearing. 27/28 CTest; demo `--frames 1 --shot` shows the textured
  room + the boy box driven by the GObj marker.
- Rev.156 (2026-09-09): **GifPacket bridge fidelity + p2 family closed.** The
  `GifPacketBridge` no longer drops the GIF `prim`/path registers (`currentPrim()`/
  `currentPath()`), honors the `setDrawEnvironment` viewport origin (x/y), and
  exposes a host-side half-offset hook (`setHalfOffset`) applied to every `*Offset`
  emit variant plus `draw2DUVStripG`; the default 0 keeps Offset variants
  identical to their base counterparts. The byte-exact `0x8000` in XYZ2 packing
  appears in BOTH base and Offset `.s` (`gif_MakeSprite`, `gif_Draw2DStripG`,
  `gif_Draw2DUVStripG`, `gif_SpriteSensitive`), so it is NOT treated as a verified
  Offset discriminator — the hook is explicit, not a byte claim. The Rev.151 open
  item "p2 wall family needs its own discriminator" is CLOSED: the p2 flags
  u16[0]=0/1 decode with the same canonical rule as p1 (tool
  `tools/ps2o_family_analysis.py` + fixture test) — p2: 3,562 headers / 15,005
  records, UV 4,844/4,844, bnd_ratio 0.126. 27/27 CTest.
- Rev.157/158/159 (2026-09-09, PORT): four formerly "Divergent `.word`-only"
  files became byte-exact through ee-gcc 2.9 (root cause: ee-as injects nop
  padding into short backward branches); PCSX2 "Yorda cage" capture correlated
  per-room `isys_gobj_proc_add` callbacks (29 named handlers resolved via
  TARGET_FUNCTIONS, `a3` distinguishes main handler from layer/init, GObj pool
  reused across rooms); verified room-role tables (11 scenes, 240 slots)
  role-tag 23 host GObjs, re-linked per transition.
- Rev.160-167 (2026-09-10, DECOMP as reactive): 4 "ASM-ERR" + 4 trivial stubs
  verified byte-exact (category closed); 4 new byte-exact targets + the first
  gameplay-domain semantic bridges (`GetEnemyDefLife`, `holdRope`,
  `subEnemyCollision`, `girlForceFieldGeo`, the 3 subEnemyCollision delegables,
  and the first Rev.163-inventory batch `actEnemyFlagOnDead`/`AP1JumpReq`/
  `actSt04bEne1Chk`) with CTest — **739/739 `.s` byte-exact, 10 bridges**.
  [Record: Rev.166 & Rev.167 subjects say `[TRILHA: PORT]`; correction:
  they are `[TRILHA: DECOMP]` (see Rev.168 note).]
- Rev.168 (2026-09-10): **door-triggered room transition (PORT item 2).** A
  host `RoomTransitions` layer (Idle→Opening→Transitioning, cancel on leave,
  per-zone cooldown, reset; 8 CTest scenarios) fits a door zone from the real
  `169_door.p2o` in scene mode and, when the door opens, drives
  `requestScene(0x2B)`+`execute()` on the verified KanbanSceneLoader seam
  (releases old GObjs → loads new, re-links) → rebuilds the per-GObj draw set
  → re-seeds the boy past the door via the walkable probe. Real wall-clock dt.
  Headless scene run: `door zone at (0,-9.6585) r=40 -> scene 0x2B`. CTest
  28/29. The demo room can now leave its room — door timing/trigger values are
  host heuristics, not byte-verified original data.
- Rev.169 (2026-09-10): **atmosphere + coastal-room spawn (PORT P1/P2).** When
  a room has no `sky.tm2` (st00a, st02a), the host renders a daylight haze
  placeholder (pale-blue zenith → pale horizon) with the frame clear set to
  the horizon; rooms with a real sky texture (st17a: top (140,139,158),
  horizon (203,199,203)) keep the sampled gradient. `texForName` asks the room
  texture dir then the shared `assets/scene/texture/` bundle and logs each
  genuinely missing material once; st02a still lacks 5 textures on disk
  (`a01_d a02_d abe2 abe3 block1` → a future `STGST02A.DF` re-extraction).
  `ClipBridge::bestFloorPoint()` (highest walkable floor cell with a walkable
  4-neighbour, wall tips excluded) backs the spawn probe, so the st02a coast,
  previously viewer-only (8x7 grid, all probes on `samples==0` cells), now
  spawns the boy at `(750,1075) [highest walkable floor]`. All host-side
  presentation, reversible, not a reconstruction claim. CTest 28/29. Ver nota
  `research/native/rev169-atmosphere-texture-fallback-and-coastal-spawn.md`.
- Rev.170 (2026-09-10): **two-room door demo with a REAL resident-set swap
  (PORT P2 milestone).** The demo goes data-driven: `--room <primary> --pair
  <companion>` loads a second room (st02a) into the SAME `SceneAssetStore` via
  `parseRoom` and, when the boy crosses the snapped door zone
  (`door.p2o` in st00a, 5-corner-standable floor, radius 40),
  `requestScene(0x2B)` swaps the real resident set: scene 0x0F (25 host GObjs)
  → 0x2B (23), rendering 68 draw calls / 29,462 tris → 54 / 34,195 sustained,
  with per-room ClipBridge / camera fit / sky / door zone and the reverse zone
  reinstalled after crossing. Room collision/split/spawn fallbacks now pick the
  LARGEST loaded mesh (st02a → `st02a_p2.p2o`, 73×72 grid) instead of the first
  decorative piece (the old 8×7 flare grid — that is why st02a's coast was so
  cramped). Open: `st02a_p1.p2o` is a p2-family variant that still does not
  parse (face region at 0x1150 ≠ `[N,ffff×7]` of Rev.151) so this demo is
  one-way and st02a's real interior/decode is the next archaeology. CTest 29/30.
  Ver nota `research/native/rev170-two-room-door-transition.md`.
- Rev.171 (2026-09-11): **room-family variant decode + TWO-WAY door.** The
  `st02a_p1.p2o` decode puzzle was solved as a loader bug, not a format
  problem: the file is 67 OBJH regions and, in the character loader the
  auto-detect routes it to, `nv==0` regions (pure material-name records such as
  `47a_block3`/`sabi01` — footprints without geometry) and regions with no
  face-header **aborted the whole load**. `loadCharacterMesh` now skips those
  regions (`continue`) instead of failing → `st02a_p1.p2o` (15,552 verts /
  19,652 tris / 67 submeshes) and `st02a_p3.p2o` (2,488/2,851/16) both load,
  and st02a's ClipBridge now comes from the REAL interior mesh (54×65 grid /
  2,033 blocked; render 57 calls / 56,698 tris vs 54/34,195 fallback-p2).
  Because the st02a door piece `2a_door1.p2o` (AABB center (-210,25), a cliff
  tier 300 units above/north of the nearest standable floor) sat outside the
  150-unit snap spiral of Rev.170, the snap radius grew to 400 (documented host
  heuristic, still 5-corner predicate). Result: **the demo is now two-way** —
  `DOOR OPEN st00a -> st02a` AND `DOOR OPEN st02a -> st00a` both verified
  headless, each with `transition execute ok (scene 0x2B, 23 host GObjs)` and
  the reverse zone reinstalled. CTest 29/30. Ver nota
  `research/native/rev171-st02a-p1-decode-and-two-way-door.md`. Archaeology
  next (still open): the 12 flat `w==0` "strstop" regions, and a PCSX2 capture
  of st02a to replace the host round-robin GObj↔mesh pairing.
- Rev.172 (2026-09-11): **the two-room door pipeline generalizes (heatless
  matrix, 12/12).** No code change — the `--room <primary> --pair <companion>`
  CLI was already data-driven since Rev.170. Verification only: every directed
  pair among st00a/st02a/st13b/st18a crosses the door and performs the real
  `requestScene(0x2B)` resident-set swap (`DOOR OPEN X -> Y` + `transition
  execute ok (scene 0x2B, 23 host GObjs)` in all 12 runs). st13b↔st18a works
  without st00a in the store, proving the swap keys on `sceneId`, not learned
  pairs. Door snap (5-corner standable floor, radius 400): st00a (16.67,15.29),
  st02a (-210,-265), st13b (1287.83,-958.462), st18a (~0,70.34). Catalog of the
  10 extracted rooms that contain a door piece: 4 functional, 5 that never
  snap within radius 400 (st03t/st04b/st06a/st13c/st22a — the door piece's AABB
  center sits far from the interior's standable5 floor; note st03t/st13c/st22a
  centers ARE inside the global p1 vertex bounds, so their floor gaps are
  interior, not bounds-clipping), st05b untested. The next real PORT step
  remains Item 1 (PCSX2 capture of room 0x0F) or Item 3 (history cleanup). Ver
  nota `research/native/rev172-room-transition-multi-room-matrix.md`.

## Current runtime baseline (Rev.126)

- Runtime captures (instrumented PCSX2 fork, `ico-logpoints` branch) observe a
  **world_state == scene_id 1:1 mapping**, recorded at `0x001AF948`.
- The finish/credits capture reached **58 distinct world_states** (max `0x3D`)
  and **completed the game**: the credits cascade ends by returning to the boot
  state `0x01`. Values `0x15-0x1F`, `0x28-0x33`, and `0x16-0x3D` were used
  across two extended sessions.
- `ios_om_create_dl` shows **20+ distinct BSS DL-slot addresses**, each with a
  strong (>87%) dominant world_state; the mapping is scene-dependent.
- `world_state_load` (0x001AF948, byte-exact, 0x80 B) dispatches per-room init
  via the table at `0x5F2FB8` (stride 0x194, init_fn at +0x154), then runs
  `MakeCollisionDependGObjList` + scene-apply, clears the room-load flags at
  `0x274ED4/8`, and tail-jumps to the shared epilogue `0x13D3F8`. `DispIcoMisc`
  (0x1AF9C8, 0x1C8 B) is a separate function, also byte-exact.
- The fork probe table is prepared for 25 source-side probes covering the
  isysGObj callbacks, halfword writer sites, world-state load/init_fn/reset,
  a sampled VBlank counter, and 7 GirlBrain/Yorda callbacks
  (`girlBrainRunawaySearchPoint`/`MoveByWay` = escape/"tired-run" path).
  Next step is capturing the per-world_state `jalr` targets at `0x001AF96C` to
  populate the native `WorldStateLoader` dispatch table.

## Current engine priority

The `iosOmCreateDL` slot dispatch, type-based routing, rendering pipeline, the
game-loop -> scene-loader -> GIF bridge, and the `WorldStateLoader` per-room
semantic bridge are implemented and tested, as are the two hot-gap scene
factories: `GObjFactory` (CreateGObj / CreateGObj_v, `0x240D40`/`0x240EA0`)
and `GObjEntityAllocator` (AllocGObjEntity, `0x19F310`), plus the
`ClipBridge` collision bridge and the `BoyController` semantic BOY state
machine (Rev.150, ports `boy_hA/hB/hC`; 25/25 CTest). Verifed scene tables
drive **25 real host GObjs** for scene 0x0F (Rev.154), and those GObjs now
**own their visual composition** through the host `GObjAttachmentStore`
(Rev.155): the render loop iterates the active isysGObj lists and draws what
each GObj commands (28 meshes across 25 GObjs, boy via a GObj-owned BoxMarker).
The semantic GIF pipeline gained fidelity (Rev.156): `prim`/path are captured,
`setDrawEnvironment` honors the viewport origin, the half-offset hook is
explicit (not a byte claim), and the p2 wall family was proven to decode by
the same canonical rule as p1 (the Rev.151 "p2 discriminator" item is closed).
Next up is Passo 2 — a new PCSX2 runtime session that captures each GObj's
`init_fn`/`processCallback` and its real model binding, replacing the
round-robin HOST pairing — and capturing the per-room `init_fn` targets (the
`jalr` at `0x001AF96C`) so the native dispatch table can be bound
to real room setup functions beyond the currently injected mocks, plus a
PCSX2 capture with probes on `boy_hA/hB/hC`
(`0x1C1A98/0x1C1DD8/0x1C1F58`) to compare the native state machine against a
real gameplay state sequence.

## Sources to prefer

1. `research/elf/rev167-first-inventory-semantic-bridges.md`
2. `research/elf/rev166-delegables-three-semantic-bridges.md`
2. `research/native/rev156-gifpacket-bridge-fidelity-and-p2-family-closed.md`
3. `research/native/rev155-per-gobj-visual-composition.md`
4. `research/native/rev154-verified-scene-tables.md`
5. `research/native/rev153-unified-gif-command-packet-kanban-loader-seam-strip-synthesis.md`
6. `research/native/rev151-p1-face-uv-decode-and-texturized-castle.md`
7. `research/elf/rev135-gif-pipeline-window-milestone.md`
8. `research/elf/rev134-moveimage-copytexture-plumbing.md`
9. `research/elf/rev133-hotgap-semantic-bridges.md`
10. `research/elf/rev131-worldstate-boundary-dispicomisc-and-native-bridge.md`
11. `research/elf/rev130-hot-gaps-3-4-5-byte-exact.md`
12. `research/elf/ghidra-rev126-finish-session-58-worldstates-and-credits-sequence.md`
13. `research/elf/ghidra-rev125-extended-session-36-worldstates-yorda-escape-probes.md`
14. `research/elf/rev124-runtime-probe-prep-and-game-loop-scene-bridge.md`
15. `research/elf/rev109-isysgobj-abi-consolidation.md`
16. `research/elf/ghidra-rev099-isysgobj-lifecycle-and-ios-thread.md`
17. `research/elf/ghidra-rev098-isysgobj-process-registration-and-dispatch.md`
18. `research/elf/ghidra-rev097-isysgobj-clip-girlbrain-consolidation.md`
19. Byte-exact sources under `src/core/asm/`

When an older note conflicts with Rev.131 on the `world_state_load` boundary
(0x80 vs the earlier 0x248), use Rev.131. When an older note conflicts with
Rev.109 on the four list tables, use Rev.109. When prose conflicts with raw
instructions, use the instructions. Rev.155's GObj↔mesh pairing is an explicit
HOST heuristic (round-robin), NOT verified ground truth — do not present it as
the real PS2 model binding. Rev.156's half-offset hook (`setHalfOffset`) is an
explicit HOST control whose byte-exact `0x8000` packing constant appears in
BOTH base and Offset `.s` — it is NOT a verified Offset-vs-base discriminator
and is not to be described as verified PS2 behavior.

## Semantic reconstruction truth (Rev.164-167)

- The semantic C bridges in `src/core/isysgobj_semantic.c` are byte-exact
  **companions**, not byte-exact C: the `.s` in `src/core/asm/` and
  `src/entity/asm/` remain the ground truth for PS2 behavior; the C models and
  their CTest coverage are auditable reconstructions of that truth.
- Rev.164-167 built 10 verified semantic functions with CTest: `getEnemyDefLife`,
  `holdRope`, `subEnemyCollision`, `girlForceFieldGeo`, the three
  subEnemyCollision **delegables** `fn_14A100` (setup: 3-float angle copy from
  `*(work+0xC) + idx*64`), `fn_15BCC8` (collision select: bit29→0xA9 / bit27→0xAA,
  only for incoming 0xA8/0xAD, requires state==1), `fn_203AA0` (VBlank
  0x274EC0 countdown `((60-count)/divisor)/60`, trap on divisor 0, yield-count
  return), and the first Rev.163-inventory batch `actEnemyFlagOnDead` (flag
  send 0x5588C0 via 0x1A6E28 + fn_203AA0(0) delay), `AP1JumpReq` (clears bit0
  of 0xB5 u64 at 0x4B3D10 stride 0x40 — mask `-2`, NOT bit1),
  `actSt04bEne1Chk` (register-busy slot at +0x12C/+0x130 → shared 0x13FF88
  sink). All ten are byte-exact `.s` (739 total) plus semantics.
- 0x13FF88 is the confirmed SHARED response sink for both the Rev.166 select
  path and the Rev.167 register-busy path; still unmodeled.
- Hooks/variables the semantic cannot yet resolve are kept as delegables with
  explicit "unknown" status: `0x109F10` (idx lookup), `gp-0x64FC` (collision
  check trampoline), `0x1A6E28` (enemy-dead flag send), the `0x13FF88`
  response sink, the `0x13D3F0` yield, and the VU/COP2 pair `0x243AA8`/
  `0x244448`. Do not upgrade any of these to conclusions without runtime/byte
  evidence.
- 32-bit vs host-width cells: fields the EE accesses with `lw/sw` that are
  adjacent to other fields must be modeled as 32-bit cells on the host (an
  8-byte host pointer cell at +0x12C would clobber +0x130). Always check the
  actual load/store width in the `.s` before choosing sec_cell vs sec_st_u32.
- Avoid attributing invented behavior to any bridge: the Rev.165
  GirlForceFieldGeo `*255/u16` output write was removed because it was not in
  the disassembly. Semantics must trace to the verified disassembly or be
  marked as modeling choices.
