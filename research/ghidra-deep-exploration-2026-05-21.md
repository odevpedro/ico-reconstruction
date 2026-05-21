# Ghidra Deep Exploration — dispatch table, _Clip xrefs, system map
**Date:** 2026-05-21

## 1. Dispatch table 0x282690 — field decoding

### Raw decoded entries

| N | +0x00 flag | +0x04 mode | +0x08 tier | +0x0C callback | Name |
|---|-----------|------------|------------|----------------|------|
| 0 | 0x00000001 | 0x00000000 | 0x00000000 | 0x00168DA8 | `_clipWDebug` |
| 1 | 0x00000001 | 0x00000000 | 0x00000000 | 0x00168ED0 | `_clipW` |
| 2 | 0x00000001 | 0x00000000 | 0x00000000 | 0x001692F0 | `_clipWR` |
| 3 | 0x00000001 | 0x00000000 | 0x00000000 | 0x00169440 | `_clipWField` |
| 4 | 0x00000001 | 0x00000001 | 0x00000000 | 0x00169020 | `_clipWE` |
| 5 | 0x00000001 | 0x00000001 | 0x00000000 | 0x00169190 | `_clipWEField` |
| 6 | 0x00000001 | 0x00000000 | 0x00000000 | 0x001696C0 | `_clipWWaveForce` |
| 7 | 0x00000001 | 0x00000000 | 0x00000000 | 0x00169580 | `_clipWDitchHangWalkStop` |
| 8 | 0x00000001 | 0x00000000 | 0x00000001 | 0x00168ED0 | `_clipW` |
| 9 | 0x00000001 | 0x00000000 | 0x00000001 | 0x00169440 | `_clipWField` |
| 10 | 0x00000001 | 0x00000000 | 0x00000000 | 0x00169800 | `_clipWBoxStop` |
| 11 | 0x00000001 | 0x00000000 | 0x00000000 | 0x00169968 | `FUN_00169968` |
| 12 | 0x00000000 | 0x00000000 | 0x00000000 | 0x00169AA8 | `_clipF` |
| 13 | 0x00000000 | 0x00000001 | 0x00000000 | 0x00169BD0 | `_clipFE` |
| 14 | 0x00000000 | 0x00000000 | 0x00000000 | 0x00169E58 | `_clipFR` |
| 15 | 0x00000000 | 0x00000000 | 0x00000000 | 0x00169D18 | `_clipFIH` |
| 16 | 0x00000000 | 0x00000000 | 0x00000001 | 0x00169AA8 | `_clipF` |

### Callback group summary

**Wall clipping (`_clipW*`):**
- `_clipWDebug` (entry 0) — debug variant
- `_clipW` (entries 1, 8) — base wall clip
- `_clipWR` (entry 2) — wall-ref
- `_clipWField` (entries 3, 9) — wall field
- `_clipWE` (entry 4) — wall edge
- `_clipWEField` (entry 5) — wall edge field
- `_clipWWaveForce` (entry 6) — wave force
- `_clipWDitchHangWalkStop` (entry 7) — ditch/hang/walk stop
- `_clipWBoxStop` (entry 10) — box stop
- FUN_00169968 (entry 11) — unknown wall variant

**Floor clipping (`_clipF*`):**
- `_clipF` (entries 12, 16) — base floor clip
- `_clipFE` (entry 13) — floor edge
- `_clipFR` (entry 14) — floor ref
- `_clipFIH` (entry 15) — floor ???

**Field analysis:**
- `+0x04` = 1 on entries 4-5: `_clipWE`, `_clipWEField` — edge-related variants
- `+0x08` = 1 on entries 8-9: `_clipW`, `_clipWField` — possibly a second layer/ref pass
- `+0x08` = 1 on entry 16: `_clipF` — duplicata de ref
- When flag=0 (entries 12-16): floor clipping, possibly inactive by default

## 2. Code references to dispatch table 0x282690

| Address | Context | Function |
|---------|---------|----------|
(no direct xrefs found — may be accessed via GP-relative addressing)

## 3. Callers of `_Clip` (0x00166E10)

| Caller Address | Caller Name |
|----------------|-------------|
| `0x00167250` | `FUN_00167230` |
| `0x00167270` | `FUN_00167258` |

## 4. All `_clip*` functions

| VA | Name |
|----|------|
| `0x00168DA8` | `_clipWDebug` |
| `0x00168ED0` | `_clipW` |
| `0x00169020` | `_clipWE` |
| `0x00169190` | `_clipWEField` |
| `0x001692F0` | `_clipWR` |
| `0x00169440` | `_clipWField` |
| `0x00169580` | `_clipWDitchHangWalkStop` |
| `0x001696C0` | `_clipWWaveForce` |
| `0x00169800` | `_clipWBoxStop` |
| `0x00169AA8` | `_clipF` |
| `0x00169BD0` | `_clipFE` |
| `0x00169D18` | `_clipFIH` |
| `0x00169E58` | `_clipFR` |
| `0x00169F80` | `__ClipWallWithDrawRay` |

