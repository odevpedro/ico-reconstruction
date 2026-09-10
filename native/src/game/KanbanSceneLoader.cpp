#include "game/KanbanSceneLoader.h"

#include "engine/GifPacket.h"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <utility>

bool KanbanSceneLoader::initialize(IsysGObj& runtime) {
    m_runtime = &runtime;
    m_initialized = true;
    m_currentSceneId = 0;
    m_requests.clear();

    m_sceneGObjs.clear();
    m_sceneGObjSources.clear();
    for (std::size_t i = 0; i < m_descriptors.size(); ++i) {
        m_descriptors[i].descriptorIndex = static_cast<u16>(i);
        m_descriptors[i].listId = 0;
        m_descriptors[i].hasInitFn = false;
        m_descriptors[i].initFn = {};
        m_descriptors[i].processCallback_40 = 0;
        m_descriptors[i].gate_44 = 1;
    }

    for (auto& record : m_entries) {
        record.sceneId = 0;
        record.descriptorIndex = kInvalidSceneDescriptorIndex;
        record.enabled = false;
        record.sortKey = 0;
        record.userData = 0;
        record.processCallback_24 = 0;
        record.processArgument_40 = 0;
        record.listId = kSceneEntryListIdUnknown;
        record.gobjType = 0;
        record.flag_44 = 0;
    }

    return true;
}

