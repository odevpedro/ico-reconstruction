#pragma once

#include "engine/GObj.h"
#include "engine/RenderBackend.h"
#include "ps2/Ps2Types.h"

#include <cstddef>
#include <string>
#include <vector>

namespace ico::engine {

/*
 * Host-side per-GObj visual composition (Rev.155 / Passo 1).
 *
 * The PS2 GObj ABI is byte-exact and fixed-width (IcoGObj, 0x174), so the
 * native port must NOT grow it. The visual payload a GObj "owns" (mesh path,
 * per-material texture names, world transform) lives in a side registry keyed
 * by the GObj's handle. The renderer iterates the active isysGObj lists and,
 * for each GObj, resolves its attachment(s) and draws what it commands —
 * instead of reading the SceneAssetStore directly.
 *
 * A single GObj may own several meshes (e.g. a scene object whose model is
 * split into pieces): the store keeps one record per (handle, mesh) pair.
 * The GObj → asset pairing produced by
 * KanbanSceneLoader::attachBoundAssetsToGObjs() is a round-robin HOST mapping
 * over the scene GObjs, pending a real runtime (PCSX2) capture that binds each
 * GObj to its model. It is not a byte-verified reconstruction.
 */
enum class GObjAttachmentKind : u8 {
    Mesh = 0,        // textured strips from a .p2o mesh (material names → TM2)
    BoxMarker = 1,   // host placeholder box (boy avatar), transform-driven
};

struct GObjRenderAttachment {
    GObjHandle handle = kNullGObjHandle;
    GObjAttachmentKind kind = GObjAttachmentKind::Mesh;
    std::string meshPath;                 // .p2o source this GObj owns
    std::string meshLabel;                // asset label (debug)
    std::vector<std::string> materialNames; // TM2 base names, per material f
    Matrix4x4 transform;                  // world placement (identity default)
    bool active = true;
    /* BoxMarker parameters (kind==BoxMarker). */
    float halfExtent = 20.0f;
    u8 boxColor[4] = { 255, 100, 100, 255 };
};

class GObjAttachmentStore {
public:
    /* Adds one record; replaces the record with the same handle+meshPath. */
    void attach(const GObjRenderAttachment& att);
    /* Removes every record owned by handle. */
    void detach(GObjHandle handle);
    /* Removes the record for handle+meshPath if present. */
    void detachMesh(GObjHandle handle, const std::string& meshPath);
    void clear();

    /* First record owned by handle, or nullptr when the GObj owns none. */
    const GObjRenderAttachment* find(GObjHandle handle) const;
    GObjRenderAttachment* find(GObjHandle handle);
    std::size_t count() const { return m_attachments.size(); }

    /* Total records owned by handle. */
    std::size_t countFor(GObjHandle handle) const;

    /* Iterates every registered record (owner GObj may be inactive). */
    template <typename F>
    void forEach(F&& fn) const {
        for (const auto& att : m_attachments) fn(att);
    }

private:
    std::vector<GObjRenderAttachment> m_attachments;
};

}  // namespace ico::engine