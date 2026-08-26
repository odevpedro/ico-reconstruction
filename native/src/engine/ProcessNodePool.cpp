#include "engine/ProcessNodePool.h"

#include <functional>
#include <limits>

namespace ico::engine {

ProcessNodePool::ProcessNodePool(std::size_t capacity)
{
    initialize(capacity);
}

void ProcessNodePool::initialize(std::size_t capacity)
{
    const auto maxCapacity = static_cast<std::size_t>(
        std::numeric_limits<ProcessHandle>::max() - 1u);
    if (capacity > maxCapacity) {
        capacity = maxCapacity;
    }

    m_slots.resize(capacity);
    clear();
}

void ProcessNodePool::clear()
{
    for (ProcessNode& process : m_slots) {
        resetProcessNodeSlot(process);
    }
    m_activeCount = 0;
}

ProcessNode* ProcessNodePool::acquire(GObjHandle parent,
                                      u32 typeMask,
                                      u32 priority)
{
    for (std::size_t i = 0; i < m_slots.size(); ++i) {
        ProcessNode& process = m_slots[i];
        if (!isProcessNodeSlotFree(process)) {
            continue;
        }

        resetProcessNodeSlot(process);
        process.self = static_cast<ProcessHandle>(i + 1u);
        process.parent = parent;
        process.type_mask = typeMask;
        process.priority = priority;
        process.active = 1;
        ++m_activeCount;
        return &process;
    }
    return nullptr;
}

bool ProcessNodePool::release(ProcessNode& process)
{
    if (!owns(process) || isProcessNodeSlotFree(process)) {
        return false;
    }
    resetProcessNodeSlot(process);
    --m_activeCount;
    return true;
}

ProcessNode* ProcessNodePool::get(ProcessHandle handle)
{
    if (handle == kNullProcessHandle || handle > m_slots.size()) {
        return nullptr;
    }
    return &m_slots[handle - 1u];
}

const ProcessNode* ProcessNodePool::get(ProcessHandle handle) const
{
    if (handle == kNullProcessHandle || handle > m_slots.size()) {
        return nullptr;
    }
    return &m_slots[handle - 1u];
}

ProcessHandle ProcessNodePool::handleOf(const ProcessNode& process) const
{
    if (!owns(process)) {
        return kNullProcessHandle;
    }
    return static_cast<ProcessHandle>(&process - m_slots.data()) + 1u;
}

bool ProcessNodePool::owns(const ProcessNode& process) const
{
    if (m_slots.empty()) {
        return false;
    }
    const ProcessNode* begin = m_slots.data();
    const ProcessNode* end = begin + m_slots.size();
    const std::less<const ProcessNode*> less;
    return !less(&process, begin) && less(&process, end);
}

std::size_t ProcessNodePool::capacity() const
{
    return m_slots.size();
}

std::size_t ProcessNodePool::activeCount() const
{
    return m_activeCount;
}

bool ProcessNodePool::empty() const
{
    return m_activeCount == 0;
}

bool ProcessNodePool::full() const
{
    return m_activeCount == m_slots.size();
}

} // namespace ico::engine
