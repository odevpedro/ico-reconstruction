#pragma once

#include "game/IsysGObj.h"

#include <array>
#include <cstddef>
#include <functional>
#include <string>
#include <vector>
namespace ico::engine {
class GifPacketBridge;
}


constexpr std::size_t kSceneDescriptorCount = 68;
constexpr std::size_t kSceneEntryCount = 512;
constexpr u16 kInvalidSceneDescriptorIndex = 0xFFFFu;

struct SceneGObjDescriptor {
    using InitFn = std::function<void(ico::engine::GObj&, const SceneGObjDescriptor&)>;

    u16 descriptorIndex = 0;
    u8 listId = 0;
    bool hasInitFn = false;
    InitFn initFn{};
    /* Raw original descriptor field at +0x40; its source-level name is unknown. */
    ico_ptr32 processCallback_40 = 0;
};

struct SceneEntryRecord {
    u32 sceneId = 0;
    u16 descriptorIndex = kInvalidSceneDescriptorIndex;
    bool enabled = false;
    u32 sortKey = 0;
    ico_ptr32 userData = 0;
    /* Raw original entry fields used by initSceneGObj's registration gate. */
    ico_ptr32 processCallback_24 = 0;
    u16 processArgument_40 = 0;
};

struct SceneProcessRegistrationSpec {
    ico_ptr32 callback = 0;
    u32 wrapperT0 = 0x1800;
    bool usesEntryOverride = false;
};

/*
 * Verified static descriptor datum exported from the USA ELF. This is only
 * the raw descriptor +0x40 field consumed by selectProcessRegistration(); it
 * is neither a room assignment nor a decoded model/resource record.
 */
struct VerifiedSceneDescriptorRecord {
    u16 descriptorIndex = kInvalidSceneDescriptorIndex;
    ico_ptr32 processCallback_40 = 0;
};
/*
 * Host-only layout for exercising the scene → GIF path. It is not extracted
 * ICO geometry, materials, or a claim about original visual placement.
 */
struct SyntheticSceneRenderStyle {
    float originX = 0.0f;
    float originY = 0.0f;
    float cellWidth = 16.0f;
    float cellHeight = 16.0f;
    u32 columns = 16;
};

/*
 * Host debug metadata for the GObjs created by the currently selected scene.
 * gobjType is the raw verified GObj+0x0C value, not a recovered game-facing
 * entity classification. label is supplied to a host overlay; the GIF bridge
 * intentionally has no text rasterizer yet.
 */
struct StaticSceneDebugItem {
    u32 sceneId = 0;
    u16 descriptorIndex = kInvalidSceneDescriptorIndex;
    u32 gobjType = 0;
    u8 listId = 0;
    u32 sortKey = 0;
    ico::engine::GObjHandle handle = ico::engine::kNullGObjHandle;
    std::string label;
};

struct StaticSceneDebugViewStyle {
    float originX = 0.0f;
    float originY = 0.0f;
    float cellWidth = 16.0f;
    float cellHeight = 16.0f;
    u32 columns = 16;
    float listGap = 4.0f;
};

using StaticSceneDebugLabelSink = std::function<void(const StaticSceneDebugItem&,
                                                      float x,
                                                      float y)>;


class KanbanSceneLoader {
public:
    bool initialize(IsysGObj& runtime);
    void shutdown();
    bool isInitialized() const;

    SceneGObjDescriptor* descriptor(std::size_t index);
    const SceneGObjDescriptor* descriptor(std::size_t index) const;
    SceneEntryRecord* entry(std::size_t index);
    const SceneEntryRecord* entry(std::size_t index) const;

    /* Applies a bounded set of verified raw descriptor fields. */
    bool applyVerifiedDescriptorRecords(const VerifiedSceneDescriptorRecord* records,
                                        std::size_t count);

    bool requestScene(u32 sceneId);
    void clearRequests();
    std::size_t pendingRequestCount() const;

    bool execute();
    std::size_t initSceneGObj(u32 sceneId);
    std::size_t hotInitSceneObjects(u32 sceneId) const;
    /* Emits one untextured host-test sprite per GObj created for this scene.
       The caller owns the GIF packet lifecycle. */
    std::size_t renderSyntheticScene(ico::engine::GifPacketBridge& bridge,
                                     const SyntheticSceneRenderStyle& style = {}) const;
    /* Structural debug view: placeholders use GIF, labels go to the host sink. */
    std::vector<StaticSceneDebugItem> staticSceneDebugItems() const;
    std::size_t renderStaticSceneDebugView(
        ico::engine::GifPacketBridge& bridge,
        const StaticSceneDebugViewStyle& style = {},
        const StaticSceneDebugLabelSink& labelSink = {}) const;

    /* Returns raw PS2 values; it intentionally does not invoke host callbacks. */
    static SceneProcessRegistrationSpec selectProcessRegistration(
        const SceneEntryRecord& record, const SceneGObjDescriptor& descriptor);

    u32 currentSceneId() const;

private:
    IsysGObj* m_runtime = nullptr;
    bool m_initialized = false;
    u32 m_currentSceneId = 0;
    std::array<SceneGObjDescriptor, kSceneDescriptorCount> m_descriptors{};
    std::array<SceneEntryRecord, kSceneEntryCount> m_entries{};
    std::vector<u32> m_requests;
    std::vector<ico::engine::GObjHandle> m_sceneGObjs;
    struct SceneGObjSource {
        ico::engine::GObjHandle handle;
        u16 descriptorIndex;
        u32 sceneId;
    };
    std::vector<SceneGObjSource> m_sceneGObjSources;
};
