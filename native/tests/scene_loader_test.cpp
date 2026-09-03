#include "engine/GifPacket.h"
#include "engine/RenderBackend.h"
#include "game/KanbanSceneLoader.h"

#include <cassert>
#include <cstdio>

int main() {
    IsysGObj runtime;
    assert(runtime.initialize(8, 8));

    KanbanSceneLoader loader;
    assert(loader.initialize(runtime));
    assert(loader.isInitialized());

    SceneGObjDescriptor* desc = loader.descriptor(3);
    assert(desc != nullptr);
    desc->listId = 2;
    desc->hasInitFn = true;

    SceneGObjDescriptor* earlierListDesc = loader.descriptor(4);
    assert(earlierListDesc != nullptr);
    earlierListDesc->listId = 1;

    int initCalls = 0;
    desc->initFn = [&](ico::engine::GObj& gobj, const SceneGObjDescriptor& descriptor) {
        ++initCalls;
        gobj.state_15c = descriptor.descriptorIndex;
    };

    SceneEntryRecord* record = loader.entry(0);
    assert(record != nullptr);
    record->sceneId = 7;
    record->descriptorIndex = 3;
    SceneEntryRecord* earlierListRecord = loader.entry(1);
    assert(earlierListRecord != nullptr);
    earlierListRecord->sceneId = 7;
    earlierListRecord->descriptorIndex = 4;
    earlierListRecord->enabled = true;
    earlierListRecord->sortKey = 1;
    earlierListRecord->userData = 0x5678;

    record->enabled = true;
    record->sortKey = 42;
    record->userData = 0x1234;

    // Rev.112: entry+0x24 wins; entry+0x40 supplies the wrapper's t0 value.
    desc->processCallback_40 = 0x00123456;
    record->processCallback_24 = 0x00654321;
    record->processArgument_40 = 5;
    const SceneProcessRegistrationSpec entryRegistration =
        KanbanSceneLoader::selectProcessRegistration(*record, *desc);
    assert(entryRegistration.callback == 0x00654321);
    assert(entryRegistration.wrapperT0 == 0x1400);
    assert(entryRegistration.usesEntryOverride);

    record->processCallback_24 = 0;
    record->processArgument_40 = 0;
    const SceneProcessRegistrationSpec fallbackRegistration =
        KanbanSceneLoader::selectProcessRegistration(*record, *desc);
    assert(fallbackRegistration.callback == 0x00123456);
    assert(fallbackRegistration.wrapperT0 == 0x1800);
    assert(!fallbackRegistration.usesEntryOverride);

    desc->processCallback_40 = 0;
    const SceneProcessRegistrationSpec noRegistration =
        KanbanSceneLoader::selectProcessRegistration(*record, *desc);
    assert(noRegistration.callback == 0);
    assert(!noRegistration.usesEntryOverride);

    assert(loader.hotInitSceneObjects(7) == 1);
    assert(loader.requestScene(7));
    assert(loader.pendingRequestCount() == 1);
    assert(loader.execute());
    assert(loader.pendingRequestCount() == 0);
    assert(loader.currentSceneId() == 7);

    assert(runtime.pool().activeCount() == 2);
    ico::engine::GObj* gobj = runtime.head(2);
    assert(gobj != nullptr);
    assert(gobj->sort_key == 42);
    assert(gobj->user_data == 0x1234);
    assert(gobj->state_15c == 3);
    assert(initCalls == 1);

    // Host-only bridge: this validates GObj list order reaches GIF commands;
    // it does not claim a game model, texture, or scene placement.
    auto backend = ico::engine::createRenderBackend();
    assert(backend->initialize(64, 64));
    ico::engine::GifPacketBridge bridge(*backend);
    bridge.init(64, 64);
    assert(loader.renderSyntheticScene(bridge) == 0);

    bridge.startPacketPri(0);
    const SyntheticSceneRenderStyle style{10.0f, 20.0f, 8.0f, 10.0f, 1};
    assert(loader.renderSyntheticScene(bridge, style) == 2);
    assert(bridge.commandBuffer().commandCount() == 2);
    const ico::engine::RenderCmd& synthetic = bridge.commandBuffer().command(0);
    assert(synthetic.type == ico::engine::RenderCommand::DrawSprite);
    assert(synthetic.sprite.x == 10.0f);
    assert(synthetic.sprite.y == 20.0f);
    assert(synthetic.sprite.w == 8.0f);
    assert(synthetic.sprite.h == 10.0f);
    const ico::engine::RenderCmd& secondSynthetic = bridge.commandBuffer().command(1);
    assert(secondSynthetic.type == ico::engine::RenderCommand::DrawSprite);
    /* Entry 0 is list 2, but list 1 must be emitted first. */
    assert(synthetic.sprite.x == 10.0f);
    assert(synthetic.sprite.y == 20.0f);
    assert(secondSynthetic.sprite.x == 10.0f);
    assert(secondSynthetic.sprite.y == 30.0f);

    bridge.endPacket();
    assert(bridge.commandBuffer().commandCount() == 0);
    backend->shutdown();
    loader.clearRequests();
    assert(loader.pendingRequestCount() == 0);

    loader.shutdown();
    assert(!loader.isInitialized());
    runtime.shutdown();
    return 0;
}
