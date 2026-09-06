#include "engine/GifPacket.h"
#include "engine/RenderBackend.h"

#include <cassert>
#include <cstdio>
#include <cstring>

using namespace ico::engine;

class TestBackend : public RenderBackend {
public:
    bool initialize(u32 width, u32 height) override {
        m_width = width; m_height = height; m_initialized = true; return true;
    }
    void shutdown() override { m_initialized = false; }
    bool isInitialized() const override { return m_initialized; }

    void beginFrame() override {}
    void endFrame() override {}
    void present() override {}

    void clear(u8, u8, u8, u8) override {}
    void clearDepth() override {}
    void setViewport(u32 x, u32 y, u32 w, u32 h) override {
        m_viewportCalls++; m_lastVp = {x, y, w, h};
    }
    void setScissor(u32, u32, u32, u32) override {}
    void swapBuffers() override {}

    void setBlendMode(GSBlendMode mode) override { m_lastBlend = mode; }
    void setDepthTest(GSDepthTest test, bool write) override {
        m_lastDepthTest = test; m_depthWrite = write;
    }
    void setAlphaTest(GSAlphaTest test, u8 ref, u8 mask) override {
        m_lastAlphaTest = test; m_alphaRef = ref; m_alphaMask = mask;
    }
    void setFramebuffer(u32 fbp, u32 fbw, u32 psm) override {
        m_fbCalls++; m_lastFbp = fbp; m_lastFbw = fbw; m_lastPsm = psm;
    }
    void setZBuffer(u32 zbp, u32 psm, bool zmsk) override {
        m_zbCalls++; m_lastZbp = zbp; m_lastZpsm = psm; m_lastZmsk = zmsk;
    }
    void setAlpha(u32 aba, u32 abb, u32 abc, u32 abd, u32 afix) override {
        m_alphaCalls++; m_lastAba = aba; m_lastAbb = abb; m_lastAbc = abc;
        m_lastAbd = abd; m_lastAfix = afix;
    }

    TextureHandle createTexture(const TextureDesc&) override { return ++m_nextTex; }
    void destroyTexture(TextureHandle) override {}
    void bindTexture(TextureHandle h, u32 s) override { m_boundTex = h; m_texSlot = s; }

    RenderTargetHandle createRenderTarget(u32, u32) override { return ++m_nextRT; }
    void destroyRenderTarget(RenderTargetHandle) override {}
    void setRenderTarget(RenderTargetHandle) override {}

    void setMatrices(const Matrix4x4&, const Matrix4x4&, const Matrix4x4&) override {}

    void drawPrimitive(GSPrimitive p, RenderList l, const RenderVertex*, u32 c,
                       TextureHandle, u8, u8, u8, u8) override {
        m_drawPrimCalls++; m_lastPrim = p; m_lastList = l; m_lastPrimCount = c;
    }
    void drawIndexed(GSPrimitive, RenderList, const u32*, u32,
                     const RenderVertex*, u32, TextureHandle, u8, u8, u8, u8) override {}
    void drawSprite(float x, float y, float w, float h,
                    float u0, float v0, float u1, float v1,
                    TextureHandle t, u8 r, u8 g, u8 b, u8 a) override {
        m_spriteCalls++;
        m_lastSpriteX = x; m_lastSpriteY = y; m_lastSpriteW = w; m_lastSpriteH = h;
        m_lastSpriteU0 = u0; m_lastSpriteV0 = v0; m_lastSpriteU1 = u1; m_lastSpriteV1 = v1;
        m_lastSpriteTex = t; m_lastSpriteR = r; m_lastSpriteA = a;
    }
    void drawSpriteGouraud(float, float, float, float,
                           float, float, float, float,
                           TextureHandle, const u8[4][4]) override {}
    void copyTexture(float srcX, float srcY, float dstX, float dstY,
                     float w, float h) override {
        m_copyCalls++;
        m_lastCopySrcX = srcX; m_lastCopySrcY = srcY;
        m_lastCopyDstX = dstX; m_lastCopyDstY = dstY;
        m_lastCopyW = w; m_lastCopyH = h;
    }

