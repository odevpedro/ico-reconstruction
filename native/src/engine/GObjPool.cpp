#include "engine/GObjPool.h"

#include <functional>
#include <limits>

namespace ico::engine {

GObjPool::GObjPool(std::size_t capacity)
{
    initialize(capacity);
}

void GObjPool::initialize(std::size_t capacity)
{
    const auto maxCapacity = static_cast<std::size_t>(
        std::numeric_limits<GObjHandle>::max() - 1u);
    if (capacity > maxCapacity) {
        capacity = maxCapacity;
    }

    m_slots.resize(capacity);
    clear();
}

void GObjPool::clear()
{
    for (GObj& gobj : m_slots) {
        resetGObjSlot(gobj);
    }
    m_activeCount = 0;
}

GObj* GObjPool::acquire()
{
    for (std::size_t i = 0; i < m_slots.size(); ++i) {
        GObj& gobj = m_slots[i];
        if (!isGObjSlotFree(gobj)) {
            continue;
        }

        resetGObjSlot(gobj);
        gobj.self = static_cast<GObjHandle>(i + 1u);
        ++m_activeCount;
        return &gobj;
    }
    return nullptr;
}

bool GObjPool::release(GObj& gobj)
{
    if (!owns(gobj) || isGObjSlotFree(gobj)) {
        return false;
    }
    resetGObjSlot(gobj);
    --m_activeCount;
    return true;
}

GObj* GObjPool::get(GObjHandle handle)
{
    if (handle == kNullGObjHandle || handle > m_slots.size()) {
        return nullptr;
    }
    return &m_slots[handle - 1u];
}

const GObj* GObjPool::get(GObjHandle handle) const
{
    if (handle == kNullGObjHandle || handle > m_slots.size()) {
        return nullptr;
    }
    return &m_slots[handle - 1u];
}

GObjHandle GObjPool::handleOf(const GObj& gobj) const
{
    if (!owns(gobj)) {
        return kNullGObjHandle;
    }
    return static_cast<GObjHandle>(&gobj - m_slots.data()) + 1u;
}

bool GObjPool::owns(const GObj& gobj) const
{
    if (m_slots.empty()) {
        return false;
    }
    const GObj* begin = m_slots.data();
    const GObj* end = begin + m_slots.size();
    const std::less<const GObj*> less;
    return !less(&gobj, begin) && less(&gobj, end);
}

std::size_t GObjPool::capacity() const
{
    return m_slots.size();
}

std::size_t GObjPool::activeCount() const
{
    return m_activeCount;
}

bool GObjPool::empty() const
{
    return m_activeCount == 0;
}

bool GObjPool::full() const
{
    return m_activeCount == m_slots.size();
}

} // namespace ico::engine
