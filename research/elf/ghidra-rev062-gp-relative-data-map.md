# Rev.062 — GP-Relative Global Data Map

**Date:** 2026-05-16  
**Objective:** Map all global variables accessed via `$gp +/- offset` across the entire `.text` section — identifying sections, access patterns, writers, readers, and struct clusters.  
**Method:** Byte-level instruction scan + `.sdata`/`.sbss` initial value dump + section VA matching.  
**Status:** Complete.

---

## 1. GP Value: Definitively Resolved

**GP = 0x006388F0**

Found at `0x10005C` in the boot sequence:

```asm
0x00100034: lui   $4, 0x0064                # upper half
0x00100048: addiu $4, $4, -30480 (0x88F0)   # lower half (sign-extended)
0x0010005C: daddu $28, $4, $0              # $28 (gp) = 0x006388F0
```

**Relationship to sections:**

| Reference point | VA | Distance from GP |
|---|---|---|
| `.sdata` start | `0x00631900` | `GP - 0x6FF0` |
| `.sdata` end | `0x00633BC6` | `GP - 0x4D2A` |
| `.sbss` start | `0x00633C00` | `GP - 0x4CF0` |
| `.sbss` end | `0x00633FF4` | `GP - 0x48FC` |
| `.bss` start | `0x00634000` | `GP - 0x48F0` |

Not the conventional `.sdata + 0x7FF0`. The EE GCC 2.9 linker script for this binary places GP at `.sdata + 0x6FF0`.

---

## 2. Overall Statistics

| Metric | Value |
|---|---|
| Total GP accesses in `.text` | **11,547** |
| Unique GP offsets accessed | **2,131** |
| In `.lit4` (float constants) | **1,012 offsets, 1,015 accesses** |
| In `.sdata` (initialized small data) | **853 offsets, 8,669 accesses** |
| In `.sbss` (uninitialized small data) | **248 offsets, 1,831 accesses** |
| In `.bss` | **18 offsets, 32 accesses** |

---

## 3. Section-by-Section Breakdown

### 3.1 `.lit4` — Float Constant Literal Pool

**VA range:** `0x00630900` to `0x006318D0`  
**Accesses:** 1,015, all via LWC1/SWC1 (read-only floats)  
**GP offset range:** `-32752` to `-28708`  

These are compiler-generated float constants. The EMACS-style `GPRel` relocations in `.lit4` are resolved by the linker to GP-relative addresses. Not game variables — treated as read-only literal data.

### 3.2 `.sdata` — Initialized Small Data (853 variables)

**VA range:** `0x00631900` to `0x00633BC6`

#### 3.2.1 Entity/Object Pointer Array (gp[-28176] to gp[-28152])

| GP offset | VA | Name | Access count | Primary writers |
|---|---|---|---|---|
| `-28176` | `0x00631AE0` | `obj_ptr_0` | 2 | — |
| `-28172` | `0x00631AE4` | **`girl_obj_ptr`** | **1,260** | `descriptor_iteration` |
| `-28168` | `0x00631AE8` | **`other_obj_ptr`** | **986** | `descriptor_iteration` |
| `-28164` | `0x00631AEC` | `obj_ptr_2` | 113 | `scene_loader_approx` |
| `-28160` | `0x00631AF0` | `obj_ptr_3` | 8 | — |
| `-28156` | `0x00631AF4` | `obj_ptr_4` | 3 | `scene_loader_approx` |

All start at `0x00000000` (zero-initialized). The two most-accessed pointers (`-28172`, `-28168`) collectively account for **2,246 accesses** — by far the hottest globals. The `descriptor_iteration` function is the writer, linking the scene descriptor table to runtime object slots.

#### 3.2.2 World State Region (gp[-28592] to gp[-28300])

| GP offset | VA | Name | Access count | Notes |
|---|---|---|---|---|
| `-28592` | `0x00631940` | `state_field_0` | 28 | |
| `-28560` | `0x00631960` | `state_field_1` | 47 | Used by `heap_alloc` |
| `-28544` | `0x00631970` | `state_field_2` | 160 | `cloth_cop2_plane_clip`, `enemy1_hA`, `girl_hB` |
| `-28512` | `0x00631990` | **`world_state`** | **139** | `descriptor_iteration`, `boy_subfn_C` |

