#include "engine/Tm2Converter.h"

#include <algorithm>
#include <cassert>
#include <cstring>

namespace ico::engine {

u32 Tm2Converter::padded(u32 value, u32 alignment) {
    return (value + alignment - 1) & ~(alignment - 1);
}

void Tm2Converter::convertPSMCT32(const u8* src, u32 width, u32 height, std::vector<u8>& out) {
    const u32 numPixels = width * height;
    out.resize(numPixels * 4);
    std::memcpy(out.data(), src, numPixels * 4);
}

void Tm2Converter::convertPSMCT24(const u8* src, u32 width, u32 height, std::vector<u8>& out) {
    const u32 numPixels = width * height;
    out.resize(numPixels * 4);
    for (u32 i = 0; i < numPixels; ++i) {
        out[i * 4 + 0] = src[i * 3 + 0];
        out[i * 4 + 1] = src[i * 3 + 1];
        out[i * 4 + 2] = src[i * 3 + 2];
        out[i * 4 + 3] = 255;
    }
}

void Tm2Converter::convertPSMCT16(const u8* src, u32 width, u32 height, std::vector<u8>& out) {
    const u32 numPixels = width * height;
    out.resize(numPixels * 4);
    for (u32 i = 0; i < numPixels; ++i) {
        const u16 pixel = static_cast<u16>(src[i * 2 + 0] | (src[i * 2 + 1] << 8));
        const u8 r = static_cast<u8>((pixel >>  0) & 0x1F);
        const u8 g = static_cast<u8>((pixel >>  5) & 0x1F);
        const u8 b = static_cast<u8>((pixel >> 10) & 0x1F);
        const u8 a = static_cast<u8>((pixel >> 15) & 0x01);
        out[i * 4 + 0] = static_cast<u8>((r << 3) | (r >> 2));
        out[i * 4 + 1] = static_cast<u8>((g << 3) | (g >> 2));
        out[i * 4 + 2] = static_cast<u8>((b << 3) | (b >> 2));
        out[i * 4 + 3] = (a != 0) ? 255 : 0;
    }
}

void Tm2Converter::convertPSMT8(const u8* src, u32 width, u32 height,
                                 const Tm2Clut& clut, std::vector<u8>& out) {
    const u32 numPixels = width * height;
    out.resize(numPixels * 4);

    if (!clut.data || clut.numEntries == 0) {
        std::memset(out.data(), 0, out.size());
        return;
    }

    std::vector<u8> rgbaClut;
    if (clut.format == Tm2ClutFormat::RGBA32) {
        convertClut32(clut.data, clut.numEntries, rgbaClut);
    } else {
        convertClut16(clut.data, clut.numEntries, rgbaClut);
    }

    for (u32 i = 0; i < numPixels; ++i) {
        const u8 index = src[i];
        if (index < (rgbaClut.size() / 4)) {
            std::memcpy(&out[i * 4], &rgbaClut[index * 4], 4);
        } else {
            out[i * 4 + 0] = 0;
            out[i * 4 + 1] = 0;
            out[i * 4 + 2] = 0;
            out[i * 4 + 3] = 0;
        }
    }
}

void Tm2Converter::convertPSMT4(const u8* src, u32 width, u32 height,
                                 const Tm2Clut& clut, std::vector<u8>& out) {
    const u32 numPixels = width * height;
    out.resize(numPixels * 4);

    if (!clut.data || clut.numEntries == 0) {
        std::memset(out.data(), 0, out.size());
        return;
    }

    std::vector<u8> rgbaClut;
    if (clut.format == Tm2ClutFormat::RGBA32) {
        convertClut32(clut.data, clut.numEntries, rgbaClut);
    } else {
        convertClut16(clut.data, clut.numEntries, rgbaClut);
    }

    for (u32 i = 0; i < numPixels; ++i) {
        const u32 byteIdx = i / 2;
        const u8 nibble = (i & 1) ? (src[byteIdx] & 0x0F) : (src[byteIdx] >> 4);
        if (nibble < (rgbaClut.size() / 4)) {
            std::memcpy(&out[i * 4], &rgbaClut[nibble * 4], 4);
        } else {
            out[i * 4 + 0] = 0;
            out[i * 4 + 1] = 0;
            out[i * 4 + 2] = 0;
            out[i * 4 + 3] = 0;
        }
    }
}

void Tm2Converter::convertClut32(const u8* src, u32 numEntries, std::vector<u8>& out) {
    out.resize(numEntries * 4);
    for (u32 i = 0; i < numEntries; ++i) {
        const u32 pixel = static_cast<u32>(
            src[i * 4 + 0] | (src[i * 4 + 1] << 8)
            | (src[i * 4 + 2] << 16) | (src[i * 4 + 3] << 24));
        out[i * 4 + 0] = static_cast<u8>((pixel >>  0) & 0xFF);
        out[i * 4 + 1] = static_cast<u8>((pixel >>  8) & 0xFF);
        out[i * 4 + 2] = static_cast<u8>((pixel >> 16) & 0xFF);
        out[i * 4 + 3] = static_cast<u8>((pixel >> 24) & 0xFF);
    }
}

void Tm2Converter::convertClut16(const u8* src, u32 numEntries, std::vector<u8>& out) {
    out.resize(numEntries * 4);
    for (u32 i = 0; i < numEntries; ++i) {
        const u16 pixel = static_cast<u16>(src[i * 2 + 0] | (src[i * 2 + 1] << 8));
        const u8 r = static_cast<u8>((pixel >>  0) & 0x1F);
        const u8 g = static_cast<u8>((pixel >>  5) & 0x1F);
        const u8 b = static_cast<u8>((pixel >> 10) & 0x1F);
        const u8 a = static_cast<u8>((pixel >> 15) & 0x01);
        out[i * 4 + 0] = static_cast<u8>((r << 3) | (r >> 2));
        out[i * 4 + 1] = static_cast<u8>((g << 3) | (g >> 2));
        out[i * 4 + 2] = static_cast<u8>((b << 3) | (b >> 2));
        out[i * 4 + 3] = (a != 0) ? 255 : 0;
    }
}

void Tm2Converter::deswizzlePSMCT32(const u8* src, u8* dst, u32 width, u32 height) {
    const u32 padW = padded(width, 256);
    const u32 padH = padded(height, 256);
    for (u32 dy = 0; dy < padH; ++dy) {
        for (u32 dx = 0; dx < padW; ++dx) {
            const u32 pageY = dy / 128;
            const u32 pageX = dx / 128;
            const u32 inPageY = dy % 128;
            const u32 inPageX = dx % 128;
            const u32 blockY = inPageY / 32;
            const u32 blockX = inPageX / 32;
            const u32 row = (inPageY % 32) / 8;
            const u32 srcOff = (pageY * padW * 128 + pageX * 128) * 4
                + (blockY * 4 + blockX) * 128
                + (row * 4 + (inPageX % 32) / 8) * 4;
            if (dx < width && dy < height) {
                const u32 dstIdx = (dy * width + dx) * 4;
                std::memcpy(dst + dstIdx, src + srcOff, 4);
            }
        }
    }
}

void Tm2Converter::deswizzlePSMCT16(const u8* src, u8* dst, u32 width, u32 height) {
    const u32 padW = padded(width, 256);
    const u32 padH = padded(height, 256);
    for (u32 dy = 0; dy < padH; ++dy) {
        for (u32 dx = 0; dx < padW; ++dx) {
            const u32 pageY = dy / 128;
            const u32 pageX = dx / 128;
            const u32 inPageY = dy % 128;
            const u32 inPageX = dx % 128;
            const u32 blockY = inPageY / 64;
            const u32 blockX = inPageX / 64;
            const u32 row = (inPageY % 64) / 16;
            const u32 srcOff = (pageY * padW * 128 + pageX * 128) * 2
                + (blockY * 4 + blockX) * 128
                + (row * 8 + (inPageX % 64) / 8) * 2;
            if (dx < width && dy < height) {
                const u32 dstIdx = (dy * width + dx) * 2;
                dst[dstIdx + 0] = src[srcOff + 0];
                dst[dstIdx + 1] = src[srcOff + 1];
            }
        }
    }
}

void Tm2Converter::deswizzlePSMT8(const u8* src, u8* dst, u32 width, u32 height) {
    const u32 padW = padded(width, 128);
    const u32 padH = padded(height, 128);
    for (u32 dy = 0; dy < padH; ++dy) {
        for (u32 dx = 0; dx < padW; ++dx) {
            const u32 pageY = dy / 64;
            const u32 pageX = dx / 128;
            const u32 inPageY = dy % 64;
            const u32 inPageX = dx % 128;
            const u32 blockY = inPageY / 32;
            const u32 blockX = inPageX / 64;
            const u32 row = (inPageY % 32) / 8;
            const u32 srcOff = (pageY * padW * 64 + pageX * 128) * 1
                + (blockY * 2 + blockX) * 128
                + (row * 8 + (inPageX % 64) / 8) * 1;
            if (dx < width && dy < height) {
                dst[dy * width + dx] = src[srcOff];
            }
        }
    }
}

void Tm2Converter::deswizzlePSMT4(const u8* src, u8* dst, u32 width, u32 height) {
    const u32 padW = padded(width, 128);
    const u32 padH = padded(height, 128);
    for (u32 dy = 0; dy < padH; ++dy) {
        for (u32 dx = 0; dx < padW; ++dx) {
            const u32 pageY = dy / 64;
            const u32 pageX = dx / 128;
            const u32 inPageY = dy % 64;
            const u32 inPageX = dx % 128;
            const u32 blockY = inPageY / 32;
            const u32 blockX = inPageX / 32;
            const u32 row = (inPageY % 32) / 8;
            const u32 srcByte = (pageY * padW * 64 + pageX * 128)
                + (blockY * 4 + blockX) * 128
                + (row * 4 + (inPageX % 32) / 8);
            if (dx < width && dy < height) {
                const u32 dstIdx = dy * width + dx;
                if (dstIdx & 1) {
                    dst[dstIdx / 2] = (dst[dstIdx / 2] & 0x0F)
                        | ((src[srcByte] & 0x0F) << 4);
                } else {
                    dst[dstIdx / 2] = (dst[dstIdx / 2] & 0xF0)
                        | (src[srcByte] & 0x0F);
                }
            }
        }
    }
}

void Tm2Converter::swizzlePSMCT32(const u8* src, u8* dst, u32 width, u32 height) {
    const u32 padW = padded(width, 256);
    const u32 padH = padded(height, 256);
    std::memset(dst, 0, padW * padH * 4);
    for (u32 dy = 0; dy < padH; ++dy) {
        for (u32 dx = 0; dx < padW; ++dx) {
            const u32 pageY = dy / 128;
            const u32 pageX = dx / 128;
            const u32 inPageY = dy % 128;
            const u32 inPageX = dx % 128;
            const u32 blockY = inPageY / 32;
            const u32 blockX = inPageX / 32;
            const u32 row = (inPageY % 32) / 8;
            const u32 dstOff = (pageY * padW * 128 + pageX * 128) * 4
                + (blockY * 4 + blockX) * 128
                + (row * 4 + (inPageX % 32) / 8) * 4;
            if (dx < width && dy < height) {
                std::memcpy(dst + dstOff, src + (dy * width + dx) * 4, 4);
            }
        }
    }
}

void Tm2Converter::swizzlePSMCT16(const u8* src, u8* dst, u32 width, u32 height) {
    const u32 padW = padded(width, 256);
    const u32 padH = padded(height, 256);
    std::memset(dst, 0, padW * padH * 2);
    for (u32 dy = 0; dy < padH; ++dy) {
        for (u32 dx = 0; dx < padW; ++dx) {
            const u32 pageY = dy / 128;
            const u32 pageX = dx / 128;
            const u32 inPageY = dy % 128;
            const u32 inPageX = dx % 128;
            const u32 blockY = inPageY / 64;
            const u32 blockX = inPageX / 64;
            const u32 row = (inPageY % 64) / 16;
            const u32 dstOff = (pageY * padW * 128 + pageX * 128) * 2
                + (blockY * 4 + blockX) * 128
                + (row * 8 + (inPageX % 64) / 8) * 2;
            if (dx < width && dy < height) {
                const u32 srcIdx = (dy * width + dx) * 2;
                dst[dstOff + 0] = src[srcIdx + 0];
                dst[dstOff + 1] = src[srcIdx + 1];
            }
        }
    }
}

void Tm2Converter::swizzlePSMT8(const u8* src, u8* dst, u32 width, u32 height) {
    const u32 padW = padded(width, 128);
    const u32 padH = padded(height, 128);
    std::memset(dst, 0, padW * padH);
    for (u32 dy = 0; dy < padH; ++dy) {
        for (u32 dx = 0; dx < padW; ++dx) {
            const u32 pageY = dy / 64;
            const u32 pageX = dx / 128;
            const u32 inPageY = dy % 64;
            const u32 inPageX = dx % 128;
            const u32 blockY = inPageY / 32;
            const u32 blockX = inPageX / 64;
            const u32 row = (inPageY % 32) / 8;
            const u32 dstOff = (pageY * padW * 64 + pageX * 128)
                + (blockY * 2 + blockX) * 128
                + (row * 8 + (inPageX % 64) / 8);
            if (dx < width && dy < height) {
                dst[dstOff] = src[dy * width + dx];
            }
        }
    }
}

void Tm2Converter::swizzlePSMT4(const u8* src, u8* dst, u32 width, u32 height) {
    const u32 padW = padded(width, 128);
    const u32 padH = padded(height, 128);
    std::memset(dst, 0, (padW * padH + 1) / 2);
    for (u32 dy = 0; dy < padH; ++dy) {
        for (u32 dx = 0; dx < padW; ++dx) {
            const u32 pageY = dy / 64;
            const u32 pageX = dx / 128;
            const u32 inPageY = dy % 64;
            const u32 inPageX = dx % 128;
            const u32 blockY = inPageY / 32;
            const u32 blockX = inPageX / 32;
            const u32 row = (inPageY % 32) / 8;
            const u32 dstByte = (pageY * padW * 64 + pageX * 128)
                + (blockY * 4 + blockX) * 128
                + (row * 4 + (inPageX % 32) / 8);
            if (dx < width && dy < height) {
                const u32 srcIdx = dy * width + dx;
                const u8 nibble = (srcIdx & 1)
                    ? (src[srcIdx / 2] >> 4)
                    : (src[srcIdx / 2] & 0x0F);
                if (srcIdx & 1) {
                    dst[dstByte] = (dst[dstByte] & 0x0F) | (nibble << 4);
                } else {
                    dst[dstByte] = (dst[dstByte] & 0xF0) | nibble;
                }
            }
        }
    }
}

bool Tm2Converter::convertImage(const Tm2Image& image, Tm2Texture& out) {
    const u32 w = image.header.width();
    const u32 h = image.header.height();
    if (w == 0 || h == 0 || !image.pixelData) {
        return false;
    }

    out.width = w;
    out.height = h;
    out.format = static_cast<TextureFormat>(image.header.texMode);

    const Tm2TexMode mode = image.header.textureMode();
    switch (mode) {
    case Tm2TexMode::PSMCT32:
        convertPSMCT32(image.pixelData, w, h, out.rgbaData);
        out.hasAlpha = true;
        break;
    case Tm2TexMode::PSMCT24:
        convertPSMCT24(image.pixelData, w, h, out.rgbaData);
        out.hasAlpha = false;
        break;
    case Tm2TexMode::PSMCT16:
        convertPSMCT16(image.pixelData, w, h, out.rgbaData);
        out.hasAlpha = true;
        break;
    case Tm2TexMode::PSMT8:
        convertPSMT8(image.pixelData, w, h, image.clut, out.rgbaData);
        out.hasAlpha = true;
        break;
    case Tm2TexMode::PSMT4:
        convertPSMT4(image.pixelData, w, h, image.clut, out.rgbaData);
        out.hasAlpha = true;
        break;
    default:
        return false;
    }

    return !out.rgbaData.empty();
}

} // namespace ico::engine
