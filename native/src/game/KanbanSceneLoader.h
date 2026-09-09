#pragma once

#include "engine/SceneAssetStore.h"
#include "game/GeneratedSceneTables.h"
#include "game/IsysGObj.h"

#include <array>
#include <cstddef>
#include <functional>
#include <string>
#include <vector>
namespace ico::engine {
class GifPacketBridge;
}


constexpr std::size_t kSceneDescriptorCount = ico::engine::kVerifiedSceneDescriptorCount;
/*
 * Rev.154: the USA entry table's valid descriptor-index run is contiguous
 * from idx 0 through 3590 (first invalid at 3591). AGENTS' former "512
 * entries" was an understatement; the native loader owns up to the verified
 * table size so scene 0x0F's slice (idx 847..875) is addressable.
 */
constexpr std::size_t kSceneEntryCount = 3600;
constexpr u16 kInvalidSceneDescriptorIndex = 0xFFFFu;
/* Sentinel: an entry whose list is not known falls back to the descriptor's. */
constexpr u8 kSceneEntryListIdUnknown = 0xFFu;

struct SceneGObjDescriptor {
    using InitFn = std::function<void(ico::engine::GObj&, const SceneGObjDescriptor&)>;

    u16 descriptorIndex = 0;
    u8 listId = 0;
    bool hasInitFn = false;
    InitFn initFn{};
    /* Raw original descriptor field at +0x40; its source-level name is unknown. */
    ico_ptr32 processCallback_40 = 0;
    /* Rev.154: descriptor +0x44 gate. 0 skips GObj creation entirely. */
    u32 gate_44 = 1;
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
    /* Rev.154: per-entry verified fields from the USA entry table. */
    u8 listId = kSceneEntryListIdUnknown;
    u8 gobjType = 0;
    u16 flag_44 = 0;
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

    /*
     * Applies the verified raw scene tables exported by
     * tools/extract_scene_tables.py (Rev.154):
     *   - descriptors feed processCallback_40 (+0x40 of each descriptor);
     *   - scene ranges map sceneId -> contiguous entry indices;
     *   - payload entries are per-scene slices of the entry table; each is
     *     enabled against its scene's range with the verified raw fields
     *     (listId from +0x48 bits[16:14], gobjType from +0x47 low 5, flag_44,
     *     processCallback_24, userData_30, processArgument_40).
     * No claim of byte-exactness is made for the scene->entry mapping itself
     * beyond the tiling validated from the dispatch table (0x5F2FB8).
     */
    bool applyVerifiedSceneTables(const ico::engine::VerifiedSceneDescriptor* descriptors,
                                  std::size_t descriptorCount,
                                  const ico::engine::VerifiedSceneEntry* payload,
                                  std::size_t payloadCount,
                                  const ico::engine::VerifiedSceneRange* ranges,
                                  std::size_t rangeCount);

    bool requestScene(u32 sceneId);
    void clearRequests();
    std::size_t pendingRequestCount() const;

    /*
     * Binds the host-side asset composition for a scene. This is the native
     * seam where the semantic loader acquires the real room geometry: after
     * execute()/initSceneGObj() creates the GObjs for a requested scene, the
     * bound mesh list is what the renderer should draw for that scene.
     * Returns false when store has no entry for sceneId.
     */
    bool bindSceneAssets(const ico::engine::SceneAssetStore& store, u32 sceneId);
    bool hasBoundAssets(u32 sceneId) const;
    std::size_t boundAssetCount(u32 sceneId) const;
    const ico::engine::SceneAssetEntry* boundAsset(u32 sceneId,
                                                   std::size_t index) const;

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
    /* Rev.154: number of host GObjs created by the last initSceneGObj(). */
    std::size_t sceneGObjCount() const { return m_sceneGObjs.size(); }

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

    struct SceneAssetBinding {
        u32 sceneId;
        std::vector<ico::engine::SceneAssetEntry> assets;
    };
    std::vector<SceneAssetBinding> m_assetBindings;
};