    void beginPass(RenderList) override {}
    void endPass() override {}

    u32 getWidth() const override { return m_width; }
    u32 getHeight() const override { return m_height; }

    bool m_initialized = false;
    u32 m_width = 0, m_height = 0;

    u32 m_viewportCalls = 0;
    struct { u32 x, y, w, h; } m_lastVp{};

    GSBlendMode m_lastBlend = GSBlendMode::None;
    GSDepthTest m_lastDepthTest = GSDepthTest::Always;
    bool m_depthWrite = true;
    GSAlphaTest m_lastAlphaTest = GSAlphaTest::Always;
    u8 m_alphaRef = 0, m_alphaMask = 0xFF;

    u32 m_fbCalls = 0;
    u32 m_lastFbp = 0, m_lastFbw = 0, m_lastPsm = 0;
    u32 m_zbCalls = 0;
    u32 m_lastZbp = 0, m_lastZpsm = 0;
    bool m_lastZmsk = false;
    u32 m_alphaCalls = 0;
    u32 m_lastAba = 0, m_lastAbb = 0, m_lastAbc = 0, m_lastAbd = 0, m_lastAfix = 0;

    TextureHandle m_nextTex = kNullTexture;
    TextureHandle m_boundTex = kNullTexture;
    u32 m_texSlot = 0;
    RenderTargetHandle m_nextRT = kNullRenderTarget;

    u32 m_drawPrimCalls = 0;
    GSPrimitive m_lastPrim = GSPrimitive::Point;
    RenderList m_lastList = RenderList::Opaque;
    u32 m_lastPrimCount = 0;

    u32 m_spriteCalls = 0;
    float m_lastSpriteX = 0, m_lastSpriteY = 0, m_lastSpriteW = 0, m_lastSpriteH = 0;
    float m_lastSpriteU0 = 0, m_lastSpriteV0 = 0, m_lastSpriteU1 = 0, m_lastSpriteV1 = 0;
    TextureHandle m_lastSpriteTex = kNullTexture;
    u8 m_lastSpriteR = 0, m_lastSpriteA = 0;

    u32 m_copyCalls = 0;
    float m_lastCopySrcX = 0, m_lastCopySrcY = 0;
    float m_lastCopyDstX = 0, m_lastCopyDstY = 0;
    float m_lastCopyW = 0, m_lastCopyH = 0;
};

static void test_packet_lifecycle() {
    TestBackend backend;
    backend.initialize(640, 448);
    GifPacketBridge bridge(backend);
    bridge.init(640, 448);

    assert(!bridge.checkOpen());
    bridge.startPacketPri(0);
    assert(bridge.checkOpen());
    bridge.endPacket();
    assert(!bridge.checkOpen());
}

static void test_closed_packet_is_noop() {
    TestBackend backend;
    backend.initialize(640, 448);
    GifPacketBridge bridge(backend);
    bridge.init(640, 448);

    bridge.sprite(10.0f, 20.0f, 30.0f, 40.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1, 2, 3, 4);
    bridge.setAlpha(0, 0, 1, 0);
    bridge.moveImage(0, 0, 0, 0, 16, 16);

    assert(bridge.commandBuffer().commandCount() == 0);
    assert(backend.m_spriteCalls == 0);
}

