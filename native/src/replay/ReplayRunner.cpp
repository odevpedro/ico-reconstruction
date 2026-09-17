#include "replay/ReplayRunner.h"

#include <cmath>
#include <cstdio>
#include <fstream>
#include <vector>

#include "engine/ClipBridge.h"
#include "engine/IsysGObjRuntime.h"
#include "game/BoyController.h"
#include "game/KanbanSceneLoader.h"

namespace ico::replay {

namespace {

using ico::game::BoyController;

// Synthetic walkable room shared with boy_controller_test: floor at y=0 over
// [-200,200]x[-200,200] plus an east wall. Deterministic and asset-free, so
// the digest golden is reproducible on any clean checkout.
bool buildSynthRoom(ico::engine::ClipBridge& bridge) {
    std::vector<float> pos;
    std::vector<uint32_t> tris;
    auto pushTri = [&](float x0, float y0, float z0,
                       float x1, float y1, float z1,
                       float x2, float y2, float z2) {
        const uint32_t b = static_cast<uint32_t>(pos.size() / 3);
        pos.insert(pos.end(), {x0, y0, z0, x1, y1, z1, x2, y2, z2});
        tris.insert(tris.end(), {b, b + 1, b + 2});
    };
    pushTri(-200, 0, -200, -200, 0, 200, 200, 0, 200);
    pushTri(-200, 0, -200, 200, 0, 200, 200, 0, -200);
    pushTri(200, 0, -200, 200, 0, 200, 200, 150, 200);
    pushTri(200, 0, -200, 200, 150, 200, 200, 150, -200);
    return bridge.buildFromMesh(pos.data(), static_cast<uint32_t>(pos.size() / 3),
                                tris.data(), static_cast<uint32_t>(tris.size() / 3));
}

// Pad nibble v1 → desired per-frame delta. Bits 0/1/2/3 = +x/-x/+z/-z;
// magnitude fixed at the host walk tier (BoyController::kWalkSpeed).
void padToMove(uint32_t pad, float& dx, float& dz) {
    dx = 0.0f;
    dz = 0.0f;
    if (pad & 0x1u) dx += BoyController::kWalkSpeed;
    if (pad & 0x2u) dx -= BoyController::kWalkSpeed;
    if (pad & 0x4u) dz += BoyController::kWalkSpeed;
    if (pad & 0x8u) dz -= BoyController::kWalkSpeed;
    if (dx != 0.0f && dz != 0.0f) {
        // Diagonal: keep a unit-normalized magnitude on the walk tier.
        const float inv = 1.0f / std::sqrt(2.0f);
        dx *= inv;
        dz *= inv;
    }
}

}  // namespace

std::vector<std::string> runReplayDeterministic(const ReplayData& data,
                                                ReplayRunnerConfig config,
                                                std::string& error) {
    using namespace ico::engine;
    std::vector<std::string> lines;

    ico::engine::IsysGObjRuntime runtime;
    if (!runtime.initialize(0x140, 0x500)) {
        error = "failed to initialize isysGObj runtime";
        return lines;
    }

    KanbanSceneLoader loader;
    if (!loader.initialize(runtime)) {
        error = "failed to initialize KanbanSceneLoader";
        return lines;
    }
    if (!loader.applyVerifiedSceneTables(ico::engine::kVerifiedSceneDescriptors,
                                         ico::engine::kVerifiedSceneDescriptorCount,
                                         ico::engine::kVerifiedScenePayload,
                                         ico::engine::kVerifiedScenePayloadCount,
                                         ico::engine::kVerifiedSceneRanges,
                                         ico::engine::kVerifiedSceneRangeCount)) {
        error = "failed to apply verified scene tables";
        return lines;
    }
    const uint32_t initScene = data.header.worldStateInit;
    if (!loader.requestScene(initScene) || !loader.execute()) {
        error = "requestScene/execute failed";
        return lines;
    }

    ico::engine::ClipBridge bridge;
    if (!buildSynthRoom(bridge)) {
        error = "failed to build synthetic room";
        return lines;
    }

    ico::game::BoyController boy;
    if (!boy.initialize(runtime, bridge, 1u)) {
        error = "failed to initialize BoyController";
        return lines;
    }
    if (!boy.spawn(0.0f, 0.0f, boy.halfExtent())) {
        error = "failed to spawn boy at room center";
        return lines;
    }

    const std::size_t frames = std::max<std::size_t>(1, ReplayFile::estimatedFrameCount(data));
    for (std::size_t f = 0; f < frames; ++f) {
        // A world event consumed on frame <f> swaps the resident scene and
        // re-groups the boy on the (new) room floor — host deterministic
        // semantic, mirroring the Rev.170 door-transition swap (setBridge +
        // walkable respawn). The GL demo and the future PCSX2 golden share
        // this path.
        if (const ReplayWorldEvent* ev = ReplayFile::eventAtFrame(data, static_cast<uint32_t>(f))) {
            if (!loader.requestScene(ev->sceneId) || !loader.execute()) {
                error = "world event F" + std::to_string(f) +
                        ": requestScene/execute failed (scene " + std::to_string(ev->sceneId) + ")";
                return std::vector<std::string>();
            }
            boy.setWorldState(ev->worldState);
            boy.setBridge(bridge);
            if (!boy.spawn(boy.x(), boy.z(), boy.halfExtent())) {
                error = "world event F" + std::to_string(f) + ": boy re-spawn failed";
                return std::vector<std::string>();
            }
        }

        const uint32_t pad = ReplayFile::padForFrame(data, static_cast<uint32_t>(f));
        float dx = 0.0f, dz = 0.0f;
        padToMove(pad, dx, dz);
        boy.setMove(dx, dz);
        boy.update();

        StateDigestFrame sf;
        sf.frame = static_cast<uint32_t>(f);
        sf.scene = loader.currentSceneId();
        sf.gobj = static_cast<uint32_t>(runtime.pool().activeCount());
        sf.active = boy.isActive() ? 1u : 0u;
        sf.bx = boy.x();
        sf.by = boy.y();
        sf.bz = boy.z();
        sf.mx = static_cast<int32_t>(dx);
        sf.mz = static_cast<int32_t>(dz);
        lines.push_back(formatDigestLine(sf));
    }

    if (!config.capturePath.empty()) {
        std::ofstream out(config.capturePath);
        if (!out) {
            error = "cannot write capture " + config.capturePath;
            return std::vector<std::string>();
        }
        out << "# replay deterministic capture (format v" << kReplayFormatVersion << ")\n";
        out << "ISO_SHA1 " << data.header.isoSha1 << "\n";
        out << "SAVE_STATE " << data.header.saveState << "\n";
        out << "WORLD_STATE_INIT 0x" << std::hex << data.header.worldStateInit << std::dec << "\n";
        for (const auto& l : lines) out << l << "\n";
    }
    return lines;
}

}  // namespace ico::replay