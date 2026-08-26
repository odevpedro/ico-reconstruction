#include "engine/IsysGObjRuntime.h"

#include <utility>

namespace ico::engine {

bool IsysGObjRuntime::initialize(std::size_t gobjCapacity,
                                 std::size_t processCapacity)
{
    m_pool.initialize(gobjCapacity);
    m_processPool.initialize(processCapacity);
    m_heads.fill(kNullGObjHandle);
    m_tails.fill(kNullGObjHandle);
    m_callbacks.clear();
    m_callbacks.resize(gobjCapacity);
    m_processCallbacks.clear();
    m_processCallbacks.resize(processCapacity);
    m_activeMask = 0;
    m_initialized = true;
    return true;
}

void IsysGObjRuntime::shutdown()
{
    m_processPool.clear();
    m_pool.clear();
    m_heads.fill(kNullGObjHandle);
    m_tails.fill(kNullGObjHandle);
    m_callbacks.clear();
    m_processCallbacks.clear();
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

    removeAllProcesses(gobj);

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
        Callback callback = m_callbacks[handle - 1u];
        if (callback) {
            callback(*gobj);
            ++calls;
        }
        handle = next;
    }
    return calls;
}

ProcessNode* IsysGObjRuntime::registerProcess(GObj& gobj,
                                              u32 typeMask,
                                              u32 priority,
                                              ProcessCallback callback)
{
    if (!m_initialized || !m_pool.owns(gobj) || isGObjSlotFree(gobj)) {
        return nullptr;
    }

    ProcessNode* process = m_processPool.acquire(
        m_pool.handleOf(gobj), typeMask, priority);
    if (process == nullptr) {
        return nullptr;
    }

    const ProcessHandle handle = m_processPool.handleOf(*process);
    m_processCallbacks[handle - 1u] = std::move(callback);
    process->callback = m_processCallbacks[handle - 1u] ? 1u : 0u;
    insertProcessSorted(gobj, *process);
    return process;
}

void IsysGObjRuntime::insertProcessSorted(GObj& gobj, ProcessNode& process)
{
    const ProcessHandle handle = m_processPool.handleOf(process);
    ProcessHandle currentHandle = gobj.process_head;

    if (currentHandle == kNullProcessHandle) {
        gobj.process_head = handle;
        gobj.process_tail = handle;
        return;
    }

    ProcessNode* current = m_processPool.get(currentHandle);
    if (process.priority < current->priority) {
        process.next = currentHandle;
        current->prev = handle;
        gobj.process_head = handle;
        return;
    }

    for (;;) {
        ProcessNode* next = m_processPool.get(current->next);
        if (next == nullptr || process.priority < next->priority) {
            process.prev = m_processPool.handleOf(*current);
            process.next = current->next;
            current->next = handle;
            if (next != nullptr) {
                next->prev = handle;
            } else {
                gobj.process_tail = handle;
            }
            return;
        }
        current = next;
    }
}

bool IsysGObjRuntime::removeProcess(ProcessNode& process)
{
    if (!m_initialized || !m_processPool.owns(process) ||
        isProcessNodeSlotFree(process)) {
        return false;
    }

    GObj* parent = m_pool.get(process.parent);
    if (parent == nullptr || isGObjSlotFree(*parent)) {
        return false;
    }

    ProcessNode* next = m_processPool.get(process.next);
    ProcessNode* prev = m_processPool.get(process.prev);

    if (prev != nullptr) {
        prev->next = process.next;
    } else {
        parent->process_head = process.next;
    }

    if (next != nullptr) {
        next->prev = process.prev;
    } else {
        parent->process_tail = process.prev;
    }

    const ProcessHandle handle = m_processPool.handleOf(process);
    m_processCallbacks[handle - 1u] = ProcessCallback{};
    return m_processPool.release(process);
}

void IsysGObjRuntime::removeAllProcesses(GObj& gobj)
{
    while (gobj.process_head != kNullProcessHandle) {
        ProcessNode* process = m_processPool.get(gobj.process_head);
        if (process == nullptr || !removeProcess(*process)) {
            gobj.process_head = kNullProcessHandle;
            gobj.process_tail = kNullProcessHandle;
            return;
        }
    }
}

bool IsysGObjRuntime::setProcessActive(ProcessNode& process, bool active)
{
    if (!m_initialized || !m_processPool.owns(process) ||
        isProcessNodeSlotFree(process)) {
        return false;
    }
    process.active = active ? 1u : 0u;
    return true;
}

std::size_t IsysGObjRuntime::dispatchProcesses(GObj& gobj, u32 priority)
{
    return dispatchProcessesImpl(gobj, true, priority);
}

