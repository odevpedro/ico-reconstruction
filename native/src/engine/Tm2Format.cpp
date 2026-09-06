#include "engine/Tm2Format.h"

#include <cstring>

namespace ico::engine {

bool Tm2FileHeader::valid() const {
    return magic[0] == 'T' && magic[1] == 'I' && magic[2] == 'M' && magic[3] == '2'
        && (version == 2 || version == 3 || version == 4)
        && numImages > 0;
}

u32 Tm2FileHeader::imageCount() const {
    return static_cast<u32>(numImages);
}

Tm2TexMode Tm2ImageHeader::textureMode() const {
    return static_cast<Tm2TexMode>(texMode);
}

Tm2ClutFormat Tm2ImageHeader::clutFmt() const {
    return static_cast<Tm2ClutFormat>(clutFormat);
}

u32 Tm2ImageHeader::width() const {
    return static_cast<u32>(imageWidth);
}

u32 Tm2ImageHeader::height() const {
    return static_cast<u32>(imageHeight);
}

u32 Tm2ImageHeader::clutByteSize() const {
    return static_cast<u32>(clutSize);
}

bool Tm2ImageHeader::hasClut() const {
    return clutSize > 0 && (texMode == static_cast<u8>(Tm2TexMode::PSMT8)
                         || texMode == static_cast<u8>(Tm2TexMode::PSMT4));
}

const u8* Tm2ImageHeader::clutData(const u8* imageBase) const {
    return imageBase + sizeof(Tm2ImageHeader);
}

const u8* Tm2ImageHeader::pixelData(const u8* imageBase) const {
    return imageBase + sizeof(Tm2ImageHeader) + clutByteSize();
}

u32 Tm2Parser::firstPictureOffset(const Tm2FileHeader& header) {
    return (header.version == 4) ? 16u : sizeof(Tm2FileHeader);
}

namespace {

// TIM2 v4 keeps nothing in the legacy texMode byte; the real pixel mode is the
// GS PSM field (bits 20..25) of TEX0. Values match the GS PSM constants that
// Tm2TexMode was modeled after (0x00 CT32, 0x02 CT16, 0x13 PSMT8, 0x14 PSMT4).
Tm2TexMode v4TexMode(u64 gsTex0) {
    const u32 psm = static_cast<u32>((gsTex0 >> 20) & 0x3F);
    switch (psm) {
    case 0x00: return Tm2TexMode::PSMCT32;
    case 0x01: return Tm2TexMode::PSMCT24;
    case 0x02: return Tm2TexMode::PSMCT16;
    case 0x13: return Tm2TexMode::PSMT8;
    case 0x14: return Tm2TexMode::PSMT4;
    default:   return Tm2TexMode::PSMT4;
    }
}

// v4 CLUT format from byte layout: clutSize / clutColors decides RGBA32 (4 B/entry)
// vs RGBA16 (2 B/entry). Falls back to the clutType color-type byte when colors unknown.
Tm2ClutFormat v4ClutFormat(const Tm2PictureBlockV4& block) {
    if (block.clutColors > 0 && block.clutSize > 0) {
        const u32 bytesPerEntry = block.clutSize / block.clutColors;
        if (bytesPerEntry >= 4) {
            return Tm2ClutFormat::RGBA32;
        }
        if (bytesPerEntry == 2) {
            return Tm2ClutFormat::RGBA16;
        }
    }
    return (block.clutType == 0x02) ? Tm2ClutFormat::RGBA32 : Tm2ClutFormat::RGBA16;
}

} // namespace

static bool parseV23(const u8* data, u32 size, Tm2File& out) {
    const u32 numImages = out.header.imageCount();
    out.images.resize(numImages);

    u32 offset = sizeof(Tm2FileHeader);
    for (u32 i = 0; i < numImages; ++i) {
        if (offset + sizeof(Tm2ImageHeader) > size) {
            return false;
        }

        Tm2Image& image = out.images[i];
        std::memcpy(&image.header, data + offset, sizeof(Tm2ImageHeader));

        const u32 w = image.header.width();
        const u32 h = image.header.height();
        if (w == 0 || h == 0 || w > 4096 || h > 4096) {
            return false;
        }

        const u8* imageBase = data + offset;

        if (image.header.hasClut()) {
            image.clut.data = image.header.clutData(imageBase);
            image.clut.format = image.header.clutFmt();

            const u32 clutBytes = image.header.clutByteSize();
            const u32 entrySize = (image.clut.format == Tm2ClutFormat::RGBA32) ? 4 : 2;
            image.clut.numEntries = (entrySize > 0) ? (clutBytes / entrySize) : 0;
        }

        image.pixelData = image.header.pixelData(imageBase);
        image.pixelDataSize = image.header.imageSize;

        const u32 pixelEnd = static_cast<u32>(image.pixelData - data) + image.pixelDataSize;
        if (pixelEnd > size) {
            return false;
        }

        offset = pixelEnd;
    }

    return true;
}

static bool parseV4(const u8* data, u32 size, Tm2File& out);

bool Tm2Parser::parse(const u8* data, u32 size, Tm2File& out) {
    if (!data || size < sizeof(Tm2FileHeader)) {
        return false;
    }

    std::memcpy(&out.header, data, sizeof(Tm2FileHeader));
    if (!out.header.valid()) {
        return false;
    }

    if (out.header.version == 4) {
        return parseV4(data, size, out);
    }
    // v2/v3 preferred; some ICO files carry a v3 version byte but a v4 body
    // (validated: wall_cl.tm2, byte-exact v4 layout at offset 16). Fall back.
    if (parseV23(data, size, out)) {
        return true;
    }
    return parseV4(data, size, out);
}

// v4 picture block layout: [blockTotal|clutSize|imgSize|hdrSize|clutColors|...|width|height|tex0...],
// then pixel data at picture + hdrSize, then CLUT right after pixel data.
// hdrSize already accounts for the 0x30 header plus any mipmap block, and
// blockTotal == hdrSize + imgSize + clutSize (validated on real ICO v4 files).
static bool parseV4(const u8* data, u32 size, Tm2File& out) {
    const u32 numImages = out.header.imageCount();
    out.images.resize(numImages);

    u32 offset = 16;
    u32 decoded = 0;
    for (u32 i = 0; i < numImages; ++i) {
        // A picture block may be absent at the tail: accept as many complete
        // blocks as the file actually holds (ct_michi.tm2 declares 2 images
        // but ships a single block).
        if (offset + sizeof(Tm2PictureBlockV4) > size) {
            break;
        }

        Tm2PictureBlockV4 block;
        std::memcpy(&block, data + offset, sizeof(Tm2PictureBlockV4));

        const u32 w = block.width();
        const u32 h = block.height();
        if (w == 0 || h == 0 || w > 4096 || h > 4096) {
            return false;
        }

        Tm2Image& image = out.images[i];
        image.blockTotal = block.blockTotal;

        // Populate the legacy v2/v3-style accessor header so the rest of the
        // pipeline (width/height/texMode/clutFmt/hasClut) works unchanged.
        image.header.imageWidth = static_cast<u16>(w);
        image.header.imageHeight = static_cast<u16>(h);
        image.header.imageSize = block.imgSize;
        image.header.clutSize = (block.clutSize > 0) ? static_cast<u16>(block.clutSize) : 0;
        image.header.texMode = static_cast<u8>(v4TexMode(block.gsTex0));
        image.header.clutFormat = static_cast<u8>(v4ClutFormat(block));

        const u8* pixelStart = data + offset + block.hdrSize;
        if (pixelStart + block.imgSize > data + size) {
            return false;
        }
        image.pixelData = pixelStart;
        image.pixelDataSize = block.imgSize;

        // CLUT lives after the pixel data in v4 (layout order is header, [mipmap], pixels, CLUT).
        if (block.clutSize > 0
            && (image.header.texMode == static_cast<u8>(Tm2TexMode::PSMT8)
                || image.header.texMode == static_cast<u8>(Tm2TexMode::PSMT4))) {
            image.clut.data = pixelStart + block.imgSize;
            image.clut.format = image.header.clutFmt();
            image.clut.numEntries = (block.clutColors > 0)
                ? block.clutColors
                : Tm2Parser::clutEntryCount(image.header);
            if (image.clut.data + block.clutSize > data + size) {
                return false;
            }
        }

        if (block.blockTotal == 0) {
            offset = static_cast<u32>((pixelStart - data)) + block.imgSize + block.clutSize;
        } else {
            offset += block.blockTotal;
        }
        ++decoded;
    }

    if (decoded > 0 && decoded < numImages) {
        out.images.resize(decoded);
    }
    return decoded > 0;
}

u32 Tm2Parser::imageDataOffset(const Tm2File& file, u32 index) {
    u32 offset = firstPictureOffset(file.header);
    for (u32 i = 0; i < index && i < file.header.imageCount(); ++i) {
        const Tm2Image& img = file.images[i];
        if (file.header.version == 4) {
            offset += (img.blockTotal > 0) ? img.blockTotal
                                          : img.header.clutByteSize() + img.header.imageSize;
        } else {
            offset += sizeof(Tm2ImageHeader) + img.header.clutByteSize() + img.header.imageSize;
        }
    }
    return offset;
}

u32 Tm2Parser::clutEntryCount(const Tm2ImageHeader& header) {
    const Tm2ClutFormat fmt = header.clutFmt();
    const u32 entrySize = (fmt == Tm2ClutFormat::RGBA32) ? 4 : 2;
    return (entrySize > 0) ? (header.clutByteSize() / entrySize) : 0;
}

} // namespace ico::engine
