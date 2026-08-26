# Native PC Port Roadmap

**Date:** 2026-08-25  
**Objective:** Define the technical transition from verified reverse engineering to a native PC port of ICO  
**Status:** Foundation phase — reverse engineering complete for core systems, port not yet started  

---

## 1. Final Goal

Build a native PC executable that runs reconstructed ICO game logic against modern platform services (OpenGL/Vulkan rendering, SDL input, native audio, host filesystem).

The current reverse engineering work is the **foundation** for this port. Byte-exact `.s` assembly is valuable as documentation and behavioral ground truth, but it is not the final portable form. Every verified function must eventually be converted to readable C/C++ or replaced with a native equivalent.

---

## 2. Current State

### What exists

| Asset | Count | Status |
|-------|-------|--------|
| Byte-exact .s functions | 1.224 | Verified, not portable |
| Entity work area struct | ~0x0680 bytes | Partially reconstructed |
| GObj struct | 0x174 bytes | Documented (Rev.098-099) |
| Descriptor table | 68 entries | Mapped |
| World state system | 31 states | Runtime validated |
| DL slot system | 29 slots | Runtime validated |
| IOP SDK mapping | 150 functions | Mapped, not decompiled |
| Runtime sessions | 6.7hr gameplay | Validated |

### What is NOT port-ready

| System | Status | Gap |
|--------|--------|-----|
| Rendering | Not started | PS2 GS → modern GPU |
| Audio | Not started | SPU2/IOP → native audio |
| Input | Not started | scePad → SDL |
| Filesystem | Not started | CDVD → host FS |
| Memory allocators | Not started | PS2 heap → native |
| Timing | Not started | vblank → game loop |
| Math library | Not started | VU0 → SSE/NEON |
| Game logic | 28% decompiled | .s only, not C |

---

## 3. What Already Helps the Port

### Architecture knowledge

The reverse engineering has revealed the engine's internal architecture:

- **isysGObj* system** (36 functions): complete lifecycle — init, alloc, add, dispatch, remove
- **Entity system**: descriptor table (68 types), handler callbacks (hA/hB/hC/hD), work area structs
- **Scene loading**: kanban.c pipeline (21 stages, 7 unique handlers)
- **Main loop**: vblankHandler → ACTGame → backStageProcessMain / kanbanExec
- **Dispatch model**: per-area DL slots, 17-slot mask+type system

This knowledge is **essential** for designing the port's architecture. Without it, we'd be reverse engineering while coding — much slower.

### Verified data structures

| Structure | Size | Fields known |
|-----------|------|--------------|
| GObj | 0x174 | type, state, callback, slot_mask, user_data_ptr |
| Entity work area | ~0x0680 | type, state, anim, timer, position, motion, AI, scene |
| Process node (TCB) | 0x94 | callback, priority, type_mask, active_flag |
| Descriptor entry | 0x64 | init_fn, hA, hB, hC, hD, vtable |

These structures define the **API surface** of the game logic. A port can implement these structs in C and use the same data flow.

### Verified behaviors

- Entity lifecycle: spawn → init → register callbacks → per-frame update → remove
- Scene transitions: world_state changes trigger scene reload
- Dispatch: iosOmMain iterates 17 slots, fires registered callbacks
- Collision: _Clip with _clipW* callback variants

---

## 4. What Does NOT Help the Port Directly

### Byte-exact .s assembly

1.224 functions are verified as byte-exact PS2 MIPS assembly. This is:
- **Valuable** as documentation of exact behavior
- **Valuable** as ground truth for C reimplementation
- **Not portable** — PS2 MIPS cannot compile on x86/ARM

### Runtime probe data

6.7 hours of gameplay logs validate the architecture but:
- Do not provide runnable code
- Do not replace C implementation
- Are useful for testing the port's behavior matches

### Ghidra analysis

Function boundaries, xrefs, and naming are useful for understanding but:
- Ghidra output is not compilable
- Auto-generated names need human verification
- Decompiled C from Ghidra is often wrong (PS2-specific patterns)

---

## 5. `.s` Byte-Exact vs C/C++ Portable

| Aspect | .s byte-exact | C/C++ portable |
|--------|---------------|----------------|
| Compiles on PC | No (PS2 MIPS) | Yes |
| Behavior accuracy | 100% verified | Must be tested |
| Readability | Low (assembly) | High (source) |
| Maintainability | None | Full |
| Port contribution | Documentation only | Direct runtime |
| Effort to produce | Medium (pipeline) | High (semantic understanding) |

**Key insight:** A function can be 100% verified as `.s` and still be 0% useful for the port until it's reimplanted in C. The `.s` files are **specifications**, not **implementations**.

