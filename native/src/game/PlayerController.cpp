#include "game/PlayerController.h"

#include <array>
#include <cmath>
#include <limits>

namespace ico::game {
namespace {
using ico::engine::GObj;
using ico::engine::IsysGObjRuntime;
using ico::engine::ProcessNode;
}  // namespace

PlayerController::~PlayerController() {
    shutdown();
}

bool PlayerController::initialize(ico::engine::IsysGObjRuntime& runtime,
                                  const ico::engine::ClipBridge& bridge,
                                  u8 listId) {
    shutdown();
    if (listId >= ico::engine::kPrimaryListCount) {
        return false;
    }
    if (!runtime.isInitialized() || !bridge.isInitialized()) {
        return false;
    }
    this->runtime = &runtime;
    this->bridge = &bridge;

    ico::engine::GObj* g = runtime.add(listId, 0u, 0);
    if (g == nullptr) {
        this->runtime = nullptr;
        this->bridge = nullptr;
        return false;
    }
    gobj = g;

    // Register the char's own update process (typeMask 1, priority 0). The
    // callback is invoked by the runtime dispatcher on each update() pass —
    // this is the Phase A seam through which real BoyBrain logic will later
    // run (Phase C).
    auto* proc = runtime.registerProcess(
        *gobj, 1u, 0u,
        [this](ico::engine::GObj& g, ico::engine::ProcessNode&) {
            onProcessDispatch(g);
        });
    if (proc == nullptr) {
        runtime.remove(*gobj);
        gobj = nullptr;
        this->runtime = nullptr;
        this->bridge = nullptr;
        return false;
    }
    return true;
}

void PlayerController::shutdown() {
    if (gobj != nullptr && runtime != nullptr) {
        runtime->remove(*gobj);
    }
    gobj = nullptr;
    runtime = nullptr;
    bridge = nullptr;
    moveDx = moveDz = 0.0f;
}

bool PlayerController::spawn(float x, float z, float halfExtent) {
    if (bridge == nullptr) {
        return false;
    }
    const float ext = std::max(halfExtent, 0.0f);
    auto standable = [&](float px, float pz) -> bool {
        float h = 0.0f;
        for (const auto& pt : std::array<std::array<float, 2>, 5>{
                 std::array<float, 2>{px, pz},
                 std::array<float, 2>{px - ext, pz - ext},
                 std::array<float, 2>{px + ext, pz - ext},
                 std::array<float, 2>{px - ext, pz + ext},
                 std::array<float, 2>{px + ext, pz + ext}}) {
            if (!bridge->floorHeightAt(pt[0], pt[1], h)) {
                return false;
            }
        }
        return true;
    };
    if (standable(x, z)) {
        x_ = x;
        z_ = z;
        y_ = 0.0f;
        return bridge->floorHeightAt(x, z, y_);
    }
    // Outward spiral scan for the nearest walkable surface, bounded to a few
    // cells so a requested point far outside the room fails instead of
    // teleporting the character.
    const float cell = bridge->cellSize();
    const int maxRing = 2;
    for (int ring = 1; ring <= maxRing; ++ring) {
        const float step = static_cast<float>(ring) * cell;
        const std::array<std::pair<float, float>, 4> base{
            std::pair<float, float>{x + step, z},
            std::pair<float, float>{x - step, z},
            std::pair<float, float>{x, z + step},
            std::pair<float, float>{x, z - step},
        };
        for (const auto& b : base) {
            const std::pair<float, float> pts[4] = {
                {b.first + ext, b.second + ext},
                {b.first - ext, b.second + ext},
                {b.first - ext, b.second - ext},
                {b.first + ext, b.second - ext},
            };
            for (const auto& pt : pts) {
                if (standable(pt.first, pt.second)) {
                    x_ = pt.first;
                    z_ = pt.second;
                    y_ = 0.0f;
                    return bridge->floorHeightAt(x_, z_, y_);
                }
            }
        }
    }
    return false;
}

void PlayerController::setMove(f32 dx, f32 dz) {
    moveDx = dx;
    moveDz = dz;
}

void PlayerController::onProcessDispatch(ico::engine::GObj& g) {
    (void)g;
    if (bridge == nullptr) {
        moveDx = moveDz = 0.0f;
        return;
    }
    bridge->move(x_, y_, z_, moveDx, moveDz, halfExtent_, stepHeight_);
    moveDx = moveDz = 0.0f;
}

void PlayerController::update() {
    if (runtime == nullptr || gobj == nullptr) {
        return;
    }
    runtime->dispatchAllProcesses(*gobj);
}

}  // namespace ico::game