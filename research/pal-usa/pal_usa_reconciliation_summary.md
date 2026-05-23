# PAL→USA Reconciliation Summary

Generated: 2026-05-22 23:07

## Inputs

- MAIN.MAP: 4368 functions
- PAL .text: 0x00100000 (1612740 bytes)
- USA .text: 0x00100000 (1504724 bytes)
- PAL fingerprints extracted (from MAIN.MAP boundaries): 4357

## Results

| Confidence | Objects | Functions Mapped |
|------------|---------|------------------|
| HIGH       | 0 | 0 |
| MEDIUM     | 0 | 0 |
| LOW        | 19 | 45 |
| INVALID    | 0 | — |
| No Seeds   | 189 | — |
| **Total**  | **208** | **45** |

Match rate: 45/4357 (1.0%)

## Top Objects by Confidence

| Confidence | Object | Mapped | Total | Ratio | Offset |
|------------|--------|--------|-------|-------|--------|
| LOW        | st06a.o                        |   14/  44 | 0.32 | 0x-0011B98 |
| LOW        | soundManager.o                 |    7/  92 | 0.08 | 0x-0002780 |
| LOW        | st04r.o                        |    7/ 104 | 0.07 | 0x-0011678 |
| LOW        | pad.o                          |    2/  20 | 0.10 | 0x-003AD98 |
| LOW        | geometryManager.o              |    1/  45 | 0.02 | 0x00169B14 |
| LOW        | matrixDrive.o                  |    1/  37 | 0.03 | 0x000BBA78 |
| LOW        | motionManager2.o               |    1/  76 | 0.01 | 0x00078F58 |
| LOW        | DisplayP2O.o                   |    1/  11 | 0.09 | 0x0007DAE0 |
| LOW        | mcard.o                        |    1/  18 | 0.06 | 0x00054EE8 |
| LOW        | brain.o                        |    1/ 107 | 0.01 | 0x000E2964 |
| LOW        | mv_videodec.o                  |    1/   9 | 0.11 | 0x000AC100 |
| LOW        | haveParentSimpleObj.o          |    1/   3 | 0.33 | 0x00009C90 |
| LOW        | layout_texture.o               |    1/  13 | 0.08 | 0x-00A2A20 |
| LOW        | box.o                          |    1/  44 | 0.02 | 0x-00BCE40 |
| LOW        | flyManager.o                   |    1/   5 | 0.20 | 0x0007A7B0 |
| LOW        | BgAnimation.o                  |    1/  20 | 0.05 | 0x-00D8398 |
| LOW        | objact.o                       |    1/   5 | 0.20 | 0x0000EE38 |
| LOW        | mv_disp.o                      |    1/  34 | 0.03 | 0x-00E4F80 |
| LOW        | vobj.o                         |    1/ 706 | 0.00 | 0x00006E90 |

## Unmatched Objects