### Conversion strategy

For each verified `.s` function:
1. Study the assembly to understand the algorithm
2. Write equivalent C/C++ code
3. Compile and test against PS2 behavior (using runtime logs as ground truth)
4. Verify output matches expected behavior

This is the **semantic reconstruction** phase — understanding what the code does, not just what bytes it produces.

---

## 6. PS2 Platform Dependencies Requiring Replacement

### Rendering / GS

| PS2 System | Replacement | Complexity |
|------------|-------------|------------|
| Graphics Synthesizer | OpenGL 3.3+ / Vulkan | Very High |
| VU0/VU1 programs | Vertex/fragment shaders | High |
| GS packet delivery | GPU command buffers | High |
| Framebuffer management | Render targets | Medium |
| Z-buffer | Depth buffer | Low |
| Texture formats (PSMCT32/16) | GPU textures | Medium |
| Alpha blending | Standard blending | Low |
| clipping / scissoring | Standard scissor | Low |

**Critical path:** The rendering pipeline is the largest single component. ICO uses GS for 3D rendering with VU0 for vertex transformation and VU1 for clip/draw.

### Audio / SPU2 / IOP

| PS2 System | Replacement | Complexity |
|------------|-------------|------------|
| SPU2 audio | SDL_mixer / FMOD / Wwise | High |
| IOP audio threads | Native audio callbacks | Medium |
| ADPCM codec | PCM conversion | Low |
| Audio routing | Mixer pipeline | Medium |

### Input / scePad

| PS2 System | Replacement | Complexity |
|------------|-------------|------------|
| scePad (DualShock 2) | SDL_GameController | Low |
| Analog sticks | SDL thumbsticks | Low |
| Pressure sensitivity | Not available on PC | N/A |
| Rumble | SDL haptic | Low |

**Low complexity** — SDL provides a clean abstraction.

### Filesystem / CDVD / sceFs

| PS2 System | Replacement | Complexity |
|------------|-------------|------------|
| CDVD disc read | Host filesystem | Low |
| sceOpen/sceRead/sceWrite | stdio/fopen/fread | Low |
| sceFs filesystem | Virtual filesystem | Medium |
| File caching | OS cache | Low |

**Key decision:** Extract game assets to a host directory tree, then use standard file I/O.

### SIF RPC

| PS2 System | Replacement | Complexity |
|------------|-------------|------------|
| EE→IOP communication | Direct function calls | Medium |
| sceSifCallRpc | Native function dispatch | Low |
| sceSifBindRpc | Service registration | Low |
| DMA transfers | Memory copies | Low |

**Key insight:** The IOP is a separate processor on PS2. On PC, all IOP code runs as native functions. SIF RPC becomes simple function calls.

### Memory Allocators

| PS2 System | Replacement | Complexity |
|------------|-------------|------------|
| EE heap (0x001xxxxx) | malloc/free | Low |
| Pool allocators | Custom pools | Medium |
| IOP heap | Not needed (unified memory) | N/A |
| Cache coherency (D-Cache) | Not needed on x86 | N/A |

### VBlank / Timing

| PS2 System | Replacement | Complexity |
|------------|-------------|------------|
| VBlank interrupt (60Hz) | Game loop timing | Low |
| VBlank counter (0x274EC0) | Frame counter | Low |
| Timer interrupts | std::chrono | Low |

---

## 7. Port Strategy in Phases

### Phase 1: Platform abstraction layer (PAL)

**Goal:** Define the interface between game logic and platform services.

```
┌─────────────────────────────┐
│       Game Logic (C++)      │
├─────────────────────────────┤
│    Platform Abstraction     │
├──────┬──────┬──────┬───────┤
│Render│Audio │Input │  FS   │
│  GL  │ SDL  │ SDL  │ stdio │
└──────┴──────┴──────┴───────┘
```

**Deliverables:**
- `platform/` directory with abstract interfaces
- `platform/render.h` — rendering API
- `platform/audio.h` — audio API
- `platform/input.h` — input API
- `platform/fs.h` — filesystem API
- `platform/time.h` — timing API

### Phase 2: Core engine reimplementation

**Goal:** Reimplement the isysGObj* system and main loop in C++.

**Priority order:**
1. `isysGObj*` (36 functions) — already fully documented
2. `iosOmMain` dispatch — 17-slot system
3. Main loop (vblankHandler → ACTGame → backStageProcessMain)
4. Scene loading (kanban.c)
5. Entity lifecycle (init, add, remove, dispatch)

**Deliverables:**
- `src/core/` — reimplemented engine core
- Unit tests comparing behavior against runtime logs

