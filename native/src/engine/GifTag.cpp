#include "engine/GifTag.h"

#include <cstring>

namespace ico::engine {

u16 GifTag::nloop() const {
    u16 v = 0;
    std::memcpy(&v, bytes + 0, 2);
    return v & 0x7FFF;
}

bool GifTag::eop() const {
    return (bytes[2] & 0x01) != 0;
}

bool GifTag::pre() const {
    return (bytes[3] & 0x04) != 0;
}

u32 GifTag::prim() const {
    u32 v = 0;
    v |= static_cast<u32>(bytes[3] & 0x03) << 0;
    v |= static_cast<u32>(bytes[4]) << 2;
    v |= static_cast<u32>(bytes[5] & 0x07) << 10;
    return v & 0x7FF;
}

GifFlg GifTag::flg() const {
    return static_cast<GifFlg>((bytes[5] >> 3) & 0x03);
}

u32 GifTag::nreg() const {
    u32 v = (bytes[6] >> 0) & 0x0F;
    return v == 0 ? 16 : v;
}

u32 GifTag::regCount() const {
    u32 f = static_cast<u32>(flg());
    u32 nr = nreg();
    if (f == 0 || f == 1) {
        return nr;
    }
    return 0;
}

GifReg GifTag::reg(u32 index) const {
    if (index >= 16) {
        return GifReg::NOP;
    }
    u32 byteOffset = 8 + (index / 2);
    u32 shift = (index % 2) * 4;
    if (byteOffset >= kGifTagSize) {
        return GifReg::NOP;
    }
    u32 val = (bytes[byteOffset] >> shift) & 0x0F;
    return static_cast<GifReg>(val);
}

void GifTag::setNloop(u16 value) {
    std::memcpy(bytes + 0, &value, 2);
    bytes[1] &= 0x7F;
}

void GifTag::setEop(bool value) {
    if (value) {
        bytes[2] |= 0x01;
    } else {
        bytes[2] &= ~0x01;
    }
}

void GifTag::setPre(bool value) {
    if (value) {
        bytes[3] |= 0x04;
    } else {
        bytes[3] &= ~0x04;
    }
}

void GifTag::setPrim(u32 value) {
    bytes[3] = (bytes[3] & 0xFC) | static_cast<u8>((value >> 0) & 0x03);
    bytes[4] = static_cast<u8>((value >> 2) & 0xFF);
    bytes[5] = (bytes[5] & 0xF8) | static_cast<u8>((value >> 10) & 0x07);
}

void GifTag::setFlg(GifFlg value) {
    bytes[5] = (bytes[5] & 0xE7) | (static_cast<u8>(value) & 0x03) << 3;
}

void GifTag::setNreg(u32 value) {
    u32 enc = value == 16 ? 0 : value;
    bytes[6] = (bytes[6] & 0xF0) | (enc & 0x0F);
}

void GifTag::setReg(u32 index, GifReg value) {
    if (index >= 16) return;
    u32 byteOffset = 8 + (index / 2);
    if (byteOffset >= kGifTagSize) return;
    u32 shift = (index % 2) * 4;
    bytes[byteOffset] = (bytes[byteOffset] & ~(0x0F << shift)) |
                        (static_cast<u8>(value) & 0x0F) << shift;
}

GifTag GifTag::makeSimple(u16 nloop, bool eop, GifFlg flg, u32 nreg) {
    GifTag tag{};
    tag.setNloop(nloop);
    tag.setEop(eop);
    tag.setFlg(flg);
    tag.setNreg(nreg);
    return tag;
}

GifTag GifTag::makePrim(u16 nloop, bool eop, u32 prim) {
    GifTag tag{};
    tag.setNloop(nloop);
    tag.setEop(eop);
    tag.setPre(true);
    tag.setPrim(prim);
    tag.setFlg(GifFlg::Packed);
    tag.setNreg(3);
    tag.setReg(0, GifReg::PRIM);
    tag.setReg(1, GifReg::RGBAQ);
    tag.setReg(2, GifReg::STQ);
    return tag;
}

GsPrimType GsPrimReg::type() const {
    return static_cast<GsPrimType>(value & 0x07);
}

bool GsPrimReg::iip() const { return (value & 0x10) != 0; }
bool GsPrimReg::tme() const { return (value & 0x20) != 0; }
bool GsPrimReg::fge() const { return (value & 0x40) != 0; }
bool GsPrimReg::abe() const { return (value & 0x80) != 0; }
bool GsPrimReg::aa1() const { return (value & 0x100) != 0; }
bool GsPrimReg::fix() const { return (value & 0x200) != 0; }

void GsPrimReg::setType(GsPrimType t) {
    value = (value & ~0x07u) | (static_cast<u32>(t) & 0x07);
}
void GsPrimReg::setIip(bool v)  { if (v) value |= 0x10; else value &= ~0x10u; }
void GsPrimReg::setTme(bool v)  { if (v) value |= 0x20; else value &= ~0x20u; }
void GsPrimReg::setFge(bool v)  { if (v) value |= 0x40; else value &= ~0x40u; }
void GsPrimReg::setAbe(bool v)  { if (v) value |= 0x80; else value &= ~0x80u; }
void GsPrimReg::setAa1(bool v)  { if (v) value |= 0x100; else value &= ~0x100u; }
void GsPrimReg::setFix(bool v)  { if (v) value |= 0x200; else value &= ~0x200u; }

GsPrimReg GsPrimReg::make(GsPrimType type, bool gouraud, bool texture,
                           bool fog, bool alpha, bool aa, bool fixedAlpha) {
    GsPrimReg reg{};
    reg.setType(type);
    reg.setIip(gouraud);
    reg.setTme(texture);
    reg.setFge(fog);
    reg.setAbe(alpha);
    reg.setAa1(aa);
    reg.setFix(fixedAlpha);
    return reg;
}

u32 GsTex0::tbp0() const  { return static_cast<u32>(value) & 0x3FFF; }
u32 GsTex0::tbw() const   { return (static_cast<u32>(value) >> 14) & 0x3F; }
u32 GsTex0::psm() const   { return (static_cast<u32>(value) >> 20) & 0x3F; }
u32 GsTex0::tw() const    { return static_cast<u32>((value >> 26) & 0x0F); }
u32 GsTex0::th() const    { return static_cast<u32>((value >> 30) & 0x0F); }
u32 GsTex0::tcc() const   { return static_cast<u32>((value >> 34) & 0x01); }
u32 GsTex0::tfx() const   { return static_cast<u32>((value >> 35) & 0x03); }
u32 GsTex0::cbp() const   { return static_cast<u32>((value >> 37) & 0x3FFF); }
u32 GsTex0::cpsm() const  { return static_cast<u32>((value >> 51) & 0x0F); }
u32 GsTex0::csm() const   { return static_cast<u32>((value >> 55) & 0x01); }
u32 GsTex0::csa() const   { return static_cast<u32>((value >> 56) & 0x1F); }
u32 GsTex0::cld() const   { return static_cast<u32>((value >> 61) & 0x07); }

void GsTex0::setTbp0(u32 v)  { value = (value & ~0x3FFFull) | (v & 0x3FFF); }
void GsTex0::setTbw(u32 v)   { value = (value & ~(0x3Full << 14)) | (static_cast<u64>(v & 0x3F) << 14); }
void GsTex0::setPsm(u32 v)   { value = (value & ~(0x3Full << 20)) | (static_cast<u64>(v & 0x3F) << 20); }
void GsTex0::setTw(u32 v)    { value = (value & ~(0x0Full << 26)) | (static_cast<u64>(v & 0x0F) << 26); }
void GsTex0::setTh(u32 v)    { value = (value & ~(0x0Full << 30)) | (static_cast<u64>(v & 0x0F) << 30); }
void GsTex0::setTcc(u32 v)   { value = (value & ~(1ull << 34)) | (static_cast<u64>(v & 1) << 34); }
void GsTex0::setTfx(u32 v)   { value = (value & ~(3ull << 35)) | (static_cast<u64>(v & 3) << 35); }
void GsTex0::setCbp(u32 v)   { value = (value & ~(0x3Full << 37)) | (static_cast<u64>(v & 0x3FFF) << 37); }
void GsTex0::setCpsm(u32 v)  { value = (value & ~(0x0Full << 51)) | (static_cast<u64>(v & 0x0F) << 51); }
void GsTex0::setCsm(u32 v)   { value = (value & ~(1ull << 55)) | (static_cast<u64>(v & 1) << 55); }
void GsTex0::setCsa(u32 v)   { value = (value & ~(0x1Full << 56)) | (static_cast<u64>(v & 0x1F) << 56); }
void GsTex0::setCld(u32 v)   { value = (value & ~(7ull << 61)) | (static_cast<u64>(v & 7) << 61); }

u32 GsTex1::lcm() const   { return static_cast<u32>(value) & 0x01; }
u32 GsTex1::mxl() const   { return (static_cast<u32>(value) >> 2) & 0x07; }
u32 GsTex1::mmag() const  { return (static_cast<u32>(value) >> 5) & 0x01; }
u32 GsTex1::mmin() const  { return (static_cast<u32>(value) >> 6) & 0x07; }
u32 GsTex1::mtba() const  { return (static_cast<u32>(value) >> 9) & 0x01; }
u32 GsTex1::l() const     { return (static_cast<u32>(value) >> 19) & 0x01; }
u32 GsTex1::k() const     { return (static_cast<u32>(value) >> 20) & 0xFFF; }
u32 GsTex1::fix1() const  { return static_cast<u32>((value >> 32) & 0x01); }

void GsTex1::setLcm(u32 v)   { value = (value & ~1ull) | (v & 1); }
void GsTex1::setMxl(u32 v)   { value = (value & ~(7ull << 2)) | (static_cast<u64>(v & 7) << 2); }
void GsTex1::setMmag(u32 v)  { value = (value & ~(1ull << 5)) | (static_cast<u64>(v & 1) << 5); }
void GsTex1::setMmin(u32 v)  { value = (value & ~(7ull << 6)) | (static_cast<u64>(v & 7) << 6); }
void GsTex1::setMtba(u32 v)  { value = (value & ~(1ull << 9)) | (static_cast<u64>(v & 1) << 9); }
void GsTex1::setL(u32 v)     { value = (value & ~(1ull << 19)) | (static_cast<u64>(v & 1) << 19); }
void GsTex1::setK(u32 v)     { value = (value & ~(0xFFFull << 20)) | (static_cast<u64>(v & 0xFFF) << 20); }
void GsTex1::setFix1(u32 v)  { value = (value & ~(1ull << 32)) | (static_cast<u64>(v & 1) << 32); }

u32 GsFrame::fw() const   { return static_cast<u32>((value >> 0) & 0x7FF); }
u32 GsFrame::fh() const   { return static_cast<u32>((value >> 11) & 0xFFF); }
u32 GsFrame::psm() const  { return static_cast<u32>((value >> 24) & 0x3F); }
u32 GsFrame::fbw() const  { return static_cast<u32>((value >> 32) & 0x3F); }
u32 GsFrame::fbp() const  { return static_cast<u32>((value >> 36) & 0x1FF); }

void GsFrame::setFw(u32 v)   { value = (value & ~0x7FFull) | (v & 0x7FF); }
void GsFrame::setFh(u32 v)   { value = (value & ~(0xFFFull << 11)) | (static_cast<u64>(v & 0xFFF) << 11); }
void GsFrame::setPsm(u32 v)  { value = (value & ~(0x3Full << 24)) | (static_cast<u64>(v & 0x3F) << 24); }
void GsFrame::setFbw(u32 v)  { value = (value & ~(0x3Full << 32)) | (static_cast<u64>(v & 0x3F) << 32); }
void GsFrame::setFbp(u32 v)  { value = (value & ~(0x1FFull << 36)) | (static_cast<u64>(v & 0x1FF) << 36); }

u32 GsZbuf::zbp() const   { return static_cast<u32>(value) & 0x1FF; }
u32 GsZbuf::psm() const   { return static_cast<u32>((value >> 24) & 0x0F); }
u32 GsZbuf::zmsk() const  { return static_cast<u32>((value >> 31) & 0x01); }

void GsZbuf::setZbp(u32 v)   { value = (value & ~0x1FFull) | (v & 0x1FF); }
void GsZbuf::setPsm(u32 v)   { value = (value & ~(0x0Full << 24)) | (static_cast<u64>(v & 0x0F) << 24); }
void GsZbuf::setZmsk(u32 v)  { value = (value & ~(1ull << 31)) | (static_cast<u64>(v & 1) << 31); }

u32 GsAlpha::aba() const  { return value & 0x03; }
u32 GsAlpha::abb() const  { return (value >> 2) & 0x03; }
u32 GsAlpha::abc() const  { return (value >> 4) & 0x03; }
u32 GsAlpha::abd() const  { return (value >> 6) & 0x03; }
u32 GsAlpha::afix() const { return value & 0xFF; }

void GsAlpha::setAba(u32 v)  { value = (value & ~0x03u) | (v & 0x03); }
void GsAlpha::setAbb(u32 v)  { value = (value & ~(0x03u << 2)) | ((v & 0x03) << 2); }
void GsAlpha::setAbc(u32 v)  { value = (value & ~(0x03u << 4)) | ((v & 0x03) << 4); }
void GsAlpha::setAbd(u32 v)  { value = (value & ~(0x03u << 6)) | ((v & 0x03) << 6); }
void GsAlpha::setAfix(u32 v) { value = (value & ~0xFF00u) | ((v & 0xFF) << 8); }

u32 GsTest::ate() const   { return static_cast<u32>(value) & 0x01; }
u32 GsTest::atst() const  { return static_cast<u32>((value >> 1) & 0x07); }
u32 GsTest::aref() const  { return static_cast<u32>((value >> 4) & 0xFF); }
u32 GsTest::afail() const { return static_cast<u32>((value >> 12) & 0x03); }
u32 GsTest::date() const  { return static_cast<u32>((value >> 14) & 0x01); }
u32 GsTest::datm() const  { return static_cast<u32>((value >> 15) & 0x01); }
u32 GsTest::zte() const   { return static_cast<u32>((value >> 16) & 0x01); }
u32 GsTest::ztst() const  { return static_cast<u32>((value >> 17) & 0x03); }

void GsTest::setAte(u32 v)   { value = (value & ~1ull) | (v & 1); }
void GsTest::setAtst(u32 v)  { value = (value & ~(7ull << 1)) | (static_cast<u64>(v & 7) << 1); }
void GsTest::setAref(u32 v)  { value = (value & ~(0xFFull << 4)) | (static_cast<u64>(v & 0xFF) << 4); }
void GsTest::setAfail(u32 v) { value = (value & ~(3ull << 12)) | (static_cast<u64>(v & 3) << 12); }
void GsTest::setDate(u32 v)  { value = (value & ~(1ull << 14)) | (static_cast<u64>(v & 1) << 14); }
void GsTest::setDatm(u32 v)  { value = (value & ~(1ull << 15)) | (static_cast<u64>(v & 1) << 15); }
void GsTest::setZte(u32 v)   { value = (value & ~(1ull << 16)) | (static_cast<u64>(v & 1) << 16); }
void GsTest::setZtst(u32 v)  { value = (value & ~(3ull << 17)) | (static_cast<u64>(v & 3) << 17); }

u8 GsRgbaq::r() const { return static_cast<u8>(value); }
u8 GsRgbaq::g() const { return static_cast<u8>(value >> 8); }
u8 GsRgbaq::b() const { return static_cast<u8>(value >> 16); }
u8 GsRgbaq::a() const { return static_cast<u8>(value >> 24); }

void GsRgbaq::setR(u8 v) { value = (value & ~0xFFull) | v; }
void GsRgbaq::setG(u8 v) { value = (value & ~0xFF00ull) | (static_cast<u64>(v) << 8); }
void GsRgbaq::setB(u8 v) { value = (value & ~0xFF0000ull) | (static_cast<u64>(v) << 16); }
void GsRgbaq::setA(u8 v) { value = (value & ~0xFF000000ull) | (static_cast<u64>(v) << 24); }

float GsRgbaq::q() const {
    float fq;
    u32 qbits = static_cast<u32>(value >> 32);
    std::memcpy(&fq, &qbits, sizeof(fq));
    return fq;
}

void GsRgbaq::setQ(float v) {
    u32 qbits;
    std::memcpy(&qbits, &v, sizeof(qbits));
    value = (value & 0xFFFFFFFFull) | (static_cast<u64>(qbits) << 32);
}

GsRgbaq GsRgbaq::make(u8 r, u8 g, u8 b, u8 a, float q) {
    GsRgbaq rgbaq{};
    rgbaq.setR(r);
    rgbaq.setG(g);
    rgbaq.setB(b);
    rgbaq.setA(a);
    rgbaq.setQ(q);
    return rgbaq;
}

float GsStq::s() const {
    float fs;
    u32 sbits = static_cast<u32>(value);
    std::memcpy(&fs, &sbits, sizeof(fs));
    return fs;
}

float GsStq::t() const {
    float ft;
    u32 tbits = static_cast<u32>(value >> 32);
    std::memcpy(&ft, &tbits, sizeof(ft));
    return ft;
}

GsStq GsStq::make(float s, float t) {
    GsStq stq{};
    u32 sbits;
    u32 tbits;
    std::memcpy(&sbits, &s, sizeof(sbits));
    std::memcpy(&tbits, &t, sizeof(tbits));
    stq.value = static_cast<u64>(sbits) | (static_cast<u64>(tbits) << 32);
    return stq;
}

float GsUv::u() const { return static_cast<float>(value & 0xFFFF) / 16.0f; }
float GsUv::v() const { return static_cast<float>((value >> 16) & 0xFFFF) / 16.0f; }

GsUv GsUv::make(float u, float v) {
    GsUv uv{};
    u32 ui = static_cast<u32>(u * 16.0f) & 0xFFFF;
    u32 vi = static_cast<u32>(v * 16.0f) & 0xFFFF;
    uv.value = ui | (vi << 16);
    return uv;
}

float GsXyz2::x() const { return static_cast<float>(static_cast<s32>(value & 0xFFFF)) / 16.0f; }
float GsXyz2::y() const { return static_cast<float>(static_cast<s32>((value >> 16) & 0xFFFF)) / 16.0f; }
float GsXyz2::z() const { return static_cast<float>(static_cast<s32>(value >> 32)) / 1.0f; }
bool GsXyz2::sar() const { return (value & 0x8000000000000000ull) != 0; }

GsXyz2 GsXyz2::make(float x, float y, float z, bool sar) {
    GsXyz2 xyz{};
    s32 sx = static_cast<s32>(x * 16.0f);
    s32 sy = static_cast<s32>(y * 16.0f);
    s32 sz = static_cast<s32>(z);
    u64 val = 0;
    val |= static_cast<u64>(sx & 0xFFFF);
    val |= static_cast<u64>(sy & 0xFFFF) << 16;
    val |= static_cast<u64>(sz & 0xFFFFFFFF) << 32;
    if (sar) {
        val |= 0x8000000000000000ull;
    }
    xyz.value = val;
    return xyz;
}

u8 GsFog::fmin() const { return static_cast<u8>(value); }
u8 GsFog::fmax() const { return static_cast<u8>(value >> 8); }

GsFog GsFog::make(u8 fmin, u8 fmax) {
    GsFog fog{};
    fog.value = fmin | (static_cast<u32>(fmax) << 8);
    return fog;
}

} // namespace ico::engine
