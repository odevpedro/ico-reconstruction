#include "engine/ProcessNodePool.h"

#include <cassert>
#include <cstddef>

using ico::engine::ProcessNode;
using ico::engine::ProcessNodePool;

int main()
{
    static_assert(sizeof(ProcessNode) == 0x94);
    static_assert(offsetof(ProcessNode, parent) == 0x04);
    static_assert(offsetof(ProcessNode, next) == 0x08);
    static_assert(offsetof(ProcessNode, prev) == 0x0C);
    static_assert(offsetof(ProcessNode, priority) == 0x14);
    static_assert(offsetof(ProcessNode, callback) == 0x1C);

    ProcessNodePool pool(2);
    ProcessNode foreign{};
    assert(pool.capacity() == 2);
    assert(pool.empty());
    assert(!pool.full());
    assert(!pool.owns(foreign));
    assert(!pool.release(foreign));

    ProcessNode* first = pool.acquire(7, 1, 30);
    ProcessNode* second = pool.acquire(7, 2, 10);
    assert(first != nullptr && second != nullptr);
    assert(first->parent == 7);
    assert(first->type_mask == 1);
    assert(first->priority == 30);
    assert(first->active == 1);
    assert(pool.full());
    assert(pool.acquire(7, 1, 20) == nullptr);

    const auto firstHandle = pool.handleOf(*first);
    first->callback = 0x1234;
    assert(pool.release(*first));
    assert(first->self == 0);
    assert(first->callback == 0);

    ProcessNode* reused = pool.acquire(9, 4, 20);
    assert(reused == first);
    assert(pool.handleOf(*reused) == firstHandle);
    assert(reused->parent == 9);
    assert(reused->next == 0);
    assert(reused->prev == 0);

    assert(pool.release(*second));
    assert(pool.release(*reused));
    assert(pool.empty());
    assert(!pool.release(*reused));
    return 0;
}
