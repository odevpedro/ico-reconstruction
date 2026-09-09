#include "engine/GObjAttachment.h"

namespace ico::engine {

namespace {
const GObjRenderAttachment* findRecord(
    const std::vector<GObjRenderAttachment>& records, GObjHandle handle,
    const std::string* meshPath) {
    for (const auto& att : records) {
        if (att.handle == handle &&
            (meshPath == nullptr || att.meshPath == *meshPath)) {
            return &att;
        }
    }
    return nullptr;
}
}  // namespace

void GObjAttachmentStore::attach(const GObjRenderAttachment& att) {
    for (auto& existing : m_attachments) {
        if (existing.handle == att.handle && existing.meshPath == att.meshPath) {
            existing = att;
            return;
        }
    }
    m_attachments.push_back(att);
}

void GObjAttachmentStore::detach(GObjHandle handle) {
    std::vector<GObjRenderAttachment> kept;
    kept.reserve(m_attachments.size());
    for (const auto& att : m_attachments) {
        if (att.handle != handle) {
            kept.push_back(att);
        }
    }
    m_attachments.swap(kept);
}

void GObjAttachmentStore::detachMesh(GObjHandle handle,
                                     const std::string& meshPath) {
    std::vector<GObjRenderAttachment> kept;
    kept.reserve(m_attachments.size());
    for (const auto& att : m_attachments) {
        if (!(att.handle == handle && att.meshPath == meshPath)) {
            kept.push_back(att);
        }
    }
    m_attachments.swap(kept);
}

void GObjAttachmentStore::clear() {
    m_attachments.clear();
}

const GObjRenderAttachment* GObjAttachmentStore::find(GObjHandle handle) const {
    return findRecord(m_attachments, handle, nullptr);
}

GObjRenderAttachment* GObjAttachmentStore::find(GObjHandle handle) {
    for (auto& att : m_attachments) {
        if (att.handle == handle) {
            return &att;
        }
    }
    return nullptr;
}

std::size_t GObjAttachmentStore::countFor(GObjHandle handle) const {
    std::size_t n = 0;
    for (const auto& att : m_attachments) {
        if (att.handle == handle) {
            ++n;
        }
    }
    return n;
}

}  // namespace ico::engine