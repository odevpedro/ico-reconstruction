#include "engine/GifCommandBuffer.h"

#include <cstring>

namespace ico::engine {

GifCommandBuffer::GifCommandBuffer()
    : m_screenWidth(kPs2ScreenWidth)
    , m_screenHeight(kPs2ScreenHeight)
    , m_currentPrim{}
    , m_currentRgbaq{}
    , m_currentStq{}
    , m_currentUv(0)
    , m_currentXyz(0)
    , m_currentTex0{}
    , m_currentTex1{}
    , m_currentAlpha{}
    , m_currentTest{}
    , m_currentFrame{}
    , m_currentZbuf{}
    , m_currentFog{}
    , m_hasPrim(false)
    , m_hasRgbaq(false)
    , m_hasStq(false)
    , m_hasUv(false)
    , m_hasXyz(false)
    , m_spritePos{}
    , m_spriteSize{}
    , m_spriteUv0{}
    , m_spriteUv1{}
    , m_spriteVertexCount(0)
    , m_transferBitbltBuf{}
    , m_transferTrxPos{}
    , m_transferTrxReg{}
    , m_transferTrxDir{}
    , m_nextTextureHandle(0)
{
}

void GifCommandBuffer::reset() {
    m_commands.clear();
    m_currentPrim = GsPrimReg{};
    m_currentRgbaq = GsRgbaq{};
    m_currentStq = GsStq{};
    m_currentUv = 0;
    m_currentXyz = 0;
    m_currentTex0 = GsTex0{};
    m_currentTex1 = GsTex1{};
    m_currentAlpha = GsAlpha{};
    m_currentTest = GsTest{};
    m_currentFrame = GsFrame{};
    m_currentZbuf = GsZbuf{};
    m_currentFog = GsFog{};
    m_hasPrim = false;
    m_hasRgbaq = false;
    m_hasStq = false;
    m_hasUv = false;
    m_hasXyz = false;
    m_spriteVertexCount = 0;
    m_spriteVertices.clear();
    m_spriteUvs.clear();
    m_spriteColors.clear();
    m_transferBitbltBuf = GsBitbltBuf{};
    m_transferTrxPos = GsTrxPos{};
    m_transferTrxReg = GsTrxReg{};
    m_transferTrxDir = GsTrxDir{};
    m_uploads.clear();
    m_nextTextureHandle = 0;
}

bool GifCommandBuffer::parsePacket(const u8* data, u32 size) {
    if (size < kGifTagSize) {
        return false;
    }

    u32 offset = 0;
    while (offset + kGifTagSize <= size) {
        GifTag tag{};
        std::memcpy(tag.bytes, data + offset, kGifTagSize);
        offset += kGifTagSize;

        u16 nloop = tag.nloop();
        bool eop = tag.eop();

        if (nloop == 0 && eop) {
            break;
        }

        /* Data size depends on the transfer mode (PS2Tek GIF Data Formats):
           - Packed/Unpacked: NLOOP * NREG quadwords
           - Regs (REGLIST):  NLOOP * NREG doublewords
           - Image:           NLOOP quadwords */
        u32 regsPerLoop = tag.regCount();
        u32 packetBytes;
        switch (tag.flg()) {
            case GifFlg::Regs:
                packetBytes = static_cast<u32>(nloop) * regsPerLoop * 8;
                break;
            case GifFlg::Image:
                packetBytes = static_cast<u32>(nloop) * kGifTagSize;
                break;
            case GifFlg::Packed:
            case GifFlg::Unpacked:
            default:
                packetBytes = static_cast<u32>(nloop) * kGifTagSize;
                break;
        }
        if (offset + packetBytes > size) {
            packetBytes = size - offset;
        }

        processTag(tag, data + offset, packetBytes);
        offset += packetBytes;

        if (eop) {
            break;
        }
    }

    return true;
}

void GifCommandBuffer::processTag(const GifTag& tag, const u8* data, u32 dataSize) {
    GifFlg flg = tag.flg();
    if (flg == GifFlg::Packed) {
        processPackedData(tag, data, dataSize);
    } else if (flg == GifFlg::Regs) {
        processRegsData(tag, data, dataSize);
    } else if (flg == GifFlg::Image) {
        processImageLoop(tag, data, dataSize);
    }
}

void GifCommandBuffer::processPackedData(const GifTag& tag, const u8* data, u32 dataSize) {
    if (tag.pre()) {
        m_currentPrim.value = tag.prim();
        m_hasPrim = true;
    }

    u32 nreg = tag.regCount();
    if (nreg == 0) return;

    u32 offset = 0;

    while (offset + kGifTagSize <= dataSize) {
        for (u32 r = 0; r < nreg && offset + kGifTagSize <= dataSize; ++r) {
            GifReg reg = tag.reg(r);
            dispatchReg(reg, data + offset);
            offset += kGifTagSize;
        }
    }
}

void GifCommandBuffer::processRegsData(const GifTag& tag, const u8* data, u32 dataSize) {
    /* REGLIST (REGS): PS2Tek "GIF Data Formats" — each data element is one
       doubleword (8 bytes), routed to the register named by the tag's
       descriptor list. Total elements = NREGS * NLOOP. */
    if (tag.pre()) {
        m_currentPrim.value = tag.prim();
        m_hasPrim = true;
    }

    u32 nreg = tag.regCount();
    if (nreg == 0) return;

    u32 offset = 0;
    while (offset + 8 <= dataSize) {
        for (u32 r = 0; r < nreg && offset + 8 <= dataSize; ++r) {
            GifReg reg = tag.reg(r);
            dispatchReg(reg, data + offset);
            offset += 8;
        }
    }
}

void GifCommandBuffer::dispatchReg(GifReg reg, const u8* regData) {
    switch (reg) {
        case GifReg::PRIM:   handlePrim(regData); break;
        case GifReg::RGBAQ:  handleRgbaq(regData); break;
        case GifReg::STQ:    handleStq(regData); break;
        case GifReg::UV:     handleUv(regData); break;
        case GifReg::XYZ2:   handleXyz2(regData); break;
        case GifReg::XYZ3:   handleXyz2(regData); break;
        case GifReg::TEX0_1: handleTex0(regData); break;
        case GifReg::TEX0_2: handleTex0(regData); break;
        case GifReg::TEX1_1: handleTex1(regData); break;
        case GifReg::TEX1_2: handleTex1(regData); break;
        case GifReg::A_D:    handleAD(regData); break;
        case GifReg::NOP:    handleNop(regData); break;
        default: break;
    }
}

void GifCommandBuffer::handleAD(const u8* data) {
    /* A+D (reg=Eh): the data quadword embeds the target register address in
       byte 8, data in bytes 0-7. (PS2Tek GIF Data Formats, PCSX2
       Gif_HandlerAD). */
    u8 addr = data[8];
    dispatchByAddress(addr, data);
}

void GifCommandBuffer::dispatchByAddress(u8 addr, const u8* regData) {
    /* Map a raw GS register address (PS2Tek GS Register List) to a handler.
       The low 5 bits (plus a few 6-bit page-2 addresses) cover the registers
       the buffer understands. Unhandled addresses are ignored (VRAM/bitblt
       transfer registers are not modeled yet). */
    switch (addr) {
        case kGsAddrPRIM:    handlePrim(regData); break;
        case kGsAddrRGBAQ:   handleRgbaq(regData); break;
        case kGsAddrSTQ:     handleStq(regData); break;
        case kGsAddrUV:      handleUv(regData); break;
        case kGsAddrXYZF2:   handleXyz2(regData); break;
        case kGsAddrXYZ2:    handleXyz2(regData); break;
        case kGsAddrXYZF3:   handleXyz2(regData); break;
        case kGsAddrXYZ3:    handleXyz2(regData); break;
        case kGsAddrTEX0_1:  handleTex0(regData); break;
        case kGsAddrTEX0_2:  handleTex0(regData); break;
        case kGsAddrTEX1_1:  handleTex1(regData); break;
        case kGsAddrTEX1_2:  handleTex1(regData); break;
        case kGsAddrFOG:     handleFog(regData); break;
        case kGsAddrALPHA_1: handleAlpha(regData); break;
        case kGsAddrALPHA_2: handleAlpha(regData); break;
        case kGsAddrTEST_1:  handleTest(regData); break;
        case kGsAddrTEST_2:  handleTest(regData); break;
        case kGsAddrFRAME_1: handleFrame(regData); break;
        case kGsAddrFRAME_2: handleFrame(regData); break;
        case kGsAddrZBUF_1:  handleZbuf(regData); break;
        case kGsAddrZBUF_2:  handleZbuf(regData); break;
        case kGsAddrBITBLTBUF: handleBitbltBuf(regData); break;
        case kGsAddrTRXPOS:    handleTrxPos(regData); break;
        case kGsAddrTRXREG:    handleTrxReg(regData); break;
        case kGsAddrTRXDIR:    handleTrxDir(regData); break;
        default: break;
    }
}

void GifCommandBuffer::processImageLoop(const GifTag& tag, const u8* data, u32 dataSize) {
    /* IMAGE: PS2Tek "GIF Data Formats" — raw raster upload. Total data =
       NLOOP quadwords, written to the GS HWREG / VRAM write pointer. The
       destination comes from BITBLTBUF/TRXPOS/TRXREG registers. Rev.118 models
       the host->VRAM direction (TRXDIR xdir=0) as a decoded RGBA8 texture and
       resolves it to a texture handle via TEX0.tbp0. */
    u32 nloop = tag.nloop();
    u32 expected = nloop * kGifTagSize;
    if (dataSize < expected) {
        return;
    }

    if (m_transferTrxDir.xdir() != 0) {
        /* Only host->VRAM (xdir=0) is modeled. xdir=1 (local->host) and
           xdir=2 (local->local) are outside this model. */
        return;
    }

    if (m_transferBitbltBuf.dpsm() != static_cast<u32>(TextureFormat::PSMCT32)) {
        /* Only PSMCT32 host->VRAM uploads are decoded (Rev.118). Other PSMs
           (16/8/4, indexed palettes) remain unmodeled. */
        return;
    }

    u32 w = m_transferTrxReg.rrw();
    u32 h = m_transferTrxReg.rrh();
    if (w == 0 || h == 0 || w > 512 || h > 512) {
        return;
    }

    std::size_t pixelCount = static_cast<std::size_t>(w) * h;
    std::vector<u8> rgba(pixelCount * 4, 0);

    u32 quadwords = nloop;
    std::size_t pixelIndex = 0;
    for (u32 qi = 0; qi < quadwords; ++qi) {
        const u8* qw = data + static_cast<std::size_t>(qi) * kGifTagSize;
        u32 px[4];
        std::memcpy(px, qw, sizeof(px));
        for (u32 pi = 0; pi < 4; ++pi) {
            if (pixelIndex >= pixelCount) {
                break;
            }
            u32 c = px[pi];
            std::size_t o = pixelIndex * 4;
            rgba[o + 0] = static_cast<u8>(c & 0xFF);
            rgba[o + 1] = static_cast<u8>((c >> 8) & 0xFF);
            rgba[o + 2] = static_cast<u8>((c >> 16) & 0xFF);
            rgba[o + 3] = static_cast<u8>((c >> 24) & 0xFF);
            ++pixelIndex;
        }
    }

    UploadedTexture tex;
    tex.handle = ++m_nextTextureHandle;
    tex.tbp0 = m_transferBitbltBuf.dbase();
    tex.width = w;
    tex.height = h;
    tex.format = TextureFormat::PSMCT32;
    tex.rgba = std::move(rgba);
    m_uploads.push_back(std::move(tex));
}

void GifCommandBuffer::handlePrim(const u8* data) {
    u32 primVal = 0;
    std::memcpy(&primVal, data, 4);
    m_currentPrim.value = primVal & 0x7FF;
    m_hasPrim = true;
}

void GifCommandBuffer::handleRgbaq(const u8* data) {
    u64 val = 0;
    std::memcpy(&val, data, 8);
    m_currentRgbaq.value = val;
    m_hasRgbaq = true;
}

void GifCommandBuffer::handleStq(const u8* data) {
    u64 val = 0;
    std::memcpy(&val, data, 8);
    m_currentStq.value = val;
    m_hasStq = true;
}

void GifCommandBuffer::handleUv(const u8* data) {
    u32 val = 0;
    std::memcpy(&val, data, 4);
    m_currentUv = val;
    m_hasUv = true;

    float u = static_cast<float>(val & 0xFFFF) / 16.0f;
    float v = static_cast<float>((val >> 16) & 0xFFFF) / 16.0f;

    GsPrimType primType = m_currentPrim.type();
    if (primType == GsPrimType::Sprite) {
        if (m_spriteVertexCount == 0) {
            m_spriteUv0[0] = u;
            m_spriteUv0[1] = v;
        } else {
            m_spriteUv1[0] = u;
            m_spriteUv1[1] = v;
        }
    } else {
        m_spriteUvs.push_back({u, v});
    }
}

void GifCommandBuffer::handleXyz2(const u8* data) {
    u64 val = 0;
    std::memcpy(&val, data, 8);
    m_currentXyz = val;
    m_hasXyz = true;

    GsXyz2 xyz{};
    xyz.value = val;

    GsPrimType primType = m_currentPrim.type();

    if (primType == GsPrimType::Sprite) {
        if (m_spriteVertexCount == 0) {
            m_spritePos[0] = xyz.x();
            m_spritePos[1] = xyz.y();
            m_spriteVertexCount = 1;
        } else {
            m_spriteSize[0] = xyz.x() - m_spritePos[0];
            m_spriteSize[1] = xyz.y() - m_spritePos[1];
            m_spriteVertexCount = 0;

            if (m_currentPrim.iip()) {
                emitGouraudSprite();
            } else {
                emitSprite();
            }
        }
    } else if (primType == GsPrimType::Line || primType == GsPrimType::LineStrip) {
        if (m_spriteVertexCount == 0) {
            m_spritePos[0] = xyz.x();
            m_spritePos[1] = xyz.y();
            m_spriteVertexCount = 1;
        } else {
            m_spriteSize[0] = xyz.x();
            m_spriteSize[1] = xyz.y();
            m_spriteVertexCount = 0;
            emitLine();
        }
    } else if (primType == GsPrimType::Point) {
        m_spritePos[0] = xyz.x();
        m_spritePos[1] = xyz.y();
        emitPoint();
    } else if (primType == GsPrimType::Triangle ||
               primType == GsPrimType::TriStrip ||
               primType == GsPrimType::TriFan) {
        float px = xyz.x();
        float py = xyz.y();
        m_spriteVertices.push_back({px, py});

        if (m_hasUv) {
            float u = static_cast<float>(m_currentUv & 0xFFFF) / 16.0f;
            float v = static_cast<float>((m_currentUv >> 16) & 0xFFFF) / 16.0f;
            m_spriteUvs.push_back({u, v});
        }

        m_spriteColors.push_back(m_currentRgbaq);
        ++m_spriteVertexCount;

        bool emit = false;
        if (primType == GsPrimType::Triangle && m_spriteVertexCount == 3) {
            emit = true;
        } else if (primType == GsPrimType::TriStrip && m_spriteVertexCount >= 3) {
            emit = true;
        }

        if (emit) {
            RenderCmd cmd = makeRenderCmd(RenderCommand::DrawPrimitive);
            cmd.draw.primitive = GSPrimitive::Triangle;
            cmd.draw.list = RenderList::Opaque;
            cmd.draw.vertexOffset = 0;
            cmd.draw.vertexCount = static_cast<u32>(m_spriteVertices.size());
            cmd.draw.texture = currentTexture();

            if (!m_spriteColors.empty()) {
                cmd.draw.r = m_spriteColors.back().r();
                cmd.draw.g = m_spriteColors.back().g();
                cmd.draw.b = m_spriteColors.back().b();
                cmd.draw.a = m_spriteColors.back().a();
            }

            m_commands.push_back(cmd);

            if (primType == GsPrimType::TriStrip) {
                auto last2 = m_spriteVertices.size() >= 2 ? m_spriteVertices.size() - 2 : 0;
                std::vector<std::array<float, 2>> newVerts(
                    m_spriteVertices.begin() + static_cast<ptrdiff_t>(last2),
                    m_spriteVertices.end());
                m_spriteVertices = std::move(newVerts);

                auto last2uv = m_spriteUvs.size() >= 2 ? m_spriteUvs.size() - 2 : 0;
                std::vector<std::array<float, 2>> newUvs(
                    m_spriteUvs.begin() + static_cast<ptrdiff_t>(last2uv),
                    m_spriteUvs.end());
                m_spriteUvs = std::move(newUvs);

                auto last2c = m_spriteColors.size() >= 2 ? m_spriteColors.size() - 2 : 0;
                std::vector<GsRgbaq> newColors(
                    m_spriteColors.begin() + static_cast<ptrdiff_t>(last2c),
                    m_spriteColors.end());
                m_spriteColors = std::move(newColors);

                m_spriteVertexCount = static_cast<u32>(m_spriteVertices.size());
            } else {
                m_spriteVertices.clear();
                m_spriteUvs.clear();
                m_spriteColors.clear();
                m_spriteVertexCount = 0;
            }
        }
    }
}

void GifCommandBuffer::handleTex0(const u8* data) {
    u64 val = 0;
    std::memcpy(&val, data, 8);
    m_currentTex0.value = val;
}

void GifCommandBuffer::handleTex1(const u8* data) {
    u64 val = 0;
    std::memcpy(&val, data, 8);
    m_currentTex1.value = val;
}

void GifCommandBuffer::handleAlpha(const u8* data) {
    u32 val = 0;
    std::memcpy(&val, data, 4);
    m_currentAlpha.value = val;
}

void GifCommandBuffer::handleTest(const u8* data) {
    u64 val = 0;
    std::memcpy(&val, data, 8);
    m_currentTest.value = val;
}

void GifCommandBuffer::handleFrame(const u8* data) {
    u64 val = 0;
    std::memcpy(&val, data, 8);
    m_currentFrame.value = val;
}

void GifCommandBuffer::handleZbuf(const u8* data) {
    u64 val = 0;
    std::memcpy(&val, data, 8);
    m_currentZbuf.value = val;
}

void GifCommandBuffer::handleBitbltBuf(const u8* data) {
    u64 val = 0;
    std::memcpy(&val, data, 8);
    m_transferBitbltBuf.value = val;
}

void GifCommandBuffer::handleTrxPos(const u8* data) {
    u64 val = 0;
    std::memcpy(&val, data, 8);
    m_transferTrxPos.value = val;
}

void GifCommandBuffer::handleTrxReg(const u8* data) {
    u64 val = 0;
    std::memcpy(&val, data, 8);
    m_transferTrxReg.value = val;
}

void GifCommandBuffer::handleTrxDir(const u8* data) {
    u64 val = 0;
    std::memcpy(&val, data, 8);
    m_transferTrxDir.value = val;
}

void GifCommandBuffer::handleFog(const u8* data) {
    u32 val = 0;
    std::memcpy(&val, data, 4);
    m_currentFog.value = val;
}

void GifCommandBuffer::handleNop(const u8* /*data*/) {
}

void GifCommandBuffer::emitSprite() {
    RenderCmd cmd = makeRenderCmd(RenderCommand::DrawSprite);
    cmd.sprite.x = m_spritePos[0];
    cmd.sprite.y = m_spritePos[1];
    cmd.sprite.w = m_spriteSize[0];
    cmd.sprite.h = m_spriteSize[1];
    cmd.sprite.u0 = m_spriteUv0[0];
    cmd.sprite.v0 = m_spriteUv0[1];
    cmd.sprite.u1 = m_spriteUv1[0];
    cmd.sprite.v1 = m_spriteUv1[1];
    cmd.sprite.r = m_currentRgbaq.r();
    cmd.sprite.g = m_currentRgbaq.g();
    cmd.sprite.b = m_currentRgbaq.b();
    cmd.sprite.a = m_currentRgbaq.a();
    cmd.sprite.texture = currentTexture();
    m_commands.push_back(cmd);
}

void GifCommandBuffer::emitGouraudSprite() {
    RenderCmd cmd = makeRenderCmd(RenderCommand::DrawSpriteGouraud);
    cmd.spriteGouraud.x = m_spritePos[0];
    cmd.spriteGouraud.y = m_spritePos[1];
    cmd.spriteGouraud.w = m_spriteSize[0];
    cmd.spriteGouraud.h = m_spriteSize[1];
    cmd.spriteGouraud.u0 = m_spriteUv0[0];
    cmd.spriteGouraud.v0 = m_spriteUv0[1];
    cmd.spriteGouraud.u1 = m_spriteUv1[0];
    cmd.spriteGouraud.v1 = m_spriteUv1[1];
    cmd.spriteGouraud.texture = currentTexture();
    cmd.spriteGouraud.corners[0][0] = m_currentRgbaq.r();
    cmd.spriteGouraud.corners[0][1] = m_currentRgbaq.g();
    cmd.spriteGouraud.corners[0][2] = m_currentRgbaq.b();
    cmd.spriteGouraud.corners[0][3] = m_currentRgbaq.a();
    cmd.spriteGouraud.corners[1][0] = m_currentRgbaq.r();
    cmd.spriteGouraud.corners[1][1] = m_currentRgbaq.g();
    cmd.spriteGouraud.corners[1][2] = m_currentRgbaq.b();
    cmd.spriteGouraud.corners[1][3] = m_currentRgbaq.a();
    cmd.spriteGouraud.corners[2][0] = m_currentRgbaq.r();
    cmd.spriteGouraud.corners[2][1] = m_currentRgbaq.g();
    cmd.spriteGouraud.corners[2][2] = m_currentRgbaq.b();
    cmd.spriteGouraud.corners[2][3] = m_currentRgbaq.a();
    cmd.spriteGouraud.corners[3][0] = m_currentRgbaq.r();
    cmd.spriteGouraud.corners[3][1] = m_currentRgbaq.g();
    cmd.spriteGouraud.corners[3][2] = m_currentRgbaq.b();
    cmd.spriteGouraud.corners[3][3] = m_currentRgbaq.a();
    m_commands.push_back(cmd);
}

void GifCommandBuffer::emitLine() {
    RenderCmd cmd = makeRenderCmd(RenderCommand::DrawPrimitive);
    cmd.draw.primitive = GSPrimitive::Line;
    cmd.draw.list = RenderList::Opaque;
    cmd.draw.vertexOffset = 0;
    cmd.draw.vertexCount = 2;
    cmd.draw.texture = currentTexture();
    cmd.draw.r = m_currentRgbaq.r();
    cmd.draw.g = m_currentRgbaq.g();
    cmd.draw.b = m_currentRgbaq.b();
    cmd.draw.a = m_currentRgbaq.a();
    m_commands.push_back(cmd);
}

void GifCommandBuffer::emitPoint() {
    RenderCmd cmd = makeRenderCmd(RenderCommand::DrawPrimitive);
    cmd.draw.primitive = GSPrimitive::Point;
    cmd.draw.list = RenderList::Opaque;
    cmd.draw.vertexOffset = 0;
    cmd.draw.vertexCount = 1;
    cmd.draw.texture = currentTexture();
    cmd.draw.r = m_currentRgbaq.r();
    cmd.draw.g = m_currentRgbaq.g();
    cmd.draw.b = m_currentRgbaq.b();
    cmd.draw.a = m_currentRgbaq.a();
    m_commands.push_back(cmd);
}

void GifCommandBuffer::emitTile() {
    RenderCmd cmd = makeRenderCmd(RenderCommand::DrawSprite);
    cmd.sprite.x = m_spritePos[0];
    cmd.sprite.y = m_spritePos[1];
    cmd.sprite.w = m_spriteSize[0];
    cmd.sprite.h = m_spriteSize[1];
    cmd.sprite.u0 = 0.0f;
    cmd.sprite.v0 = 0.0f;
    cmd.sprite.u1 = 1.0f;
    cmd.sprite.v1 = 1.0f;
    cmd.sprite.r = m_currentRgbaq.r();
    cmd.sprite.g = m_currentRgbaq.g();
    cmd.sprite.b = m_currentRgbaq.b();
    cmd.sprite.a = m_currentRgbaq.a();
    cmd.sprite.texture = kNullTexture;
    m_commands.push_back(cmd);
}

RenderCmd GifCommandBuffer::makeRenderCmd(RenderCommand type) const {
    RenderCmd cmd{};
    cmd.type = type;
    return cmd;
}

u32 GifCommandBuffer::commandCount() const {
    return static_cast<u32>(m_commands.size());
}

const RenderCmd& GifCommandBuffer::command(u32 index) const {
    return m_commands[index];
}

std::vector<RenderCmd>& GifCommandBuffer::commands() {
    return m_commands;
}

const std::vector<RenderCmd>& GifCommandBuffer::commands() const {
    return m_commands;
}

void GifCommandBuffer::setScreenSize(u32 width, u32 height) {
    m_screenWidth = width;
    m_screenHeight = height;
}

GSBlendMode GifCommandBuffer::currentBlendMode() const {
    u32 aba = m_currentAlpha.aba();
    u32 abb = m_currentAlpha.abb();
    if (aba == 0 && abb == 1) {
        return GSBlendMode::Alpha;
    }
    if (aba == 1 && abb == 2) {
        return GSBlendMode::Additive;
    }
    return GSBlendMode::None;
}

TextureHandle GifCommandBuffer::currentTexture() const {
    if (m_currentTex0.tcc() == 0 || m_uploads.empty()) {
        return kNullTexture;
    }
    u32 tbp0 = m_currentTex0.tbp0();
    for (const UploadedTexture& tex : m_uploads) {
        if (tex.tbp0 == tbp0) {
            return tex.handle;
        }
    }
    return kNullTexture;
}

bool GifCommandBuffer::uploadedTexture(TextureHandle handle, u32& width, u32& height,
                                       TextureFormat& format, std::vector<u8>& rgba) const {
    for (const UploadedTexture& tex : m_uploads) {
        if (tex.handle == handle) {
            width = tex.width;
            height = tex.height;
            format = tex.format;
            rgba = tex.rgba;
            return true;
        }
    }
    return false;
}

std::size_t GifCommandBuffer::uploadedTextureCount() const {
    return m_uploads.size();
}

bool GifCommandBuffer::currentDepthWrite() const {
    return m_currentZbuf.zmsk() == 0;
}

GSDepthTest GifCommandBuffer::currentDepthTest() const {
    u32 ztst = m_currentTest.ztst();
    if (ztst == 0) return GSDepthTest::Never;
    if (ztst == 1) return GSDepthTest::Always;
    if (ztst == 2) return GSDepthTest::Greater;
    return GSDepthTest::Less;
}

} // namespace ico::engine