void KanbanSceneLoader::shutdown() {
    m_requests.clear();
    m_runtime = nullptr;
    m_initialized = false;
    m_sceneGObjs.clear();
    m_sceneGObjSources.clear();
    m_attachments.clear();
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

bool KanbanSceneLoader::applyVerifiedSceneTables(
    const ico::engine::VerifiedSceneDescriptor* descriptors,
    std::size_t descriptorCount,
    const ico::engine::VerifiedSceneEntry* payload,
    std::size_t payloadCount,
    const ico::engine::VerifiedSceneRange* ranges,
    std::size_t rangeCount) {
    if (!m_initialized || (descriptors == nullptr && descriptorCount != 0) ||
        (payload == nullptr && payloadCount != 0) ||
        (ranges == nullptr && rangeCount != 0)) {
        return false;
    }
    for (std::size_t i = 0; i < descriptorCount; ++i) {
        if (descriptors[i].descriptorIndex >= m_descriptors.size()) {
            return false;
        }
    }

    // Validate scene ranges: scene ids must be distinct and entry spans must
    // not wrap over the verified table size.
    for (std::size_t i = 0; i < rangeCount; ++i) {
        const ico::engine::VerifiedSceneRange& range = ranges[i];
        if (range.endEntry <= range.startEntry ||
            range.startEntry >= m_entries.size() ||
            range.endEntry > m_entries.size()) {
            return false;
        }
        for (std::size_t j = 0; j < i; ++j) {
            if (ranges[j].sceneId == range.sceneId) {
                return false;
            }
        }
    }

    // Descriptor process callbacks feed the registration fallback path.
    for (std::size_t i = 0; i < descriptorCount; ++i) {
        const u16 index = descriptors[i].descriptorIndex;
        m_descriptors[index].processCallback_40 = descriptors[i].processCallback_40;
        m_descriptors[index].gate_44 = descriptors[i].gate_44;
        m_descriptors[index].listId = 0;
        m_descriptors[index].hasInitFn = false;
        m_descriptors[index].initFn = {};
    }

    // Enable every payload entry against its scene's range with raw fields.
    for (std::size_t i = 0; i < payloadCount; ++i) {
        const ico::engine::VerifiedSceneEntry& verified = payload[i];
        const ico::engine::VerifiedSceneRange* range = nullptr;
        for (std::size_t j = 0; j < rangeCount; ++j) {
            if (verified.entryIndex >= ranges[j].startEntry &&
                verified.entryIndex < ranges[j].endEntry) {
                range = &ranges[j];
                break;
            }
        }
        if (range == nullptr || verified.descriptorIndex >= m_descriptors.size() ||
            verified.entryIndex >= m_entries.size()) {
            return false;
        }
        SceneEntryRecord& record = m_entries[verified.entryIndex];
        record.sceneId = range->sceneId;
        record.descriptorIndex = verified.descriptorIndex;
        record.enabled = true;
        record.sortKey = 0;
        record.userData = verified.userData_30;
        record.processCallback_24 = verified.processCallback_24;
        record.processArgument_40 = verified.processArgument_40;
        record.listId = verified.listId;
        record.gobjType = verified.gobjType & 0x1Fu;
        record.flag_44 = verified.flag_44;
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

bool KanbanSceneLoader::bindSceneAssets(const ico::engine::SceneAssetStore& store,
                                        u32 sceneId) {
    if (!m_initialized || !store.isInitialized()) {
        return false;
    }

    const std::size_t count = store.sceneAssetCount(sceneId);
    if (count == 0 && !store.hasScene(sceneId)) {
        return false;
    }

    for (SceneAssetBinding& binding : m_assetBindings) {
        if (binding.sceneId == sceneId) {
            binding.assets.clear();
            for (std::size_t i = 0; i < count; ++i) {
                const ico::engine::SceneAssetEntry* entry = store.sceneAsset(sceneId, i);
                if (entry != nullptr) {
                    binding.assets.push_back(*entry);
                }
            }
            return true;
        }
    }

    SceneAssetBinding binding;
    binding.sceneId = sceneId;
    for (std::size_t i = 0; i < count; ++i) {
        const ico::engine::SceneAssetEntry* entry = store.sceneAsset(sceneId, i);
        if (entry != nullptr) {
            binding.assets.push_back(*entry);
        }
    }
    m_assetBindings.push_back(std::move(binding));
    return true;
}

bool KanbanSceneLoader::hasBoundAssets(u32 sceneId) const {
    for (const SceneAssetBinding& binding : m_assetBindings) {
        if (binding.sceneId == sceneId) {
            return !binding.assets.empty();
        }
    }
    return false;
}

std::size_t KanbanSceneLoader::boundAssetCount(u32 sceneId) const {
    for (const SceneAssetBinding& binding : m_assetBindings) {
        if (binding.sceneId == sceneId) {
            return binding.assets.size();
        }
    }
    return 0;
}

const ico::engine::SceneAssetEntry* KanbanSceneLoader::boundAsset(
    u32 sceneId, std::size_t index) const {
    for (const SceneAssetBinding& binding : m_assetBindings) {
        if (binding.sceneId == sceneId) {
            return (index < binding.assets.size()) ? &binding.assets[index] : nullptr;
        }
    }
    return nullptr;
}

std::size_t KanbanSceneLoader::attachBoundAssetsToGObjs(u32 sceneId) {
    if (!m_initialized || m_runtime == nullptr || m_sceneGObjs.empty()) {
        return 0;
    }

    const SceneAssetBinding* binding = nullptr;
    for (const SceneAssetBinding& candidate : m_assetBindings) {
        if (candidate.sceneId == sceneId) {
            binding = &candidate;
            break;
        }
    }
    if (binding == nullptr || binding->assets.empty()) {
        return 0;
    }

    // Rev.159: when a verified room role plan exists for this room, tag each
    // scene GObj with a handler from the runtime repertoire and pair the
    // bound assets round-robin onto the tagged GObjs (asset label -> verified
    // handler attachment is NOT byte-verified; the per-room multiset is).
    const RoomRolePlan* plan = nullptr;
    for (const RoomRolePlan& candidate : m_roomRolePlans) {
        if (candidate.sceneId == sceneId) {
            plan = &candidate;
            break;
        }
    }
    m_gobjHandlers.clear();
    if (plan != nullptr && !plan->roles.empty()) {
        // Build the expanded role-slot iterator: each role contributes
        // roleCount slots (one per distinct runtime GObj). Walk the scene
        // GObjs in creation order and tag each to the next slot.
        std::vector<ico_ptr32> slots;
        for (const ico::engine::VerifiedRoomRole& role : plan->roles) {
            for (u16 i = 0; i < role.roleCount; ++i) {
                slots.push_back(role.handlerAddr);
            }
        }
        if (!slots.empty()) {
            std::size_t slotIndex = 0;
            for (const ico::engine::GObjHandle handle : m_sceneGObjs) {
                m_gobjHandlers.emplace_back(handle,
                                            slots[slotIndex++ % slots.size()]);
            }
        }

        std::size_t attached = 0;
        for (const ico::engine::SceneAssetEntry& asset : binding->assets) {
            // Round-robin over the role-tagged GObjs (host heuristic; the
            // asset label -> handler association is NOT verified).
            const ico::engine::GObjHandle owner =
                m_gobjHandlers[attached % m_gobjHandlers.size()].first;
            ico::engine::GObjRenderAttachment att{};
            att.handle = owner;
            att.kind = ico::engine::GObjAttachmentKind::Mesh;
            att.meshPath = asset.meshPath;
            att.meshLabel = asset.label;
            att.transform = ico::engine::Matrix4x4::identity();
            att.active = true;
            m_attachments.attach(att);
            ++attached;
        }
        return attached;
    }

    // Best-effort HOST pairing: round-robin the scene's allowed payload over
    // the GObjs the verified entry table actually created. Not a verified
    // original GObj<->model link (see header comment).
    if (binding->assets.size() > m_sceneGObjs.size()) {
        std::fprintf(stderr,
                     "loader: note: scene 0x%02X has %zu assets but only %zu "
                     "GObjs; round-robin pairing distributes them\n",
                     sceneId, binding->assets.size(), m_sceneGObjs.size());
    }
    const std::size_t bound = binding->assets.size();
    for (std::size_t i = 0; i < bound; ++i) {
        const ico::engine::SceneAssetEntry& asset = binding->assets[i];
        const ico::engine::GObjHandle owner =
            m_sceneGObjs[i % m_sceneGObjs.size()];
        ico::engine::GObjRenderAttachment att{};
        att.handle = owner;
        att.kind = ico::engine::GObjAttachmentKind::Mesh;
        att.meshPath = asset.meshPath;
        att.meshLabel = asset.label;
        att.transform = ico::engine::Matrix4x4::identity();
        att.active = true;
        m_attachments.attach(att);
    }
    return bound;
}

bool KanbanSceneLoader::applyVerifiedRoomRolePlans(
    const ico::engine::VerifiedRoomRolePlan* plans, std::size_t planCount) {
    if (!m_initialized || (plans == nullptr && planCount != 0)) {
        return false;
    }
    m_roomRolePlans.clear();
    for (std::size_t i = 0; i < planCount; ++i) {
        if (plans[i].roles == nullptr && plans[i].roleCount != 0) {
            return false;
        }
        RoomRolePlan plan;
        plan.sceneId = plans[i].sceneId;
        for (u16 r = 0; r < plans[i].roleCount; ++r) {
            plan.roles.push_back(plans[i].roles[r]);
        }
        m_roomRolePlans.push_back(std::move(plan));
    }
    return true;
}

bool KanbanSceneLoader::hasRoomRolePlan(u32 sceneId) const {
    for (const RoomRolePlan& plan : m_roomRolePlans) {
        if (plan.sceneId == sceneId) {
            return true;
        }
    }
    return false;
}

std::size_t KanbanSceneLoader::relinkAttachmentsForCurrentScene() {
    m_attachments.clear();
    const u32 sceneId = m_currentSceneId;
    if (!hasBoundAssets(sceneId)) {
        return 0;
    }
    return attachBoundAssetsToGObjs(sceneId);
}

const ico::engine::GObjAttachmentStore& KanbanSceneLoader::attachmentStore() const {
    return m_attachments;
}

ico::engine::GObjAttachmentStore& KanbanSceneLoader::attachmentStore() {
    return m_attachments;
}

ico_ptr32 KanbanSceneLoader::gobjHandlerRole(u32 index) const {
    if (index >= m_gobjHandlers.size()) {
        return 0;
    }
    return m_gobjHandlers[index].second;
}

bool KanbanSceneLoader::execute() {
    if (!m_initialized || m_runtime == nullptr || m_requests.empty()) {
        return false;
    }

    const u32 sceneId = m_requests.front();
    m_requests.erase(m_requests.begin());
    m_currentSceneId = sceneId;
    const bool ok = initSceneGObj(sceneId) != 0;
    if (ok) {
        // Rev.159 (Passo 2): re-link every room transition, not only the
        // first load. New world-state loads pick the room's verified handler
        // repertoire and re-pair assets onto the (reallocated) GObjs.
        relinkAttachmentsForCurrentScene();
    }
    return ok;
}

std::size_t KanbanSceneLoader::initSceneGObj(u32 sceneId) {
    if (!m_initialized || m_runtime == nullptr || !m_runtime->isInitialized()) {
        return 0;
    }

    std::size_t created = 0;
    // Rev.159: the GObj pool is reused across rooms (Rev.158 runtime capture:
    // a pool address changes owner between scenes). Release the previous
    // scene's GObjs so a new room starts from free slots and the active-count
    // invariant holds per scene instead of accumulating.
    for (const ico::engine::GObjHandle handle : m_sceneGObjs) {
        ico::engine::GObj* gobj = m_runtime->pool().get(handle);
        if (gobj != nullptr) {
            m_runtime->remove(*gobj);
        }
    }
    m_sceneGObjs.clear();
    m_sceneGObjSources.clear();
    m_attachments.clear();
    for (const SceneEntryRecord& record : m_entries) {
        if (!record.enabled || record.sceneId != sceneId ||
            record.descriptorIndex >= m_descriptors.size()) {
            continue;
        }

        const SceneGObjDescriptor& desc = m_descriptors[record.descriptorIndex];
        // Rev.154: descriptor+0x44 gate; 0 skips GObj creation entirely.
        if (desc.gate_44 == 0) {
            continue;
        }
        const u8 listId = (record.listId != kSceneEntryListIdUnknown)
                              ? record.listId
                              : desc.listId;
        ico::engine::GObj* gobj =
            m_runtime->add(listId, record.sortKey, record.userData);
        if (gobj == nullptr) {
            continue;
        }
        // Rev.154: entry+0x47 low 5 bits drive the GObj kind selector (0x115108).
        gobj->type = record.gobjType;

        const ico::engine::GObjHandle handle = m_runtime->pool().handleOf(*gobj);
        m_sceneGObjs.push_back(handle);
        m_sceneGObjSources.push_back({handle, record.descriptorIndex, sceneId});
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

ico::engine::GObjHandle KanbanSceneLoader::sceneGObjHandle(std::size_t index) const {
    return (index < m_sceneGObjs.size()) ? m_sceneGObjs[index]
                                         : ico::engine::kNullGObjHandle;
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

std::vector<StaticSceneDebugItem> KanbanSceneLoader::staticSceneDebugItems() const {
    std::vector<StaticSceneDebugItem> items;
    if (!m_initialized || m_runtime == nullptr) {
        return items;
    }

    for (u8 listId = 0; listId < ico::engine::kPrimaryListCount; ++listId) {
        const ico::engine::GObj* gobj = m_runtime->head(listId);
        while (gobj != nullptr) {
            const ico::engine::GObjHandle handle = m_runtime->pool().handleOf(*gobj);
            const auto source = std::find_if(
                m_sceneGObjSources.begin(), m_sceneGObjSources.end(),
                [handle](const SceneGObjSource& candidate) {
                    return candidate.handle == handle;
                });
            if (source != m_sceneGObjSources.end()) {
                StaticSceneDebugItem item{};
                item.sceneId = source->sceneId;
                item.descriptorIndex = source->descriptorIndex;
                item.gobjType = gobj->type;
                item.listId = gobj->list_id;
                item.sortKey = gobj->sort_key;
                item.handle = handle;
                char label[128]{};
                std::snprintf(label, sizeof(label),
                              "scene=%u descriptor=%u gobj.type=%u list=%u sort=%u handle=%u",
                              item.sceneId, item.descriptorIndex, item.gobjType,
                              item.listId, item.sortKey, item.handle);
                item.label = label;
                items.push_back(std::move(item));
            }
            gobj = m_runtime->pool().get(gobj->next);
        }
    }
    return items;
}

std::size_t KanbanSceneLoader::renderStaticSceneDebugView(
    ico::engine::GifPacketBridge& bridge,
    const StaticSceneDebugViewStyle& style,
    const StaticSceneDebugLabelSink& labelSink) const {
    if (!bridge.checkOpen() || style.columns == 0 || style.cellWidth <= 0.0f ||
        style.cellHeight <= 0.0f || style.listGap < 0.0f) {
        return 0;
    }

    const std::vector<StaticSceneDebugItem> items = staticSceneDebugItems();
    std::array<std::size_t, ico::engine::kPrimaryListCount> listCounts{};
    for (const StaticSceneDebugItem& item : items) {
        const std::size_t indexInList = listCounts[item.listId]++;
        const float x = style.originX + static_cast<float>(indexInList % style.columns) *
            style.cellWidth;
        const float y = style.originY + static_cast<float>(item.listId) *
            (style.cellHeight + style.listGap) +
            static_cast<float>(indexInList / style.columns) * style.cellHeight;
        bridge.makeSpriteNoTexture(x, y, style.cellWidth, style.cellHeight);
        if (labelSink) {
            labelSink(item, x, y);
        }
    }
    return items.size();
}