std::size_t IsysGObjRuntime::dispatchAllProcesses(GObj& gobj)
{
    return dispatchProcessesImpl(gobj, false, 0);
}

std::size_t IsysGObjRuntime::dispatchProcessesImpl(GObj& gobj,
                                                   bool filterPriority,
                                                   u32 priority)
{
    if (!m_initialized || !m_pool.owns(gobj) || isGObjSlotFree(gobj)) {
        return 0;
    }

    std::size_t calls = 0;
    ProcessHandle handle = gobj.process_head;
    while (handle != kNullProcessHandle) {
        ProcessNode* process = m_processPool.get(handle);
        if (process == nullptr || isProcessNodeSlotFree(*process)) {
            break;
        }

        const ProcessHandle next = process->next;
        ProcessCallback callback = m_processCallbacks[handle - 1u];
        if (process->active != 0 && process->type_mask != 0 && callback &&
            (!filterPriority || process->priority == priority)) {
            callback(gobj, *process);
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

std::size_t IsysGObjRuntime::dispatchTypeSlots()
{
    if (!m_initialized) {
        return 0;
    }

    std::size_t calls = 0;
    for (u32 bit = 0; bit < kPrimaryListCount; ++bit) {
        if ((m_activeMask & (1u << bit)) == 0) {
            continue;
        }

        GObjHandle gobjHandle = m_heads[bit];
        while (gobjHandle != kNullGObjHandle) {
            GObj* gobj = m_pool.get(gobjHandle);
            if (gobj == nullptr || isGObjSlotFree(*gobj)) {
                break;
            }

            const GObjHandle nextGObj = gobj->next;

            if (gobj->state_16c == 0) {
                gobjHandle = nextGObj;
                continue;
            }

            for (u32 typeSlot = kTypeSlotStart; typeSlot < kTypeSlotEnd;
                 ++typeSlot) {
                ProcessHandle processHandle = gobj->process_head;
                while (processHandle != kNullProcessHandle) {
                    ProcessNode* process = m_processPool.get(processHandle);
                    if (process == nullptr ||
                        isProcessNodeSlotFree(*process)) {
                        break;
                    }

                    const ProcessHandle nextProcess = process->next;

                    if (process->priority == typeSlot &&
                        process->active != 0) {
                        const ProcessHandle pHandle =
                            m_processPool.handleOf(*process);
                        ProcessCallback& cb =
                            m_processCallbacks[pHandle - 1u];
                        if (cb) {
                            cb(*gobj, *process);
                            ++calls;
                        }
                    }

                    processHandle = nextProcess;
                }
            }

            gobjHandle = nextGObj;
        }
    }
    return calls;
}

std::size_t IsysGObjRuntime::dispatchAll()
{
    return dispatchActiveLists() + dispatchTypeSlots();
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

ProcessNodePool& IsysGObjRuntime::processPool()
{
    return m_processPool;
}

const ProcessNodePool& IsysGObjRuntime::processPool() const
{
    return m_processPool;
}

bool IsysGObjRuntime::checkInvariants() const
{
    std::size_t visitedGObjs = 0;
    std::size_t visitedProcesses = 0;

    for (u8 listId = 0; listId < kPrimaryListCount; ++listId) {
        GObjHandle handle = m_heads[listId];
        GObjHandle previous = kNullGObjHandle;
        std::size_t visitedInList = 0;

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

            ProcessHandle processHandle = gobj->process_head;
            ProcessHandle processPrevious = kNullProcessHandle;
            u32 previousPriority = 0;
            bool havePriority = false;
            std::size_t objectProcesses = 0;

            if ((gobj->process_head == kNullProcessHandle) !=
                (gobj->process_tail == kNullProcessHandle)) {
                return false;
            }

            while (processHandle != kNullProcessHandle) {
                const ProcessNode* process = m_processPool.get(processHandle);
                if (process == nullptr || isProcessNodeSlotFree(*process) ||
                    process->parent != handle ||
                    process->prev != processPrevious ||
                    (havePriority && process->priority < previousPriority)) {
                    return false;
                }

                havePriority = true;
                previousPriority = process->priority;
                processPrevious = processHandle;
                processHandle = process->next;
                ++objectProcesses;
                ++visitedProcesses;
                if (objectProcesses > m_processPool.activeCount()) {
                    return false;
                }
            }

            if (processPrevious != gobj->process_tail) {
                return false;
            }

            previous = handle;
            handle = gobj->next;
            ++visitedGObjs;
            if (++visitedInList > m_pool.activeCount()) {
                return false;
            }
        }

        if (previous != m_tails[listId]) {
            return false;
        }
    }
    return visitedGObjs == m_pool.activeCount() &&
           visitedProcesses == m_processPool.activeCount();
}

} // namespace ico::engine
