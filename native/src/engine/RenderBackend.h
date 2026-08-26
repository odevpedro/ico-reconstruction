#pragma once

#include "ps2/Ps2Types.h"

#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

namespace ico::engine {

// PS2 resolution constants
constexpr u32 kPs2ScreenWidth = 640;
constexpr u32 kPs2ScreenHeight = 448;
constexpr u32 kPs2FramebufferCount = 2;

// Number of primary rendering lists (matches iosOmCreateDL)
constexpr u32 kRenderListCount = 8;

// Texture formats matching PS2 GS
enum class TextureFormat : u8 {
    PSMCT32 = 0,   // 32-bit RGBA
    PSMCT24 = 1,   // 24-bit RGB
    PSMCT16 = 2,   // 16-bit RGBA
    PSMCT16S = 10, // 16-bit RGBA (swizzled)
    PSMT8 = 19,    // 8-bit indexed
    PSMT4 = 20,    // 4-bit indexed
    PSMT8H = 24,   // 8-bit indexed (high byte)
    PSMT4HL = 26,  // 4-bit indexed (high nibble)
    PSMT4HH = 27,  // 4-bit indexed (high nibble)
    PSMZ32 = 48,   // 32-bit depth
    PSMZ24 = 49,   // 24-bit depth
    PSMZ16 = 50,   // 16-bit depth
    PSMZ16S = 58,  // 16-bit depth (swizzled)
};

// PS2 GS blend modes
enum class GSBlendMode : u8 {
    None,
    Alpha,        // Cs * As + Cd * (1 - As)
    Additive,     // Cs * As + Cd * 1
    Subtractive,  // Cd * (1 - Cs) - Cs * As
};

// PS2 GS alpha test modes
enum class GSAlphaTest : u8 {
    Never,
    Always,
    Less,
    LessEqual,
    Equal,
    Greater,
    GreaterEqual,
    NotEqual,
};

// PS2 GS depth test modes
enum class GSDepthTest : u8 {
    Never,
    Always,
    Less,
    LessEqual,
    Equal,
    Greater,
    GreaterEqual,
    NotEqual,
};

// PS2 GS primitive types
enum class GSPrimitive : u8 {
    Point,
    Line,
    Triangle,
    Sprite,
};

// Display list priorities (matching the 8 primary lists)
enum class RenderList : u8 {
    Background = 0,   // list 0: background sprites
    Opaque = 1,       // list 1: opaque geometry
    Transparent = 2,  // list 2: transparent geometry
    Shadow = 3,       // list 3: shadow rendering
    Particle = 4,     // list 4: particle effects
    UI = 5,           // list 5: user interface
    Debug = 6,        // list 6: debug rendering
    PostFX = 7,       // list 7: post-processing
};

// Matrix 4x4 (column-major, matches PS2 VU0 format)
struct alignas(16) Matrix4x4 {
    float m[16];

    static Matrix4x4 identity();
    static Matrix4x4 perspective(float fovY, float aspect, float near, float far);
    static Matrix4x4 ortho(float left, float right, float bottom, float top, float near, float far);
    static Matrix4x4 lookAt(const float eye[3], const float target[3], const float up[3]);
    static Matrix4x4 multiply(const Matrix4x4& a, const Matrix4x4& b);
    static Matrix4x4 inverse(const Matrix4x4& mat);
    static Matrix4x4 transpose(const Matrix4x4& mat);
    static Matrix4x4 rotationX(float angle);
    static Matrix4x4 rotationY(float angle);
    static Matrix4x4 rotationZ(float angle);
    static Matrix4x4 scale(float sx, float sy, float sz);
    static Matrix4x4 translation(float tx, float ty, float tz);
};

// Vertex format matching PS2 VIF1 vertex data
struct RenderVertex {
    float x, y, z;       // position
    float nx, ny, nz;    // normal
    float u, v;           // texture coordinates
    u8 r, g, b, a;       // color
};

// Texture handle (opaque ID)
using TextureHandle = u32;
constexpr TextureHandle kNullTexture = 0;

// Render target handle
using RenderTargetHandle = u32;
constexpr RenderTargetHandle kNullRenderTarget = 0;

// Render command types
enum class RenderCommand : u8 {
    Clear,
    SetViewport,
    SetScissor,
    SetBlendMode,
    SetDepthTest,
    SetAlphaTest,
    SetTexture,
    SetMatrices,
    DrawPrimitive,
    DrawIndexed,
    DrawSprite,
    DrawSpriteGouraud,
    DrawLine,
    DrawPoint,
    DrawTriangle,
    CopyTexture,
    BeginPass,
    EndPass,
};

// A single render command in the command buffer
struct RenderCmd {
    RenderCommand type;
    union {
        struct { u32 x, y, w, h; } viewport;
        struct { u32 x, y, w, h; } scissor;
        struct { GSBlendMode mode; } blendMode;
        struct { GSDepthTest test; bool write; } depthTest;
        struct { GSAlphaTest test; u8 ref; u8 mask; } alphaTest;
        struct { TextureHandle tex; u32 slot; } texture;
        struct { Matrix4x4 projection; Matrix4x4 view; Matrix4x4 model; } matrices;
        struct {
            GSPrimitive primitive;
            RenderList list;
            u32 vertexOffset;
            u32 vertexCount;
            TextureHandle texture;
            u8 r, g, b, a;
        } draw;
        struct {
            GSPrimitive primitive;
            RenderList list;
            u32 indexOffset;
            u32 indexCount;
            u32 vertexOffset;
            TextureHandle texture;
            u8 r, g, b, a;
        } drawIndexed;
        struct {
            float x, y, w, h;
            float u0, v0, u1, v1;
            TextureHandle texture;
            u8 r, g, b, a;
        } sprite;
        struct {
            float x, y, w, h;
            float u0, v0, u1, v1;
            TextureHandle texture;
            u8 corners[4][4]; // per-corner RGBA (gouraud)
        } spriteGouraud;
        struct {
            float x0, y0, x1, y1;
            u8 r, g, b, a;
            u8 gouraud;
        } line;
        struct {
            float x, y;
            u8 r, g, b, a;
        } point;
        struct {
            float x0, y0, x1, y1, x2, y2;
            float u0, v0, u1, v1, u2, v2;
            u8 r, g, b, a;
        } triangle;
        struct { RenderTargetHandle target; RenderList list; } pass;
    };
};

// Texture upload descriptor
struct TextureDesc {
    u32 width;
    u32 height;
    TextureFormat format;
    const void* data;
    u32 dataSize;
    bool generateMipmaps;
};

// Abstract rendering backend
//
// This interface maps the PS2 GIF/GS rendering pipeline to modern GPU APIs.
// The PS2 builds GIF packets (command buffers) that are DMA'd to VU1 for
// vertex processing, then to the GS for rasterization. On PC, we accumulate
// render commands and execute them against OpenGL/Vulkan/D3D.
//
// Lifecycle:
//   initialize() -> [beginFrame/draw/endFrame]* -> shutdown()
//
class RenderBackend {
public:
    virtual ~RenderBackend() = default;

