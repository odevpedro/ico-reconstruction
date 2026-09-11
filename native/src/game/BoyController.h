#pragma once

#include "engine/ClipBridge.h"
#include "engine/IsysGObjRuntime.h"

#include <cstddef>

namespace ico::game {

// BoyController — semantic native port of the PS2 boy_hA/boy_hB/boy_hC state
// machine (USA 0x1C1A98 / 0x1C1DD8 / 0x1C1F58, src/entity/boy.c), running as a
// GObj process in the isysGObj runtime (Phase A seam) and resolving motion
// through ClipBridge (the native equivalent of the PS2 `_Clip` walkable
// contract, Phase B).
//
// It is NOT a byte-exact port: the PS2 handlers drive PS2 animation/cloth/
// model-crown systems (sub_104508, sub_1D23E0, boy_dispCrown...) that have no
// native counterpart yet. This bridge reproduces the *behavioral contract* the
// original encodes, fed by the platform input vector instead of a PS2 control
// read:
//
//   hC (0x1C1A98): per-instance constructor. Defaults the work area:
//     active=0 (idle), anim_time=0, config_A/B=20, range_A/B=300.0f,
//     flag_mask=0x80808080. Mirrored as kDefault* constants below.
//
//   hB (0x1C1DD8): per-frame update. Calls the movement solver
//     sub_103D50(entity, speed, spread, damping) with a speed selected by the
//     walk/run discriminator sub_14A0D8 (walk→15.0, run→30.0) and a damping
//     factor of 0.7 (FLOAT_GPCONST at gp-0x771C). At-rest detection fires the
//     "rest" event path (sub_13FF88 with arg 6). On native the solver is the
//     ClipBridge::move() sweep (axis-separated, damped approach toward the
//     requested delta).
//
//   hA (0x1C1F58): primary update. Two-path controller:
//     Path A (active!=0): full animation+physics.
//     Path B (active==0): transform-only idle.
//     Convergence: when world_state==0x27 and proximity<20.0f and the scene
//     object carries an interaction id and is not busy, the contact handler is
//     invoked. Mirrored here by the worldState()/setInteractionCallback()
//     seam (no Girl entity exists yet on native, so the gate is wired but the
//     handler is a stub).
//
// Movement semantics: the input vector is treated as a per-frame "desired
// delta". If its magnitude exceeds the run threshold the controller requests
// the 30.0 walk/run speed, otherwise 15.0; the approach is damped
// (0.7 velocity blend) so the character accelerates/decelerates smoothly, which
// mirrors the algebraic form of sub_1034B8 (f20=sep; f12=(speed-sep)*damping).
class BoyController {
public:
    // PS2 constant set lifted from src/entity/boy.c + disassembly.
    static constexpr float kWalkSpeed = 15.0f;   // sub_14A0D8 "walking" tier
    static constexpr float kRunSpeed = 30.0f;    // sub_14A0D8 "running" tier
    static constexpr float kDamping = 0.7f;      // FLOAT_GPCONST gp-0x771C
    static constexpr float kRunThreshold = 30.0f; // |input| beyond which run is requested
                                                  // (demo: single key 25 walks, W+D ≈ 35 runs)
    static constexpr u32 kWorldStateGameplay = 0x27;  // hA interaction gate
    static constexpr float kInteractionRange = 20.0f; // hA proximity gate

    // hC work-area defaults.
    static constexpr u32 kDefaultConfig = 20;
    static constexpr float kDefaultRange = 300.0f;
    static constexpr u32 kDefaultFlagMask = 0x80808080u;

    BoyController() = default;
    ~BoyController();
    BoyController(const BoyController&) = delete;
    BoyController& operator=(const BoyController&) = delete;

    // Binds the controller to a runtime + clip bridge and creates the owned
    // GObj, registering the per-frame process (the Phase A / Phase C seam).
    // listId is the isysGObj primary list the GObj joins (0..7).
    bool initialize(ico::engine::IsysGObjRuntime& runtime,
                    const ico::engine::ClipBridge& bridge,
                    u8 listId = 1u);

    void shutdown();
    bool isInitialized() const { return gobj != nullptr; }

