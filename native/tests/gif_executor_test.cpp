#include "engine/GifCommandExecutor.h"
#include "engine/GifCommandBuffer.h"
#include "engine/RenderBackend.h"

#include <cassert>
#include <cstdio>
#include <cstring>
#include <vector>

using namespace ico::engine;

class TestBackend : public RenderBackend {
public:
    bool initialize(u32 width, u32 height) override {
        m_width = width;
        m_height = height;
        m_initialized = true;
        return true;
    }
    void shutdown() override { m_initialized = false; }
    bool isInitialized() const override { return m_initialized; }

    void beginFrame() override {}
    void endFrame() override {}
    void present() override {}

    void clear(u8 r, u8 g, u8 b, u8 a) override {
        m_clearCalls++;
        m_lastClearR = r; m_lastClearG = g; m_lastClearB = b; m_lastClearA = a;
    }
    void clearDepth() override {}
    void setViewport(u32 x, u32 y, u32 w, u32 h) override {
        m_viewportCalls++;
        m_lastVp = {x, y, w, h};
    }
    void setScissor(u32 x, u32 y, u32 w, u32 h) override { (void)x; (void)y; (void)w; (void)h; }
    void swapBuffers() override {}

    void setBlendMode(GSBlendMode mode) override { m_lastBlend = mode; }
    void setDepthTest(GSDepthTest test, bool write) override { m_lastDepthTest = test; m_depthWrite = write; }
    void setAlphaTest(GSAlphaTest test, u8 ref, u8 mask) override { (void)test; (void)ref; (void)mask; }
    void setFramebuffer(u32, u32, u32) override {}
    void setZBuffer(u32, u32, bool) override {}
    void setAlpha(u32, u32, u32, u32, u32) override {}

    TextureHandle createTexture(const TextureDesc& desc) override { (void)desc; ++m_createTextureCalls; return ++m_nextTex; }
    void destroyTexture(TextureHandle) override {}
    void bindTexture(TextureHandle h, u32 s) override { m_boundTex = h; m_texSlot = s; }

    RenderTargetHandle createRenderTarget(u32, u32) override { return ++m_nextRT; }
    void destroyRenderTarget(RenderTargetHandle) override {}
    void setRenderTarget(RenderTargetHandle h) override { m_boundRT = h; }

    void setMatrices(const Matrix4x4& p, const Matrix4x4& v, const Matrix4x4& m) override {
        (void)p; (void)v; (void)m; m_matrixCalls++;
    }

    void drawPrimitive(GSPrimitive p, RenderList l, const RenderVertex* v, u32 c,
                       TextureHandle t, u8 r, u8 g, u8 b, u8 a) override {
        m_drawPrimCalls++;
        m_lastPrim = p; m_lastList = l; m_lastPrimCount = c; m_lastPrimTex = t;
        m_lastPrimR = r; m_lastPrimG = g; m_lastPrimB = b; m_lastPrimA = a;
        (void)v;
    }
    void drawIndexed(GSPrimitive, RenderList, const u32*, u32,
                     const RenderVertex*, u32, TextureHandle, u8, u8, u8, u8) override {}
    void drawSprite(float x, float y, float w, float h,
                    float u0, float v0, float u1, float v1,
                    TextureHandle t, u8 r, u8 g, u8 b, u8 a) override {
        m_spriteCalls++;
        m_lastSpriteX = x; m_lastSpriteY = y; m_lastSpriteW = w; m_lastSpriteH = h;
        m_lastSpriteU0 = u0; m_lastSpriteV0 = v0; m_lastSpriteU1 = u1; m_lastSpriteV1 = v1;
        m_lastSpriteTex = t; m_lastSpriteR = r; m_lastSpriteG = g; m_lastSpriteB = b; m_lastSpriteA = a;
    }
    void drawSpriteGouraud(float x, float y, float w, float h,
                           float u0, float v0, float u1, float v1,
                           TextureHandle t, const u8 corners[4][4]) override {
        m_gouraudCalls++;
        m_lastGouraudX = x; m_lastGouraudY = y; m_lastGouraudW = w; m_lastGouraudH = h;
        m_lastGouraudTex = t;
        std::memcpy(m_lastGouraudCorners, corners, sizeof(m_lastGouraudCorners));
        (void)u0; (void)v0; (void)u1; (void)v1;
    }
    void copyTexture(float srcX, float srcY, float dstX, float dstY,
                     float w, float h) override {
        m_copyCalls++;
        m_lastCopySrcX = srcX; m_lastCopySrcY = srcY;
        m_lastCopyDstX = dstX; m_lastCopyDstY = dstY;
        m_lastCopyW = w; m_lastCopyH = h;
    }

