#include "engine/GifPacket.h"
#include "engine/RenderBackend.h"
#include "game/KanbanSceneLoader.h"
#include "runtime/GameLoop.h"
#include "runtime/IcoRuntime.h"

#include <cassert>
#include <cstdio>
#include <string>
#include <vector>

/*
 * Host-only integration: GameLoop drives KanbanSceneLoader->execute()
 * (initSceneGObj) and pipes the scene's static debug view into a
 * GifPacketBridge backed by the engine RenderBackend. This validates the
 * frame-by-frame seam (vblank -> ACTGame -> kanbanExec -> initSceneGObj)
 * without claiming original geometry, textures, or placement.
 */
int main() {
    IcoRuntime runtime;
    assert(runtime.initialize());

    const VerifiedSceneDescriptorRecord verifiedDescriptor[] = {
        {2, 0x0015f298},
    };
    assert(runtime.getSceneLoader().applyVerifiedDescriptorRecords(verifiedDescriptor, 1));

    SceneGObjDescriptor* desc = runtime.getSceneLoader().descriptor(2);
    assert(desc != nullptr);
    desc->listId = 0;
    desc->hasInitFn = true;

    SceneEntryRecord* record = runtime.getSceneLoader().entry(3);
    assert(record != nullptr);
    record->sceneId = 7;
    record->descriptorIndex = 2;
    record->enabled = true;
    record->sortKey = 9;
    record->userData = 0xAABB;

    auto backend = ico::engine::createRenderBackend();
    assert(backend->initialize(64, 64));
    ico::engine::GifPacketBridge bridge(*backend);
    bridge.init(64, 64);

    int sceneLoads = 0;
    int framesRendered = 0;

    runtime.getGameLoop().setUpdateCallback([&](u32 frame) -> bool {
        if (frame == 1) {
            /* Request + execute resolve on the same tick (kanbanExec / la_load_processing). */
            assert(runtime.getSceneLoader().requestScene(7));
            assert(runtime.getSceneLoader().execute());
            assert(runtime.getSceneLoader().currentSceneId() == 7);
            sceneLoads++;
            return true;
        }

        /* Each subsequent frame renders the current scene through the GIF bridge. */
        bridge.startPacketPri(0);
        const std::size_t sprites =
            runtime.getSceneLoader().renderStaticSceneDebugView(
                bridge, /* style = */ {}, [](const StaticSceneDebugItem&, float, float) {
                });
        assert(sprites == 1);
        assert(bridge.commandBuffer().commandCount() == 1);
        assert(bridge.commandBuffer().command(0).type ==
               ico::engine::RenderCommand::DrawSprite);
        bridge.endPacket();
        framesRendered++;
        return true;
    });

    runtime.getGameLoop().run(4);

    assert(sceneLoads == 1);
    assert(framesRendered == 3);

    const std::vector<StaticSceneDebugItem> debugItems =
        runtime.getSceneLoader().staticSceneDebugItems();
    assert(debugItems.size() == 1);
    assert(debugItems[0].descriptorIndex == 2);
    assert(debugItems[0].sceneId == 7);
    assert(debugItems[0].sortKey == 9);
    assert(debugItems[0].label == "scene=7 descriptor=2 gobj.type=0 list=0 sort=9 handle=1");

    backend->shutdown();
    runtime.shutdown();

    std::printf("game_loop_scene_test: all passed\n");
    return 0;
}