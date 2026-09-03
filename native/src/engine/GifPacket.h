#pragma once

#include "engine/GifCommandBuffer.h"
#include "engine/GifCommandExecutor.h"
#include "engine/GifTag.h"
#include "engine/RenderBackend.h"

#include <array>
#include <cstdint>
#include <vector>

namespace ico::engine {

class GifPacketBridge {
public:
    explicit GifPacketBridge(RenderBackend& backend);

    void init(u32 screenWidth, u32 screenHeight);

    void startPacketPri(u32 prim);
    void startPacketPriPath1(u32 prim);
    bool checkOpen() const;
    void endPacket();
    void endPacketPath1();

    void setGsReg(u32 addr, u64 value);
    void setAlpha(u32 abc, u32 abd, u32 abe, u32 abfix);
    void setZTest(u32 ate, u32 atst, u32 aref, u32 afail);
    void setZWrite(u32 zte, u32 ztst);
    void setDrawEnvironment(float x, float y, float w, float h, u32 fbp, u32 psm, u32 fbw);
    void setHalfOffset(u32 h, u32 v);

    void makeSprite(float x, float y, float w, float h, float u0, float v0, float u1, float v1);
    void makeSpriteOffset(float x, float y, float w, float h, float u0, float v0, float u1, float v1);
    void makeSpriteWithStrip(float x, float y, float w, float h, float u0, float v0, float u1, float v1);
    void makeSpriteNoTexture(float x, float y, float w, float h);
    void makeSpriteNoTextureOffset(float x, float y, float w, float h);
    void makeLine2D(float x0, float y0, float x1, float y1);
    void makeLine2DOffset(float x0, float y0, float x1, float y1);
    void makePoint2D(float x, float y);
    void makePoint2DOffset(float x, float y);

    void sprite(float x, float y, float w, float h, float u0, float v0, float u1, float v1,
                u8 r, u8 g, u8 b, u8 a);
    void spriteOffset(float x, float y, float w, float h, float u0, float v0, float u1, float v1,
                      u8 r, u8 g, u8 b, u8 a);
    void spriteOrg(float x, float y, float w, float h, float u0, float v0, float u1, float v1,
                   u8 r, u8 g, u8 b, u8 a);
    void spriteSensitive(float x, float y, float w, float h, float u0, float v0, float u1, float v1,
                         u8 r, u8 g, u8 b, u8 a);
    void spriteSensitiveOffset(float x, float y, float w, float h, float u0, float v0, float u1, float v1,
                               u8 r, u8 g, u8 b, u8 a);
    void spriteSensitiveOrg(float x, float y, float w, float h, float u0, float v0, float u1, float v1,
                            u8 r, u8 g, u8 b, u8 a);

    void drawPolyF4(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3,
                    float u0, float v0, float u1, float v1, u8 r, u8 g, u8 b, u8 a);
    void drawStripF(const std::vector<std::array<float, 2>>& vertices,
                    float u0, float v0, float u1, float v1, u8 r, u8 g, u8 b, u8 a);
    void drawStripG(const std::vector<std::array<float, 2>>& vertices,
                    const std::vector<std::array<u8, 4>>& colors);
    void draw2DStripG(const std::vector<std::array<float, 2>>& vertices,
                      const std::vector<std::array<u8, 4>>& colors);
    void draw2DUVStripG(const std::vector<std::array<float, 2>>& vertices,
                        const std::vector<std::array<float, 2>>& uvs,
                        const std::vector<std::array<u8, 4>>& colors);

    void line(float x0, float y0, float x1, float y1, u8 r, u8 g, u8 b, u8 a);
    void lineOffset(float x0, float y0, float x1, float y1, u8 r, u8 g, u8 b, u8 a);
    void point(float x, float y, u8 r, u8 g, u8 b, u8 a);
    void pointOffset(float x, float y, u8 r, u8 g, u8 b, u8 a);

    void moveImage(float srcX, float srcY, float dstX, float dstY, float w, float h);
    bool isInScreen(float x, float y, float w, float h) const;

    GifCommandBuffer& commandBuffer();
    const GifCommandBuffer& commandBuffer() const;

    void flush();
    RenderBackend& backend() const;

private:
    void emitSpriteQuad(float x, float y, float w, float h,
                        float u0, float v0, float u1, float v1,
                        u8 r, u8 g, u8 b, u8 a, bool textured, bool gouraud,
                        const std::array<std::array<u8, 4>, 4>& cornerColors = {});

    void emitLine(float x0, float y0, float x1, float y1,
                  u8 r, u8 g, u8 b, u8 a, bool gouraud = false,
                  const std::array<u8, 4>& color0 = {}, const std::array<u8, 4>& color1 = {});

    void emitPoint(float x, float y, u8 r, u8 g, u8 b, u8 a);

    void emitTriangle(float x0, float y0, float x1, float y1, float x2, float y2,
                      float u0, float v0, float u1, float v1, float u2, float v2,
                      u8 r, u8 g, u8 b, u8 a);

    GifCommandBuffer m_buffer;
    RenderBackend& m_backend;
    GifCommandExecutor m_executor;
    bool m_packetOpen;
    u32 m_currentPath;

    u32 m_screenWidth;
    u32 m_screenHeight;
};

} // namespace ico::engine
