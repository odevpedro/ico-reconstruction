# Ghidra Full System Analysis
**Date:** 2026-05-21

## 1. isysGObj* — Game Object processing system

### All named isysGObj* / iosOm* functions

| VA | Name | Type |
|----|------|------|
| `0x0013DDA0` | `isysGObjInit` | Function |
| `0x0013DEA0` | `isysGObjRemoveAll` | Function |
| `0x0013E190` | `isysGObjMove` | Function |
| `0x0013E220` | `isysGObjAddAfterGObj` | Function |
| `0x0013E350` | `isysGObjAddBeforeGObj` | Function |
| `0x0013E4D0` | `isysGObjAlloc` | Function |
| `0x0013E548` | `isysGObjRemove` | Function |
| `0x0013E648` | `isysGObjKindTableAdd` | Function |
| `0x0013E728` | `isysGObjKindTableRemove` | Function |
| `0x0013E7F8` | `isysGObjMoveAfterGObj` | Function |
| `0x0013E868` | `isysGObjMoveBeforeGObj` | Function |
| `0x0013E8D8` | `isysGObjAdd` | Function |
| `0x0013E9E0` | `isysGObjAddHead` | Function |
| `0x0013F130` | `isysGObjLinkObjDL` | Function |
| `0x0013F2C8` | `isysGObjDlInit` | Function |
| `0x0013F3E8` | `isysGObjProcessAlloc` | Function |
| `0x0013F3F0` | `isysGObjProcAdd_` | Function |
| `0x0013F6B8` | `isysGObjProcRemove` | Function |
| `0x0013F700` | `isysGObjProcessAlloc` | Function |
| `0x0013F808` | `isysGObjProcPause` | Label |
| `0x0013F8C0` | `isysGObjProcRemoveAll` | Function |
| `0x0013F8F8` | `isysGObjProcThreadSleep` | Function |
| `0x0013F9A0` | `iosOmInit` | Label |
| `0x0013F9D0` | `_iosOmMain` | Label |
| `0x0013FC00` | `iosOmCreateDL` | Label |
| `0x0013FD10` | `iosOmExeEachGObj` | Function |
| `0x0013FD78` | `iosOmExeEachGObjAll` | Function |
| `0x0013FE18` | `iosOmReturnExeEachGObj` | Function |
| `0x0013FEB0` | `iosOmGetGObjStatus` | Function |
| `0x00140048` | `iosOmExeMail` | Function |
| `0x00141128` | `isysGObjActiveLink` | Label |
| `0x00141160` | `isysGObjActiveDlLink` | Label |
| `0x00141D18` | `isysGObjProcAddSGOppArg` | Function |
| `0x001FC168` | `isysGObjMoveCameraDL` | Function |
| `0x001FC1A0` | `isysGObjLinkCameraDL` | Function |
| `0x001FC2E0` | `isysGObjMoveCameraDLHead` | Function |

### isysGObjProcAdd_ (0x0013F3F0) — callback registration

```
Body: 0x0013F3F0 - 0x0013F62B
Parameters:

  0x0013F3F0: addiu    addiu sp,sp,-0x90
  0x0013F3F4: sd       sd s7,0x70(sp)
  0x0013F3F8: sd       sd s6,0x60(sp)
  0x0013F3FC: daddu    daddu s7,t1,zero
  0x0013F400: sd       sd s5,0x50(sp)
  0x0013F404: andi     andi s6,a3,0xff
  0x0013F408: sd       sd s4,0x40(sp)
  0x0013F40C: daddu    daddu s5,a1,zero
  0x0013F410: sd       sd s3,0x30(sp)
  0x0013F414: daddu    daddu s4,a2,zero
  0x0013F418: sd       sd s2,0x20(sp)
  0x0013F41C: daddu    daddu s3,t0,zero
  0x0013F420: sd       sd ra,0x80(sp)
  0x0013F424: daddu    daddu s2,a0,zero
  0x0013F428: sd       sd s1,0x10(sp)
  0x0013F42C: bne      bne s4,zero,0x0013f43c
  0x0013F430: _sd      _sd s0,0x0(sp)
  0x0013F434: b        b 0x0013f608
  0x0013F438: _clear   _clear v0
  0x0013F43C: lw       lw a1,-0x4c44(gp)
  0x0013F440: beq      beq a1,zero,0x0013f478
  0x0013F444: _clear   _clear a0
  0x0013F448: lw       lw v1,-0x4c48(gp)
  0x0013F44C: lw       lw v0,0x0(v1)
  0x0013F450: beq      beq v0,zero,0x0013f478
  0x0013F454: _nop     _nop
  0x0013F458: daddu    daddu a2,a1,zero
  0x0013F45C: addiu    addiu a0,a0,0x1
  0x0013F460: sltu     sltu v0,a0,a2
  0x0013F464: beq      beq v0,zero,0x0013f478
  0x0013F468: _addiu   _addiu v1,v1,0x94
  0x0013F46C: lw       lw v0,0x0(v1)
```

