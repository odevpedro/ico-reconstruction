#include "game/RoomTransitions.h"

#include <algorithm>
#include <utility>

namespace ico::game {

void RoomTransitions::initialize(const RoomTransitionZone* zones,
                                 std::size_t zoneCount,
                                 Callback onTransition) {
    zones_.assign(zones, zones + zoneCount);
    cooldowns_.assign(zoneCount, 0.0f);
    onTransition_ = std::move(onTransition);
    phase_ = Phase::Idle;
    activeZone_ = 0;
    openingProgress_ = 0.0f;
    firedCount_ = 0;
}

void RoomTransitions::reset() {
    phase_ = Phase::Idle;
    activeZone_ = 0;
    openingProgress_ = 0.0f;
    std::fill(cooldowns_.begin(), cooldowns_.end(), 0.0f);
}

void RoomTransitions::setBoyPosition(float x, float z) {
    boyX_ = x;
    boyZ_ = z;
}

namespace {
bool contains(const RoomTransitionZone& zone, float x, float z) {
    const float dx = x - zone.x;
    const float dz = z - zone.z;
    return dx * dx + dz * dz <= zone.radius * zone.radius;
}
}  // namespace

void RoomTransitions::update(float dtSeconds) {
    if (dtSeconds <= 0.0f) {
        return;
    }
    for (std::size_t i = 0; i < cooldowns_.size(); ++i) {
        if (cooldowns_[i] > 0.0f) {
            cooldowns_[i] -= dtSeconds;
        }
    }

    std::size_t entered = zones_.size();
    for (std::size_t i = 0; i < zones_.size(); ++i) {
        if (cooldowns_[i] <= 0.0f && contains(zones_[i], boyX_, boyZ_)) {
            entered = i;
            break;
        }
    }

    if (phase_ == Phase::Idle) {
        if (entered < zones_.size()) {
            phase_ = Phase::Opening;
            activeZone_ = entered;
            /* The door starts opening on the frame the player enters. */
            openingProgress_ =
                (openDelay_ > 0.0f) ? dtSeconds / openDelay_ : 1.0f;
        }
        return;
    }

    if (phase_ == Phase::Opening) {
        if (entered != activeZone_) {
            /* Player walked out while the door was opening. */
            phase_ = Phase::Idle;
            openingProgress_ = 0.0f;
            return;
        }
        openingProgress_ +=
            (openDelay_ > 0.0f) ? dtSeconds / openDelay_ : 1.0f;
        if (openingProgress_ >= 1.0f) {
            phase_ = Phase::Transitioning;
            if (const RoomTransitionZone* zone = zoneByIndex(activeZone_)) {
                cooldowns_[activeZone_] = cooldown_;
                ++firedCount_;
                if (onTransition_) {
                    onTransition_(*zone);
                }
            }
            phase_ = Phase::Idle;
            openingProgress_ = 0.0f;
        }
        return;
    }
}

}  // namespace ico::game