- main.o (0x00101C80-0x00102620, 5 funcs)
- sampleMode.o (0x00102648-0x00102868, 5 funcs)
- delayFreeManager.o (0x001028F8-0x00102A20, 3 funcs)
- keyInput.o (0x00104D68-0x00104E50, 2 funcs)
- pool.o (0x0010B798-0x0010DAC0, 14 funcs)
- quaternion.o (0x0010DB40-0x0010F088, 45 funcs)
- tableSin.o (0x0010F0B0-0x0010F368, 6 funcs)
- FileManager.o (0x0010F440-0x0010FD20, 2 funcs)
- GifPacket.o (0x0010FD98-0x001129C8, 38 funcs)
- GsBase.o (0x00112A30-0x00115718, 18 funcs)
- Light.o (0x00115900-0x00118010, 8 funcs)
- Matrix.o (0x00118140-0x001192A8, 62 funcs)
- MicroCode.o (0x00119458-0x00119788, 4 funcs)
- Packet.o (0x001199B0-0x0011C750, 5 funcs)
- Primitive.o (0x0011C7B8-0x0011EC78, 12 funcs)
- RegistPacket.o (0x001223D8-0x00122EF8, 9 funcs)
- Shadow.o (0x00122F48-0x00128A58, 11 funcs)
- StageAnimation.o (0x00128CF0-0x0012B0D8, 23 funcs)
- Texture.o (0x0012B168-0x00130470, 22 funcs)
- ZFog.o (0x001304E8-0x00131230, 3 funcs)
- cdvd.o (0x001319B0-0x001336A0, 25 funcs)
- inflate.o (0x00135530-0x00135AC0, 3 funcs)
- ios.o (0x00135E08-0x00135FD8, 2 funcs)
- mblock.o (0x001361A8-0x001362B8, 4 funcs)
- mcdata.o (0x00138630-0x00138738, 2 funcs)
- memory.o (0x00138910-0x0013A068, 12 funcs)
- message.o (0x0013A150-0x0013A6C8, 7 funcs)
- shockdriver.o (0x0013BA80-0x0013CFA0, 38 funcs)
- thread.o (0x0013D0B0-0x0013DC20, 23 funcs)
- gobj.o (0x0013DCA0-0x0013EC78, 23 funcs)
- gobj_dl.o (0x0013ED58-0x0013F278, 10 funcs)
- gobj_process.o (0x0013F2E8-0x0013F890, 16 funcs)
- isys.o (0x0013F898-0x0013F898, 1 funcs)
- obj_manager.o (0x0013F8A0-0x0013FF48, 13 funcs)
- s_init.o (0x00140030-0x001435B8, 40 funcs)
- boyact.o (0x0014C230-0x00153588, 72 funcs)
- commonact.o (0x00153618-0x0015D890, 138 funcs)
- enemy_act.o (0x0015DF28-0x00163F78, 58 funcs)
- fieldCollision.o (0x00163FA0-0x00168130, 47 funcs)
- fuzio.o (0x00168150-0x001683D8, 8 funcs)
- girl_act.o (0x001685F0-0x00173E38, 60 funcs)
- jimaku.o (0x00174018-0x00174A50, 8 funcs)
- way_sys.o (0x00174A60-0x00176588, 6 funcs)
- way_util.o (0x001765E8-0x00178CD0, 39 funcs)
- gflag.o (0x00178D60-0x00178ED8, 6 funcs)
- script.o (0x00178F08-0x0017C338, 89 funcs)
- st25a.o (0x0017C3D0-0x0017E0A0, 20 funcs)
- warpGirl.o (0x0017E128-0x0017E4A0, 3 funcs)
- attackhit.o (0x0017F098-0x0017F268, 5 funcs)
- chain.o (0x0018B2F8-0x0018E198, 28 funcs)
- ebrain.o (0x0018E280-0x0018F470, 11 funcs)
- fightSound.o (0x0018F6A0-0x0018F7E8, 6 funcs)
- generator.o (0x0018F7F8-0x001915F8, 33 funcs)
- gv.o (0x00191670-0x00192B60, 36 funcs)
- act_bird.o (0x00192D38-0x001948F8, 12 funcs)
- gather_effect.o (0x00194968-0x00194D68, 4 funcs)
- itou_boss.o (0x00194E70-0x00195990, 10 funcs)
- itou_gflag.o (0x001959C0-0x001959D0, 3 funcs)
- itou_sub.o (0x00195C18-0x00195CB0, 4 funcs)
- lightning.o (0x00196110-0x00196E60, 5 funcs)
- queen.o (0x00197000-0x001991D0, 23 funcs)
- queen_barrier_disp.o (0x001994D8-0x00199C60, 4 funcs)
- stage_orient.o (0x00199C98-0x00199D88, 2 funcs)
- mv_main.o (0x00199EB8-0x0019A800, 4 funcs)
- mv_readbuf.o (0x0019A870-0x0019A9F0, 6 funcs)
- mv_strfile.o (0x0019AA10-0x0019AA68, 3 funcs)
- mv_vobuf.o (0x0019B220-0x0019B4C8, 8 funcs)
- DObj.o (0x0019C1B0-0x0019C3C0, 4 funcs)
- StageManager.o (0x0019CB00-0x0019CEC8, 8 funcs)
- backStage.o (0x0019CF28-0x0019DE68, 8 funcs)
- charFileManager.o (0x0019DE70-0x0019FB98, 21 funcs)
- debug.o (0x0019FBB8-0x001A7C70, 59 funcs)
- debug_exception.o (0x001A7DF0-0x001AA3A0, 17 funcs)
- debug_menu.o (0x001AA4C0-0x001AA628, 2 funcs)
- gamesys.o (0x001AA678-0x001AB3B0, 19 funcs)
- icoMisc.o (0x001ABBF8-0x001AC608, 4 funcs)
- kanban.o (0x001AC610-0x001AD020, 7 funcs)
- kanbanBoot.o (0x001AD358-0x001AD4C0, 3 funcs)
- layout_action.o (0x001AD4D0-0x001B24B0, 57 funcs)
- sceneManager.o (0x001B3CB0-0x001B4A38, 9 funcs)
- staffroll.o (0x001B4A48-0x001B4F50, 3 funcs)
- a_p_1.o (0x001B4FB0-0x001B7050, 11 funcs)
- act_a_p_1.o (0x001B7770-0x001B8140, 12 funcs)
- actressLight.o (0x001B8150-0x001B8150, 1 funcs)
- attackCheckBoundary.o (0x001B8218-0x001B8928, 12 funcs)
- boy.o (0x001BDA70-0x001BE748, 8 funcs)
- cage.o (0x001BE790-0x001BF498, 9 funcs)
- cageFix.o (0x001BF4E0-0x001BF5A8, 3 funcs)
- candle.o (0x001BF5B0-0x001BF8F8, 5 funcs)
- chandelier.o (0x001BF998-0x001BFA78, 3 funcs)
- clipCollisionManager.o (0x001BFCA0-0x001BFCE0, 2 funcs)
- clothAnimation.o (0x001BFD38-0x001C5770, 34 funcs)
- clothTest.o (0x001C5898-0x001C58F0, 3 funcs)
- darkVolume.o (0x001C7ED0-0x001C8438, 11 funcs)
- effectTool.o (0x001C9A98-0x001C9A98, 1 funcs)
- enemy.o (0x001CA3F0-0x001CB318, 29 funcs)
- enemyParts.o (0x001CB370-0x001CC000, 11 funcs)
- flag.o (0x001CC138-0x001CC7C8, 3 funcs)
- frameDependSequence.o (0x001CD400-0x001CDA10, 7 funcs)
- girl.o (0x001CDD98-0x001CE208, 5 funcs)
- girlForceField.o (0x001CE260-0x001CE428, 3 funcs)
- handManager.o (0x001CEB10-0x001CEB10, 1 funcs)
- item.o (0x001CEC80-0x001D08A0, 18 funcs)
- lineManager.o (0x001D0958-0x001D1188, 7 funcs)
- lodManager.o (0x001D1270-0x001D1270, 1 funcs)
- motionFileManager.o (0x001D13F8-0x001D15A0, 7 funcs)
- motionManager.o (0x001D4B08-0x001DD2A0, 12 funcs)
- motionOrientManager.o (0x001DD5F0-0x001E0F10, 16 funcs)
- motionViewer.o (0x001E1E20-0x001E2550, 2 funcs)
- moveColTest.o (0x001E2D60-0x001E2F40, 3 funcs)
- multiBgaManager.o (0x001E2FB0-0x001E3278, 6 funcs)
- particleEffect.o (0x001E49E0-0x001E5438, 19 funcs)
- particleLayout.o (0x001E5460-0x001E54E8, 4 funcs)
- rope.o (0x001E54F0-0x001E5DC8, 7 funcs)
- ropeFix.o (0x001E5E40-0x001E5EC8, 3 funcs)
- rotObject.o (0x001E5F08-0x001E6608, 13 funcs)
- spider.o (0x001E6628-0x001E7890, 16 funcs)
- spiderGroupManager.o (0x001E7908-0x001E7F38, 7 funcs)
- stageMultiBgaManager.o (0x001E7F98-0x001E81C8, 4 funcs)
- staticBlur.o (0x001E97E0-0x001EBB30, 17 funcs)
- stormTest.o (0x001EBB38-0x001EC640, 8 funcs)
- streamMotionManager.o (0x001EC910-0x001ED208, 13 funcs)
- sugiTree.o (0x001ED398-0x001ED818, 5 funcs)
- torch.o (0x001ED8A0-0x001EE168, 12 funcs)
- waySystemManager.o (0x001EE1B8-0x001EE1F8, 2 funcs)
- weapon.o (0x001EE2A0-0x001F03E0, 28 funcs)
- windField.o (0x001F03F8-0x001F0E80, 10 funcs)
- windManager.o (0x001F0F00-0x001F11B0, 5 funcs)
- wireLetter.o (0x001F1290-0x001F1538, 5 funcs)
- worm.o (0x001F1FF0-0x001F29B8, 8 funcs)
- Basic.o (0x001F2B38-0x001F2D18, 10 funcs)
- DisplayFont.o (0x001F6618-0x001F7010, 5 funcs)
- DisplayList.o (0x001F7030-0x001F76A8, 11 funcs)
- DmaPacket.o (0x001F76B0-0x001F7780, 3 funcs)
- EnemyInit.o (0x001F77B0-0x001F7AF0, 2 funcs)
- gobj_cam_dl.o (0x001F7BA8-0x001F8028, 10 funcs)
- adpcm_init.o (0x001F8088-0x001FE370, 30 funcs)
- act.o (0x001FE388-0x00200258, 18 funcs)
- act2.o (0x00200318-0x00200470, 2 funcs)
- seMail.o (0x002004D0-0x00200648, 2 funcs)
- way_kidnap.o (0x00200788-0x00201588, 9 funcs)
- way_llf.o (0x00201608-0x00201E60, 25 funcs)
- way_tool.o (0x00202638-0x00203478, 3 funcs)
- deja.o (0x002034F8-0x00203D98, 6 funcs)
- e3.o (0x00203DC0-0x00206A38, 41 funcs)
- end.o (0x00206AC8-0x00209B58, 52 funcs)
- op.o (0x00209C18-0x0020BBE0, 15 funcs)
- st00a.o (0x0020BD68-0x0020CEF8, 22 funcs)
- st01b.o (0x0020CFD8-0x0020D908, 18 funcs)
- st02a.o (0x0020D9B8-0x0020F168, 36 funcs)
- st03t.o (0x0020F1B8-0x00210230, 21 funcs)
- st04a.o (0x002102D8-0x002142C8, 30 funcs)
- st04b.o (0x002143A8-0x00214C48, 14 funcs)
- st04c.o (0x00214CD0-0x002155A0, 16 funcs)
- st04d.o (0x002155B0-0x00215FE0, 12 funcs)
- st04e.o (0x002160C0-0x002162B8, 5 funcs)
- st04l.o (0x00216330-0x0021B2A0, 115 funcs)
- st05b.o (0x0021FA70-0x00220068, 9 funcs)
- st05c.o (0x002200F0-0x00220A00, 14 funcs)
- st05d.o (0x00220A70-0x002211A8, 10 funcs)
- st05e.o (0x00221218-0x002215E8, 7 funcs)
- st07a.o (0x00223528-0x002252F0, 32 funcs)
- st08a.o (0x00225368-0x00225BB8, 16 funcs)
- st08b.o (0x00225C60-0x00226A88, 16 funcs)
- st09a.o (0x00226AF0-0x00227210, 14 funcs)
- st10l.o (0x00227268-0x00228940, 37 funcs)
- st10r.o (0x00228998-0x00229E08, 28 funcs)
- st13a.o (0x00229E80-0x0022A900, 15 funcs)
- st13b.o (0x0022A9B0-0x0022CA30, 32 funcs)
- st13b2.o (0x0022CAE8-0x0022CAE8, 1 funcs)
- st13c.o (0x0022CB18-0x0022F7B8, 42 funcs)
- st13d.o (0x0022F810-0x0022FA90, 4 funcs)
- st17a.o (0x0022FA98-0x002308F8, 17 funcs)
- st17b.o (0x00230948-0x00230AC8, 5 funcs)
- st18a.o (0x00230B68-0x00231CC0, 21 funcs)
- st19a.o (0x00231D40-0x00232AB8, 18 funcs)
- st20a.o (0x00232B58-0x00233C58, 24 funcs)
- st22a.o (0x00233CD0-0x00234188, 4 funcs)
- st24a.o (0x002342A0-0x002344E8, 3 funcs)
- st47a.o (0x00234528-0x00235CA0, 41 funcs)
- st99a.o (0x00235D10-0x002363A0, 16 funcs)
- stageSEProc.o (0x00236408-0x00238070, 47 funcs)
- access.o (0x00238100-0x00238148, 2 funcs)
- lws_kyomi.o (0x002381F8-0x00238288, 3 funcs)
- mv_audiodec.o (0x00238580-0x00238FF0, 9 funcs)
- mv_sub.o (0x0023A468-0x0023A478, 2 funcs)
- mv_vibuf.o (0x0023A5E8-0x0023B6F8, 12 funcs)
- GobjProc.o (0x0023B7E8-0x0023BB28, 8 funcs)
- PObj.o (0x0023C078-0x0023C688, 3 funcs)

## Key Objects Now Mapped

- gobj.o: NONE (0/23 mapped, offset=0x00000000)
- gobj_process.o: NONE (0/16 mapped, offset=0x00000000)
- girl_act.o: NONE (0/60 mapped, offset=0x00000000)
- ebrain.o: NONE (0/11 mapped, offset=0x00000000)
- queen.o: NONE (0/23 mapped, offset=0x00000000)
- backStage.o: NONE (0/8 mapped, offset=0x00000000)
- kanban.o: NONE (0/7 mapped, offset=0x00000000)
- kanbanBoot.o: NONE (0/3 mapped, offset=0x00000000)
- rope.o: NONE (0/7 mapped, offset=0x00000000)
- gobj_cam_dl.o: NONE (0/10 mapped, offset=0x00000000)

## Next Steps

1. Review HIGH+MEDIUM objects — apply as ground truth
2. For unmatched objects, investigate PAL-only code or different structure
3. Generate `symbol_addrs_usa.txt` from function_map_candidates
4. Cross-check `SRCFILE.TXT` for source-level validation
5. Update splat YAML with reconciled addresses
