# ICO Native Runtime Prototype

**This is not a playable port.** This is the first runtime scaffold for a future native PC port of ICO.

## What is this?

A minimal, compilable C++17 project that establishes the architecture where reconstructed game logic will eventually run. It provides:

- PS2 memory simulation
- Platform stubs (input, filesystem, timing, audio, render)
- SIF RPC stub
- IOP stub
- Game loop framework
- Logging system

## What is this NOT?

- A playable game
- An ELF loader
- A MIPS interpreter
- A complete PS2 emulator

## Problem this solves

The reverse engineering project has 688 / 710 functions verified as byte-exact PS2 assembly. Before those can be converted to portable C/C++, we need a runtime environment where they can execute. This scaffold is that environment.

## PS2 systems this intends to replace

| PS2 System | Stub | Current / Future Replacement |
|------------|------|-------------------|
| scePad | Input | SDL GameController (native) |
| CDVD/sceFs | FileSystem | Host filesystem |
| SPU2/IOP audio | AudioStub | SDL_mixer / FMOD |
| GS/VU rendering | — | OpenGL backend (default, real GLX window) |
| vblank/timing | Timing | std::chrono |
| EE heap | Ps2Memory | Native allocator |
| SIF RPC | SifRpcStub | Direct function calls |
| IOP | IopStub | Native functions |

## How to compile

The OpenGL backend is ON by default (`ICO_ENABLE_OPENGL=ON`). It requires X11
and OpenGL development headers. Headless builds (CI) may turn it off:

```bash
cmake -S native -B native/build
cmake --build native/build
# headless alternative:
cmake -S native -B native/build -DICO_ENABLE_OPENGL=OFF
```

## How to run

When OpenGL is enabled, `ico_native` opens a real GLX window (640&#215;448) and
draws a synthetic frame through the GIF command pipeline
(`GifPacketBridge` &#8594; `GifCommandExecutor` &#8594; `OpenGLBackend`) until
`--frames N` (default 180, &#8776;3&#160;s) is reached:

```bash
./native/build/ico_native --frames 300
```

Without OpenGL, the binary falls back to the headless runtime smoke loop
(3 frames, no window).

## First visible milestone (2026-09-05)

`ico_native` opens a real window and renders three animated colored quads at
60&#160;fps, driven entirely by the PS2-style GIF pipeline (GIF tag/command
model &#8594; executor &#8594; OpenGL). Evidence:
`../research/native/ico-native-first-window-2026-09-05.png`. This was the
first observable milestone of the native PC port (Rev.135).

Two rendering-blocking bugs were found and fixed on the way in
`OpenGLBackend.cpp`:

- `glCreateProgram` was called where `glCreateShader` was required, so shaders
  never compiled and nothing drew (silent black window).
- The GL context was made current to the X window instead of the GLX window,
  so the swap never presented the rendered frame.

## Expected output

```
ICO Native Runtime Prototype
[INFO] [runtime] Initializing ICO Native Runtime...
[INFO] [ps2] PS2 memory initialized: 32505856 bytes at 0x00100000
[INFO] [fs] FileSystem stub initialized: base='.'
[INFO] [sif] SIF RPC stub initialized
[INFO] [iop] IOP stub initialized
[INFO] [input] Input stub initialized
[INFO] [audio] Audio stub initialized
[INFO] [render] Render stub initialized: 640x480
[INFO] [timing] Timing initialized: target=60 FPS
[INFO] [gameloop] GameLoop initialized (target=60 FPS)
[INFO] [runtime] ICO Native Runtime initialized successfully
[INFO] [gameloop] GameLoop started (maxFrames=3)
[INFO] [main] [frame 1] tick
[INFO] [main] [frame 2] tick
[INFO] [main] [frame 3] tick
[INFO] [gameloop] GameLoop ended at frame 3
[INFO] [runtime] Shutting down ICO Native Runtime...
[INFO] [gameloop] GameLoop shutdown
[INFO] [timing] Timing shutdown
[INFO] [render] Render stub shutdown
[INFO] [audio] Audio stub shutdown
[INFO] [input] Input stub shutdown
[INFO] [iop] IOP stub shutdown
[INFO] [sif] SIF RPC stub shutdown
[INFO] [fs] FileSystem stub shutdown
[INFO] [ps2] PS2 memory shutdown
[INFO] [runtime] ICO Native Runtime shutdown complete
[shutdown] clean exit
```

## Next steps

After this scaffold, the plan is:

1. Choose a small, well-understood function from the decomp
2. Create a C/C++ port of that function
3. Verify it runs correctly in this runtime
4. Repeat for increasingly complex systems

## Architecture

```
native/
  CMakeLists.txt                cmake_minimum 3.14, C++17, 21 test targets
  README.md
  src/
    main.cpp                    Entry point (OpenGL demo window or headless boot)
    runtime/
      IcoRuntime.h/.cpp         Central runtime orchestrator
      GameLoop.h/.cpp           Frame loop with callback
      Logger.h/.cpp             Logging system
    platform/
      Platform.h/.cpp           Platform abstraction
      Input.h/.cpp              Input stub
      FileSystem.h/.cpp         Filesystem stub
      Timing.h/.cpp             Timing stub
      AudioStub.h/.cpp          Audio stub
      RenderStub.h/.cpp         Render stub (headless fallback)
      SifRpcStub.h/.cpp         SIF RPC stub
      IopStub.h/.cpp            IOP stub
    ps2/
      Ps2Types.h                PS2 type aliases
      Ps2Memory.h/.cpp          PS2 memory simulation
    engine/
      RenderBackend.h/.cpp      GS-style render command model, matrices, stub
      OpenGLBackend.h/.cpp      GLX/OpenGL backend (window, batch renderer)
      GifTag.h/.cpp             GIF tag/register model (GsTex0, GsAlpha, ...)
      GifCommandBuffer.h/.cpp   Render command accumulation
      GifCommandExecutor.h/.cpp Executes commands against RenderBackend
      GifPacket.h/.cpp          GifPacketBridge (PS2 GifPacket* API surface)
      Tm2Format.h/.cpp, Tm2Converter.h/.cpp   TM2 texture parsing/PSM swizzle
      GObj.h/.cpp, GObjPool.h/.cpp, ProcessNodePool.h/.cpp, IsysGObjRuntime.h/.cpp
      WorldStateLoader.h/.cpp, GObjFactory.h/.cpp, GObjEntityAllocator.h/.cpp
    game/
      IsysGObj.h/.cpp           Semantic bridge over src/core/isysgobj_semantic.c
      KanbanSceneLoader.h/.cpp  Scene loader bridge
      GirlBrainSemantic.h/.cpp  GirlBrain semantic C bridge
  tests/
    smoke_test.cpp              Basic functionality test
    render_backend_test.cpp, tm2_test.cpp, gif_command_test.cpp,
    gif_executor_test.cpp, gif_packet_test.cpp, opengl_backend_test.cpp,
    scene_loader_test.cpp, game_loop_scene_test.cpp, gobj_pool_test.cpp,
    process_node_pool_test.cpp, isysgobj_*_test.cpp, slot_dispatch_test.cpp,
    world_state_*_test.cpp, gobj_factory_test.cpp, gobj_entity_allocator_test.cpp,
    girlbrain_semantic_test.cpp
```

The 688 / 710 byte-exact `.s` functions of the decompilation project are the
ground truth that these semantic bridges model (see the project
`src/core/isysgobj_semantic.c` and `src/entity/asm/`, `src/core/asm/`).
