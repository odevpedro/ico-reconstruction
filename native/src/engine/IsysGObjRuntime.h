#pragma once

#include "engine/GObjPool.h"
#include "engine/ProcessNodePool.h"

#include <array>
#include <cstddef>
#include <functional>
#include <vector>

namespace ico::engine {

class IsysGObjRuntime {
public:
    using Callback = std::function<void(GObj&)>;
    using ProcessCallback = std::function<void(GObj&, ProcessNode&)>;

    bool initialize(std::size_t gobjCapacity = 0x140,
                    std::size_t processCapacity = 0x500);
    void shutdown();
    bool isInitialized() const;

    GObj* add(u8 listId, u32 sortKey = 0, ico_ptr32 userData = 0);
    GObj* addHead(u8 listId, u32 sortKey = 0, ico_ptr32 userData = 0);
    GObj* addAfter(GObj& reference, ico_ptr32 userData = 0);
    GObj* addBefore(GObj& reference, ico_ptr32 userData = 0);
    bool move(GObj& gobj, u8 listId, u32 sortKey);
    bool moveBefore(GObj& gobj, GObj& reference);
    bool moveAfter(GObj& gobj, GObj& reference);

    // Mirrors the verified 0x6A93D0 kind-table and gp-0x6730 gate.
    bool setKindTableDisabled(bool disabled);
    bool setKind(GObj& gobj, u32 kind);
    GObj* kindHead(u32 kind);
    const GObj* kindHead(u32 kind) const;
    bool remove(GObj& gobj);

    bool setCallback(GObj& gobj, Callback callback);
    std::size_t dispatchList(u8 listId);
    std::size_t dispatchActiveLists();
    ProcessNode* registerProcess(GObj& gobj,
                                 u32 typeMask,
                                 u32 priority,
                                 ProcessCallback callback = {});
    bool removeProcess(ProcessNode& process);
    bool setProcessActive(ProcessNode& process, bool active);
    std::size_t dispatchProcesses(GObj& gobj, u32 priority);
    std::size_t dispatchAllProcesses(GObj& gobj);


    bool setActiveMask(u32 mask);
    u32 activeMask() const;

    std::size_t dispatchTypeSlots();
    std::size_t dispatchAll();

    GObj* head(u8 listId);
    const GObj* head(u8 listId) const;
    GObj* tail(u8 listId);
    const GObj* tail(u8 listId) const;

    GObjPool& pool();
    const GObjPool& pool() const;

    ProcessNodePool& processPool();
    const ProcessNodePool& processPool() const;
    bool checkInvariants() const;

private:
    GObj* addImpl(u8 listId, u32 sortKey, ico_ptr32 userData, bool forceHead);
    GObj* addRelative(GObj& reference, ico_ptr32 userData, bool before);
    void insertSorted(GObj& gobj, bool forceHead);
    void unlinkPrimary(GObj& gobj);
    void unlinkKind(GObj& gobj);

    bool m_initialized = false;
    void insertProcessSorted(GObj& gobj, ProcessNode& process);
    void removeAllProcesses(GObj& gobj);
    std::size_t dispatchProcessesImpl(GObj& gobj,
                                      bool filterPriority,
                                      u32 priority);

    GObjPool m_pool;
    std::array<GObjHandle, kPrimaryListCount> m_heads{};
    std::array<GObjHandle, kPrimaryListCount> m_tails{};
    std::array<GObjHandle, kTypeTableEntries> m_kindHeads{};
    bool m_kindTableDisabled = false;
    std::vector<Callback> m_callbacks;
    ProcessNodePool m_processPool;
    std::vector<ProcessCallback> m_processCallbacks;
    u32 m_activeMask = 0;
};

} // namespace ico::engine
