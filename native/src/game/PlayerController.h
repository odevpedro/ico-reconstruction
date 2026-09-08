#pragma once

#include "engine/ClipBridge.h"
#include "engine/IsysGObjRuntime.h"

#include <cstddef>

namespace ico::game {

// PlayerController — a playable placeholder entity for the native-port front
// (Rev.149, report "instrucoes-agente-input-colisao-boy.md", items 3/4/5,
// Phase A + B). It is a GObj registered in the isysGObj-runtime whose move
// applied each frame is driven by the platform Input vector, resolved through
// ClipBridge (the native equivalent of the PS2 `_Clip` walkable contract).
//
// Phase A boundary: the controller is a *consumer* of the GObj dispatch, not
// a byte-exact port of a PS2 BoyBrain routine. It exists so the input path
// (X11 → Input → move vector) reaches an in-dispatcher entity. Phase C (the
// real boy_hA/hB/hC logic, src/entity/boy.c) is out of scope for this
// revision and will be layered on top of this controller.
class PlayerController {
public:
    PlayerController() = default;
    ~PlayerController();
    PlayerController(const PlayerController&) = delete;
    PlayerController& operator=(const PlayerController&) = delete;

    // Binds the controller to a runtime and a clip bridge, and creates the
    // owned GObj. listId is the isysGObj primary list the GObj joins
    // (0..7, see kPrimaryListCount). Returns false if binding fails.
    bool initialize(ico::engine::IsysGObjRuntime& runtime,
                    const ico::engine::ClipBridge& bridge,
                    u8 listId = 1u);

    // Removes the GObj from the runtime and unbinds.
    void shutdown();
    bool isInitialized() const { return gobj != nullptr; }

    // Seeds the world position; spawns only on a walkable surface (a cell
    // with floor support). Returns true when the surface was found.
    bool spawn(float x, float z, float halfExtent);

    // Ground-plane movement from the input layer, in world units (PS2
    // cm-scale). Applied at the next update() through the GObj process.
    void setMove(f32 dx, f32 dz);

    // Size in world units used for the collision footprint.
    float halfExtent() const { return halfExtent_; }
    void setHalfExtent(float ext) { halfExtent_ = ext; }

    // Max vertical delta allowed per step (prevents ledge snaps).
    float stepHeight() const { return stepHeight_; }
    void setStepHeight(float step) { stepHeight_ = step; }

    // World position of the character (updates after update()).
    float x() const { return x_; }
    float y() const { return y_; }
    float z() const { return z_; }

    ico::engine::GObj* getGObj() const { return gobj; }

    // Applies the pending move vector through the clip bridge and updates the
    // position. Called once per frame by the game loop.
    void update();

private:
    void onProcessDispatch(ico::engine::GObj& g);

    ico::engine::IsysGObjRuntime* runtime = nullptr;
    const ico::engine::ClipBridge* bridge = nullptr;
    ico::engine::GObj* gobj = nullptr;

    float x_ = 0.0f, y_ = 0.0f, z_ = 0.0f;
    float moveDx = 0.0f, moveDz = 0.0f;
    float halfExtent_ = 12.0f;
    float stepHeight_ = 45.0f;
};

}  // namespace ico::game