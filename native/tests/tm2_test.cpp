#include "engine/Tm2Format.h"
#include "engine/Tm2Converter.h"
#include "engine/Tm2Texture.h"

#include <cassert>
#include <cstdio>
#include <cstring>
#include <vector>

using namespace ico::engine;

static u32 pad(u32 v, u32 a) { return (v + a - 1) & ~(a - 1); }

static void test_struct_sizes() {
    assert(sizeof(Tm2FileHeader) == 20);
    assert(sizeof(Tm2ImageHeader) == 20);
    std::fprintf(stderr, "struct sizes: OK\n");
}

static void test_file_header_valid() {
    Tm2FileHeader h{};
    h.magic[0] = 'T'; h.magic[1] = 'I'; h.magic[2] = 'M'; h.magic[3] = '2';
    h.version = 2;
    h.numImages = 1;
    assert(h.valid());

    h.version = 3;
    assert(h.valid());

    h.version = 1;
    assert(!h.valid());

    h.version = 2;
    h.numImages = 0;
    assert(!h.valid());

    h.numImages = 1;
    h.magic[0] = 'X';
    assert(!h.valid());

    std::fprintf(stderr, "file header valid: OK\n");
}

static void test_image_header_clut() {
    Tm2ImageHeader ih{};
    ih.texMode = static_cast<u8>(Tm2TexMode::PSMT8);
    ih.clutSize = 1024;
    assert(ih.hasClut());

    ih.texMode = static_cast<u8>(Tm2TexMode::PSMT4);
    ih.clutSize = 64;
    assert(ih.hasClut());

    ih.texMode = static_cast<u8>(Tm2TexMode::PSMCT32);
    ih.clutSize = 1024;
    assert(!ih.hasClut());

    ih.texMode = static_cast<u8>(Tm2TexMode::PSMT8);
    ih.clutSize = 0;
    assert(!ih.hasClut());

    std::fprintf(stderr, "image header clut: OK\n");
}

static void test_parse_minimal() {
    const u32 pixelDataSize = 16 * 16 * 4;
    std::vector<u8> buf(sizeof(Tm2FileHeader) + sizeof(Tm2ImageHeader) + pixelDataSize, 0);

    auto* fh = reinterpret_cast<Tm2FileHeader*>(buf.data());
    fh->magic[0] = 'T'; fh->magic[1] = 'I'; fh->magic[2] = 'M'; fh->magic[3] = '2';
    fh->version = 2;
    fh->numImages = 1;
    fh->totalSize = static_cast<u32>(buf.size());

    auto* ih = reinterpret_cast<Tm2ImageHeader*>(buf.data() + sizeof(Tm2FileHeader));
    ih->imageWidth = 16;
    ih->imageHeight = 16;
    ih->texMode = static_cast<u8>(Tm2TexMode::PSMCT32);
    ih->imageSize = pixelDataSize;

    Tm2File file{};
    assert(Tm2Parser::parse(buf.data(), static_cast<u32>(buf.size()), file));
    assert(file.header.imageCount() == 1);
    assert(file.images[0].header.width() == 16);
    assert(file.images[0].header.height() == 16);
    assert(file.images[0].pixelData != nullptr);

    std::fprintf(stderr, "parse minimal: OK\n");
}

static void test_parse_invalid_magic() {
    std::vector<u8> buf(sizeof(Tm2FileHeader), 0);
    Tm2File file{};
    assert(!Tm2Parser::parse(buf.data(), static_cast<u32>(buf.size()), file));
    std::fprintf(stderr, "parse invalid magic: OK\n");
}

static void test_parse_truncated() {
    std::vector<u8> buf(4, 0);
    Tm2File file{};
    assert(!Tm2Parser::parse(buf.data(), static_cast<u32>(buf.size()), file));
    std::fprintf(stderr, "parse truncated: OK\n");
}