### isysGObjProcRemove (0x0013F6B8)

```
  0x0013F6B8: addiu    addiu sp,sp,-0x20
  0x0013F6BC: sd       sd s0,0x0(sp)
  0x0013F6C0: sd       sd ra,0x10(sp)
  0x0013F6C4: jal      jal 0x0013f638
  0x0013F6C8: _daddu   _daddu s0,a0,zero
  0x0013F6CC: lw       lw v0,0x10(s0)
  0x0013F6D0: bne      bne v0,zero,0x0013f6ec
  0x0013F6D4: _sw      _sw zero,0x0(s0)
  0x0013F6D8: addiu    addiu a0,s0,0x24
  0x0013F6DC: ld       ld ra,0x10(sp)
  0x0013F6E0: ld       ld s0,0x0(sp)
  0x0013F6E4: j        j 0x0013d3f8
  0x0013F6E8: _addiu   _addiu sp,sp,0x20
  0x0013F6EC: ld       ld ra,0x10(sp)
  0x0013F6F0: ld       ld s0,0x0(sp)
```

### iosOmExeEachGObj (0x0013FD10) — object traversal dispatcher

```
  0x0013FD10: addiu    addiu sp,sp,-0x40
  0x0013FD14: aui      aui v0,zero,0x280000
  0x0013FD18: sd       sd s2,0x20(sp)
  0x0013FD1C: sll      sll a0,a0,0x2
  0x0013FD20: sd       sd s1,0x10(sp)
  0x0013FD24: addiu    addiu v0,v0,0x1a70
  0x0013FD28: sd       sd ra,0x30(sp)
  0x0013FD2C: addu     addu a0,a0,v0
  0x0013FD30: sd       sd s0,0x0(sp)
  0x0013FD34: daddu    daddu s2,a1,zero
  0x0013FD38: lw       lw s0,0x0(a0)
  0x0013FD3C: beq      beq s0,zero,0x0013fd5c
  0x0013FD40: _daddu   _daddu s1,a2,zero
  0x0013FD44: daddu    daddu a0,s0,zero
  0x0013FD48: jalr     jalr s2
  0x0013FD4C: _daddu   _daddu a1,s1,zero
  0x0013FD50: lw       lw s0,0x10(s0)
  0x0013FD54: bne      bne s0,zero,0x0013fd48
  0x0013FD58: _daddu   _daddu a0,s0,zero
  0x0013FD5C: ld       ld ra,0x30(sp)
  0x0013FD60: ld       ld s2,0x20(sp)
  0x0013FD64: ld       ld s1,0x10(sp)
  0x0013FD68: ld       ld s0,0x0(sp)
```

### iosOmExeEachGObjAll (0x0013FD78)

```
  0x0013FD78: addiu    addiu sp,sp,-0x60
  0x0013FD7C: aui      aui v0,zero,0x280000
  0x0013FD80: sd       sd s4,0x40(sp)
  0x0013FD84: clear    clear v1
  0x0013FD88: sd       sd s3,0x30(sp)
  0x0013FD8C: addiu    addiu s4,v0,0x1a70
  0x0013FD90: sd       sd s2,0x20(sp)
  0x0013FD94: daddu    daddu s3,a0,zero
  0x0013FD98: sd       sd ra,0x50(sp)
  0x0013FD9C: daddu    daddu s2,a1,zero
  0x0013FDA0: sd       sd s1,0x10(sp)
  0x0013FDA4: sd       sd s0,0x0(sp)
  0x0013FDA8: sll      sll v0,v1,0x2
  0x0013FDAC: nop      nop
  0x0013FDB0: addu     addu v0,v0,s4
  0x0013FDB4: lw       lw s0,0x0(v0)
  0x0013FDB8: beq      beq s0,zero,0x0013fde4
  0x0013FDBC: _addiu   _addiu s1,v1,0x1
  0x0013FDC0: daddu    daddu a0,s0,zero
  0x0013FDC4: nop      nop
  0x0013FDC8: jalr     jalr s3
  0x0013FDCC: _daddu   _daddu a1,s2,zero
  0x0013FDD0: lw       lw s0,0x10(s0)
  0x0013FDD4: bne      bne s0,zero,0x0013fdc8
  0x0013FDD8: _daddu   _daddu a0,s0,zero
  0x0013FDDC: b        b 0x0013fde8
  0x0013FDE0: _daddu   _daddu v1,s1,zero
  0x0013FDE4: daddu    daddu v1,s1,zero
  0x0013FDE8: slti     slti v0,v1,0x8
```

