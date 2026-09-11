#include "game/RoomTransitions.h"

#include <cassert>
#include <cstddef>

namespace {
ico::game::RoomTransitionZone g_zones[3] = {};

std::size_t g_firedCount = 0;
u32 g_firedScene = 0;
float g_firedSpawnX = 0.0f;
float g_firedSpawnZ = 0.0f;

void onTransition(const ico::game::RoomTransitionZone& zone) {
    ++g_firedCount;
    g_firedScene = zone.targetSceneId;
    g_firedSpawnX = zone.spawnX;
    g_firedSpawnZ = zone.spawnZ;
}
}  // namespace

int main() {
    /* Zone 0: door at origin, targets scene 0x2B, spawn 60 units past it.
     * Zone 1: far door; Zone 2: cooldown-only door (never entered in tests). */
    g_zones[0].sceneId = 0x0Fu;
    g_zones[0].name = "169_door";
    g_zones[0].x = 0.0f;
    g_zones[0].z = 0.0f;
    g_zones[0].radius = 40.0f;
    g_zones[0].targetSceneId = 0x2Bu;
    g_zones[0].spawnX = -60.0f;
    g_zones[0].spawnZ = 0.0f;

    g_zones[1].sceneId = 0x0Fu;
    g_zones[1].name = "far_door";
    g_zones[1].x = 500.0f;
    g_zones[1].z = 0.0f;
    g_zones[1].radius = 30.0f;
    g_zones[1].targetSceneId = 0x0Fu;
    g_zones[1].spawnX = 440.0f;
    g_zones[1].spawnZ = 0.0f;

    g_zones[2].sceneId = 0x2Bu;
    g_zones[2].name = "back_door";
    g_zones[2].x = 900.0f;
    g_zones[2].z = 0.0f;
    g_zones[2].radius = 20.0f;
    g_zones[2].targetSceneId = 0x0Fu;
    g_zones[2].spawnX = 0.0f;
    g_zones[2].spawnZ = 0.0f;

    ico::game::RoomTransitions once;
    once.initialize(g_zones, 1, onTransition);
    once.setOpenDelay(1.0f);
    once.setCrossBackCooldown(2.0f);

    /* Outside every zone: no state change. */
    once.setBoyPosition(300.0f, 0.0f);
    for (int i = 0; i < 60; ++i) {
        once.update(1.0f / 60.0f);
    }
    assert(once.phase() == ico::game::RoomTransitions::Phase::Idle);
    assert(once.firedCount() == 0);

    /* Enter the zone: Opening begins, but no fire before the delay elapses. */
    once.setBoyPosition(10.0f, 0.0f);
    once.update(1.0f / 60.0f);
    assert(once.phase() == ico::game::RoomTransitions::Phase::Opening);
    assert(once.firedCount() == 0);
    assert(once.openingProgress() > 0.0f);

    /* Leave during Opening: cancels (door closes). */
    once.setBoyPosition(300.0f, 0.0f);
    once.update(1.0f / 60.0f);
    assert(once.phase() == ico::game::RoomTransitions::Phase::Idle);
    assert(once.openingProgress() == 0.0f);
    assert(once.firedCount() == 0);

    /* Stay inside for the full opening delay: fires once, correct zone data. */
    once.setBoyPosition(10.0f, 0.0f);
    for (int i = 0; i < 62; ++i) {
        once.update(1.0f / 60.0f);
    }
    assert(once.firedCount() == 1);
    assert(g_firedCount == 1u);
    assert(g_firedScene == 0x2Bu);
    assert(g_firedSpawnX == -60.0f);
    assert(g_firedSpawnZ == 0.0f);
    assert(once.phase() == ico::game::RoomTransitions::Phase::Idle);

    /* Cooldown: crossing straight back does NOT instantly re-fire. */
    once.setBoyPosition(10.0f, 0.0f);
    once.update(1.0f / 60.0f);
    assert(once.phase() == ico::game::RoomTransitions::Phase::Idle);
    assert(once.firedCount() == 1);
    /* After the cooldown elapses the same zone opens again. */
    for (int i = 0; i < 125; ++i) {
        once.update(1.0f / 60.0f);
    }
    assert(once.phase() == ico::game::RoomTransitions::Phase::Opening);
    for (int i = 0; i < 70; ++i) {
        once.update(1.0f / 60.0f);
    }
    assert(once.firedCount() == 2);

    /* Two zones: the correct (nearest) zone is selected and its data fires. */
    g_firedCount = 0;
    g_firedScene = 0;
    ico::game::RoomTransitions multi;
    multi.initialize(g_zones, 2, onTransition);
    multi.setOpenDelay(0.5f);
    multi.setBoyPosition(480.0f, 0.0f); /* inside zone[1] only */
    for (int i = 0; i < 32; ++i) {
        multi.update(1.0f / 60.0f);
    }
    assert(multi.firedCount() == 1);
    assert(g_firedCount == 1u);
    assert(g_firedScene == 0x0Fu && g_firedScene != g_zones[0].targetSceneId);
    assert(g_firedSpawnX == 440.0f);

    /* reset(): clears phase/progress/cooldowns so the door opens instantly. */
    g_firedCount = 0;
    ico::game::RoomTransitions resetCtl;
    resetCtl.initialize(g_zones, 2, onTransition);
    resetCtl.setOpenDelay(5.0f);
    resetCtl.setBoyPosition(10.0f, 0.0f);
    resetCtl.update(1.0f / 60.0f);
    assert(resetCtl.phase() == ico::game::RoomTransitions::Phase::Opening);
    resetCtl.reset();
    assert(resetCtl.phase() == ico::game::RoomTransitions::Phase::Idle);
    assert(resetCtl.openingProgress() == 0.0f);
    for (int i = 0; i < 305; ++i) {
        resetCtl.update(1.0f / 60.0f);
    }
    assert(resetCtl.firedCount() == 1);
    assert(g_firedCount == 1u);

    /* Re-initialize replaces both zones and the callback. */
    g_firedCount = 0;
    ico::game::RoomTransitions reinit;
    reinit.initialize(g_zones + 2, 1, onTransition);
    assert(reinit.activeZoneIndex() == 0);
    reinit.setBoyPosition(905.0f, 0.0f);
    for (int i = 0; i < 75; ++i) {
        reinit.update(1.0f / 60.0f);
    }
    assert(reinit.firedCount() == 1);
    assert(g_firedCount == 1u);
    assert(g_firedScene == 0x0Fu);

    return 0;
}