#include "engine/SceneAssetStore.h"
#include "game/KanbanSceneLoader.h"
#include "game/IsysGObj.h"

#include <cassert>
#include <cstdio>
#include <fstream>
#include <string>

/*
 * SceneAssetStore is the native seam that resolves a kanban scene id to real
 * room geometry (.p2o meshes + .tm2 texture dir) without baking the list into
 * main.cpp. This test validates the manifest parse and the KanbanSceneLoader
 * binding against the actual STGST00A manifest shipped in native/assets.
 * It is host data, not extracted game data: no original layout claim.
 */
int main() {
    const char* kManifestCandidates[] = {
        "assets/scene/stgst00a.manifest",
        "native/assets/scene/stgst00a.manifest",
        "../native/assets/scene/stgst00a.manifest",
        "../assets/scene/stgst00a.manifest",
        nullptr,
    };
    std::string manifestPath;
    for (int i = 0; kManifestCandidates[i] != nullptr; ++i) {
        std::ifstream f(kManifestCandidates[i]);
        if (f.good()) {
            manifestPath = kManifestCandidates[i];
            break;
        }
    }
    assert(!manifestPath.empty());

    ico::engine::SceneAssetStore store;
    assert(!store.isInitialized());

    /* Reject a missing file. */
    assert(!store.parse("nonexistent_manifest_42.txt"));
    assert(!store.isInitialized());

    assert(store.parse(manifestPath.c_str()));
    assert(store.isInitialized());

    /* The manifest ships one exhibition scene (0x0F). */
    const std::vector<u32> sceneIds = store.sceneIds();
    assert(sceneIds.size() >= 1);
    bool found0x0F = false;
    for (u32 id : sceneIds) {
        if (id == 0x0Fu) found0x0F = true;
    }
    assert(found0x0F);
    assert(store.hasScene(0x0Fu));
    assert(!store.hasScene(0xABCu));

    /* 28 pieces means a valid parse; also the demo requirement. */
    assert(store.sceneAssetCount(0x0Fu) == 28);
    assert(!store.textureDir().empty());
    assert(!store.sceneAsset(0x0Fu, 0)->meshPath.empty());
    assert(store.sceneAsset(0x0Fu, 28) == nullptr);

    /* First piece must be the door; label is basename without extension. */
    const ico::engine::SceneAssetEntry* first = store.sceneAsset(0x0Fu, 0);
    assert(first != nullptr);
    assert(first->label == "169_door");
    assert(first->meshPath.find("169_door.p2o") != std::string::npos);

    /* The dominant room mesh must be present. */
    bool hasP1 = false;
    for (std::size_t i = 0; i < store.sceneAssetCount(0x0Fu); ++i) {
        if (store.sceneAsset(0x0Fu, i)->label == "170_st00a_p1") hasP1 = true;
    }
    assert(hasP1);

    /* KanbanSceneLoader binding: the semantic loader now owns the composition. */
    IsysGObj runtime;
    assert(runtime.initialize(8, 8));
    KanbanSceneLoader loader;
    assert(loader.initialize(runtime));

    /* Binding a scene that exists in the store succeeds and is queryable
       before the scene is executed (composition is data, not render-time). */
    assert(loader.bindSceneAssets(store, 0x0Fu));
    assert(loader.hasBoundAssets(0x0Fu));
    assert(loader.boundAssetCount(0x0Fu) == 28);
    assert(loader.boundAsset(0x0Fu, 0)->label == "169_door");

    /* Re-binding the same scene replaces its asset list (idempotent). */
    assert(loader.bindSceneAssets(store, 0x0Fu));
    assert(loader.boundAssetCount(0x0Fu) == 28);

    /* Unknown scene: no binding, and it must not claim one. */
    assert(!loader.bindSceneAssets(store, 0x42u));
    assert(!loader.hasBoundAssets(0x42u));
    assert(loader.boundAssetCount(0x42u) == 0);
    assert(loader.boundAsset(0x42u, 0) == nullptr);

    /* Uninitialized store rejects. */
    ico::engine::SceneAssetStore empty;
    assert(!loader.bindSceneAssets(empty, 0x0Fu));

    loader.shutdown();
    runtime.shutdown();

    std::printf("scene_asset_store_test: all passed (%zu scenes, scene 0x0F = %zu assets)\n",
                sceneIds.size(), store.sceneAssetCount(0x0Fu));
    return 0;
}