### Phase 3: Game logic reimplementation

**Goal:** Reimplement entity handlers (BOY, GIRL, ENEMY1, etc.) in C++.

**Priority order:**
1. BOY handler (hA/hB/hC + 564 callbacks)
2. GIRL handler (shared offsets with BOY)
3. ENEMY1 handler
4. Simple entities (BARREL, WOODBOX, etc.)
5. Remaining entity types

**Deliverables:**
- `src/entities/` — reimplemented entity handlers
- Behavioral tests against runtime logs

### Phase 4: Rendering backend

**Goal:** Replace PS2 GS with OpenGL/Vulkan.

**Sub-tasks:**
1. Mesh loading and rendering
2. Texture loading and binding
3. Shader programs (replacing VU0/VU1)
4. Camera and projection
5. Lighting
6. Alpha blending and transparency

### Phase 5: Platform integration

**Goal:** Wire everything together into a runnable PC executable.

**Sub-tasks:**
1. Asset extraction pipeline (textures, models, animations)
2. Input handling (SDL)
3. Audio (SDL_mixer)
4. Filesystem (host FS)
5. Game loop timing

### Phase 6: Polish and testing

**Goal:** Achieve feature parity with PS2 version.

**Sub-tasks:**
1. Visual comparison testing
2. Gameplay behavior verification
3. Performance optimization
4. Bug fixes

---

## 8. First Viable Prototype

### Minimum viable product (MVP)

A PC executable that:
1. Loads ICO's asset files from disk
2. Renders the first room (Beach, world_state 0x08) with basic 3D
3. Accepts controller input
4. Runs the BOY entity through the isysGObj* dispatch
5. Displays a simple scene with the player character

### MVP scope

| System | MVP implementation |
|--------|--------------------|
| Rendering | Basic 3D mesh rendering (no shaders) |
| Audio | Silence (audio deferred) |
| Input | Keyboard fallback |
| Filesystem | Host directory with extracted assets |
| Game logic | isysGObj* + BOY handler only |
| Scenes | Beach room only (0x08) |

### MVP timeline estimate

| Phase | Effort | Duration |
|-------|--------|----------|
| PAL design | 1 week | Week 1-2 |
| Core engine | 4 weeks | Week 3-6 |
| BOY handler | 3 weeks | Week 7-9 |
| Rendering | 4 weeks | Week 10-13 |
| Integration | 2 weeks | Week 14-15 |
| **Total MVP** | **~14 weeks** | **~3.5 months** |

---

## 9. Criteria for "Port-Ready" Function

A function is considered **port-ready** when:

| Criterion | Evidence |
|-----------|----------|
| Algorithm understood | Pseudocode or natural language description exists |
| Data structures documented | Struct layout verified (byte offsets, sizes) |
| C/C++ equivalent written | Compiles on PC (gcc/clang) |
| Behavior tested | Output matches runtime log for same inputs |
| Platform dependencies replaced | No PS2-specific instructions (COP0, COP1, VU0) |
| Performance acceptable | Runs at 60fps on target hardware |

### Function status categories

| Category | Meaning | Action |
|----------|---------|--------|
| `.s` verified | Byte-exact PS2 assembly | Study, then convert to C |
| `C` reconstructed | Readable C source | Test against runtime |
| `C` validated | C matches PS2 behavior | Port-ready |
| `native` | Replaced with platform API | Done |

---

## 10. Next 10 Practical Steps

| # | Task | Phase | Effort |
|---|------|-------|--------|
| 1 | Update AGENTS.md with native port goal | Foundation | 1 hour |
| 2 | Design platform abstraction interfaces | PAL | 1 week |
| 3 | Create `platform/` directory structure | PAL | 1 day |
| 4 | Implement `platform/fs.h` (filesystem) | PAL | 2 days |
| 5 | Implement `platform/time.h` (timing) | PAL | 1 day |
| 6 | Implement `platform/input.h` (SDL input) | PAL | 3 days |
| 7 | Begin isysGObj* C++ reimplementation | Core | 2 weeks |
| 8 | Extract first room assets (Beach) | Assets | 1 week |
| 9 | Basic 3D renderer (OpenGL) | Rendering | 3 weeks |
| 10 | Wire BOY handler to renderer | Integration | 1 week |

---

## Summary

The project has completed the **reverse engineering foundation** for ICO's core systems. The next phase is **semantic C++ reimplementation** guided by the verified `.s` assembly and runtime logs.

The path is: `.s` specification → C++ implementation → behavioral testing → platform integration.

This is a long-term project (6-12 months for a playable prototype), but the foundation is solid. Every verified function brings us closer to a native PC port.
