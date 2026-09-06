#include "engine/RenderBackend.h"

#include <cmath>
#include <cstdio>

namespace ico::engine {

Matrix4x4 Matrix4x4::identity() {
    Matrix4x4 m{};
    m.m[0] = 1.0f; m.m[5] = 1.0f; m.m[10] = 1.0f; m.m[15] = 1.0f;
    return m;
}

Matrix4x4 Matrix4x4::perspective(float fovY, float aspect, float near, float far) {
    Matrix4x4 m{};
    float tanHalf = 1.0f / std::tan(fovY * 0.5f);
    m.m[0] = tanHalf / aspect;
    m.m[5] = tanHalf;
    m.m[10] = -(far + near) / (far - near);
    m.m[11] = -1.0f;
    m.m[14] = -(2.0f * far * near) / (far - near);
    return m;
}

Matrix4x4 Matrix4x4::ortho(float l, float r, float b, float t, float n, float f) {
    Matrix4x4 m{};
    m.m[0] = 2.0f / (r - l);
    m.m[5] = 2.0f / (t - b);
    m.m[10] = -2.0f / (f - n);
    m.m[12] = -(r + l) / (r - l);
    m.m[13] = -(t + b) / (t - b);
    m.m[14] = -(f + n) / (f - n);
    m.m[15] = 1.0f;
    return m;
}

Matrix4x4 Matrix4x4::lookAt(const float eye[3], const float target[3], const float up[3]) {
    float fx = target[0] - eye[0];
    float fy = target[1] - eye[1];
    float fz = target[2] - eye[2];
    float len = std::sqrt(fx * fx + fy * fy + fz * fz);
    if (len > 0.0f) { fx /= len; fy /= len; fz /= len; }

    float sx = fy * up[2] - fz * up[1];
    float sy = fz * up[0] - fx * up[2];
    float sz = fx * up[1] - fy * up[0];
    len = std::sqrt(sx * sx + sy * sy + sz * sz);
    if (len > 0.0f) { sx /= len; sy /= len; sz /= len; }

    float ux = sy * fz - sz * fy;
    float uy = sz * fx - sx * fz;
    float uz = sx * fy - sy * fx;

    Matrix4x4 m{};
    m.m[0] = sx;  m.m[4] = sy;  m.m[8]  = sz;
    m.m[1] = ux;  m.m[5] = uy;  m.m[9]  = uz;
    m.m[2] = -fx; m.m[6] = -fy; m.m[10] = -fz;
    m.m[12] = -(sx * eye[0] + sy * eye[1] + sz * eye[2]);
    m.m[13] = -(ux * eye[0] + uy * eye[1] + uz * eye[2]);
    m.m[14] =  (fx * eye[0] + fy * eye[1] + fz * eye[2]);
    m.m[15] = 1.0f;
    return m;
}

Matrix4x4 Matrix4x4::multiply(const Matrix4x4& a, const Matrix4x4& b) {
    Matrix4x4 r{};
    for (int c = 0; c < 4; ++c) {
        for (int row = 0; row < 4; ++row) {
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k) {
                sum += a.m[k * 4 + row] * b.m[c * 4 + k];
            }
            r.m[c * 4 + row] = sum;
        }
    }
    return r;
}

