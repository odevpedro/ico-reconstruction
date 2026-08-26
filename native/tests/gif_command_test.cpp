#include "engine/GifTag.h"
#include "engine/GifCommandBuffer.h"

#include <cassert>
#include <cstdio>
#include <cstring>
#include <vector>

using ico::engine::GifTag;
using ico::engine::GifFlg;
using ico::engine::GifReg;
using ico::engine::GsPrimReg;
using ico::engine::GsPrimType;
using ico::engine::GsRgbaq;
using ico::engine::GsStq;
using ico::engine::GsUv;
using ico::engine::GsXyz2;
using ico::engine::GsTex0;
using ico::engine::GsTex1;
using ico::engine::GsAlpha;
using ico::engine::GsTest;
using ico::engine::GsFrame;
using ico::engine::GsZbuf;
using ico::engine::GsFog;
using ico::engine::GifCommandBuffer;
using ico::engine::kGifTagSize;
using ico::engine::RenderCommand;
using ico::engine::RenderCmd;
using ico::engine::GSPrimitive;
using ico::engine::GSBlendMode;
using ico::engine::GSDepthTest;

static void test_gif_tag_nloop() {
    GifTag tag{};
    tag.setNloop(5);
    u16 n5 = tag.nloop();
    assert(n5 == 5);

    tag.setNloop(0);
    u16 n0 = tag.nloop();
    assert(n0 == 0);

    tag.setNloop(0x7FFF);
    u16 nmax = tag.nloop();
    assert(nmax == 0x7FFF);
    (void)n5; (void)n0; (void)nmax;
}

static void test_gif_tag_eop() {
    GifTag tag{};
    assert(!tag.eop());

    tag.setEop(true);
    assert(tag.eop());

    tag.setEop(false);
    assert(!tag.eop());
}

static void test_gif_tag_pre() {
    GifTag tag{};
    assert(!tag.pre());

    tag.setPre(true);
    assert(tag.pre());
}

static void test_gif_tag_flg() {
    GifTag tag{};
    tag.setFlg(GifFlg::Packed);
    GifFlg f1 = tag.flg();
    assert(f1 == GifFlg::Packed);

    tag.setFlg(GifFlg::Regs);
    GifFlg f2 = tag.flg();
    assert(f2 == GifFlg::Regs);

    tag.setFlg(GifFlg::Image);
    GifFlg f3 = tag.flg();
    assert(f3 == GifFlg::Image);
    (void)f1; (void)f2; (void)f3;
}

static void test_gif_tag_nreg() {
    GifTag tag{};
    tag.setNreg(3);
    assert(tag.nreg() == 3);

    tag.setNreg(16);
    assert(tag.nreg() == 16);

    tag.setNreg(0);
    assert(tag.nreg() == 16);
}

static void test_gif_tag_reg() {
    GifTag tag{};
    tag.setReg(0, GifReg::PRIM);
    tag.setReg(1, GifReg::RGBAQ);
    tag.setReg(2, GifReg::STQ);

    assert(tag.reg(0) == GifReg::PRIM);
    assert(tag.reg(1) == GifReg::RGBAQ);
    assert(tag.reg(2) == GifReg::STQ);
    assert(tag.reg(3) == GifReg::PRIM);
}

static void test_gif_tag_make_simple() {
    GifTag tag = GifTag::makeSimple(4, true, GifFlg::Packed, 3);
    assert(tag.nloop() == 4);
    assert(tag.eop());
    assert(tag.flg() == GifFlg::Packed);
    assert(tag.nreg() == 3);
    (void)tag;
}

static void test_gif_tag_make_prim() {
    u32 prim = 6 | (1 << 4) | (1 << 5);
    GifTag tag = GifTag::makePrim(2, false, prim);
    assert(tag.nloop() == 2);
    assert(!tag.eop());
    assert(tag.pre());
    assert(tag.flg() == GifFlg::Packed);
    assert(tag.nreg() == 3);
    assert(tag.reg(0) == GifReg::PRIM);
    assert(tag.reg(1) == GifReg::RGBAQ);
    assert(tag.reg(2) == GifReg::STQ);
    (void)tag;
}

