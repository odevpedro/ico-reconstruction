#include "runtime/IcoRuntime.h"
#include "runtime/Logger.h"
#include <cstdio>

#ifdef ICO_HAS_OPENGL
#include "engine/GifPacket.h"
#include "engine/OpenGLBackend.h"
#include "engine/Ps2oMesh.h"
#include "engine/RenderBackend.h"
#include "engine/Tm2Converter.h"
#include "engine/Tm2Format.h"
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <thread>
#include <vector>
#endif

namespace {

#ifdef ICO_HAS_OPENGL
int runMeshDemo(const char* p2oPath, u32 frames, const char* shotPath, bool uvTest, bool matTest, const char* tm2Path) {
    using namespace ico::engine;

    Ps2oMesh mesh;
    if (!loadPs2oMeshFromFile(p2oPath, mesh)) {
        std::fprintf(stderr, "main: failed to load PS2O mesh %s\n", p2oPath);
        return 1;
    }

    const uint32_t nv = mesh.vertexCount;
    const uint32_t triCount = static_cast<uint32_t>(mesh.triangles.size() / 3);
    const uint32_t uvCount = static_cast<uint32_t>(mesh.uvs.size() / 2);
    const uint32_t matCount = static_cast<uint32_t>(mesh.triMaterials.size());
    std::fprintf(stderr, "main: PS2O %s: %u verts, %u triangles, %u quads, "
                         "%u submeshes, %u uvs, %u triMaterials\n",
                 p2oPath, nv, triCount,
                 static_cast<uint32_t>(mesh.quadRecords.size() / 8),
                 mesh.subMeshCount, uvCount, matCount);

    // Bounding box of triangle-referenced vertices (the decoded room piece).
    float minX = 1e30f, maxX = -1e30f;
    float minY = 1e30f, maxY = -1e30f;
    float minZ = 1e30f, maxZ = -1e30f;
    for (uint32_t i = 0; i < mesh.triangles.size(); ++i) {
        const uint32_t vi = mesh.triangles[i];
        const float x = mesh.positions[vi * 3 + 0];
        const float y = mesh.positions[vi * 3 + 1];
        const float z = mesh.positions[vi * 3 + 2];
        minX = std::min(minX, x); maxX = std::max(maxX, x);
        minY = std::min(minY, y); maxY = std::max(maxY, y);
        minZ = std::min(minZ, z); maxZ = std::max(maxZ, z);
    }
    const float cx = (minX + maxX) * 0.5f;
    const float cy = (minY + maxY) * 0.5f;
    const float cz = (minZ + maxZ) * 0.5f;
    const float extent = std::max({maxX - minX, maxY - minY, maxZ - minZ, 1.0f});
    const float dist = extent * 1.1f;

    OpenGLBackend backend;
    if (!backend.initialize(kPs2ScreenWidth, kPs2ScreenHeight)) {
        std::fprintf(stderr, "main: OpenGL backend failed to initialize\n");
        return 1;
    }

    const Matrix4x4 proj = Matrix4x4::perspective(70.0f, 640.0f / 448.0f, 1.0f, dist * 10.0f);
    backend.setViewport(0, 0, kPs2ScreenWidth, kPs2ScreenHeight);
    backend.setDepthTest(GSDepthTest::Less, true);

    // Load TM2 texture if provided.
    TextureHandle texHandle = kNullTexture;
    u32 texW = 0, texH = 0;
    if (tm2Path && *tm2Path) {
        std::ifstream f(tm2Path, std::ios::binary);
        if (f) {
            f.seekg(0, std::ios::end);
            const std::streamoff sz = f.tellg();
            f.seekg(0, std::ios::beg);
            std::vector<u8> buf(static_cast<size_t>(sz));
            f.read(reinterpret_cast<char*>(buf.data()), sz);
            Tm2File file{};
            if (Tm2Parser::parse(buf.data(), static_cast<u32>(buf.size()), file) && !file.images.empty()) {
                Tm2Texture tex{};
                if (Tm2Converter::convertImage(file.images[0], tex) && !tex.rgbaData.empty()) {
                    TextureDesc desc{};
                    desc.width = tex.width; desc.height = tex.height;
                    desc.format = TextureFormat::PSMCT32;
                    desc.data = tex.rgbaData.data(); desc.dataSize = static_cast<u32>(tex.rgbaData.size());
                    desc.generateMipmaps = false;
                    texHandle = backend.createTexture(desc);
                    texW = tex.width; texH = tex.height;
                    std::fprintf(stderr, "main: loaded TM2 %s (%ux%u)\n", tm2Path, texW, texH);
                }
            }
        }
        if (texHandle == kNullTexture) {
            std::fprintf(stderr, "main: failed to load TM2 %s\n", tm2Path);
        }
    }

    // UV-validation checkerboard (see --uv-test).
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

    const auto frameTime = std::chrono::milliseconds(16);
    std::vector<uint8_t> tmpFrame(kPs2ScreenWidth * kPs2ScreenHeight * 3);
    for (u32 f = 0; frames == 0 || f < frames; ++f) {
        backend.beginFrame();
        backend.clear(10, 12, 18, 255);

        const float ang = 2.0f * std::fmod(static_cast<float>(f) / 300.0f, 1.0f);
        const float eyeX = cx + dist * std::cos(ang);
        const float eyeZ = cz + dist * std::sin(ang);
        const float eye[3] = { eyeX, cy + dist * 0.15f, eyeZ };
        const float tgt[3] = { cx, cy, cz };
        const float up[3] = { 0.0f, 1.0f, 0.0f };
        const Matrix4x4 view = Matrix4x4::lookAt(eye, tgt, up);
        const Matrix4x4 model = Matrix4x4::identity();
        backend.setMatrices(proj, view, model);

        for (uint32_t t = 0; t < triCount; ++t) {
            const uint32_t i0 = mesh.triangles[t * 3 + 0];
            const uint32_t i1 = mesh.triangles[t * 3 + 1];
            const uint32_t i2 = mesh.triangles[t * 3 + 2];

            // Use UVs from triVertUVs if available; otherwise fallback to mesh.uvs.
            const bool hasTexUV = mesh.triVertUVs.size() >= (t * 6 + 2);
            RenderVertex v[4] = {};
            for (int s = 0; s < 3; ++s) {
                const uint32_t vi = mesh.triangles[t * 3 + s];
                v[s].x = mesh.positions[vi * 3 + 0];
                v[s].y = mesh.positions[vi * 3 + 1];
                v[s].z = mesh.positions[vi * 3 + 2];
                if (hasTexUV) {
                    v[s].u = mesh.triVertUVs[t * 6 + s * 2 + 0];
                    v[s].v = mesh.triVertUVs[t * 6 + s * 2 + 1];
                } else if (uvCount > 0 && vi * 2 + 1 < mesh.uvs.size()) {
                    v[s].u = mesh.uvs[vi * 2 + 0];
                    v[s].v = mesh.uvs[vi * 2 + 1];
                }
            }
            v[3] = v[2];

            TextureHandle useTex = kNullTexture;
            if (texHandle != kNullTexture) useTex = texHandle;
            else if (uvTest) useTex = checkerTex;

            u8 r = 255, g = 255, b = 255;
            if (!hasTexUV && !texHandle) {
                const float yAvg = (mesh.positions[i0 * 3 + 1] +
                                     mesh.positions[i1 * 3 + 1] +
                                     mesh.positions[i2 * 3 + 1]) / 3.0f;
                const float k = (yAvg - minY) / std::max(maxY - minY, 1e-6f);
                r = static_cast<u8>(60 + 160 * k);
                g = static_cast<u8>(60 + 120 * (1.0f - k));
                b = static_cast<u8>(120 + 60 * k);
            }
            backend.drawPrimitive(GSPrimitive::Triangle, RenderList::Opaque,
                                  v, 4, useTex, r, g, b, 255);
        }

        backend.endFrame();
        backend.present();
        std::this_thread::sleep_for(frameTime);
    }

    // Capture a proof-of-render frame (PPM) if requested.
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
    if (texHandle != kNullTexture) backend.destroyTexture(texHandle);
    backend.shutdown();
    return 0;
}

int runOpenGLDemo(int argc, char* argv[]) {
    using namespace ico::engine;

    u32 frames = 180;
    const char* tm2Path = nullptr;
    const char* p2oPath = nullptr;
    const char* shotPath = nullptr;
    bool uvTest = false;
    bool matTest = false;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--frames") == 0 && i + 1 < argc) {
            frames = static_cast<u32>(std::atoi(argv[i + 1]));
        } else if (std::strcmp(argv[i], "--tm2") == 0 && i + 1 < argc) {
            tm2Path = argv[i + 1];
        } else if (std::strcmp(argv[i], "--p2o") == 0 && i + 1 < argc) {
            p2oPath = argv[i + 1];
        } else if (std::strcmp(argv[i], "--shot") == 0 && i + 1 < argc) {
            shotPath = argv[i + 1];
        } else if (std::strcmp(argv[i], "--uv-test") == 0) {
            uvTest = true;
        } else if (std::strcmp(argv[i], "--mat-test") == 0) {
            matTest = true;
        }
    }

    // Auto-discover .p2o mesh in native/assets/ if not provided.
    if (p2oPath == nullptr) {
        const char* candidates[] = {
            "../native/assets/170_st00a_p1.p2o",
            "native/assets/170_st00a_p1.p2o",
            "assets/170_st00a_p1.p2o",
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