static void test_state_mapping() {
    TestBackend backend;
    backend.initialize(640, 448);
    GifPacketBridge bridge(backend);
    bridge.init(640, 448);
    bridge.startPacketPri(0);

    GsTex0 tex0{};
    tex0.setTbp0(123);
    bridge.setGsReg(kGsAddrTEX0_1, tex0.value);
    bridge.setAlpha(0, 0, 1, 0);
    bridge.setZWrite(1, 2);
    bridge.setDrawEnvironment(0.0f, 0.0f, 320.0f, 240.0f, 0, 0, 0);

    /* TEX0 + ALPHA + ZWRITE + DRAWENV(viewport+frame) = 5 commands */
    assert(bridge.commandBuffer().commandCount() == 5);

    const RenderCmd& texCmd = bridge.commandBuffer().command(0);
    assert(texCmd.type == RenderCommand::SetTexture);
    assert(texCmd.texture.tex == 123);
    assert(texCmd.texture.slot == 0);

    const RenderCmd& blendCmd = bridge.commandBuffer().command(1);
    assert(blendCmd.type == RenderCommand::SetBlendMode);
    assert(blendCmd.blendMode.mode == GSBlendMode::Alpha);

    const RenderCmd& depthCmd = bridge.commandBuffer().command(2);
    assert(depthCmd.type == RenderCommand::SetDepthTest);
    assert(depthCmd.depthTest.test == GSDepthTest::Less);
    assert(depthCmd.depthTest.write);

    const RenderCmd& viewportCmd = bridge.commandBuffer().command(3);
    assert(viewportCmd.type == RenderCommand::SetViewport);
    assert(viewportCmd.viewport.w == 320);
    assert(viewportCmd.viewport.h == 240);

    const RenderCmd& frameCmd = bridge.commandBuffer().command(4);
    assert(frameCmd.type == RenderCommand::SetFramebuffer);
}

static void test_set_gs_reg_expanded() {
    TestBackend backend;
    backend.initialize(640, 448);
    GifPacketBridge bridge(backend);
    bridge.init(640, 448);
    bridge.startPacketPri(0);

    /* FRAME register. fbw is a 6-bit field, so 640 (0x280) is masked to 0x20=32.
       fbp is a 9-bit field (bits 0-8), no overlap with fbw/psm. */
    GsFrame frame{};
    frame.setFbp(0x100);
    frame.setFbw(32);
    frame.setPsm(0);
    bridge.setGsReg(kGsAddrFRAME_1, frame.value);
    assert(bridge.commandBuffer().commandCount() == 1);
    const RenderCmd& fCmd = bridge.commandBuffer().command(0);
    assert(fCmd.type == RenderCommand::SetFramebuffer);
    assert(fCmd.framebuffer.fbp == 0x100);
    assert(fCmd.framebuffer.fbw == 32);
    assert(fCmd.framebuffer.psm == 0);
    bridge.commandBuffer().reset();

    /* ZBUF register (zbp=9bit field, psm=4bit field so <16) */
    GsZbuf zbuf{};
    zbuf.setZbp(0x100);
    zbuf.setPsm(2);
    zbuf.setZmsk(1);
    bridge.setGsReg(kGsAddrZBUF_1, zbuf.value);
    assert(bridge.commandBuffer().commandCount() == 1);
    const RenderCmd& zCmd = bridge.commandBuffer().command(0);
    assert(zCmd.type == RenderCommand::SetZBuffer);
    assert(zCmd.zbuffer.zbp == 0x100);
    assert(zCmd.zbuffer.psm == 2);
    assert(zCmd.zbuffer.zmsk == true);
    bridge.commandBuffer().reset();

    /* ALPHA register */
    GsAlpha alpha{};
    alpha.setAba(0);
    alpha.setAbb(1);
    alpha.setAbc(0);
    alpha.setAbd(0);
    alpha.setAfix(0x80);
    bridge.setGsReg(kGsAddrALPHA_1, alpha.value);
    assert(bridge.commandBuffer().commandCount() == 1);
    const RenderCmd& aCmd = bridge.commandBuffer().command(0);
    assert(aCmd.type == RenderCommand::SetAlpha);
    assert(aCmd.alpha.aba == 0);
    assert(aCmd.alpha.abb == 1);
    assert(aCmd.alpha.afix == 0x80);
    bridge.commandBuffer().reset();

    /* TEST register (zte=1, ztst=1 -> Always; ate=1, atst=2 -> Less) */
    GsTest test{};
    test.setZte(1);
    test.setZtst(1);
    test.setAte(1);
    test.setAtst(2);
    test.setAref(64);
    bridge.setGsReg(kGsAddrTEST_1, test.value);
    assert(bridge.commandBuffer().commandCount() == 2);
    const RenderCmd& dCmd = bridge.commandBuffer().command(0);
    assert(dCmd.type == RenderCommand::SetDepthTest);
    assert(dCmd.depthTest.test == GSDepthTest::Always);
    assert(dCmd.depthTest.write);
    const RenderCmd& atCmd = bridge.commandBuffer().command(1);
    assert(atCmd.type == RenderCommand::SetAlphaTest);
    assert(atCmd.alphaTest.test == GSAlphaTest::Less);
    assert(atCmd.alphaTest.ref == 64);
}