A run of ~264 bytes (66 consecutive 4-byte slots) all initialized to `0x00000000`. This is a **global state block** — likely the entity/behavior state machine root.

#### 3.2.3 Entity Descriptor Type Tags (scattered in `.sdata`)

**Non-zero initialized values decoded as big-endian ASCII 4-char tags:**

| GP offset | VA | Hex value | Tag | Meaning |
|---|---|---|---|---|
| `-28624` | `0x00631920` | `0x004C4150` | `"PAL"` | Palette descriptor |
| `-27760` | `0x00631C80` | `0x004A424F` | `"OBJ"` | Object descriptor |
| `-27488` | `0x00631D90` | `0x00414742` | `"BGA"` | Background/area descriptor |
| `-27452` | `0x00631DB4` | `0x00323354` | `"T32"` | Texture 32-bit |
| `-27444` | `0x00631DBC` | `0x00343254` | `"T24"` | Texture 24-bit |
| `-27436` | `0x00631DC4` | `0x00363154` | `"T16"` | Texture 16-bit |
| `-27432` | `0x00631DC8` | `0x004E4F4E` | `"NONE"` | Sentry |
| `-27408` | `0x00631DE0` | `0x004F4349` | `"ICO"` | ICO root/identifier |
| `-27228` | `0x00631E94` | `0x004B5041` | `"APK"` | Animation pack |
| `-27220` | `0x00631E9C` | `0x004C5041` | `"APL"` | Animation palette |
| `-27184` | `0x00631EC0` | `0x00382D43` | `"C-8"` | Color 8-bit |
| `-27176` | `0x00631EC8` | `0x00342D43` | `"C-4"` | Color 4-bit |
| `-27168` | `0x00631ED0` | `0x00323344` | `"D32"` | Depth 32-bit |
| `-27160` | `0x00631ED8` | `0x00343244` | `"D24"` | Depth 24-bit |
| `-27152` | `0x00631EE0` | `0x00363144` | `"D16"` | Depth 16-bit |
| `-27144` | `0x00631EE8` | `0x004E4F4E` | `"NON"` | Sentry |
| `-26564` | `0x0063212C` | `0x00444D43` | `"CMD"` | Command |
| `-26548` | `0x0063213C` | `0x00444150` | `"PAD"` | Padding/filler |
| `-25996` | `0x00632364` | `0x00454E45` | `"ENE"` | Enemy descriptor |
| `-25988` | `0x0063236C` | `0x004C5249` | `"IRL"` | ??? |
| `-25752` | `0x00632458` | `0x00494949` | `"III"` | ??? |
| `-25712` | `0x00632480` | `0x00444E45` | `"END"` | End/sentinel |
| `-25668` | `0x006324AC` | `0x00544341` | `"ACT"` | Action descriptor |
| `-25152` | `0x006326B0` | `0x0046464F` | `"OFF"` | Switch off |
| `-25060` | `0x0063270C` | `0x004D4143` | `"CAM"` | Camera descriptor |
| `-24888` | `0x006327B8` | `0x00544145` | `"EAT"` | ??? |
| `-24616` | `0x006328C8` | `0x004C4150` | `"PAL"` | Palette (dupe) |
| `-24544` | `0x00632910` | `0x00504D42` | `"BMP"` | Bitmap |
| `-23844` | `0x00632BCC` | `0x00545349` | `"IST"` | Initial state |
| `-23684` | `0x00632C6C` | `0x00545345` | `"EST"` | End state |
| `-22828` | `0x00632FC4` | `0x00314E4F` | `"ON1"` | ??? |
| `-22820` | `0x00632FCC` | `0x00324E4F` | `"ON2"` | ??? |
| `-22812` | `0x00632FD4` | `0x00334E4F` | `"ON3"` | ??? |
| `-21948` | `0x00633334` | `0x00455341` | `"ASE"` | ??? |
| `-21944` | `0x00633338` | `0x00425553` | `"SUB"` | Subtract |
| `-21936` | `0x00633340` | `0x00444441` | `"ADD"` | Add |
| `-21916` | `0x00633354` | `0x00422052` | `"R B"` | Register B (blue) |
| `-21908` | `0x0063335C` | `0x00472052` | `"R G"` | Register G (green) |
| `-21900` | `0x00633364` | `0x00522052` | `"R R"` | Register R (red) |
| `-21544` | `0x006334C8` | `0x00544948` | `"HIT"` | Hit/damage |
| `-21508` | `0x006334EC` | `0x00474E49` | `"ING"` | ??? |
| `-21256` | `0x006335E8` | `0x00534559` | `"YES"` | Yes |
| `-21012` | `0x006336DC` | `0x0052554C` | `"LUR"` | ??? |
| `-20976` | `0x00633700` | `0x004E5553` | `"SUN"` | Sun (light) |
| `-20784` | `0x006337C0` | `0x00414742` | `"BGA"` | BGA (dupe) |
| `-20768` | `0x006337D0` | `0x00464453` | `"SDF"` | ??? |
| `-20744` | `0x006337E8` | `0x0053574C` | `"LWS"` | ??? |
| `-19844` | `0x00633B6C` | `0x00534154` | `"TAS"` | ??? |
| `-19824` | `0x00633B80` | `0x004E4942` | `"BIN"` | Binary |