static void test_gs_prim_reg() {
    GsPrimReg reg = GsPrimReg::make(GsPrimType::Sprite, true, true, false, true, false, false);
    assert(reg.type() == GsPrimType::Sprite);
    assert(reg.iip());
    assert(reg.tme());
    assert(!reg.fge());
    assert(reg.abe());
    assert(!reg.aa1());
    assert(!reg.fix());

    reg.setAbe(false);
    assert(!reg.abe());
    reg.setAbe(true);
    assert(reg.abe());
}

static void test_gs_rgbaq() {
    GsRgbaq rgbaq = GsRgbaq::make(128, 64, 32, 255, 2.0f);
    assert(rgbaq.r() == 128);
    assert(rgbaq.g() == 64);
    assert(rgbaq.b() == 32);
    assert(rgbaq.a() == 255);
    float q = rgbaq.q();
    assert(q > 1.9f && q < 2.1f);
    (void)rgbaq; (void)q;
}

static void test_gs_stq() {
    GsStq stq = GsStq::make(0.5f, 1.0f);
    float s = stq.s();
    float t = stq.t();
    assert(s > 0.4f && s < 0.6f);
    assert(t > 0.9f && t < 1.1f);
    (void)stq; (void)s; (void)t;
}

static void test_gs_uv() {
    GsUv uv = GsUv::make(128.0f, 64.0f);
    float u = uv.u();
    float v = uv.v();
    assert(u > 127.0f && u < 129.0f);
    assert(v > 63.0f && v < 65.0f);
    (void)uv; (void)u; (void)v;
}

static void test_gs_xyz2() {
    GsXyz2 xyz = GsXyz2::make(100.0f, 200.0f, 0.0f);
    float x = xyz.x();
    float y = xyz.y();
    assert(x > 99.0f && x < 101.0f);
    assert(y > 199.0f && y < 201.0f);
    assert(!xyz.sar());
    (void)xyz; (void)x; (void)y;

    GsXyz2 xyzSar = GsXyz2::make(0.0f, 0.0f, 0.0f, true);
    assert(xyzSar.sar());
    (void)xyzSar;
}

static void test_gs_tex0() {
    GsTex0 tex0{};
    tex0.setTbp0(100);
    tex0.setTbw(2);
    tex0.setPsm(0);
    tex0.setTw(6);
    tex0.setTh(6);

    assert(tex0.tbp0() == 100);
    assert(tex0.tbw() == 2);
    assert(tex0.psm() == 0);
    assert(tex0.tw() == 6);
    assert(tex0.th() == 6);
}

static void test_gs_tex1() {
    GsTex1 tex1{};
    tex1.setMmag(1);
    tex1.setMmin(4);

    assert(tex1.mmag() == 1);
    assert(tex1.mmin() == 4);
}

static void test_gs_alpha() {
    GsAlpha alpha{};
    alpha.setAba(0);
    alpha.setAbb(1);
    alpha.setAbc(0);
    alpha.setAbd(2);

    assert(alpha.aba() == 0);
    assert(alpha.abb() == 1);
    assert(alpha.abc() == 0);
    assert(alpha.abd() == 2);
}

static void test_gs_test() {
    GsTest t{};
    t.setAte(1);
    t.setAtst(4);
    t.setAref(0x80);
    t.setZte(1);
    t.setZtst(2);

    assert(t.ate() == 1);
    assert(t.atst() == 4);
    assert(t.aref() == 0x80);
    assert(t.zte() == 1);
    assert(t.ztst() == 2);
}