static void test_geometry_mapping() {
    TestBackend backend;
    backend.initialize(640, 448);
    GifPacketBridge bridge(backend);
    bridge.init(640, 448);
    bridge.startPacketPri(0);

    bridge.sprite(10.0f, 20.0f, 30.0f, 40.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5, 6, 7, 8);
    bridge.line(1.0f, 2.0f, 3.0f, 4.0f, 9, 10, 11, 12);
    bridge.point(5.0f, 6.0f, 13, 14, 15, 16);
    bridge.drawPolyF4(0.0f, 0.0f, 8.0f, 0.0f, 8.0f, 8.0f, 0.0f, 8.0f,
                      0.0f, 0.0f, 1.0f, 1.0f, 20, 21, 22, 23);
    bridge.moveImage(0.0f, 0.0f, 32.0f, 32.0f, 64.0f, 64.0f);

    assert(bridge.commandBuffer().commandCount() == 6);

    const RenderCmd& spriteCmd = bridge.commandBuffer().command(0);
    assert(spriteCmd.type == RenderCommand::DrawSprite);
    assert(spriteCmd.sprite.x == 10.0f);
    assert(spriteCmd.sprite.y == 20.0f);
    assert(spriteCmd.sprite.w == 30.0f);
    assert(spriteCmd.sprite.h == 40.0f);
    assert(spriteCmd.sprite.r == 5);
    assert(spriteCmd.sprite.a == 8);

    const RenderCmd& lineCmd = bridge.commandBuffer().command(1);
    assert(lineCmd.type == RenderCommand::DrawLine);
    assert(lineCmd.line.x0 == 1.0f);
    assert(lineCmd.line.y1 == 4.0f);
    assert(lineCmd.line.r == 9);

    const RenderCmd& pointCmd = bridge.commandBuffer().command(2);
    assert(pointCmd.type == RenderCommand::DrawPoint);
    assert(pointCmd.point.x == 5.0f);
    assert(pointCmd.point.y == 6.0f);
    assert(pointCmd.point.g == 14);

    const RenderCmd& tri0 = bridge.commandBuffer().command(3);
    const RenderCmd& tri1 = bridge.commandBuffer().command(4);
    assert(tri0.type == RenderCommand::DrawTriangle);
    assert(tri1.type == RenderCommand::DrawTriangle);
    assert(tri0.triangle.r == 20);
    assert(tri1.triangle.a == 23);

    const RenderCmd& copyCmd = bridge.commandBuffer().command(5);
    assert(copyCmd.type == RenderCommand::CopyTexture);
    assert(copyCmd.copy.srcX == 0.0f);
    assert(copyCmd.copy.srcY == 0.0f);
    assert(copyCmd.copy.dstX == 32.0f);
    assert(copyCmd.copy.dstY == 32.0f);
    assert(copyCmd.copy.w == 64.0f);
    assert(copyCmd.copy.h == 64.0f);
}

