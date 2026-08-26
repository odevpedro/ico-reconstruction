#include "engine/IsysGObjRuntime.h"

#include <cassert>
#include <vector>

using ico::engine::GObj;
using ico::engine::IsysGObjRuntime;
using ico::engine::ProcessNode;
using ico::engine::kPrimaryListCount;
using ico::engine::kTypeSlotStart;
using ico::engine::kTypeSlotEnd;

int main()
{
    IsysGObjRuntime runtime;
    const bool initOk = runtime.initialize(8, 16);
    assert(initOk);
    (void)initOk;

    // Empty mask → no calls
    assert(runtime.dispatchTypeSlots() == 0);
    assert(runtime.dispatchAll() == 0);

    // Add a GObj on list 0 with state_16c = 1
    GObj* gobj = runtime.add(0, 0);
    assert(gobj != nullptr);
    gobj->state_16c = 1;

    // Active mask but no processes → 0 type-slot calls
    assert(runtime.setActiveMask(1u << 0));
    assert(runtime.dispatchTypeSlots() == 0);

    // Register a process with priority matching type slot 19 (kTypeSlotStart)
    std::vector<u32> priorities;
    auto cb = [&](GObj&, ProcessNode& p) {
        priorities.push_back(p.priority);
    };

    ProcessNode* p19 = runtime.registerProcess(*gobj, 1, kTypeSlotStart, cb);
    assert(p19 != nullptr);
    assert(runtime.dispatchTypeSlots() == 1);
    assert((priorities == std::vector<u32>{kTypeSlotStart}));
    assert(runtime.checkInvariants());

    // Register processes for type slots 20 and 22
    priorities.clear();
    ProcessNode* p20 = runtime.registerProcess(*gobj, 1, kTypeSlotStart + 1, cb);
    ProcessNode* p22 = runtime.registerProcess(*gobj, 1, kTypeSlotStart + 3, cb);
    assert(p20 != nullptr && p22 != nullptr);

    // All three should fire: priorities 19, 20, 22
    assert(runtime.dispatchTypeSlots() == 3);
    assert((priorities == std::vector<u32>{
        kTypeSlotStart, kTypeSlotStart + 1, kTypeSlotStart + 3}));
    assert(runtime.checkInvariants());

    // Deactivate p20 → only p19 and p22 fire
    priorities.clear();
    assert(runtime.setProcessActive(*p20, false));
    assert(runtime.dispatchTypeSlots() == 2);
    assert((priorities == std::vector<u32>{
        kTypeSlotStart, kTypeSlotStart + 3}));
    assert(runtime.setProcessActive(*p20, true));

    // state_16c == 0 → entire GObj skipped
    priorities.clear();
    gobj->state_16c = 0;
    assert(runtime.dispatchTypeSlots() == 0);
    gobj->state_16c = 1;
    assert(runtime.dispatchTypeSlots() == 3);

    // dispatchAll = dispatchActiveLists + dispatchTypeSlots
    int gobjCallbacks = 0;
    assert(runtime.setCallback(*gobj, [&](GObj&) { ++gobjCallbacks; }));
    priorities.clear();
    assert(runtime.dispatchAll() == 4);  // 1 gobj callback + 3 process callbacks
    assert(gobjCallbacks == 1);
    assert(priorities.size() == 3);

    // Two GObjs in different lists, different type slots
    GObj* gobj2 = runtime.add(3, 0);
    assert(gobj2 != nullptr);
    gobj2->state_16c = 1;
    assert(runtime.setActiveMask((1u << 0) | (1u << 3)));

    priorities.clear();
    int gobj2Callbacks = 0;
    gobjCallbacks = 0;
    assert(runtime.setCallback(*gobj2, [&](GObj&) { ++gobj2Callbacks; }));
    ProcessNode* p23 = runtime.registerProcess(
        *gobj2, 1, kTypeSlotStart + 4, cb);
    assert(p23 != nullptr);

    // gobj: 3 process callbacks; gobj2: 1 process callback; 2 gobj callbacks
    assert(runtime.dispatchAll() == 6);
    assert(gobjCallbacks == 1);
    assert(gobj2Callbacks == 1);
    assert(priorities.size() == 4);
    assert(runtime.checkInvariants());

    // Mask with only bit 3 → only gobj2's processes fire
    priorities.clear();
    gobjCallbacks = 0;
    gobj2Callbacks = 0;
    assert(runtime.setActiveMask(1u << 3));
    assert(runtime.dispatchAll() == 2);  // 1 gobj + 1 process
    assert(gobjCallbacks == 0);
    assert(gobj2Callbacks == 1);
    assert(priorities.size() == 1);
    assert(priorities[0] == kTypeSlotStart + 4);

    // Pool full → no more registrations
    // (we already used 3 gobjs + 4 processes = 7 of 8 gobj + 16 process slots)
    assert(runtime.pool().activeCount() == 2);
    assert(runtime.processPool().activeCount() == 4);

    // Cleanup: remove gobj2 cascades its processes
    assert(runtime.remove(*gobj2));
    assert(runtime.processPool().activeCount() == 3);
    assert(runtime.checkInvariants());

    // Final dispatch: only gobj's 3 processes
    priorities.clear();
    assert(runtime.setActiveMask(1u << 0));
    assert(runtime.dispatchAll() == 4);  // 1 gobj + 3 process
    assert(priorities.size() == 3);

    // Remove all
    assert(runtime.remove(*gobj));
    assert(runtime.pool().empty());
    assert(runtime.processPool().empty());
    assert(runtime.checkInvariants());

    runtime.shutdown();
    return 0;
}
