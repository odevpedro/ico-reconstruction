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

    /*
     * Rev.159 (Passo 3) — verified per-room handler repertoire drives the
     * GObjHandle->handler binding (scene 0x2B role plan).
     *
     * Scene 0x2B has a runtime-verified plan in GeneratedRoomRoleTables.h
     * (Rev.158): 26 slots / 8 handlers (flag_hB x5, torch_hB x5, type36_hB x5,
     * type60_hB x5, type6_hB x3, type39_hB x1, type22_hB x1, boy_hB x1).
     * initSceneGObj(0x2B) creates 23 GObjs (25 payload rows minus 2 gate-0
     * descriptors: DYNAMICMOTIONDAT, STAGESETTING). The loader must:
     *   - consume the generated plans and recognize the room;
     *   - tag each created GObj with a handler from the room's repertoire
     *     (the GObjHandle->handler map = role slots expanded in order);
     *   - pair the bound assets while keeping the round-robin within the
     *     tagged set (asset label -> handler is still a HOST heuristic);
     *   - re-pair on every execute() transition (Passo 2), not only once.
     */
    assert(loader.applyVerifiedRoomRolePlans(
        ico::engine::kVerifiedRoomRolePlans,
        ico::engine::kVerifiedRoomRolePlanCount));
    assert(loader.hasRoomRolePlan(0x2Bu));
    /* 0x0F (demo room) is not in the runtime capture: no plan, round-robin
       fallback stays the documented behavior for it. */
    assert(!loader.hasRoomRolePlan(0x0Fu));

    /* Verify the 0x2B plan contents we are about to assert against. */
    const ico::engine::VerifiedRoomRolePlan* plan2B = nullptr;
    for (std::size_t i = 0; i < ico::engine::kVerifiedRoomRolePlanCount; ++i) {
        if (ico::engine::kVerifiedRoomRolePlans[i].sceneId == 0x2Bu) {
            plan2B = &ico::engine::kVerifiedRoomRolePlans[i];
            break;
        }
    }
    assert(plan2B != nullptr);
    assert(plan2B->roleCount == 8);
    std::size_t planSlots2B = 0;
    for (u16 r = 0; r < plan2B->roleCount; ++r) {
        planSlots2B += plan2B->roles[r].roleCount;
    }
    assert(planSlots2B == 26);

    /* Bind the 0x2B block (host test data reusing the room piece library). */
    assert(store.sceneAssetCount(0x2Bu) == 28);
    assert(loader.bindSceneAssets(store, 0x2Bu));
    assert(loader.hasBoundAssets(0x2Bu));

    assert(loader.requestScene(0x2Bu));
    const std::size_t created2B = loader.initSceneGObj(0x2Bu);
    assert(created2B == 23);

    const std::size_t attached2B = loader.attachBoundAssetsToGObjs(0x2Bu);
    assert(attached2B == 28);
    assert(loader.attachmentStore().count() == 28);

    /*
     * Every created GObj must carry a handler tag taken from the 0x2B
     * repertoire. The tags are the role-slot expansion cycling the plan order
     * (flag x5, torch x5, type36 x5, type60 x5, type6 x3, ...) over the 23
     * host GObjs: slot 0..22 of the expanded 26-slot list. Assert the exact
     * resulting multiset — a concrete, deterministic binding.
     */
    struct ExpectedRole {
        ico_ptr32 addr;
        std::size_t tagCount;
    };
    const ExpectedRole kExpected2B[] = {
        {0x001D00F8u, 5},
        {0x001F1CF0u, 5},
        {0x001F44C8u, 5},
        {0x0023D518u, 5},
        {0x001CE6F0u, 3},
        {0x0010D070u, 0},
        {0x001BC1A8u, 0},
        {0x001C1DD8u, 0},
    };
    std::size_t taggedTotal = 0;
    for (std::size_t i = 0; i < created2B; ++i) {
        const ico_ptr32 handler = loader.gobjHandlerRole(i);
        assert(handler != 0);
        bool inRepertoire = false;
        for (u16 r = 0; r < plan2B->roleCount; ++r) {
            if (plan2B->roles[r].handlerAddr == handler) {
                inRepertoire = true;
                break;
            }
        }
        assert(inRepertoire);
        ++taggedTotal;
    }
    assert(taggedTotal == 23);

    /* Multiset check: count how many GObjs carry each plan handler. */
    for (const ExpectedRole& e : kExpected2B) {
        std::size_t n = 0;
        for (std::size_t i = 0; i < created2B; ++i) {
            if (loader.gobjHandlerRole(i) == e.addr) {
                ++n;
            }
        }
        assert(n == e.tagCount);
    }

    /* Passo 2: execute() re-pairs on transition — a fresh scene load re-links
       attachments without an explicit attach call after initSceneGObj.
       clearRequests() first: requestScene() enqueues and execute() pops the
       front; the earlier scene-0x0F request must not be consumed here. */
    loader.clearRequests();
    assert(loader.requestScene(0x2Bu));
    assert(loader.execute());
    assert(loader.attachmentStore().count() == 28);
    assert(loader.sceneGObjCount() == 23);

    /* The attachment owners from the relink are the freshly created GObjs. */
    std::size_t relinkedOwners = 0;
    loader.attachmentStore().forEach(
        [&](const ico::engine::GObjRenderAttachment& att) {
            ico::engine::GObj* gobj = runtime.pool().get(att.handle);
            assert(gobj != nullptr);
            ++relinkedOwners;
        });
    assert(relinkedOwners == 28);

    /* A role plan for a room with no bound assets must stay inert. */
    assert(!loader.bindSceneAssets(store, 0x07u) ||
           loader.attachmentStore().count() == 28);

    loader.shutdown();
    runtime.shutdown();

    std::printf("gobj_attachment_test: all passed (%zu assets paired onto %zu "
                "GObjs, %zu double-owned; %zu tagged onto %zu GObjs)\n",
                attached, created, doubleOwned, attached2B, created2B);
    return 0;
}