## 5. Cloth/physics functions (getCloth*, cloth*, InitCloth*)

| VA | Name |
|----|------|
| `0x001C1250` | `boy_dispClothes` |
| `0x001C4A28` | `GetClothAnimation` |
| `0x001C5BA0` | `InitClothes` |
| `0x001C5FD0` | `DispClothMesh` |
| `0x001C62D0` | `DispCloth4D` |
| `0x001C7FB0` | `getCloth4D_postProcess` |
| `0x001C8478` | `InitCloth4D` |
| `0x001C8B60` | `ResetClothAnimation` |
| `0x001C91B0` | `getCloth4D_PlaneClip` |
| `0x001C92D8` | `InitClothTestGeo` |
| `0x001D35C8` | `GetCloth4DWithDetail` |
| `0x00618930` | `s_src/clothAnimation.c_00618930` |
| `0x00618960` | `s_src/clothTest.c_00618960` |

## 6. Entity geometry functions (*Geo, *DL)

| VA | Name |
|----|------|
| `0x0010B2D0` | `InitPoolGeo` |
| `0x0010B770` | `updatePoolGeo` |
| `0x0010C5C0` | `PoolDL` |
| `0x0013EE60` | `add_gobj_to_tailDL` |
| `0x0013F130` | `isysGObjLinkObjDL` |
| `0x0013FC00` | `iosOmCreateDL` |
| `0x0018E5B0` | `InitChainGeo` |
| `0x0018F640` | `ChainDL` |
| `0x00191D08` | `InitEnemyCtrlGeo` |
| `0x00193600` | `GeneratorDL` |
| `0x00193A00` | `RestoreGeneratorExtGeo` |
| `0x00197080` | `BirdDL` |
| `0x00197240` | `InitBirdGeo` |
| `0x00198000` | `BossCtrlDL` |
| `0x00198218` | `InitBossCtrlGeo` |
| `0x0019A7E8` | `InitQueenGeo` |
| `0x0019A8F0` | `QueenGeo` |
| `0x0019A9A0` | `QueenDL` |
| `0x0019AA20` | `QueenBarrierGeo` |
| `0x0019BAA8` | `InitQueenBarrierGeo` |
| `0x0019BC58` | `InitQueenBallGeo` |
| `0x001BA330` | `AP1Geo` |
| `0x001BA530` | `AP1DL` |
| `0x001BBB20` | `InitAttackCheckBoundaryManagerGeo` |
| `0x001BBE50` | `InitAttackCheckBoundaryGeo` |
| `0x001BBEC8` | `AttackCheckBoundaryDL` |
| `0x001BC130` | `FloorLeverTriStateGeo` |
| `0x001BC1A8` | `FloorLeverGeo` |
| `0x001BC438` | `FloorLeverDL` |
| `0x001BC530` | `WallLeverGeo` |
| `0x001BFFE8` | `ReInitBoxGeo` |
| `0x001C0538` | `BoxGeo` |
| `0x001C05D0` | `BoxDL` |
| `0x001C09C8` | `InitFloorLeverGeo` |
| `0x001C1380` | `InitLightLineGeo` |
| `0x001C16A8` | `LightLineDL` |
| `0x001C1DD8` | `BoyGeo` |
| `0x001C20A8` | `LightLineGeo` |
| `0x001C2338` | `InitCageGeo` |
| `0x001C2760` | `HotInitCageGeo` |
| `0x001C28D0` | `CageGeo` |
| `0x001C2DF8` | `CageDL` |
| `0x001C2F20` | `CageFixGeo` |
| `0x001C2FA0` | `CageFixDL` |
| `0x001C2FF0` | `InitCandleGeo` |
| `0x001C33D8` | `ChandelierGeo` |
| `0x001C92D8` | `InitClothTestGeo` |
| `0x001CE690` | `EnemyDL` |
| `0x001CEB18` | `InitDemoMotionGeo` |
| `0x001CEB68` | `HotInitDemoMotionGeo` |
| `0x001D17F8` | `GirlGeo` |
| `0x001D1B30` | `GirlForceFieldDL` |
| `0x001D1C78` | `InitGirlForceFieldGeo` |
| `0x001D37C8` | `execBombGeo` |
| `0x001D3A30` | `ItemGeo` |
| `0x001E08B8` | `SkelTestGeo` |
| `0x001E6788` | `MoveColTestGeo` |
| `0x001E6968` | `InitMoveColTestGeo` |
| `0x001E8EB8` | `InitParticleLayoutGeo` |
| `0x001E8F38` | `InitRopeGeo` |
| `0x001E9588` | `ropeGeo` |
| `0x001E9630` | `RopeDL` |
| `0x001E9810` | `RopeGeo` |
| `0x001E9888` | `RopeFixGeo` |
| `0x001EA030` | `RotObjectDL` |
| `0x001EA278` | `InitSpiderLayoutGeo` |
| `0x001EA5E8` | `SpiderLayoutGeo` |
| `0x001EB0C0` | `RestoreSpiderLayoutExtGeo` |
| `0x001F0568` | `InitStormTestGeo` |
| `0x001F16A0` | `InitSugiLeafGeo` |
| `0x001F16F8` | `SugiLeafGeo` |
| `0x001F62E8` | `InitWormGeo` |
| `0x001F66F0` | `WormGeo` |
| `0x001FC048` | `add_gobj_to_tailCameraDL` |
| `0x001FC168` | `isysGObjMoveCameraDL` |
| `0x001FC1A0` | `isysGObjLinkCameraDL` |
| `0x00206A90` | `way_toolDL` |

