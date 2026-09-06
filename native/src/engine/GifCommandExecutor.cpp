#include "engine/GifCommandExecutor.h"

#include <vector>

namespace ico::engine {

GifCommandExecutor::GifCommandExecutor(RenderBackend& backend)
    : m_backend(backend) {
}

void GifCommandExecutor::execute(const GifCommandBuffer& buffer) {
    for (u32 i = 0; i < buffer.commandCount(); ++i) {
        RenderCmd cmd = buffer.command(i);
        resolveTextureHandles(cmd, buffer);
        executeCommand(cmd);
    }
}

void GifCommandExecutor::resolveTextureHandles(RenderCmd& cmd, const GifCommandBuffer& buffer) {
    TextureHandle* texPtr = nullptr;
    switch (cmd.type) {
        case RenderCommand::DrawPrimitive:      texPtr = &cmd.draw.texture; break;
        case RenderCommand::DrawIndexed:        texPtr = &cmd.drawIndexed.texture; break;
        case RenderCommand::DrawSprite:         texPtr = &cmd.sprite.texture; break;
        case RenderCommand::DrawSpriteGouraud:  texPtr = &cmd.spriteGouraud.texture; break;
        default: return;
    }
    if (*texPtr == kNullTexture) {
        return;
    }

    auto it = m_virtualToReal.find(*texPtr);
    if (it != m_virtualToReal.end()) {
        *texPtr = it->second;
        return;
    }

    TextureFormat format;
    u32 w = 0;
    u32 h = 0;
    std::vector<u8> rgba;
    if (!buffer.uploadedTexture(*texPtr, w, h, format, rgba)) {
        return;
    }

    TextureDesc desc{};
    desc.width = w;
    desc.height = h;
    desc.format = format;
    desc.data = rgba.data();
    desc.dataSize = static_cast<u32>(rgba.size());
    desc.generateMipmaps = false;

    TextureHandle real = m_backend.createTexture(desc);
    m_backend.bindTexture(real, 0);
    m_virtualToReal[*texPtr] = real;
    *texPtr = real;
}

void GifCommandExecutor::executeCommand(const RenderCmd& cmd) {
    switch (cmd.type) {
        case RenderCommand::Clear:
            m_backend.clear(cmd.viewport.x, cmd.viewport.y, cmd.viewport.w);
            break;
        case RenderCommand::SetViewport:
            m_backend.setViewport(cmd.viewport.x, cmd.viewport.y, cmd.viewport.w, cmd.viewport.h);
            break;
        case RenderCommand::SetScissor:
            m_backend.setScissor(cmd.scissor.x, cmd.scissor.y, cmd.scissor.w, cmd.scissor.h);
            break;
        case RenderCommand::SetBlendMode:
            m_backend.setBlendMode(cmd.blendMode.mode);
            break;
        case RenderCommand::SetDepthTest:
            m_backend.setDepthTest(cmd.depthTest.test, cmd.depthTest.write);
            break;
        case RenderCommand::SetAlphaTest:
            m_backend.setAlphaTest(cmd.alphaTest.test, cmd.alphaTest.ref, cmd.alphaTest.mask);
            break;
        case RenderCommand::SetFramebuffer:
            m_backend.setFramebuffer(cmd.framebuffer.fbp, cmd.framebuffer.fbw, cmd.framebuffer.psm);
            break;
        case RenderCommand::SetZBuffer:
            m_backend.setZBuffer(cmd.zbuffer.zbp, cmd.zbuffer.psm, cmd.zbuffer.zmsk);
            break;
        case RenderCommand::SetAlpha:
            m_backend.setAlpha(cmd.alpha.aba, cmd.alpha.abb, cmd.alpha.abc,
                               cmd.alpha.abd, cmd.alpha.afix);
            break;
        case RenderCommand::SetTexture:
            m_backend.bindTexture(cmd.texture.tex, cmd.texture.slot);
            break;
        case RenderCommand::SetMatrices:
            m_backend.setMatrices(cmd.matrices.projection, cmd.matrices.view, cmd.matrices.model);
            break;
        case RenderCommand::DrawPrimitive:
            m_backend.drawPrimitive(cmd.draw.primitive, cmd.draw.list,
                                    nullptr, cmd.draw.vertexCount,
                                    cmd.draw.texture,
                                    cmd.draw.r, cmd.draw.g, cmd.draw.b, cmd.draw.a);
            break;
        case RenderCommand::DrawIndexed:
            m_backend.drawIndexed(cmd.drawIndexed.primitive, cmd.drawIndexed.list,
                                  nullptr, cmd.drawIndexed.indexCount,
                                  nullptr, cmd.drawIndexed.vertexOffset,
                                  cmd.drawIndexed.texture,
                                  cmd.drawIndexed.r, cmd.drawIndexed.g,
                                  cmd.drawIndexed.b, cmd.drawIndexed.a);
            break;
        case RenderCommand::DrawSprite:
            m_backend.drawSprite(cmd.sprite.x, cmd.sprite.y, cmd.sprite.w, cmd.sprite.h,
                                 cmd.sprite.u0, cmd.sprite.v0, cmd.sprite.u1, cmd.sprite.v1,
                                 cmd.sprite.texture,
                                 cmd.sprite.r, cmd.sprite.g, cmd.sprite.b, cmd.sprite.a);
            break;
        case RenderCommand::DrawSpriteGouraud:
            m_backend.drawSpriteGouraud(cmd.spriteGouraud.x, cmd.spriteGouraud.y,
                                        cmd.spriteGouraud.w, cmd.spriteGouraud.h,
                                        cmd.spriteGouraud.u0, cmd.spriteGouraud.v0,
                                        cmd.spriteGouraud.u1, cmd.spriteGouraud.v1,
                                        cmd.spriteGouraud.texture,
                                        cmd.spriteGouraud.corners);
            break;
        case RenderCommand::DrawLine: {
            RenderVertex vertices[2]{};
            vertices[0].x = cmd.line.x0;
            vertices[0].y = cmd.line.y0;
            vertices[0].r = cmd.line.r;
            vertices[0].g = cmd.line.g;
            vertices[0].b = cmd.line.b;
            vertices[0].a = cmd.line.a;
            vertices[1].x = cmd.line.x1;
            vertices[1].y = cmd.line.y1;
            vertices[1].r = cmd.line.r;
            vertices[1].g = cmd.line.g;
            vertices[1].b = cmd.line.b;
            vertices[1].a = cmd.line.a;
            m_backend.drawPrimitive(GSPrimitive::Line, RenderList::Opaque,
                                    vertices, 2, kNullTexture,
                                    cmd.line.r, cmd.line.g, cmd.line.b, cmd.line.a);
            break;
        }
        case RenderCommand::DrawPoint: {
            RenderVertex vertex{};
            vertex.x = cmd.point.x;
            vertex.y = cmd.point.y;
            vertex.r = cmd.point.r;
            vertex.g = cmd.point.g;
            vertex.b = cmd.point.b;
            vertex.a = cmd.point.a;
            m_backend.drawPrimitive(GSPrimitive::Point, RenderList::Opaque,
                                    &vertex, 1, kNullTexture,
                                    cmd.point.r, cmd.point.g, cmd.point.b, cmd.point.a);
            break;
        }
        case RenderCommand::DrawTriangle: {
            RenderVertex vertices[3]{};
            vertices[0].x = cmd.triangle.x0;
            vertices[0].y = cmd.triangle.y0;
            vertices[0].u = cmd.triangle.u0;
            vertices[0].v = cmd.triangle.v0;
            vertices[0].r = cmd.triangle.r;
            vertices[0].g = cmd.triangle.g;
            vertices[0].b = cmd.triangle.b;
            vertices[0].a = cmd.triangle.a;
            vertices[1].x = cmd.triangle.x1;
            vertices[1].y = cmd.triangle.y1;
            vertices[1].u = cmd.triangle.u1;
            vertices[1].v = cmd.triangle.v1;
            vertices[1].r = cmd.triangle.r;
            vertices[1].g = cmd.triangle.g;
            vertices[1].b = cmd.triangle.b;
            vertices[1].a = cmd.triangle.a;
            vertices[2].x = cmd.triangle.x2;
            vertices[2].y = cmd.triangle.y2;
            vertices[2].u = cmd.triangle.u2;
            vertices[2].v = cmd.triangle.v2;
            vertices[2].r = cmd.triangle.r;
            vertices[2].g = cmd.triangle.g;
            vertices[2].b = cmd.triangle.b;
            vertices[2].a = cmd.triangle.a;
            m_backend.drawPrimitive(GSPrimitive::Triangle, RenderList::Opaque,
                                    vertices, 3, kNullTexture,
                                    cmd.triangle.r, cmd.triangle.g, cmd.triangle.b, cmd.triangle.a);
            break;
        }
        case RenderCommand::CopyTexture:
            m_backend.copyTexture(cmd.copy.srcX, cmd.copy.srcY,
                                  cmd.copy.dstX, cmd.copy.dstY,
                                  cmd.copy.w, cmd.copy.h);
            break;
        case RenderCommand::BeginPass:
            m_backend.beginPass(cmd.pass.list);
            break;
        case RenderCommand::EndPass:
            m_backend.endPass();
            break;
    }
}

} // namespace ico::engine
