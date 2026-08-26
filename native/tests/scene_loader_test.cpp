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

    int initCalls = 0;
    desc->initFn = [&](ico::engine::GObj& gobj, const SceneGObjDescriptor& descriptor) {
        ++initCalls;
        gobj.state_15c = descriptor.descriptorIndex;
    };

    SceneEntryRecord* record = loader.entry(0);
    assert(record != nullptr);
    record->sceneId = 7;
    record->descriptorIndex = 3;
    record->enabled = true;
    record->sortKey = 42;
    record->userData = 0x1234;

    assert(loader.hotInitSceneObjects(7) == 1);
    assert(loader.requestScene(7));
    assert(loader.pendingRequestCount() == 1);
    assert(loader.execute());
    assert(loader.pendingRequestCount() == 0);
    assert(loader.currentSceneId() == 7);

    assert(runtime.pool().activeCount() == 1);
    ico::engine::GObj* gobj = runtime.head(2);
    assert(gobj != nullptr);
    assert(gobj->sort_key == 42);
    assert(gobj->user_data == 0x1234);
    assert(gobj->state_15c == 3);
    assert(initCalls == 1);

    loader.clearRequests();
    assert(loader.pendingRequestCount() == 0);

    loader.shutdown();
    assert(!loader.isInitialized());
    runtime.shutdown();
    return 0;
}
