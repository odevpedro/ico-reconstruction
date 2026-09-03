#include "engine/IsysGObjRuntime.h"

#include <cassert>

using ico::engine::GObj;
using ico::engine::IsysGObjRuntime;
using ico::engine::kPrimaryListCount;

int main()
{
    IsysGObjRuntime runtime;
    const bool initOk = runtime.initialize(4);
    assert(initOk);
    (void)initOk;
    assert(runtime.isInitialized());
    assert(runtime.pool().empty());

    for (u8 listId = 0; listId < kPrimaryListCount; ++listId) {
        assert(runtime.head(listId) == nullptr);
        assert(runtime.tail(listId) == nullptr);
        assert(runtime.dispatchList(listId) == 0);
    }
    assert(runtime.checkInvariants());

    GObj* middle = runtime.add(2, 20, 0x2000);
    GObj* head = runtime.add(2, 10, 0x1000);
    GObj* tail = runtime.add(2, 30, 0x3000);
    GObj* other = runtime.add(5, 1, 0x5000);
    assert(middle != nullptr && head != nullptr && tail != nullptr && other != nullptr);
    assert(runtime.pool().full());
    assert(runtime.add(2, 40) == nullptr);

    // Enable the confirmed _iosOmMain dispatch gates (+0x16C and +0x170).
    head->state_16c = 1;
    head->state_170 = 1;
    middle->state_16c = 1;
    middle->state_170 = 1;
    tail->state_16c = 1;
    tail->state_170 = 1;

    assert(runtime.head(2) == head);
    assert(runtime.tail(2) == tail);
    assert(head->next == runtime.pool().handleOf(*middle));
    assert(middle->prev == runtime.pool().handleOf(*head));
    assert(middle->next == runtime.pool().handleOf(*tail));
    assert(tail->prev == runtime.pool().handleOf(*middle));
    assert(runtime.checkInvariants());

    int callbackCount = 0;
    u32 callbackSum = 0;
    assert(runtime.setCallback(*head, [&](GObj& gobj) {
        ++callbackCount;
        callbackSum += gobj.sort_key;
    }));
    assert(runtime.setCallback(*middle, [&](GObj& gobj) {
        ++callbackCount;
        callbackSum += gobj.sort_key;
    }));
    assert(runtime.setCallback(*tail, [&](GObj& gobj) {
        ++callbackCount;
        callbackSum += gobj.sort_key;
    }));
    assert(runtime.dispatchList(2) == 3);
    assert(callbackCount == 3);
    assert(callbackSum == 60);

    assert(runtime.setActiveMask(1u << 2));
    assert(runtime.dispatchActiveLists() == 3);
    assert(callbackCount == 6);
    assert(!runtime.setActiveMask(1u << 8));
    assert(runtime.activeMask() == (1u << 2));

    const auto middleHandle = runtime.pool().handleOf(*middle);
    assert(runtime.remove(*middle));
    assert(head->next == runtime.pool().handleOf(*tail));
    assert(tail->prev == runtime.pool().handleOf(*head));
    assert(runtime.checkInvariants());

    GObj* reused = runtime.addHead(2, 99, 0x9999);
    assert(reused != nullptr);
    assert(runtime.pool().handleOf(*reused) == middleHandle);
    assert(runtime.head(2) == reused);
    assert(reused->next == runtime.pool().handleOf(*head));
    assert(head->prev == runtime.pool().handleOf(*reused));
    assert(runtime.checkInvariants());

    assert(runtime.remove(*reused));
    assert(runtime.remove(*head));
    assert(runtime.head(2) == tail);
    assert(runtime.tail(2) == tail);
    assert(runtime.remove(*tail));
    assert(runtime.head(2) == nullptr);
    assert(runtime.tail(2) == nullptr);
    assert(runtime.dispatchList(2) == 0);
    assert(runtime.remove(*other));
    assert(runtime.pool().empty());
    assert(runtime.checkInvariants());

    runtime.shutdown();
    assert(!runtime.isInitialized());
    return 0;
}