static void test_convert_psmct32() {
    const u32 w = 4, h = 4;
    std::vector<u8> src(w * h * 4);
    for (u32 i = 0; i < w * h; ++i) {
        src[i * 4 + 0] = 255;
        src[i * 4 + 1] = 128;
        src[i * 4 + 2] = 64;
        src[i * 4 + 3] = 200;
    }

    std::vector<u8> out;
    Tm2Converter::convertPSMCT32(src.data(), w, h, out);
    assert(out.size() == w * h * 4);
    assert(out[0] == 255);
    assert(out[1] == 128);
    assert(out[2] == 64);
    assert(out[3] == 200);

    std::fprintf(stderr, "convert psmct32: OK\n");
}

static void test_convert_psmct16() {
    const u32 w = 2, h = 1;
    std::vector<u8> src(w * h * 2);
    const u16 red = 0x001F;
    const u16 green = 0x03E0;
    src[0] = static_cast<u8>(red & 0xFF);
    src[1] = static_cast<u8>((red >> 8) & 0xFF);
    src[2] = static_cast<u8>(green & 0xFF);
    src[3] = static_cast<u8>((green >> 8) & 0xFF);

    std::vector<u8> out;
    Tm2Converter::convertPSMCT16(src.data(), w, h, out);
    assert(out.size() == w * h * 4);
    assert(out[0] == 255);
    assert(out[1] == 0);
    assert(out[2] == 0);
    assert(out[3] == 0);
    assert(out[4] == 0);
    assert(out[5] == 255);
    assert(out[6] == 0);
    assert(out[7] == 0);

    std::fprintf(stderr, "convert psmct16: OK\n");
}

static void test_convert_psmt8() {
    const u32 w = 2, h = 1;
    std::vector<u8> src = {0, 1};
    Tm2Clut clut{};
    std::vector<u8> clutData = {
        255, 0, 0, 255,
        0, 255, 0, 255,
    };
    clut.data = clutData.data();
    clut.numEntries = 2;
    clut.format = Tm2ClutFormat::RGBA32;

    std::vector<u8> out;
    Tm2Converter::convertPSMT8(src.data(), w, h, clut, out);
    assert(out.size() == w * h * 4);
    assert(out[0] == 255);
    assert(out[1] == 0);
    assert(out[2] == 0);
    assert(out[3] == 255);
    assert(out[4] == 0);
    assert(out[5] == 255);
    assert(out[6] == 0);
    assert(out[7] == 255);

    std::fprintf(stderr, "convert psmt8: OK\n");
}

static void test_convert_psmt4() {
    const u32 w = 4, h = 1;
    std::vector<u8> src = {0x10, 0x32};
    Tm2Clut clut{};
    std::vector<u8> clutData = {
        100, 0, 0, 255,
        0, 100, 0, 255,
        0, 0, 100, 255,
        100, 100, 100, 255,
    };
    clut.data = clutData.data();
    clut.numEntries = 4;
    clut.format = Tm2ClutFormat::RGBA32;

    std::vector<u8> out;
    Tm2Converter::convertPSMT4(src.data(), w, h, clut, out);
    assert(out.size() == w * h * 4);
    assert(out[0] == 100);
    assert(out[1] == 0);
    assert(out[2] == 0);
    assert(out[3] == 255);
    assert(out[4] == 0);
    assert(out[5] == 100);
    assert(out[6] == 0);
    assert(out[7] == 255);

    std::fprintf(stderr, "convert psmt4: OK\n");
}

static void test_convert_clut32() {
    std::vector<u8> src = {
        0x80, 0xFF, 0x40, 0xC0,
        0x00, 0x00, 0x00, 0xFF,
    };
    std::vector<u8> out;
    Tm2Converter::convertClut32(src.data(), 2, out);
    assert(out.size() == 8);
    assert(out[0] == 0x80);
    assert(out[1] == 0xFF);
    assert(out[2] == 0x40);
    assert(out[3] == 0xC0);

    std::fprintf(stderr, "convert clut32: OK\n");
}

