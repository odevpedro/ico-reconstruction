#include "engine/GifPacket.h"

#include <algorithm>
#include <cstring>

namespace ico::engine {

GifPacketBridge::GifPacketBridge(RenderBackend& backend)
    : m_buffer{}
    , m_backend(backend)
    , m_executor(backend)
    , m_packetOpen(false)
    , m_currentPath(0)
    , m_screenWidth(kPs2ScreenWidth)
    , m_screenHeight(kPs2ScreenHeight)
{
}

void GifPacketBridge::init(u32 screenWidth, u32 screenHeight) {
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;
    m_buffer.setScreenSize(screenWidth, screenHeight);
    m_buffer.reset();
    m_packetOpen = false;
}

void GifPacketBridge::startPacketPri(u32 /* prim */) {
    m_packetOpen = true;
    m_currentPath = 0;
    m_buffer.reset();
}

void GifPacketBridge::startPacketPriPath1(u32 /* prim */) {
    m_packetOpen = true;
    m_currentPath = 1;
    m_buffer.reset();
}

bool GifPacketBridge::checkOpen() const {
    return m_packetOpen;
}

void GifPacketBridge::endPacket() {
    flush();
    m_packetOpen = false;
}

void GifPacketBridge::endPacketPath1() {
    flush();
    m_packetOpen = false;
}

void GifPacketBridge::setGsReg(u32 addr, u64 value) {
    if (!m_packetOpen) return;

    switch (addr) {
        case kGsAddrTEX0_1:
        case kGsAddrTEX0_2: {
            GsTex0 tex0{};
            tex0.value = value;

            RenderCmd cmd{};
            cmd.type = RenderCommand::SetTexture;
            cmd.texture.tex = tex0.tbp0();
            cmd.texture.slot = (addr == kGsAddrTEX0_2) ? 1u : 0u;
            m_buffer.commands().push_back(cmd);
            break;
        }
        case kGsAddrALPHA_1:
        case kGsAddrALPHA_2: {
            GsAlpha alpha{};
            alpha.value = static_cast<u32>(value & 0xFFFFFFFFu);
            RenderCmd cmd{};
            cmd.type = RenderCommand::SetAlpha;
            cmd.alpha.aba = alpha.aba();
            cmd.alpha.abb = alpha.abb();
            cmd.alpha.abc = alpha.abc();
            cmd.alpha.abd = alpha.abd();
            cmd.alpha.afix = alpha.afix();
            m_buffer.commands().push_back(cmd);
            break;
        }
        case kGsAddrTEST_1:
        case kGsAddrTEST_2: {
            GsTest test{};
            test.value = value;
            RenderCmd cmd{};
            cmd.type = RenderCommand::SetDepthTest;
            cmd.depthTest.test = static_cast<GSDepthTest>(test.ztst());
            cmd.depthTest.write = test.zte() != 0;
            m_buffer.commands().push_back(cmd);

            RenderCmd atestCmd{};
            atestCmd.type = RenderCommand::SetAlphaTest;
            atestCmd.alphaTest.test = static_cast<GSAlphaTest>(test.atst());
            atestCmd.alphaTest.ref = static_cast<u8>(test.aref());
            atestCmd.alphaTest.mask = 0xFF;
            m_buffer.commands().push_back(atestCmd);
            break;
        }
        case kGsAddrFRAME_1:
        case kGsAddrFRAME_2: {
            GsFrame frame{};
            frame.value = value;
            RenderCmd cmd{};
            cmd.type = RenderCommand::SetFramebuffer;
            cmd.framebuffer.fbp = frame.fbp();
            cmd.framebuffer.fbw = frame.fbw();
            cmd.framebuffer.psm = frame.psm();
            m_buffer.commands().push_back(cmd);
            break;
        }
        case kGsAddrZBUF_1:
        case kGsAddrZBUF_2: {
            GsZbuf zbuf{};
            zbuf.value = value;
            RenderCmd cmd{};
            cmd.type = RenderCommand::SetZBuffer;
            cmd.zbuffer.zbp = zbuf.zbp();
            cmd.zbuffer.psm = zbuf.psm();
            cmd.zbuffer.zmsk = zbuf.zmsk() != 0;
            m_buffer.commands().push_back(cmd);
            break;
        }
        default:
            break;
    }
}

void GifPacketBridge::setAlpha(u32 abc, u32 /* abd */, u32 abe, u32 /* abfix */) {
    if (!m_packetOpen) return;

    RenderCmd cmd{};
    cmd.type = RenderCommand::SetBlendMode;
    cmd.blendMode.mode = abe ? GSBlendMode::Alpha : GSBlendMode::None;
    (void)abc;
    m_buffer.commands().push_back(cmd);
}

void GifPacketBridge::setZTest(u32 ate, u32 atst, u32 /* aref */, u32 /* afail */) {
    if (!m_packetOpen) return;

    RenderCmd cmd{};
    cmd.type = RenderCommand::SetDepthTest;
    cmd.depthTest.test = static_cast<GSDepthTest>(ate ? atst : 0);
    cmd.depthTest.write = true;
    m_buffer.commands().push_back(cmd);
}

void GifPacketBridge::setZWrite(u32 zte, u32 ztst) {
    if (!m_packetOpen) return;

    RenderCmd cmd{};
    cmd.type = RenderCommand::SetDepthTest;
    cmd.depthTest.test = static_cast<GSDepthTest>(zte ? ztst : 0);
    cmd.depthTest.write = zte != 0;
    m_buffer.commands().push_back(cmd);
}

void GifPacketBridge::setDrawEnvironment(float /* x */, float /* y */, float w, float h,
                                         u32 fbp, u32 psm, u32 fbw) {
    if (!m_packetOpen) return;

    RenderCmd cmd{};
    cmd.type = RenderCommand::SetViewport;
    cmd.viewport.x = 0;
    cmd.viewport.y = 0;
    cmd.viewport.w = static_cast<u32>(w);
    cmd.viewport.h = static_cast<u32>(h);
    m_buffer.commands().push_back(cmd);

    RenderCmd frameCmd{};
    frameCmd.type = RenderCommand::SetFramebuffer;
    frameCmd.framebuffer.fbp = fbp;
    frameCmd.framebuffer.fbw = fbw;
    frameCmd.framebuffer.psm = psm;
    m_buffer.commands().push_back(frameCmd);
}

void GifPacketBridge::setHalfOffset(u32 /* h */, u32 /* v */) {
}

void GifPacketBridge::makeSprite(float x, float y, float w, float h,
                                 float u0, float v0, float u1, float v1) {
    emitSpriteQuad(x, y, w, h, u0, v0, u1, v1, 128, 128, 128, 128, true, false);
}

void GifPacketBridge::makeSpriteOffset(float x, float y, float w, float h,
                                       float u0, float v0, float u1, float v1) {
    emitSpriteQuad(x, y, w, h, u0, v0, u1, v1, 128, 128, 128, 128, true, false);
}

void GifPacketBridge::makeSpriteWithStrip(float x, float y, float w, float h,
                                          float u0, float v0, float u1, float v1) {
    emitSpriteQuad(x, y, w, h, u0, v0, u1, v1, 128, 128, 128, 128, true, false);
}

void GifPacketBridge::makeSpriteNoTexture(float x, float y, float w, float h) {
    emitSpriteQuad(x, y, w, h, 0, 0, 1, 1, 128, 128, 128, 128, false, false);
}

void GifPacketBridge::makeSpriteNoTextureOffset(float x, float y, float w, float h) {
    emitSpriteQuad(x, y, w, h, 0, 0, 1, 1, 128, 128, 128, 128, false, false);
}

void GifPacketBridge::makeLine2D(float x0, float y0, float x1, float y1) {
    emitLine(x0, y0, x1, y1, 128, 128, 128, 128);
}

void GifPacketBridge::makeLine2DOffset(float x0, float y0, float x1, float y1) {
    emitLine(x0, y0, x1, y1, 128, 128, 128, 128);
}

void GifPacketBridge::makePoint2D(float x, float y) {
    emitPoint(x, y, 128, 128, 128, 128);
}

void GifPacketBridge::makePoint2DOffset(float x, float y) {
    emitPoint(x, y, 128, 128, 128, 128);
}

void GifPacketBridge::sprite(float x, float y, float w, float h,
                             float u0, float v0, float u1, float v1,
                             u8 r, u8 g, u8 b, u8 a) {
    emitSpriteQuad(x, y, w, h, u0, v0, u1, v1, r, g, b, a, true, false);
}

void GifPacketBridge::spriteOffset(float x, float y, float w, float h,
                                   float u0, float v0, float u1, float v1,
                                   u8 r, u8 g, u8 b, u8 a) {
    emitSpriteQuad(x, y, w, h, u0, v0, u1, v1, r, g, b, a, true, false);
}

void GifPacketBridge::spriteOrg(float x, float y, float w, float h,
                                float u0, float v0, float u1, float v1,
                                u8 r, u8 g, u8 b, u8 a) {
    emitSpriteQuad(x, y, w, h, u0, v0, u1, v1, r, g, b, a, true, false);
}

void GifPacketBridge::spriteSensitive(float x, float y, float w, float h,
                                      float u0, float v0, float u1, float v1,
                                      u8 r, u8 g, u8 b, u8 a) {
    emitSpriteQuad(x, y, w, h, u0, v0, u1, v1, r, g, b, a, true, false);
}

void GifPacketBridge::spriteSensitiveOffset(float x, float y, float w, float h,
                                            float u0, float v0, float u1, float v1,
                                            u8 r, u8 g, u8 b, u8 a) {
    emitSpriteQuad(x, y, w, h, u0, v0, u1, v1, r, g, b, a, true, false);
}

void GifPacketBridge::spriteSensitiveOrg(float x, float y, float w, float h,
                                         float u0, float v0, float u1, float v1,
                                         u8 r, u8 g, u8 b, u8 a) {
    emitSpriteQuad(x, y, w, h, u0, v0, u1, v1, r, g, b, a, true, false);
}

void GifPacketBridge::drawPolyF4(float x0, float y0, float x1, float y1,
                                 float x2, float y2, float x3, float y3,
                                 float u0, float v0, float u1, float v1,
                                 u8 r, u8 g, u8 b, u8 a) {
    emitTriangle(x0, y0, x1, y1, x2, y2, u0, v0, u1, v1, u1, v1, r, g, b, a);
    emitTriangle(x2, y2, x3, y3, x0, y0, u1, v1, u0, v0, u0, v0, r, g, b, a);
}

void GifPacketBridge::drawStripF(const std::vector<std::array<float, 2>>& vertices,
                                 float u0, float v0, float u1, float v1,
                                 u8 r, u8 g, u8 b, u8 a) {
    if (vertices.size() < 2) return;

    float minX = vertices[0][0];
    float maxX = vertices[0][0];
    for (const auto& v : vertices) {
        minX = std::min(minX, v[0]);
        maxX = std::max(maxX, v[0]);
    }
    float range = maxX - minX;
    if (range < 0.001f) range = 1.0f;

    for (size_t i = 1; i < vertices.size(); ++i) {
        float t0 = (vertices[i - 1][0] - minX) / range;
        float t1 = (vertices[i][0] - minX) / range;
        float uA = u0 + t0 * (u1 - u0);
        float vA = v0 + t0 * (v1 - v0);
        float uB = u0 + t1 * (u1 - u0);
        float vB = v0 + t1 * (v1 - v0);
        emitTriangle(
            vertices[i - 1][0], vertices[i - 1][1],
            vertices[i][0], vertices[i][1],
            vertices[i][0], vertices[i][1] + 1.0f,
            uA, vA, uB, vB, uB, vB, r, g, b, a);
    }
}

void GifPacketBridge::drawStripG(const std::vector<std::array<float, 2>>& vertices,
                                 const std::vector<std::array<u8, 4>>& colors) {
    if (vertices.size() < 2 || colors.size() < 2) return;

    for (size_t i = 1; i < vertices.size() && i < colors.size(); ++i) {
        emitLine(vertices[i - 1][0], vertices[i - 1][1],
                 vertices[i][0], vertices[i][1],
                 colors[i - 1][0], colors[i - 1][1], colors[i - 1][2], colors[i - 1][3],
                 true, colors[i - 1], colors[i]);
    }
}

void GifPacketBridge::draw2DStripG(const std::vector<std::array<float, 2>>& vertices,
                                   const std::vector<std::array<u8, 4>>& colors) {
    drawStripG(vertices, colors);
}

void GifPacketBridge::draw2DUVStripG(const std::vector<std::array<float, 2>>& vertices,
                                     const std::vector<std::array<float, 2>>& /* uvs */,
                                     const std::vector<std::array<u8, 4>>& colors) {
    drawStripG(vertices, colors);
}

void GifPacketBridge::line(float x0, float y0, float x1, float y1,
                           u8 r, u8 g, u8 b, u8 a) {
    emitLine(x0, y0, x1, y1, r, g, b, a);
}

void GifPacketBridge::lineOffset(float x0, float y0, float x1, float y1,
                                 u8 r, u8 g, u8 b, u8 a) {
    emitLine(x0, y0, x1, y1, r, g, b, a);
}

void GifPacketBridge::point(float x, float y, u8 r, u8 g, u8 b, u8 a) {
    emitPoint(x, y, r, g, b, a);
}

void GifPacketBridge::pointOffset(float x, float y, u8 r, u8 g, u8 b, u8 a) {
    emitPoint(x, y, r, g, b, a);
}

void GifPacketBridge::moveImage(float /* srcX */, float /* srcY */, float /* dstX */, float /* dstY */,
                                float w, float h) {
    if (!m_packetOpen || w <= 0.0f || h <= 0.0f) return;

    RenderCmd cmd{};
    cmd.type = RenderCommand::CopyTexture;
    m_buffer.commands().push_back(cmd);
}

bool GifPacketBridge::isInScreen(float x, float y, float w, float h) const {
    return x + w > 0 && x < static_cast<float>(m_screenWidth)
        && y + h > 0 && y < static_cast<float>(m_screenHeight);
}

GifCommandBuffer& GifPacketBridge::commandBuffer() { return m_buffer; }
const GifCommandBuffer& GifPacketBridge::commandBuffer() const { return m_buffer; }

void GifPacketBridge::flush() {
    if (m_buffer.commandCount() == 0 || !m_packetOpen) {
        return;
    }
    m_executor.execute(m_buffer);
    m_buffer.reset();
}

RenderBackend& GifPacketBridge::backend() const {
    return m_backend;
}

void GifPacketBridge::emitSpriteQuad(float x, float y, float w, float h,
                                     float u0, float v0, float u1, float v1,
                                     u8 r, u8 g, u8 b, u8 a, bool textured, bool gouraud,
                                     const std::array<std::array<u8, 4>, 4>& cornerColors) {
    if (!m_packetOpen) return;

    if (gouraud) {
        RenderCmd cmd{};
        cmd.type = RenderCommand::DrawSpriteGouraud;
        cmd.spriteGouraud.x = x;
        cmd.spriteGouraud.y = y;
        cmd.spriteGouraud.w = w;
        cmd.spriteGouraud.h = h;
        cmd.spriteGouraud.u0 = u0;
        cmd.spriteGouraud.v0 = v0;
        cmd.spriteGouraud.u1 = u1;
        cmd.spriteGouraud.v1 = v1;
        cmd.spriteGouraud.texture = textured ? 1 : 0;
        for (u32 c = 0; c < 4; ++c) {
            for (u32 ch = 0; ch < 4; ++ch) {
                cmd.spriteGouraud.corners[c][ch] = cornerColors[c][ch];
            }
        }
        m_buffer.commands().push_back(cmd);
    } else {
        RenderCmd cmd{};
        cmd.type = RenderCommand::DrawSprite;
        cmd.sprite.x = x;
        cmd.sprite.y = y;
        cmd.sprite.w = w;
        cmd.sprite.h = h;
        cmd.sprite.u0 = u0;
        cmd.sprite.v0 = v0;
        cmd.sprite.u1 = u1;
        cmd.sprite.v1 = v1;
        cmd.sprite.texture = textured ? 1 : 0;
        cmd.sprite.r = r;
        cmd.sprite.g = g;
        cmd.sprite.b = b;
        cmd.sprite.a = a;
        m_buffer.commands().push_back(cmd);
    }
}

void GifPacketBridge::emitLine(float x0, float y0, float x1, float y1,
                               u8 r, u8 g, u8 b, u8 a, bool gouraud,
                               const std::array<u8, 4>& /* color0 */,
                               const std::array<u8, 4>& /* color1 */) {
    if (!m_packetOpen) return;

    RenderCmd cmd{};
    cmd.type = RenderCommand::DrawLine;
    cmd.line.x0 = x0;
    cmd.line.y0 = y0;
    cmd.line.x1 = x1;
    cmd.line.y1 = y1;
    cmd.line.r = r;
    cmd.line.g = g;
    cmd.line.b = b;
    cmd.line.a = a;
    cmd.line.gouraud = gouraud ? 1 : 0;
    m_buffer.commands().push_back(cmd);
}

void GifPacketBridge::emitPoint(float x, float y, u8 r, u8 g, u8 b, u8 a) {
    if (!m_packetOpen) return;

    RenderCmd cmd{};
    cmd.type = RenderCommand::DrawPoint;
    cmd.point.x = x;
    cmd.point.y = y;
    cmd.point.r = r;
    cmd.point.g = g;
    cmd.point.b = b;
    cmd.point.a = a;
    m_buffer.commands().push_back(cmd);
}

void GifPacketBridge::emitTriangle(float x0, float y0, float x1, float y1,
                                   float x2, float y2,
                                   float u0, float v0, float u1, float v1,
                                   float u2, float v2,
                                   u8 r, u8 g, u8 b, u8 a) {
    if (!m_packetOpen) return;

    RenderCmd cmd{};
    cmd.type = RenderCommand::DrawTriangle;
    cmd.triangle.x0 = x0;
    cmd.triangle.y0 = y0;
    cmd.triangle.x1 = x1;
    cmd.triangle.y1 = y1;
    cmd.triangle.x2 = x2;
    cmd.triangle.y2 = y2;
    cmd.triangle.u0 = u0;
    cmd.triangle.v0 = v0;
    cmd.triangle.u1 = u1;
    cmd.triangle.v1 = v1;
    cmd.triangle.u2 = u2;
    cmd.triangle.v2 = v2;
    cmd.triangle.r = r;
    cmd.triangle.g = g;
    cmd.triangle.b = b;
    cmd.triangle.a = a;
    m_buffer.commands().push_back(cmd);
}

} // namespace ico::engine
