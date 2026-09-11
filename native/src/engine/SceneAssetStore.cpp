#include "engine/SceneAssetStore.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>

namespace ico::engine {

namespace {
const char* kManifestDir(const char* path) {
    /* Points at the last '/' (or the start). Call offset from the result. */
    const char* lastSlash = path;
    for (const char* p = path; *p; ++p) {
        if (*p == '/' || *p == '\\') {
            lastSlash = p;
        }
    }
    return lastSlash + 1;
}
}  // namespace

bool SceneAssetStore::parse(const char* manifestPath) {
    if (manifestPath == nullptr || *manifestPath == '\0') {
        return false;
    }

    std::ifstream f(manifestPath);
    if (!f) {
        return false;
    }

    const std::string manifest = manifestPath;
    const size_t slash = manifest.find_last_of('/');
    const std::string base = (slash == std::string::npos)
        ? std::string() : manifest.substr(0, slash + 1);
    return parseInto(f, base, -1, /*reset=*/true);
}

bool SceneAssetStore::parseRoom(const char* manifestPath, u32 sceneId) {
    if (manifestPath == nullptr || *manifestPath == '\0') {
        return false;
    }

    std::ifstream f(manifestPath);
    if (!f) {
        return false;
    }

    const std::string manifest = manifestPath;
    const size_t slash = manifest.find_last_of('/');
    const std::string base = (slash == std::string::npos)
        ? std::string() : manifest.substr(0, slash + 1);
    return parseInto(f, base, static_cast<int>(sceneId), /*reset=*/false);
}

bool SceneAssetStore::parseInto(std::istream& f, const std::string& base,
                                int forcedSceneId, bool reset) {
    if (reset) {
        m_scenes.clear();
        m_textureDir.clear();
        m_piecesDir.clear();
    }
    const std::size_t firstNewBlock = m_scenes.size();

    SceneBlock* currentScene = nullptr;
    std::string line;
    int lineNo = 0;
    while (std::getline(f, line)) {
        ++lineNo;
        /* Strip comments. */
        const size_t hash = line.find('#');
        if (hash != std::string::npos) {
            line.resize(hash);
        }
        std::istringstream ss(line);
        std::string token;
        if (!(ss >> token)) {
            continue;  /* blank or comment-only line */
        }

        if (token == "texturedir") {
            std::string value;
            if (!(ss >> value)) return false;
            m_textureDir = base + value;
            if (m_textureDir.empty() || m_textureDir.back() != '/') m_textureDir += '/';
        } else if (token == "piecesdir") {
            std::string value;
            if (!(ss >> value)) return false;
            m_piecesDir = base + value;
            if (m_piecesDir.empty() || m_piecesDir.back() != '/') m_piecesDir += '/';
        } else if (token == "scene") {
            std::string idStr;
            if (!(ss >> idStr)) return false;
            u32 id = 0;
            if (idStr.size() > 2 && idStr[0] == '0' && (idStr[1] == 'x' || idStr[1] == 'X')) {
                if (std::sscanf(idStr.c_str() + 2, "%x", &id) != 1) return false;
            } else {
                if (std::sscanf(idStr.c_str(), "%u", &id) != 1) return false;
            }
            m_scenes.push_back({id, {}});
            currentScene = &m_scenes.back();
        } else if (token == "end") {
            currentScene = nullptr;
        } else {
            /* A mesh file (relative to piecesdir). */
            if (currentScene == nullptr) return false;
            SceneAssetEntry entry;
            entry.meshPath = m_piecesDir + token;
            const size_t lastDot = token.find_last_of('.');
            const size_t lastSlash = token.find_last_of('/');
            const size_t start = (lastSlash == std::string::npos) ? 0 : lastSlash + 1;
            entry.label = token.substr(start, lastDot == std::string::npos
                                             ? std::string::npos : lastDot - start);
            currentScene->assets.push_back(std::move(entry));
        }
    }

    /* In parseRoom mode the caller forces a scene id; the manifest's own
       `scene` blocks are ignored. Only the blocks created by THIS call are
       merged, so a previous room's assets are never absorbed. */
    if (forcedSceneId >= 0 && firstNewBlock < m_scenes.size()) {
        std::vector<SceneAssetEntry> merged;
        for (std::size_t i = firstNewBlock; i < m_scenes.size(); ++i) {
            for (SceneAssetEntry& e : m_scenes[i].assets) {
                merged.push_back(std::move(e));
            }
        }
        m_scenes.resize(firstNewBlock);
        if (merged.empty()) return false;
        for (SceneBlock& block : m_scenes) {
            if (block.sceneId == static_cast<u32>(forcedSceneId)) {
                block.assets = std::move(merged);   /* dedupe into existing id */
                return true;
            }
        }
        m_scenes.push_back({static_cast<u32>(forcedSceneId), std::move(merged)});
    }

    if (m_textureDir.empty() || m_piecesDir.empty() || m_scenes.empty()) {
        return false;
    }
    m_initialized = true;
    return true;
}

bool SceneAssetStore::isInitialized() const {
    return m_initialized;
}

bool SceneAssetStore::hasScene(u32 sceneId) const {
    for (const SceneBlock& block : m_scenes) {
        if (block.sceneId == sceneId) return true;
    }
    return false;
}

std::size_t SceneAssetStore::sceneAssetCount(u32 sceneId) const {
    for (const SceneBlock& block : m_scenes) {
        if (block.sceneId == sceneId) {
            return block.assets.size();
        }
    }
    return 0;
}

const SceneAssetEntry* SceneAssetStore::sceneAsset(u32 sceneId,
                                                   std::size_t index) const {
    for (const SceneBlock& block : m_scenes) {
        if (block.sceneId == sceneId) {
            return (index < block.assets.size()) ? &block.assets[index] : nullptr;
        }
    }
    return nullptr;
}

std::vector<u32> SceneAssetStore::sceneIds() const {
    std::vector<u32> ids;
    ids.reserve(m_scenes.size());
    for (const SceneBlock& block : m_scenes) {
        ids.push_back(block.sceneId);
    }
    return ids;
}

}  // namespace ico::engine