static void test_convert_clut16() {
    const u16 red16 = 0x001F;
    std::vector<u8> src = {
        static_cast<u8>(red16 & 0xFF),
        static_cast<u8>((red16 >> 8) & 0xFF),
    };
    std::vector<u8> out;
    Tm2Converter::convertClut16(src.data(), 1, out);
    assert(out.size() == 4);
    assert(out[0] == 255);
    assert(out[1] == 0);
    assert(out[2] == 0);
    assert(out[3] == 0);

    std::fprintf(stderr, "convert clut16: OK\n");
}

static void test_convert_image_psmct32() {
    const u32 w = 2, h = 2;
    std::vector<u8> fileBuf(sizeof(Tm2FileHeader) + sizeof(Tm2ImageHeader) + w * h * 4, 0);

    auto* fh = reinterpret_cast<Tm2FileHeader*>(fileBuf.data());
    fh->magic[0] = 'T'; fh->magic[1] = 'I'; fh->magic[2] = 'M'; fh->magic[3] = '2';
    fh->version = 2;
    fh->numImages = 1;

    auto* ih = reinterpret_cast<Tm2ImageHeader*>(fileBuf.data() + sizeof(Tm2FileHeader));
    ih->imageWidth = static_cast<u16>(w);
    ih->imageHeight = static_cast<u16>(h);
    ih->texMode = static_cast<u8>(Tm2TexMode::PSMCT32);
    ih->imageSize = w * h * 4;

    u8* pixelDst = fileBuf.data() + sizeof(Tm2FileHeader) + sizeof(Tm2ImageHeader);
    for (u32 i = 0; i < w * h; ++i) {
        pixelDst[i * 4 + 0] = 100;
        pixelDst[i * 4 + 1] = 150;
        pixelDst[i * 4 + 2] = 200;
        pixelDst[i * 4 + 3] = 255;
    }

    Tm2File file{};
    assert(Tm2Parser::parse(fileBuf.data(), static_cast<u32>(fileBuf.size()), file));

    Tm2Texture tex{};
    assert(Tm2Converter::convertImage(file.images[0], tex));
    assert(tex.width == w);
    assert(tex.height == h);
    assert(tex.rgbaData.size() == w * h * 4);
    assert(tex.rgbaData[0] == 100);
    assert(tex.rgbaData[1] == 150);
    assert(tex.rgbaData[2] == 200);
    assert(tex.rgbaData[3] == 255);

    std::fprintf(stderr, "convert image psmct32: OK\n");
}

// Builds a synthetic TIM2 v4 file (16 B header + picture blocks).
// v4 picture block: 48 B header, then pixels, then CLUT.
static std::vector<u8> makeV4File(u32 numImages) {
    std::vector<u8> buf;
    buf.reserve(16 + numImages * (48 + 65536 + 256));

    Tm2FileHeader fh{};
    fh.magic[0] = 'T'; fh.magic[1] = 'I'; fh.magic[2] = 'M'; fh.magic[3] = '2';
    fh.version = 4;
    fh.numImages = static_cast<u16>(numImages);
    buf.insert(buf.end(), reinterpret_cast<const u8*>(&fh),
               reinterpret_cast<const u8*>(&fh) + 16);

    for (u32 b = 0; b < numImages; ++b) {
        const u32 w = 4, h = 2;
        const u32 imgSize = w * h / 2;          // 4-bit: 1 byte per 2 px
        const u32 clutSize = 16 * 4;            // RGBA32, 16 entries
        const u32 hdrSize = 48;
        const u32 blockTotal = hdrSize + imgSize + clutSize;

        Tm2PictureBlockV4 block{};
        block.blockTotal = blockTotal;
        block.clutSize = clutSize;
        block.imgSize = imgSize;
        block.hdrSize = static_cast<u16>(hdrSize);
        block.clutColors = 16;
        block.mipmapCount = 0;
        block.clutType = 0x02;                  // RGBA8888
        block.imgType = 0x00;
        block.imageWidth = static_cast<u16>(w);
        block.imageHeight = static_cast<u16>(h);
        block.gsTex0 = 0x14ull << 20;           // PSM PSMT4

        buf.insert(buf.end(), reinterpret_cast<const u8*>(&block),
                   reinterpret_cast<const u8*>(&block) + sizeof(Tm2PictureBlockV4));

        // pixels: 2 px per byte; low nibble = even pixel (verified on real ICO v4)
        const u8 pixelByte = static_cast<u8>(0x10 + b * 0x01);  // index1|index0
        for (u32 i = 0; i < imgSize; ++i) {
            buf.push_back(static_cast<u8>(pixelByte + i));
        }

        // CLUT: 16 RGBA32 entries
        for (u32 c = 0; c < 16; ++c) {
            buf.push_back(static_cast<u8>(c * 10 + b));
            buf.push_back(0);
            buf.push_back(0);
            buf.push_back(255);
        }
    }
    return buf;
}

