#include "engine/WorldStateLoader.h"

#include <cassert>
#include <cstddef>

namespace {
ico::engine::WorldStateDescriptor g_table[3] = {};
int g_initCalls[3] = {0, 0, 0};
int g_collisionCalls = 0;
int g_sceneApplyCalls = 0;

void initRoom0() {
    g_initCalls[0]++;
}
void initRoom1() {
    g_initCalls[1]++;
}
void initRoom2() {
    g_initCalls[2]++;
}

void collisionList() {
    g_collisionCalls++;
}
void sceneApply() {
    g_sceneApplyCalls++;
}
}  // namespace

int main() {
    g_table[0].worldState = 0;
    g_table[0].hasInitFn = true;
    g_table[0].initFn = initRoom0;

    g_table[1].worldState = 1;
    g_table[1].hasInitFn = false;
    g_table[1].initFn = nullptr;

    g_table[2].worldState = 2;
    g_table[2].hasInitFn = true;
    g_table[2].initFn = initRoom2;

    ico::engine::WorldStateLoader loader;

    /* Reject a half-configured table. */
    assert(!loader.initialize(nullptr, 3, collisionList, sceneApply));
    assert(!loader.isInitialized());

    assert(loader.initialize(g_table, 3, collisionList, sceneApply));
    assert(loader.isInitialized());

    /* State 0: dispatch initFn + the two followed calls. */
    const u32 r0 = loader.loadWorldState(0);
    assert(r0 == 1u);
    assert(g_initCalls[0] == 1);
    assert(g_initCalls[1] == 0);
    assert(g_collisionCalls == 1);
    assert(g_sceneApplyCalls == 1);

    /* State 1 has no init_fn: the two followed calls still fire. */
    const u32 r1 = loader.loadWorldState(1);
    assert(r1 == 0u);
    assert(g_collisionCalls == 2);
    assert(g_sceneApplyCalls == 2);

    /* Out-of-range world_state is a no-op for dispatch but still runs the
       followed calls, matching the original's unconditional tail work. */
    const u32 r2 = loader.loadWorldState(99);
    assert(r2 == 0u);
    assert(g_collisionCalls == 3);
    assert(g_sceneApplyCalls == 3);

    /* Sanity: stride/offset constants encode the documented table layout. */
    static_assert(ico::engine::kWorldStateSlotStride == 0x194,
                  "slot stride must match the original dispatch table");
    static_assert(ico::engine::kWorldStateInitFnOffset == 0x154,
                  "init_fn offset must match the original dispatch table");
    static_assert(ico::engine::kWorldStateDispatchTableBase == 0x005F2FB8,
                  "dispatch table base must match the original .rodata");
    static_assert(ico::engine::kWorldStateLoadFlags == 0x00274EC0,
                  "room-load flags base must match the original .bss");

    loader.shutdown();
    assert(!loader.isInitialized());

    /* After shutdown the dispatcher is inert. */
    assert(loader.loadWorldState(0) == 0u);
    assert(g_initCalls[0] == 1);
    return 0;
}