    void beginPass(RenderList l) override { m_passCalls++; m_lastPassList = l; }
    void endPass() override { m_endPassCalls++; }

    u32 getWidth() const override { return m_width; }
    u32 getHeight() const override { return m_height; }

    bool m_initialized = false;
    u32 m_width = 0;
    u32 m_height = 0;

    u32 m_clearCalls = 0;
    u8 m_lastClearR = 0, m_lastClearG = 0, m_lastClearB = 0, m_lastClearA = 0;
    u32 m_viewportCalls = 0;
    struct { u32 x, y, w, h; } m_lastVp{};

    GSBlendMode m_lastBlend = GSBlendMode::None;
    GSDepthTest m_lastDepthTest = GSDepthTest::Always;
    bool m_depthWrite = true;

    TextureHandle m_nextTex = kNullTexture;
    u32 m_createTextureCalls = 0;
    TextureHandle m_boundTex = kNullTexture;
    u32 m_texSlot = 0;
    RenderTargetHandle m_nextRT = kNullRenderTarget;
    RenderTargetHandle m_boundRT = kNullRenderTarget;
    u32 m_matrixCalls = 0;

    u32 m_drawPrimCalls = 0;
    GSPrimitive m_lastPrim = GSPrimitive::Point;
    RenderList m_lastList = RenderList::Opaque;
    u32 m_lastPrimCount = 0;
    TextureHandle m_lastPrimTex = kNullTexture;
    u8 m_lastPrimR = 0, m_lastPrimG = 0, m_lastPrimB = 0, m_lastPrimA = 0;

    u32 m_spriteCalls = 0;
    float m_lastSpriteX = 0, m_lastSpriteY = 0, m_lastSpriteW = 0, m_lastSpriteH = 0;
    float m_lastSpriteU0 = 0, m_lastSpriteV0 = 0, m_lastSpriteU1 = 0, m_lastSpriteV1 = 0;
    TextureHandle m_lastSpriteTex = kNullTexture;
    u8 m_lastSpriteR = 0, m_lastSpriteG = 0, m_lastSpriteB = 0, m_lastSpriteA = 0;

    u32 m_gouraudCalls = 0;
    float m_lastGouraudX = 0, m_lastGouraudY = 0, m_lastGouraudW = 0, m_lastGouraudH = 0;
    TextureHandle m_lastGouraudTex = kNullTexture;
    u8 m_lastGouraudCorners[4][4]{};

    u32 m_copyCalls = 0;
    float m_lastCopySrcX = 0, m_lastCopySrcY = 0;
    float m_lastCopyDstX = 0, m_lastCopyDstY = 0;
    float m_lastCopyW = 0, m_lastCopyH = 0;

    u32 m_passCalls = 0;
    RenderList m_lastPassList = RenderList::Opaque;
    u32 m_endPassCalls = 0;
};

