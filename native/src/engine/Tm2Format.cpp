#include "engine/Tm2Format.h"

#include <cstring>

namespace ico::engine {

bool Tm2FileHeader::valid() const {
    return magic[0] == 'T' && magic[1] == 'I' && magic[2] == 'M' && magic[3] == '2'
        && (version == 2 || version == 3)
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

bool Tm2Parser::parse(const u8* data, u32 size, Tm2File& out) {
    if (!data || size < sizeof(Tm2FileHeader)) {
        return false;
    }

    std::memcpy(&out.header, data, sizeof(Tm2FileHeader));
    if (!out.header.valid()) {
        return false;
    }

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

u32 Tm2Parser::imageDataOffset(const Tm2File& file, u32 index) {
    u32 offset = sizeof(Tm2FileHeader);
    for (u32 i = 0; i < index && i < file.header.imageCount(); ++i) {
        const Tm2Image& img = file.images[i];
        offset += sizeof(Tm2ImageHeader) + img.header.clutByteSize() + img.header.imageSize;
    }
    return offset;
}

u32 Tm2Parser::clutEntryCount(const Tm2ImageHeader& header) {
    const Tm2ClutFormat fmt = header.clutFmt();
    const u32 entrySize = (fmt == Tm2ClutFormat::RGBA32) ? 4 : 2;
    return (entrySize > 0) ? (header.clutByteSize() / entrySize) : 0;
}

} // namespace ico::engine