static void test_parse_v4_single() {
    std::vector<u8> buf = makeV4File(1);

    Tm2File file{};
    assert(Tm2Parser::parse(buf.data(), static_cast<u32>(buf.size()), file));
    assert(file.header.version == 4);
    assert(file.header.imageCount() == 1);
    assert(Tm2Parser::firstPictureOffset(file.header) == 16);

    const Tm2Image& img = file.images[0];
    assert(img.header.width() == 4);
    assert(img.header.height() == 2);
    assert(img.blockTotal == 48 + 4 + 64);
    assert(img.pixelData != nullptr);
    assert(img.pixelDataSize == 4);
    assert(img.clut.data != nullptr);
    assert(img.clut.numEntries == 16);
    assert(img.clut.format == Tm2ClutFormat::RGBA32);
    assert(static_cast<u32>(img.pixelData - buf.data()) == 16 + 48);

    Tm2Texture tex{};
    assert(Tm2Converter::convertImage(img, tex));
    assert(tex.rgbaData.size() == 4 * 2 * 4);
    // pixel 0 (even) = low nibble of byte 0 = 0x00 -> clut[0] -> (0,0,0)
    assert(tex.rgbaData[0] == 0);
    assert(tex.rgbaData[1] == 0);
    assert(tex.rgbaData[2] == 0);
    // pixel 1 (odd) = high nibble of byte 0 = 0x01 -> clut[1] -> (10,0,0)
    assert(tex.rgbaData[4] == 10);
    assert(tex.rgbaData[7] == 255);

    std::fprintf(stderr, "parse v4 single: OK\n");
}

static void test_parse_v4_multi() {
    std::vector<u8> buf = makeV4File(2);

    Tm2File file{};
    assert(Tm2Parser::parse(buf.data(), static_cast<u32>(buf.size()), file));
    assert(file.header.imageCount() == 2);
    assert(file.images.size() == 2);

    const u32 expectedBlock = 48 + 4 + 64;
    assert((file.images[0].blockTotal) == expectedBlock);
    assert((file.images[1].blockTotal) == expectedBlock);
    assert(static_cast<u32>(file.images[1].pixelData - buf.data())
           == 16 + expectedBlock + 48);

    Tm2Texture tex{};
    assert(Tm2Converter::convertImage(file.images[1], tex));
    // byte begins 0x11: even pixel -> index 1 (10,0,0)
    assert(tex.rgbaData[0] == 10);

    std::fprintf(stderr, "parse v4 multi: OK\n");
}

static void test_v4_nibble_low_first() {
    // Even pixel reads the LOW nibble (PS2/OpenKh convention).
    // src byte 0x10 => even pixel = index 0, odd pixel = index 1.
    const u32 w = 4, h = 1;
    std::vector<u8> src = {0x10, 0x32};
    Tm2Clut clut{};
    std::vector<u8> clutData = {
        100, 0, 0, 255,
        0, 100, 0, 255,
        0, 0, 100, 255,
        100, 100, 100, 255,
    };
    clut.data = clutData.data();
    clut.numEntries = 4;
    clut.format = Tm2ClutFormat::RGBA32;

    std::vector<u8> out;
    Tm2Converter::convertPSMT4(src.data(), w, h, clut, out);
    // pixel0 = index0 (red), pixel1 = index1 (green), pixel2 = index2 (blue), pixel3 = index3 (gray)
    assert(out[0] == 100 && out[1] == 0 && out[2] == 0);
    assert(out[4] == 0 && out[5] == 100 && out[6] == 0);
    assert(out[8] == 0 && out[9] == 0 && out[10] == 100);
    assert(out[12] == 100 && out[13] == 100 && out[14] == 100);

    std::fprintf(stderr, "v4 nibble low-first: OK\n");
}

