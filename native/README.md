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
- A rendering engine
- An ELF loader
- A MIPS interpreter
- A complete PS2 emulator

## Problem this solves

The reverse engineering project has 1224 functions verified as byte-exact PS2 assembly. Before those can be converted to portable C/C++, we need a runtime environment where they can execute. This scaffold is that environment.

## PS2 systems this intends to replace

| PS2 System | Stub | Future Replacement |
|------------|------|-------------------|
| scePad | Input | SDL GameController |
| CDVD/sceFs | FileSystem | Host filesystem |
| SPU2/IOP audio | AudioStub | SDL_mixer / FMOD |
| GS/VU rendering | RenderStub | OpenGL / Vulkan |
| vblank/timing | Timing | std::chrono |
| EE heap | Ps2Memory | Native allocator |
| SIF RPC | SifRpcStub | Direct function calls |
| IOP | IopStub | Native functions |

## How to compile

```bash
cmake -S native -B native/build
cmake --build native/build
```

## How to run

```bash
./native/build/ico_native
```

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
  CMakeLists.txt
  README.md
  src/
    main.cpp                    Entry point
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
      RenderStub.h/.cpp         Render stub
      SifRpcStub.h/.cpp         SIF RPC stub
      IopStub.h/.cpp            IOP stub
    ps2/
      Ps2Types.h                PS2 type aliases
      Ps2Memory.h/.cpp          PS2 memory simulation
  tests/
    smoke_test.cpp              Basic functionality test
```
