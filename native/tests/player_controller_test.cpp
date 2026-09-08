#include "engine/ClipBridge.h"
#include "engine/IsysGObjRuntime.h"
#include "game/PlayerController.h"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <vector>

namespace {
constexpr float kEps = 0.01f;

// Small synthetic room: floor at y=0 (z in [-200,200], x in [-200,200]) and
// an east wall at x=200.
void buildRoom(ico::engine::ClipBridge& bridge) {
    std::vector<float> pos;
    std::vector<uint32_t> tris;
    auto pushTri = [&](float x0, float y0, float z0,
                       float x1, float y1, float z1,
                       float x2, float y2, float z2) {
        const uint32_t b = static_cast<uint32_t>(pos.size() / 3);
        pos.insert(pos.end(), {x0, y0, z0, x1, y1, z1, x2, y2, z2});
        tris.insert(tris.end(), {b, b + 1, b + 2});
    };
    pushTri(-200, 0, -200,   -200, 0, 200,   200, 0, 200);
    pushTri(-200, 0, -200,   200, 0, 200,   200, 0, -200);
    pushTri(200, 0, -200,   200, 0, 200,   200, 150, 200);
    pushTri(200, 0, -200,   200, 150, 200,   200, 150, -200);
    assert(bridge.buildFromMesh(pos.data(), static_cast<u32>(pos.size() / 3),
                                tris.data(), static_cast<u32>(tris.size() / 3)));
}
}  // namespace

int main() {
    ico::engine::IsysGObjRuntime runtime;
    assert(runtime.initialize(0x40, 0x40));
    assert(runtime.isInitialized());

    ico::engine::ClipBridge bridge;
    buildRoom(bridge);
    assert(bridge.isInitialized());

    ico::game::PlayerController player;

    /* Cannot initialize before the runtime/bridge are ready. */
    assert(!player.isInitialized());

    assert(player.initialize(runtime, bridge, 1u));
    assert(player.isInitialized());
    assert(player.getGObj() != nullptr);
    assert(player.getGObj()->self != 0);

    /* Spawn only where the floor is walkable. */
    assert(player.spawn(0, 0, player.halfExtent()));
    assert(std::fabs(player.y() - 0.0f) < kEps);

    /* Moving is applied through the GObj process: update() dispatches the
       registered process, which consumes the pending move vector. */
    player.setMove(40.0f, 0.0f);
    player.update();
    assert(std::fabs(player.x() - 40.0f) < kEps);
    assert(std::fabs(player.y() - 0.0f) < kEps);

    /* A blocked move (into the east wall) keeps the character inside. */
    player.setHalfExtent(10.0f);  // footprint reaches x=200 from x=190
    player.setMove(600.0f, 0.0f);
    player.update();
    assert(player.x() < 200.0f);
    assert(std::fabs(player.y() - 0.0f) < kEps);

    /* Dispatch consumed the pending move: a second update with nothing new
       leaves the position unchanged. */
    const float xBefore = player.x();
    player.update();
    assert(std::fabs(player.x() - xBefore) < kEps);

    /* Process is registered and fired by the runtime. */
    assert(runtime.processPool().activeCount() >= 1);

    /* Spawn rejection on an unsupported point leaves the controller usable. */
    const float xUnsupported = player.x();
    assert(!player.spawn(-500, -500, player.halfExtent()));
    assert(std::fabs(player.x() - xUnsupported) < kEps);

    /* Shutdown removes the GObj from the runtime. */
    player.shutdown();
    assert(!player.isInitialized());
    runtime.shutdown();
    assert(!runtime.isInitialized());

    std::fprintf(stderr, "player_controller_test: OK\n");
    return 0;
}