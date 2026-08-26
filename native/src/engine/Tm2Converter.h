#pragma once

#include "engine/Tm2Format.h"
#include "engine/Tm2Texture.h"
#include "ps2/Ps2Types.h"

#include <vector>

namespace ico::engine {

class Tm2Converter {
public:
    static bool convertImage(const Tm2Image& image, Tm2Texture& out);

    static void convertPSMCT32(const u8* src, u32 width, u32 height, std::vector<u8>& out);
    static void convertPSMCT24(const u8* src, u32 width, u32 height, std::vector<u8>& out);
    static void convertPSMCT16(const u8* src, u32 width, u32 height, std::vector<u8>& out);
    static void convertPSMT8(const u8* src, u32 width, u32 height,
                             const Tm2Clut& clut, std::vector<u8>& out);
    static void convertPSMT4(const u8* src, u32 width, u32 height,
                             const Tm2Clut& clut, std::vector<u8>& out);

    static void convertClut32(const u8* src, u32 numEntries, std::vector<u8>& out);
    static void convertClut16(const u8* src, u32 numEntries, std::vector<u8>& out);

    static void deswizzlePSMCT32(const u8* src, u8* dst, u32 width, u32 height);
    static void deswizzlePSMCT16(const u8* src, u8* dst, u32 width, u32 height);
    static void deswizzlePSMT8(const u8* src, u8* dst, u32 width, u32 height);
    static void deswizzlePSMT4(const u8* src, u8* dst, u32 width, u32 height);

    static void swizzlePSMCT32(const u8* src, u8* dst, u32 width, u32 height);
    static void swizzlePSMCT16(const u8* src, u8* dst, u32 width, u32 height);
    static void swizzlePSMT8(const u8* src, u8* dst, u32 width, u32 height);
    static void swizzlePSMT4(const u8* src, u8* dst, u32 width, u32 height);

private:
    static u32 padded(u32 value, u32 alignment);
};

} // namespace ico::engine
