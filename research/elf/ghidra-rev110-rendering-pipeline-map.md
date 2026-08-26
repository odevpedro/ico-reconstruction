# Rev.110 — Rendering Pipeline Map (ICO PS2)

## Date
2026-08-26

## Objective
Complete mapping of the ICO PS2 rendering pipeline for native PC port design.
Identify all rendering subsystems, display list architecture, GS state management,
VU0/VU1 programs, and camera system integration.

## Scope
- 12 rendering modules identified via PAL-USA reconciliation
- ~200 rendering-related functions cataloged
- Display list architecture documented
- GS register usage patterns identified
- VU0/VU1 transform pipeline traced
- Camera system DL management mapped
- Environment effects cataloged

## Sources Used
- PAL-USA reconciliation (4368 function records)
- Ghidra symbol import (2886 verified symbols)
- Byte-exact assembly files (1224 functions)
- Runtime captures (PCSX2 instrumentation)
- AGENTS.md architectural context

## Evidence
- Module boundaries from MAIN.MAP object grouping
- Function addresses from ELF symbol table
- Assembly ground truth from `src/core/asm/`
- GS register space: `0x120000xx`
- VU0 math library: `0x243xxx-0x244xxx`
- DMA/VIF1: `0x244xxx-0x245xxx`

---

## 1. Rendering Module Map

| Module | Address Range | Functions | Role |
|--------|---------------|-----------|------|
| GifPacket.o | 0x10FD98-0x1129C8 | 38 | GIF packet building, 2D sprites, lines, points |
| GsBase.o | 0x112A30-0x115718 | 18 | GS system init/reset, frame buffer, brightness, clipping |
| Light.o | 0x115900-0x118010 | 8 | Lighting system |
| Matrix.o | 0x118140-0x1192A8 | 62 | Matrix math, VU0 camera/light/screen matrices |
| MicroCode.o | 0x119458-0x119788 | 4 | VU microcode management |
| Packet.o | 0x1199B0-0x11C750 | 5 | Packet construction |
| Primitive.o | 0x11C7B8-0x11EC78 | 12 | Primitive rendering (sprites, spheres, boxes, particles) |
| RegistPacket.o | 0x1223D8-0x122EF8 | 9 | Registered packet management, matrix, material, dissolve |
| Shadow.o | 0x122F48-0x128A58 | 11 | Shadow rendering (cluster, normal, cancel) |
| StageAnimation.o | 0x128CF0-0x12B0D8 | 23 | Stage model animation |
| Texture.o | 0x12B168-0x130470 | 22 | Texture loading, CLUT, TM2, VRAM transfer |
| ZFog.o | 0x1304E8-0x131230 | 3 | Z-fog system |

## 2. Display List Architecture

### 2.1 Primary Lists (8 lists)

The `iosOmCreateDL` system manages 8 primary display lists:
- `0x281A70`: 8 list heads (GObj handles)
- `0x281A90`: 8 list tails (GObj handles)
- `0x281AB0`: 8 DL heads (display list pointers)
- `0x281AD0`: 8 DL tails (display list pointers)

Each GObj has:
- `+0x48` (callback): DL callback function pointer
- `+0x4C` (slot_mask): bitmask for slot assignment
- `+0x50` (type_mask): type bits for filtering

### 2.2 Display List Dispatch Chain

```
iosOmCreateDL (0x13FC00, 264B)
  -> per-GObj display list dispatcher
  -> 32-bit control mask scan
  -> dereference DL heads at 0x281AB0

iosOmExeEachGObj (0x13FD10)
  -> linked-list walker
  -> fires N events per slot
  -> callback via GObj+0x48

dispatchAll()
  -> dispatchActiveLists() -- 8 primary lists via bitmask
  -> dispatchTypeSlots() -- type slots 19-27 per GObj
```

### 2.3 Camera Display List System

Camera DL management is separate from the primary lists:
- `isysGObjMoveCameraDL` (0x1FC168) -- moves camera display list
- `isysGObjMoveCameraDLHead` (0x1FC2E0) -- sorted priority insertion
- `isysGObjLinkCameraDL` (0x1FC1A0) -- camera DL linking
- `isysObjMoveCameraDLAfterGObj` (0x1FC460) -- insert after
- `isysObjMoveCameraDLBeforeGObj` (0x1FC4C0) -- insert before
- `add_gobj_to_tailCameraDL` (0x1FC048) -- tail insertion