## 2. execBombGeo (0x001D37C8) — 5-state geometry function

### Jump table targets (0x00618FB0)

| Index | Target |
|-------|--------|
| 0 | `0x001D3818` |
| 1 | `0x001D3844` |
| 2 | `0x001D391C` |
| 3 | `0x001D39E0` |
| 4 | `0x001D3A10` |

### Full disassembly
```
  0x001D37C8: addiu    addiu sp,sp,-0x60
  0x001D37CC: sd       sd s2,0x30(sp)
  0x001D37D0: sd       sd s1,0x20(sp)
  0x001D37D4: daddu    daddu s2,a0,zero
  0x001D37D8: sd       sd ra,0x50(sp)
  0x001D37DC: sd       sd s3,0x40(sp)
  0x001D37E0: sd       sd s0,0x10(sp)
  0x001D37E4: lw       lw v0,0x15c(s2)
  0x001D37E8: lw       lw s3,0x800(v0)
  0x001D37EC: addiu    addiu s1,s3,0x40
  0x001D37F0: lw       lw v1,0x8(s1)
  0x001D37F4: sltiu    sltiu v0,v1,0x5
  0x001D37F8: beq      beq v0,zero,0x001d3818
  0x001D37FC: _aui     _aui v0,zero,0x620000
  0x001D3800: sll      sll v1,v1,0x2  <-- sll $v1,$v1,2 (jump index)
  0x001D3804: addiu    addiu v0,v0,-0x7050
  0x001D3808: addu     addu v1,v1,v0
  0x001D380C: lw       lw a0,0x0(v1)
```

## 3. DispCollisionPC (0x00166A10) — halfword writer host

### Full disassembly with probe markers
```
  0x00166A10: addiu    addiu sp,sp,-0x20
  0x00166A14: lw       lw v0,-0x6ed0(gp)
  0x00166A18: sd       sd ra,0x10(sp)
  0x00166A1C: beq      beq v0,zero,0x00166b9c
  0x00166A20: _sd      _sd s0,0x0(sp)
  0x00166A24: aui      aui v1,zero,0x10000000
  0x00166A28: lw       lw a2,-0x4bd8(gp)
  0x00166A2C: lw       lw a0,0x0(v1)
  0x00166A30: aui      aui v0,zero,0x6b0000
  0x00166A34: addiu    addiu s0,v0,-0x5480
  0x00166A38: aui      aui a1,zero,0x560000
  0x00166A3C: subu     subu a0,a0,a2
  0x00166A40: lw       lw a3,-0x4bd4(gp)
  0x00166A44: sw       sw a0,-0x4bd8(gp)
  0x00166A48: addiu    addiu a1,a1,-0x6dc0
  0x00166A4C: lw       lw a2,-0x4be8(gp)
  0x00166A50: jal      jal 0x00264df8
  0x00166A54: _daddu   _daddu a0,s0,zero
```

## 4. vblankHandler (0x0023EE28)

### Xrefs / callers

| Caller | Name |
|--------|------|
| `0x0019D4C0` | `?` |

### Disassembly
```
```

## 5. Main loop call graph

### Function: FUN_00132630 (0x132630)

| Calls | Target |
|-------|--------|

### Function: unifile_read_func (0x1321c8)

| Calls | Target |
|-------|--------|
| `0x001320E8` | `iosCdvdHandlerRead` |
| `0x00264DF8` | `FUN_00264df8` |

### Function: backStageProcessInStage (0x1a0a38)

| Calls | Target |
|-------|--------|

### Function: _Clip (0x166e10)

