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
#include "engine/SceneAssetStore.h"
#include "engine/Tm2Converter.h"
#include "engine/Tm2Format.h"
#include "game/KanbanSceneLoader.h"
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <thread>
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

// Per-piece scene render: draws every piece every frame, binding the texture
// that the piece's material name table maps to material index f.
int runSceneDemo(const std::vector<std::string>& piecePaths,
                 const std::string& texDir,
                 u32 frames, const char* shotPath, bool uvTest,
                 float camAngleRad, bool fitMacro) {
    using namespace ico::engine;

    // Load all pieces.
    std::vector<ScenePiece> pieces;
    for (const auto& path : piecePaths) {
        ScenePiece sp;
        sp.name = path;
        if (!loadPs2oMeshFromFile(path.c_str(), sp.mesh)) {
            std::fprintf(stderr, "main: failed to load PS2O mesh %s\n", path.c_str());
            continue;
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
    if (pieces.empty()) {
        std::fprintf(stderr, "main: no pieces loaded\n");
        return 1;
    }

    // Camera fit target. Default orbits the room piece (p1). With --fit-macro
    // the camera centers the union bbox of ALL loaded pieces, so the whole
    // stage (p2 backdrop included) is framed from afar.
    const ScenePiece* fitPiece = nullptr;
    if (!fitMacro) {
        fitPiece = &pieces.front();
        for (const auto& sp : pieces) {
            if (sp.name.find("170_st00a_p1") != std::string::npos ||
                sp.name.find("_p1.") != std::string::npos) {
                fitPiece = &sp;
                break;
            }
        }
    }
    float minX = 1e30f, maxX = -1e30f;
    float minY = 1e30f, maxY = -1e30f;
    float minZ = 1e30f, maxZ = -1e30f;
    if (fitMacro) {
        for (const auto& sp : pieces) {
            const auto& m = sp.mesh;
            for (uint32_t i = 0; i < m.triangles.size(); ++i) {
                const uint32_t vi = m.triangles[i];
                minX = std::min(minX, m.positions[vi * 3 + 0]);
                maxX = std::max(maxX, m.positions[vi * 3 + 0]);
                minY = std::min(minY, m.positions[vi * 3 + 1]);
                maxY = std::max(maxY, m.positions[vi * 3 + 1]);
                minZ = std::min(minZ, m.positions[vi * 3 + 2]);
                maxZ = std::max(maxZ, m.positions[vi * 3 + 2]);
            }
        }
    } else {
        const auto& fitMesh = fitPiece->mesh;
        for (uint32_t i = 0; i < fitMesh.triangles.size(); ++i) {
            const uint32_t vi = fitMesh.triangles[i];
            minX = std::min(minX, fitMesh.positions[vi * 3 + 0]);
            maxX = std::max(maxX, fitMesh.positions[vi * 3 + 0]);
            minY = std::min(minY, fitMesh.positions[vi * 3 + 1]);
            maxY = std::max(maxY, fitMesh.positions[vi * 3 + 1]);
            minZ = std::min(minZ, fitMesh.positions[vi * 3 + 2]);
            maxZ = std::max(maxZ, fitMesh.positions[vi * 3 + 2]);
        }
    }
    const float cx = (minX + maxX) * 0.5f;
    const float cy = (minY + maxY) * 0.5f;
    const float cz = (minZ + maxZ) * 0.5f;
    const float extent = std::max({maxX - minX, maxY - minY, maxZ - minZ, 1.0f});
    const float dist = extent * 1.15f;
    std::fprintf(stderr, "main: camera fit %s: cx=%g cy=%g cz=%g extent=%g dist=%g\n",
                 fitMacro ? "macro(union)" : "p1(room)",
                 cx, cy, cz, extent, dist);

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
    auto texForName = [&](const std::string& name) -> TextureHandle {
        for (const auto& kv : texCache) if (kv.first == name) return kv.second;
        std::string path = texDir.empty() ? (name + ".tm2") : (texDir + "/" + name + ".tm2");
        TextureHandle h = loadTm2Tex(backend, path);
        texCache.emplace_back(name, h);
        if (h == kNullTexture)
            std::fprintf(stderr, "main: missing texture %s\n", path.c_str());
        return h;
    };

    // Resolve per-piece, per-material textures from the piece's name table.
    // Pieces with no explicit material names (e.g. the 0str decorative
    // structures) default every material slot to the stage texture st0_a.
    for (auto& sp : pieces) {
        const size_t matCount = sp.texByMat.size();
        if (sp.mesh.materialNames.empty()) {
            if (matCount > 0) {
                const TextureHandle stage = texForName("st0_a");
                for (size_t f = 0; f < matCount; ++f) sp.texByMat[f] = stage;
            }
            continue;
        }
        for (size_t f = 0; f < sp.mesh.materialNames.size() && f < matCount; ++f) {
            sp.texByMat[f] = texForName(sp.mesh.materialNames[f]);
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
    auto buildBatches = [&](TextureHandle fallbackTex) -> std::vector<TextureBatch> {
        std::vector<TextureBatch> batch;
        for (const auto& sp : pieces) {
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

    // Strip-semantics batch builder: reconstruct per-texture strip blocks
    // from the preserved Ps2oStrip topology. Each strip contributes N spine
    // verts explicitly (UV = k + offset[material] per spine position), and
    // the plane between the current and next strip uses the same N keeps
    // N-2 triangles. When no piece has strips (fallback file), this returns
    // an empty vector and the flat TextureBatch path below is used.
    auto buildStripBatches = [&](TextureHandle fallbackTex) -> std::vector<StripBatch> {
        std::vector<StripBatch> batch;
        for (const auto& sp : pieces) {
            const auto& mesh = sp.mesh;
            if (mesh.strips.empty()) continue;
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
        }
        return batch;
    };

    // Checkerboard is applied only when --uv-test is set; otherwise the loop
    // assigns kNullTexture for untextured batches (white).
    const std::vector<TextureBatch> textureBatches = buildBatches(
        uvTest ? checkerTex : kNullTexture);
    const std::vector<StripBatch> stripBatches = buildStripBatches(
        uvTest ? checkerTex : kNullTexture);
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

    // Interactive camera state (zoom/orbit via keyboard + mouse wheel).
    float curDist = dist;
    float curAngle = (camAngleRad > 0.0f) ? camAngleRad : 0.0f;

    // Input wiring: platform Input class fed from the X11 event pump. The
    // key state then drives a real world-space placeholder marker (WASD/D-
    // pad move it in the room's XZ plane, shown as a red floor quad) so the
    // native runtime consumes genuine keyboard input from a single source.
    Input input;
    input.initialize();
    float markerX = cx, markerZ = cz, markerY = cy;

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

    auto pollCameraInput = [&]() {
        void* dispV = backend.getNativeDisplay();
        const unsigned long winU = backend.getNativeWindow();
        if (!dispV || !winU) return;
        ::Display* d = static_cast<::Display*>(dispV);
        ::Window w = static_cast<::Window>(winU);
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
                }
                std::fprintf(stderr, "main: cam dist=%.1f ang=%.2f rad\n", curDist, curAngle);
            }
        }
    };
    std::fprintf(stderr, "main: controls: Z/+/wheel-up zoom-in, X/-/wheel-down zoom-out, arrows orbit, Q/Esc quit\n");
    std::fprintf(stderr, "main: WASD/arrows move the placeholder marker (red floor quad)\n");

    for (u32 f = 0; frames == 0 || f < frames; ++f) {
        backend.beginFrame();
        backend.clear(10, 12, 18, 255);

        // Top-of-frame key snapshot must precede the X11 pump so edge
        // queries compare against last frame's state (see input_test).
        input.update();
        pollCameraInput();

        // Move the placeholder marker in the room's XZ plane (world units
        // per frame; PS2 units are cm-scale, so 25 animated tatami-like grid
        // is a comfortable walk pace relative to the ~200+ unit room).
        const float step = 25.0f;
        if (input.isKeyDown(KeyW)) markerZ += step;
        if (input.isKeyDown(KeyS)) markerZ -= step;
        if (input.isKeyDown(KeyA)) markerX -= step;
        if (input.isKeyDown(KeyD)) markerX += step;

        const float ang = curAngle;
        const float eyeX = cx + curDist * std::cos(ang);
        const float eyeZ = cz + curDist * std::sin(ang);
        const float eye[3] = { eyeX, cy + curDist * 0.15f, eyeZ };
        const Matrix4x4 proj2 = Matrix4x4::perspective(70.0f, 640.0f / 448.0f,
                                                       curDist * 0.01f,
                                                       curDist * 10.0f);
        const float tgt[3] = { cx, cy, cz };
        const float up[3] = { 0.0f, 1.0f, 0.0f };
        const Matrix4x4 view = Matrix4x4::lookAt(eye, tgt, up);
        const Matrix4x4 model = Matrix4x4::identity();
        backend.setMatrices(proj2, view, model);

        // Single draw per pre-computed per-texture block: one state bind +
        // one flush per texture instead of per-mesh/per-material vectors.
        if (useStrips) {
            // Strip-semantics path: the PS2O geometry renders through
            // glMultiDrawArrays(GL_TRIANGLE_STRIP) — N spine verts per strip
            // become N-2 triangles with no indexed duplication.
            for (const auto& sb : stripBatches) {
                if (sb.sourceStrips == 0) continue;
                TextureHandle tex = (sb.texture != kNullTexture) ? sb.texture
                                    : (uvTest ? checkerTex : kNullTexture);
                backend.bindTexture(tex, 0);
                backend.drawStrips(RenderList::Opaque,
                                   sb.vertices.data(),
                                   static_cast<uint32_t>(sb.vertices.size()),
                                   sb.firsts.data(), sb.counts.data(),
                                   static_cast<uint32_t>(sb.counts.size()),
                                   sb.texture, 255, 255, 255, 255);
            }
        } else {
            for (const auto& tb : textureBatches) {
                if (tb.indices.empty()) continue;
                TextureHandle tex = (tb.texture != kNullTexture) ? tb.texture
                                    : (uvTest ? checkerTex : kNullTexture);
                backend.bindTexture(tex, 0);
                backend.drawIndexed(GSPrimitive::Triangle, RenderList::Opaque,
                                    tb.indices.data(),
                                    static_cast<uint32_t>(tb.indices.size()),
                                    tb.vertices.data(), 0,
                                    tb.texture, 255, 255, 255, 255);
            }
        }

        // Placeholder marker: a red floor quad in world space driven by the
        // wired keyboard input (WASD). It doubles as an input self-test — a
        // live red square proves isKeyDown state flows from X11 → Input.
        // drawPrimitive consumes groups of 4 vertices ([A,B,C,C] degenerates
        // to two triangles), so a plain 4-corner quad is one call.
        {
            const float s = 30.0f; // half-size in world units
            RenderVertex mq[4]{};
            mq[0].y = mq[1].y = mq[2].y = mq[3].y = markerY;
            mq[0].x = markerX - s; mq[0].z = markerZ - s;
            mq[1].x = markerX + s; mq[1].z = markerZ - s;
            mq[2].x = markerX + s; mq[2].z = markerZ + s;
            mq[3].x = markerX - s; mq[3].z = markerZ + s;
            backend.drawPrimitive(GSPrimitive::Triangle, RenderList::Opaque,
                                  mq, 4, kNullTexture, 255, 30, 30, 255);
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
    backend.shutdown();
    return 0;
}

int runMeshDemo(const char* p2oPath, u32 frames, const char* shotPath, bool uvTest, bool matTest, const char* tm2Path) {
    return runSceneDemo({ p2oPath }, tm2Path ? std::string(tm2Path) : std::string(), frames, shotPath, uvTest, -1.0f, false);
}

int runOpenGLDemo(int argc, char* argv[]) {
    using namespace ico::engine;

    u32 frames = 180;
    const char* tm2Path = nullptr;
    const char* p2oPath = nullptr;
    const char* shotPath = nullptr;
    const char* sceneDir = nullptr;
    const char* texDir = nullptr;
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
    if (sceneDir != nullptr) {
        std::vector<std::string> pieces;
        const char* sceneCandidates[] = {
            "assets/scene/pieces/", "../native/assets/scene/pieces/", nullptr
        };
        std::string dir = sceneDir;
        if (std::strchr(sceneDir, '/') == nullptr) {
            for (int c = 0; sceneCandidates[c] != nullptr; ++c) {
                std::ifstream f(std::string(sceneCandidates[c]) + "169_door.p2o");
                if (f.good()) { dir = sceneCandidates[c]; break; }
            }
        } else if (!dir.empty() && dir.back() != '/') {
            dir += '/';
        }
        std::string path = dir + "169_door.p2o";
        std::ifstream test(path);
        if (!test.good()) {
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
        for (int c = 0; manifestCandidates[c] != nullptr; ++c) {
            std::ifstream f(manifestCandidates[c]);
            if (f.good()) { manifestPath = manifestCandidates[c]; break; }
        }
        if (!manifestPath.empty() && store.parse(manifestPath.c_str())) {
            const u32 exhibitionScene = 0x0Fu;
            const std::size_t count = store.sceneAssetCount(exhibitionScene);
            std::fprintf(stderr, "main: scene composition from manifest %s "
                                 "(scene 0x0F, %zu pieces)\n",
                         manifestPath.c_str(), count);
            for (std::size_t i = 0; i < count; ++i) {
                const ico::engine::SceneAssetEntry* entry =
                    store.sceneAsset(exhibitionScene, i);
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
            std::ifstream t(texDirResolved + "st0_a.tm2");
            if (!t.good()) {
                std::fprintf(stderr, "main: --scene texture dir '%s' has no st0_a.tm2\n",
                             texDirResolved.c_str());
                return 1;
            }
        }
        return runSceneDemo(pieces, texDirResolved, frames, shotPath, uvTest,
                            camAngleRad, fitMacro);
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
        return runMeshDemo(p2oPath, frames, shotPath, uvTest, matTest, tm2Path);
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