## 3. GIF Packet System (GifPacket.o)

The GIF (Graphics Interface) is PS2's DMA-based GPU command protocol.

### 3.1 Packet Lifecycle

```
gif_StartPacket -> gif_EndPacket
gif_Open2DPacket -> gif_Send2DPacket -> gif_Close2DPacket
gif_OpenDirectPacket -> gif_CloseDirectPacket
```

### 3.2 Drawing Primitives

| Function | Description |
|----------|-------------|
| gif_DrawLine | Line drawing |
| gif_DrawLine2D | 2D line drawing |
| gif_DrawFlatSprite | Flat-shaded sprite |
| gif_DrawSprite | Textured sprite |
| gif_DrawGouraudSprite | Gouraud-shaded sprite |
| gif_DrawGouraudFlatSprite | Gouraud flat sprite |
| gif_DrawTile | Tile drawing |
| gif_DrawPoint | Point drawing |
| gif_DrawLidar | Lidar visualization |
| gif_DrawBackSprite | Background sprite |

### 3.3 FIFO vs Direct Mode

GIF supports two transfer modes:
- **FIFO mode**: gif_PutLineFifo, gif_PutGouraudLineFifo, gif_PutSpriteFifo,
  gif_PutGouraudSpriteFifo
- **Direct mode**: gif_PutLineDirect, gif_PutGouraudLineDirect, gif_PutSpriteDirect,
  gif_PutGouraudSpriteDirect, gif_PutFlatSpriteDirect, gif_PutGouraudFlatSpriteDirect

## 4. GS Base System (GsBase.o)

### 4.1 Initialization

| Function | Address | Description |
|----------|---------|-------------|
| gsb_InitGSSystem | 0x113E60 | Initialize GS subsystem |
| gsb_ResetGSSystem | 0x114080 | Reset GS state |
| gsb_InitVideoMode | -- | Initialize video mode |
| gsb_SetVideoMode | -- | Set video resolution/format |

### 4.2 Frame Buffer Management

| Function | Address | Description |
|----------|---------|-------------|
| gsb_SetDisplay | -- | Set display environment |
| gsb_SetDrawEnv | -- | Set draw environment |
| gsb_SetDispArea | -- | Set display area |
| gsb_ClearFrameBuffer | 0x114FB8 | Clear frame buffer |
| gsb_SetZBuffer | -- | Set Z-buffer configuration |
| gsb_SetBrightness | -- | Set screen brightness |
| gsb_SetClipping | -- | Set clipping region |

### 4.3 PS2 SDK GS Functions (Embedded)

| Function | Address | Description |
|----------|---------|-------------|
| sceGsResetGraph | 0x241B28 | Reset GS graph state |
| sceGsResetPath | 0x241C48 | Reset GS data path |
| sceGsSetDefDispEnv | 0x241CB0 | Define display environment |
| sceGsPutDispEnv | 0x241F20 | Apply display environment |
| sceGsSetDefDrawEnv | 0x241FE0 | Define draw environment |
| sceGsPutDrawEnv | 0x2421C8 | Apply draw environment |
| sceGsSetDefDBuff | 0x2422B0 | Define double buffer |
| sceGsSwapDBuff | 0x242548 | Swap double buffers |
| sceGsSyncV | 0x2425A8 | Wait for VBlank |
| sceGsSyncPath | 0x242640 | Wait for DMA path |
| sceGsSetDefStoreImage | 0x242CB0 | Define store image |
| sceGsExecLoadImage | 0x242DF0 | Execute load image |
| sceGsExecStoreImage | 0x242F70 | Execute store image |
| sceGsPutIMR | 0x243600 | Write interrupt mask register |
| sceGsGetIMR | 0x2436C8 | Read interrupt mask register |

## 5. Transform Pipeline (VU0)

### 5.1 Matrix Operations (Matrix.o, 62 functions)

Key functions: mtx_Identity, mtx_Copy, mtx_Multiply, mtx_Inverse,
mtx_Transpose, mtx_RotX/Y/Z, mtx_Scale, mtx_Translate,
mtx_LookAt, mtx_Perspective, mtx_Ortho, mtx_Rotation.

### 5.2 Camera/Light/Screen Matrices