static void test_executor_sprite() {
    TestBackend backend;
    backend.initialize(640, 448);
    GifCommandExecutor exec(backend);

    GifCommandBuffer buf;

    std::vector<u8> packet;

    GifTag tag{};
    tag.setNloop(6);
    tag.setEop(true);
    tag.setPre(true);
    tag.setPrim(6 | (1 << 5));
    tag.setFlg(GifFlg::Packed);
    tag.setNreg(3);
    tag.setReg(0, GifReg::RGBAQ);
    tag.setReg(1, GifReg::UV);
    tag.setReg(2, GifReg::XYZ2);
    packet.resize(kGifTagSize);
    std::memcpy(packet.data(), tag.bytes, kGifTagSize);

    GsRgbaq rgbaq0 = GsRgbaq::make(255, 128, 64, 200);
    packet.resize(packet.size() + 16);
    std::memcpy(packet.data() + kGifTagSize, &rgbaq0.value, 8);

    GsUv uv0 = GsUv::make(0.0f, 0.0f);
    packet.resize(packet.size() + 16);
    std::memcpy(packet.data() + kGifTagSize + 16, &uv0.value, 4);

    GsXyz2 xyz0 = GsXyz2::make(10.0f, 20.0f, 0.0f);
    packet.resize(packet.size() + 16);
    std::memcpy(packet.data() + kGifTagSize + 32, &xyz0.value, 8);

    GsRgbaq rgbaq1 = GsRgbaq::make(255, 128, 64, 200);
    packet.resize(packet.size() + 16);
    std::memcpy(packet.data() + kGifTagSize + 48, &rgbaq1.value, 8);

    GsUv uv1 = GsUv::make(100.0f, 50.0f);
    packet.resize(packet.size() + 16);
    std::memcpy(packet.data() + kGifTagSize + 64, &uv1.value, 4);

    GsXyz2 xyz1 = GsXyz2::make(110.0f, 70.0f, 0.0f);
    packet.resize(packet.size() + 16);
    std::memcpy(packet.data() + kGifTagSize + 80, &xyz1.value, 8);

    buf.parsePacket(packet.data(), static_cast<u32>(packet.size()));
    assert(buf.commandCount() == 1);

    exec.execute(buf);

    assert(backend.m_spriteCalls == 1);
    assert(backend.m_lastSpriteX == 10.0f);
    assert(backend.m_lastSpriteY == 20.0f);
    assert(backend.m_lastSpriteW == 100.0f);
    assert(backend.m_lastSpriteH == 50.0f);
    assert(backend.m_lastSpriteR == 255);
    assert(backend.m_lastSpriteG == 128);
    assert(backend.m_lastSpriteB == 64);
    assert(backend.m_lastSpriteA == 200);
}

static void test_executor_direct_commands() {
    TestBackend backend;
    backend.initialize(640, 448);
    GifCommandExecutor exec(backend);

    RenderCmd blendCmd{};
    blendCmd.type = RenderCommand::SetBlendMode;
    blendCmd.blendMode.mode = GSBlendMode::Alpha;

    RenderCmd depthCmd{};
    depthCmd.type = RenderCommand::SetDepthTest;
    depthCmd.depthTest.test = GSDepthTest::Less;
    depthCmd.depthTest.write = false;

    exec.executeCommand(blendCmd);
    assert(backend.m_lastBlend == GSBlendMode::Alpha);

    exec.executeCommand(depthCmd);
    assert(backend.m_lastDepthTest == GSDepthTest::Less);
    assert(backend.m_depthWrite == false);
}

