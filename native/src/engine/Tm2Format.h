#pragma once

#include "ps2/Ps2Types.h"

#include <cstdint>
#include <vector>

namespace ico::engine {

enum class Tm2TexMode : u8 {
    PSMCT32 = 0,
    PSMCT24 = 1,
    PSMCT16 = 2,
    PSMT8 = 19,
    PSMT4 = 20,
};

enum class Tm2ClutFormat : u8 {
    RGBA32 = 0,
    RGBA16 = 1,
    RGBA24 = 2,
};

struct __attribute__((packed)) Tm2FileHeader {
    u8 magic[4];
    u8 version;
    u8 format;
    u16 numImages;
    u32 totalSize;
    u8 reserved[8];

    bool valid() const;
    u32 imageCount() const;
};

static_assert(sizeof(Tm2FileHeader) == 20, "Tm2FileHeader must be 20 bytes");

struct __attribute__((packed)) Tm2ImageHeader {
    u32 imageSize;
    u16 imageWidth;
    u16 imageHeight;
    u8 texMode;
    u8 mipLevels;
    u8 clutFormat;
    u8 clutCount;
    u16 clutSize;
    u16 imageOffset;
    u32 dataOffset;

    Tm2TexMode textureMode() const;
    Tm2ClutFormat clutFmt() const;
    u32 width() const;
    u32 height() const;
    u32 clutByteSize() const;
    bool hasClut() const;

    const u8* clutData(const u8* imageBase) const;
    const u8* pixelData(const u8* imageBase) const;
};

static_assert(sizeof(Tm2ImageHeader) == 20, "Tm2ImageHeader must be 20 bytes");

struct Tm2Clut {
    const u8* data = nullptr;
    u32 numEntries = 0;
    Tm2ClutFormat format = Tm2ClutFormat::RGBA32;
};

struct Tm2Image {
    Tm2ImageHeader header;
    Tm2Clut clut;
    const u8* pixelData = nullptr;
    u32 pixelDataSize = 0;
};

struct Tm2File {
    Tm2FileHeader header;
    std::vector<Tm2Image> images;
};

class Tm2Parser {
public:
    static bool parse(const u8* data, u32 size, Tm2File& out);
    static u32 imageDataOffset(const Tm2File& file, u32 index);
    static u32 clutEntryCount(const Tm2ImageHeader& header);
};

} // namespace ico::engine