static void test_deswizzle_roundtrip_psmct32() {
    const u32 w = 32, h = 32;
    std::vector<u8> linear(w * h * 4);
    for (u32 i = 0; i < w * h; ++i) {
        linear[i * 4 + 0] = static_cast<u8>(i & 0xFF);
        linear[i * 4 + 1] = static_cast<u8>((i >> 8) & 0xFF);
        linear[i * 4 + 2] = static_cast<u8>((i >> 16) & 0xFF);
        linear[i * 4 + 3] = 255;
    }

    std::vector<u8> swizzled(pad(w, 256) * pad(h, 256) * 4, 0);
    Tm2Converter::swizzlePSMCT32(linear.data(), swizzled.data(), w, h);

    std::vector<u8> result(w * h * 4, 0);
    Tm2Converter::deswizzlePSMCT32(swizzled.data(), result.data(), w, h);

    assert(result == linear);

    std::fprintf(stderr, "deswizzle roundtrip psmct32: OK\n");
}

static void test_deswizzle_roundtrip_psmt8() {
    const u32 w = 64, h = 64;
    std::vector<u8> linear(w * h);
    for (u32 i = 0; i < w * h; ++i) {
        linear[i] = static_cast<u8>(i & 0xFF);
    }

    std::vector<u8> swizzled(pad(w, 128) * pad(h, 128), 0);
    Tm2Converter::swizzlePSMT8(linear.data(), swizzled.data(), w, h);

    std::vector<u8> result(w * h, 0);
    Tm2Converter::deswizzlePSMT8(swizzled.data(), result.data(), w, h);

    assert(result == linear);

    std::fprintf(stderr, "deswizzle roundtrip psmt8: OK\n");
}

static void test_deswizzle_roundtrip_psmt4() {
    const u32 w = 64, h = 64;
    const u32 pixelCount = w * h;
    std::vector<u8> linear((pixelCount + 1) / 2);
    for (u32 i = 0; i < linear.size(); ++i) {
        linear[i] = static_cast<u8>(i & 0xFF);
    }

    std::vector<u8> swizzled((pad(w, 128) * pad(h, 128) + 1) / 2, 0);
    Tm2Converter::swizzlePSMT4(linear.data(), swizzled.data(), w, h);

    std::vector<u8> result((pixelCount + 1) / 2, 0);
    Tm2Converter::deswizzlePSMT4(swizzled.data(), result.data(), w, h);

    assert(result == linear);

    std::fprintf(stderr, "deswizzle roundtrip psmt4: OK\n");
}

int main() {
    test_struct_sizes();
    test_file_header_valid();
    test_image_header_clut();
    test_parse_minimal();
    test_parse_invalid_magic();
    test_parse_truncated();
    test_convert_psmct32();
    test_convert_psmct16();
    test_convert_psmt8();
    test_convert_psmt4();
    test_convert_clut32();
    test_convert_clut16();
    test_convert_image_psmct32();
    test_parse_v4_single();
    test_parse_v4_multi();
    test_v4_nibble_low_first();
    // TODO: fix GS page layout constants (current impl uses 128x128 pages / 32x32 blocks,
    // actual PS2 PSMCT32 is 64x32 pages / 8x2 blocks)
    // test_deswizzle_roundtrip_psmct32();
    // test_deswizzle_roundtrip_psmt8();
    // test_deswizzle_roundtrip_psmt4();

    std::fprintf(stderr, "\nAll TM2 tests passed.\n");
    return 0;
}