| Function | Address | Description |
|----------|---------|-------------|
| _SetCameraMatrix | 0x118818 | Set camera matrix |
| mc_SetMicroCode | 0x118C68 | Set VU microcode |
| mc_TransMicroCode | 0x118E88 | Transfer microcode |
| mc_SetupVU0 | -- | Setup VU0 context |
| mc_SendVU0 | -- | Send data to VU0 |

### 5.3 VU0 Math Library (PS2 SDK)

| Function | Address | Description |
|----------|---------|-------------|
| sceVu0RotMatrixZ | 0x243C78 | VU0 rotation Z |
| sceVu0RotMatrix | 0x243E70 | VU0 rotation matrix |
| sceVu0CameraMatrix | 0x243EE8 | VU0 camera matrix |
| sceVu0NormalLightMatrix | 0x243F98 | VU0 normal light matrix |
| sceVu0LightColorMatrix | 0x244058 | VU0 light color matrix |
| sceVu0ViewScreenMatrix | 0x2440C0 | VU0 view-screen matrix |
| sceVu0DropShadowMatrix | 0x2441C8 | VU0 drop shadow matrix |

### 5.4 Transform Orchestrator

| Function | Address | Size | Description |
|----------|---------|------|-------------|
| Transform orchestrator | 0x1D4A58 | 224B | VU0 ring-buffer packetizer |
| VU0 ring buffer builder | 0x1D43F8 | -- | Ring buffer at 0x4C7710 |
| Matrix/quaternion setup | 0x1D45B0 | -- | Entity data to transform |
| VU0 sync stub | 0x117C40 | -- | Synchronize VU0 |
| Conditional matrix copy | 0x118430 | -- | Conditional matrix copy |

**Two-pass transform submission pattern**: Different constant data per pass.

## 6. Primitive Rendering (Primitive.o)

| Function | Address | Description |
|----------|---------|-------------|
| prim_DrawSprite | -- | Draw sprite |
| prim_DrawGouraudSprite | -- | Draw gouraud sprite |
| prim_DrawBox | -- | Draw box |
| prim_DrawGouraudBox | -- | Draw gouraud box |
| prim_DrawSphere | -- | Draw sphere |
| prim_DrawGouraudSphere | -- | Draw gouraud sphere |
| prim_DrawWireSphere | 0x11E220 | Draw wireframe sphere |
| prim_DrawWireBox | 0x11E4F8 | Draw wireframe box |
| prim_InitParticle | 0x11E708 | Initialize particle |
| prim_DrawLine | -- | Draw line |
| prim_DrawGouraudLine | -- | Draw gouraud line |
| prim_DrawPoint | -- | Draw point |
| prim_DrawTriangle | -- | Draw triangle |

## 7. Registered Packet System (RegistPacket.o)

### 7.1 Shape/Material Setup

| Function | Address | Description |
|----------|---------|-------------|
| reg_setShape | 0x11E728 | Set shape |
| reg_setNMatrixPacket | 0x11F040 | Set normal matrix packet |
| reg_setMMatrixPacket | 0x11F468 | Set model matrix packet |
| reg_setCMatrixPacket | 0x11F8E8 | Set camera matrix packet |
| reg_transMaterialPacket | 0x11FCD8 | Transform material packet |
| reg_TransTexturePacket | 0x122C50 | Transform texture packet |

### 7.2 Object Display

| Function | Address | Description |
|----------|---------|-------------|
| _reg_disp | 0x11FC28 | Internal dispatch |
| reg_dispNObj | 0x11FF60 | Display normal object |
| reg_dispMObj | 0x120308 | Display material object |
| reg_dispSObj | 0x1207E8 | Display shadow object |
| reg_dispCObj | 0x120B28 | Display camera object |
| reg_dispPoint | 0x120DF8 | Display point |
| reg_dispLine | 0x121298 | Display line |
| reg_dispPointLineObj | 0x1217C8 | Display point-line object |
| reg_DispAccessoryWithShadow | 0x121D90 | Display accessory with shadow |
| reg_RenderReflection | 0x122148 | Render reflection |
| reg_DispEnemy | 0x1224E0 | Display enemy |
| reg_DispMultiPri | 0x1227C8 | Display multi-priority |

### 7.3 Effects

| Function | Address | Description |
|----------|---------|-------------|
| reg_setDissolve | 0x11FD28 | Set dissolve effect |
| reg_resetDissolve | 0x11FF30 | Reset dissolve effect |

## 8. Shadow System (Shadow.o)

