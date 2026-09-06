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

// TIM2 v4 picture block header (48 bytes, OpenKh layout).
// The picture block begins at file offset 16 for v4 files.
struct __attribute__((packed)) Tm2PictureBlockV4 {
    u32 blockTotal;    // +0x00  total bytes of the picture block
    u32 clutSize;      // +0x04  CLUT byte size (0 = none)
    u32 imgSize;       // +0x08  pixel data byte size
    u16 hdrSize;       // +0x0C  0x30 standard, 0x50 when a mipmap block is present
    u16 clutColors;    // +0x0E  CLUT entry count
    u8 mipmapCount;    // +0x10
    u8 clutType;       // +0x11  CLUT color type byte (0x02 = RGBA8888, 0x01 = RGBA5551)
    u8 imgType;        // +0x12  image color type byte
    u8 reservedA;      // +0x13
    u16 imageWidth;    // +0x14
    u16 imageHeight;   // +0x16
    u64 gsTex0;        // +0x18  TEX0 (PSM bits 20-25, TW/TH bits 26-30)
    u64 gsTex1;        // +0x20
    u32 gsTexclut;     // +0x28
    u32 reservedB;     // +0x2C

    u32 width() const { return static_cast<u32>(imageWidth); }
    u32 height() const { return static_cast<u32>(imageHeight); }
};

static_assert(sizeof(Tm2PictureBlockV4) == 48, "Tm2PictureBlockV4 must be 48 bytes");

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
    // v4 only: total byte size of the owning picture block (for multi-image walks).
    u32 blockTotal = 0;
};

struct Tm2File {
    Tm2FileHeader header;
    std::vector<Tm2Image> images;
};

class Tm2Parser {
public:
    static bool parse(const u8* data, u32 size, Tm2File& out);

    // Absolute file offset of the first picture block (16 for v4, 20 for v2/v3).
    static u32 firstPictureOffset(const Tm2FileHeader& header);

    static u32 imageDataOffset(const Tm2File& file, u32 index);
    static u32 clutEntryCount(const Tm2ImageHeader& header);
};

} // namespace ico::engine
