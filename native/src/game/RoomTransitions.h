#pragma once

#include "ps2/Ps2Types.h"

#include <cstddef>
#include <functional>
#include <vector>

namespace ico::game {

/*
 * RoomTransitionZone — host data for one door/portal in a room. When the
 * player-controlled GObj stays inside the (x,z,radius) circle for the opening
 * delay, the controller calls onTransition() with the zone.
 *
 *   sceneId        the current room's scene id the door belongs to
 *   name           debug label (e.g. "169_door")
 *   x, z, radius   trigger zone in world XZ (PS2 cm scale)
 *   targetSceneId  scene KanbanSceneLoader::requestScene()/execute() must load
 *                  (releases the current room's GObjs, creates the next ones)
 *   spawnX, spawnZ walkable probe point the host re-seeds the boy at
 *
 * The zone contents are HOST heuristics (same discipline as the Rev.155/159
 * room-role pairing) — not byte-verified original door tables. What IS real is
 * the chain the zone triggers: door open -> initSceneGObj swap (verified scene
 * tables) -> boy respawn.
 */
struct RoomTransitionZone {
    u32 sceneId = 0;
    const char* name = "";
    float x = 0.0f;
    float z = 0.0f;
    float radius = 40.0f;
    u32 targetSceneId = 0;
    float spawnX = 0.0f;
    float spawnZ = 0.0f;
};

/*
 * RoomTransitions — host door/portal state machine.
 *
 *   Idle            player outside every zone
 *   Opening         player entered a zone; progress counts toward open
 *   Transitioning   set while the callback fires (transient)
 *
 * Leaving the zone during Opening cancels (the door "closes"). After the
 * callback fires the zone is put in cooldown so the player crossing straight
 * back through the door does not instantly re-transition. Host model of the
 * original "approach door -> door opens -> load next room" beat; it does NOT
 * claim byte-level fidelity to the PS2 door handler.
 */
class RoomTransitions {
public:
    using Callback = std::function<void(const RoomTransitionZone&)>;

    void initialize(const RoomTransitionZone* zones, std::size_t zoneCount,
                    Callback onTransition);
    void reset();
    void setOpenDelay(float seconds) { openDelay_ = seconds; }
    void setCrossBackCooldown(float seconds) { cooldown_ = seconds; }

    void setBoyPosition(float x, float z);
    void update(float dtSeconds);

    enum class Phase { Idle, Opening, Transitioning };
    Phase phase() const { return phase_; }
    std::size_t activeZoneIndex() const { return activeZone_; }
    float openingProgress() const { return openingProgress_; }
    std::size_t firedCount() const { return firedCount_; }

private:
    const RoomTransitionZone* zoneByIndex(std::size_t i) const {
        return (i < zones_.size()) ? &zones_[i] : nullptr;
    }
    std::vector<RoomTransitionZone> zones_;
    Callback onTransition_;
    float openDelay_ = 1.2f;
    float cooldown_ = 2.0f;
    Phase phase_ = Phase::Idle;
    std::size_t activeZone_ = 0;
    float openingProgress_ = 0.0f;
    float boyX_ = 0.0f;
    float boyZ_ = 0.0f;
    std::vector<float> cooldowns_;
    std::size_t firedCount_ = 0;
};

}  // namespace ico::game