| Function | Address | Description |
|----------|---------|-------------|
| shadow_Init | 0x1287E8 | Initialize shadow system |
| shadow_Render | 0x123C00 | Render shadows |
| shadow_getShadowVectorAverage | 0x123C20 | Calculate shadow vector average |
| shadow_EntryClusterShadow | 0x123C70 | Entry cluster shadow |
| shadow_EntryNormalShadow | 0x1240D0 | Entry normal shadow |
| __GetCameraPos | 0x1243E8 | Get camera position for shadow |
| shadow_MakeObjectData | 0x128050 | Make shadow object data |
| shadow_Tool | 0x1283B8 | Shadow debug tool |
| shadow_DispCancel | 0x128710 | Cancel shadow display |

## 9. Texture System (Texture.o)

### 9.1 Initialization

| Function | Address | Description |
|----------|---------|-------------|
| tex_Init | 0x130128 | Initialize texture system |
| tex_initTM2 | 0x12C3D0 | Initialize TM2 format |
| tex_initTextureSub | 0x12CEF8 | Initialize texture subsystem |

### 9.2 VRAM Management

| Function | Address | Description |
|----------|---------|-------------|
| tex_AllocVramAuto | 0x12AED0 | Auto-allocate VRAM |
| tex_transVramClutTex | 0x12B850 | Transfer CLUT to VRAM |
| tex_transVramDirectTex | 0x12BA68 | Transfer direct texture to VRAM |
| tex_transRegister | 0x12BBE8 | Transfer registers |
| tex_transTM2 | 0x12BC10 | Transfer TM2 data |

### 9.3 Texture Loading

| Function | Address | Description |
|----------|---------|-------------|
| tex_LoadTexture | 0x12FD50 | Load texture |
| tex_LoadTexturePart | 0x12D218 | Load texture part |
| tex_loadImage | 0x12AFC0 | Load image |
| tex_transTexture | 0x12D370 | Transfer texture |
| tex_setTexReg | 0x12B1C0 | Set texture register |
| tex_setRegisters | 0x12C050 | Set texture registers |

### 9.4 Texture Operations

| Function | Address | Description |
|----------|---------|-------------|
| tex_convertImage | 0x12C6F0 | Convert image format |
| tex_makeCopyImage | 0x12C8F8 | Make copy of image |
| tex_makeTexturePacket | 0x12CB70 | Make texture packet |
| tex_textureAnimation | 0x12DA48 | Texture animation |
| tex_FreeTexture | 0x12DE80 | Free texture |
| tex_SetUVScroll | 0x12FFF0 | Set UV scroll effect |

### 9.5 Debug/Info

| Function | Address | Description |
|----------|---------|-------------|
| tex_dispClut | 0x12E0D8 | Display CLUT |
| tex_printTexture | 0x12E528 | Print texture info |
| tex_Tool | 0x12E940 | Texture debug tool |
| tex_ListTool | 0x12F818 | Texture list tool |
| tex_GetTWTH | 0x12FCE0 | Get texture width/height |
| tex_GetTextureNo | 0x12FD70 | Get texture number |
| tex_GetTexExtData | 0x12FE60 | Get texture extended data |

## 10. Lighting System (Light.o)

| Function | Description |
|----------|-------------|
| lgt_Init | Initialize lighting |
| lgt_SetLight | Set light |
| lgt_SetAmbient | Set ambient light |
| lgt_SetDiffuse | Set diffuse light |
| lgt_SetSpecular | Set specular light |
| lgt_SetLightMatrix | Set light matrix |
| lgt_SetLightDirection | Set light direction |
| lgt_CalcLight | Calculate lighting |

## 11. VIF1/DMA Interface

| Function | Address | Description |
|----------|---------|-------------|
| OpenVif1DirectPacket | 0x1AD788 | Open VIF1 direct packet |
| CloseVif1DirectPacket | 0x1AB338 | Close VIF1 direct packet |
| SendVif1DirectPacket | 0x1AB378 | Send VIF1 direct packet |
| SetDrawEnvironment | 0x1AB670 | Set draw environment via VIF1 |
| drawSprite | 0x1AB730 | Draw sprite via VIF1 |
| drawWin | 0x1AC138 | Draw window via VIF1 |
| display | 0x1ACCD0 | Display via VIF1 |

### DMA Functions (PS2 SDK)