static void test_gs_frame() {
    GsFrame frame{};
    frame.setFw(32);
    frame.setFh(256);
    frame.setPsm(0);
    frame.setFbw(10);

    assert(frame.fw() == 32);
    assert(frame.fh() == 256);
    assert(frame.psm() == 0);
    assert(frame.fbw() == 10);
}

static void test_gs_zbuf() {
    GsZbuf zbuf{};
    zbuf.setZbp(200);
    zbuf.setPsm(0);
    zbuf.setZmsk(1);

    assert(zbuf.zbp() == 200);
    assert(zbuf.psm() == 0);
    assert(zbuf.zmsk() == 1);
}

static void test_gs_fog() {
    GsFog fog = GsFog::make(32, 200);
    u8 fmin = fog.fmin();
    u8 fmax = fog.fmax();
    assert(fmin == 32);
    assert(fmax == 200);
    (void)fog; (void)fmin; (void)fmax;
}

static void test_command_buffer_empty() {
    GifCommandBuffer buf;
    assert(buf.commandCount() == 0);
    assert(buf.commands().empty());
}

static void test_command_buffer_sprite_packet() {
    GifCommandBuffer buf;

    std::vector<u8> packet;

    u32 primVal = 6 | (1 << 5);
    GifTag tag{};
    tag.setNloop(6);
    tag.setEop(true);
    tag.setPre(true);
    tag.setPrim(primVal);
    tag.setFlg(GifFlg::Packed);
    tag.setNreg(3);
    tag.setReg(0, GifReg::RGBAQ);
    tag.setReg(1, GifReg::UV);
    tag.setReg(2, GifReg::XYZ2);
    packet.resize(kGifTagSize);
    std::memcpy(packet.data(), tag.bytes, kGifTagSize);

    GsRgbaq rgbaq0 = GsRgbaq::make(255, 128, 64, 200);
    packet.resize(packet.size() + 16);
    std::memcpy(packet.data() + kGifTagSize, &rgbaq0.value, 8);

    GsUv uv0 = GsUv::make(0.0f, 0.0f);
    packet.resize(packet.size() + 16);
    std::memcpy(packet.data() + kGifTagSize + 16, &uv0.value, 4);

    GsXyz2 xyz0 = GsXyz2::make(10.0f, 20.0f, 0.0f);
    packet.resize(packet.size() + 16);
    std::memcpy(packet.data() + kGifTagSize + 32, &xyz0.value, 8);

    GsRgbaq rgbaq1 = GsRgbaq::make(255, 128, 64, 200);
    packet.resize(packet.size() + 16);
    std::memcpy(packet.data() + kGifTagSize + 48, &rgbaq1.value, 8);

    GsUv uv1 = GsUv::make(100.0f, 50.0f);
    packet.resize(packet.size() + 16);
    std::memcpy(packet.data() + kGifTagSize + 64, &uv1.value, 4);

    GsXyz2 xyz1 = GsXyz2::make(110.0f, 70.0f, 0.0f);
    packet.resize(packet.size() + 16);
    std::memcpy(packet.data() + kGifTagSize + 80, &xyz1.value, 8);

    bool ok = buf.parsePacket(packet.data(), static_cast<u32>(packet.size()));
    assert(ok);

    assert(buf.commandCount() == 1);
    const RenderCmd& cmd = buf.command(0);
    assert(cmd.type == RenderCommand::DrawSprite);
    assert(cmd.sprite.x > 9.0f && cmd.sprite.x < 11.0f);
    assert(cmd.sprite.y > 19.0f && cmd.sprite.y < 21.0f);
    assert(cmd.sprite.w > 99.0f && cmd.sprite.w < 101.0f);
    assert(cmd.sprite.h > 49.0f && cmd.sprite.h < 51.0f);
    (void)ok; (void)cmd;
}

