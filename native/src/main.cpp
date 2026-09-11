#ifdef ICO_HAS_OPENGL
#include <X11/Xlib.h>
#include <X11/keysym.h>
#undef None
#undef Bool
#undef True
#undef False
#undef Status
#undef Always
#undef Never

#include "runtime/IcoRuntime.h"
#include "runtime/Logger.h"
#include <cstdio>

#include "platform/Input.h"
#include "engine/GifPacket.h"
#include "engine/OpenGLBackend.h"
#include "engine/Ps2oMesh.h"
#include "engine/RenderBackend.h"
#include "engine/ClipBridge.h"
#include "engine/GObjAttachment.h"
#include "engine/IsysGObjRuntime.h"
#include "engine/SceneAssetStore.h"
#include "engine/Tm2Converter.h"
#include "engine/Tm2Format.h"
#include "game/KanbanSceneLoader.h"
#include "game/BoyController.h"
#include "game/RoomTransitions.h"
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <algorithm>
#include <array>
#include <functional>
#include <memory>
#include <string>
#include <fstream>
#include <thread>
#include <unordered_map>
#include <vector>
#else
#include "runtime/IcoRuntime.h"
#include "runtime/Logger.h"
#include <cstdio>
#endif

namespace {

#ifdef ICO_HAS_OPENGL

struct ScenePiece {
    ico::engine::Ps2oMesh mesh;
    std::string name;
    std::vector<ico::engine::TextureHandle> texByMat;
    int room = 0;   // index into the DemoRoom list (Rev.170)
};

// Rev.170 (PORT): one room bundle = one composable scene. A demo can host
// several rooms; the door transition switches the ACTIVE room (geometry,
// clip, sky, spawn) while both stay resident so the swap is inexpensive.
struct DemoRoom {
    std::string name;
    std::vector<std::string> piecePaths;
    std::string texDir;
    const ico::engine::SceneAssetStore* store = nullptr;
    u32 sceneId = 0x0Fu;
};

// Per-room camera/spawn facts precomputed at load. Pieces are in world space
// (Rev.170 composition probe: the 0str/p1/p2/umi stages compose by
// coordinates, sea sheets span tens of thousands of units), so the fit is a
// bbox union resolved per room and the p1 room piece supplies the walkable
// spawn anchor.
struct RoomFit {
    float cx = 0.0f, cy = 0.0f, cz = 0.0f;
    float spawnX = 0.0f, spawnZ = 0.0f;
    float extent = 1.0f, dist = 1.0f;
};

// Pre-computed per-texture draw geometry. All triangles that share a texture
// handle are concatenated into one vertex/index block so the render loop can
// emit a single drawIndexed per texture (one state bind per flush) instead of
// re-deriving per-mesh/per-material vectors every frame.
struct TextureBatch {
    ico::engine::TextureHandle texture;
    std::vector<ico::engine::RenderVertex> vertices;
    std::vector<uint32_t> indices;
    uint32_t sourceTriangles = 0; // real PS2O triangles merged into this block
};

// Strip-semantics batch (native counterpart of gif_DrawStripF/G, GIF prim
// 0xD). Each PS2O frame primitive (Ps2oStrip) keeps its spine; N spine verts
// form N-2 triangles sharing edges. Concatenated into one vertex stream per
// texture with per-strip [firsts[i], counts[i]) spans so the whole texture
// renders via a single glMultiDrawArrays(GL_TRIANGLE_STRIP, ...) call,
// eliminating the [A,B,C,C] indexed duplication of TextureBatch (which draws
// 2 GPU triangles per source triangle, half degenerate).
struct StripBatch {
    ico::engine::TextureHandle texture;
    std::vector<ico::engine::RenderVertex> vertices; // concatenated spines
    std::vector<uint32_t> firsts;   // per-strip start index into vertices
    std::vector<uint32_t> counts;   // per-strip spine vertex count
    uint32_t sourceTriangles = 0;   // sum over strips of (count-2)
    uint32_t sourceStrips = 0;      // number of strips merged into this block
};

// Rev.155 (Passo 1) — per-GObj render payload. The renderer iterates the
// ACTIVE isysGObj lists and, for each GObj, draws the strip batches of the
// meshes that GObj owns through GObjAttachmentStore — instead of reading the
// SceneAssetStore globally. Batches are grouped per texture within the GObj.
struct GObjDraw {
    ico::engine::GObjHandle handle = ico::engine::kNullGObjHandle;
    std::vector<StripBatch> batches;
    std::vector<std::string> labels;  // owned mesh labels (debug)
};

// Loads a TM2 into a texture handle (no-op on kNullTexture result).
ico::engine::TextureHandle loadTm2Tex(ico::engine::OpenGLBackend& backend,
                                      const std::string& path) {
    using namespace ico::engine;
    std::ifstream f(path, std::ios::binary);
    if (!f) return kNullTexture;
    f.seekg(0, std::ios::end);
    const std::streamoff sz = f.tellg();
    f.seekg(0, std::ios::beg);
    if (sz <= 0) return kNullTexture;
    std::vector<u8> buf(static_cast<size_t>(sz));
    f.read(reinterpret_cast<char*>(buf.data()), sz);
    Tm2File file{};
    if (!Tm2Parser::parse(buf.data(), static_cast<u32>(buf.size()), file) || file.images.empty())
        return kNullTexture;
    Tm2Texture tex{};
    if (!Tm2Converter::convertImage(file.images[0], tex) || tex.rgbaData.empty())
        return kNullTexture;
    TextureDesc desc{};
    desc.width = tex.width; desc.height = tex.height;
    desc.format = TextureFormat::PSMCT32;
    desc.data = tex.rgbaData.data(); desc.dataSize = static_cast<u32>(tex.rgbaData.size());
    desc.generateMipmaps = false;
    TextureHandle h = backend.createTexture(desc);
    std::fprintf(stderr, "main: loaded TM2 %s (%ux%u)\n", path.c_str(), tex.width, tex.height);
    return h;
}

// Reads the room's real sky texture (sky.tm2 / *sky*.tm2 in the texture dir)
// and extracts the zenith (top) and horizon (bottom) colors from its vertical
// gradient. The PS2 sky asset is a tall gradient the runtime draws as a
// background quad; we sample the same colors instead of inventing an
// atmosphere. Returns true on success.
bool loadRoomSkyColors(const std::string& texDir,
                       unsigned char topColor[4], unsigned char bottomColor[4]) {
    using namespace ico::engine;
    namespace fs = std::filesystem;

    std::string skyPath;
    const std::string primary = texDir + "sky.tm2";
    {
        std::ifstream f(primary.c_str());
        if (f.good()) skyPath = primary;
    }
    if (skyPath.empty()) {
        for (const auto& entry : fs::directory_iterator(texDir)) {
            const std::string name = entry.path().filename().string();
            if (name.size() > 7 && name.compare(name.size() - 4, 4, ".tm2") == 0 &&
                name.find("sky") != std::string::npos) {
                skyPath = entry.path().string();
                break;
            }
        }
    }
    if (skyPath.empty()) return false;

    std::ifstream f(skyPath, std::ios::binary);
    if (!f) return false;
    f.seekg(0, std::ios::end);
    const std::streamoff sz = f.tellg();
    f.seekg(0, std::ios::beg);
    if (sz <= 0) return false;
    std::vector<u8> buf(static_cast<size_t>(sz));
    f.read(reinterpret_cast<char*>(buf.data()), sz);

    Tm2File file{};
    if (!Tm2Parser::parse(buf.data(), static_cast<u32>(buf.size()), file) ||
        file.images.empty())
        return false;
    Tm2Texture tex{};
    if (!Tm2Converter::convertImage(file.images[0], tex) || tex.rgbaData.empty())
        return false;

    // Average a few pixel rows near the top and bottom of the gradient.
    unsigned sr = 0, sg = 0, sb = 0, ler = 0, lg = 0, lb = 0;
    const u32 w = tex.width, h = tex.height;
    auto accum = [&](u32 y, unsigned& cr, unsigned& cg, unsigned& cb) {
        const u32 row = y * w * 4;
        for (u32 x = 0; x < w; ++x) {
            const u32 o = row + x * 4;
            cr += tex.rgbaData[o];
            cg += tex.rgbaData[o + 1];
            cb += tex.rgbaData[o + 2];
        }
    };
    for (u32 y = 0; y < h && y < 8; ++y) accum(y, sr, sg, sb);
    for (u32 y = (h >= 8) ? (h - 8) : 0; y < h; ++y) accum(y, ler, lg, lb);
    const u32 topN = w * ((h < 8) ? h : 8);
    const u32 botN = w * ((h < 8) ? h : 8);
    topColor[0] = static_cast<unsigned char>(sr / topN);
    topColor[1] = static_cast<unsigned char>(sg / topN);
    topColor[2] = static_cast<unsigned char>(sb / topN);
    topColor[3] = 255;
    bottomColor[0] = static_cast<unsigned char>(ler / botN);
    bottomColor[1] = static_cast<unsigned char>(lg / botN);
    bottomColor[2] = static_cast<unsigned char>(lb / botN);
    bottomColor[3] = 255;
    std::fprintf(stderr, "main: sky %s gradient top=(%u,%u,%u) horizon=(%u,%u,%u) (%ux%u)\n",
                 skyPath.c_str(), topColor[0], topColor[1], topColor[2],
                 bottomColor[0], bottomColor[1], bottomColor[2], w, h);
    return true;
}

// Multi-room scene render (Rev.170): draws every piece of every room every
// frame, binding the texture that each piece's material name table maps to
// material index f. The ACTIVE room drives camera/sky/collision/spawn; a door
// transition swaps the active room through the semantic requestScene/execute
// seam (the previous rooms stay resident in the same store).
//
// teleport (optional [x,z]) seeds the player at a fixed point in the START
// room instead of the fitted p1 center — used to land the boy inside a door
// zone for headless transition verification.
int runMultiRoomDemo(const std::vector<DemoRoom>& rooms, u32 startRoom,
                     u32 frames, const char* shotPath, bool uvTest,
                     float camAngleRad, bool fitMacro,
                     const float* teleport = nullptr);

int runSceneDemo(const std::vector<std::string>& piecePaths,
                 const std::string& texDir,
                 u32 frames, const char* shotPath, bool uvTest,
                 float camAngleRad, bool fitMacro,
                 const ico::engine::SceneAssetStore* store = nullptr);

int runMultiRoomDemo(const std::vector<DemoRoom>& rooms, u32 startRoom,
                     u32 frames, const char* shotPath, bool uvTest,
                     float camAngleRad, bool fitMacro,
                     const float* teleport) {
    using namespace ico::engine;

    if (rooms.empty()) {
        std::fprintf(stderr, "main: no rooms to render\n");
        return 1;
    }

    // Load all pieces (each tagged with its owning room index).
    std::vector<ScenePiece> pieces;
    for (std::size_t ri = 0; ri < rooms.size(); ++ri) {
        for (const auto& path : rooms[ri].piecePaths) {
            ScenePiece sp;
            sp.name = path;
            sp.room = static_cast<int>(ri);
            if (!loadPs2oMeshFromFile(path.c_str(), sp.mesh)) {
                std::fprintf(stderr, "main: failed to load PS2O mesh %s\n", path.c_str());
                continue;
            }
            const std::size_t synthesized = synthesizeTriangleStrips(sp.mesh);
            if (synthesized != 0) {
                std::fprintf(stderr,
                    "main:   strip synthesis: %zu strips from %u flat triangles "
                    "(unified GL_TRIANGLE_STRIP path)\n",
                    synthesized,
                    static_cast<uint32_t>(sp.mesh.triangles.size() / 3));
            }
            uint32_t maxMat = 0;
            for (auto m : sp.mesh.triMaterials) if (m > maxMat) maxMat = m;
            sp.texByMat.assign(maxMat + 1, kNullTexture);
            std::fprintf(stderr, "main: piece %s: %u verts, %u tris, %u submeshes, "
                                 "%u material names, fmax=%u\n",
                         path.c_str(),
                         static_cast<uint32_t>(sp.mesh.vertexCount),
                         static_cast<uint32_t>(sp.mesh.triangles.size() / 3),
                         sp.mesh.subMeshCount,
                         static_cast<uint32_t>(sp.mesh.materialNames.size()), maxMat);
            for (size_t i = 0; i < sp.mesh.materialNames.size(); ++i) {
                std::fprintf(stderr, "main:   material[%zu] -> %s.tm2\n",
                             i, sp.mesh.materialNames[i].c_str());
            }
            pieces.push_back(std::move(sp));
        }
    }
    if (pieces.empty()) {
        std::fprintf(stderr, "main: no pieces loaded\n");
        return 1;
    }

    // Per-room camera fit. Default orbits the room's p1 piece; with --fit-macro
    // the camera centers the union bbox of ALL pieces of that room (p2
    // backdrop and the sea included) so the whole stage is framed from afar.
    std::vector<RoomFit> fits(rooms.size());
    for (std::size_t ri = 0; ri < rooms.size(); ++ri) {
        RoomFit& fit = fits[ri];
        const ScenePiece* fitPiece = nullptr;
        if (!fitMacro) {
            for (const auto& sp : pieces) {
                if (sp.room != static_cast<int>(ri)) continue;
                if (sp.name.find("_p1.") != std::string::npos) { fitPiece = &sp; break; }
            }
            if (fitPiece == nullptr) {
                // Rev.170: rooms whose _p1 failed to parse should not frame a
                // decorative first piece; fit the largest loaded mesh instead.
                std::size_t bestTris = 0;
                for (const auto& sp : pieces) {
                    if (sp.room != static_cast<int>(ri)) continue;
                    const std::size_t t = sp.mesh.triangles.size() / 3;
                    if (t > bestTris) { bestTris = t; fitPiece = &sp; }
                }
            }
        }
        float minX = 1e30f, maxX = -1e30f;
        float minY = 1e30f, maxY = -1e30f;
        float minZ = 1e30f, maxZ = -1e30f;
        auto accumMesh = [&](const Ps2oMesh& m) {
            for (uint32_t i = 0; i < m.triangles.size(); ++i) {
                const uint32_t vi = m.triangles[i];
                minX = std::min(minX, m.positions[vi * 3 + 0]);
                maxX = std::max(maxX, m.positions[vi * 3 + 0]);
                minY = std::min(minY, m.positions[vi * 3 + 1]);
                maxY = std::max(maxY, m.positions[vi * 3 + 1]);
                minZ = std::min(minZ, m.positions[vi * 3 + 2]);
                maxZ = std::max(maxZ, m.positions[vi * 3 + 2]);
            }
        };
        if (fitMacro) {
            for (const auto& sp : pieces) {
                if (sp.room == static_cast<int>(ri)) accumMesh(sp.mesh);
            }
        } else if (fitPiece != nullptr) {
            accumMesh(fitPiece->mesh);
        }
        fit.cx = (minX + maxX) * 0.5f;
        fit.cy = (minY + maxY) * 0.5f;
        fit.cz = (minZ + maxZ) * 0.5f;

        // Spawn anchor: the walkable p1 room mesh's XZ center (independent of
        // the camera fit). With --fit-macro the camera frames the union of all
        // pieces but the boy must still spawn on the real floor. Falls back to
        // the largest loaded mesh of the room when _p1 failed to parse.
        fit.spawnX = fit.cx;
        fit.spawnZ = fit.cz;
        const ScenePiece* anchor = nullptr;
        for (const auto& sp : pieces) {
            if (sp.room != static_cast<int>(ri)) continue;
            if (sp.name.find("_p1.") != std::string::npos) { anchor = &sp; break; }
        }
        if (anchor == nullptr) {
            std::size_t bestTris = 0;
            for (const auto& sp : pieces) {
                if (sp.room != static_cast<int>(ri)) continue;
                const std::size_t t = sp.mesh.triangles.size() / 3;
                if (t > bestTris) { bestTris = t; anchor = &sp; }
            }
        }
        if (anchor != nullptr) {
            const auto& m = anchor->mesh;
            float mnX = 1e30f, mxX = -1e30f, mnZ = 1e30f, mxZ = -1e30f;
            for (uint32_t i = 0; i < m.triangles.size(); ++i) {
                const uint32_t vi = m.triangles[i];
                mnX = std::min(mnX, m.positions[vi * 3 + 0]);
                mxX = std::max(mxX, m.positions[vi * 3 + 0]);
                mnZ = std::min(mnZ, m.positions[vi * 3 + 2]);
                mxZ = std::max(mxZ, m.positions[vi * 3 + 2]);
            }
            fit.spawnX = (mnX + mxX) * 0.5f;
            fit.spawnZ = (mnZ + mxZ) * 0.5f;
        }
        fit.extent = std::max({maxX - minX, maxY - minY, maxZ - minZ, 1.0f});
        fit.dist = fit.extent * 1.15f;
        std::fprintf(stderr, "main: room %s camera fit %s: cx=%g cy=%g cz=%g extent=%g dist=%g\n",
                     rooms[ri].name.c_str(), fitMacro ? "macro(union)" : "p1(room)",
                     fit.cx, fit.cy, fit.cz, fit.extent, fit.dist);
    }
    u32 activeRoom = (startRoom < rooms.size()) ? startRoom : 0;
    float cx = fits[activeRoom].cx, cy = fits[activeRoom].cy, cz = fits[activeRoom].cz;
    float extent = fits[activeRoom].extent, dist = fits[activeRoom].dist;
    float spawnX = fits[activeRoom].spawnX, spawnZ = fits[activeRoom].spawnZ;
    std::fprintf(stderr, "main: active room %u (%s), scene 0x%02X\n",
                 static_cast<unsigned>(activeRoom), rooms[activeRoom].name.c_str(),
                 static_cast<unsigned>(rooms[activeRoom].sceneId));

    OpenGLBackend backend;
    if (!backend.initialize(kPs2ScreenWidth, kPs2ScreenHeight)) {
        std::fprintf(stderr, "main: OpenGL backend failed to initialize\n");
        return 1;
    }
    const Matrix4x4 proj = Matrix4x4::perspective(70.0f, 640.0f / 448.0f, 1.0f, dist * 10.0f);
    (void)proj;
    backend.setViewport(0, 0, kPs2ScreenWidth, kPs2ScreenHeight);
    backend.setDepthTest(GSDepthTest::Less, true);

    // Texture cache keyed by material name (shared across pieces).
    std::vector<std::pair<std::string, TextureHandle>> texCache;
    std::vector<std::string> missingLogged;
    // Rev.169 shared-bundle fallback: resolve a material texture from the room
    // texture dir first, then the shared scene/texture bundle (many rooms
    // reference stock tiles like st0_a/torch/window that only exist there).
    auto texForName = [&](const std::string& name, int ri) -> TextureHandle {
        const std::string key = std::to_string(ri) + ":" + name;
        for (const auto& kv : texCache) if (kv.first == key) return kv.second;
        std::vector<std::string> candidates;
        const std::string& roomTex = rooms[ri].texDir;
        if (!roomTex.empty()) candidates.push_back(roomTex + "/" + name + ".tm2");
        else candidates.push_back(name + ".tm2");
        const char* sharedCandidates[] = {
            "assets/scene/texture/", "../native/assets/scene/texture/", nullptr
        };
        for (int c = 0; sharedCandidates[c] != nullptr; ++c) {
            candidates.push_back(std::string(sharedCandidates[c]) + name + ".tm2");
        }
        TextureHandle h = kNullTexture;
        for (const auto& cand : candidates) {
            h = loadTm2Tex(backend, cand);
            if (h != kNullTexture) break;
        }
        texCache.emplace_back(key, h);
        if (h == kNullTexture) {
            const bool isNew =
                std::find(missingLogged.begin(), missingLogged.end(), name) ==
                missingLogged.end();
            if (isNew) {
                missingLogged.push_back(name);
                std::fprintf(stderr,
                    "main: missing texture %s (room dir + shared scene/texture)\n",
                    name.c_str());
            }
        }
        return h;
    };

    // Resolve per-piece, per-material textures from the piece's name table.
    // Pieces with no explicit material names (e.g. the 0str decorative
    // structures) default every material slot to the stage texture st0_a.
    for (auto& sp : pieces) {
        const size_t matCount = sp.texByMat.size();
        if (sp.mesh.materialNames.empty()) {
            if (matCount > 0) {
                const TextureHandle stage = texForName("st0_a", sp.room);
                for (size_t f = 0; f < matCount; ++f) sp.texByMat[f] = stage;
            }
            continue;
        }
        for (size_t f = 0; f < sp.mesh.materialNames.size() && f < matCount; ++f) {
            sp.texByMat[f] = texForName(sp.mesh.materialNames[f], sp.room);
        }
    }

    // Rev.161: the real ICO boy character mesh (boymodel.p2c, multi-OBJH .p2c
    // family) replaces the BoxMarker placeholder. Auto-loaded when present;
    // skipped when the piece list already contains the boy (standalone .p2c
    // demo, where the piece renders itself at native scale).
    //
    // Scale: the world is PS2 cm-scale (BoyController halfExtent_=12 cm; the
    // room stretches ~1700 units). The native bind-pose mesh is ~124 units
    // tall, so ~1.2x makes ICO a ~1.4 m boy next to the ~20 m castle room.
    // (Rev.161 first shipped 12x -> the boy towered over the castle and the
    // 64 px textures stretched into a "melting" smear.)
    const float kBoyScale = 1.2f;
    const float kBoyFootOffset = 0.65f * kBoyScale; // native feet at y ~ -0.65
    bool hasBoyMesh = false;
    Ps2oMesh boyMesh;
    std::vector<TextureHandle> boyTexByMat;
    {
        bool boyAlreadyPiece = false;
        for (const auto& p : pieces) {
            if (p.name.find("boymodel") != std::string::npos) boyAlreadyPiece = true;
        }
        if (!boyAlreadyPiece) {
            const char* boyCandidates[] = {
                "assets/boy/boymodel.p2c",
                "native/assets/boy/boymodel.p2c",
                "../native/assets/boy/boymodel.p2c",
                "../assets/boy/boymodel.p2c",
                nullptr
            };
            for (int c = 0; boyCandidates[c] != nullptr; ++c) {
                std::ifstream f(boyCandidates[c], std::ios::binary);
                if (!f.good()) continue;
                if (!loadPs2oMeshFromFile(boyCandidates[c], boyMesh)) continue;
                const std::string bpath = boyCandidates[c];
                const size_t slash = bpath.find_last_of('/');
                const std::string btexDir = (slash != std::string::npos)
                    ? bpath.substr(0, slash) + "/texture/" : "texture/";
                std::vector<std::pair<std::string, TextureHandle>> boyTexCache;
                auto boyTexForName = [&](const std::string& name) -> TextureHandle {
                    for (const auto& kv : boyTexCache) if (kv.first == name) return kv.second;
                    TextureHandle h = loadTm2Tex(backend, btexDir + name + ".tm2");
                    boyTexCache.emplace_back(name, h);
                    if (h == kNullTexture)
                        std::fprintf(stderr, "main: missing boy texture %s%s.tm2\n",
                                     btexDir.c_str(), name.c_str());
                    return h;
                };
                boyTexByMat.reserve(boyMesh.materialNames.size());
                for (const auto& nm : boyMesh.materialNames)
                    boyTexByMat.push_back(boyTexForName(nm));
                hasBoyMesh = !boyMesh.strips.empty();
                std::fprintf(stderr, "main: boy mesh %s: %u verts, %u strips, "
                                     "%u submeshes, %u materials\n",
                             boyCandidates[c],
                             static_cast<u32>(boyMesh.positions.size() / 3),
                             static_cast<u32>(boyMesh.strips.size()),
                             boyMesh.subMeshCount,
                             static_cast<u32>(boyMesh.materialNames.size()));
                break;
            }
        }
    }

    // UV-validation checkerboard (see --uv-test). Created before the geometry
    // pre-pass so untextured materials can be tinted with it when enabled.
    TextureHandle checkerTex = kNullTexture;
    if (uvTest) {
        const int cw = 64, ch = 64;
        std::vector<u8> px(static_cast<size_t>(cw) * ch * 4);
        for (int y = 0; y < ch; ++y) {
            for (int x = 0; x < cw; ++x) {
                const int cellX = x / 8, cellY = y / 8;
                const int remX = x % 8, remY = y % 8;
                const bool border = (remX == 0 || remY == 0);
                const bool even = ((cellX + cellY) & 1) == 0;
                const int bright = even ? 220 : 50;
                u8 r = static_cast<u8>((bright * (7 - remX)) / 7);
                u8 g = static_cast<u8>((bright * remY) / 7);
                u8 b = border ? 255 : 90;
                if (border) { r = 255; g = 255; }
                size_t o = static_cast<size_t>(y * cw + x) * 4;
                px[o + 0] = r; px[o + 1] = g; px[o + 2] = b; px[o + 3] = 255;
            }
        }
        TextureDesc cdesc{};
        cdesc.width = cw; cdesc.height = ch;
        cdesc.format = TextureFormat::PSMCT32;
        cdesc.data = px.data(); cdesc.dataSize = static_cast<u32>(px.size());
        cdesc.generateMipmaps = false;
        checkerTex = backend.createTexture(cdesc);
        std::fprintf(stderr, "main: UV-validation checkerboard texture %ux%u enabled\n", cw, ch);
    }

    // Build a single concatenated vertex/index block per texture. Entries are
    // ordered so that repeated binds of the same texture are adjacent, and
    // texture-identical primitives across pieces/footprints share one block.
    using namespace ico::engine;
    auto buildBatches = [&](TextureHandle fallbackTex, int roomFilter) -> std::vector<TextureBatch> {
        std::vector<TextureBatch> batch;
        for (const auto& sp : pieces) {
            if (roomFilter >= 0 && sp.room != roomFilter) continue;
            const auto& mesh = sp.mesh;
            if (mesh.triangles.empty()) continue;
            const uint32_t triCount = static_cast<uint32_t>(mesh.triangles.size() / 3);

            int maxMat = 0;
            for (auto m : mesh.triMaterials) if (m > maxMat) maxMat = m;

            for (int f = 0; f <= maxMat; ++f) {
                // Collect triangles of this material on this piece.
                std::vector<uint32_t> matTris;
                for (uint32_t t = 0; t < triCount; ++t)
                    if ((int)mesh.triMaterials[t] == f) matTris.push_back(t);
                if (matTris.empty()) continue;

                TextureHandle tex = (f < (int)sp.texByMat.size())
                    ? sp.texByMat[f] : kNullTexture;
                if (tex == kNullTexture && fallbackTex != kNullTexture) tex = fallbackTex;

                // Find or create a batch for this texture.
                TextureBatch* tb = nullptr;
                for (auto& b : batch) if (b.texture == tex) { tb = &b; break; }
                if (tb == nullptr) {
                    batch.push_back(TextureBatch{});
                    tb = &batch.back();
                    tb->texture = tex;
                }

                const uint32_t baseVert = static_cast<uint32_t>(tb->vertices.size());
                tb->sourceTriangles += static_cast<uint32_t>(matTris.size());
                // Backend drawIndexed consumes quads of 4 vertices ([A,B,C,C]
                // degenerates to two triangles); emit 4 verts per source tri.
                for (uint32_t gi = 0; gi < matTris.size(); ++gi) {
                    uint32_t t = matTris[gi];
                    const bool hasTexUV = mesh.triVertUVs.size() >= (t * 6 + 2);
                    RenderVertex v[3]{};
                    for (int s = 0; s < 3; ++s) {
                        const uint32_t vi = mesh.triangles[t * 3 + s];
                        v[s].x = mesh.positions[vi * 3 + 0];
                        v[s].y = mesh.positions[vi * 3 + 1];
                        v[s].z = mesh.positions[vi * 3 + 2];
                        if (hasTexUV) {
                            v[s].u = mesh.triVertUVs[t * 6 + s * 2 + 0];
                            v[s].v = mesh.triVertUVs[t * 6 + s * 2 + 1];
                        }
                        v[s].r = 255; v[s].g = 255; v[s].b = 255; v[s].a = 255;
                    }
                    tb->vertices.push_back(v[0]);
                    tb->vertices.push_back(v[1]);
                    tb->vertices.push_back(v[2]);
                    tb->vertices.push_back(v[2]); // degenerate 4th
                    const uint32_t bv = baseVert + gi * 4;
                    tb->indices.push_back(bv + 0);
                    tb->indices.push_back(bv + 1);
                    tb->indices.push_back(bv + 2);
                    tb->indices.push_back(bv + 3);
                }
            }
        }
        return batch;
    };

    // Appends the strips of a single piece into a per-texture batch list.
    // Shared by the global room path (buildStripBatches) and the per-GObj
    // (Passo 1) path.
    auto appendStripsOfPiece = [&](const ScenePiece& sp, TextureHandle fallbackTex,
                                   std::vector<StripBatch>& batch) {
        const auto& mesh = sp.mesh;
        if (mesh.strips.empty()) return;
        for (const auto& strip : mesh.strips) {
            const uint32_t n = static_cast<uint32_t>(strip.spine.size());
            if (n < 3) continue;

            TextureHandle tex = (strip.material < sp.texByMat.size())
                ? sp.texByMat[strip.material] : kNullTexture;
            if (tex == kNullTexture && fallbackTex != kNullTexture) tex = fallbackTex;

            StripBatch* sb = nullptr;
            for (auto& b : batch) if (b.texture == tex) { sb = &b; break; }
            if (sb == nullptr) {
                batch.push_back(StripBatch{});
                sb = &batch.back();
                sb->texture = tex;
            }

            const uint32_t baseVert = static_cast<uint32_t>(sb->vertices.size());
            const bool hasStripUV = strip.uvs.size() >= n * 2;
            for (uint32_t k = 0; k < n; ++k) {
                const uint32_t vi = strip.spine[k];
                RenderVertex v{};
                v.x = mesh.positions[vi * 3 + 0];
                v.y = mesh.positions[vi * 3 + 1];
                v.z = mesh.positions[vi * 3 + 2];
                if (hasStripUV) {
                    v.u = strip.uvs[k * 2 + 0];
                    v.v = strip.uvs[k * 2 + 1];
                }
                v.r = 255; v.g = 255; v.b = 255; v.a = 255;
                sb->vertices.push_back(v);
            }
            sb->firsts.push_back(baseVert);
            sb->counts.push_back(n);
            sb->sourceTriangles += n - 2;
            sb->sourceStrips++;
        }
    };

    // Pre-built global per-texture strip batches (single room draw path,
    // used when no GObj attachment composition is available).
    auto buildStripBatches = [&](TextureHandle fallbackTex, int roomFilter) -> std::vector<StripBatch> {
        std::vector<StripBatch> batch;
        for (const auto& sp : pieces) {
            if (roomFilter >= 0 && sp.room != roomFilter) continue;
            if (sp.mesh.strips.empty()) continue;
            appendStripsOfPiece(sp, fallbackTex, batch);
        }
        return batch;
    };

    // Checkerboard is applied only when --uv-test is set; otherwise the loop
    // assigns kNullTexture for untextured batches (white).
    const std::vector<TextureBatch> textureBatches = buildBatches(
        uvTest ? checkerTex : kNullTexture, static_cast<int>(activeRoom));
    std::vector<StripBatch> stripBatches = buildStripBatches(
        uvTest ? checkerTex : kNullTexture, static_cast<int>(activeRoom));
    const bool useStrips = !stripBatches.empty();

    // Batch instrumentation: per-texture source triangles vs emitted
    // [A,B,C,C] quad-group geometry. backend.drawIndexed consumes groups of
    // 4 indices as 2 triangles (real + (A,C,C) degenerate), so emitted
    // triangle count = 2 * sourceTriangles while the GPU only rasterizes
    // sourceTriangles of them. This resolves whether the earlier per-triangle
    // duplication is real or an artifact of the quad-group contract. When the
    // mesh preserves strip topology, the StripBatch report shows the strip
    // path instead (N spine verts -> N-2 triangles, no duplication).
    if (useStrips) {
        uint32_t totalSrc = 0, totalVerts = 0, totalStrips = 0;
        std::fprintf(stderr, "main: strip batch report (per texture):\n");
        for (const auto& sb : stripBatches) {
            const uint32_t verts = static_cast<uint32_t>(sb.vertices.size());
            std::fprintf(stderr,
                "main:   tex=%llu strips=%-6u srcTris=%-6u verts=%-6u "
                "verts/srcTri=%.2f (strip: N spine verts -> N-2 tris)\n",
                static_cast<unsigned long long>(sb.texture), sb.sourceStrips,
                sb.sourceTriangles, verts,
                sb.sourceTriangles ? (float)verts / (float)sb.sourceTriangles : 0.0f);
            totalSrc += sb.sourceTriangles;
            totalVerts += verts;
            totalStrips += sb.sourceStrips;
        }
        std::fprintf(stderr,
            "main:   TOTAL strips=%u srcTris=%u verts=%u | "
            "verts/srcTri=%.2f (GPU tris = srcTris, degenerates = strip cosets)\n",
            totalStrips, totalSrc, totalVerts,
            totalSrc ? (float)totalVerts / (float)totalSrc : 0.0f);
    } else {
        uint32_t totalSrc = 0, totalVerts = 0, totalIdx = 0;
        std::fprintf(stderr, "main: batch report (per texture):\n");
        for (const auto& tb : textureBatches) {
            const uint32_t verts = static_cast<uint32_t>(tb.vertices.size());
            const uint32_t idx = static_cast<uint32_t>(tb.indices.size());
            const uint32_t quadGroups = idx / 4;
            const uint32_t emittedTris = quadGroups * 2;
            std::fprintf(stderr,
                "main:   tex=%llu srcTris=%-6u verts=%-6u idx=%-6u "
                "quadGroups=%-6u emittedTris=%-6u (real=%u degenerate=%u)\n",
                static_cast<unsigned long long>(tb.texture), tb.sourceTriangles,
                verts, idx, quadGroups, emittedTris, tb.sourceTriangles,
                emittedTris - tb.sourceTriangles);
            totalSrc += tb.sourceTriangles;
            totalVerts += verts;
            totalIdx += idx;
        }
        std::fprintf(stderr,
            "main:   TOTAL srcTris=%u verts=%u idx=%u | verts/srcTri=%.2f "
            "idx/srcTri=%.2f (ideal indexed=3 verts+3 idx/tri)\n",
            totalSrc, totalVerts, totalIdx,
            totalSrc ? (float)totalVerts / (float)totalSrc : 0.0f,
            totalSrc ? (float)totalIdx / (float)totalSrc : 0.0f);
    }

    const auto frameTime = std::chrono::milliseconds(16);
    std::vector<uint8_t> tmpFrame(kPs2ScreenWidth * kPs2ScreenHeight * 3);

    // Interactive camera state (orbit + pan + pitch via keyboard/mouse wheel).
    float curDist = dist;
    float curAngle = (camAngleRad > 0.0f) ? camAngleRad : 0.0f;
    float curPitch = 0.15f;      // elevation above the target (rad)
    float panX = 0.0f, panZ = 0.0f;  // target offset for free navigation
    bool followBoy = false;      // third-person camera tracking the player

    // Input wiring: platform Input class fed from the X11 event pump. The
    // key state then drives a real world-space player controller (WASD moves
    // it in the room's XZ plane, resolved through the ClipBridge built from
    // the room mesh) so the native runtime consumes genuine keyboard input
    // from a single source.
    Input input;
    input.initialize();

    // Collision ground truth per room: the _p1 room piece, regardless of
    // camera fit (falling back to the room's LARGEST piece that actually
    // loaded). Each room keeps its own ClipBridge; the door transition
    // re-points the player onto the freshly entered room's grid instead of
    // reusing the previous room's collision.
    //
    // Rev.170 (PORT): the first-piece fallback was wrong — several rooms load
    // a decorative flare/emitter first (st02a's 02a_flare1.p2o produced a
    // useless 8x7 grid of ~1 cell). And st02a_p1.p2o currently FAILS to parse:
    // it is a 67-submesh p2-family variant that the p1 cascade decoder (Rev.151)
    // rejects — a separate open decode item. So the fallback now picks the
    // largest successfully-parsed mesh of the room (st02a -> st02a_p2.p2o,
    // 21k tris) so the room keeps real collision.
    std::vector<std::unique_ptr<ClipBridge>> roomClips(rooms.size());
    for (std::size_t ri = 0; ri < rooms.size(); ++ri) {
        const ScenePiece* roomPiece = nullptr;
        for (const auto& sp : pieces) {
            if (sp.room == static_cast<int>(ri) &&
                sp.name.find("_p1.") != std::string::npos) { roomPiece = &sp; break; }
        }
        if (roomPiece == nullptr) {
            std::size_t bestTris = 0;
            for (const auto& sp : pieces) {
                if (sp.room != static_cast<int>(ri)) continue;
                const std::size_t t = sp.mesh.triangles.size() / 3;
                if (t > bestTris) { bestTris = t; roomPiece = &sp; }
            }
            if (roomPiece != nullptr) {
                std::fprintf(stderr,
                    "main: room %s has no loadable _p1 piece; collision from largest mesh (%zu tris)\n",
                    rooms[ri].name.c_str(), bestTris);
            }
        }
        if (roomPiece == nullptr) continue;
        roomClips[ri] = std::make_unique<ClipBridge>();
        if (!roomClips[ri]->buildFromMesh(roomPiece->mesh.positions.data(),
                                          roomPiece->mesh.vertexCount,
                                          roomPiece->mesh.triangles.data(),
                                          static_cast<u32>(roomPiece->mesh.triangles.size() / 3))) {
            std::fprintf(stderr,
                "main: room %s ClipBridge failed to build from %s; room renders viewer-only\n",
                rooms[ri].name.c_str(), roomPiece->name.c_str());
        } else {
            std::fprintf(stderr, "main: room %s ClipBridge %s grid=%ux%u blocked=%u (from %s)\n",
                         rooms[ri].name.c_str(),
                         roomClips[ri]->isInitialized() ? "ok" : "unavailable",
                         roomClips[ri]->gridWidth(), roomClips[ri]->gridHeight(),
                         roomClips[ri]->blockedCellCount(), roomPiece->name.c_str());
        }
    }
    ClipBridge* activeClip = nullptr;
    if (activeRoom < roomClips.size() && roomClips[activeRoom] &&
        roomClips[activeRoom]->isInitialized()) {
        activeClip = roomClips[activeRoom].get();
    }

    // Only the walkable room gets a runtime player (rooms whose p1 piece does
    // not rasterize a grid render viewer-only). Respawn is shared by the
    // initial seed and every door transition: the SAME BoyController keeps its
    // GObj/process and only re-points its collision bridge (no re-registration,
    // no process leak), then re-runs the walkable spawn probe.
    IsysGObjRuntime gobjRuntime;
    if (!gobjRuntime.initialize(0x40, 0x40)) {
        std::fprintf(stderr, "main: gobj runtime failed to initialize\n");
        return 1;
    }
    ico::game::BoyController player;
    bool playerSpawned = false;
    std::string spawnAnchor = "room center";
    auto respawnPlayer = [&](float sx, float sz) -> bool {
        spawnX = sx;
        spawnZ = sz;
        if (activeClip == nullptr) return false;
        if (!player.isInitialized()) {
            if (!player.initialize(gobjRuntime, *activeClip, 1u)) {
                std::fprintf(stderr, "main: BoyController failed to initialize\n");
                return false;
            }
        } else {
            player.setBridge(*activeClip);
        }
        if (player.spawn(sx, sz, player.halfExtent())) {
            playerSpawned = true;
            spawnAnchor = "room center";
            return true;
        }
        // Rev.169 spawn fallback: room bbox centres (st02a) can land in a
        // courtyard void / well mouth with no floor samples, and the p1 grid
        // may be tiny. Re-anchor the player on the highest walkable floor
        // cell found by the ClipBridge (wall-tip 1-cell floors excluded).
        float bx = 0.0f, by = 0.0f, bz = 0.0f;
        if (activeClip->bestFloorPoint(bx, by, bz) &&
            player.spawn(bx, bz, player.halfExtent())) {
            playerSpawned = true;
            spawnAnchor = "highest walkable floor";
            spawnX = bx;
            spawnZ = bz;
            (void)by;
            return true;
        }
        std::fprintf(stderr,
            "main: no walkable spawn point at (%g,%g); room renders viewer-only\n",
            sx, sz);
        playerSpawned = false;
        return false;
    };
    if (teleport != nullptr) {
        spawnX = teleport[0];
        spawnZ = teleport[1];
        respawnPlayer(spawnX, spawnZ);
        std::fprintf(stderr, "main: teleport override active at (%g,%g)\n",
                     spawnX, spawnZ);
    } else {
        respawnPlayer(fits[activeRoom].spawnX, fits[activeRoom].spawnZ);
    }
    std::fprintf(stderr, "main: player spawn at (%g,%g) [%s] -> %s pos=(%g,%g,%g)\n",
                 spawnX, spawnZ, spawnAnchor.c_str(), playerSpawned ? "ok" : "failed",
                 player.x(), player.y(), player.z());

    // Rev.155 (Passo 1): the boy's visual is ALSO a GObj-owned composition.
    // The BoyController's GObj receives a BoxMarker attachment; the render
    // loop reads that attachment's transform (updated every frame from the
    // controller position) and draws the placeholder box it commands — the
    // renderer no longer hardcodes the box around markerX/Y/Z.
    ico::engine::GObjAttachmentStore boyStore;
    ico::engine::GObjHandle boyHandle = ico::engine::kNullGObjHandle;
    if (playerSpawned && player.getGObj() != nullptr) {
        boyHandle = gobjRuntime.pool().handleOf(*player.getGObj());
        ico::engine::GObjRenderAttachment marker{};
        marker.handle = boyHandle;
        marker.kind = ico::engine::GObjAttachmentKind::BoxMarker;
        marker.active = true;
        marker.meshLabel = "BoyController";
        marker.halfExtent = 20.0f;
        marker.boxColor[0] = 255; marker.boxColor[1] = 100;
        marker.boxColor[2] = 100; marker.boxColor[3] = 255;
        boyStore.attach(marker);
    }
    std::fprintf(stderr, "main: boy GObj %u -> BoxMarker attachment (Passo 1)\n",
                 static_cast<unsigned>(boyHandle));

    // Semantic scene-load bridge (front 2 — KanbanSceneLoader seam). The
    // loader consumes the real room composition through the original
    // requestScene()/execute() (kanban.c) flow and creates host GObjs in the
    // same gobjRuntime the BoyController process uses. The render loop below
    // stays decoupled — it reads the bound SceneAssetStore directly — so this
    // is the wiring where runtime-validated GObj creation feeds later revisions.
    // Rev.154: applyVerifiedSceneTables() enables the USA-ELF scene-0x0F slice
    // (entries 847..875), so initSceneGObj() creates 25 host GObjs (descriptors
    // 44/54 are gate-0 and skip creation).
    KanbanSceneLoader sceneLoader;
    if (!sceneLoader.initialize(gobjRuntime)) {
        std::fprintf(stderr, "main: KanbanSceneLoader failed to initialize\n");
        return 1;
    }
    // Rev.154: verified descriptor/entry/range tables extracted from the USA
    // ELF (tools/extract_scene_tables.py). Enables the real scene-0x0F sample
    // so initSceneGObj() creates the Boy-facing GObjs instead of 0.
    if (!sceneLoader.applyVerifiedSceneTables(
            ico::engine::kVerifiedSceneDescriptors,
            ico::engine::kVerifiedSceneDescriptorCount,
            ico::engine::kVerifiedScenePayload,
            ico::engine::kVerifiedScenePayloadCount,
            ico::engine::kVerifiedSceneRanges,
            ico::engine::kVerifiedSceneRangeCount)) {
        std::fprintf(stderr, "main: failed to apply verified scene tables\n");
        return 1;
    }
    // Rev.159 (PORT): verified per-room primary-handler repertoires from the
    // runtime capture (tools/extract_room_role_tables.py). Feeding them makes
    // attachBoundAssetsToGObjs() re-link each GObj to its room's handler roles
    // instead of a blind round-robin, and execute() re-links on every scene
    // transition.
    sceneLoader.applyVerifiedRoomRolePlans(
        ico::engine::kVerifiedRoomRolePlans,
        ico::engine::kVerifiedRoomRolePlanCount);

    // Bind every room's store/scene pair. The same SceneAssetStore may carry
    // several scenes (Rev.170 parseRoom) or each room may supply its own.
    if (!rooms.empty()) {
        for (const DemoRoom& r : rooms) {
            if (r.store == nullptr) continue;
            if (sceneLoader.bindSceneAssets(*r.store, r.sceneId)) {
                std::fprintf(stderr,
                    "main: loader bound scene 0x%02X (%s): %zu assets (first=%s)\n",
                    static_cast<unsigned>(r.sceneId), r.name.c_str(),
                    sceneLoader.boundAssetCount(r.sceneId),
                    sceneLoader.boundAsset(r.sceneId, 0)
                        ? sceneLoader.boundAsset(r.sceneId, 0)->label.c_str()
                        : "?");
            }
        }
        const u32 requestSceneId = rooms[activeRoom].sceneId;
        sceneLoader.requestScene(requestSceneId);
        const bool loaded = sceneLoader.execute();
        const std::size_t createdGObjs = sceneLoader.sceneGObjCount();
        std::fprintf(stderr,
            "main: scene 0x%02X semantic load via requestScene/execute: %s "
            "(currentSceneId=0x%02X, %zu host GObjs)\n",
            static_cast<unsigned>(requestSceneId),
            loaded ? "ok" : "no-op",
            static_cast<unsigned>(sceneLoader.currentSceneId()), createdGObjs);
    } else {
        std::fprintf(stderr,
            "main: no SceneAssetStore(s) supplied; KanbanSceneLoader runs "
            "unbound (viewer path)\n");
    }

    // Rev.155 (Passo 1): per-GObj render payload. When a store is bound, the
    // attachment store now carries the (GObj handle → mesh) pairing the loader
    // established. Build one StripBatch set per owning GObj and give the
    // render loop a handle→draw lookup. The loop below walks the ACTIVE
    // isysGObj lists and draws what each GObj commands; the global room
    // stripBatches remain a fallback for the unbound viewer path.
    std::vector<GObjDraw> gobjDraws;
    std::unordered_map<GObjHandle, std::size_t> gobjDrawByHandle;
    const TextureHandle gObjFallbackTex = uvTest ? checkerTex : kNullTexture;
    // Rev.168 (PORT): the per-GObj draw set is rebuilt after every room
    // transition, because initSceneGObj() releases the previous scene's GObjs
    // and re-links the attachments (Rev.159). Extract the build so both the
    // initial load and the transition path consume the same logic.
    auto rebuildGObjDraws = [&]() {
        gobjDraws.clear();
        gobjDrawByHandle.clear();
        // MeshPath → loaded ScenePiece lookup (paths come from the same store).
        std::unordered_map<std::string, std::size_t> pieceByPath;
        for (std::size_t i = 0; i < pieces.size(); ++i)
            pieceByPath.emplace(pieces[i].name, i);
        sceneLoader.attachmentStore().forEach(
            [&](const GObjRenderAttachment& att) {
                if (att.kind != GObjAttachmentKind::Mesh || !att.active) return;
                if (att.handle == kNullGObjHandle) return;
                auto pit = pieceByPath.find(att.meshPath);
                if (pit == pieceByPath.end()) return;
                const ScenePiece& sp = pieces[pit->second];
                if (sp.mesh.strips.empty()) return;

                auto dit = gobjDrawByHandle.find(att.handle);
                if (dit == gobjDrawByHandle.end()) {
                    gobjDrawByHandle.emplace(att.handle, gobjDraws.size());
                    gobjDraws.push_back(GObjDraw{});
                    gobjDraws.back().handle = att.handle;
                    dit = gobjDrawByHandle.find(att.handle);
                }
                GObjDraw& draw = gobjDraws[dit->second];
                appendStripsOfPiece(sp, gObjFallbackTex, draw.batches);
                draw.labels.push_back(att.meshLabel);
            });
        for (const auto& kv : gobjDrawByHandle) {
            const auto& d = gobjDraws[kv.second];
            std::fprintf(stderr, "main:   GObj %u draws %zu labels, %zu strip batches\n",
                         static_cast<unsigned>(kv.first), d.labels.size(), d.batches.size());
        }
    };
    if (rooms[activeRoom].store != nullptr) {
        const u32 sceneId = rooms[activeRoom].sceneId;
        std::size_t attached = sceneLoader.attachBoundAssetsToGObjs(sceneId);
        std::fprintf(stderr,
            "main: Passo 1 per-GObj composition: %zu attachments for %zu host "
            "GObjs of scene 0x%02X\n",
            attached, sceneLoader.sceneGObjCount(),
            static_cast<unsigned>(sceneId));
        rebuildGObjDraws();
    }

    // Real room sky per room: sample each room's sky.tm2 gradient; rooms without
    // one fall back to a daylight haze placeholder (Rev.169, host
    // presentation). The door-transition callback switches these arrays.
    std::vector<bool> roomHasSky(rooms.size(), false);
    std::vector<std::array<unsigned char, 4>> roomSkyTop(rooms.size());
    std::vector<std::array<unsigned char, 4>> roomSkyHorizon(rooms.size());
    for (std::size_t ri = 0; ri < rooms.size(); ++ri) {
        roomSkyTop[ri] = {{10, 12, 18, 255}};
        roomSkyHorizon[ri] = {{10, 12, 18, 255}};
        if (loadRoomSkyColors(rooms[ri].texDir, roomSkyTop[ri].data(),
                              roomSkyHorizon[ri].data())) {
            roomHasSky[ri] = true;
        } else {
            // Rev.169 host presentation: rooms without an extracted sky.tm2
            // render as a daylight haze (pale-blue zenith, pale horizon) so
            // exterior vistas do not read as a black void. Explicitly host-
            // side; the original room may tint its backdrop differently.
            roomSkyTop[ri] = {{0x78, 0x98, 0xC8, 255}};
            roomSkyHorizon[ri] = {{0xC4, 0xD2, 0xDC, 255}};
        }
    }
    bool hasSky = roomHasSky[activeRoom];
    unsigned char skyTop[4];
    unsigned char skyHorizon[4];
    for (int q = 0; q < 4; ++q) {
        skyTop[q] = roomSkyTop[activeRoom][q];
        skyHorizon[q] = roomSkyHorizon[activeRoom][q];
    }

    // Rev.168/170 (PORT): door-triggered room transition. Each room declares
    // its own door piece (`2a_door1.p2o` st02a / `door.p2o` st00a / 169_door
    // scene mode); the zone is fitted from the piece AABB and the target spawn
    // sits 60 units past the door along the (door − room-center) axis. Firing
    // the transition drives requestScene/execute() (kanban.c seam): it releases
    // the current room's GObjs, loads the target scene's (verified isysGObj
    // tables, Rev.154/159), re-links attachments, rebuilds the per-GObj draw
    // set, re-points the boy onto the new room's ClipBridge and re-seeds him at
    // the walkable spawn probe. Zone/spawn values are HOST heuristics, not
    // byte-verified original door data.
    struct RoomDoor {
        bool present = false;
        float x = 0.0f, z = 0.0f;
        float spawnX = 0.0f, spawnZ = 0.0f;
        u32 targetScene = 0;
    };
    std::vector<RoomDoor> roomDoors(rooms.size());
    for (std::size_t ri = 0; ri < rooms.size(); ++ri) {
        const ScenePiece* doorPiece = nullptr;
        for (const auto& sp : pieces) {
            if (sp.room != static_cast<int>(ri)) continue;
            if (sp.name.find("door") != std::string::npos) { doorPiece = &sp; break; }
        }
        if (doorPiece == nullptr || doorPiece->mesh.triangles.empty()) continue;
        float mnX = 1e30f, mxX = -1e30f, mnZ = 1e30f, mxZ = -1e30f;
        const auto& m = doorPiece->mesh;
        for (uint32_t i = 0; i < m.triangles.size(); ++i) {
            const uint32_t vi = m.triangles[i];
            mnX = std::min(mnX, m.positions[vi * 3 + 0]);
            mxX = std::max(mxX, m.positions[vi * 3 + 0]);
            mnZ = std::min(mnZ, m.positions[vi * 3 + 2]);
            mxZ = std::max(mxZ, m.positions[vi * 3 + 2]);
        }
        RoomDoor& d = roomDoors[ri];
        d.present = true;
        d.x = (mnX + mxX) * 0.5f;
        d.z = (mnZ + mxZ) * 0.5f;

        // Rev.170 (PORT): a door trigger only works if the player can STAND on
        // it. The door piece AABB centre (2a_door1 = (-210,25)) had NO walkable
        // floor sample on st02a's p1 ClipBridge, so a spawn probe there failed
        // and the boy fell back far from the zone — the door never opened.
        // Snap the zone to the nearest world-space point with real floor
        // support (spiral search up to 150 units) via the room's ClipBridge.
        if (ri < roomClips.size() && roomClips[ri] && roomClips[ri]->isInitialized()) {
            const ClipBridge* cb = roomClips[ri].get();
            // Same 5-corner standable test BoyController::spawn uses
            // (center + 4 corners at halfExtent 12) so the zone centre is a
            // point the player can actually stand on — a single floor sample
            // (floorHeightAt) was not enough and the boy teleported ~60 units
            // off the door.
            const float ext = 12.0f;
            auto standable5 = [&](float px, float pz) -> bool {
                float h = 0.0f;
                const std::array<std::array<float, 2>, 5> pts{
                    std::array<float, 2>{px, pz},
                    std::array<float, 2>{px - ext, pz - ext},
                    std::array<float, 2>{px + ext, pz - ext},
                    std::array<float, 2>{px - ext, pz + ext},
                    std::array<float, 2>{px + ext, pz + ext}};
                for (const auto& pt : pts) {
                    if (!cb->floorHeightAt(pt[0], pt[1], h)) return false;
                }
                return true;
            };
            bool found = false;
            for (float r = 0.0f; r <= 150.0f && !found; r += 10.0f) {
                for (int k = 0; k < 32; ++k) {
                    const float a = static_cast<float>(k) * 6.28318530718f / 32.0f;
                    const float px = d.x + r * std::cos(a);
                    const float pz = d.z + r * std::sin(a);
                    if (standable5(px, pz)) {
                        std::fprintf(stderr,
                            "main:   door %s snapped to standable floor at (%g,%g)\n",
                            rooms[ri].name.c_str(), px, pz);
                        d.x = px;
                        d.z = pz;
                        found = true;
                        break;
                    }
                }
            }
        }

        float ax = d.x - fits[ri].spawnX, az = d.z - fits[ri].spawnZ;
        const float len = std::sqrt(ax * ax + az * az);
        const float nx = (len > 1e-3f) ? ax / len : 1.0f;
        const float nz = (len > 1e-3f) ? az / len : 0.0f;
        d.spawnX = d.x + nx * 60.0f;
        d.spawnZ = d.z + nz * 60.0f;
        // 2-room demo: the door targets the OTHER room's scene id.
        d.targetScene = rooms[(ri + 1) % rooms.size()].sceneId;
        std::fprintf(stderr,
            "main: room %s door '%s' at (%g,%g) r=40 -> scene 0x%02X, spawn(%g,%g)\n",
            rooms[ri].name.c_str(), doorPiece->name.c_str(), d.x, d.z,
            static_cast<unsigned>(d.targetScene), d.spawnX, d.spawnZ);
    }

    ico::game::RoomTransitions roomTransitions;
    std::function<void(u32)> installRoomTransitions = [&](u32 ri) {
        roomTransitions.reset();
        if (rooms.size() < 2) {
            std::fprintf(stderr, "main: single room; door transitions disabled\n");
            return;
        }
        const RoomDoor& d = roomDoors[ri];
        if (!d.present) {
            std::fprintf(stderr, "main: room %s has no door piece; transitions from it disabled\n",
                         rooms[ri].name.c_str());
            return;
        }
        ico::game::RoomTransitionZone zone;
        zone.sceneId = rooms[ri].sceneId;
        zone.name = rooms[ri].name.c_str();
        zone.x = d.x;
        zone.z = d.z;
        zone.radius = 40.0f;
        zone.targetSceneId = d.targetScene;
        zone.spawnX = d.spawnX;
        zone.spawnZ = d.spawnZ;
        std::fprintf(stderr, "main: door zone %s at (%g,%g) -> scene 0x%02X\n",
                     rooms[ri].name.c_str(), d.x, d.z,
                     static_cast<unsigned>(d.targetScene));
        roomTransitions.initialize(&zone, 1,
            [&](const ico::game::RoomTransitionZone& z) {
                std::size_t target = rooms.size();
                for (std::size_t k = 0; k < rooms.size(); ++k) {
                    if (rooms[k].sceneId == z.targetSceneId) { target = k; break; }
                }
                if (target >= rooms.size()) {
                    std::fprintf(stderr,
                        "main: DOOR OPEN target scene 0x%02X unknown; aborting\n",
                        static_cast<unsigned>(z.targetSceneId));
                    return;
                }
                std::fprintf(stderr,
                    "main: DOOR OPEN %s -> %s, spawn(%g,%g)\n",
                    rooms[activeRoom].name.c_str(), rooms[target].name.c_str(),
                    z.spawnX, z.spawnZ);
                activeRoom = static_cast<u32>(target);
                const RoomFit& f2 = fits[activeRoom];
                cx = f2.cx; cy = f2.cy; cz = f2.cz;
                extent = f2.extent; dist = f2.dist;
                // sky switch
                hasSky = roomHasSky[activeRoom];
                for (int q = 0; q < 4; ++q) {
                    skyTop[q] = roomSkyTop[activeRoom][q];
                    skyHorizon[q] = roomSkyHorizon[activeRoom][q];
                }
                // collision: point the player at the new room's grid first.
                activeClip = (activeRoom < roomClips.size() && roomClips[activeRoom] &&
                              roomClips[activeRoom]->isInitialized())
                    ? roomClips[activeRoom].get() : nullptr;
                if (activeClip != nullptr) {
                    respawnPlayer(z.spawnX, z.spawnZ);
                } else {
                    playerSpawned = false;
                }
                // scene swap (kanban seam)
                sceneLoader.requestScene(z.targetSceneId);
                const bool swapped = sceneLoader.execute();
                std::fprintf(stderr,
                    "main:   transition execute: %s (scene 0x%02X, %zu host GObjs)\n",
                    swapped ? "ok" : "no-op",
                    static_cast<unsigned>(sceneLoader.currentSceneId()),
                    sceneLoader.sceneGObjCount());
                sceneLoader.attachBoundAssetsToGObjs(sceneLoader.currentSceneId());
                rebuildGObjDraws();
                // per-room strip fallback switch (no GObj composition path)
                stripBatches = buildStripBatches(
                    uvTest ? checkerTex : kNullTexture, static_cast<int>(activeRoom));
                // reinstall the door of the room we just entered
                installRoomTransitions(activeRoom);
            });
        roomTransitions.setOpenDelay(1.0f);
    };
    installRoomTransitions(activeRoom);

    auto feedKey = [&](XKeyEvent& xk, bool down) {
        const ::KeySym ks = ::XLookupKeysym(&xk, 0);
        switch (ks) {
            case XK_w:
            case XK_W:  input.setKeyState(KeyW, down); break;
            case XK_a:
            case XK_A:  input.setKeyState(KeyA, down); break;
            case XK_s:
            case XK_S:  input.setKeyState(KeyS, down); break;
            case XK_d:
            case XK_D:  input.setKeyState(KeyD, down); break;
            default: break;
        }
    };

    auto pollDebugCameraInput = [&]() {
        void* dispV = backend.getNativeDisplay();
        const unsigned long winU = backend.getNativeWindow();
        if (!dispV || !winU) return;
        ::Display* d = static_cast<::Display*>(dispV);
        ::Window w = static_cast<::Window>(winU);
        const float mouseSens = 0.007f;      // rad per pixel (orbit)
        float lastMX = 0.0f, lastMY = 0.0f;  // previous cursor pos for drag deltas
        int btnDown = 0;                     // 0 none, 1 left (orbit), 3 right (pan)
        while (::XPending(d) > 0) {
            XEvent ev;
            ::XNextEvent(d, &ev);
            if (ev.type == KeyPress) {
                feedKey(ev.xkey, true);
                const ::KeySym ks = ::XLookupKeysym(&ev.xkey, 0);
                if (ks == XK_plus || ks == XK_equal || ks == XK_KP_Add ||
                    ks == XK_z || ks == XK_Z || ks == XK_Page_Up) {
                    curDist *= 0.90f;
                } else if (ks == XK_minus || ks == XK_underscore || ks == XK_KP_Subtract ||
                           ks == XK_x || ks == XK_X || ks == XK_Page_Down) {
                    curDist /= 0.90f;
                } else if (ks == XK_Left) {
                    curAngle += 0.06f;
                } else if (ks == XK_Right) {
                    curAngle -= 0.06f;
                } else if (ks == XK_Up) {
                    curPitch = std::min(curPitch + 0.05f, 1.50f);
                } else if (ks == XK_Down) {
                    curPitch = std::max(curPitch - 0.05f, 0.02f);
                } else if (ks == XK_i || ks == XK_I || ks == XK_Home) {
                    // Pan the orbit target toward view-forward (deeper).
                    panX -= std::cos(curAngle) * 60.0f;
                    panZ -= std::sin(curAngle) * 60.0f;
                } else if (ks == XK_k || ks == XK_K || ks == XK_End) {
                    panX += std::cos(curAngle) * 60.0f;
                    panZ += std::sin(curAngle) * 60.0f;
                } else if (ks == XK_l || ks == XK_L) {
                    // Strafe right: R = cross(up, forward) = (-sin, cos).
                    panX -= std::sin(curAngle) * 60.0f;
                    panZ += std::cos(curAngle) * 60.0f;
                } else if (ks == XK_j || ks == XK_J) {
                    panX += std::sin(curAngle) * 60.0f;
                    panZ -= std::cos(curAngle) * 60.0f;
                } else if (ks == XK_f || ks == XK_F) {
                    followBoy = !followBoy;
                    std::fprintf(stderr, "main: camera %s\n",
                                 followBoy ? "following boy" : "free orbit");
                } else if (ks == XK_r || ks == XK_R) {
                    curDist = dist; curAngle = 0.0f; curPitch = 0.15f;
                    panX = 0.0f; panZ = 0.0f; followBoy = false;
                    std::fprintf(stderr, "main: camera reset\n");
                } else if (ks == XK_q || ks == XK_Q || ks == XK_Escape) {
                    exit(0);
                }
                std::fprintf(stderr, "main: cam dist=%.1f ang=%.2f rad\n", curDist, curAngle);
            } else if (ev.type == KeyRelease) {
                feedKey(ev.xkey, false);
            } else if (ev.type == ButtonPress) {
                if (ev.xbutton.button == Button4) {          // wheel up = zoom in
                    curDist *= 0.90f;
                } else if (ev.xbutton.button == Button5) {   // wheel down = zoom out
                    curDist /= 0.90f;
                } else if (ev.xbutton.button == Button1 ||
                           ev.xbutton.button == Button2 ||
                           ev.xbutton.button == Button3) {
                    btnDown = static_cast<int>(ev.xbutton.button);
                    lastMX = static_cast<float>(ev.xbutton.x);
                    lastMY = static_cast<float>(ev.xbutton.y);
                }
                if (ev.xbutton.button == Button4 || ev.xbutton.button == Button5)
                    std::fprintf(stderr, "main: cam dist=%.1f ang=%.2f rad\n", curDist, curAngle);
            } else if (ev.type == ButtonRelease) {
                if (static_cast<int>(ev.xbutton.button) == btnDown) btnDown = 0;
            } else if (ev.type == MotionNotify) {
                const float mx = static_cast<float>(ev.xmotion.x);
                const float my = static_cast<float>(ev.xmotion.y);
                const float dx = mx - lastMX, dy = my - lastMY;
                lastMX = mx; lastMY = my;
                if (btnDown == Button1) {
                    // Left drag orbits around the fitted target point.
                    curAngle -= dx * mouseSens;
                    curPitch = std::min(std::max(curPitch - dy * mouseSens, 0.02f), 1.50f);
                } else if (btnDown == Button3) {
                    // Right drag pans the target in the XZ plane; the step
                    // scales with camera distance so the surface tracks the
                    // cursor (Google-Maps style). Forward is -cos/-sin of the
                    // yaw, right is (-sin, cos).
                    const float panStep = curDist * 0.0016f;
                    panX += (dx * std::sin(curAngle) - dy * std::cos(curAngle)) * panStep;
                    panZ += (-dx * std::cos(curAngle) - dy * std::sin(curAngle)) * panStep;
                }
                if (btnDown) std::fprintf(stderr, "main: cam dist=%.1f ang=%.2f rad\n", curDist, curAngle);
            }
        }
    };
    std::fprintf(stderr, "main: controls: drag LEFT = orbit, drag RIGHT = pan, scroll = zoom, Z/+/X/- zoom, arrows orbit, IJKL pan, F follow boy, R reset, Q/Esc quit\n");
    std::fprintf(stderr, "main: WASD moves the player (collision-constrained)\n");

    // GIF command bridge (front 1): the semantic packet pipeline shares one
    // executor with the renderer. The sky backdrop and every strip batch ride
    // a single packet per frame; the executor flushes through the backend.
    GifPacketBridge bridge(backend);
    bridge.init(kPs2ScreenWidth, kPs2ScreenHeight);

    // Passo 1: draws the placeholder box a GObj commands through its
    // BoxMarker attachment (transform + half extent + color). drawIndexed
    // consumes groups of 4 indices ([A,B,C,C] degenerates to two triangles),
    // so each of the 12 box faces is emitted as a quad group with the 4th
    // index duplicated.
    auto drawBoxMarker = [&](const GObjRenderAttachment& marker) {
        const Matrix4x4& t = marker.transform;
        const float bx = t.m[12], by = t.m[13], bz = t.m[14];
        const float s = marker.halfExtent;
        const float x0 = bx - s, x1 = bx + s;
        const float y0 = by,       y1 = by + 2.0f * s;
        const float z0 = bz - s, z1 = bz + s;
        RenderVertex box[8]{};
        const float zyx[8][3] = {
            {x0, y0, z0}, {x1, y0, z0}, {x1, y1, z0}, {x0, y1, z0},
            {x0, y0, z1}, {x1, y0, z1}, {x1, y1, z1}, {x0, y1, z1},
        };
        for (int i = 0; i < 8; ++i) {
            box[i].x = zyx[i][0];
            box[i].y = zyx[i][1];
            box[i].z = zyx[i][2];
            box[i].r = marker.boxColor[0]; box[i].g = marker.boxColor[1];
            box[i].b = marker.boxColor[2]; box[i].a = marker.boxColor[3];
        }
        static const int faces[6][4] = {
            {0, 1, 2, 3},  // -z
            {5, 4, 7, 6},  // +z
            {4, 0, 3, 7},  // -x
            {1, 5, 6, 2},  // +x
            {3, 2, 6, 7},  // +y
            {4, 5, 1, 0},  // -y
        };
        uint32_t boxIdx[36]{};
        uint32_t n = 0;
        for (int f = 0; f < 6; ++f) {
            const int a = faces[f][0], b = faces[f][1];
            const int c = faces[f][2], d = faces[f][3];
            // two triangles -> two quad groups, each [A,B,C,C]
            boxIdx[n++] = static_cast<uint32_t>(a);
            boxIdx[n++] = static_cast<uint32_t>(b);
            boxIdx[n++] = static_cast<uint32_t>(c);
            boxIdx[n++] = static_cast<uint32_t>(c);
            boxIdx[n++] = static_cast<uint32_t>(a);
            boxIdx[n++] = static_cast<uint32_t>(c);
            boxIdx[n++] = static_cast<uint32_t>(d);
            boxIdx[n++] = static_cast<uint32_t>(d);
        }
        backend.drawIndexed(GSPrimitive::Triangle, RenderList::Opaque,
                            boxIdx, n, box, 0,
                            kNullTexture, marker.boxColor[0], marker.boxColor[1],
                            marker.boxColor[2], marker.boxColor[3]);
    };

    auto lastFrameTp = std::chrono::steady_clock::now();
    for (u32 f = 0; frames == 0 || f < frames; ++f) {
        const auto nowTp = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(nowTp - lastFrameTp).count();
        lastFrameTp = nowTp;
        if (dt <= 0.0f || dt > 0.25f) dt = 1.0f / 60.0f;
        backend.beginFrame();

        backend.clear(skyHorizon[0], skyHorizon[1], skyHorizon[2], 255);

        // Top-of-frame key snapshot must precede the X11 pump so edge
        // queries compare against last frame's state (see input_test).
        input.update();
        pollDebugCameraInput();

        if (playerSpawned) {
        // Player move vector from WASD (world units per frame; PS2 units are
        // cm-scale, so 25 units/frame is a comfortable walk pace relative to
        // the ~200+ unit room). Y is handled by the ClipBridge snap; the
        // BoyController process consumes the vector this frame. The vector is
        // the Phase A input feeding the boy_hB speed-tier discriminator
        // (single key < kRunThreshold → walk 15.0, W+D diagonal → run 30.0).
        const float step = 25.0f;
        float moveDx = 0.0f, moveDz = 0.0f;
        if (input.isKeyDown(KeyW)) moveDz += step;
        if (input.isKeyDown(KeyS)) moveDz -= step;
        if (input.isKeyDown(KeyA)) moveDx -= step;
        if (input.isKeyDown(KeyD)) moveDx += step;
        if (moveDx != 0.0f || moveDz != 0.0f) {
            const float camAng = curAngle;
            if (followBoy) {
                // Camera-relative movement: W walks along the view direction
                // (forward = -cos/-sin of yaw), D strafes camera-right
                // (right = (-sin, cos)). Keeps the boy controllable from the
                // third-person camera regardless of the orbit angle.
                const float fx = -std::cos(camAng), fz = -std::sin(camAng);
                const float rx = -std::sin(camAng), rz = std::cos(camAng);
                player.setMove(moveDz * fx + moveDx * rx,
                               moveDz * fz + moveDx * rz);
            } else {
                player.setMove(moveDx, moveDz);
            }
        }
        player.update();
        }
        const float markerX = playerSpawned ? player.x() : cx + panX;
        const float markerY = playerSpawned ? player.y() : cy;
        const float markerZ = playerSpawned ? player.z() : cz + panZ;

        // Rev.168 (PORT): door-triggered room transition driver. Feeds the
        // boy's world position and advances the door state machine; the
        // callback (installed above) performs the requestScene/execute swap.
        roomTransitions.setBoyPosition(markerX, markerZ);
        roomTransitions.update(dt);

        // Passo 1: BoyController drives the GObj's BoxMarker transform (world
        // placement read by the renderer below).
        if (boyHandle != kNullGObjHandle) {
            ico::engine::GObjRenderAttachment* m = boyStore.find(boyHandle);
            if (m != nullptr && m->kind == ico::engine::GObjAttachmentKind::BoxMarker) {
                m->transform = Matrix4x4::translation(markerX, markerY, markerZ);
            }
        }

        const float ang = curAngle;
        const float pitch = followBoy ? 0.08f : curPitch;
        // Target: follow mode tracks the boy (third-person); free orbit uses
        // the fitted center plus the pan offset.
        const float tgtX = followBoy ? markerX : (cx + panX);
        const float tgtZ = followBoy ? markerZ : (cz + panZ);
        const float tgtY = followBoy
            ? (markerY + (hasBoyMesh ? 60.0f * kBoyScale : 40.0f))
            : cy;
        // Rev.168 follow cam: a full-room fit (dist ~2300 in scene mode) makes
        // the ~1.5 m boy a sub-100-px blob. In follow mode clamp to a close
        // third-person distance; scroll still zooms it further if needed.
        const float followDist = followBoy ? std::min(curDist, 480.0f) : curDist;
        const float distXZ = followDist * std::cos(pitch);
        const float eyeY = tgtY + followDist * std::sin(pitch);
        const float eyeX = tgtX + distXZ * std::cos(ang);
        const float eyeZ = tgtZ + distXZ * std::sin(ang);
        const float eye[3] = { eyeX, eyeY, eyeZ };
        const Matrix4x4 proj2 = Matrix4x4::perspective(70.0f, 640.0f / 448.0f,
                                                       followDist * 0.01f,
                                                       followDist * 10.0f);
        const float tgt[3] = { tgtX, tgtY, tgtZ };
        const float up[3] = { 0.0f, 1.0f, 0.0f };
        const Matrix4x4 view = Matrix4x4::lookAt(eye, tgt, up);
        const Matrix4x4 model = Matrix4x4::identity();
        backend.setMatrices(proj2, view, model);

        // Unified scene packet (fronts 1+3): sky backdrop + every strip batch
        // accumulate in ONE command buffer, executed through the semantic
        // GIF pipeline — the same path a real GIF DMA upload feeds. Since
        // synthesizeTriangleStrips() runs at load, every piece carries strip
        // topology, so useStrips is always true here and the [A,B,C,C] index
        // duplication never reaches the driver.
        if (useStrips) {
            bridge.startPacketPri(0);
            {
                // Sky backdrop: depth Always / write off within the same
                // packet (setZWrite/setZTest cannot express this pair). Drawn
                // for every room: sampled sky.tm2 gradient when present, host
                // daylight placeholder otherwise (Rev.169).
                bridge.setDepthState(GSDepthTest::Always, false);
                bridge.drawSkyGradient(skyTop, skyHorizon);
                bridge.setDepthState(GSDepthTest::Less, true);
            }
            if (!gobjDraws.empty()) {
                // Rev.155 (Passo 1): render driven by the ACTIVE isysGObj
                // lists. Each GObj found in the attachment store draws the
                // strip batches of the meshes it owns; GObjs without a visual
                // composition are skipped. This replaces the global room
                // strips below whenever a GObj composition exists.
                for (u8 listId = 0; listId < kPrimaryListCount; ++listId) {
                    GObj* g = gobjRuntime.head(listId);
                    while (g != nullptr) {
                        const GObjHandle h = gobjRuntime.pool().handleOf(*g);
                        auto it = gobjDrawByHandle.find(h);
                        if (it != gobjDrawByHandle.end()) {
                            const GObjDraw& draw = gobjDraws[it->second];
                            for (const auto& sb : draw.batches) {
                                if (sb.sourceStrips == 0) continue;
                                bridge.drawStrips(RenderList::Opaque,
                                                  sb.vertices.data(),
                                                  static_cast<u32>(sb.vertices.size()),
                                                  sb.firsts.data(), sb.counts.data(),
                                                  static_cast<u32>(sb.counts.size()),
                                                  sb.texture, 255, 255, 255, 255);
                            }
                        }
                        const GObjHandle nextH = g->next;
                        g = (nextH != kNullGObjHandle)
                            ? gobjRuntime.pool().get(nextH) : nullptr;
                    }
                }
            } else {
                for (const auto& sb : stripBatches) {
                    if (sb.sourceStrips == 0) continue;
                    bridge.drawStrips(RenderList::Opaque,
                                      sb.vertices.data(),
                                      static_cast<u32>(sb.vertices.size()),
                                      sb.firsts.data(), sb.counts.data(),
                                      static_cast<u32>(sb.counts.size()),
                                      sb.texture, 255, 255, 255, 255);
                }
            }

            // Rev.161: the boy's own GObj commands the reconstructed character
            // mesh (multi-OBJH .p2c) instead of the placeholder box. Strips
            // are re-emitted each frame transformed by the BoyController's
            // world placement (markerX/Y/Z), scaled into room units.
            if (hasBoyMesh) {
                int maxBoyMat = 0;
                for (const auto& st : boyMesh.strips)
                    if ((int)st.material > maxBoyMat) maxBoyMat = st.material;
                for (int f = 0; f <= maxBoyMat; ++f) {
                    std::vector<RenderVertex> bv;
                    std::vector<u32> firsts;
                    std::vector<u32> counts;
                    for (const auto& st : boyMesh.strips) {
                        if ((int)st.material != f) continue;
                        const u32 n = static_cast<u32>(st.spine.size());
                        if (n < 3) continue;
                        firsts.push_back(static_cast<u32>(bv.size()));
                        counts.push_back(n);
                        bv.reserve(bv.size() + n);
                        for (u32 k = 0; k < n; ++k) {
                            const u32 vi = st.spine[k];
                            RenderVertex v{};
                            v.x = boyMesh.positions[vi * 3 + 0] * kBoyScale + markerX;
                            v.y = boyMesh.positions[vi * 3 + 1] * kBoyScale +
                                  markerY + kBoyFootOffset;
                            v.z = boyMesh.positions[vi * 3 + 2] * kBoyScale + markerZ;
                            if (st.uvs.size() >= n * 2) {
                                v.u = st.uvs[k * 2 + 0];
                                v.v = st.uvs[k * 2 + 1];
                            }
                            v.r = 255; v.g = 255; v.b = 255; v.a = 255;
                            bv.push_back(v);
                        }
                    }
                    if (bv.empty()) continue;
                    const TextureHandle tex = uvTest
                        ? checkerTex
                        : ((f < (int)boyTexByMat.size()) ? boyTexByMat[f] : kNullTexture);
                    bridge.drawStrips(RenderList::Opaque, bv.data(),
                                      static_cast<u32>(bv.size()),
                                      firsts.data(), counts.data(),
                                      static_cast<u32>(counts.size()),
                                      tex, 255, 255, 255, 255);
                }
            }
            bridge.endPacket();
        } else {
            // Flat fallback (pre-front-3): direct backend calls, unchanged.
            backend.setDepthTest(GSDepthTest::Always, false);
            backend.drawSkyGradient(skyTop, skyHorizon);
            backend.setDepthTest(GSDepthTest::Less, true);
            for (const auto& tb : textureBatches) {
                if (tb.indices.empty()) continue;
                TextureHandle tex = (tb.texture != kNullTexture) ? tb.texture
                                    : (uvTest ? checkerTex : kNullTexture);
                backend.bindTexture(tex, 0);
                backend.drawIndexed(GSPrimitive::Triangle, RenderList::Opaque,
                                    tb.indices.data(),
                                    static_cast<u32>(tb.indices.size()),
                                    tb.vertices.data(), 0,
                                    tb.texture, 255, 255, 255, 255);
            }
        }

        // Player visual (Rev.161): with the reconstructed character mesh the
        // boy is drawn inside the scene packet above; the placeholder box only
        // falls back when no boy model is available.
        if (!hasBoyMesh) {
            const GObjRenderAttachment* m = boyStore.find(boyHandle);
            if (m != nullptr && m->kind == ico::engine::GObjAttachmentKind::BoxMarker &&
                m->active) {
                drawBoxMarker(*m);
            }
        }

        backend.endFrame();
        backend.present();
        std::this_thread::sleep_for(frameTime);
    }

    if (shotPath != nullptr && backend.captureFrameRGB(tmpFrame.data(), kPs2ScreenWidth, kPs2ScreenHeight)) {
        std::FILE* pf = std::fopen(shotPath, "wb");
        if (pf) {
            std::fprintf(pf, "P6\n%u %u\n255\n", kPs2ScreenWidth, kPs2ScreenHeight);
            std::fwrite(tmpFrame.data(), 1, tmpFrame.size(), pf);
            std::fclose(pf);
            std::fprintf(stderr, "main: wrote %s\n", shotPath);
        }
    }

    if (checkerTex != kNullTexture) backend.destroyTexture(checkerTex);
    for (const auto& kv : texCache)
        if (kv.second != kNullTexture) backend.destroyTexture(kv.second);
    player.shutdown();
    gobjRuntime.shutdown();
    for (const auto& rc : roomClips)
        if (rc) rc->shutdown();
    backend.shutdown();
    return 0;
}

// Single-room wrapper (kept for --p2o/--scene viewers and the mesh demo).
int runSceneDemo(const std::vector<std::string>& piecePaths,
                 const std::string& texDir,
                 u32 frames, const char* shotPath, bool uvTest,
                 float camAngleRad, bool fitMacro,
                 const ico::engine::SceneAssetStore* store) {
    std::vector<DemoRoom> rooms;
    rooms.push_back(DemoRoom{});
    DemoRoom& room = rooms.back();
    room.name = "scene";
    room.piecePaths = piecePaths;
    room.texDir = texDir;
    room.store = store;
    room.sceneId = 0x0Fu;
    return runMultiRoomDemo(rooms, 0u, frames, shotPath, uvTest,
                            camAngleRad, fitMacro);
}

int runMeshDemo(const char* p2oPath, const std::string& texDir,
                u32 frames, const char* shotPath, bool uvTest, bool matTest,
                const char* tm2Path) {
    return runSceneDemo({ p2oPath }, texDir, frames, shotPath, uvTest, -1.0f, false);
}

int runOpenGLDemo(int argc, char* argv[]) {
    using namespace ico::engine;

    u32 frames = 180;
    const char* tm2Path = nullptr;
    const char* p2oPath = nullptr;
    const char* shotPath = nullptr;
    const char* sceneDir = nullptr;
    const char* roomName = nullptr;
    const char* pairName = nullptr;
    const char* texDir = nullptr;
    float teleport[2] = { 0.0f, 0.0f };
    bool hasTeleport = false;
    bool uvTest = false;
    bool matTest = false;
    bool fitMacro = false;
    float camAngleRad = -1.0f;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--frames") == 0 && i + 1 < argc) {
            frames = static_cast<u32>(std::atoi(argv[i + 1]));
        } else if (std::strcmp(argv[i], "--tm2") == 0 && i + 1 < argc) {
            tm2Path = argv[i + 1];
        } else if (std::strcmp(argv[i], "--p2o") == 0 && i + 1 < argc) {
            p2oPath = argv[i + 1];
        } else if (std::strcmp(argv[i], "--shot") == 0 && i + 1 < argc) {
            shotPath = argv[i + 1];
        } else if (std::strcmp(argv[i], "--scene") == 0 && i + 1 < argc) {
            sceneDir = argv[i + 1];
        } else if (std::strcmp(argv[i], "--room") == 0 && i + 1 < argc) {
            roomName = argv[i + 1];
        } else if (std::strcmp(argv[i], "--pair") == 0 && i + 1 < argc) {
            pairName = argv[i + 1];
        } else if (std::strcmp(argv[i], "--teleport") == 0 && i + 1 < argc) {
            const char* tp = argv[i + 1];
            char* end = nullptr;
            teleport[0] = std::strtof(tp, &end);
            if (end != nullptr && *end == ',') {
                teleport[1] = std::strtof(end + 1, &end);
                hasTeleport = true;
            } else {
                std::fprintf(stderr, "main: --teleport expects 'x,z'\n");
            }
        } else if (std::strcmp(argv[i], "--tex-dir") == 0 && i + 1 < argc) {
            texDir = argv[i + 1];
        } else if (std::strcmp(argv[i], "--cam-angle") == 0 && i + 1 < argc) {
            camAngleRad = static_cast<float>(std::atof(argv[i + 1])) * 3.14159265f / 180.0f;
        } else if (std::strcmp(argv[i], "--uv-test") == 0) {
            uvTest = true;
        } else if (std::strcmp(argv[i], "--mat-test") == 0) {
            matTest = true;
        } else if (std::strcmp(argv[i], "--fit-macro") == 0) {
            fitMacro = true;
        }
    }

    // Multi-piece scene mode: --scene DIR loads every *.p2o in DIR and
    // renders all of them together, each with its own per-material textures
    // resolved by name from the piece's material table. When a scene manifest
    // (stgst00a.manifest) is present next to the pieces, the mesh list is
    // taken from it via SceneAssetStore instead of the baked-in kOrder below,
    // so the composition is data-driven like the semantic loader expects.
    if (sceneDir != nullptr || roomName != nullptr) {
        // Room/stage mode frames the FULL scene (playable p1 + real p2
        // backdrop: sea/coast/horizon), never just the p1 interior.
        if (roomName != nullptr) fitMacro = true;
        std::vector<std::string> pieces;
        const char* sceneCandidates[] = {
            "assets/scene/pieces/", "../native/assets/scene/pieces/", nullptr
        };
        std::string dir;
        if (roomName != nullptr) {
            // Room mode: scene assets live per-room under assets/scene/rooms/<room>/.
            const char* roomCandidates[] = {
                "assets/scene/rooms/", "../native/assets/scene/rooms/", nullptr
            };
            std::string roomDir;
            for (int c = 0; roomCandidates[c] != nullptr; ++c) {
                std::string cand = std::string(roomCandidates[c]) + roomName + "/";
                std::ifstream f((cand + roomName + ".manifest").c_str());
                if (f.good()) { roomDir = cand; break; }
            }
            if (roomDir.empty()) {
                std::fprintf(stderr, "main: --room '%s': no manifest found under "
                             "assets/scene/rooms/\n", roomName);
                return 1;
            }
            dir = roomDir;
        } else {
            dir = sceneDir;
            if (std::strchr(sceneDir, '/') == nullptr) {
                for (int c = 0; sceneCandidates[c] != nullptr; ++c) {
                    std::ifstream f(std::string(sceneCandidates[c]) + "169_door.p2o");
                    if (f.good()) { dir = sceneCandidates[c]; break; }
                }
            } else if (!dir.empty() && dir.back() != '/') {
                dir += '/';
            }
        }
        std::string path = dir + "169_door.p2o";
        std::ifstream test(path);
        if (!test.good() && roomName == nullptr) {
            std::fprintf(stderr, "main: --scene dir '%s' has no 169_door.p2o\n", dir.c_str());
            return 1;
        }

        // Data-driven composition (KanbanSceneLoader seam): prefer the manifest.
        ico::engine::SceneAssetStore store;
        const char* manifestCandidates[] = {
            "assets/scene/stgst00a.manifest",
            "native/assets/scene/stgst00a.manifest",
            "../native/assets/scene/stgst00a.manifest",
            nullptr
        };
        std::string manifestPath;
        if (roomName != nullptr) {
            manifestPath = dir + roomName + ".manifest";
            std::ifstream f(manifestPath.c_str());
            if (!f.good()) {
                std::fprintf(stderr, "main: room manifest %s missing\n",
                             manifestPath.c_str());
                return 1;
            }
        } else {
            for (int c = 0; manifestCandidates[c] != nullptr; ++c) {
                std::ifstream f(manifestCandidates[c]);
                if (f.good()) { manifestPath = manifestCandidates[c]; break; }
            }
        }
        if (!manifestPath.empty() && store.parse(manifestPath.c_str())) {
            // Data-driven composition through the KanbanSceneLoader seam
            // (front 2): a composition runtime drives the semantic loader the
            // same way the game would, and boundAsset() supplies the piece
            // list. This is the same API runSceneDemo consumes at load time.
            IsysGObj compositionRuntime;
            if (!compositionRuntime.initialize(0x40, 0x40)) {
                std::fprintf(stderr, "main: composition runtime failed to initialize\n");
                return 1;
            }
            KanbanSceneLoader compositionLoader;
            if (!compositionLoader.initialize(compositionRuntime) ||
                !compositionLoader.bindSceneAssets(store, 0x0Fu)) {
                std::fprintf(stderr, "main: composition loader bind of scene 0x0F failed\n");
                return 1;
            }
            const u32 exhibitionScene = 0x0Fu;
            const std::size_t count = compositionLoader.boundAssetCount(exhibitionScene);
            std::fprintf(stderr, "main: scene composition from manifest %s "
                                 "(scene 0x0F via KanbanSceneLoader, %zu pieces)\n",
                         manifestPath.c_str(), count);
            for (std::size_t i = 0; i < count; ++i) {
                const ico::engine::SceneAssetEntry* entry =
                    compositionLoader.boundAsset(exhibitionScene, i);
                if (entry != nullptr) {
                    std::ifstream f(entry->meshPath.c_str());
                    if (f.good()) pieces.push_back(entry->meshPath);
                }
            }
            if (pieces.empty()) {
                std::fprintf(stderr, "main: manifest resolved 0 pieces; aborting\n");
                return 1;
            }
        } else {
            // Deterministic order: door, p1, p2, torches, windows, bridge.
            static const char* kOrder[] = {
            "169_door.p2o", "170_st00a_p1.p2o", "171_st00a_p2.p2o",
            "172_st00a_torch1_add.p2o", "173_st00a_torch2_add.p2o",
            "174_st00a_torch3_add.p2o", "175_st00a_window_flare.p2o",
            "176_st00a_window_glow.p2o", "179_08_00_brdg.p2o",
            // 0str decorative structures are already in world space; each pair
            // (even/odd) is byte-identical, keep the first of each pair.
            "128_0str01_s2.p2o", "130_0str02_s2.p2o", "132_0str03_s2.p2o",
            "134_0str04_s2.p2o", "136_0str05_s2.p2o", "138_0str06_s2.p2o",
            "141_0str07_s2.p2o", "144_0str08_s2.p2o", "147_0str09_s2.p2o",
            "149_0str10_s2.p2o", "151_0str11_s2.p2o", "153_0str12_s2.p2o",
            "155_0str13_s2.p2o", "157_0str14_s2.p2o", "160_0str15_s2.p2o",
            "163_0str16_s2.p2o",
            "140_0str06_s2_sd.p2o", "146_0str08_s2_sd.p2o",
            "162_0str15_s2_sd.p2o",
            nullptr
        };
        for (int c = 0; kOrder[c] != nullptr; ++c) {
            std::string fp = dir + kOrder[c];
            std::ifstream f(fp.c_str());
            if (f.good()) pieces.push_back(fp);
        }
        }
        const std::string td = texDir ? texDir : ((dir + "texture/"));
        // Normalize: if pieces were taken from "<dir>pieces/", textures sit in
        // a sibling "<dir>texture/" rather than "<dir>pieces/texture/".
        std::string texDirResolved = td;
        {
            std::string alt = td;
            const std::string pc("pieces/");
            const size_t pos = alt.rfind(pc);
            if (pos != std::string::npos) {
                alt.replace(pos, pc.size(), "");
                std::ifstream t(alt + "st0_a.tm2");
                if (t.good()) texDirResolved = alt;
            }
            if (roomName != nullptr) {
                // Room mode: any .tm2 present proves the texture dir resolved.
                std::ifstream any(texDirResolved + "st0_a.tm2");
                if (!any.good()) {
                    std::string probe = texDirResolved + "st0_a.tm2";
                    // The room's own first-referenced texture is room-specific;
                    // accept the dir if it contains any .tm2 at all.
                    bool hasAny = false;
                    {
                        namespace fs = std::filesystem;
                        std::error_code ec;
                        fs::directory_iterator it(texDirResolved, ec);
                        if (!ec) {
                            for (; it != fs::directory_iterator(); ++it) {
                                if (it->path().extension() == ".tm2") { hasAny = true; break; }
                            }
                        }
                    }
                    if (!hasAny) {
                        std::fprintf(stderr, "main: room texture dir '%s' has no .tm2\n",
                                     texDirResolved.c_str());
                        return 1;
                    }
                }
            } else {
                std::ifstream t(texDirResolved + "st0_a.tm2");
                if (!t.good()) {
                    std::fprintf(stderr, "main: --scene texture dir '%s' has no st0_a.tm2\n",
                                 texDirResolved.c_str());
                    return 1;
                }
            }
        }
        // Rev.170 (PORT -- pair): two-room door demo. The primary room (from
        // --room/--scene) is kept under scene 0x0F; --pair <room> parses the
        // companion manifest into the SAME store under scene 0x2B
        // (SceneAssetStore::parseRoom), so KanbanSceneLoader holds BOTH rooms'
        // resident sets and requestScene(0x2B) swaps real geometry (different
        // pieces/camera/ClipBridge/sky) instead of reloading the same room.
        std::vector<DemoRoom> rooms;
        rooms.push_back(DemoRoom{});
        DemoRoom& primary = rooms.back();
        primary.name = roomName ? roomName : "scene";
        primary.piecePaths = pieces;
        primary.texDir = texDirResolved;
        primary.store = manifestPath.empty() ? nullptr : &store;
        primary.sceneId = 0x0Fu;

        if (pairName != nullptr) {
            const char* pairCandidates[] = {
                "assets/scene/rooms/", "../native/assets/scene/rooms/", nullptr
            };
            std::string pairDir;
            for (int c = 0; pairCandidates[c] != nullptr; ++c) {
                std::string cand = std::string(pairCandidates[c]) + pairName + "/";
                std::ifstream f((cand + pairName + ".manifest").c_str());
                if (f.good()) { pairDir = cand; break; }
            }
            if (pairDir.empty()) {
                std::fprintf(stderr, "main: --pair '%s': no manifest under assets/scene/rooms/\n",
                             pairName);
                return 1;
            }
            const std::string pairManifest = pairDir + pairName + ".manifest";
            const u32 pairScene = 0x2Bu;
            if (!store.parseRoom(pairManifest.c_str(), pairScene)) {
                std::fprintf(stderr, "main: --pair '%s': parseRoom(scene 0x2B) failed\n",
                             pairName);
                return 1;
            }
            rooms.push_back(DemoRoom{});
            DemoRoom& companion = rooms.back();
            companion.name = pairName;
            companion.texDir = pairDir + "texture/";
            companion.store = &store;
            companion.sceneId = pairScene;
            const std::size_t cnt = store.sceneAssetCount(pairScene);
            std::fprintf(stderr, "main: pair room %s under scene 0x%02X: %zu assets\n",
                         pairName, static_cast<unsigned>(pairScene), cnt);
            for (std::size_t i = 0; i < cnt; ++i) {
                const SceneAssetEntry* e = store.sceneAsset(pairScene, i);
                if (e == nullptr) continue;
                std::ifstream f(e->meshPath.c_str());
                if (f.good()) companion.piecePaths.push_back(e->meshPath);
            }
            if (companion.piecePaths.empty()) {
                std::fprintf(stderr, "main: --pair resolved 0 pieces; ignoring pair\n");
                rooms.pop_back();
            }
        }

        return runMultiRoomDemo(rooms, 0u, frames, shotPath, uvTest,
                                camAngleRad, fitMacro,
                                hasTeleport ? teleport : nullptr);
    }

    // Auto-discover .p2o mesh in native/assets/ if not provided.
    if (p2oPath == nullptr) {
        const char* candidates[] = {
            "../native/assets/170_st00a_p1.p2o",
            "native/assets/170_st00a_p1.p2o",
            "assets/170_st00a_p1.p2o",
            "assets/scene/pieces/170_st00a_p1.p2o",
            nullptr
        };
        for (int c = 0; candidates[c] != nullptr; ++c) {
            std::ifstream f(candidates[c]);
            if (f.good()) { p2oPath = candidates[c]; break; }
        }
    }

    if (p2oPath != nullptr) {
        // Resolve textures from --tex-dir when given; otherwise default to a
        // directory that actually contains the piece (assets/ mirrors the
        // extracted .tm2), so `--p2o assets/170_st00a_p1.p2o` finds e.g.
        // assets/st0_a.tm2 instead of st0_a.tm2 in the CWD.
        std::string texDirResolved = texDir ? texDir : "";
        if (texDirResolved.empty()) {
            const std::string p2oStr = p2oPath;
            const std::size_t slash = p2oStr.find_last_of('/');
            if (slash != std::string::npos)
                texDirResolved = p2oStr.substr(0, slash);
        }
        return runMeshDemo(p2oPath, texDirResolved, frames, shotPath, uvTest,
                           matTest, tm2Path);
    }

    // Auto-discover .tm2 texture in native/assets/ if not provided.
    if (tm2Path == nullptr) {
        const char* tcandidates[] = {
            "../native/assets/st00a.tm2",
            "native/assets/st00a.tm2",
            "assets/st00a.tm2",
            nullptr
        };
        for (int c = 0; tcandidates[c] != nullptr; ++c) {
            std::ifstream f(tcandidates[c]);
            if (f.good()) { tm2Path = tcandidates[c]; break; }
        }
    }

    OpenGLBackend backend;
    if (!backend.initialize(kPs2ScreenWidth, kPs2ScreenHeight)) {
        std::fprintf(stderr, "main: OpenGL backend failed to initialize\n");
        return 1;
    }

    GifPacketBridge bridge(backend);
    bridge.init(kPs2ScreenWidth, kPs2ScreenHeight);

    // Compensate the renderer's (x*sx+tx, y*sy+ty) pre-transform so pixel
    // coordinates in [0,640]x[0,448] map to full-screen NDC [-1,1].
    // See OpenGLBackend::drawSprite.
    Matrix4x4 proj = Matrix4x4::ortho(-0.5f, 1.5f, -0.5f, 1.5f, -1.0f, 1.0f);
    backend.setMatrices(proj, Matrix4x4::identity(), Matrix4x4::identity());

    // Real TM2 texture (from a PAL .DF container) rendered as a textured
    // sprite; falls back to the 1x1 white quad demo when --tm2 is absent.
    TextureHandle realTex = kNullTexture;
    u32 texW = 0, texH = 0;
    if (tm2Path) {
        std::ifstream f(tm2Path, std::ios::binary);
        if (f) {
            f.seekg(0, std::ios::end);
            const std::streamoff sz = f.tellg();
            f.seekg(0, std::ios::beg);
            std::vector<u8> buf(static_cast<size_t>(sz));
            f.read(reinterpret_cast<char*>(buf.data()), sz);
            Tm2File file{};
            if (Tm2Parser::parse(buf.data(), static_cast<u32>(buf.size()), file) &&
                !file.images.empty()) {
                Tm2Texture tex{};
                if (Tm2Converter::convertImage(file.images[0], tex) &&
                    !tex.rgbaData.empty()) {
                    TextureDesc desc{};
                    desc.width = tex.width;
                    desc.height = tex.height;
                    desc.format = TextureFormat::PSMCT32;
                    desc.data = tex.rgbaData.data();
                    desc.dataSize = static_cast<u32>(tex.rgbaData.size());
                    desc.generateMipmaps = false;
                    realTex = backend.createTexture(desc);
                    texW = tex.width;
                    texH = tex.height;
                    std::fprintf(stderr, "main: loaded TM2 %s (%ux%u)\n",
                                 tm2Path, texW, texH);
                }
            }
        }
        if (realTex == kNullTexture) {
            std::fprintf(stderr, "main: failed to load TM2 %s\n", tm2Path);
        }
    }

    // 1x1 white texture so the GIF pipeline can bind a real sampler and the
    // vertex colors show. TextureHandle = tbp0 in this model.
    const u8 whitePixel[4] = { 255, 255, 255, 255 };
    TextureDesc whiteDesc{};
    whiteDesc.width = 1;
    whiteDesc.height = 1;
    whiteDesc.format = TextureFormat::PSMCT32;
    whiteDesc.data = whitePixel;
    whiteDesc.dataSize = sizeof(whitePixel);
    whiteDesc.generateMipmaps = false;
    TextureHandle whiteTex = backend.createTexture(whiteDesc);

    const auto frameTime = std::chrono::milliseconds(16);
    for (u32 f = 0; f < frames; ++f) {
        backend.beginFrame();
        backend.clear(24, 28, 36, 255);

        bridge.startPacketPri(0);
        if (realTex != kNullTexture) {
            bridge.setGsReg(kGsAddrTEX0_1, realTex);
            // Full-screen fit with aspect preserved, animated subtly.
            const float scale = 1.0f + 0.05f * sinf(static_cast<float>(f) / 30.0f);
            const float w = static_cast<float>(texW) * scale;
            const float h = static_cast<float>(texH) * scale;
            const float cx = 320.0f;
            const float cy = 224.0f;
            bridge.sprite(cx - w / 2.0f, cy - h / 2.0f, w, h,
                          0.0f, 0.0f, 1.0f, 1.0f, 255, 255, 255, 255);
        } else {
            if (whiteTex != kNullTexture) {
                bridge.setGsReg(kGsAddrTEX0_1, whiteTex);
            }
            for (u32 i = 0; i < 3; ++i) {
                const float t = static_cast<float>(f) / 60.0f + static_cast<float>(i) * 2.1f;
                const float cx = 320.0f + 150.0f * sinf(t);
                const float cy = 224.0f + 110.0f * cosf(t * 0.9f);
                static const u8 color[3][3] = {
                    { 150, 200,  90 },
                    { 240, 170,  90 },
                    { 120, 170, 240 },
                };
                bridge.sprite(cx - 45.0f, cy - 35.0f, 90.0f, 70.0f,
                              0.0f, 0.0f, 1.0f, 1.0f,
                              color[i][0], color[i][1], color[i][2], 255);
            }
        }
        bridge.flush();

        backend.endFrame();
        backend.present();
        std::this_thread::sleep_for(frameTime);
    }

    if (realTex != kNullTexture) {
        backend.destroyTexture(realTex);
    }
    backend.destroyTexture(whiteTex);
    backend.shutdown();
    return 0;
}
#endif

} // namespace

int main(int argc, char* argv[]) {
    std::fprintf(stderr, "ICO Native Runtime Prototype\n");
    Logger::setLevel(Logger::Level::Info);

#ifdef ICO_HAS_OPENGL
    return runOpenGLDemo(argc, argv);
#else
    (void)argc;
    (void)argv;

    IcoRuntime runtime;
    if (!runtime.initialize()) {
        Logger::error("main", "Failed to initialize runtime");
        return 1;
    }

    runtime.getGameLoop().setUpdateCallback([&runtime](u32 frame) -> bool {
        Logger::info("main", "[frame %u] tick", frame);
        return true;
    });

    runtime.getGameLoop().run(3);

    runtime.shutdown();

    std::fprintf(stderr, "[shutdown] clean exit\n");
    return 0;
#endif
}