static void test_move_image_guards() {
    TestBackend backend;
    backend.initialize(640, 448);
    GifPacketBridge bridge(backend);
    bridge.init(640, 448);

    /* moveImage with packet closed is a no-op */
    bridge.moveImage(1.0f, 1.0f, 2.0f, 2.0f, 16.0f, 16.0f);
    assert(bridge.commandBuffer().commandCount() == 0);

    bridge.startPacketPri(0);

    /* degenerate (w or h <= 0) moves are dropped */
    bridge.moveImage(1.0f, 1.0f, 2.0f, 2.0f, 0.0f, 16.0f);
    bridge.moveImage(1.0f, 1.0f, 2.0f, 2.0f, 16.0f, -1.0f);
    assert(bridge.commandBuffer().commandCount() == 0);

    /* a valid move is emitted exactly once with full copy geometry */
    bridge.moveImage(4.0f, 8.0f, 12.0f, 16.0f, 32.0f, 64.0f);
    assert(bridge.commandBuffer().commandCount() == 1);
    const RenderCmd& c = bridge.commandBuffer().command(0);
    assert(c.type == RenderCommand::CopyTexture);
    assert(c.copy.srcX == 4.0f && c.copy.srcY == 8.0f);
    assert(c.copy.dstX == 12.0f && c.copy.dstY == 16.0f);
    assert(c.copy.w == 32.0f && c.copy.h == 64.0f);

    /* flush() forwards the copy to the backend with the recorded geometry */
    bridge.flush();
    assert(backend.m_copyCalls == 1);
    assert(backend.m_lastCopySrcX == 4.0f && backend.m_lastCopySrcY == 8.0f);
    assert(backend.m_lastCopyDstX == 12.0f && backend.m_lastCopyDstY == 16.0f);
    assert(backend.m_lastCopyW == 32.0f && backend.m_lastCopyH == 64.0f);
}

static void test_flush_executes() {
    TestBackend backend;
    backend.initialize(640, 448);
    GifPacketBridge bridge(backend);
    bridge.init(640, 448);

    bridge.startPacketPri(0);
    bridge.sprite(10.0f, 20.0f, 30.0f, 40.0f, 0.0f, 0.0f, 1.0f, 1.0f, 255, 255, 255, 255);
    assert(bridge.commandBuffer().commandCount() == 1);
    assert(backend.m_spriteCalls == 0);

    /* flush() dispatches buffered commands to the backend */
    bridge.flush();
    assert(backend.m_spriteCalls == 1);
    assert(backend.m_lastSpriteW == 30.0f);
    assert(backend.m_lastSpriteH == 40.0f);

    /* flush() resets the command buffer */
    assert(bridge.commandBuffer().commandCount() == 0);

    /* endPacket() flushes any remaining commands */
    bridge.sprite(1.0f, 1.0f, 2.0f, 2.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1, 1, 1, 1);
    assert(backend.m_spriteCalls == 1);
    bridge.endPacket();
    assert(backend.m_spriteCalls == 2);
    assert(!bridge.checkOpen());
}

static void test_flush_empty_is_noop() {
    TestBackend backend;
    backend.initialize(640, 448);
    GifPacketBridge bridge(backend);
    bridge.init(640, 448);

    bridge.startPacketPri(0);
    bridge.flush();
    assert(backend.m_spriteCalls == 0);
    assert(backend.m_drawPrimCalls == 0);
}

static void test_screen_check() {
    TestBackend backend;
    backend.initialize(640, 448);
    GifPacketBridge bridge(backend);
    bridge.init(640, 448);

    assert(bridge.isInScreen(0.0f, 0.0f, 10.0f, 10.0f));
    assert(!bridge.isInScreen(-100.0f, -100.0f, 10.0f, 10.0f));
}

int main() {
    test_packet_lifecycle();
    test_closed_packet_is_noop();
    test_state_mapping();
    test_set_gs_reg_expanded();
    test_geometry_mapping();
    test_move_image_guards();
    test_flush_executes();
    test_flush_empty_is_noop();
    test_screen_check();

    std::printf("gif_packet_test: all passed\n");
    return 0;
}