These are the **entity descriptor type system tag constants**, compared against runtime type IDs during scene loading. The `"OBJ"`, `"BGA"`, `"ENE"`, `"CAM"`, `"ACT"`, `"PAL"`, `"T32"` tags are checked in the descriptor reader to determine how to parse each entry in the table.

#### 3.2.4 Format/Debug Strings

Several null-terminated ASCII strings live in `.sdata`:

```
gp-28648: "host0:"
gp-28632: "NTSC"
gp-28616: "boot()"
gp-28608: "main"
gp-28008: "done."
gp-27992: "FALSE" (2 copies)
gp-27840: "%s : %s"
gp-27832: "%s : %d"
gp-27712: "COL "
gp-27704: "%s%11f"
gp-27696: "VEC "
gp-27688: "AMB "
gp-27672: "%02x"
gp-27656: "%12f "
gp-27632: " (%d)"
gp-27608: "%s_l"
gp-27600: "%s_ref"
gp-27384: "1:%s"
gp-27376: "%s.tm2"
gp-27200: "%s:%d"
gp-27192: "%s:%f"
```

These are used by the debug print system (`print_stub_disabled` at 0x1A6E28, which is patched to a no-op in retail). The format strings reveal debug output for vectors (`"VEC "`), colors (`"COL "`), ambient (`"AMB "`), and assert messages.

#### 3.2.5 GS (Graphics Synthesizer) Texture Parameters

```
"SELTEX"  — Select texture
"SCRL-U"  — Scroll U
"SCRL-V"  — Scroll V
"AMP-U "  — Amplitude U
"AMP-V "  — Amplitude V
"CS-BGN"  — Color cycle start
"CS-END"  — Color cycle end
"CS-SPD"  — Color cycle speed
"CS-STP"  — Color cycle step
"SHINE "  — Shine/specular
"SMPMAG"  — Magnification filter
"SMPMIN"  — Minification filter
"TEXFNC"  — Texture function
"ALPTST"  — Alpha test
"ALPFAI"  — Alpha fail
"MIPMAPK" — Mipmap K
"MIPMAPL" — Mipmap L
```

These match PlayStation 2 GS texture pipeline register names and debug control strings.

#### 3.2.6 Texture Format Strings

```
"PSMCT32"  — 32-bit color (RGBA)
"PSMCT24"  — 24-bit color (RGB)
"PSMCT16"  — 16-bit color
"PSMT8"    — 8-bit paletted
"PSMT4"    — 4-bit paletted
"NONE"     — No texture
```

These are PS2 GS pixel storage format names.

### 3.3 `.sbss` — Uninitialized Small Data (248 variables)

**VA range:** `0x00633C00` to `0x00633FF4`  
**Key variables:**

| GP offset | VA | Access count | Inference |
|---|---|---|---|
| `-18868` | `0x00633F3C` | **434** | **Cloth vertex/state pointer** |
| `-18864` | `0x00633F40` | 117 | Cloth secondary pointer |
| `-19612` | `0x00633C54` | 55 | Pool/allocator state root |
| `-19600` | `0x00633C60` | 28 | Pool state auxiliary |
| `-19528` | `0x00633CA8` | 4 | **`pool_base`** |
| `-19524` | `0x00633CAC` | 4 | **`pool_capacity`** |
| `-19520` | `0x00633CB0` | 9 | `event_feedback` state |
| `-19396` | `0x00633D2C` | 34 | Scene state field |
| `-19280` | `0x00633DA0` | 43 | Dual-use flag |

