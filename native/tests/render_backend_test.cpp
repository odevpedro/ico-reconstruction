#include "engine/RenderBackend.h"

#include <cassert>
#include <cmath>
#include <cstdio>

using ico::engine::Matrix4x4;
using ico::engine::RenderBackend;
using ico::engine::RenderVertex;
using ico::engine::TextureDesc;
using ico::engine::TextureFormat;
using ico::engine::GSPrimitive;
using ico::engine::RenderList;
using ico::engine::GSBlendMode;
using ico::engine::GSDepthTest;
using ico::engine::GSAlphaTest;
using ico::engine::kNullTexture;
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

static void test_matrix_multiply() {
    Matrix4x4 id = Matrix4x4::identity();
    Matrix4x4 t = Matrix4x4::translation(1.0f, 2.0f, 3.0f);
    Matrix4x4 r = Matrix4x4::multiply(id, t);
    assert(nearEqual(r.m[12], 1.0f));
    assert(nearEqual(r.m[13], 2.0f));
    assert(nearEqual(r.m[14], 3.0f));
}

static void test_matrix_inverse() {
    Matrix4x4 t = Matrix4x4::translation(5.0f, -3.0f, 7.0f);
    Matrix4x4 inv = Matrix4x4::inverse(t);
    Matrix4x4 product = Matrix4x4::multiply(t, inv);
    assert(nearEqual(product.m[0], 1.0f));
    assert(nearEqual(product.m[5], 1.0f));
    assert(nearEqual(product.m[10], 1.0f));
    assert(nearEqual(product.m[15], 1.0f));
    assert(nearEqual(product.m[12], 0.0f));
}

static void test_matrix_perspective() {
    Matrix4x4 p = Matrix4x4::perspective(1.047f, 4.0f / 3.0f, 0.1f, 1000.0f);
    assert(nearEqual(p.m[11], -1.0f));
    assert(p.m[10] < 0.0f);
}

static void test_backend_lifecycle() {
    auto backend = ico::engine::createRenderBackend();
    assert(backend != nullptr);
    assert(!backend->isInitialized());

    bool ok = backend->initialize(kPs2ScreenWidth, kPs2ScreenHeight);
    assert(ok);
    assert(backend->isInitialized());
    assert(backend->getWidth() == kPs2ScreenWidth);
    assert(backend->getHeight() == kPs2ScreenHeight);

    backend->shutdown();
    assert(!backend->isInitialized());
}

static void test_backend_frame() {
    auto backend = ico::engine::createRenderBackend();
    backend->initialize(640, 448);

    backend->beginFrame();
    backend->clear(0, 0, 0);
    backend->clearDepth();
    backend->setViewport(0, 0, 640, 448);
    backend->setScissor(0, 0, 640, 448);

    backend->setBlendMode(GSBlendMode::Alpha);
    backend->setDepthTest(GSDepthTest::Less, true);
    backend->setAlphaTest(GSAlphaTest::Greater, 0x80, 0xFF);

    Matrix4x4 proj = Matrix4x4::perspective(1.047f, 4.0f / 3.0f, 0.1f, 1000.0f);
    Matrix4x4 view = Matrix4x4::identity();
    Matrix4x4 model = Matrix4x4::translation(0.0f, 0.0f, -5.0f);
    backend->setMatrices(proj, view, model);

    RenderVertex tri[3] = {};
    tri[0].x = -1.0f; tri[0].y = -1.0f; tri[0].z = 0.0f;
    tri[1].x =  1.0f; tri[1].y = -1.0f; tri[1].z = 0.0f;
    tri[2].x =  0.0f; tri[2].y =  1.0f; tri[2].z = 0.0f;
    tri[0].r = tri[1].r = tri[2].r = 255;
    tri[0].g = tri[1].g = tri[2].g = 0;
    tri[0].b = tri[1].b = tri[2].b = 0;
    tri[0].a = tri[1].a = tri[2].a = 255;

    backend->drawPrimitive(GSPrimitive::Triangle, RenderList::Opaque,
                           tri, 3, kNullTexture);

    backend->drawSprite(10.0f, 10.0f, 100.0f, 50.0f,
                        0.0f, 0.0f, 1.0f, 1.0f,
                        kNullTexture);

    u8 corners[4][4] = {
        {255, 0, 0, 255}, {0, 255, 0, 255},
        {0, 0, 255, 255}, {255, 255, 0, 255}
    };
    backend->drawSpriteGouraud(100.0f, 100.0f, 200.0f, 100.0f,
                               0.0f, 0.0f, 1.0f, 1.0f,
                               kNullTexture, corners);

    backend->beginPass(RenderList::Opaque);
    backend->endPass();

    backend->swapBuffers();
    backend->endFrame();
    backend->present();

    backend->shutdown();
}

static void test_textures() {
    auto backend = ico::engine::createRenderBackend();
    backend->initialize(640, 448);

    TextureDesc desc{};
    desc.width = 64;
    desc.height = 64;
    desc.format = TextureFormat::PSMCT32;
    desc.data = nullptr;
    desc.dataSize = 0;
    desc.generateMipmaps = false;

    auto tex = backend->createTexture(desc);
    assert(tex != kNullTexture);

    backend->bindTexture(tex, 0);
    backend->destroyTexture(tex);

    backend->shutdown();
}

static void test_render_targets() {
    auto backend = ico::engine::createRenderBackend();
    backend->initialize(640, 448);

    auto rt = backend->createRenderTarget(640, 448);
    assert(rt != 0);

    backend->setRenderTarget(rt);
    backend->clear(128, 128, 128);
    backend->destroyRenderTarget(rt);

    backend->setRenderTarget(0);
    backend->shutdown();
}

static void test_render_passes() {
    auto backend = ico::engine::createRenderBackend();
    backend->initialize(640, 448);

    backend->beginFrame();

    for (u32 list = 0; list < 8; ++list) {
        backend->beginPass(static_cast<RenderList>(list));
        backend->endPass();
    }

    backend->endFrame();
    backend->shutdown();
}

int main() {
    test_matrix_identity();
    test_matrix_multiply();
    test_matrix_inverse();
    test_matrix_perspective();
    test_backend_lifecycle();
    test_backend_frame();
    test_textures();
    test_render_targets();
    test_render_passes();

    std::printf("render_backend_test: all passed\n");
    return 0;
}