static void test_executor_bridge_geometry_commands() {
    TestBackend backend;
    backend.initialize(640, 448);
    GifCommandExecutor exec(backend);

    RenderCmd lineCmd{};
    lineCmd.type = RenderCommand::DrawLine;
    lineCmd.line.x0 = 10.0f;
    lineCmd.line.y0 = 20.0f;
    lineCmd.line.x1 = 30.0f;
    lineCmd.line.y1 = 40.0f;
    lineCmd.line.r = 1;
    lineCmd.line.g = 2;
    lineCmd.line.b = 3;
    lineCmd.line.a = 4;
    exec.executeCommand(lineCmd);
    assert(backend.m_drawPrimCalls == 1);
    assert(backend.m_lastPrim == GSPrimitive::Line);
    assert(backend.m_lastPrimCount == 2);

    RenderCmd pointCmd{};
    pointCmd.type = RenderCommand::DrawPoint;
    pointCmd.point.x = 11.0f;
    pointCmd.point.y = 22.0f;
    pointCmd.point.r = 5;
    pointCmd.point.g = 6;
    pointCmd.point.b = 7;
    pointCmd.point.a = 8;
    exec.executeCommand(pointCmd);
    assert(backend.m_drawPrimCalls == 2);
    assert(backend.m_lastPrim == GSPrimitive::Point);
    assert(backend.m_lastPrimCount == 1);

    RenderCmd triCmd{};
    triCmd.type = RenderCommand::DrawTriangle;
    triCmd.triangle.x0 = 0.0f;
    triCmd.triangle.y0 = 0.0f;
    triCmd.triangle.x1 = 1.0f;
    triCmd.triangle.y1 = 0.0f;
    triCmd.triangle.x2 = 0.0f;
    triCmd.triangle.y2 = 1.0f;
    triCmd.triangle.r = 9;
    triCmd.triangle.g = 10;
    triCmd.triangle.b = 11;
    triCmd.triangle.a = 12;
    exec.executeCommand(triCmd);
    assert(backend.m_drawPrimCalls == 3);
    assert(backend.m_lastPrim == GSPrimitive::Triangle);
    assert(backend.m_lastPrimCount == 3);

    RenderCmd copyCmd{};
    copyCmd.type = RenderCommand::CopyTexture;
    copyCmd.copy.srcX = 10.0f;
    copyCmd.copy.srcY = 20.0f;
    copyCmd.copy.dstX = 30.0f;
    copyCmd.copy.dstY = 40.0f;
    copyCmd.copy.w = 50.0f;
    copyCmd.copy.h = 60.0f;
    exec.executeCommand(copyCmd);
    assert(backend.m_drawPrimCalls == 3);
    /* the copy is forwarded to the backend with full geometry, no draw side effects */
    assert(backend.m_copyCalls == 1);
    assert(backend.m_lastCopySrcX == 10.0f && backend.m_lastCopySrcY == 20.0f);
    assert(backend.m_lastCopyDstX == 30.0f && backend.m_lastCopyDstY == 40.0f);
    assert(backend.m_lastCopyW == 50.0f && backend.m_lastCopyH == 60.0f);
}

static void test_executor_empty() {
    TestBackend backend;
    backend.initialize(640, 448);
    GifCommandExecutor exec(backend);

    GifCommandBuffer buf;
    exec.execute(buf);

    assert(backend.m_spriteCalls == 0);
    assert(backend.m_drawPrimCalls == 0);
    assert(backend.m_clearCalls == 0);
}

