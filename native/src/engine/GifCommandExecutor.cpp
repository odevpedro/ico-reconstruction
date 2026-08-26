#include "engine/GifCommandExecutor.h"

namespace ico::engine {

GifCommandExecutor::GifCommandExecutor(RenderBackend& backend)
    : m_backend(backend) {
}

void GifCommandExecutor::execute(const GifCommandBuffer& buffer) {
    for (u32 i = 0; i < buffer.commandCount(); ++i) {
        executeCommand(buffer.command(i));
    }
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
        case RenderCommand::BeginPass:
            m_backend.beginPass(cmd.pass.list);
            break;
        case RenderCommand::EndPass:
            m_backend.endPass();
            break;
    }
}

} // namespace ico::engine