static void test_command_buffer_state_tracking() {
    GifCommandBuffer buf;
    assert(buf.currentBlendMode() == GSBlendMode::None);
    assert(buf.currentDepthWrite());

    std::vector<u8> packet;

    GifTag tag = GifTag::makeSimple(1, true, GifFlg::Packed, 3);
    tag.setReg(0, GifReg::NOP);
    tag.setReg(1, GifReg::A_D);
    tag.setReg(2, GifReg::NOP);
    packet.resize(kGifTagSize);
    std::memcpy(packet.data(), tag.bytes, kGifTagSize);

    packet.resize(packet.size() + 16);
    std::memset(packet.data() + kGifTagSize, 0, 16);

    bool ok = buf.parsePacket(packet.data(), static_cast<u32>(packet.size()));
    assert(ok);
    (void)ok;
}

static void test_command_buffer_reset() {
    GifCommandBuffer buf;

    std::vector<u8> packet;

    u32 primVal = 6 | (1 << 5);
    GifTag tag{};
    tag.setNloop(6);
    tag.setEop(true);
    tag.setPre(true);
    tag.setPrim(primVal);
    tag.setFlg(GifFlg::Packed);
    tag.setNreg(3);
    tag.setReg(0, GifReg::RGBAQ);
    tag.setReg(1, GifReg::UV);
    tag.setReg(2, GifReg::XYZ2);
    packet.resize(kGifTagSize);
    std::memcpy(packet.data(), tag.bytes, kGifTagSize);

    GsRgbaq rgbaq0 = GsRgbaq::make(255, 128, 64, 200);
    packet.resize(packet.size() + 16);
    std::memcpy(packet.data() + kGifTagSize, &rgbaq0.value, 8);

    GsUv uv0 = GsUv::make(0.0f, 0.0f);
    packet.resize(packet.size() + 16);
    std::memcpy(packet.data() + kGifTagSize + 16, &uv0.value, 4);

    GsXyz2 xyz0 = GsXyz2::make(10.0f, 20.0f, 0.0f);
    packet.resize(packet.size() + 16);
    std::memcpy(packet.data() + kGifTagSize + 32, &xyz0.value, 8);

    GsRgbaq rgbaq1 = GsRgbaq::make(255, 128, 64, 200);
    packet.resize(packet.size() + 16);
    std::memcpy(packet.data() + kGifTagSize + 48, &rgbaq1.value, 8);

    GsUv uv1 = GsUv::make(100.0f, 50.0f);
    packet.resize(packet.size() + 16);
    std::memcpy(packet.data() + kGifTagSize + 64, &uv1.value, 4);

    GsXyz2 xyz1 = GsXyz2::make(110.0f, 70.0f, 0.0f);
    packet.resize(packet.size() + 16);
    std::memcpy(packet.data() + kGifTagSize + 80, &xyz1.value, 8);

    buf.parsePacket(packet.data(), static_cast<u32>(packet.size()));
    assert(buf.commandCount() > 0);

    buf.reset();
    assert(buf.commandCount() == 0);
}

static void test_command_buffer_screen_size() {
    GifCommandBuffer buf;
    buf.setScreenSize(1280, 720);
    buf.reset();
    assert(buf.commandCount() == 0);
}

int main() {
    test_gif_tag_nloop();
    test_gif_tag_eop();
    test_gif_tag_pre();
    test_gif_tag_flg();
    test_gif_tag_nreg();
    test_gif_tag_reg();
    test_gif_tag_make_simple();
    test_gif_tag_make_prim();
    test_gs_prim_reg();
    test_gs_rgbaq();
    test_gs_stq();
    test_gs_uv();
    test_gs_xyz2();
    test_gs_tex0();
    test_gs_tex1();
    test_gs_alpha();
    test_gs_test();
    test_gs_frame();
    test_gs_zbuf();
    test_gs_fog();
    test_command_buffer_empty();
    test_command_buffer_sprite_packet();
    test_command_buffer_state_tracking();
    test_command_buffer_reset();
    test_command_buffer_screen_size();

    std::printf("gif_command_test: all passed\n");
    return 0;
}
