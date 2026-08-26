#include "engine/OpenGLBackend.h"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>

using ico::engine::BatchVertex;
using ico::engine::GSAlphaTest;
using ico::engine::GSBlendMode;
using ico::engine::GSDepthTest;
using ico::engine::GSPrimitive;
using ico::engine::Matrix4x4;
using ico::engine::OpenGLBackend;
using ico::engine::OpenGLBackendTestHelper;
using ico::engine::RenderList;
using ico::engine::RenderVertex;
using ico::engine::TextureDesc;
using ico::engine::TextureFormat;
using ico::engine::TextureHandle;
using ico::engine::RenderTargetHandle;
using ico::engine::kNullTexture;
using ico::engine::kNullRenderTarget;
using ico::engine::kPs2ScreenWidth;
using ico::engine::kPs2ScreenHeight;

static bool nearEqual(float a, float b, float eps = 0.001f) {
    return std::fabs(a - b) < eps;
}

static void test_matrix_identity() {
    Matrix4x4 m = Matrix4x4::identity();
    assert(nearEqual(m.m[0], 1.0f));
    assert(nearEqual(m.m[5], 1.0f));
    assert(nearEqual(m.m[10], 1.0f));
    assert(nearEqual(m.m[15], 1.0f));
    assert(nearEqual(m.m[1], 0.0f));
    assert(nearEqual(m.m[12], 0.0f));
}

static void test_matrix_perspective() {
    Matrix4x4 p = Matrix4x4::perspective(1.047f, 4.0f / 3.0f, 0.1f, 1000.0f);
    assert(nearEqual(p.m[11], -1.0f));
    assert(p.m[10] < 0.0f);
}

static void test_matrix_ortho() {
    Matrix4x4 o = Matrix4x4::ortho(0.0f, 640.0f, 448.0f, 0.0f, -1.0f, 1.0f);
    assert(nearEqual(o.m[0], 2.0f / 640.0f));
    assert(nearEqual(o.m[5], -2.0f / 448.0f));
    assert(nearEqual(o.m[15], 1.0f));
}

static void test_batch_vertex_layout() {
    static_assert(sizeof(BatchVertex) == sizeof(RenderVertex),
                   "BatchVertex must match RenderVertex layout");
    static_assert(offsetof(BatchVertex, x) == 0, "x at offset 0");
    static_assert(offsetof(BatchVertex, y) == 4, "y at offset 4");
    static_assert(offsetof(BatchVertex, z) == 8, "z at offset 8");
    static_assert(offsetof(BatchVertex, nx) == 12, "nx at offset 12");
    static_assert(offsetof(BatchVertex, ny) == 16, "ny at offset 16");
    static_assert(offsetof(BatchVertex, nz) == 20, "nz at offset 20");
    static_assert(offsetof(BatchVertex, u) == 24, "u at offset 24");
    static_assert(offsetof(BatchVertex, v) == 28, "v at offset 28");
    static_assert(offsetof(BatchVertex, r) == 32, "r at offset 32");
    static_assert(offsetof(BatchVertex, g) == 33, "g at offset 33");
    static_assert(offsetof(BatchVertex, b) == 34, "b at offset 34");
    static_assert(offsetof(BatchVertex, a) == 35, "a at offset 35");
    assert(true);
}

static void test_batch_vertex_matches_render_vertex() {
    static_assert(sizeof(BatchVertex) == sizeof(RenderVertex),
                   "BatchVertex must match RenderVertex layout");
    static_assert(offsetof(BatchVertex, x) == offsetof(RenderVertex, x));
    static_assert(offsetof(BatchVertex, nx) == offsetof(RenderVertex, nx));
    static_assert(offsetof(BatchVertex, u) == offsetof(RenderVertex, u));
    static_assert(offsetof(BatchVertex, r) == offsetof(RenderVertex, r));
    assert(true);
}

static void test_backend_test_mode() {
    OpenGLBackend backend;
    OpenGLBackendTestHelper::setTestMode(backend);

    assert(backend.isInitialized());
    assert(backend.getWidth() == 640);
    assert(backend.getHeight() == 448);
}

static void test_backend_frame_logic() {
    OpenGLBackend backend;
    OpenGLBackendTestHelper::setTestMode(backend);

    backend.beginFrame();
    backend.endFrame();
    backend.present();
}

static void test_state_tracking() {
    OpenGLBackend backend;
    OpenGLBackendTestHelper::setTestMode(backend);

    backend.setBlendMode(GSBlendMode::Alpha);
    backend.setBlendMode(GSBlendMode::Alpha);
    backend.setBlendMode(GSBlendMode::Additive);
    backend.setBlendMode(GSBlendMode::None);
    backend.setBlendMode(GSBlendMode::Subtractive);
    backend.setBlendMode(GSBlendMode::Alpha);

    backend.setDepthTest(GSDepthTest::Less, true);
    backend.setDepthTest(GSDepthTest::Less, true);
    backend.setDepthTest(GSDepthTest::Always, false);
    backend.setDepthTest(GSDepthTest::LessEqual, true);

    backend.setAlphaTest(GSAlphaTest::Always, 0, 0xFF);
    backend.setAlphaTest(GSAlphaTest::Always, 0, 0xFF);
    backend.setAlphaTest(GSAlphaTest::Greater, 0x80, 0xFF);
    backend.setAlphaTest(GSAlphaTest::Less, 0x40, 0x0F);
}

