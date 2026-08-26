#pragma once

#include "engine/GObj.h"

#include <cstddef>
#include <vector>

namespace ico::engine {

class ProcessNodePool {
public:
    explicit ProcessNodePool(std::size_t capacity = 0);

    void initialize(std::size_t capacity);
    void clear();

    ProcessNode* acquire(GObjHandle parent,
                         u32 typeMask,
                         u32 priority);
    bool release(ProcessNode& process);

    ProcessNode* get(ProcessHandle handle);
    const ProcessNode* get(ProcessHandle handle) const;
    ProcessHandle handleOf(const ProcessNode& process) const;
    bool owns(const ProcessNode& process) const;

    std::size_t capacity() const;
    std::size_t activeCount() const;
    bool empty() const;
    bool full() const;

private:
    std::vector<ProcessNode> m_slots;
    std::size_t m_activeCount = 0;
};

} // namespace ico::engine
