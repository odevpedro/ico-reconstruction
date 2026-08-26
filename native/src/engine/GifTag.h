#pragma once

#include "ps2/Ps2Types.h"

#include <cstring>

namespace ico::engine {

enum class GifFlg : u8 {
    Packed   = 0,
    Regs     = 1,
    Image    = 2,
    Unpacked = 3,
};

enum class GifReg : u8 {
    PRIM   = 0x00,
    RGBAQ  = 0x01,
    STQ    = 0x02,
    UV     = 0x03,
    XYZ2   = 0x04,
    XYZ3   = 0x05,
    TEX0_1 = 0x06,
    TEX0_2 = 0x07,
    TEX1_1 = 0x08,
    TEX1_2 = 0x09,
    TEX2_1 = 0x0A,
    TEX2_2 = 0x0B,
    CLAMP_1 = 0x0C,
    CLAMP_2 = 0x0D,
    A_D     = 0x0E,
    NOP     = 0x0F,
};

enum class GsPrimType : u8 {
    Point     = 0,
    Line      = 1,
    LineStrip = 2,
    Triangle  = 3,
    TriStrip  = 4,
    TriFan    = 5,
    Sprite    = 6,
    Patch     = 7,
};

constexpr u16 kGifTagSize = 16;

struct GifTag {
    u8 bytes[kGifTagSize];

    u16 nloop() const;
    bool eop() const;
    bool pre() const;
    u32 prim() const;
    GifFlg flg() const;
    u32 nreg() const;
    u32 regCount() const;
    GifReg reg(u32 index) const;

    void setNloop(u16 value);
    void setEop(bool value);
    void setPre(bool value);
    void setPrim(u32 value);
    void setFlg(GifFlg value);
    void setNreg(u32 value);
    void setReg(u32 index, GifReg value);

    static GifTag makeSimple(u16 nloop, bool eop, GifFlg flg, u32 nreg);
    static GifTag makePrim(u16 nloop, bool eop, u32 prim);
};

static_assert(sizeof(GifTag) == kGifTagSize, "GifTag must be 16 bytes");

struct GsPrimReg {
    u32 value;

    GsPrimType type() const;
    bool iip() const;
    bool tme() const;
    bool fge() const;
    bool abe() const;
    bool aa1() const;
    bool fix() const;

    void setType(GsPrimType t);
    void setIip(bool v);
    void setTme(bool v);
    void setFge(bool v);
    void setAbe(bool v);
    void setAa1(bool v);
    void setFix(bool v);

    static GsPrimReg make(GsPrimType type, bool gouraud, bool texture,
                          bool fog, bool alpha, bool aa, bool fixedAlpha);
};

constexpr u32 kGsAddrPRIM    = 0x00;
constexpr u32 kGsAddrRGBAQ   = 0x01;
constexpr u32 kGsAddrSTQ     = 0x02;
constexpr u32 kGsAddrUV      = 0x03;
constexpr u32 kGsAddrXYZ2    = 0x04;
constexpr u32 kGsAddrXYZ3    = 0x05;
constexpr u32 kGsAddrTEX0_1  = 0x06;
constexpr u32 kGsAddrTEX0_2  = 0x07;
constexpr u32 kGsAddrTEX1_1  = 0x08;
constexpr u32 kGsAddrTEX1_2  = 0x09;
constexpr u32 kGsAddrTEX2_1  = 0x0A;
constexpr u32 kGsAddrTEX2_2  = 0x0B;
constexpr u32 kGsAddrCLAMP_1 = 0x0C;
constexpr u32 kGsAddrCLAMP_2 = 0x0D;
constexpr u32 kGsAddrFOG     = 0x0E;
constexpr u32 kGsAddrNOP     = 0x0F;

struct GsTex0 {
    u64 value;

    u32 tbp0() const;
    u32 tbw() const;
    u32 psm() const;
    u32 tw() const;
    u32 th() const;
    u32 tcc() const;
    u32 tfx() const;
    u32 cbp() const;
    u32 cpsm() const;
    u32 csm() const;
    u32 csa() const;
    u32 cld() const;

    void setTbp0(u32 v);
    void setTbw(u32 v);
    void setPsm(u32 v);
    void setTw(u32 v);
    void setTh(u32 v);
    void setTcc(u32 v);
    void setTfx(u32 v);
    void setCbp(u32 v);
    void setCpsm(u32 v);
    void setCsm(u32 v);
    void setCsa(u32 v);
    void setCld(u32 v);
};

struct GsTex1 {
    u64 value;

    u32 lcm() const;
    u32 mxl() const;
    u32 mmag() const;
    u32 mmin() const;
    u32 mtba() const;
    u32 l() const;
    u32 k() const;
    u32 fix1() const;

    void setLcm(u32 v);
    void setMxl(u32 v);
    void setMmag(u32 v);
    void setMmin(u32 v);
    void setMtba(u32 v);
    void setL(u32 v);
    void setK(u32 v);
    void setFix1(u32 v);
};

struct GsFrame {
    u64 value;

    u32 fw() const;
    u32 fh() const;
    u32 psm() const;
    u32 fbw() const;
    u32 fbp() const;

    void setFw(u32 v);
    void setFh(u32 v);
    void setPsm(u32 v);
    void setFbw(u32 v);
    void setFbp(u32 v);
};

struct GsZbuf {
    u64 value;

    u32 zbp() const;
    u32 psm() const;
    u32 zmsk() const;

    void setZbp(u32 v);
    void setPsm(u32 v);
    void setZmsk(u32 v);
};

struct GsAlpha {
    u32 value;

    u32 aba() const;
    u32 abb() const;
    u32 abc() const;
    u32 abd() const;
    u32 afix() const;

    void setAba(u32 v);
    void setAbb(u32 v);
    void setAbc(u32 v);
    void setAbd(u32 v);
    void setAfix(u32 v);
};

struct GsTest {
    u64 value;

    u32 ate() const;
    u32 atst() const;
    u32 aref() const;
    u32 afail() const;
    u32 date() const;
    u32 datm() const;
    u32 zte() const;
    u32 ztst() const;

    void setAte(u32 v);
    void setAtst(u32 v);
    void setAref(u32 v);
    void setAfail(u32 v);
    void setDate(u32 v);
    void setDatm(u32 v);
    void setZte(u32 v);
    void setZtst(u32 v);
};

struct GsRgbaq {
    u64 value;

    u8 r() const;
    u8 g() const;
    u8 b() const;
    u8 a() const;
    float q() const;

    void setR(u8 v);
    void setG(u8 v);
    void setB(u8 v);
    void setA(u8 v);
    void setQ(float v);

    static GsRgbaq make(u8 r, u8 g, u8 b, u8 a, float q = 1.0f);
};

struct GsStq {
    u64 value;

    float s() const;
    float t() const;

    static GsStq make(float s, float t);
};

struct GsUv {
    u32 value;

    float u() const;
    float v() const;

    static GsUv make(float u, float v);
};

struct GsXyz2 {
    u64 value;

    float x() const;
    float y() const;
    float z() const;

    bool sar() const;

    static GsXyz2 make(float x, float y, float z, bool sar = false);
};

struct GsFog {
    u32 value;

    u8 fmin() const;
    u8 fmax() const;

    static GsFog make(u8 fmin, u8 fmax);
};

} // namespace ico::engine