static void test_executor_vram_upload() {
    /* Rev.118: the executor resolves a buffer-uploaded texture handle into a
       real backend texture via createTexture+bindTexture before drawing. */
    TestBackend backend;
    backend.initialize(640, 448);
    GifCommandExecutor exec(backend);

    GifCommandBuffer buf;
    auto addAD = [&](u32 addr, u64 value) {
        GifTag tag = GifTag::makeSimple(1, true, GifFlg::Packed, 1);
        tag.setReg(0, GifReg::A_D);
        std::vector<u8> packet;
        packet.resize(kGifTagSize);
        std::memcpy(packet.data(), tag.bytes, kGifTagSize);
        packet.resize(packet.size() + 16);
        std::uint8_t* qw = packet.data() + kGifTagSize;
        std::memcpy(qw, &value, 8);
        qw[8] = static_cast<std::uint8_t>(addr);
        assert(buf.parsePacket(packet.data(), static_cast<u32>(packet.size())));
    };

    GsBitbltBuf bb{};
    bb.setDbase(0x2000);
    bb.setDbw(1);
    bb.setDpsm(0);
    addAD(kGsAddrBITBLTBUF, bb.value);
    GsTrxPos tp{};
    tp.setDsx(0);
    tp.setDsy(0);
    addAD(kGsAddrTRXPOS, tp.value);
    GsTrxReg tr{};
    tr.setRrw(4);
    tr.setRrh(1);
    addAD(kGsAddrTRXREG, tr.value);
    GsTrxDir td{};
    td.setXdir(0);
    addAD(kGsAddrTRXDIR, td.value);

    {
        GifTag img{};
        img.setNloop(1);
        img.setEop(true);
        img.setFlg(GifFlg::Image);
        std::vector<u8> packet;
        packet.resize(kGifTagSize);
        std::memcpy(packet.data(), img.bytes, kGifTagSize);
        u32 px[4] = {0x000000FFu, 0x0000FF00u, 0x00FF0000u, 0xFF000080u};
        packet.resize(packet.size() + 16);
        std::memcpy(packet.data() + kGifTagSize, px, sizeof(px));
        assert(buf.parsePacket(packet.data(), static_cast<u32>(packet.size())));
    }

    GsTex0 tex0{};
    tex0.setTbp0(0x2000);
    tex0.setTcc(1);
    addAD(kGsAddrTEX0_1, tex0.value);

    GifTag sp{};
    sp.setNloop(6);
    sp.setEop(true);
    sp.setPre(true);
    sp.setPrim(6 | (1 << 5));
    sp.setFlg(GifFlg::Packed);
    sp.setNreg(3);
    sp.setReg(0, GifReg::RGBAQ);
    sp.setReg(1, GifReg::UV);
    sp.setReg(2, GifReg::XYZ2);
    std::vector<u8> spk;
    spk.resize(kGifTagSize);
    std::memcpy(spk.data(), sp.bytes, kGifTagSize);
    GsRgbaq c = GsRgbaq::make(255, 128, 64, 200);
    spk.resize(spk.size() + 16);
    std::memcpy(spk.data() + kGifTagSize, &c.value, 8);
    GsUv uv0 = GsUv::make(0.0f, 0.0f);
    spk.resize(spk.size() + 16);
    std::memcpy(spk.data() + kGifTagSize + 16, &uv0.value, 4);
    GsXyz2 x0 = GsXyz2::make(10.0f, 20.0f, 0.0f);
    spk.resize(spk.size() + 16);
    std::memcpy(spk.data() + kGifTagSize + 32, &x0.value, 8);
    GsRgbaq c2 = GsRgbaq::make(255, 128, 64, 200);
    spk.resize(spk.size() + 16);
    std::memcpy(spk.data() + kGifTagSize + 48, &c2.value, 8);
    GsUv uv1 = GsUv::make(100.0f, 50.0f);
    spk.resize(spk.size() + 16);
    std::memcpy(spk.data() + kGifTagSize + 64, &uv1.value, 4);
    GsXyz2 x1 = GsXyz2::make(110.0f, 70.0f, 0.0f);
    spk.resize(spk.size() + 16);
    std::memcpy(spk.data() + kGifTagSize + 80, &x1.value, 8);
    assert(buf.parsePacket(spk.data(), static_cast<u32>(spk.size())));

    exec.execute(buf);

    assert(backend.m_createTextureCalls == 1);
    assert(backend.m_spriteCalls == 1);
    assert(backend.m_lastSpriteTex != kNullTexture);
    assert(backend.m_boundTex == backend.m_lastSpriteTex);
}

int main() {
    test_executor_sprite();
    test_executor_direct_commands();
    test_executor_bridge_geometry_commands();
    test_executor_empty();
    test_executor_vram_upload();

    std::printf("gif_executor_test: all passed\n");
    return 0;
}
