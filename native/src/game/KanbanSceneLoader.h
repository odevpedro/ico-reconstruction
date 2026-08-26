#pragma once

#include "game/IsysGObj.h"

#include <array>
#include <cstddef>
#include <functional>
#include <vector>

constexpr std::size_t kSceneDescriptorCount = 68;
constexpr std::size_t kSceneEntryCount = 512;
constexpr u16 kInvalidSceneDescriptorIndex = 0xFFFFu;

struct SceneGObjDescriptor {
    using InitFn = std::function<void(ico::engine::GObj&, const SceneGObjDescriptor&)>;

    u16 descriptorIndex = 0;
    u8 listId = 0;
    bool hasInitFn = false;
    InitFn initFn{};
};

struct SceneEntryRecord {
    u32 sceneId = 0;
    u16 descriptorIndex = kInvalidSceneDescriptorIndex;
    bool enabled = false;
    u32 sortKey = 0;
    ico_ptr32 userData = 0;
};

class KanbanSceneLoader {
public:
    bool initialize(IsysGObj& runtime);
    void shutdown();
    bool isInitialized() const;

    SceneGObjDescriptor* descriptor(std::size_t index);
    const SceneGObjDescriptor* descriptor(std::size_t index) const;
    SceneEntryRecord* entry(std::size_t index);
    const SceneEntryRecord* entry(std::size_t index) const;

    bool requestScene(u32 sceneId);
    void clearRequests();
    std::size_t pendingRequestCount() const;

    bool execute();
    std::size_t initSceneGObj(u32 sceneId);
    std::size_t hotInitSceneObjects(u32 sceneId) const;

    u32 currentSceneId() const;

private:
    IsysGObj* m_runtime = nullptr;
    bool m_initialized = false;
    u32 m_currentSceneId = 0;
    std::array<SceneGObjDescriptor, kSceneDescriptorCount> m_descriptors{};
    std::array<SceneEntryRecord, kSceneEntryCount> m_entries{};
    std::vector<u32> m_requests;
};