**`-18868` (0x00633F3C)** is the most interesting: 432 reads / 2 writes, unknown function access patterns. Its position at `.sbss+0x33C` and massive read volume suggest it is a pointer to per-frame cloth vertex data updated by the cloth physics pipeline.

---

## 4. Key Struct Cluster Analysis

### 4.1 Callback Pool System (`.sbss+0xA0` to `.sbss+0xD0`)

```
gp[-19536] (.sbss+0xA0): event_flag (17 acc, 16R/1W)
gp[-19532] (.sbss+0xA4): event_flag_2 (15 acc, 14R/1W)
gp[-19528] (.sbss+0xA8): pool_base (4 acc, 3R/1W) — writer: node_callback_storage
gp[-19524] (.sbss+0xAC): pool_capacity (4 acc, 3R/1W) — writer: node_callback_storage
gp[-19520] (.sbss+0xB0): event_feedback_state (9 acc, 5R/4W) — writer: event_feedback
gp[-19512] (.sbss+0xB8): event_feedback_arg (7 acc, 7R/0W)
gp[-19504] (.sbss+0xC0): event_feedback_flag (3 acc, 1R/2W) — writer: event_feedback
gp[-19500] (.sbss+0xC4): event_feedback_flag2 (3 acc, 1R/2W) — writer: event_feedback
```

### 4.2 Callback/Descriptor System State (`.sdata+0x8DC`)

```
gp[-26388] (.sdata+0x8DC):  callback_system_state (62 acc, 60R/2W)
gp[-26376] (.sdata+0x8E8):  INIT=0x00000001  — count/ID
gp[-26368] (.sdata+0x8F0):  INIT=0x00000030  — 48 (size? max count?)
gp[-26360] (.sdata+0x8F8):  INIT=0x000A7325  — packed config
gp[-26352] (.sdata+0x900):  INIT=0x00005010  — packed config  
gp[-26348] (.sdata+0x904):  INIT=0x001D9020  — FUNCTION POINTER (cloth VU0 micro-mode function)
gp[-26344] (.sdata+0x908):  INIT=0x0000000A  — 10 (count/max)
```

This is a **statically-initialized descriptor function table** for the callback/entity system. The function pointer at `gp-26348` (`0x001D9020`) uses VU0 micro-mode instructions (COP2 opcodes `0x4B`...) — a cloth physics SIMD function.

### 4.3 Seeker Update State (`.sdata+0x1C30` to `+0x1C68`)

```
gp[-21424] (.sdata+0x1C40):  seeker_state (52 acc, 50R/2W) — writer: seeker_update
gp[-21412] (.sdata+0x1C4C):  INIT=0x3F800000 (1.0f) — sprite brightness/scale
gp[-21396] (.sdata+0x1C5C):  INIT=0xFFFFFFFF — -1 sentinel
gp[-21388] (.sdata+0x1C64):  INIT=0xFFFFFFFF — sentinel
```

### 4.4 Cloth Physics Area (`.sbss+0x33C` to `+0x370`)

```
gp[-18868] (.sbss+0x33C):  CLOTH_VERTEX_PTR (434 acc) — primary cloth vertex buffer pointer
gp[-18864] (.sbss+0x340):  CLOTH_SECONDARY_PTR (117 acc)
gp[-18844] (.sbss+0x354):  auxiliary_param (14 acc)
gp[-18816] (.sbss+0x370):  cloth_counter (11 acc)
```

### 4.5 Render/Texture Flags (`.sdata+0x13A0` to `+0x13E8`)

```
gp[-23628] (.sdata+0x13A4):  flag_42 (42 acc)
gp[-23604] (.sdata+0x13BC):  texture_flag (150 acc, 142R/8W) — writer: print_stub_disabled
gp[-23592] (.sdata+0x13C8):  state_check_var (17 acc)
gp[-23568] (.sdata+0x13E0):  shadow_draw_state (24 acc)
```

---

## 5. Statically-Initialized Function Pointers in `.sdata`

| GP offset | VA | Pointer value | Likely function |
|---|---|---|---|
| `-26348` | `0x00632204` | `0x001D9020` | Cloth VU0 micro-mode SIMD |
| `-25856` | `0x006323F0` | `0x00167230` | EE LQ/SQ user (init/clear function) |
| `-25852` | `0x006323F4` | `0x00167258` | EE LQ/SQ user (init/clear variant) |
| `-20896` | `0x00633750` | `0x001F4C00` | Unknown (unanalyzed range) |