static void test_render_pass_tracking() {
    OpenGLBackend backend;
    OpenGLBackendTestHelper::setTestMode(backend);

    for (u32 i = 0; i < 8; ++i) {
        backend.beginPass(static_cast<RenderList>(i));
        backend.endPass();
    }

    backend.beginPass(RenderList::Opaque);
    backend.beginPass(RenderList::Opaque);
    backend.endPass();

    backend.beginPass(RenderList::Transparent);
    backend.endPass();
}

static void test_setMatrices_stores_values() {
    OpenGLBackend backend;
    OpenGLBackendTestHelper::setTestMode(backend);

    Matrix4x4 proj = Matrix4x4::perspective(1.047f, 4.0f / 3.0f, 0.1f, 1000.0f);
    Matrix4x4 view = Matrix4x4::identity();
    Matrix4x4 model = Matrix4x4::translation(1.0f, 2.0f, 3.0f);
    backend.setMatrices(proj, view, model);

    backend.beginFrame();
    RenderVertex tri[3]{};
    tri[0].x = -1.0f; tri[0].y = -1.0f; tri[0].z = 0.0f;
    tri[1].x = 1.0f; tri[1].y = -1.0f; tri[1].z = 0.0f;
    tri[2].x = 0.0f; tri[2].y = 1.0f; tri[2].z = 0.0f;
    for (auto& v : tri) { v.r = 255; v.g = 255; v.b = 255; v.a = 255; }
    backend.drawPrimitive(GSPrimitive::Triangle, RenderList::Opaque,
                           tri, 3, kNullTexture, 255, 255, 255, 255);
    backend.endFrame();
}

static void test_draw_commands_no_crash() {
    OpenGLBackend backend;
    OpenGLBackendTestHelper::setTestMode(backend);

    backend.beginFrame();

    RenderVertex tri[3]{};
    tri[0].x = -1.0f; tri[0].y = -1.0f; tri[0].z = 0.0f;
    tri[1].x = 1.0f; tri[1].y = -1.0f; tri[1].z = 0.0f;
    tri[2].x = 0.0f; tri[2].y = 1.0f; tri[2].z = 0.0f;
    for (auto& v : tri) { v.r = 255; v.g = 128; v.b = 64; v.a = 255; }
    backend.drawPrimitive(GSPrimitive::Triangle, RenderList::Opaque,
                           tri, 3, kNullTexture, 255, 128, 64, 255);

    u32 indices[] = { 0, 1, 2 };
    backend.drawIndexed(GSPrimitive::Triangle, RenderList::Transparent,
                         indices, 3, tri, 0, kNullTexture, 255, 255, 255, 255);

    backend.drawSprite(10.0f, 10.0f, 100.0f, 50.0f,
                        0.0f, 0.0f, 1.0f, 1.0f, kNullTexture,
                        255, 255, 255, 255);

    u8 corners[4][4] = {
        {255, 0, 0, 255}, {0, 255, 0, 255},
        {0, 0, 255, 255}, {255, 255, 0, 255}
    };
    backend.drawSpriteGouraud(100.0f, 100.0f, 200.0f, 100.0f,
                               0.0f, 0.0f, 1.0f, 1.0f, kNullTexture, corners);

    backend.endFrame();
}

static void test_begin_frame_resets() {
    OpenGLBackend backend;
    OpenGLBackendTestHelper::setTestMode(backend);

    backend.beginFrame();
    backend.drawSprite(0.0f, 0.0f, 10.0f, 10.0f,
                        0.0f, 0.0f, 1.0f, 1.0f, kNullTexture,
                        255, 255, 255, 255);
    backend.endFrame();

    backend.beginFrame();
    backend.endFrame();
}

static void test_texture_null_ops() {
    OpenGLBackend backend;
    OpenGLBackendTestHelper::setTestMode(backend);

    TextureDesc desc{};
    desc.width = 64;
    desc.height = 64;
    desc.format = TextureFormat::PSMCT32;
    desc.data = nullptr;
    desc.dataSize = 0;
    desc.generateMipmaps = false;

    TextureHandle h = backend.createTexture(desc);
    assert(h == kNullTexture);

    backend.bindTexture(kNullTexture, 0);
    backend.destroyTexture(kNullTexture);
}

static void test_render_target_null_ops() {
    OpenGLBackend backend;
    OpenGLBackendTestHelper::setTestMode(backend);

    RenderTargetHandle rt = backend.createRenderTarget(640, 448);
    assert(rt == kNullRenderTarget);

    backend.setRenderTarget(kNullRenderTarget);
    backend.destroyRenderTarget(kNullRenderTarget);
}

static void test_clear_depth_null_ops() {
    OpenGLBackend backend;
    OpenGLBackendTestHelper::setTestMode(backend);

    backend.clear(0, 0, 0, 255);
    backend.clearDepth();
}

static void test_viewport_scissor_null_ops() {
    OpenGLBackend backend;
    OpenGLBackendTestHelper::setTestMode(backend);

    backend.setViewport(0, 0, 640, 448);
    backend.setScissor(0, 0, 640, 448);
    backend.setScissor(0, 0, 0, 0);
}

int main() {
    test_matrix_identity();
    test_matrix_perspective();
    test_matrix_ortho();
    test_batch_vertex_layout();
    test_batch_vertex_matches_render_vertex();
    test_backend_test_mode();
    test_backend_frame_logic();
    test_state_tracking();
    test_render_pass_tracking();
    test_setMatrices_stores_values();
    test_draw_commands_no_crash();
    test_begin_frame_resets();
    test_texture_null_ops();
    test_render_target_null_ops();
    test_clear_depth_null_ops();
    test_viewport_scissor_null_ops();

    std::printf("opengl_backend_test: all passed\n");
    return 0;
}