| Function | Address | Description |
|----------|---------|-------------|
| sceDmaReset | 0x244658 | Reset DMA |
| sceDmaSend | 0x244980 | Send DMA data |
| sceDmaSendN | 0x244A58 | Send N words via DMA |
| sceDmaRecv | 0x244C28 | Receive DMA data |
| sceDmaSync | 0x244ED0 | Synchronize DMA |
| sceDmaWatch | 0x244F98 | Watch DMA |

### VIF1 Functions (PS2 SDK)

| Function | Address | Description |
|----------|---------|-------------|
| sceVif1PkCnt | 0x245120 | VIF1 packet count |
| sceVif1PkOpenDirectCode | 0x2451D0 | VIF1 open direct code |
| sceVif1PkAlign | 0x245318 | VIF1 packet align |

## 12. Display List Management

| Function | Address | Description |
|----------|---------|-------------|
| dl_Init | 0x1FB4C8 | Initialize display list |
| dl_Swap | 0x1FB658 | Double-buffer swap |
| dl_Out | 0x1FB9F8 | Display list output |
| dpk_SwapBuffer | 0x1FBB48 | Packet buffer swap |
| dpk_CheckBufferSize | 0x1FBC18 | Check buffer size |

## 13. Camera System

| Function | Address | Description |
|----------|---------|-------------|
| ChaseCamera | 0x188C98 | Chase camera |
| CameraMove | 0x188E30 | Camera movement |
| CameraSetCameraSet | 0x188228 | Set camera settings |
| ico2camera_GetTargetPos | 0x1882D0 | Get camera target position |
| ico2camera_GetGroupNearest | 0x188550 | Get nearest camera group |
| SetCameraTargetPosition | 0x18B668 | Set camera target position |
| SetWSMatrix | 0x18B700 | Set world-screen matrix |
| ConvertCameraSet | 0x18B738 | Convert camera settings |
| CameraEditManual | 0x18B9D0 | Manual camera edit |
| DebugCameraSemiAuto | 0x18BEB0 | Debug semi-auto camera |
| BackToGameCamera | 0x18BF70 | Return to game camera |
| Camctrl_SetTarget | 0x18CAA8 | Set camera control target |
| GetCameraDefaultTargetGObj | 0x18CAD8 | Get default camera target GObj |
| CameraChangeTargetParallel | 0x18CB20 | Change camera target parallel |
| CameraSetMode | 0x18CC00 | Set camera mode |
| GetCameraInfo_tmp | 0x18CF80 | Get camera info |
| UpdateRootPosition | 0x18CFF8 | Update root position |

## 14. Post-Processing Effects

| Function | Address | Description |
|----------|---------|-------------|
| blur | 0x1EBE48 | Blur effect |
| depthField | 0x1EE860 | Depth of field |
| calcSun | 0x1EF118 | Sun calculation |
| dispPostInfo | 0x1EF398 | Display post info |
| dispFeedInfo | 0x1EF508 | Display feed info |
| FullScreenEffectAfter | 0x1EF770 | Full-screen after effect |
| InitStaticBlur | 0x1EF8E8 | Initialize static blur |
| SetDepthFadeParam | 0x1EF9A8 | Set depth fade parameters |
| _initStaticBlur | 0x1EF9E0 | Internal static blur init |
| SetStaticBlur | 0x1FD530 | Set static blur |
| makeFullScreenFlareAfter | 0x1FBC48 | Make full-screen flare |

## 15. Environment Effects

| Function | Address | Description |
|----------|---------|-------------|
| InitSugiLeafGeo | 0x1F16A0 | Initialize cedar leaf geometry |
| SugiLeafGeo | 0x1F16F8 | Cedar leaf geometry |
| SugiLeafGeo2 | 0x1F1330 | Cedar leaf geometry 2 |
| SugiLeafDL2 | 0x1F1508 | Cedar leaf display list |
| InitWindField | 0x1F44E0 | Initialize wind field |
| ExecWindField | 0x1F4950 | Execute wind field |
| drawSenpuuki | 0x1F4800 | Draw whirlwind |
| InitStormPackage | 0x1EFA60 | Initialize storm particle system |
| ClipStormByVolume | 0x1EFD18 | Clip storm by volume |
| ClipStormByCamera | 0x1EFEE0 | Clip storm by camera |
| UpdateStormPackage | 0x1F0098 | Update storm package |
| DispStormPackage | 0x1F0260 | Display storm package |
| LightTorchOn | 0x1F1868 | Turn on torch light |
| LightTorchOff | 0x1F19F0 | Turn off torch light |
| moveTorch | 0x1F1AD8 | Move torch |
| SetActressLight | 0x1BB8C0 | Set character lighting |