    virtual bool initialize(u32 width = kPs2ScreenWidth,
                            u32 height = kPs2ScreenHeight) = 0;
    virtual void shutdown() = 0;
    virtual bool isInitialized() const = 0;

    // Frame lifecycle
    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;
    virtual void present() = 0;

    // Framebuffer
    virtual void clear(u8 r, u8 g, u8 b, u8 a = 255) = 0;
    virtual void clearDepth() = 0;
    virtual void setViewport(u32 x, u32 y, u32 w, u32 h) = 0;
    virtual void setScissor(u32 x, u32 y, u32 w, u32 h) = 0;

    // Double buffering (matching dl_Swap / sceGsSwapDBuff)
    virtual void swapBuffers() = 0;

    // State management
    virtual void setBlendMode(GSBlendMode mode) = 0;
    virtual void setDepthTest(GSDepthTest test, bool write = true) = 0;
    virtual void setAlphaTest(GSAlphaTest test, u8 ref = 0, u8 mask = 0xFF) = 0;

    // Textures (matching tex_* functions)
    virtual TextureHandle createTexture(const TextureDesc& desc) = 0;
    virtual void destroyTexture(TextureHandle handle) = 0;
    virtual void bindTexture(TextureHandle handle, u32 slot = 0) = 0;

    // Render targets (matching gs_SetDisplay/gs_SetDrawEnv)
    virtual RenderTargetHandle createRenderTarget(u32 width, u32 height) = 0;
    virtual void destroyRenderTarget(RenderTargetHandle handle) = 0;
    virtual void setRenderTarget(RenderTargetHandle handle) = 0;

    // Transform (matching mtx_* / sceVu0* functions)
    virtual void setMatrices(const Matrix4x4& projection,
                             const Matrix4x4& view,
                             const Matrix4x4& model) = 0;

    // Drawing (matching gif_* / prim_* functions)
    virtual void drawPrimitive(GSPrimitive primitive, RenderList list,
                               const RenderVertex* vertices, u32 count,
                               TextureHandle texture = kNullTexture,
                               u8 r = 255, u8 g = 255, u8 b = 255, u8 a = 255) = 0;

    virtual void drawIndexed(GSPrimitive primitive, RenderList list,
                             const u32* indices, u32 indexCount,
                             const RenderVertex* vertices, u32 vertexOffset,
                             TextureHandle texture = kNullTexture,
                             u8 r = 255, u8 g = 255, u8 b = 255, u8 a = 255) = 0;

    // Sprite drawing (matching gif_DrawSprite / gif_DrawGouraudSprite)
    virtual void drawSprite(float x, float y, float w, float h,
                            float u0, float v0, float u1, float v1,
                            TextureHandle texture,
                            u8 r = 255, u8 g = 255, u8 b = 255, u8 a = 255) = 0;

    virtual void drawSpriteGouraud(float x, float y, float w, float h,
                                   float u0, float v0, float u1, float v1,
                                   TextureHandle texture,
                                   const u8 cornerColors[4][4]) = 0;

    // Render pass management (matching display list dispatch)
    virtual void beginPass(RenderList list) = 0;
    virtual void endPass() = 0;

    // Info
    virtual u32 getWidth() const = 0;
    virtual u32 getHeight() const = 0;
};

// Create a platform-specific render backend
std::unique_ptr<RenderBackend> createRenderBackend();

} // namespace ico::engine
