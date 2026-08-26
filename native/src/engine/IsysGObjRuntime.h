#pragma once

#include "engine/GObjPool.h"

#include <array>
#include <cstddef>
#include <functional>
#include <vector>

namespace ico::engine {

class IsysGObjRuntime {
public:
    using Callback = std::function<void(GObj&)>;

    bool initialize(std::size_t capacity = 0x140);
    void shutdown();
    bool isInitialized() const;

    GObj* add(u8 listId, u32 sortKey = 0, ico_ptr32 userData = 0);
    GObj* addHead(u8 listId, u32 sortKey = 0, ico_ptr32 userData = 0);
    bool remove(GObj& gobj);

    bool setCallback(GObj& gobj, Callback callback);
    std::size_t dispatchList(u8 listId);
    std::size_t dispatchActiveLists();

    bool setActiveMask(u32 mask);
    u32 activeMask() const;

    GObj* head(u8 listId);
    const GObj* head(u8 listId) const;
    GObj* tail(u8 listId);
    const GObj* tail(u8 listId) const;

    GObjPool& pool();
    const GObjPool& pool() const;

    bool checkInvariants() const;

private:
    GObj* addImpl(u8 listId, u32 sortKey, ico_ptr32 userData, bool forceHead);
    void insertSorted(GObj& gobj, bool forceHead);

    bool m_initialized = false;
    GObjPool m_pool;
    std::array<GObjHandle, kPrimaryListCount> m_heads{};
    std::array<GObjHandle, kPrimaryListCount> m_tails{};
    std::vector<Callback> m_callbacks;
    u32 m_activeMask = 0;
};

} // namespace ico::engine