---

## 6. Write-Pattern Summary

| GP offset range | Section | Writeable? | Key writers |
|---|---|---|---|
| `-32752` to `-28708` | `.lit4` | NO (read-only float constants) | — |
| `-28596` to `-18960` | `.sdata` | YES (664 writable out of 853) | `descriptor_iteration`, `scene_loader_approx`, `print_stub_disabled` |
| `-18960` to `-18588` | `.sbss` | YES (240 writable out of 248) | `event_feedback`, `node_callback_storage` |

**`descriptor_iteration` is the primary writer** for the object pointer array (`gp[-28172]`, `-28168`). It links scene descriptors to runtime entity slots.

**`print_stub_disabled` writes to many flags** because it serves as a "debug print/assert" dispatcher that is patched to a no-op in retail — but still writes to state tracking flags.

**`scene_loader_approx` writes to** `gp[-28164]` (object ptr), `gp[-22372]` (scene timer), and several `.sbss` variables at `gp[-18988]` through `gp[-18948]` (scene load state).

---

## 7. Confirmed

1. **GP = 0x006388F0** (verified via boot code at `0x100034-0x10005C` and cross-checked against known `.sdata`/`.sbss` variables).
2. **~11,547 total GP accesses** across 2,131 unique offsets, with 1012 in `.lit4` (float constants) and 1101 in `.sdata`/`.sbss` (real variables).
3. **40 entity descriptor type tags** stored as big-endian 4-byte ASCII codes in `.sdata` — the descriptor reader matches runtime type fields against these constants.
4. **Object pointer array** at `gp[-28176]` through `gp[-28152]` (6 pointers, 0x18 bytes total). Two are hot (1260/986 accesses), four are cold.
5. **Function pointer** at `gp[-26348]` initialized to `0x001D9020` (cloth VU0 micro-mode function).
6. **World state block** at `.sdata+0x040` to `+0x0C0` (~264 bytes) of zero-initialized state variables.
7. **Non-zero initial values** in `.sdata`: type tags, format strings, texture parameters, function pointers, sentinels (`0xFFFFFFFF`).

## 8. Probable

1. `gp[-18868]` (`.sbss+0x33C`, 434 accesses) = cloth vertex buffer pointer, updated per-frame by cloth physics pass.
2. `gp[-19612]` (`.sbss+0x54`, 55 accesses) = root of a pool/allocator tracking state.
3. `gp[-25404]` (`.sdata+0xCB4`, 343 accesses, 338 writes) = print/state tracking counter (written by `print_stub_disabled`).
4. The 5 text texture format strings (`PSMCT32`, `PSMT8`, etc.) map to the GS pixel format enum used in texture setup functions.
5. The `"IST"` (`0x00545349`) and `"EST"` (`0x00545345`) tag constants correspond to "initial state" / "end state" in the entity descriptor lifecycle.

## 9. Unknown

1. The semantic meaning of many `.sdata` variables with zero initial values and unknown function access patterns (most of the 853).
2. Which specific entity fields correspond to `gp[-26848]` (135 acc), `gp[-26828]` (87 acc), and `gp[-26780]` (105 acc) — likely an entity struct area.
3. The actual value of `gp[-18868]` at runtime (in `.sbss`; no static initializer).
4. The function at `0x001F4C00` (function pointer from `.sdata+0x1E50`).

## 10. Discarded

1. **GP = `.sdata + 0x7FF0`** — the conventional formula is wrong for this binary. The real GP = `.sdata + 0x6FF0`.
2. **All `.lit4` accesses are game variables** — they are compiler-generated float constant pools, not mutable state.

---

## 11. Next Minimum Test

1. Cross-reference the 40 type tags against the descriptor table at `0x004D4188` to map each tag to a descriptor handler.
2. Runtime breakpoint at `descriptor_iteration` (`0x1B76F8`) to capture the actual pointer values written to `gp[-28172]` and `gp[-28168]`.
3. Write the `.sdata`/`.sbss` initialized-data snapshot to the source tree as a reference header file.
4. Check function `0x001D9020` against ICO-decomp symbol table for a name.
