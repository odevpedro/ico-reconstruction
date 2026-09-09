#include "engine/SceneAssetStore.h"
#include "game/KanbanSceneLoader.h"
#include "game/IsysGObj.h"

#include <cassert>
#include <cstdio>
#include <fstream>
#include <string>

/*
 * Rev.155 (Passo 1) — per-GObj asset attachment.
 *
 * The renderer closes its feedback loop: instead of reading SceneAssetStore
 * directly, every bound scene asset is attached to a specific scene GObj
 * (round-robin HOST pairing) and the render iterates the ACTIVE GObjs to
 * draw what each GObj commands.
 *
 * This test validates the attachment seam without any GL dependency:
 *   - attachBoundAssetsToGObjs pairs bound assets to the GObjs that
 *     initSceneGObj() actually created;
 *   - every attachment points at a real, live GObj handle;
 *   - the attachment carries the asset's mesh path (the renderer's payload);
 *   - attachments are cleared when initSceneGObj() rebuilds the GObj set.
 *
 * Ground truth is Rev.154: scene 0x0F = entries [847,876), 29 payload rows,
 * 4 gate-0 descriptors drop out → 25 GObjs; the stgst00a manifest ships 28
 * piece assets.
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
    assert(store.parse(manifestPath.c_str()));
    assert(store.sceneAssetCount(0x0Fu) == 28);

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

    /* Same as verified_scene_test: 29 rows → 25 GObjs after the gate. */
    assert(loader.requestScene(0x0Fu));
    const std::size_t created = loader.initSceneGObj(0x0Fu);
    assert(created == 25);

    /* Bind the manifest composition, then pair it onto the GObjs. */
    assert(loader.bindSceneAssets(store, 0x0Fu));
    assert(loader.hasBoundAssets(0x0Fu));
    const std::size_t attached = loader.attachBoundAssetsToGObjs(0x0Fu);
    assert(attached == 28);

    assert(loader.attachmentStore().count() == 28);

    /*
     * Every attachment must live in the active GObj pool (the owner exists)
     * and every asset label must have landed on a real handle.
     */
    std::size_t verifiedOwners = 0;
    loader.attachmentStore().forEach(
        [&](const ico::engine::GObjRenderAttachment& att) {
            ico::engine::GObj* gobj = runtime.pool().get(att.handle);
            assert(gobj != nullptr);
            assert(att.active);
            assert(att.kind == ico::engine::GObjAttachmentKind::Mesh);
            assert(!att.meshPath.empty());
            assert(!att.meshLabel.empty());
            // Transform must start as identity: the asset owns its own space.
            const ico::engine::Matrix4x4& t = att.transform;
            for (int i = 0; i < 16; ++i) {
                if (i % 5 == 0) {
                    assert(t.m[i] == 1.0f);
                } else {
                    assert(t.m[i] == 0.0f);
                }
            }
            ++verifiedOwners;
        });
    assert(verifiedOwners == 28);

    /* Mesh path of attachment i == bound asset i (round-robin pairing). */
    for (std::size_t i = 0; i < attached; ++i) {
        const ico::engine::SceneAssetEntry* asset = loader.boundAsset(0x0Fu, i);
        assert(asset != nullptr);
        const ico::engine::GObjHandle expectedOwner =
            loader.sceneGObjHandle(i % created);
        bool found = false;
        loader.attachmentStore().forEach(
            [&](const ico::engine::GObjRenderAttachment& att) {
                if (att.handle == expectedOwner && att.meshPath == asset->meshPath) {
                    found = true;
                }
            });
        assert(found);
    }

    /*
     * The round-robin spans the full GObj set: with 28 assets over 25 GObjs
     * exactly three handles receive a second attachment (28 = 25 + 3):
     * handles for pairing index 0, 1, 2 own two records each.
     */
    std::size_t doubleOwned = 0;
    for (std::size_t i = 0; i < created; ++i) {
        const ico::engine::GObjHandle probe = loader.sceneGObjHandle(i);
        const std::size_t owners =
            loader.attachmentStore().countFor(probe);
        if (owners > 1) ++doubleOwned;
        assert(owners >= 1);
    }
    assert(doubleOwned == 3);
    assert(loader.attachmentStore().countFor(loader.sceneGObjHandle(0)) == 2);
    assert(loader.attachmentStore().countFor(loader.sceneGObjHandle(3)) == 1);

    /* find() + detach() keep the store coherent. */
    const ico::engine::GObjHandle firstOwner =
        loader.sceneGObjHandle(0 % created);
    const ico::engine::GObjRenderAttachment* found =
        loader.attachmentStore().find(firstOwner);
    assert(found != nullptr);
    assert(found->meshPath == loader.boundAsset(0x0Fu, 0)->meshPath);
    const std::size_t ownedFirst =
        loader.attachmentStore().countFor(firstOwner);
    loader.attachmentStore().detach(firstOwner);
    assert(loader.attachmentStore().count() == 28 - ownedFirst);
    assert(loader.attachmentStore().find(firstOwner) == nullptr);
    assert(loader.attachmentStore().countFor(firstOwner) == 0);

    /* Re-running initSceneGObj rebuilds the GObj set; attachments must not
       outlive the GObjs they point at. */
    assert(loader.initSceneGObj(0x0Fu) == created);
    assert(loader.attachmentStore().count() == 0);

    loader.shutdown();
    runtime.shutdown();

    std::printf("gobj_attachment_test: all passed (%zu assets paired onto %zu "
                "GObjs, %zu double-owned)\n",
                attached, created, doubleOwned);
    return 0;
}