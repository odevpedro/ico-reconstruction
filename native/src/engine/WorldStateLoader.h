#pragma once

#include "core/gobj_abi.h"

#include <cstddef>

namespace ico::engine {

/*
 * WorldStateLoader — semantic reconstruction of world_state_load (USA
 * 0x001AF948) for the native port.
 *
 * Ground truth: src/core/asm/world_state_load.s (0x80 bytes, byte-exact
 * against the USA ELF, Rev.131).
 *
 * The original dispatches per-room:
 *   state  = *(gp - 0x6F60)
 *   entry  = 0x5F2FB8 + state * 0x194
 *   initFn = *(entry + 0x154)
 *   if (initFn) initFn();
 *   MakeCollisionDependGObjList();      // 0x00166028
 *   sceneApply();                       // 0x001AE3E8
 *   clears room-load flags at 0x274EC0+0x14/+0x18
 *   tail-jumps to the shared epilogue at 0x13D3F8 (iosThreadDestroy on PS2).
 *
 * The dispatch table base 0x5F2FB8 and slot stride 0x194 are compile-time
 * data in the original; the per-slot init_fn pointer at +0x154 is stored
 * per world_state. On the host we model these as an explicit table so the
 * native port can bind room setup behind the same contract.
 *
 * This is a semantic bridge: the host-side pointers replace the PS2 absolute
 * addresses. It is not claimed byte-exact.
 */
struct WorldStateDescriptor {
    u32 worldState = 0;
    bool hasInitFn = false;
    using InitFn = void (*)();
    InitFn initFn = nullptr;
};

/* Default stride of the original dispatch table entries (0x5F2FB8). */
constexpr u32 kWorldStateSlotStride = 0x194;
/* Offset within each entry of the room init_fn pointer. */
constexpr u32 kWorldStateInitFnOffset = 0x154;
/* PS2 dispatch table base; host-side state address is replaced by a table
   index, so this is documented for archaeology, not used at run time. */
constexpr ico_ptr32 kWorldStateDispatchTableBase = 0x005F2FB8;
/* PS2 room-load flags cleared on entry: 0x274ED4 and 0x274ED8. */
constexpr ico_ptr32 kWorldStateLoadFlags = 0x00274EC0;

class WorldStateLoader {
public:
    /* Host callbacks replacing the two fixed PS2 calls made by the original
       after the per-room init dispatch. */
    using CollisionListFn = void (*)();
    using SceneApplyFn = void (*)();

    bool initialize(WorldStateDescriptor* table, std::size_t count,
                    CollisionListFn collisionListFn, SceneApplyFn sceneApplyFn);
    void shutdown();
    bool isInitialized() const;

    /* Semantics of world_state_load(0x001AF948). Returns the room init_fn
       that was dispatched, or nullptr if the state has no init_fn. */
    u32 loadWorldState(u32 worldState);

    /* Host-side room-load flags; the loader clears the two words the
       original clears at 0x274ED4/0x274ED8. */
    u8 flagBytes[8] = {0};

private:
    WorldStateDescriptor* m_table = nullptr;
    std::size_t m_count = 0;
    CollisionListFn m_collisionListFn = nullptr;
    SceneApplyFn m_sceneApplyFn = nullptr;
    bool m_initialized = false;
};

}  // namespace ico::engine