#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <vector>
#include <memory>
#include <string>

namespace platform {

// Forward declarations
class Texture;
class Shader;
class Mesh;
class RenderTarget;

// Math types
struct Vec2 { float x, y; };
struct Vec3 { float x, y, z; };
struct Vec4 { float x, y, z, w; };
struct Mat4 { float m[16]; };

struct Color {
    uint8_t r, g, b, a;
    
    Color() : r(255), g(255), b(255), a(255) {}
    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : r(r), g(g), b(b), a(a) {}
};

// Vertex format
struct Vertex {
    Vec3 position;
    Vec3 normal;
    Vec2 texcoord;
    Color color;
};

// Texture formats
enum class TextureFormat : uint8_t {
    RGBA8,
    RGB8,
    RGBA4,
    RGB565,
    Index8,
    Index4,
    DXT1,
    DXT3,
    DXT5
};

// Primitive types
enum class PrimitiveType : uint8_t {
    Points,
    Lines,
    LineStrip,
    Triangles,
    TriangleStrip,
    TriangleFan
};

// Blend modes
enum class BlendMode : uint8_t {
    None,
    Alpha,
    Additive,
    Multiplicative,
    Screen
};

// Cull modes
enum class CullMode : uint8_t {
    None,
    Front,
    Back,
    FrontAndBack
};

// Depth modes
enum class DepthMode : uint8_t {
    None,
    Less,
    LessEqual,
    Equal,
    Greater,
    GreaterEqual
};

class Texture {
public:
    virtual ~Texture() = default;
    
    virtual uint32_t getWidth() const = 0;
    virtual uint32_t getHeight() const = 0;
    virtual TextureFormat getFormat() const = 0;
    
    virtual void bind(int slot = 0) = 0;
    virtual void unbind() = 0;
    
    virtual void setData(const void* data, size_t size) = 0;
};

class Shader {
public:
    virtual ~Shader() = default;
    
    virtual void bind() = 0;
    virtual void unbind() = 0;
    
    virtual void setBool(const std::string& name, bool value) = 0;
    virtual void setInt(const std::string& name, int value) = 0;
    virtual void setFloat(const std::string& name, float value) = 0;
    virtual void setVec2(const std::string& name, const Vec2& value) = 0;
    virtual void setVec3(const std::string& name, const Vec3& value) = 0;
    virtual void setVec4(const std::string& name, const Vec4& value) = 0;
    virtual void setMat4(const std::string& name, const Mat4& value) = 0;
};

class Mesh {
public:
    virtual ~Mesh() = default;
    
    virtual void draw() = 0;
    virtual void drawInstanced(int count) = 0;
    
    virtual uint32_t getVertexCount() const = 0;
    virtual uint32_t getIndexCount() const = 0;
};

class Renderer {
public:
    virtual ~Renderer() = default;
    
    virtual void initialize(int width, int height, bool fullscreen) = 0;
    virtual void shutdown() = 0;
    
    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;
    
    virtual void clear(Color color, float depth = 1.0f) = 0;
    virtual void setViewport(int x, int y, int width, int height) = 0;
    
    // State
    virtual void setBlendMode(BlendMode mode) = 0;
    virtual void setCullMode(CullMode mode) = 0;
    virtual void setDepthMode(DepthMode mode) = 0;
    virtual void setDepthWrite(bool enable) = 0;
    
    // Resources
    virtual std::unique_ptr<Texture> createTexture(uint32_t width, uint32_t height, TextureFormat format) = 0;
    virtual std::unique_ptr<Texture> loadTexture(const std::string& path) = 0;
    virtual std::unique_ptr<Shader> createShader(const std::string& vertexSrc, const std::string& fragmentSrc) = 0;
    virtual std::unique_ptr<Shader> loadShader(const std::string& path) = 0;
    virtual std::unique_ptr<Mesh> createMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) = 0;
    virtual std::unique_ptr<RenderTarget> createRenderTarget(int width, int height) = 0;
    
    // Transform
    virtual void setProjection(const Mat4& proj) = 0;
    virtual void setView(const Mat4& view) = 0;
    virtual void setModel(const Mat4& model) = 0;
    
    // Screen
    virtual int getWidth() const = 0;
    virtual int getHeight() const = 0;
    virtual void resize(int width, int height) = 0;
    virtual void setFullscreen(bool fullscreen) = 0;
    
    // Screenshot
    virtual bool saveScreenshot(const std::string& path) = 0;
};

std::unique_ptr<Renderer> createRenderer();

} // namespace platform
