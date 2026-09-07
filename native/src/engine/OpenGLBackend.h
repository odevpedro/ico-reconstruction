#pragma once

#include "engine/RenderBackend.h"

namespace ico::engine {

struct BatchVertex {
    float x, y, z;
    float nx, ny, nz;
    float u, v;
    u8 r, g, b, a;
};

class OpenGLBackend;

class OpenGLBackendTestHelper {
public:
    static void setTestMode(OpenGLBackend& backend);
};

class OpenGLBackend final : public RenderBackend {
public:
    OpenGLBackend();
    ~OpenGLBackend() override;

    bool initialize(u32 width, u32 height) override;
    void shutdown() override;
    bool isInitialized() const override;

    void beginFrame() override;
    void endFrame() override;
    void present() override;

    void clear(u8 r, u8 g, u8 b, u8 a) override;
    void clearDepth() override;
    void setViewport(u32 x, u32 y, u32 w, u32 h) override;
    void setScissor(u32 x, u32 y, u32 w, u32 h) override;

    void swapBuffers() override;

    void setBlendMode(GSBlendMode mode) override;
    void setDepthTest(GSDepthTest test, bool write) override;
    void setAlphaTest(GSAlphaTest test, u8 ref, u8 mask) override;
    void setFramebuffer(u32 fbp, u32 fbw, u32 psm) override;
    void setZBuffer(u32 zbp, u32 psm, bool zmsk) override;
    void setAlpha(u32 aba, u32 abb, u32 abc, u32 abd, u32 afix) override;

    TextureHandle createTexture(const TextureDesc& desc) override;
    void destroyTexture(TextureHandle handle) override;
    void bindTexture(TextureHandle handle, u32 slot) override;

    RenderTargetHandle createRenderTarget(u32 width, u32 height) override;
    void destroyRenderTarget(RenderTargetHandle handle) override;
    void setRenderTarget(RenderTargetHandle handle) override;

    void setMatrices(const Matrix4x4& projection,
                     const Matrix4x4& view,
                     const Matrix4x4& model) override;

    void drawPrimitive(GSPrimitive primitive, RenderList list,
                       const RenderVertex* vertices, u32 count,
                       TextureHandle texture,
                       u8 r, u8 g, u8 b, u8 a) override;

    void drawIndexed(GSPrimitive primitive, RenderList list,
                     const u32* indices, u32 indexCount,
                     const RenderVertex* vertices, u32 vertexOffset,
                     TextureHandle texture,
                     u8 r, u8 g, u8 b, u8 a) override;

    void drawSprite(float x, float y, float w, float h,
                    float u0, float v0, float u1, float v1,
                    TextureHandle texture,
                    u8 r, u8 g, u8 b, u8 a) override;

    void drawSpriteGouraud(float x, float y, float w, float h,
                           float u0, float v0, float u1, float v1,
                           TextureHandle texture,
                           const u8 cornerColors[4][4]) override;

    void copyTexture(float srcX, float srcY, float dstX, float dstY,
                     float w, float h) override;

    // Reads back the current backbuffer (RGBA8, bottom-up) into a
    // width*height*4 buffer. Returns false if the GL context is unavailable.
    bool captureFrameRGB(u8* outRgb, u32 w, u32 h) const;

    void beginPass(RenderList list) override;
    void endPass() override;

    u32 getWidth() const override;
    u32 getHeight() const override;

    class Impl;

private:
    friend class OpenGLBackendTestHelper;
    void setTestMode();
    void flushBatch();
    bool hasGL() const;

    std::unique_ptr<Impl> m_impl;
};

} // namespace ico::engine
