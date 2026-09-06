#include "runtime/IcoRuntime.h"
#include "runtime/Logger.h"
#include <cstdio>

#ifdef ICO_HAS_OPENGL
#include "engine/GifPacket.h"
#include "engine/OpenGLBackend.h"
#include "engine/RenderBackend.h"
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <thread>
#endif

namespace {

#ifdef ICO_HAS_OPENGL
int runOpenGLDemo(int argc, char* argv[]) {
    using namespace ico::engine;

    u32 frames = 180;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--frames") == 0 && i + 1 < argc) {
            frames = static_cast<u32>(std::atoi(argv[i + 1]));
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
        bridge.flush();

        backend.endFrame();
        backend.present();
        std::this_thread::sleep_for(frameTime);
    }

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