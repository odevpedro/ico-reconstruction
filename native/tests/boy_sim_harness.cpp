// Headless harness: run BoyController (Rev.150) against the real p1 room mesh.
// Simulates: 12 frames walk (W, step 25), 12 frames run (W+D diagonal ~35.4),
// release to idle (damping decay), then a wall push to confirm the clamp.
#include "engine/ClipBridge.h"
#include "engine/IsysGObjRuntime.h"
#include "engine/Ps2oMesh.h"
#include "game/BoyController.h"

#include <cstdio>
#include <cmath>
#include <vector>

using namespace ico::engine;
using ico::game::BoyController;

int main(int argc, char** argv) {
    const char* p2o = argc > 1 ? argv[1] : "assets/170_st00a_p1.p2o";
    Ps2oMesh mesh;
    if (!loadPs2oMeshFromFile(p2o, mesh)) {
        std::fprintf(stderr, "loadPs2oMeshFromFile failed: %s\n", p2o);
        return 1;
    }

    ClipBridge clip;
    if (!clip.buildFromMesh(mesh.positions.data(), mesh.vertexCount,
                            mesh.triangles.data(),
                            static_cast<u32>(mesh.triangles.size() / 3))) {
        std::fprintf(stderr, "ClipBridge build failed\n");
        return 1;
    }
    std::fprintf(stderr, "grid=%ux%u blocked=%u bounds x[%g,%g] z[%g,%g]\n",
                 clip.gridWidth(), clip.gridHeight(), clip.blockedCellCount(),
                 clip.minX(), clip.maxX(), clip.minZ(), clip.maxZ());

    IsysGObjRuntime runtime;
    if (!runtime.initialize(0x40, 0x40)) return 1;

    BoyController boy;
    if (!boy.initialize(runtime, clip, 1u)) return 1;
    if (!boy.spawn(-50.0f, 1000.0f, boy.halfExtent())) return 1;

    int frameN = 0;
    std::printf("frame active animTime  x        z        |v|      tier\n");
    float px = boy.x(), pz = boy.z();
    auto frame = [&](const char* tag) {
        const float dv = std::hypot(boy.x() - px, boy.z() - pz);
        px = boy.x(); pz = boy.z();
        std::printf("%5d %5d %8d %8.2f %8.2f %8.2f  %s\n",
                    frameN, boy.isActive() ? 1 : 0, boy.animTime(),
                    boy.x(), boy.z(), dv, tag);
        ++frameN;
    };

    // Phase 1: walk forward (W = +25 z, below kRunThreshold=30).
    constexpr float step = 25.0f;
    for (int i = 0; i < 12; ++i) { boy.setMove(0.0f, step); boy.update(); frame("walk"); }

    // Phase 2: run diagonal (W+D = (25,25), magnitude 35.36 >= 30).
    for (int i = 0; i < 12; ++i) { boy.setMove(step, step); boy.update(); frame("run"); }

    // Phase 3: release to idle, damping decays over frames.
    for (int i = 0; i < 12; ++i) { boy.update(); frame("idle"); }

    // Phase 4: push far east (+x) to hit the wall face and confirm clamp.
    for (int i = 0; i < 12; ++i) { boy.setMove(600.0f, 0.0f); boy.update(); frame("wall+"); }

    boy.shutdown();
    std::printf("final: x=%.2f y=%.2f z=%.2f (y must sit on floor, x clamped by walls)\n",
                boy.x(), boy.y(), boy.z());
    return 0;
}