## 7. Functions in clothAnimation.c range (0x001D27A8 - 0x001D3B28)

| VA | Name |
|----|------|
| `0x001D29B8` | `FUN_001d29b8` |
| `0x001D2BF0` | `FUN_001d2bf0` |
| `0x001D37C8` | `execBombGeo` |
| `0x001D3A30` | `ItemGeo` |
| `0x001D3B28` | `FUN_001d3b28` |

## 8. Boy/Girl/Enemy named functions

| VA | Name |
|----|------|
| `0x0014E5E8` | `BoyBgaManager` |
| `0x00154448` | `BoyInfoUpdate_StageChange` |
| `0x00154DD0` | `Boy_Init` |
| `0x00165B80` | `EnemyUtil_TurnToBoy` |
| `0x00165F00` | `EnemyUtil_isOtherStatus` |
| `0x0016AC10` | `GirlBrainClearTarget` |
| `0x0016BCA0` | `girlBrainMain_PositionUpdate` |
| `0x0016EB68` | `girlBrainHide_GoalTurn` |
| `0x0016F410` | `girlBrainRunawaySearchPoint` |
| `0x0016F9A8` | `girlBrainRunawayMoveByWay` |
| `0x00174D78` | `GirlAct_BoyAndMeCollisionMail` |
| `0x00180550` | `BoySekikaTexScroll` |
| `0x00191C80` | `EnemyCtrlBeforeFunc` |
| `0x001956E8` | `birdBeforeFunc` |
| `0x00197080` | `BirdDL` |
| `0x00199A08` | `queen_effect_end_func` |
| `0x00199C30` | `queen_gene_enemy` |
| `0x0019A8F0` | `QueenGeo` |
| `0x0019A9A0` | `QueenDL` |
| `0x0019AA20` | `QueenBarrierGeo` |
| `0x0019B888` | `QueenStartAttack` |
| `0x0019B8E8` | `QueenInqDead` |
| `0x0019B910` | `QueenBoysWeaponPower` |
| `0x0019B948` | `QueenBarrierInqBreakable` |
| `0x0019B998` | `queenBarrierBeforeFunc` |
| `0x0019BB60` | `queenBallBeforeFunc` |
| `0x0019C558` | `queen_barrier_disp_init` |
| `0x001C1250` | `boy_dispClothes` |
| `0x001C1DD8` | `BoyGeo` |
| `0x001C1EA8` | `boy_dispCrown` |
| `0x001CDE30` | `EnemyCheckHit` |
| `0x001CE690` | `EnemyDL` |
| `0x001CE818` | `EnemySetfDisappear` |
| `0x001CE8F0` | `enemySetParticleDie` |
| `0x001CEA00` | `EnemyDeleteParticle` |
| `0x001D17F8` | `GirlGeo` |
| `0x001D1B30` | `GirlForceFieldDL` |

## 9. Barrel/Rope/Woodbox named functions

| VA | Name |
|----|------|
| `0x001BBEC8` | `AttackCheckBoundaryDL` |
| `0x001E9588` | `ropeGeo` |
| `0x001E9630` | `RopeDL` |
| `0x001E9810` | `RopeGeo` |
| `0x001E9888` | `RopeFixGeo` |

## 10. Functions that reference both _Clip and execBombGeo

(no function calls both _Clip and execBombGeo)

## 11. `_Clip` function internals

- Body range: `0x00166E10` - `0x00166E97`
- Number of instructions: 34

### Internal labels / sub-blocks in _Clip range

| VA | Label |
|----|-------|

