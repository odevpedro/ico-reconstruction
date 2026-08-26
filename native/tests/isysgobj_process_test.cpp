#include "engine/IsysGObjRuntime.h"

#include <cassert>
#include <vector>

using ico::engine::GObj;
using ico::engine::IsysGObjRuntime;
using ico::engine::ProcessNode;

int main()
{
    IsysGObjRuntime runtime;
    assert(runtime.initialize(2, 4));

    GObj* gobj = runtime.add(1, 0);
    GObj* other = runtime.add(2, 0);
    assert(gobj != nullptr && other != nullptr);
    assert(gobj->process_head == 0);
    assert(gobj->process_tail == 0);
    assert(runtime.dispatchAllProcesses(*gobj) == 0);

    std::vector<u32> order;
    auto callback = [&](GObj& owner, ProcessNode& process) {
        assert(&owner == gobj);
        order.push_back(process.priority);
    };

    ProcessNode* priority30 = runtime.registerProcess(*gobj, 1, 30, callback);
    ProcessNode* priority10 = runtime.registerProcess(*gobj, 1, 10, callback);
    ProcessNode* priority20 = runtime.registerProcess(*gobj, 1, 20, callback);
    ProcessNode* threadMode = runtime.registerProcess(*gobj, 0, 40, callback);
    assert(priority30 != nullptr && priority10 != nullptr &&
           priority20 != nullptr && threadMode != nullptr);
    assert(runtime.processPool().full());
    assert(runtime.registerProcess(*other, 1, 1, callback) == nullptr);

    assert(runtime.processPool().get(gobj->process_head) == priority10);
    assert(runtime.processPool().get(gobj->process_tail) == threadMode);
    assert(priority10->next == runtime.processPool().handleOf(*priority20));
    assert(priority20->next == runtime.processPool().handleOf(*priority30));
    assert(priority30->next == runtime.processPool().handleOf(*threadMode));
    assert(runtime.checkInvariants());

    assert(runtime.dispatchAllProcesses(*gobj) == 3);
    assert((order == std::vector<u32>{10, 20, 30}));

    order.clear();
    assert(runtime.dispatchProcesses(*gobj, 20) == 1);
    assert((order == std::vector<u32>{20}));

    assert(runtime.setProcessActive(*priority20, false));
    order.clear();
    assert(runtime.dispatchAllProcesses(*gobj) == 2);
    assert((order == std::vector<u32>{10, 30}));
    assert(runtime.setProcessActive(*priority20, true));

    const auto priority20Handle = runtime.processPool().handleOf(*priority20);
    assert(runtime.removeProcess(*priority20));
    assert(priority10->next == runtime.processPool().handleOf(*priority30));
    assert(priority30->prev == runtime.processPool().handleOf(*priority10));

    ProcessNode* reused = runtime.registerProcess(*other, 2, 5,
        [&](GObj& owner, ProcessNode& process) {
            assert(&owner == other);
            order.push_back(process.priority);
        });
    assert(reused != nullptr);
    assert(runtime.processPool().handleOf(*reused) == priority20Handle);
    assert(other->process_head == priority20Handle);
    assert(other->process_tail == priority20Handle);
    assert(runtime.checkInvariants());

    assert(runtime.removeProcess(*priority10));
    assert(runtime.processPool().get(gobj->process_head) == priority30);
    assert(runtime.removeProcess(*threadMode));
    assert(runtime.processPool().get(gobj->process_tail) == priority30);
    assert(runtime.checkInvariants());

    assert(runtime.remove(*gobj));
    assert(priority30->self == 0);
    assert(runtime.processPool().activeCount() == 1);
    assert(runtime.remove(*other));
    assert(runtime.processPool().empty());
    assert(runtime.pool().empty());
    assert(runtime.checkInvariants());

    runtime.shutdown();
    return 0;
}
