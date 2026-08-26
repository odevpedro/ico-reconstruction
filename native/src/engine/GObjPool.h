#pragma once

#include "engine/GObj.h"

#include <cstddef>
#include <vector>

namespace ico::engine {

class GObjPool {
public:
    explicit GObjPool(std::size_t capacity = 0);

    void initialize(std::size_t capacity);
    void clear();

    GObj* acquire();
    bool release(GObj& gobj);

    GObj* get(GObjHandle handle);
    const GObj* get(GObjHandle handle) const;
    GObjHandle handleOf(const GObj& gobj) const;
    bool owns(const GObj& gobj) const;

    std::size_t capacity() const;
    std::size_t activeCount() const;
    bool empty() const;
    bool full() const;

private:
    std::vector<GObj> m_slots;
    std::size_t m_activeCount = 0;
};

} // namespace ico::engine
