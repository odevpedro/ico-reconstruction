#include "game/BoyController.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace ico::game {
namespace {
using ico::engine::GObj;
using ico::engine::IsysGObjRuntime;
using ico::engine::ProcessNode;
}  // namespace

BoyController::~BoyController() {
    shutdown();
}

bool BoyController::initialize(ico::engine::IsysGObjRuntime& runtime,
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
    // callback is the Phase A/C seam: it runs the boy_hA/hB semantic state
    // machine inside the isysGObj dispatch.
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

void BoyController::shutdown() {
    if (gobj != nullptr && runtime != nullptr) {
        runtime->remove(*gobj);
    }
    gobj = nullptr;
    runtime = nullptr;
    bridge = nullptr;
    moveDx = moveDz = 0.0f;
    velX = velZ = 0.0f;
}

bool BoyController::spawn(float x, float z, float halfExtent) {
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

void BoyController::setMove(f32 dx, f32 dz) {
    moveDx = dx;
    moveDz = dz;
}

void BoyController::onProcessDispatch(ico::engine::GObj& g) {
    (void)g;
    if (bridge == nullptr) {
        return;
    }

    // hB: request a speed tier from the input magnitude. The original picks
    // walk(15)/run(30) via sub_14A0D8 (walk/run discriminators); the native
    // input layer has no analog throttle yet, so the tier follows the
    // requested delta magnitude (kRunThreshold = run).
    const float reqMag = std::hypot(moveDx, moveDz);
    const bool wantsMove = reqMag > 0.0001f;
    const float speed = (!wantsMove)
        ? 0.0f
        : (reqMag >= kRunThreshold ? kRunSpeed : kWalkSpeed);

    // Desired unit direction times tier speed = target velocity.
    float targetVx = 0.0f, targetVz = 0.0f;
    if (wantsMove) {
        targetVx = (moveDx / reqMag) * speed;
        targetVz = (moveDz / reqMag) * speed;
    }

    // hB: damped approach used by the PS2 solver (0x1034B8:
    // f20=separation; f12=(speed-sep)*damping; factor 0.7). Blend current
    // velocity toward the target so the character accelerates/decelerates.
    velX += (targetVx - velX) * kDamping;
    velZ += (targetVz - velZ) * kDamping;

    // Resolve the integrated velocity through the clip bridge (the native
    // sweep of `_Clip`), axis-separated, snapping to the floor.
    const float hx = velX, hz = velZ;
    const bool progressed = bridge->move(x_, y_, z_, hx, hz,
                                         halfExtent_, stepHeight_);

    // hA: two-path controller. Moving input (or residual velocity)  → Path A
    // (active: anim+physics). Otherwise → Path B (transform-only idle). The
    // native has no animation system yet, so Path A merely advances the work
    // state; both paths converge below, exactly as in the PS2 hA.
    const bool wantsActive = wantsMove || std::hypot(velX, velZ) > 0.0001f;
    if (wantsActive) {
        state_.active = 1;
        ++state_.animTime;
    } else {
        state_.active = 0;
    }

    // Convergence: interaction gate (hA). Only in gameplay world_state, when
    // the proximity sensor is inside the 20.0 interaction radius and the
    // scene object carries an interaction id and is not busy, is the contact
    // handler fired.
    if (worldState_ == kWorldStateGameplay &&
        distanceSensor_ < kInteractionRange && interactionId_ != 0 &&
        !busy_ && interactionCb_ != nullptr) {
        interactionCb_(interactionId_, interactionCtx_);
    }

    // A dispatch consumed the pending move (successfully or blocked).
    moveDx = moveDz = 0.0f;
    (void)progressed;
}

void BoyController::update() {
    if (runtime == nullptr || gobj == nullptr) {
        return;
    }
    runtime->dispatchAllProcesses(*gobj);
}

}  // namespace ico::game