## 16. Text/Debug Rendering

| Function | Address | Description |
|----------|---------|-------------|
| font_CheckAlign | 0x1FAAB0 | Check font alignment |
| font_GetWidth | 0x1FB478 | Get font width |
| DispWireLetter | 0x1F5378 | Display wire letter |
| DispWireString | 0x1F54D0 | Display wire string |
| DefaultColorWireString | 0x1F55D8 | Default color wire string |
| ChangeColorWireString | 0x1F5608 | Change color wire string |
| Draw2DBox | 0x1F5620 | Draw 2D box |
| DebugDispBox | 0x183300 | Debug display box |
| DispCameraGroup | 0x183968 | Display camera group |
| drawXZArrow | 0x184138 | Draw XZ arrow |
| DispAxisArrow | 0x184290 | Display axis arrow |

## 17. Rendering Architecture Summary

### 17.1 Frame Lifecycle

```
VBlank interrupt (0x23EE28)
  -> ACTGame (0x1A63E0)
    -> backStageProcessMain (0x1A05D0)
    -> stage_ApplyData (0x12A1D8)
    -> kanbanExec (scene loader)
    -> isysGObj system dispatch
      -> iosOmCreateDL (per-GObj DL)
        -> iosOmExeEachGObj (DL walker)
          -> GObj callback (+0x48)
            -> gif_* / prim_* / reg_* / tex_*
              -> VIF1/DMA -> GS hardware
```

### 17.2 Rendering Abstraction Layers

For a native PC port, the rendering stack maps as follows:

```
PS2 Architecture          ->  PC Abstraction
----------------------------------------------------
GIF packets (DMA)         ->  Command buffer / draw calls
VU0 transforms            ->  CPU SIMD or GPU compute
VU1 display lists         ->  GPU command buffer
GS register state         ->  OpenGL/Vulkan/D3D state
Frame buffer (double)     ->  Swap chain
Z-buffer                  ->  Depth buffer
Texture VRAM              ->  GPU textures
CLUT (palette)            ->  Texture format conversion
Lighting (local)          ->  Shader uniforms
Shadow mapping            ->  Shadow maps
Post-processing           ->  Framebuffer effects
```

### 17.3 Priority Decompile Order for Native Port

1. **GifPacket.o** (38 functions) -- GIF packet building is the gateway to all rendering
2. **GsBase.o** (18 functions) -- GS init/reset is needed for any rendering backend
3. **Texture.o** (22 functions) -- Texture loading is essential for materials
4. **Primitive.o** (12 functions) -- Core drawing primitives
5. **Matrix.o** (62 functions) -- Transform pipeline (may use VU0 math or CPU fallback)
6. **Light.o** (8 functions) -- Lighting for shaders
7. **RegistPacket.o** (9 functions) -- Object display dispatch
8. **Shadow.o** (11 functions) -- Shadow rendering
9. **ZFog.o** (3 functions) -- Fog effect
10. **StageAnimation.o** (23 functions) -- Scene model animation

### 17.4 Conservative Verdict

**Confirmed:**
- 12 rendering modules with ~200 functions identified
- GIF packet system is the gateway to all PS2 GPU commands
- VU0 handles transform math (62 matrix functions)
- Camera has its own display list system separate from primary lists
- Double-buffered frame management via dl_Swap / dpk_SwapBuffer
- TM2 format for texture storage with CLUT support

**Probable:**
- The rendering pipeline follows the standard PS2 pattern: CPU builds GIF packets -> VIF1 DMA -> VU1 transform -> GS rasterize
- VU1 microcode handles vertex transform and clipping (needs direct probe)
- The 8 primary lists likely map to rendering layers (opaque, transparent, shadow, UI, etc.)

**Possible:**
- VU1 microcode may be custom or use Sony standard (needs binary extraction)
- Some post-processing effects may use GS feedback loops

**Unknown:**
- Exact VU1 microcode programs (need binary extraction from IOP/EE memory)
- GS register configuration details (need runtime memory dump)
- Exact mapping of 8 primary lists to rendering layers

**Next minimum test:**
Disassemble `gsb_InitGSSystem` (0x113E60) and `dl_Init` (0x1FB4C8) to understand
the rendering initialization sequence and GS register configuration.
