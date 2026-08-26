#include "engine/GObjPool.h"

#include <cassert>
#include <cstddef>

using ico::engine::GObj;
using ico::engine::GObjPool;

int main()
{
    static_assert(sizeof(GObj) == 0x174);
    static_assert(sizeof(IcoProcessNode) == 0x94);
    static_assert(offsetof(GObj, callback) == 0x48);
    static_assert(offsetof(GObj, state_15c) == 0x15C);

    GObjPool pool(2);
    GObj foreign{};
    assert(pool.capacity() == 2);
    assert(pool.empty());
    assert(!pool.full());
    assert(!pool.owns(foreign));
    assert(!pool.release(foreign));

    GObj* first = pool.acquire();
    GObj* second = pool.acquire();
    assert(first != nullptr);
    assert(second != nullptr);
    assert(first != second);
    assert(pool.activeCount() == 2);
    assert(pool.full());
    assert(pool.acquire() == nullptr);

    const auto firstHandle = pool.handleOf(*first);
    first->callback = 0x1234;
    assert(pool.release(*first));
    assert(pool.activeCount() == 1);
    assert(first->self == 0);
    assert(first->callback == 0);

    GObj* reused = pool.acquire();
    assert(reused == first);
    assert(pool.handleOf(*reused) == firstHandle);
    assert(reused->unknown_004 == -1);
    assert(reused->unknown_008 == -1);
    assert(pool.full());

    assert(pool.release(*second));
    assert(pool.release(*reused));
    assert(pool.empty());
    assert(!pool.release(*reused));
    return 0;
}
