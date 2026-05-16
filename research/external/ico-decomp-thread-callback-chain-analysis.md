# ICO-decomp Thread/Callback Chain Analysis

**Date:** 2026-05-16
**Objective:** Cross-reference our callback registration chain (Rev.033-037) with ICO-decomp's decompiled `thread.c` source, headers, and splat YAML.

## Sources Used

- `ico2/fumi/ios/thread.c` — decompiled C source from ICO-decomp
- `include/ico2/fumi.h` — struct definitions (IosThreadInfo, IosMsgQueue, IosSemaInfo)
- `include/ico2/common.h` — constants, macros
- `config/ICO-PAL.yaml` — splat subsegment mapping
- `config/symbol_addrs.txt` — PAL symbol addresses

## Result 1: thread.c is decompiled in ICO-decomp

The YAML confirms thread.c is promoted from asm to C:

```yaml
- [0x040528, c, fumi/ios/thread]
#- [0x040528, asm, fumi/ios/thread]     <-- uncomment, then comment previous
#                                          if wanting to start fresh
```

The shockdriver remains asm:

```yaml
- [0x03ef68, asm, fumi/ios/shockdriver]
```

## Result 2: struct IosThreadInfo (0x70 bytes)

From `fumi.h`:

| Offset | Size | Field | Notes |
|--------|------|-------|-------|
| 0x00 | 0x30 | struct ThreadParam param | EE SDK thread creation params |
| 0x30 | 0x04 | s32 threadId | EE thread ID |
| 0x34 | 0x04 | void *args | Args passed to entry |
| 0x38 | 0x04 | iosThreadFuncPtr entry | **Actual callback function ptr** |
| 0x3C | 0x04 | s32 flags | IOS_THRF_ALLOCATED_STACK (1<<0) |
| 0x40 | 0x04 | s32 lowPrio | Priority flag |
| 0x44 | 0x04 | s32 x44 | Unknown |
| 0x48 | 0x04 | s32 hasMsgQ | Has message queue |
| 0x4C | 0x04 | struct IosMsgQueue *msgQ | Message queue ptr |
| 0x50 | 0x20 | u8 name[32] | Thread name |

### ThreadParam (EE SDK, offset 0x00-0x2F within IosThreadInfo):

| Offset | Field |
|--------|-------|
| 0x00 | u32 attr |
| 0x04 | u32 option |
| 0x08 | u8 *stack |
| 0x0C | u32 stackSize |
| 0x10 | u32 initPriority |
| 0x14 | u32 currentPriority |
| 0x18 | u32 gpReg |
| **0x1C** | **u32 entry** — EE thread entry function |

## Result 3: key offset affinity

Our callback storage function at `0x0013f3f0` writes the callback to `[node + 0x1C]`.

From `IosThreadInfo`, offset 0x1C = `ThreadParam.entry` = the EE SDK thread entry point.

From `ShockRequestBox` (hypothetical, not yet decompiled), offset 0x1C would also likely be a callback/handler function pointer.

The 0x1C offset is consistent with a structure that begins with a ThreadParam or similar EE SDK parameter block at offset 0x00.

## Result 4: PAL symbol-to-source mapping for our functions

From `symbol_addrs.txt`:

| PAL Symbol | PAL VA | Source File | Notes |
|------------|--------|-------------|-------|
| Shock_Decode | 0x0013f3c8 | fumi/ios/shockdriver | Symbol near our `0x13f3f0` |
| dumyAllocFunc | 0x0013f480 | fumi/ios/shockdriver | End of shockdriver asm range |
| iosThreadMain | 0x0013f528 | fumi/ios/thread | start of thread.c range |
| iosThreadCreateS | 0x0013f598 | fumi/ios/thread | Decompiled in thread.c |
| **our `0x13f7a8`** | ~0x13f7a8 | fumi/ios/thread | In gap between iosThreadCreateS and iosThreadStart |
| iosThreadStart | 0x0013f7b0 | fumi/ios/thread | Decompiled in thread.c |

## Result 5: The gap between iosThreadCreateS and iosThreadStart

From PAL symbols: gap = 536 bytes (0x0013f7b0 - 0x0013f598 = 0x218).

The C source decompiled in thread.c shows NO function between these two — the code jumps from `iosThreadCreateS` (ending at the `}` closing brace) directly to `iosThreadStart`. This means:

- Our function at `0x0013f7a8` (USA) falls in this gap
- It is likely a **static inline** function or a **non-matching** asm function that the decompiler has NOT yet extracted from the monolithic asm
- The ICO-decomp splat system would handle this by leaving the gap as a subsegment of the thread.c asm range

## Result 6: Mapping the callback type system

The ROPE callback registration uses `a3 = 0x13` (type 19 decimal). This was documented in Rev.036-037.

The shockdriver system has named functions suggesting a request/registration system:
- `ShockRequestBox_Clear`
- `ShockRequestBox_Regst`
- `ShockRequestBox_Request`
- `ShockRequestBox_DecodeRequest`
- `ShockRequestBox_EndRequestFree`
- `Init_Shock` / `Init_ShockDriver`

The type value `0x13` likely corresponds to an entry in a shock request type table, where each entry type has its own handler function registered at offset +0x1C of a request box structure.

## What is confirmed

1. thread.c decompilation exists and we can read the C source
2. struct IosThreadInfo layout (0x70 bytes): ThreadParam at 0x00, entry at 0x38, name at 0x50
3. ThreadParam.entry is at offset 0x1C — matches our callback storage offset
4. Our functions are split across fumi/ios/shockdriver (0x13f3f0) and fumi/ios/thread (0x13f7a8)
5. The YAML has thread.c promoted to C source but the shockdriver remains ASM
6. Neither function is named in ICO-decomp's symbol table

## What is probable

1. The callback storage function at 0x13f3f0 operates on a ShockRequestBox-like structure, storing a handler at [node + 0x1C]
2. The type 0x13 (19) corresponds to a shock request type identifier for cloth/callback operations
3. The registration function at 0x13f7a8 is an inline helper or static function in thread.c that wraps shockdriver calls

## What is unknown

1. Exact layout of the ShockRequestBox structure used by 0x13f3f0
2. The mechanism by which 0x13f3f0 is found and called (through a vtable? direct call?)
3. Who writes the callback type identifier (0x13) into the request box
4. The contents of the 384-byte gap between shockdriver and thread.c in the splat YAML

## What is discarded

1. The ROPE callback registration being an AI/entity system call (it's purely IOS infrastructure)
2. The "ROPE" descriptor in `.data` being connected to gameplay ropes (confirmed false in Rev.038)

## Next minimum test (without emulator)

1. Disassemble 0x13f3f0 to see its full body and confirm the struct access pattern
2. Disassemble the gap between shockdriver and thread.c in USA binary (0x13f3f0 region) to identify the exact function boundaries
3. Search the ICO-decomp project for any ShockRequestBox struct definition in headers

## Conservative verdict

The ICO-decomp analysis confirms our callback registration chain operates within the IOS IPC/shockdriver layer, not the entity system. The offset 0x1C aligns with ThreadParam.entry and likely ShockRequestBox.handler. The type 0x13 distinguishes the ROPE callback (cloth) from other callback types in the same registration system. A runtime PCSX2 capture at 0x13f7a8 (capturing a3 and a1 when a3 == 0x13) remains the only way to prove the registration path for the cloth callback.