| Calls | Target |
|-------|--------|
| `0x00243B60` | `FUN_00243b60` |

### Function: DispCollisionPC (0x166a10)

| Calls | Target |
|-------|--------|
| `0x00264DF8` | `FUN_00264df8` |

### Function: isysGObjProcAdd_ (0x13f3f0)

| Calls | Target |
|-------|--------|

### Function: iosOmExeEachGObj (0x13fd10)

| Calls | Target |
|-------|--------|

## 6. Scene loader (kanban.c) — GP=0x27A7A8

### Functions referencing GP base 0x27A7A8

| Function | Reference at |
|----------|--------------|
(no direct xrefs)

### Functions with kanban/scene/load in name

| VA | Name |
|----|------|
| `0x0010EE90` | `file_LoadCDFile` |
| `0x0010F5B8` | `file_LoadFile` |
| `0x001147C4` | `debug_TryToGetStartStage` |
| `0x00128868` | `stage_MakeGObj` |
| `0x001297A0` | `stage_SetAnimation` |
| `0x00129970` | `stage_ContinueAnimation` |
| `0x0012A1D8` | `stage_ApplyData` |
| `0x0012A618` | `stage_MakePlayBgAnimation` |
| `0x0012A7F8` | `stage_DispBgAnimation` |
| `0x0012AA80` | `stage_CheckAnimationFinish` |
| `0x0012AB50` | `stage_CheckAnimationFrame` |
| `0x0012ABE0` | `stage_SetLoopFlag` |
| `0x0012AC28` | `stage_SetFrameStep` |
| `0x0012AC70` | `stage_SetParentOfGObj` |
| `0x0012ACD8` | `stage_SetParentOfGObjWithLocalRotationFlag` |
| `0x0012AD40` | `stage_SetLocalizeGeometry` |
| `0x0012AFC0` | `tex_loadImage` |
| `0x0012D218` | `tex_LoadTexturePart` |
| `0x0012FD50` | `tex_LoadTexture` |
| `0x00131780` | `temp_loadfunc` |
| `0x00144780` | `sndBgmReadyNextStage` |
| `0x00144A08` | `ACTGame_StageChangeGObj` |
| `0x00149F70` | `ACTGame_StageChangeGObjID` |
| `0x00149FF0` | `ACTGame_StageChangeGObjDirect` |
| `0x0014B7A8` | `GetOtherStageGirlOrient` |
| `0x0014BBF0` | `GetGirlPositionAtThisStage` |
| `0x0014E708` | `E3_StageStartBoy` |
| `0x001504D8` | `ACTDispLwsBoyStonize_InQueenStage` |
| `0x00153318` | `SetStatusBoy_OtherStageGirlPinch` |
| `0x00154448` | `BoyInfoUpdate_StageChange` |
| `0x001545D8` | `IsGirlEscortedInNextStage` |
| `0x001546C0` | `RequestStageChangeKidnapEnd` |
| `0x00154770` | `GetEfStageCameraTargetID` |
| `0x0017E330` | `RequestStageChange` |
| `0x0017E4A0` | `RequestStageChangeSimple` |
| `0x0017E4F0` | `RequestStageChangeDirect` |
| `0x00180BC0` | `warpGirlOutStage` |
| `0x00191AA0` | `GetStageFromLabel` |
| `0x00191B70` | `eBrainGetTargetGeneratorFromLabelStage` |
| `0x0019C7D0` | `GetStageDifferenceMatrix` |
| `0x0019C9A0` | `StageOrientGet2` |
| `0x0019CBB8` | `OtherStagePositionGet` |
| `0x0019CD50` | `StageOrientInit` |
| `0x0019CE40` | `StageOrientGet` |
| `0x0019F758` | `exit_stage` |
| `0x001A00A0` | `backStageProcessOutStage` |
| `0x001A05D0` | `backStageProcessMain` |
| `0x001A0A38` | `backStageProcessInStage` |
| `0x001A0EB8` | `backStageSave` |
| `0x001A0F60` | `backStageLoad` |
| `0x001A1D88` | `ReadStageAnimationFile` |
| `0x001A2758` | `ReadStageSettingFile` |
| `0x001A3398` | `debug_Load` |
| `0x001A7AD0` | `debug_SelectStageMain` |
| `0x001A7BB0` | `debug_SelectStage` |
| `0x001AA6A0` | `debug_SaveStartStageFile` |
| `0x001AB190` | `debug_BackStageTest` |
| `0x001AE3B0` | `gamesysObjInfoStageInitFlagCls` |
| `0x001AE420` | `gamesysObjInfoPosSetStage` |
| `0x001AE4C8` | `gamesysObjInfoPosNewStageSet` |
| `0x001AE830` | `gamesysGetGirlStageIDAndPosition` |
| `0x001AE880` | `gamesysStageExitTimeSet` |
| `0x001AFB98` | `kanbanReqAdd` |
| `0x001AFE50` | `kanbanInit` |
| `0x001B0538` | `kanbanReqAllDel` |
| `0x001B05A8` | `kanbanExec` |
| `0x001B2A30` | `la_load_processing` |
| `0x001B5958` | `la_switching_stage` |
| `0x001B76F8` | `initSceneGObj` |
| `0x001B7F20` | `HotInitSceneObjects` |
| `0x001B81A8` | `MoveNextStage_Clear` |
| `0x001F6DB8` | `mallocsekistage` |
| `0x00206208` | `quick_load_wpfile` |
| `0x0023B728` | `stageSEtaimatsu` |
| `0x0023B8B8` | `stageSE04eriver` |
| `0x0023BAB0` | `stageSE06ariver` |
| `0x0023BC08` | `stageSE10lstrong2` |
| `0x0023BD50` | `stageSE19ataki` |
| `0x0023BF70` | `stageSE02astrong` |
| `0x0023C090` | `stageSE02ataki` |
| `0x0023C0E8` | `stageSE02atakib` |
| `0x0023C148` | `stageSE03tsuiro` |
| `0x0023C180` | `stageSE03tnotSuiro` |
| `0x0023C1C0` | `stageSE04agate` |
| `0x0023C290` | `stageSE04bstrong` |
| `0x0023C2F0` | `stageSE04ewind` |
| `0x0023C380` | `stageSE04eriverDown` |
| `0x0023C430` | `stageSE06astrong` |
| `0x0023C540` | `stageSE06abirdIn` |
| `0x0023C5F0` | `stageSE06ataimatsu` |
| `0x0023C660` | `stageSE08astrong` |
| `0x0023C730` | `stageSE08astrong2` |
| `0x0023C808` | `stageSE08anoise3` |
| `0x0023C890` | `stageSE08ataimatsu` |
| `0x0023C988` | `stageSE08brail` |
| `0x0023CA40` | `stageSE10lstrong` |
| `0x0023CB80` | `stageSE10rstrong` |
| `0x0023CCE0` | `stageSE13dterrace` |
| `0x0023CD20` | `stageSE13dstrong` |
| `0x0023CDD8` | `stageSE17astrong` |
| `0x0023D050` | `stageSE19astrong` |
| `0x0023D0F0` | `stageSE20astrong` |
| `0x0023D1D0` | `stageSE22astrong` |
| `0x00554010` | `s_file_LoadCDFile:file_is_not_exis_00554010` |
| `0x005547A0` | `s_object/stagesetting/%s.ssb_005547a0` |
| `0x00554BE0` | `s_StageSetting_Tool_00554be0` |
| `0x00554C38` | `s_Load_Settings_00554c38` |
| `0x00555BF8` | `s_src/StageAnimation.c_00555bf8` |
| `0x00555D78` | `s_stage_CheckAnimationFinish:illeg_00555d78` |
| `0x00555DB0` | `s_stage_ContinueAnimation:illegal_A_00555db0` |
| `0x0055AE18` | `s_UseStageNo>0_0055ae18` |
| `0x00612800` | `s_stage_manager()_in_00612800` |
| `0x00612828` | `s_STAGE_MANAGER_START_00612828` |
| `0x00612A88` | `s_ReadModelFile:Already_loaded._(i_00612a88` |
| `0x00614A10` | `s_====_Save_start_stage_==========_00614a10` |
| `0x00614A68` | `s_debug_SaveStartStageFile:_host_f_00614a68` |
| `0x00615228` | `s_stage:%d_00615228` |
| `0x00615790` | `s_CD_LOAD_INFO_COMMON_00615790` |
| `0x00616F38` | `s_load_processing_00616f38` |
| `0x0061B760` | `s_quick_load_0061b760` |

### Functions referencing world_state (0x282740 or GP+0x6F60)

| VA | Name |
|----|------|
(GP-relative — no direct xrefs)

