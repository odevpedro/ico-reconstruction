#include "game/KanbanSceneLoader.h"

#include "engine/GifPacket.h"

#include <algorithm>
#include <cassert>

bool KanbanSceneLoader::initialize(IsysGObj& runtime) {
    m_runtime = &runtime;
    m_initialized = true;
    m_currentSceneId = 0;
    m_requests.clear();

    m_sceneGObjs.clear();
    for (std::size_t i = 0; i < m_descriptors.size(); ++i) {
        m_descriptors[i].descriptorIndex = static_cast<u16>(i);
        m_descriptors[i].listId = 0;
        m_descriptors[i].hasInitFn = false;
        m_descriptors[i].initFn = {};
        m_descriptors[i].processCallback_40 = 0;
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
    m_sceneGObjs.clear();
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

bool KanbanSceneLoader::applyVerifiedDescriptorRecords(
    const VerifiedSceneDescriptorRecord* records,
    std::size_t count) {
    if (!m_initialized || (records == nullptr && count != 0)) {
        return false;
    }

    for (std::size_t i = 0; i < count; ++i) {
        if (records[i].descriptorIndex >= m_descriptors.size()) {
            return false;
        }
    }
    for (std::size_t i = 0; i < count; ++i) {
        m_descriptors[records[i].descriptorIndex].processCallback_40 =
            records[i].processCallback_40;
    }
    return true;
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
    m_sceneGObjs.clear();
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

        m_sceneGObjs.push_back(m_runtime->pool().handleOf(*gobj));
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

std::size_t KanbanSceneLoader::renderSyntheticScene(
    ico::engine::GifPacketBridge& bridge,
    const SyntheticSceneRenderStyle& style) const {
    if (!m_initialized || m_runtime == nullptr || !bridge.checkOpen() ||
        style.columns == 0 || style.cellWidth <= 0.0f || style.cellHeight <= 0.0f) {
        return 0;
    }

    std::size_t emitted = 0;
    for (u8 listId = 0; listId < ico::engine::kPrimaryListCount; ++listId) {
        ico::engine::GObj* gobj = m_runtime->head(listId);
        while (gobj != nullptr) {
            const ico::engine::GObjHandle handle = m_runtime->pool().handleOf(*gobj);
            if (std::find(m_sceneGObjs.begin(), m_sceneGObjs.end(), handle) !=
                m_sceneGObjs.end()) {
                const std::size_t column = emitted % style.columns;
                const std::size_t row = emitted / style.columns;
                bridge.makeSpriteNoTexture(
                    style.originX + static_cast<float>(column) * style.cellWidth,
                    style.originY + static_cast<float>(row) * style.cellHeight,
                    style.cellWidth,
                    style.cellHeight);
                ++emitted;
            }
            gobj = m_runtime->pool().get(gobj->next);
        }
    }
    return emitted;
}