    // Rev.170 (PORT): re-points the collision bridge when a room transition
    // swaps the walkable room. The BoyController keeps its own GObj/process
    // (no re-registration); only the collision context changes.
    void setBridge(const ico::engine::ClipBridge& bridge) { bridge_ = &bridge; }

    // Seeds the world position; spawns only on a walkable surface.
    bool spawn(float x, float z, float halfExtent);

    // Phase A input: ground-plane desired delta (world units, PS2 cm-scale).
    // Consumed at the next update() through the GObj process.
    void setMove(f32 dx, f32 dz);

    float halfExtent() const { return halfExtent_; }
    void setHalfExtent(float ext) { halfExtent_ = ext; }
    float stepHeight() const { return stepHeight_; }
    void setStepHeight(float step) { stepHeight_ = step; }

    float x() const { return x_; }
    float y() const { return y_; }
    float z() const { return z_; }

    // hA/hC state surface (semantic; not byte-exact).
    bool isActive() const { return state_.active != 0; }
    s32 animTime() const { return state_.animTime; }
    u32 variant() const { return state_.variant; }
    void setVariant(u32 v) { state_.variant = v; }
    u32 configA() const { return state_.configA; }
    u32 configB() const { return state_.configB; }
    float rangeA() const { return state_.rangeA; }
    float rangeB() const { return state_.rangeB; }
    u32 flagMask() const { return state_.flagMask; }

    // hA convergence gate: world_state_select (GAMEPLAY_FLAG gp-0x6f60).
    u32 worldState() const { return worldState_; }
    void setWorldState(u32 ws) { worldState_ = ws; }

    // hA interaction seam (no Girl entity exists yet on native). When the
    // controller is about to fire the proximity contact it calls the handler,
    // if set. Passed the interaction id stored on the scene object.
    using InteractionCallback = void (*)(u32 interactionId, void* userData);
    void setInteractionHandler(InteractionCallback cb, void* userData) {
        interactionCb_ = cb;
        interactionCtx_ = userData;
    }

    // Distance sensor value mirrored from scene_obj+0x644 (hA proximity input).
    // Native runtime has no distance sensor yet; callers may seed it.
    float distanceSensor() const { return distanceSensor_; }
    void setDistanceSensor(float d) { distanceSensor_ = d; }

    // Interaction id mirrored from scene_obj+0x648.
    u32 interactionId() const { return interactionId_; }
    void setInteractionId(u32 id) { interactionId_ = id; }

    // "Busy" flag mirrors the !sub_10D180 gate in hA Path convergence.
    bool isBusy() const { return busy_; }
    void setBusy(bool b) { busy_ = b; }

    ico::engine::GObj* getGObj() const { return gobj; }

    // Applies the pending move vector through the clip bridge and advances the
    // state machine. Called once per frame by the game loop.
    void update();

private:
    void onProcessDispatch(ico::engine::GObj& g);

    // Semantic mirror of struct boy_work (src/entity/boy.c, stride 0x4C).
    struct BoyWork {
        u32 active = 0;          // +0x10: 0 → hA Path B (idle), else Path A
        s32 animTime = 0;        // +0x14: animation time accumulator
        u32 variant = 0;         // +0x00: animation variant
        u32 configA = kDefaultConfig;   // +0x2C
        u32 configB = kDefaultConfig;   // +0x30
        float rangeA = kDefaultRange;   // +0x34
        float rangeB = kDefaultRange;   // +0x38
        u32 flagMask = kDefaultFlagMask;  // +0x48
    };

    ico::engine::IsysGObjRuntime* runtime = nullptr;
    const ico::engine::ClipBridge* bridge_ = nullptr;
    ico::engine::GObj* gobj = nullptr;

    // Motion state.
    float x_ = 0.0f, y_ = 0.0f, z_ = 0.0f;
    float velX = 0.0f, velZ = 0.0f;      // damped velocity approach
    float moveDx = 0.0f, moveDz = 0.0f;  // pending desired delta
    float halfExtent_ = 12.0f;
    float stepHeight_ = 45.0f;

    BoyWork state_;
    u32 worldState_ = kWorldStateGameplay;
    float distanceSensor_ = kInteractionRange;  // default: inside gate range
    u32 interactionId_ = 0;
    bool busy_ = false;

    InteractionCallback interactionCb_ = nullptr;
    void* interactionCtx_ = nullptr;
};

}  // namespace ico::game