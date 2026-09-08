#include "engine/ClipBridge.h"
#include "engine/IsysGObjRuntime.h"
#include "game/BoyController.h"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <vector>

using ico::game::BoyController;

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

    ico::game::BoyController boy;

    /* Cannot initialize before the runtime/bridge are ready. */
    assert(!boy.isInitialized());

    assert(boy.initialize(runtime, bridge, 1u));
    assert(boy.isInitialized());
    assert(boy.getGObj() != nullptr);
    assert(boy.getGObj()->self != 0);

    /* Spawn only where the floor is walkable. */
    assert(boy.spawn(0, 0, boy.halfExtent()));
    assert(std::fabs(boy.y() - 0.0f) < kEps);

    /* ---- hC defaults: the semantic work area starts in the PS2
       constructor state. ---- */
    assert(!boy.isActive());
    assert(boy.animTime() == 0);
    assert(boy.configA() == BoyController::kDefaultConfig);
    assert(boy.configB() == BoyController::kDefaultConfig);
    assert(std::fabs(boy.rangeA() - BoyController::kDefaultRange) < kEps);
    assert(std::fabs(boy.rangeB() - BoyController::kDefaultRange) < kEps);
    assert(boy.flagMask() == BoyController::kDefaultFlagMask);
    assert(boy.worldState() == BoyController::kWorldStateGameplay);

    /* ---- hB: walking tier (|input| < kRunThreshold) damps toward 15.0
       per frame, not raw 25 like the placeholder. After one update the
       velocity blend is target*0.7. ---- */
    boy.setMove(25.0f, 0.0f);       // below run threshold → walk tier 15
    boy.update();
    assert(boy.isActive());
    assert(boy.animTime() == 1);
    const float walkFrame = 15.0f * BoyController::kDamping;  // ≈ 10.5
    assert(std::fabs(boy.x() - walkFrame) < 0.02f);
    assert(std::fabs(boy.y() - 0.0f) < kEps);

    /* Re-issuing the same move keeps damping toward 15.0/frame. */
    boy.setMove(25.0f, 0.0f);
    boy.update();
    assert(boy.x() > walkFrame);
    assert(boy.x() < 15.0f * 2.0f + kEps);

    /* ---- hB: running tier (|input| >= kRunThreshold) targets 30.0. ---- */
    {
        /* Skip back to origin for a clean measurement. */
        boy.shutdown();
        assert(!boy.isInitialized());
        runtime.shutdown();
        assert(!runtime.isInitialized());

        assert(runtime.initialize(0x40, 0x40));
        assert(boy.initialize(runtime, bridge, 1u));
        assert(boy.spawn(0, 0, boy.halfExtent()));
        assert(std::fabs(boy.y() - 0.0f) < kEps);
    }
    boy.setMove(40.0f, 0.0f);       // >= kRunThreshold → run tier 30
    boy.update();
    const float runFrame = 30.0f * BoyController::kDamping;  // ≈ 21.0
    assert(std::fabs(boy.x() - runFrame) < 0.02f);
    assert(boy.isActive());

    /* With no new input the hB damping blends velocity toward zero (the echo
       of sub_1034B8's `(speed-sep)*damping`). The character decelerates over
       a few frames, then hA drops to Path B (idle) once motion dies out. */
    const float xAfterRun = boy.x();
    int idleFrames = 0;
    for (int i = 0; i < 40; ++i) {
        boy.update();
        if (!boy.isActive()) {
            idleFrames = i;
            break;
        }
    }
    assert(idleFrames > 0 && idleFrames < 40);
    assert(std::fabs(boy.x() - xAfterRun) < BoyController::kRunSpeed * 2.0f + kEps);
    assert(!boy.isActive());

    /* Fully idled: a further no-input frame moves nothing. */
    const float xIdle = boy.x();
    boy.update();
    assert(std::fabs(boy.x() - xIdle) < kEps);
    assert(!boy.isActive());

    /* A blocked move (into the east wall) keeps the character inside. */
    boy.setHalfExtent(10.0f);  // footprint reaches x=200 from x=190
    boy.setMove(600.0f, 0.0f);
    boy.update();
    assert(boy.x() < 200.0f);
    assert(boy.y() < kEps);

    /* ---- hA convergence: interaction gate (world_state == 0x27) ---- */
    {
        struct Ctx { int fired; u32 seenId; };
        Ctx ctx{0, 0};
        boy.setInteractionHandler(
            [](u32 id, void* u) { Ctx* c = static_cast<Ctx*>(u);
                                   ++c->fired; c->seenId = id; },
            &ctx);
        boy.setInteractionId(7);
        boy.setDistanceSensor(5.0f);   // inside 20.0 radius
        boy.update();
        assert(ctx.fired == 1);
        assert(ctx.seenId == 7);

        /* Gate closed when the world state is not gameplay. */
        boy.setWorldState(0x10);
        boy.update();
        assert(ctx.fired == 1);

        /* Gate closed when the sensor is outside the radius. */
        boy.setWorldState(BoyController::kWorldStateGameplay);
        boy.setDistanceSensor(80.0f);
        boy.update();
        assert(ctx.fired == 1);

        /* Gate closed when busy (mirrors !sub_10D180). */
        boy.setDistanceSensor(5.0f);
        boy.setBusy(true);
        boy.update();
        assert(ctx.fired == 1);
        boy.setBusy(false);
    }

    /* Process is registered and fired by the runtime. */
    assert(runtime.processPool().activeCount() >= 1);

    /* Spawn rejection on an unsupported point leaves the controller usable. */
    const float xUnsupported = boy.x();
    assert(!boy.spawn(-500, -500, boy.halfExtent()));
    assert(std::fabs(boy.x() - xUnsupported) < kEps);

    /* Shutdown removes the GObj from the runtime. */
    boy.shutdown();
    assert(!boy.isInitialized());
    runtime.shutdown();
    assert(!runtime.isInitialized());

    std::fprintf(stderr, "boy_controller_test: OK\n");
    return 0;
}