Matrix4x4 Matrix4x4::inverse(const Matrix4x4& m) {
    Matrix4x4 inv{};
    float* o = inv.m;
    const float* s = m.m;

    o[0]  =  s[5]*s[10]*s[15] - s[5]*s[11]*s[14] - s[9]*s[6]*s[15] + s[9]*s[7]*s[14] + s[13]*s[6]*s[11] - s[13]*s[7]*s[10];
    o[4]  = -s[4]*s[10]*s[15] + s[4]*s[11]*s[14] + s[8]*s[6]*s[15] - s[8]*s[7]*s[14] - s[12]*s[6]*s[11] + s[12]*s[7]*s[10];
    o[8]  =  s[4]*s[9]*s[15]  - s[4]*s[11]*s[13] - s[8]*s[5]*s[15] + s[8]*s[7]*s[13] + s[12]*s[5]*s[11] - s[12]*s[7]*s[9];
    o[12] = -s[4]*s[9]*s[14]  + s[4]*s[10]*s[13] + s[8]*s[5]*s[14] - s[8]*s[6]*s[13] - s[12]*s[5]*s[10] + s[12]*s[6]*s[9];
    o[1]  = -s[1]*s[10]*s[15] + s[1]*s[11]*s[14] + s[9]*s[2]*s[15] - s[9]*s[3]*s[14] - s[13]*s[2]*s[11] + s[13]*s[3]*s[10];
    o[5]  =  s[0]*s[10]*s[15] - s[0]*s[11]*s[14] - s[8]*s[2]*s[15] + s[8]*s[3]*s[14] + s[12]*s[2]*s[11] - s[12]*s[3]*s[10];
    o[9]  = -s[0]*s[9]*s[15]  + s[0]*s[11]*s[13] + s[8]*s[1]*s[15] - s[8]*s[3]*s[13] - s[12]*s[1]*s[11] + s[12]*s[3]*s[9];
    o[13] =  s[0]*s[9]*s[14]  - s[0]*s[10]*s[13] - s[8]*s[1]*s[14] + s[8]*s[2]*s[13] + s[12]*s[1]*s[10] - s[12]*s[2]*s[9];
    o[2]  =  s[1]*s[6]*s[15]  - s[1]*s[7]*s[14]  - s[5]*s[2]*s[15] + s[5]*s[3]*s[14] + s[13]*s[2]*s[7]  - s[13]*s[3]*s[6];
    o[6]  = -s[0]*s[6]*s[15]  + s[0]*s[7]*s[14]  + s[4]*s[2]*s[15] - s[4]*s[3]*s[14] - s[12]*s[2]*s[7]  + s[12]*s[3]*s[6];
    o[10] =  s[0]*s[5]*s[15]  - s[0]*s[7]*s[13]  - s[4]*s[1]*s[15] + s[4]*s[3]*s[13] + s[12]*s[1]*s[7]  - s[12]*s[3]*s[5];
    o[14] = -s[0]*s[5]*s[14]  + s[0]*s[6]*s[13]  + s[4]*s[1]*s[14] - s[4]*s[2]*s[13] - s[12]*s[1]*s[6]  + s[12]*s[2]*s[5];
    o[3]  = -s[1]*s[6]*s[11]  + s[1]*s[7]*s[10]  + s[5]*s[2]*s[11] - s[5]*s[3]*s[10] - s[9]*s[2]*s[7]   + s[9]*s[3]*s[6];
    o[7]  =  s[0]*s[6]*s[11]  - s[0]*s[7]*s[10]  - s[4]*s[2]*s[11] + s[4]*s[3]*s[10] + s[8]*s[2]*s[7]   - s[8]*s[3]*s[6];
    o[11] = -s[0]*s[5]*s[11]  + s[0]*s[7]*s[9]   + s[4]*s[1]*s[11] - s[4]*s[3]*s[9]  - s[8]*s[1]*s[7]   + s[8]*s[3]*s[5];
    o[15] =  s[0]*s[5]*s[10]  - s[0]*s[6]*s[9]   - s[4]*s[1]*s[10] + s[4]*s[2]*s[9]  + s[8]*s[1]*s[6]   - s[8]*s[2]*s[5];

    float det = s[0]*o[0] + s[1]*o[4] + s[2]*o[8] + s[3]*o[12];
    if (std::abs(det) < 1e-10f) {
        return identity();
    }
    float invDet = 1.0f / det;
    for (int i = 0; i < 16; ++i) {
        o[i] *= invDet;
    }
    return inv;
}

Matrix4x4 Matrix4x4::transpose(const Matrix4x4& m) {
    Matrix4x4 r{};
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            r.m[j * 4 + i] = m.m[i * 4 + j];
        }
    }
    return r;
}

Matrix4x4 Matrix4x4::rotationX(float angle) {
    Matrix4x4 m = identity();
    float cs = std::cos(angle);
    float sn = std::sin(angle);
    m.m[5] = cs; m.m[6] = sn; m.m[9] = -sn; m.m[10] = cs;
    return m;
}

Matrix4x4 Matrix4x4::rotationY(float angle) {
    Matrix4x4 m = identity();
    float cs = std::cos(angle);
    float sn = std::sin(angle);
    m.m[0] = cs; m.m[2] = -sn; m.m[8] = sn; m.m[10] = cs;
    return m;
}

Matrix4x4 Matrix4x4::rotationZ(float angle) {
    Matrix4x4 m = identity();
    float cs = std::cos(angle);
    float sn = std::sin(angle);
    m.m[0] = cs; m.m[1] = sn; m.m[4] = -sn; m.m[5] = cs;
    return m;
}

Matrix4x4 Matrix4x4::scale(float sx, float sy, float sz) {
    Matrix4x4 m{};
    m.m[0] = sx; m.m[5] = sy; m.m[10] = sz; m.m[15] = 1.0f;
    return m;
}

Matrix4x4 Matrix4x4::translation(float tx, float ty, float tz) {
    Matrix4x4 m = identity();
    m.m[12] = tx; m.m[13] = ty; m.m[14] = tz;
    return m;
}

// ---------------------------------------------------------------------------

class RenderStubBackend final : public RenderBackend {
public:
    RenderStubBackend() = default;
    ~RenderStubBackend() override = default;

    bool initialize(u32 width, u32 height) override {
        m_width = width;
        m_height = height;
        m_initialized = true;
        std::fprintf(stderr, "[render] RenderBackend (stub) initialized: %ux%u\n", width, height);
        return true;
    }

    void shutdown() override {
        m_initialized = false;
        std::fprintf(stderr, "[render] RenderBackend (stub) shutdown\n");
    }

    bool isInitialized() const override { return m_initialized; }

    void beginFrame() override {
        if (!m_initialized) return;
        m_drawCallCount = 0;
        m_triangleCount = 0;
    }

