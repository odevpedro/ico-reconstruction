#include "engine/IsysGObjRuntime.h"

#include <utility>

namespace ico::engine {

bool IsysGObjRuntime::initialize(std::size_t capacity)
{
    m_pool.initialize(capacity);
    m_heads.fill(kNullGObjHandle);
    m_tails.fill(kNullGObjHandle);
    m_callbacks.clear();
    m_callbacks.resize(capacity);
    m_activeMask = 0;
    m_initialized = true;
    return true;
}

void IsysGObjRuntime::shutdown()
{
    m_pool.clear();
    m_heads.fill(kNullGObjHandle);
    m_tails.fill(kNullGObjHandle);
    m_callbacks.clear();
    m_activeMask = 0;
    m_initialized = false;
}

bool IsysGObjRuntime::isInitialized() const
{
    return m_initialized;
}

GObj* IsysGObjRuntime::add(u8 listId, u32 sortKey, ico_ptr32 userData)
{
    return addImpl(listId, sortKey, userData, false);
}

GObj* IsysGObjRuntime::addHead(u8 listId, u32 sortKey, ico_ptr32 userData)
{
    return addImpl(listId, sortKey, userData, true);
}

GObj* IsysGObjRuntime::addImpl(u8 listId,
                              u32 sortKey,
                              ico_ptr32 userData,
                              bool forceHead)
{
    if (!m_initialized || listId >= kPrimaryListCount) {
        return nullptr;
    }

    GObj* gobj = m_pool.acquire();
    if (gobj == nullptr) {
        return nullptr;
    }

    gobj->type = listId;
    gobj->list_id = listId;
    gobj->sort_key = sortKey;
    gobj->user_data = userData;
    gobj->state_15c = 0;
    gobj->unknown_164 = 0;
    gobj->state_170 = 0;
    insertSorted(*gobj, forceHead);
    return gobj;
}

void IsysGObjRuntime::insertSorted(GObj& gobj, bool forceHead)
{
    const u8 listId = gobj.list_id;
    const GObjHandle handle = m_pool.handleOf(gobj);
    GObjHandle currentHandle = m_heads[listId];

    if (currentHandle == kNullGObjHandle) {
        m_heads[listId] = handle;
        m_tails[listId] = handle;
        return;
    }

    GObj* current = m_pool.get(currentHandle);
    if (forceHead || gobj.sort_key < current->sort_key) {
        gobj.next = currentHandle;
        current->prev = handle;
        m_heads[listId] = handle;
        return;
    }

    for (;;) {
        GObj* next = m_pool.get(current->next);
        if (next == nullptr || gobj.sort_key < next->sort_key) {
            gobj.prev = m_pool.handleOf(*current);
            gobj.next = current->next;
            current->next = handle;
            if (next != nullptr) {
                next->prev = handle;
            } else {
                m_tails[listId] = handle;
            }
            return;
        }
        current = next;
    }
}

bool IsysGObjRuntime::remove(GObj& gobj)
{
    if (!m_initialized || !m_pool.owns(gobj) || isGObjSlotFree(gobj) ||
        gobj.list_id >= kPrimaryListCount) {
        return false;
    }

    const u8 listId = gobj.list_id;
    GObj* next = m_pool.get(gobj.next);
    GObj* prev = m_pool.get(gobj.prev);

    if (prev != nullptr) {
        prev->next = gobj.next;
    } else {
        m_heads[listId] = gobj.next;
    }

    if (next != nullptr) {
        next->prev = gobj.prev;
    } else {
        m_tails[listId] = gobj.prev;
    }

    const GObjHandle handle = m_pool.handleOf(gobj);
    m_callbacks[handle - 1u] = Callback{};
    return m_pool.release(gobj);
}

bool IsysGObjRuntime::setCallback(GObj& gobj, Callback callback)
{
    if (!m_initialized || !m_pool.owns(gobj) || isGObjSlotFree(gobj)) {
        return false;
    }

    const GObjHandle handle = m_pool.handleOf(gobj);
    m_callbacks[handle - 1u] = std::move(callback);
    gobj.callback = m_callbacks[handle - 1u] ? 1u : 0u;
    return true;
}

std::size_t IsysGObjRuntime::dispatchList(u8 listId)
{
    if (!m_initialized || listId >= kPrimaryListCount) {
        return 0;
    }

    std::size_t calls = 0;
    GObjHandle handle = m_heads[listId];
    while (handle != kNullGObjHandle) {
        GObj* gobj = m_pool.get(handle);
        if (gobj == nullptr || isGObjSlotFree(*gobj)) {
            break;
        }

        const GObjHandle next = gobj->next;
        Callback& callback = m_callbacks[handle - 1u];
        if (callback) {
            callback(*gobj);
            ++calls;
        }
        handle = next;
    }
    return calls;
}

std::size_t IsysGObjRuntime::dispatchActiveLists()
{
    std::size_t calls = 0;
    for (u32 bit = 0; bit < kDlMaskBits; ++bit) {
        if ((m_activeMask & (1u << bit)) == 0) {
            continue;
        }
        if (bit >= kPrimaryListCount) {
            return calls;
        }
        calls += dispatchList(static_cast<u8>(bit));
    }
    return calls;
}

bool IsysGObjRuntime::setActiveMask(u32 mask)
{
    constexpr u32 validMask = (1u << kPrimaryListCount) - 1u;
    if ((mask & ~validMask) != 0) {
        return false;
    }
    m_activeMask = mask;
    return true;
}

u32 IsysGObjRuntime::activeMask() const
{
    return m_activeMask;
}

GObj* IsysGObjRuntime::head(u8 listId)
{
    return listId < kPrimaryListCount ? m_pool.get(m_heads[listId]) : nullptr;
}

const GObj* IsysGObjRuntime::head(u8 listId) const
{
    return listId < kPrimaryListCount ? m_pool.get(m_heads[listId]) : nullptr;
}

GObj* IsysGObjRuntime::tail(u8 listId)
{
    return listId < kPrimaryListCount ? m_pool.get(m_tails[listId]) : nullptr;
}

const GObj* IsysGObjRuntime::tail(u8 listId) const
{
    return listId < kPrimaryListCount ? m_pool.get(m_tails[listId]) : nullptr;
}

GObjPool& IsysGObjRuntime::pool()
{
    return m_pool;
}

const GObjPool& IsysGObjRuntime::pool() const
{
    return m_pool;
}

bool IsysGObjRuntime::checkInvariants() const
{
    for (u8 listId = 0; listId < kPrimaryListCount; ++listId) {
        GObjHandle handle = m_heads[listId];
        GObjHandle previous = kNullGObjHandle;
        std::size_t visited = 0;

        if ((m_heads[listId] == kNullGObjHandle) !=
            (m_tails[listId] == kNullGObjHandle)) {
            return false;
        }

        while (handle != kNullGObjHandle) {
            const GObj* gobj = m_pool.get(handle);
            if (gobj == nullptr || isGObjSlotFree(*gobj) ||
                gobj->list_id != listId || gobj->prev != previous) {
                return false;
            }
            previous = handle;
            handle = gobj->next;
            if (++visited > m_pool.activeCount()) {
                return false;
            }
        }

        if (previous != m_tails[listId]) {
            return false;
        }
    }
    return true;
}

} // namespace ico::engine
