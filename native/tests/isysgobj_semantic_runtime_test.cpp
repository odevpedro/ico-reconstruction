#include "engine/IsysGObjRuntime.h"

#include <cassert>

using ico::engine::GObj;
using ico::engine::IsysGObjRuntime;

int main()
{
    IsysGObjRuntime runtime;
    assert(runtime.initialize(8, 4));

    GObj* first = runtime.add(2, 20, 0x1000);
    GObj* second = runtime.add(2, 30, 0x2000);
    assert(first != nullptr && second != nullptr);

    GObj* before = runtime.addBefore(*second, 0x3000);
    assert(before != nullptr);
    assert(runtime.head(2) == first);
    assert(first->next == runtime.pool().handleOf(*before));
    assert(before->next == runtime.pool().handleOf(*second));
    assert(runtime.checkInvariants());

    assert(runtime.moveAfter(*first, *second));
    assert(runtime.head(2) == before);
    assert(before->next == runtime.pool().handleOf(*second));
    assert(second->next == runtime.pool().handleOf(*first));
    assert(runtime.tail(2) == first);

    assert(runtime.setKind(*before, 5));
    assert(runtime.setKind(*second, 5));
    assert(runtime.kindHead(5) == before);
    assert(before->type_next == runtime.pool().handleOf(*second));
    assert(runtime.checkInvariants());

    assert(runtime.remove(*before));
    assert(runtime.kindHead(5) == second);
    assert(runtime.head(2) == second);
    assert(runtime.checkInvariants());

    assert(runtime.setKindTableDisabled(true));
    assert(runtime.setKind(*second, 7));
    assert(runtime.kindHead(7) == nullptr);
    assert(runtime.checkInvariants());

    assert(runtime.remove(*first));
    assert(runtime.remove(*second));
    assert(runtime.pool().empty());
    assert(runtime.checkInvariants());
    return 0;
}
