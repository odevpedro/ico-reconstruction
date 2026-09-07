#pragma once

#include "ps2/Ps2Types.h"

#include <cstddef>
#include <string>
#include <vector>

namespace ico::engine {

/*
 * SceneAssetStore — host-side, data-driven composition of a kanban scene.
 *
 * This is NOT extracted game data and it does not claim to reproduce the
 * original scene loader's resource table. It is the native-port seam where a
 * scene id (the same id KanbanSceneLoader::requestScene accepts) resolves to
 * the `.p2o` geometry files that compose the room, plus the directory holding
 * the `.tm2` textures those meshes reference by material name.
 *
 * Replacing the demo's hardcoded piece list (previously baked in main.cpp's
 * kOrder) with a parsed manifest keeps scene composition in data, verified by
 * the semantic loader, and leaves the renderer as a pure consumer.
 *
 * Manifest format (line based, '#' comments):
 *   texturedir <rel>          texture directory, relative to the manifest file
 *   piecesdir  <rel>          mesh directory, relative to the manifest file
 *   scene <id>                start of a scene block (id decimal or 0x hex)
 *     <file.p2o>              one mesh file per line, relative to piecesdir
 *   end                       end of the current scene block
 *
 * Paths are resolved absolute against the directory that contains the
 * manifest, so the store is independent of the process CWD.
 */
struct SceneAssetEntry {
    std::string meshPath;   // absolute .p2o path
    std::string label;      // file basename without extension (debug only)
};

class SceneAssetStore {
public:
    /* Parses the manifest file. Returns false on I/O or format errors. */
    bool parse(const char* manifestPath);

    bool isInitialized() const;
    std::string textureDir() const { return m_textureDir; }

    bool hasScene(u32 sceneId) const;
    std::size_t sceneAssetCount(u32 sceneId) const;
    const SceneAssetEntry* sceneAsset(u32 sceneId, std::size_t index) const;
    std::vector<u32> sceneIds() const;

private:
    bool m_initialized = false;
    std::string m_textureDir;
    std::string m_piecesDir;
    struct SceneBlock {
        u32 sceneId;
        std::vector<SceneAssetEntry> assets;
    };
    std::vector<SceneBlock> m_scenes;
};

}  // namespace ico::engine