    void endFrame() override {
        if (!m_initialized) return;
    }

    void present() override {
        if (!m_initialized) return;
        std::fprintf(stderr, "[render] Frame: %u draw calls, %u triangles\n",
                      m_drawCallCount, m_triangleCount);
    }

    void clear(u8 r, u8 g, u8 b, u8 a) override { (void)r; (void)g; (void)b; (void)a; }
    void clearDepth() override {}
    void setViewport(u32 x, u32 y, u32 w, u32 h) override { (void)x; (void)y; (void)w; (void)h; }
    void setScissor(u32 x, u32 y, u32 w, u32 h) override { (void)x; (void)y; (void)w; (void)h; }
    void swapBuffers() override {}

    void setBlendMode(GSBlendMode mode) override { (void)mode; }
    void setDepthTest(GSDepthTest test, bool write) override { (void)test; (void)write; }
    void setAlphaTest(GSAlphaTest test, u8 ref, u8 mask) override { (void)test; (void)ref; (void)mask; }
    void setFramebuffer(u32 fbp, u32 fbw, u32 psm) override { (void)fbp; (void)fbw; (void)psm; }
    void setZBuffer(u32 zbp, u32 psm, bool zmsk) override { (void)zbp; (void)psm; (void)zmsk; }
    void setAlpha(u32 aba, u32 abb, u32 abc, u32 abd, u32 afix) override {
        (void)aba; (void)abb; (void)abc; (void)abd; (void)afix;
    }

    TextureHandle createTexture(const TextureDesc& desc) override {
        (void)desc;
        return ++m_nextTexture;
    }
    void destroyTexture(TextureHandle handle) override { (void)handle; }
    void bindTexture(TextureHandle handle, u32 slot) override { (void)handle; (void)slot; }

    RenderTargetHandle createRenderTarget(u32 width, u32 height) override {
        (void)width; (void)height;
        return ++m_nextRT;
    }
    void destroyRenderTarget(RenderTargetHandle handle) override { (void)handle; }
    void setRenderTarget(RenderTargetHandle handle) override { (void)handle; }

    void setMatrices(const Matrix4x4& proj, const Matrix4x4& view, const Matrix4x4& model) override {
        (void)proj; (void)view; (void)model;
    }

    void drawPrimitive(GSPrimitive primitive, RenderList list,
                       const RenderVertex* vertices, u32 count,
                       TextureHandle texture,
                       u8 r, u8 g, u8 b, u8 a) override {
        (void)primitive; (void)list; (void)vertices; (void)texture;
        (void)r; (void)g; (void)b; (void)a;
        ++m_drawCallCount;
        m_triangleCount += count / 3;
    }

    void drawIndexed(GSPrimitive primitive, RenderList list,
                     const u32* indices, u32 indexCount,
                     const RenderVertex* vertices, u32 vertexOffset,
                     TextureHandle texture,
                     u8 r, u8 g, u8 b, u8 a) override {
        (void)primitive; (void)list; (void)indices; (void)vertices;
        (void)vertexOffset; (void)texture; (void)r; (void)g; (void)b; (void)a;
        ++m_drawCallCount;
        m_triangleCount += indexCount / 3;
    }

    void drawSprite(float x, float y, float w, float h,
                    float u0, float v0, float u1, float v1,
                    TextureHandle texture,
                    u8 r, u8 g, u8 b, u8 a) override {
        (void)x; (void)y; (void)w; (void)h;
        (void)u0; (void)v0; (void)u1; (void)v1;
        (void)texture; (void)r; (void)g; (void)b; (void)a;
        ++m_drawCallCount;
        m_triangleCount += 2;
    }

void drawSpriteGouraud(float x, float y, float w, float h,
                           float u0, float v0, float u1, float v1,
                           TextureHandle texture,
                           const u8 cornerColors[4][4]) override {
        (void)x; (void)y; (void)w; (void)h;
        (void)u0; (void)v0; (void)u1; (void)v1;
        (void)texture; (void)cornerColors;
    }

    void copyTexture(float srcX, float srcY, float dstX, float dstY,
                     float w, float h) override {
        /* Rev.134: the stub backend has no VRAM buffers to move; the command
           boundary is modeled, the copy itself is a no-op until a backend
           binds source/dest VRAM buffers. */
        (void)srcX; (void)srcY; (void)dstX; (void)dstY; (void)w; (void)h;
    }

    void beginPass(RenderList list) override { (void)list; }
    void endPass() override {}

    u32 getWidth() const override { return m_width; }
    u32 getHeight() const override { return m_height; }

private:
    bool m_initialized = false;
    u32 m_width = 0;
    u32 m_height = 0;
    u32 m_drawCallCount = 0;
    u32 m_triangleCount = 0;
    TextureHandle m_nextTexture = kNullTexture;
    RenderTargetHandle m_nextRT = kNullRenderTarget;
};

std::unique_ptr<RenderBackend> createRenderBackend() {
    return std::make_unique<RenderStubBackend>();
}

} // namespace ico::engine
