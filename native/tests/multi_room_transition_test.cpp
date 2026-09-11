#include "engine/SceneAssetStore.h"
#include "game/KanbanSceneLoader.h"
#include "game/IsysGObj.h"

#include <cassert>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

/*
 * Rev.170 (PORT — multi-room): two-room store + real second-room door.
 *
 * Before Rev.170 a door transition reloaded the SAME room: only one
 * SceneAssetStore-backed scene (0x0F) existed, so requestScene(0x2B) swapped
 * the verified entry slice but re-rendered the primary room's geometry.
 *
 * This test wires the native demo's actual room pair (st02a primary, st00a as
 * the 0x2B companion — the two real rooms the door demo switches between) into
 * ONE SceneAssetStore via SceneAssetStore::parseRoom(sceneId=0x2B), then
 * drives KanbanSceneLoader through the same path main.cpp uses: verified scene
 * tables + room role plans + bind (0x0F and 0x2B) + requestScene/execute and
 * asserts the loader really owns both room bundles and that a 0x2B swap
 * produces a different current scene and a fresh GObj/resident set.
 *
 * Host data (native/assets room manifests), not extracted game data.
 */
int main() {
    const char* kRoomCandidates[] = {
        "assets/scene/rooms/%s/%s.manifest",
        "native/assets/scene/rooms/%s/%s.manifest",
        "../native/assets/scene/rooms/%s/%s.manifest",
        "../assets/scene/rooms/%s/%s.manifest",
        nullptr,
    };
    auto resolveManifest = [&kRoomCandidates](const char* room, std::string& out) -> bool {
        for (int i = 0; kRoomCandidates[i] != nullptr; ++i) {
            char buf[512];
            std::snprintf(buf, sizeof(buf), kRoomCandidates[i], room, room);
            std::ifstream f(buf);
            if (f.good()) {
                out = buf;
                return true;
            }
        }
        return false;
    };
    std::string st02aManifest, st00aManifest;
    assert(resolveManifest("st02a", st02aManifest));
    assert(resolveManifest("st00a", st00aManifest));

    /* One store, two rooms: parse() clears, parseRoom(sceneId) appends. */
    ico::engine::SceneAssetStore store;
    assert(store.parse(st02aManifest.c_str()));
    assert(store.hasScene(0x0Fu));
    const std::size_t primaryCount = store.sceneAssetCount(0x0Fu);
    assert(primaryCount > 0);
    assert(!store.hasScene(0x2Bu));

    assert(store.parseRoom(st00aManifest.c_str(), 0x2Bu));
    assert(store.hasScene(0x0Fu));
    assert(store.hasScene(0x2Bu));
    const std::size_t companionCount = store.sceneAssetCount(0x2Bu);
    assert(companionCount > 0);
    assert(store.sceneAssetCount(0x0Fu) == primaryCount);

    const std::vector<u32> sceneIds = store.sceneIds();
    assert(sceneIds.size() == 2);

    /* The two rooms are genuinely different bundles. */
    const ico::engine::SceneAssetEntry* firstA = store.sceneAsset(0x0Fu, 0);
    const ico::engine::SceneAssetEntry* firstB = store.sceneAsset(0x2Bu, 0);
    assert(firstA != nullptr && firstB != nullptr);
    assert(firstA->meshPath != firstB->meshPath);
    assert(firstA->meshPath.find("door") != std::string::npos ||
           firstB->meshPath.find("door") != std::string::npos);
    std::fprintf(stderr, "rev170: store scenes 0x0F=%zu / 0x2B=%zu assets\n",
                 primaryCount, companionCount);

    /* Same wiring as main.cpp runMultiRoomDemo. */
    IsysGObj runtime;
    assert(runtime.initialize(0x140, 0x500));
    KanbanSceneLoader loader;
    assert(loader.initialize(runtime));
    assert(loader.applyVerifiedSceneTables(
        ico::engine::kVerifiedSceneDescriptors,
        ico::engine::kVerifiedSceneDescriptorCount,
        ico::engine::kVerifiedScenePayload,
        ico::engine::kVerifiedScenePayloadCount,
        ico::engine::kVerifiedSceneRanges,
        ico::engine::kVerifiedSceneRangeCount));
    assert(loader.applyVerifiedRoomRolePlans(
        ico::engine::kVerifiedRoomRolePlans,
        ico::engine::kVerifiedRoomRolePlanCount));
    assert(loader.hasRoomRolePlan(0x2Bu));

    assert(loader.bindSceneAssets(store, 0x0Fu));
    assert(loader.bindSceneAssets(store, 0x2Bu));
    assert(loader.boundAssetCount(0x0Fu) == primaryCount);
    assert(loader.boundAssetCount(0x2Bu) == companionCount);

    /* Primary load: scene 0x0F creates its host GObjs. */
    assert(loader.requestScene(0x0Fu));
    assert(loader.execute());
    assert(loader.currentSceneId() == 0x0Fu);
    assert(loader.sceneGObjCount() == 25);
    const std::size_t attach0F = loader.attachBoundAssetsToGObjs(0x0Fu);
    assert(attach0F > 0);

    /* Real swap: requestScene(0x2B)/execute now loads the OTHER room's
       resident bundle; initSceneGObj releases the previous room's GObjs so
       pool active count equals this scene's own created count. */
    assert(loader.requestScene(0x2Bu));
    assert(loader.execute());
    assert(loader.currentSceneId() == 0x2Bu);
    assert(loader.sceneGObjCount() == 23);
    assert(runtime.pool().activeCount() == 23);
    const std::size_t attach2B = loader.attachBoundAssetsToGObjs(0x2Bu);
    assert(attach2B > 0);
    std::fprintf(stderr,
                 "rev170: swap 0x0F(25 GObjs, %zu att) -> 0x2B(23 GObjs, %zu att) ok\n",
                 attach0F, attach2B);

    loader.shutdown();
    runtime.shutdown();
    std::fprintf(stderr, "rev170: multi-room transition test OK\n");
    return 0;
}