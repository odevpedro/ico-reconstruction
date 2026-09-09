#include "engine/GifPacket.h"
#include "engine/RenderBackend.h"
#include "game/KanbanSceneLoader.h"

#include <cassert>
#include <cstdio>
#include <string>
#include <vector>

/*
 * Rev.154 — verified scene tables drive real GObj creation.
 *
 * This test wires the byte-level tables from tools/extract_scene_tables.py
 * (GeneratedSceneTables.h) into the semantic loader exactly as main.cpp does,
 * then asserts that initSceneGObj() actually creates host GObjs for scene
 * 0x0F (the native demo room, stage st00a).
 *
 * Ground truth: USA ELF scene 0x0F = entry indices [847,876) from the world
 * dispatch table 0x5F2FB8; descriptors at 0x2A31B8; entries at 0x2A4C48.
 */
int main() {
    IsysGObj runtime;
    assert(runtime.initialize(0x140, 0x500));

    KanbanSceneLoader loader;
    assert(loader.initialize(runtime));
    assert(loader.isInitialized());

    // Same wiring as main.cpp: apply the verified exported tables.
    assert(loader.applyVerifiedSceneTables(
        ico::engine::kVerifiedSceneDescriptors,
        ico::engine::kVerifiedSceneDescriptorCount,
        ico::engine::kVerifiedScenePayload,
        ico::engine::kVerifiedScenePayloadCount,
        ico::engine::kVerifiedSceneRanges,
        ico::engine::kVerifiedSceneRangeCount));

    // The export must carry the correct scene-0x0F slice.
    assert(ico::engine::kVerifiedScenePayloadCount == 29);
    assert(ico::engine::kVerifiedScenePayload[0].entryIndex == 847);
    assert(ico::engine::kVerifiedScenePayload[0].descriptorIndex == 44);
    /* 0x0F covers [847,876): all 29 slice entries must be enabled. */
    std::size_t enabled = 0;
    for (std::size_t i = 0; i < kSceneEntryCount; ++i) {
        const SceneEntryRecord* record = loader.entry(i);
        if (record != nullptr && record->enabled && record->sceneId == 0x0Fu) {
            ++enabled;
        }
    }
    assert(enabled == 29);

    // GObjs are created from verified entries. Gate-0 descriptors (44
    // DYNAMICMOTIONDAT, 54 STAGESETTING) skip creation entirely: entries
    // 847, 872, 875 (desc 44) and 873 (desc 54) drop out, so 29 - 4 = 25.
    assert(loader.requestScene(0x0Fu));
    const std::size_t created = loader.initSceneGObj(0x0Fu);
    assert(created == 25);
    std::fprintf(stderr, "rev154: scene 0x0F created %zu host GObjs\n", created);

    // Spot-check GObj list placement: entry 855 (SOBJ) is list 0, 854 list 1,
    // 853 list 2; the rest are list 7. Pool must reflect the created count.
    assert(runtime.pool().activeCount() == created);
    const std::vector<StaticSceneDebugItem> items = loader.staticSceneDebugItems();
    std::size_t list7Count = 0;
    std::size_t list0Count = 0;
    for (const StaticSceneDebugItem& item : items) {
        if (item.listId == 7) ++list7Count;
        if (item.listId == 0) ++list0Count;
    }
    /* list 7 dominates; exactly one entry maps to list 0 (entry 855). */
    assert(list0Count == 1);
    assert(list7Count > 0);
    /* BIRD + GIRLFORCEFIELD use descriptors 32 / 55. */
    std::size_t birdCount = 0;
    std::size_t forceFieldCount = 0;
    for (const StaticSceneDebugItem& item : items) {
        if (item.descriptorIndex == 32) ++birdCount;
        if (item.descriptorIndex == 55) ++forceFieldCount;
    }
    assert(birdCount == 4);
    assert(forceFieldCount == 1);

    // Trivial render sanity through the GIF bridge (host-only).
    auto backend = ico::engine::createRenderBackend();
    assert(backend->initialize(64, 64));
    ico::engine::GifPacketBridge bridge(*backend);
    bridge.init(64, 64);
    bridge.startPacketPri(0);
    const std::size_t rendered = loader.renderSyntheticScene(bridge);
    assert(rendered == created);
    bridge.endPacket();
    backend->shutdown();

    loader.clearRequests();
    assert(loader.pendingRequestCount() == 0);
    loader.shutdown();
    assert(!loader.isInitialized());
    runtime.shutdown();
    std::fprintf(stderr, "rev154: verified-scene-tables test OK\n");
    return 0;
}