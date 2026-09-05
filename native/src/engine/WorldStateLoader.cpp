#include "engine/WorldStateLoader.h"

namespace ico::engine {

bool WorldStateLoader::initialize(WorldStateDescriptor* table, std::size_t count,
                                  CollisionListFn collisionListFn,
                                  SceneApplyFn sceneApplyFn) {
    if (table == nullptr || count == 0 || collisionListFn == nullptr ||
        sceneApplyFn == nullptr) {
        return false;
    }
    m_table = table;
    m_count = count;
    m_collisionListFn = collisionListFn;
    m_sceneApplyFn = sceneApplyFn;
    /* The original writes the two zero words unconditionally at
       0x274ED4/+0x18; the host mirrors that as a deterministic clear. */
    flagBytes[0] = 0;
    flagBytes[4] = 0;
    m_initialized = true;
    return true;
}

void WorldStateLoader::shutdown() {
    m_table = nullptr;
    m_count = 0;
    m_collisionListFn = nullptr;
    m_sceneApplyFn = nullptr;
    m_initialized = false;
}

bool WorldStateLoader::isInitialized() const {
    return m_initialized;
}

u32 WorldStateLoader::loadWorldState(u32 worldState) {
    if (!m_initialized) {
        return 0;
    }

    /* Original: entry = 0x5F2FB8 + state * 0x194; initFn = entry->initFn. */
    u32 result = 0;
    if (worldState < m_count && m_table != nullptr) {
        const WorldStateDescriptor& desc = m_table[worldState];
        if (desc.hasInitFn && desc.initFn != nullptr) {
            result = 1u;
            desc.initFn();
        }
    }

    if (m_collisionListFn != nullptr) {
        m_collisionListFn();
    }
    if (m_sceneApplyFn != nullptr) {
        m_sceneApplyFn();
    }

    /* Mirrors sw $0,0x14($v0); sw $0,0x18($v0) at 0x274EC0. */
    flagBytes[0] = 0;
    flagBytes[4] = 0;
    return result;
}

}  // namespace ico::engine