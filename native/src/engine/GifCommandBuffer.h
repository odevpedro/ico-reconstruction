#pragma once

#include "engine/GifTag.h"
#include "engine/RenderBackend.h"

#include <array>
#include <cstdint>
#include <vector>

namespace ico::engine {

enum class GifTransferMode : u8 {
    Direct,
    DirectHl,
};

struct GifPacketHeader {
    GifTag tag;
    u16 totalQwords;
    GifTransferMode mode;
};

struct SpriteDesc {
    float x, y, w, h;
    float u0, v0, u1, v1;
    u8 r, g, b, a;
    bool gouraud;
    std::array<std::array<u8, 4>, 4> cornerColors;
};

struct LineDesc {
    float x0, y0, x1, y1;
    u8 r, g, b, a;
    bool gouraud;
};

struct PointDesc {
    float x, y;
    u8 r, g, b, a;
};

struct TileDesc {
    float x, y, w, h;
    u8 r, g, b, a;
};

class GifCommandBuffer {
public:
    GifCommandBuffer();

    void reset();

    bool parsePacket(const u8* data, u32 size);

    u32 commandCount() const;
    const RenderCmd& command(u32 index) const;
    std::vector<RenderCmd>& commands();
    const std::vector<RenderCmd>& commands() const;

    void setScreenSize(u32 width, u32 height);

    GSBlendMode currentBlendMode() const;
    TextureHandle currentTexture() const;
    bool currentDepthWrite() const;
    GSDepthTest currentDepthTest() const;

    /* VRAM texture upload model: after a host->VRAM IMAGE transfer resolved by
       a matching TEX0, exposes the decoded RGBA8 plane for a texture handle so a
       renderer can create a real texture. */
    bool uploadedTexture(TextureHandle handle, u32& width, u32& height,
                         TextureFormat& format, std::vector<u8>& rgba) const;
    std::size_t uploadedTextureCount() const;

private:
    void processTag(const GifTag& tag, const u8* data, u32 dataSize);
    void processPackedData(const GifTag& tag, const u8* data, u32 dataSize);
    void processRegsData(const GifTag& tag, const u8* data, u32 dataSize);
    void processImageLoop(const GifTag& tag, const u8* data, u32 dataSize);

    void dispatchReg(GifReg reg, const u8* regData);
    void dispatchByAddress(u8 addr, const u8* regData);
    void handleAD(const u8* data);

    void handlePrim(const u8* data);
    void handleRgbaq(const u8* data);
    void handleStq(const u8* data);
    void handleUv(const u8* data);
    void handleXyz2(const u8* data);
    void handleTex0(const u8* data);
    void handleTex1(const u8* data);
    void handleAlpha(const u8* data);
    void handleTest(const u8* data);
    void handleFrame(const u8* data);
    void handleZbuf(const u8* data);
    void handleBitbltBuf(const u8* data);
    void handleTrxPos(const u8* data);
    void handleTrxReg(const u8* data);
    void handleTrxDir(const u8* data);
    void handleFog(const u8* data);
    void handleNop(const u8* data);

    void emitSprite();
    void emitGouraudSprite();
    void emitLine();
    void emitPoint();
    void emitTile();

    RenderCmd makeRenderCmd(RenderCommand type) const;

    u32 m_screenWidth;
    u32 m_screenHeight;
    std::vector<RenderCmd> m_commands;

    GsPrimReg m_currentPrim;
    GsRgbaq m_currentRgbaq;
    GsStq m_currentStq;
    u32 m_currentUv;
    u64 m_currentXyz;
    GsTex0 m_currentTex0;
    GsTex1 m_currentTex1;
    GsAlpha m_currentAlpha;
    GsTest m_currentTest;
    GsFrame m_currentFrame;
    GsZbuf m_currentZbuf;
    GsFog m_currentFog;

    bool m_hasPrim;
    bool m_hasRgbaq;
    bool m_hasStq;
    bool m_hasUv;
    bool m_hasXyz;

    std::array<float, 2> m_spritePos;
    std::array<float, 2> m_spriteSize;
    std::array<float, 2> m_spriteUv0;
    std::array<float, 2> m_spriteUv1;
    u32 m_spriteVertexCount;
    std::vector<std::array<float, 2>> m_spriteVertices;
    std::vector<std::array<float, 2>> m_spriteUvs;
    std::vector<GsRgbaq> m_spriteColors;

    /* GS block-transfer state (PS2Tek GS registers 0x50-0x53). */
    GsBitbltBuf m_transferBitbltBuf;
    GsTrxPos m_transferTrxPos;
    GsTrxReg m_transferTrxReg;
    GsTrxDir m_transferTrxDir;

    /* Decoded host->VRAM texture uploads, keyed by an opaque handle. */
    struct UploadedTexture {
        TextureHandle handle;
        u32 tbp0;
        u32 width;
        u32 height;
        TextureFormat format;
        std::vector<u8> rgba; /* RGBA8 plane, row-major */
    };
    std::vector<UploadedTexture> m_uploads;
    TextureHandle m_nextTextureHandle;
};

} // namespace ico::engine
