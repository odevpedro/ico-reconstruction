#include "game/KanbanSceneLoader.h"

#include <cassert>

bool KanbanSceneLoader::initialize(IsysGObj& runtime) {
    m_runtime = &runtime;
    m_initialized = true;
    m_currentSceneId = 0;
    m_requests.clear();

    for (std::size_t i = 0; i < m_descriptors.size(); ++i) {
        m_descriptors[i].descriptorIndex = static_cast<u16>(i);
        m_descriptors[i].listId = 0;
        m_descriptors[i].hasInitFn = false;
        m_descriptors[i].initFn = {};
    }

    for (auto& record : m_entries) {
        record.sceneId = 0;
        record.descriptorIndex = kInvalidSceneDescriptorIndex;
        record.enabled = false;
        record.sortKey = 0;
        record.userData = 0;
        record.processCallback_24 = 0;
        record.processArgument_40 = 0;
    }

    return true;
}

void KanbanSceneLoader::shutdown() {
    m_requests.clear();
    m_runtime = nullptr;
    m_initialized = false;
    m_currentSceneId = 0;
}

bool KanbanSceneLoader::isInitialized() const {
    return m_initialized;
}

SceneGObjDescriptor* KanbanSceneLoader::descriptor(std::size_t index) {
    if (index >= m_descriptors.size()) {
        return nullptr;
    }
    return &m_descriptors[index];
}

const SceneGObjDescriptor* KanbanSceneLoader::descriptor(std::size_t index) const {
    if (index >= m_descriptors.size()) {
        return nullptr;
    }
    return &m_descriptors[index];
}

SceneEntryRecord* KanbanSceneLoader::entry(std::size_t index) {
    if (index >= m_entries.size()) {
        return nullptr;
    }
    return &m_entries[index];
}

const SceneEntryRecord* KanbanSceneLoader::entry(std::size_t index) const {
    if (index >= m_entries.size()) {
        return nullptr;
    }
    return &m_entries[index];
}

bool KanbanSceneLoader::requestScene(u32 sceneId) {
    if (!m_initialized) {
        return false;
    }
    m_requests.push_back(sceneId);
    return true;
}

void KanbanSceneLoader::clearRequests() {
    m_requests.clear();
}

std::size_t KanbanSceneLoader::pendingRequestCount() const {
    return m_requests.size();
}

bool KanbanSceneLoader::execute() {
    if (!m_initialized || m_runtime == nullptr || m_requests.empty()) {
        return false;
    }

    const u32 sceneId = m_requests.front();
    m_requests.erase(m_requests.begin());
    m_currentSceneId = sceneId;
    return initSceneGObj(sceneId) != 0;
}

std::size_t KanbanSceneLoader::initSceneGObj(u32 sceneId) {
    if (!m_initialized || m_runtime == nullptr || !m_runtime->isInitialized()) {
        return 0;
    }

    std::size_t created = 0;
    for (const SceneEntryRecord& record : m_entries) {
        if (!record.enabled || record.sceneId != sceneId ||
            record.descriptorIndex >= m_descriptors.size()) {
            continue;
        }

        const SceneGObjDescriptor& desc = m_descriptors[record.descriptorIndex];
        ico::engine::GObj* gobj = m_runtime->add(desc.listId, record.sortKey, record.userData);
        if (gobj == nullptr) {
            continue;
        }

        ++created;
        if (desc.hasInitFn && desc.initFn) {
            desc.initFn(*gobj, desc);
        }
    }

    return created;
}

std::size_t KanbanSceneLoader::hotInitSceneObjects(u32 sceneId) const {
    if (!m_initialized) {
        return 0;
    }

    std::size_t hotCount = 0;
    for (const SceneEntryRecord& record : m_entries) {
        if (!record.enabled || record.sceneId != sceneId ||
            record.descriptorIndex >= m_descriptors.size()) {
            continue;
        }

        const SceneGObjDescriptor& desc = m_descriptors[record.descriptorIndex];
        if (desc.hasInitFn && static_cast<bool>(desc.initFn)) {
            ++hotCount;
        }
    }

    return hotCount;
}

SceneProcessRegistrationSpec KanbanSceneLoader::selectProcessRegistration(
    const SceneEntryRecord& record, const SceneGObjDescriptor& descriptor) {
    SceneProcessRegistrationSpec result;
    if (record.processArgument_40 != 0) {
        result.wrapperT0 = static_cast<u32>(record.processArgument_40) << 10u;
    }
    if (record.processCallback_24 != 0) {
        result.callback = record.processCallback_24;
        result.usesEntryOverride = true;
    } else {
        result.callback = descriptor.processCallback_40;
    }
    return result;
}

u32 KanbanSceneLoader::currentSceneId() const {
    return m_currentSceneId;
}
