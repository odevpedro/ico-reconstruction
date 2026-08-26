#include "engine/GifPacket.h"

#include <cassert>
#include <cstdio>

using namespace ico::engine;

static void test_packet_lifecycle() {
    GifPacketBridge bridge;
    bridge.init(640, 448);

    assert(!bridge.checkOpen());

    bridge.startPacketPri(0);
    assert(bridge.checkOpen());

    bridge.endPacket();
    assert(!bridge.checkOpen());
}

static void test_closed_packet_is_noop() {
    GifPacketBridge bridge;
    bridge.init(640, 448);

    bridge.sprite(10.0f, 20.0f, 30.0f, 40.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1, 2, 3, 4);
    bridge.setAlpha(0, 0, 1, 0);
    bridge.moveImage(0, 0, 0, 0, 16, 16);

    assert(bridge.commandBuffer().commandCount() == 0);
}

static void test_state_mapping() {
    GifPacketBridge bridge;
    bridge.init(640, 448);
    bridge.startPacketPri(0);

    GsTex0 tex0{};
    tex0.setTbp0(123);
    bridge.setGsReg(kGsAddrTEX0_1, tex0.value);
    bridge.setAlpha(0, 0, 1, 0);
    bridge.setZWrite(1, 2);
    bridge.setDrawEnvironment(0.0f, 0.0f, 320.0f, 240.0f, 0, 0, 0);

    assert(bridge.commandBuffer().commandCount() == 4);

    const RenderCmd& texCmd = bridge.commandBuffer().command(0);
    assert(texCmd.type == RenderCommand::SetTexture);
    assert(texCmd.texture.tex == 123);
    assert(texCmd.texture.slot == 0);

    const RenderCmd& blendCmd = bridge.commandBuffer().command(1);
    assert(blendCmd.type == RenderCommand::SetBlendMode);
    assert(blendCmd.blendMode.mode == GSBlendMode::Alpha);

    const RenderCmd& depthCmd = bridge.commandBuffer().command(2);
    assert(depthCmd.type == RenderCommand::SetDepthTest);
    assert(depthCmd.depthTest.test == GSDepthTest::Less);
    assert(depthCmd.depthTest.write);

    const RenderCmd& viewportCmd = bridge.commandBuffer().command(3);
    assert(viewportCmd.type == RenderCommand::SetViewport);
    assert(viewportCmd.viewport.w == 320);
    assert(viewportCmd.viewport.h == 240);
}

static void test_geometry_mapping() {
    GifPacketBridge bridge;
    bridge.init(640, 448);
    bridge.startPacketPri(0);

    bridge.sprite(10.0f, 20.0f, 30.0f, 40.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5, 6, 7, 8);
    bridge.line(1.0f, 2.0f, 3.0f, 4.0f, 9, 10, 11, 12);
    bridge.point(5.0f, 6.0f, 13, 14, 15, 16);
    bridge.drawPolyF4(0.0f, 0.0f, 8.0f, 0.0f, 8.0f, 8.0f, 0.0f, 8.0f,
                      0.0f, 0.0f, 1.0f, 1.0f, 20, 21, 22, 23);
    bridge.moveImage(0.0f, 0.0f, 32.0f, 32.0f, 64.0f, 64.0f);

    assert(bridge.commandBuffer().commandCount() == 6);

    const RenderCmd& spriteCmd = bridge.commandBuffer().command(0);
    assert(spriteCmd.type == RenderCommand::DrawSprite);
    assert(spriteCmd.sprite.x == 10.0f);
    assert(spriteCmd.sprite.y == 20.0f);
    assert(spriteCmd.sprite.w == 30.0f);
    assert(spriteCmd.sprite.h == 40.0f);
    assert(spriteCmd.sprite.r == 5);
    assert(spriteCmd.sprite.a == 8);

    const RenderCmd& lineCmd = bridge.commandBuffer().command(1);
    assert(lineCmd.type == RenderCommand::DrawLine);
    assert(lineCmd.line.x0 == 1.0f);
    assert(lineCmd.line.y1 == 4.0f);
    assert(lineCmd.line.r == 9);

    const RenderCmd& pointCmd = bridge.commandBuffer().command(2);
    assert(pointCmd.type == RenderCommand::DrawPoint);
    assert(pointCmd.point.x == 5.0f);
    assert(pointCmd.point.y == 6.0f);
    assert(pointCmd.point.g == 14);

    const RenderCmd& tri0 = bridge.commandBuffer().command(3);
    const RenderCmd& tri1 = bridge.commandBuffer().command(4);
    assert(tri0.type == RenderCommand::DrawTriangle);
    assert(tri1.type == RenderCommand::DrawTriangle);
    assert(tri0.triangle.r == 20);
    assert(tri1.triangle.a == 23);

    const RenderCmd& copyCmd = bridge.commandBuffer().command(5);
    assert(copyCmd.type == RenderCommand::CopyTexture);
}

static void test_screen_check() {
    GifPacketBridge bridge;
    bridge.init(640, 448);

    assert(bridge.isInScreen(0.0f, 0.0f, 10.0f, 10.0f));
    assert(!bridge.isInScreen(-100.0f, -100.0f, 10.0f, 10.0f));
}

int main() {
    test_packet_lifecycle();
    test_closed_packet_is_noop();
    test_state_mapping();
    test_geometry_mapping();
    test_screen_check();

    std::printf("gif_packet_test: all passed\n");
    return 0;
}
