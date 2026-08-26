#include "engine/OpenGLBackend.h"

#include <X11/Xlib.h>

#include <GL/gl.h>
#include <GL/glx.h>

#undef None
#undef Bool
#undef True
#undef False
#undef Status
#undef Always
#undef Never

#include <cstdio>
#include <cstring>
#include <dlfcn.h>
#include <unordered_map>
#include <vector>

namespace ico::engine {

static constexpr u32 kMaxBatchVertices = 65536;
static constexpr u32 kVertsPerQuad = 6;
static constexpr u32 kMaxQuads = kMaxBatchVertices / kVertsPerQuad;

struct RenderTargetGL {
    GLuint fbo;
    GLuint colorTex;
    GLuint depthRbo;
    u32 width;
    u32 height;
};

struct TextureGL {
    GLuint tex;
};

struct OpenGLBackend::Impl {
    ::Display* display = nullptr;
    ::Window window = 0;
    ::GLXContext glxContext = nullptr;
    ::GLXWindow glxWindow = 0;
    ::GLXFBConfig fbConfig = nullptr;

    u32 width = 0;
    u32 height = 0;
    bool initialized = false;
    bool isGL33 = false;

    GLuint batchVAO = 0;
    GLuint batchVBO = 0;
    GLuint batchEBO = 0;
    u32 batchVertexCount = 0;
    BatchVertex batchVertices[kMaxBatchVertices];
    std::vector<u32> batchIndices;

    GLuint textureProgram = 0;
    GLuint solidProgram = 0;
    GLuint currentProgram = 0;
    GLint mvpLoc = -1;

    Matrix4x4 projMat = Matrix4x4::identity();
    Matrix4x4 viewMat = Matrix4x4::identity();
    Matrix4x4 modelMat = Matrix4x4::identity();

    TextureHandle boundTexture = kNullTexture;
    GSBlendMode currentBlendMode = GSBlendMode::None;
    GSDepthTest currentDepthTest = GSDepthTest::Less;
    bool depthWrite = true;
    GSAlphaTest currentAlphaTest = GSAlphaTest::Always;
    u8 alphaRef = 0;
    u8 alphaMask = 0xFF;
    RenderList currentList = RenderList::Opaque;
    u32 currentViewport[4] = {};
    bool scissorEnabled = false;

    std::unordered_map<TextureHandle, TextureGL> textures;
    std::unordered_map<RenderTargetHandle, RenderTargetGL> renderTargets;
    TextureHandle nextTexId = 1;
    RenderTargetHandle nextRTId = 1;
    RenderTargetHandle currentRT = kNullRenderTarget;

    u32 drawCallCount = 0;
    u32 triangleCount = 0;
};

typedef GLXContext (*PFNGLXCREATECONTEXTATTRIBSARBPROC)(::Display*, ::GLXFBConfig, GLXContext, int, const int*);

static PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARBFunc = nullptr;

static void* getGLProcAddress(const char* name) {
    static void* glLib = nullptr;
    if (!glLib) {
        glLib = dlopen("libGL.so.1", RTLD_LAZY | RTLD_NOLOAD);
        if (!glLib) {
            glLib = dlopen("libGL.so", RTLD_LAZY);
        }
    }
    if (!glLib) {
        return nullptr;
    }
    void* addr = dlsym(glLib, name);
    if (addr) {
        return addr;
    }
    typedef void* (*glXGetProcFn)(const unsigned char*);
    auto getProc = reinterpret_cast<glXGetProcFn>(dlsym(glLib, "glXGetProcAddressARB"));
    if (getProc) {
        return getProc(reinterpret_cast<const unsigned char*>(name));
    }
    return nullptr;
}

typedef void (*PFN_GLSHADERSOURCE)(GLuint, GLsizei, const GLchar**, const GLint*);
typedef void (*PFN_GLCOMPILESHADER)(GLuint);
typedef void (*PFN_GLGETSHADERIV)(GLuint, GLenum, GLint*);
typedef void (*PFN_GLGETSHADERINFOLOG)(GLuint, GLsizei, GLsizei*, GLchar*);
typedef GLuint (*PFN_GLCREATEPROGRAM)(void);
typedef void (*PFN_GLATTACHSHADER)(GLuint, GLuint);
typedef void (*PFN_GLLINKPROGRAM)(GLuint);
typedef void (*PFN_GLGETPROGRAMIV)(GLuint, GLenum, GLint*);
typedef void (*PFN_GLGETPROGRAMINFOLOG)(GLuint, GLsizei, GLsizei*, GLchar*);
typedef void (*PFN_GLUSEPROGRAM)(GLuint);
typedef void (*PFN_GLDELETESHADER)(GLuint);
typedef void (*PFN_GLDELETEPROGRAM)(GLuint);
typedef GLint (*PFN_GLGETUNIFORMLOCATION)(GLuint, const GLchar*);
typedef void (*PFN_GLUNIFORMMATRIX4FV)(GLint, GLsizei, GLboolean, const GLfloat*);
typedef void (*PFN_GLUNIFORM1I)(GLint, GLint);
typedef void (*PFN_GLGENBUFFERS)(GLsizei, GLuint*);
typedef void (*PFN_GLBINDBUFFER)(GLenum, GLuint);
typedef void (*PFN_GLBUFFERDATA)(GLenum, GLsizeiptr, const void*, GLenum);
typedef void (*PFN_GLBUFFERSUBDATA)(GLenum, GLintptr, GLsizeiptr, const void*);
typedef void (*PFN_GLDELETEBUFFERS)(GLsizei, const GLuint*);
typedef void (*PFN_GLENABLEVERTEXATTRIBARRAY)(GLuint);
typedef void (*PFN_GLDISABLEVERTEXATTRIBARRAY)(GLuint);
typedef void (*PFN_GLVERTEXATTRIBPOINTER)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*);
typedef void (*PFN_GLBINDATTRIBLOCATION)(GLuint, GLuint, const GLchar*);
typedef void (*PFN_GLGENVERTEXARRAYS)(GLsizei, GLuint*);
typedef void (*PFN_GLBINDVERTEXARRAY)(GLuint);
typedef void (*PFN_GLDELETEVERTEXARRAYS)(GLsizei, const GLuint*);
typedef void (*PFN_GLDRAWELEMENTS)(GLenum, GLsizei, GLenum, const void*);
typedef void (*PFN_GLDRAWARRAYS)(GLenum, GLint, GLsizei);
typedef void (*PFN_GLACTIVETEXTURE)(GLenum);
typedef void (*PFN_GLGENTEXTURES)(GLsizei, GLuint*);
typedef void (*PFN_GLBINDTEXTURE)(GLenum, GLuint);
typedef void (*PFN_GLTEXIMAGE2D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*);
typedef void (*PFN_GLTEXSUBIMAGE2D)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const void*);
typedef void (*PFN_GLDELETETEXTURES)(GLsizei, const GLuint*);
typedef void (*PFN_GLTEXPARAMETERI)(GLenum, GLenum, GLint);
typedef void (*PFN_GLGENERATEMIPMAP)(GLenum);
typedef void (*PFN_GLCOLORMASK)(GLboolean, GLboolean, GLboolean, GLboolean);
typedef void (*PFN_GLDEPTHMASK)(GLboolean);
typedef void (*PFN_GLDEPTHFUNC)(GLenum);
typedef void (*PFN_GLENABLE)(GLenum);
typedef void (*PFN_GLDISABLE)(GLenum);
typedef void (*PFN_GLBLENDFUNC)(GLenum, GLenum);
typedef void (*PFN_GLBLENDEQUATION)(GLenum);
typedef void (*PFN_GLSCISSOR)(GLint, GLint, GLsizei, GLsizei);
typedef void (*PFN_GLVIEWPORT)(GLint, GLint, GLsizei, GLsizei);
typedef void (*PFN_GLCLEARCOLOR)(GLfloat, GLfloat, GLfloat, GLfloat);
typedef void (*PFN_GLCLEAR)(GLbitfield);
typedef void (*PFN_GLCLEARDEPTHF)(GLfloat);
typedef void (*PFN_GLFRAMEBUFFERTEXTURE2D)(GLenum, GLenum, GLenum, GLuint, GLint);
typedef void (*PFN_GLFRAMEBUFFERRENDERBUFFER)(GLenum, GLenum, GLenum, GLuint);
typedef GLenum (*PFN_GLCHECKFRAMEBUFFERSTATUS)(GLenum);
typedef void (*PFN_GLGENFRAMEBUFFERS)(GLsizei, GLuint*);
typedef void (*PFN_GLBINDFRAMEBUFFER)(GLenum, GLuint);
typedef void (*PFN_GLDELETEFRAMEBUFFERS)(GLsizei, const GLuint*);
typedef void (*PFN_GLGENRENDERBUFFERS)(GLsizei, GLuint*);
typedef void (*PFN_GLBINDRENDERBUFFER)(GLenum, GLuint);
typedef void (*PFN_GLRENDERBUFFERSTORAGE)(GLenum, GLenum, GLsizei, GLsizei);
typedef void (*PFN_GLDELETERENDERBUFFERS)(GLsizei, const GLuint*);

#define LOAD_GL_FUNC(var, type, name) var = reinterpret_cast<type>(getGLProcAddress(name))

static PFN_GLSHADERSOURCE p_glShaderSource = nullptr;
static PFN_GLCOMPILESHADER p_glCompileShader = nullptr;
static PFN_GLGETSHADERIV p_glGetShaderiv = nullptr;
static PFN_GLGETSHADERINFOLOG p_glGetShaderInfoLog = nullptr;
static PFN_GLCREATEPROGRAM p_glCreateProgram = nullptr;
static PFN_GLATTACHSHADER p_glAttachShader = nullptr;
static PFN_GLLINKPROGRAM p_glLinkProgram = nullptr;
static PFN_GLGETPROGRAMIV p_glGetProgramiv = nullptr;
static PFN_GLGETPROGRAMINFOLOG p_glGetProgramInfoLog = nullptr;
static PFN_GLUSEPROGRAM p_glUseProgram = nullptr;
static PFN_GLDELETESHADER p_glDeleteShader = nullptr;
static PFN_GLDELETEPROGRAM p_glDeleteProgram = nullptr;
static PFN_GLGETUNIFORMLOCATION p_glGetUniformLocation = nullptr;
static PFN_GLUNIFORMMATRIX4FV p_glUniformMatrix4fv = nullptr;
static PFN_GLUNIFORM1I p_glUniform1i = nullptr;
static PFN_GLGENBUFFERS p_glGenBuffers = nullptr;
static PFN_GLBINDBUFFER p_glBindBuffer = nullptr;
static PFN_GLBUFFERDATA p_glBufferData = nullptr;
static PFN_GLBUFFERSUBDATA p_glBufferSubData = nullptr;
static PFN_GLDELETEBUFFERS p_glDeleteBuffers = nullptr;
static PFN_GLENABLEVERTEXATTRIBARRAY p_glEnableVertexAttribArray = nullptr;
static PFN_GLDISABLEVERTEXATTRIBARRAY p_glDisableVertexAttribArray = nullptr;
static PFN_GLVERTEXATTRIBPOINTER p_glVertexAttribPointer = nullptr;
static PFN_GLBINDATTRIBLOCATION p_glBindAttribLocation = nullptr;
static PFN_GLGENVERTEXARRAYS p_glGenVertexArrays = nullptr;
static PFN_GLBINDVERTEXARRAY p_glBindVertexArray = nullptr;
static PFN_GLDELETEVERTEXARRAYS p_glDeleteVertexArrays = nullptr;
static PFN_GLDRAWELEMENTS p_glDrawElements = nullptr;
static PFN_GLDRAWARRAYS p_glDrawArrays = nullptr;
static PFN_GLACTIVETEXTURE p_glActiveTexture = nullptr;
static PFN_GLGENTEXTURES p_glGenTextures = nullptr;
static PFN_GLBINDTEXTURE p_glBindTexture = nullptr;
static PFN_GLTEXIMAGE2D p_glTexImage2D = nullptr;
static PFN_GLTEXSUBIMAGE2D p_glTexSubImage2D = nullptr;
static PFN_GLDELETETEXTURES p_glDeleteTextures = nullptr;
static PFN_GLTEXPARAMETERI p_glTexParameteri = nullptr;
static PFN_GLGENERATEMIPMAP p_glGenerateMipmap = nullptr;
static PFN_GLCOLORMASK p_glColorMask = nullptr;
static PFN_GLDEPTHMASK p_glDepthMask = nullptr;
static PFN_GLDEPTHFUNC p_glDepthFunc = nullptr;
static PFN_GLENABLE p_glEnable = nullptr;
static PFN_GLDISABLE p_glDisable = nullptr;
static PFN_GLBLENDFUNC p_glBlendFunc = nullptr;
static PFN_GLBLENDEQUATION p_glBlendEquation = nullptr;
static PFN_GLSCISSOR p_glScissor = nullptr;
static PFN_GLVIEWPORT p_glViewport = nullptr;
static PFN_GLCLEARCOLOR p_glClearColor = nullptr;
static PFN_GLCLEAR p_glClear = nullptr;
static PFN_GLCLEARDEPTHF p_glClearDepthf = nullptr;
static PFN_GLFRAMEBUFFERTEXTURE2D p_glFramebufferTexture2D = nullptr;
static PFN_GLFRAMEBUFFERRENDERBUFFER p_glFramebufferRenderbuffer = nullptr;
static PFN_GLCHECKFRAMEBUFFERSTATUS p_glCheckFramebufferStatus = nullptr;
static PFN_GLGENFRAMEBUFFERS p_glGenFramebuffers = nullptr;
static PFN_GLBINDFRAMEBUFFER p_glBindFramebuffer = nullptr;
static PFN_GLDELETEFRAMEBUFFERS p_glDeleteFramebuffers = nullptr;
static PFN_GLGENRENDERBUFFERS p_glGenRenderbuffers = nullptr;
static PFN_GLBINDRENDERBUFFER p_glBindRenderbuffer = nullptr;
static PFN_GLRENDERBUFFERSTORAGE p_glRenderbufferStorage = nullptr;
static PFN_GLDELETERENDERBUFFERS p_glDeleteRenderbuffers = nullptr;

static bool loadGLFunctions() {
    LOAD_GL_FUNC(p_glShaderSource,             PFN_GLSHADERSOURCE,             "glShaderSource");
    LOAD_GL_FUNC(p_glCompileShader,            PFN_GLCOMPILESHADER,            "glCompileShader");
    LOAD_GL_FUNC(p_glGetShaderiv,              PFN_GLGETSHADERIV,              "glGetShaderiv");
    LOAD_GL_FUNC(p_glGetShaderInfoLog,         PFN_GLGETSHADERINFOLOG,         "glGetShaderInfoLog");
    LOAD_GL_FUNC(p_glCreateProgram,            PFN_GLCREATEPROGRAM,            "glCreateProgram");
    LOAD_GL_FUNC(p_glAttachShader,             PFN_GLATTACHSHADER,             "glAttachShader");
    LOAD_GL_FUNC(p_glLinkProgram,              PFN_GLLINKPROGRAM,              "glLinkProgram");
    LOAD_GL_FUNC(p_glGetProgramiv,             PFN_GLGETPROGRAMIV,             "glGetProgramiv");
    LOAD_GL_FUNC(p_glGetProgramInfoLog,        PFN_GLGETPROGRAMINFOLOG,        "glGetProgramInfoLog");
    LOAD_GL_FUNC(p_glUseProgram,               PFN_GLUSEPROGRAM,               "glUseProgram");
    LOAD_GL_FUNC(p_glDeleteShader,             PFN_GLDELETESHADER,             "glDeleteShader");
    LOAD_GL_FUNC(p_glDeleteProgram,            PFN_GLDELETEPROGRAM,            "glDeleteProgram");
    LOAD_GL_FUNC(p_glGetUniformLocation,       PFN_GLGETUNIFORMLOCATION,       "glGetUniformLocation");
    LOAD_GL_FUNC(p_glUniformMatrix4fv,         PFN_GLUNIFORMMATRIX4FV,         "glUniformMatrix4fv");
    LOAD_GL_FUNC(p_glUniform1i,                PFN_GLUNIFORM1I,                "glUniform1i");
    LOAD_GL_FUNC(p_glGenBuffers,               PFN_GLGENBUFFERS,               "glGenBuffers");
    LOAD_GL_FUNC(p_glBindBuffer,               PFN_GLBINDBUFFER,               "glBindBuffer");
    LOAD_GL_FUNC(p_glBufferData,               PFN_GLBUFFERDATA,               "glBufferData");
    LOAD_GL_FUNC(p_glBufferSubData,            PFN_GLBUFFERSUBDATA,            "glBufferSubData");
    LOAD_GL_FUNC(p_glDeleteBuffers,            PFN_GLDELETEBUFFERS,            "glDeleteBuffers");
    LOAD_GL_FUNC(p_glEnableVertexAttribArray,  PFN_GLENABLEVERTEXATTRIBARRAY,  "glEnableVertexAttribArray");
    LOAD_GL_FUNC(p_glDisableVertexAttribArray, PFN_GLDISABLEVERTEXATTRIBARRAY, "glDisableVertexAttribArray");
    LOAD_GL_FUNC(p_glVertexAttribPointer,      PFN_GLVERTEXATTRIBPOINTER,      "glVertexAttribPointer");
    LOAD_GL_FUNC(p_glBindAttribLocation,       PFN_GLBINDATTRIBLOCATION,       "glBindAttribLocation");
    LOAD_GL_FUNC(p_glGenVertexArrays,          PFN_GLGENVERTEXARRAYS,          "glGenVertexArrays");
    LOAD_GL_FUNC(p_glBindVertexArray,          PFN_GLBINDVERTEXARRAY,          "glBindVertexArray");
    LOAD_GL_FUNC(p_glDeleteVertexArrays,       PFN_GLDELETEVERTEXARRAYS,       "glDeleteVertexArrays");
    LOAD_GL_FUNC(p_glDrawElements,             PFN_GLDRAWELEMENTS,             "glDrawElements");
    LOAD_GL_FUNC(p_glDrawArrays,               PFN_GLDRAWARRAYS,               "glDrawArrays");
    LOAD_GL_FUNC(p_glActiveTexture,            PFN_GLACTIVETEXTURE,            "glActiveTexture");
    LOAD_GL_FUNC(p_glGenTextures,              PFN_GLGENTEXTURES,              "glGenTextures");
    LOAD_GL_FUNC(p_glBindTexture,              PFN_GLBINDTEXTURE,              "glBindTexture");
    LOAD_GL_FUNC(p_glTexImage2D,               PFN_GLTEXIMAGE2D,               "glTexImage2D");
    LOAD_GL_FUNC(p_glTexSubImage2D,            PFN_GLTEXSUBIMAGE2D,            "glTexSubImage2D");
    LOAD_GL_FUNC(p_glDeleteTextures,           PFN_GLDELETETEXTURES,           "glDeleteTextures");
    LOAD_GL_FUNC(p_glTexParameteri,            PFN_GLTEXPARAMETERI,            "glTexParameteri");
    LOAD_GL_FUNC(p_glGenerateMipmap,           PFN_GLGENERATEMIPMAP,           "glGenerateMipmap");
    LOAD_GL_FUNC(p_glColorMask,                PFN_GLCOLORMASK,                "glColorMask");
    LOAD_GL_FUNC(p_glDepthMask,                PFN_GLDEPTHMASK,                "glDepthMask");
    LOAD_GL_FUNC(p_glDepthFunc,                PFN_GLDEPTHFUNC,                "glDepthFunc");
    LOAD_GL_FUNC(p_glEnable,                   PFN_GLENABLE,                   "glEnable");
    LOAD_GL_FUNC(p_glDisable,                  PFN_GLDISABLE,                  "glDisable");
    LOAD_GL_FUNC(p_glBlendFunc,                PFN_GLBLENDFUNC,                "glBlendFunc");
    LOAD_GL_FUNC(p_glBlendEquation,            PFN_GLBLENDEQUATION,            "glBlendEquation");
    LOAD_GL_FUNC(p_glScissor,                  PFN_GLSCISSOR,                  "glScissor");
    LOAD_GL_FUNC(p_glViewport,                 PFN_GLVIEWPORT,                 "glViewport");
    LOAD_GL_FUNC(p_glClearColor,               PFN_GLCLEARCOLOR,               "glClearColor");
    LOAD_GL_FUNC(p_glClear,                    PFN_GLCLEAR,                    "glClear");
    LOAD_GL_FUNC(p_glClearDepthf,              PFN_GLCLEARDEPTHF,              "glClearDepthf");
    LOAD_GL_FUNC(p_glFramebufferTexture2D,     PFN_GLFRAMEBUFFERTEXTURE2D,     "glFramebufferTexture2D");
    LOAD_GL_FUNC(p_glFramebufferRenderbuffer,  PFN_GLFRAMEBUFFERRENDERBUFFER,  "glFramebufferRenderbuffer");
    LOAD_GL_FUNC(p_glCheckFramebufferStatus,   PFN_GLCHECKFRAMEBUFFERSTATUS,   "glCheckFramebufferStatus");
    LOAD_GL_FUNC(p_glGenFramebuffers,          PFN_GLGENFRAMEBUFFERS,          "glGenFramebuffers");
    LOAD_GL_FUNC(p_glBindFramebuffer,          PFN_GLBINDFRAMEBUFFER,          "glBindFramebuffer");
    LOAD_GL_FUNC(p_glDeleteFramebuffers,       PFN_GLDELETEFRAMEBUFFERS,       "glDeleteFramebuffers");
    LOAD_GL_FUNC(p_glGenRenderbuffers,         PFN_GLGENRENDERBUFFERS,         "glGenRenderbuffers");
    LOAD_GL_FUNC(p_glBindRenderbuffer,         PFN_GLBINDRENDERBUFFER,         "glBindRenderbuffer");
    LOAD_GL_FUNC(p_glRenderbufferStorage,      PFN_GLRENDERBUFFERSTORAGE,      "glRenderbufferStorage");
    LOAD_GL_FUNC(p_glDeleteRenderbuffers,      PFN_GLDELETERENDERBUFFERS,      "glDeleteRenderbuffers");
    return p_glCreateProgram != nullptr;
}

#undef LOAD_GL_FUNC

static const GLchar* kVertexShader33 =
    "#version 330 core\n"
    "layout(location = 0) in vec3 aPos;\n"
    "layout(location = 1) in vec3 aNormal;\n"
    "layout(location = 2) in vec2 aUV;\n"
    "layout(location = 3) in vec4 aColor;\n"
    "uniform mat4 uMVP;\n"
    "out vec2 vUV;\n"
    "out vec4 vColor;\n"
    "void main() {\n"
    "    gl_Position = uMVP * vec4(aPos, 1.0);\n"
    "    vUV = aUV;\n"
    "    vColor = aColor;\n"
    "}\n";

static const GLchar* kFragmentShader33 =
    "#version 330 core\n"
    "in vec2 vUV;\n"
    "in vec4 vColor;\n"
    "uniform sampler2D uTexture;\n"
    "uniform float uAlphaRef;\n"
    "uniform int uAlphaTestOp;\n"
    "out vec4 fragColor;\n"
    "void main() {\n"
    "    vec4 texColor = texture(uTexture, vUV);\n"
    "    vec4 result = texColor * vColor;\n"
    "    if (uAlphaTestOp == 5 && result.a < uAlphaRef) discard;\n"
    "    if (uAlphaTestOp == 2 && result.a > uAlphaRef) discard;\n"
    "    if (uAlphaTestOp == 6 && result.a <= uAlphaRef) discard;\n"
    "    if (uAlphaTestOp == 3 && result.a >= uAlphaRef) discard;\n"
    "    if (uAlphaTestOp == 4 && abs(result.a - uAlphaRef) > 0.001) discard;\n"
    "    if (uAlphaTestOp == 7 && abs(result.a - uAlphaRef) < 0.001) discard;\n"
    "    fragColor = result;\n"
    "}\n";

static const GLchar* kVertexShader21 =
    "varying vec2 vUV;\n"
    "varying vec4 vColor;\n"
    "void main() {\n"
    "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
    "    vUV = vec2(gl_MultiTexCoord0);\n"
    "    vColor = gl_Color;\n"
    "}\n";

static const GLchar* kFragmentShader21 =
    "varying vec2 vUV;\n"
    "varying vec4 vColor;\n"
    "uniform sampler2D uTexture;\n"
    "void main() {\n"
    "    vec4 texColor = texture2D(uTexture, vUV);\n"
    "    gl_FragColor = texColor * vColor;\n"
    "}\n";

static GLuint compileShaderProgram(const GLchar* vertSrc, const GLchar* fragSrc,
                                   bool isGL33) {
    GLuint vert = 0;
    {
        GLuint s = p_glCreateProgram();
        p_glShaderSource(s, 1, &vertSrc, nullptr);
        p_glCompileShader(s);
        GLint ok = 0;
        p_glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            char log[512];
            p_glGetShaderInfoLog(s, sizeof(log), nullptr, log);
            std::fprintf(stderr, "[render] vertex compile: %s\n", log);
            p_glDeleteShader(s);
            return 0;
        }
        vert = s;
    }

    GLuint frag = 0;
    {
        GLuint s = p_glCreateProgram();
        p_glShaderSource(s, 1, &fragSrc, nullptr);
        p_glCompileShader(s);
        GLint ok = 0;
        p_glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            char log[512];
            p_glGetShaderInfoLog(s, sizeof(log), nullptr, log);
            std::fprintf(stderr, "[render] fragment compile: %s\n", log);
            p_glDeleteShader(vert);
            p_glDeleteShader(s);
            return 0;
        }
        frag = s;
    }

    GLuint prog = p_glCreateProgram();
    p_glAttachShader(prog, vert);
    p_glAttachShader(prog, frag);

    if (!isGL33 && p_glBindAttribLocation) {
        p_glBindAttribLocation(prog, 0, "gl_Vertex");
        p_glBindAttribLocation(prog, 2, "gl_MultiTexCoord0");
        p_glBindAttribLocation(prog, 3, "gl_Color");
    }

    p_glLinkProgram(prog);
    GLint linked = 0;
    p_glGetProgramiv(prog, GL_LINK_STATUS, &linked);
    if (!linked) {
        char log[512];
        p_glGetProgramInfoLog(prog, sizeof(log), nullptr, log);
        std::fprintf(stderr, "[render] link: %s\n", log);
        p_glDeleteShader(vert);
        p_glDeleteShader(frag);
        p_glDeleteProgram(prog);
        return 0;
    }

    p_glDeleteShader(vert);
    p_glDeleteShader(frag);
    return prog;
}

// --- OpenGLBackend ---

OpenGLBackend::OpenGLBackend() : m_impl(std::make_unique<Impl>()) {}

OpenGLBackend::~OpenGLBackend() {
    shutdown();
}

void OpenGLBackendTestHelper::setTestMode(OpenGLBackend& backend) {
    backend.setTestMode();
}

void OpenGLBackend::setTestMode() {
    m_impl->initialized = true;
    m_impl->width = 640;
    m_impl->height = 448;
    m_impl->nextTexId = 1;
    m_impl->nextRTId = 1;
}

bool OpenGLBackend::initialize(u32 width, u32 height) {
    auto& I = *m_impl;

    I.width = width;
    I.height = height;

    I.display = XOpenDisplay(nullptr);
    if (!I.display) {
        std::fprintf(stderr, "[render] XOpenDisplay failed\n");
        return false;
    }

    int glxMajor = 0, glxMinor = 0;
    if (!glXQueryVersion(I.display, &glxMajor, &glxMinor) ||
        (glxMajor == 1 && glxMinor < 3) || glxMajor < 1) {
        std::fprintf(stderr, "[render] GLX 1.3+ required\n");
        XCloseDisplay(I.display);
        I.display = nullptr;
        return false;
    }

    int fbAttribs[] = {
        GLX_X_RENDERABLE,  1,
        GLX_DRAWABLE_TYPE,  GLX_WINDOW_BIT,
        GLX_RENDER_TYPE,    GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE,  GLX_TRUE_COLOR,
        GLX_RED_SIZE,       8,
        GLX_GREEN_SIZE,     8,
        GLX_BLUE_SIZE,      8,
        GLX_ALPHA_SIZE,     8,
        GLX_DEPTH_SIZE,     24,
        GLX_STENCIL_SIZE,   8,
        GLX_DOUBLEBUFFER,   1,
        0
    };

    int fbCount = 0;
    GLXFBConfig* fbs = glXChooseFBConfig(I.display, DefaultScreen(I.display),
                                          fbAttribs, &fbCount);
    if (!fbs || fbCount == 0) {
        std::fprintf(stderr, "[render] glXChooseFBConfig failed\n");
        XCloseDisplay(I.display);
        I.display = nullptr;
        return false;
    }

    I.fbConfig = fbs[0];

    XVisualInfo* vi = glXGetVisualFromFBConfig(I.display, I.fbConfig);
    if (!vi) {
        std::fprintf(stderr, "[render] glXGetVisualFromFBConfig failed\n");
        XFree(fbs);
        XCloseDisplay(I.display);
        I.display = nullptr;
        return false;
    }

    XSetWindowAttributes swa;
    swa.colormap = XCreateColormap(I.display, RootWindow(I.display, vi->screen),
                                   vi->visual, AllocNone);
    swa.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;

    I.window = XCreateWindow(I.display, RootWindow(I.display, vi->screen),
                              0, 0, width, height, 0, vi->depth, InputOutput,
                              vi->visual, CWColormap | CWEventMask, &swa);
    XFree(vi);

    if (!I.window) {
        std::fprintf(stderr, "[render] XCreateWindow failed\n");
        XFree(fbs);
        XCloseDisplay(I.display);
        I.display = nullptr;
        return false;
    }

    XStoreName(I.display, I.window, "ICO");
    XMapWindow(I.display, I.window);

    glXCreateContextAttribsARBFunc = reinterpret_cast<PFNGLXCREATECONTEXTATTRIBSARBPROC>(
        getGLProcAddress("glXCreateContextAttribsARB"));

    bool created = false;
    if (glXCreateContextAttribsARBFunc) {
        int ctxAttribs[] = {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
            GLX_CONTEXT_MINOR_VERSION_ARB, 3,
            GLX_CONTEXT_PROFILE_MASK_ARB,  GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
            0
        };
        I.glxContext = glXCreateContextAttribsARBFunc(I.display, I.fbConfig,
                                                       nullptr, 1, ctxAttribs);
        if (I.glxContext) {
            glXMakeCurrent(I.display, I.window, I.glxContext);
            I.isGL33 = true;
            created = true;
            std::fprintf(stderr, "[render] GL 3.3 core\n");
        }
    }

    if (!created) {
        int ctxAttribs[] = { 0 };
        I.glxContext = glXCreateContextAttribsARBFunc
            ? glXCreateContextAttribsARBFunc(I.display, I.fbConfig, nullptr, 1, ctxAttribs)
            : glXCreateNewContext(I.display, I.fbConfig, GLX_RGBA_TYPE, nullptr, 1);
        if (I.glxContext) {
            glXMakeCurrent(I.display, I.window, I.glxContext);
            I.isGL33 = false;
            created = true;
            std::fprintf(stderr, "[render] GL 2.1 fallback\n");
        }
    }

    XFree(fbs);

    if (!created || !I.glxContext) {
        std::fprintf(stderr, "[render] GL context creation failed\n");
        XDestroyWindow(I.display, I.window);
        XCloseDisplay(I.display);
        I.display = nullptr;
        return false;
    }

    if (!loadGLFunctions()) {
        std::fprintf(stderr, "[render] failed to load GL functions\n");
        return false;
    }

    I.glxWindow = glXCreateWindow(I.display, I.fbConfig, I.window, nullptr);

    p_glEnable(GL_DEPTH_TEST);
    p_glDepthFunc(GL_LEQUAL);
    p_glDepthMask(GL_TRUE);
    p_glEnable(GL_BLEND);
    p_glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    p_glEnable(GL_ALPHA_TEST);
    p_glEnable(GL_SCISSOR_TEST);
    p_glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
    p_glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    p_glClearDepthf(1.0f);

    p_glGenVertexArrays(1, &I.batchVAO);
    p_glGenBuffers(1, &I.batchVBO);
    p_glGenBuffers(1, &I.batchEBO);

    p_glBindVertexArray(I.batchVAO);
    p_glBindBuffer(GL_ARRAY_BUFFER, I.batchVBO);
    p_glBufferData(GL_ARRAY_BUFFER,
                    static_cast<GLsizeiptr>(kMaxBatchVertices * sizeof(BatchVertex)),
                    nullptr, GL_DYNAMIC_DRAW);

    p_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, I.batchEBO);
    std::vector<u32> indices;
    indices.reserve(kMaxBatchVertices);
    for (u32 i = 0; i < kMaxBatchVertices; i += 4) {
        indices.push_back(i);
        indices.push_back(i + 1);
        indices.push_back(i + 2);
        indices.push_back(i);
        indices.push_back(i + 2);
        indices.push_back(i + 3);
    }
    p_glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                    static_cast<GLsizeiptr>(indices.size() * sizeof(u32)),
                    indices.data(), GL_STATIC_DRAW);

    constexpr GLsizei stride = sizeof(BatchVertex);
    p_glEnableVertexAttribArray(0);
    p_glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride,
                             reinterpret_cast<const void*>(offsetof(BatchVertex, x)));
    p_glEnableVertexAttribArray(1);
    p_glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                             reinterpret_cast<const void*>(offsetof(BatchVertex, nx)));
    p_glEnableVertexAttribArray(2);
    p_glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride,
                             reinterpret_cast<const void*>(offsetof(BatchVertex, u)));
    p_glEnableVertexAttribArray(3);
    p_glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride,
                             reinterpret_cast<const void*>(offsetof(BatchVertex, r)));

    p_glBindVertexArray(0);

    I.textureProgram = compileShaderProgram(kVertexShader33, kFragmentShader33, true);
    I.solidProgram = compileShaderProgram(kVertexShader33, kFragmentShader33, true);
    I.currentProgram = I.textureProgram;
    I.mvpLoc = p_glGetUniformLocation(I.textureProgram, "uMVP");

    if (I.isGL33) {
        GLuint texLoc = p_glGetUniformLocation(I.textureProgram, "uTexture");
        p_glUseProgram(I.textureProgram);
        p_glUniform1i(texLoc, 0);
        texLoc = p_glGetUniformLocation(I.solidProgram, "uTexture");
        p_glUseProgram(I.solidProgram);
        p_glUniform1i(texLoc, 0);
    }

    I.initialized = true;
    I.batchVertexCount = 0;
    I.currentBlendMode = GSBlendMode::None;
    I.currentDepthTest = GSDepthTest::Less;
    I.depthWrite = true;
    I.currentAlphaTest = GSAlphaTest::Always;
    I.alphaRef = 0;
    I.alphaMask = 0xFF;
    I.currentList = RenderList::Opaque;
    I.currentRT = kNullRenderTarget;

    std::fprintf(stderr, "[render] OpenGLBackend initialized: %ux%u (%s)\n",
                  width, height, I.isGL33 ? "GL3.3" : "GL2.1");
    return true;
}

void OpenGLBackend::shutdown() {
    auto& I = *m_impl;
    if (!hasGL()) {
        I.initialized = false;
        return;
    }

    for (auto& [id, tex] : I.textures) {
        p_glDeleteTextures(1, &tex.tex);
    }
    I.textures.clear();

    for (auto& [id, rt] : I.renderTargets) {
        p_glDeleteFramebuffers(1, &rt.fbo);
        p_glDeleteTextures(1, &rt.colorTex);
        p_glDeleteRenderbuffers(1, &rt.depthRbo);
    }
    I.renderTargets.clear();

    if (I.textureProgram) p_glDeleteProgram(I.textureProgram);
    if (I.solidProgram) p_glDeleteProgram(I.solidProgram);
    I.textureProgram = 0;
    I.solidProgram = 0;
    I.currentProgram = 0;

    p_glDeleteBuffers(1, &I.batchVBO);
    p_glDeleteBuffers(1, &I.batchEBO);
    p_glDeleteVertexArrays(1, &I.batchVAO);

    if (I.glxWindow) {
        glXDestroyWindow(I.display, I.glxWindow);
        I.glxWindow = 0;
    }
    if (I.glxContext) {
        glXMakeCurrent(I.display, 0, nullptr);
        glXDestroyContext(I.display, I.glxContext);
        I.glxContext = nullptr;
    }
    if (I.window) {
        XDestroyWindow(I.display, I.window);
        I.window = 0;
    }
    if (I.display) {
        XCloseDisplay(I.display);
        I.display = nullptr;
    }

    I.initialized = false;
    std::fprintf(stderr, "[render] OpenGLBackend shutdown\n");
}

bool OpenGLBackend::isInitialized() const { return m_impl->initialized; }

void OpenGLBackend::beginFrame() {
    auto& I = *m_impl;
    if (!I.initialized) return;
    I.batchVertexCount = 0;
    I.drawCallCount = 0;
    I.triangleCount = 0;
}

void OpenGLBackend::endFrame() {
    auto& I = *m_impl;
    if (!hasGL()) return;
    flushBatch();
}

void OpenGLBackend::present() {
    auto& I = *m_impl;
    if (!I.initialized) return;
    if (!I.display || !I.glxWindow) return;
    typedef void (*PFNGLXSWAPBUFFERS)(::Display*, ::GLXDrawable);
    auto fn = reinterpret_cast<PFNGLXSWAPBUFFERS>(getGLProcAddress("glXSwapBuffers"));
    if (fn) {
        fn(I.display, I.glxWindow);
    }
    std::fprintf(stderr, "[render] Frame: %u draw calls, %u triangles\n",
                  I.drawCallCount, I.triangleCount);
}

void OpenGLBackend::clear(u8 r, u8 g, u8 b, u8 a) {
    auto& I = *m_impl;
    if (!hasGL()) return;
    flushBatch();
    p_glClearColor(static_cast<float>(r) / 255.0f,
                    static_cast<float>(g) / 255.0f,
                    static_cast<float>(b) / 255.0f,
                    static_cast<float>(a) / 255.0f);
    p_glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLBackend::clearDepth() {
    auto& I = *m_impl;
    if (!hasGL()) return;
    flushBatch();
    p_glClear(GL_DEPTH_BUFFER_BIT);
}

void OpenGLBackend::setViewport(u32 x, u32 y, u32 w, u32 h) {
    auto& I = *m_impl;
    if (!hasGL()) return;
    flushBatch();
    I.currentViewport[0] = static_cast<GLint>(x);
    I.currentViewport[1] = static_cast<GLint>(y);
    I.currentViewport[2] = static_cast<GLsizei>(w);
    I.currentViewport[3] = static_cast<GLsizei>(h);
    p_glViewport(I.currentViewport[0], I.currentViewport[1],
                  I.currentViewport[2], I.currentViewport[3]);
}

void OpenGLBackend::setScissor(u32 x, u32 y, u32 w, u32 h) {
    auto& I = *m_impl;
    if (!hasGL()) return;
    flushBatch();
    I.scissorEnabled = (w > 0 && h > 0);
    if (I.scissorEnabled) {
        p_glEnable(GL_SCISSOR_TEST);
        p_glScissor(static_cast<GLint>(x), static_cast<GLint>(y),
                     static_cast<GLsizei>(w), static_cast<GLsizei>(h));
    } else {
        p_glDisable(GL_SCISSOR_TEST);
    }
}

void OpenGLBackend::swapBuffers() {
    auto& I = *m_impl;
    if (!I.initialized) return;
    if (!I.display || !I.glxWindow) return;
    typedef void (*PFNGLXSWAPBUFFERS)(::Display*, ::GLXDrawable);
    auto fn = reinterpret_cast<PFNGLXSWAPBUFFERS>(getGLProcAddress("glXSwapBuffers"));
    if (fn) {
        fn(I.display, I.glxWindow);
    }
}

void OpenGLBackend::setBlendMode(GSBlendMode mode) {
    auto& I = *m_impl;
    if (!hasGL()) return;
    if (I.currentBlendMode == mode) return;
    flushBatch();
    I.currentBlendMode = mode;

    switch (mode) {
        case GSBlendMode::None:
            p_glDisable(GL_BLEND);
            break;
        case GSBlendMode::Alpha:
            p_glEnable(GL_BLEND);
            p_glBlendEquation(GL_FUNC_ADD);
            p_glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;
        case GSBlendMode::Additive:
            p_glEnable(GL_BLEND);
            p_glBlendEquation(GL_FUNC_ADD);
            p_glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            break;
        case GSBlendMode::Subtractive:
            p_glEnable(GL_BLEND);
            p_glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
            p_glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            break;
    }
}

void OpenGLBackend::setDepthTest(GSDepthTest test, bool write) {
    auto& I = *m_impl;
    if (!hasGL()) return;
    if (I.currentDepthTest == test && I.depthWrite == write) return;
    flushBatch();
    I.currentDepthTest = test;
    I.depthWrite = write;

    static const GLenum glDepthFuncs[] = {
        GL_NEVER, GL_ALWAYS, GL_LESS, GL_LEQUAL,
        GL_EQUAL, GL_GREATER, GL_GEQUAL, GL_NOTEQUAL
    };

    p_glDepthFunc(glDepthFuncs[static_cast<u32>(test)]);
    p_glDepthMask(write ? GL_TRUE : GL_FALSE);
}

void OpenGLBackend::setAlphaTest(GSAlphaTest test, u8 ref, u8 mask) {
    auto& I = *m_impl;
    if (!hasGL()) return;
    if (I.currentAlphaTest == test && I.alphaRef == ref && I.alphaMask == mask) return;
    flushBatch();
    I.currentAlphaTest = test;
    I.alphaRef = ref;
    I.alphaMask = mask;

    if (I.isGL33) {
        GLint loc = p_glGetUniformLocation(I.currentProgram, "uAlphaRef");
        if (loc >= 0) {
            p_glUniform1i(loc, static_cast<GLint>(static_cast<u32>(ref) & mask));
        }
        loc = p_glGetUniformLocation(I.currentProgram, "uAlphaTestOp");
        if (loc >= 0) {
            p_glUniform1i(loc, static_cast<GLint>(test));
        }
    } else {
        static const GLenum glAlphaFuncs[] = {
            GL_NEVER, GL_ALWAYS, GL_LESS, GL_LEQUAL,
            GL_EQUAL, GL_GREATER, GL_GEQUAL, GL_NOTEQUAL
        };
        if (test == GSAlphaTest::Always) {
            p_glDisable(GL_ALPHA_TEST);
        } else {
            p_glEnable(GL_ALPHA_TEST);
            glAlphaFunc(glAlphaFuncs[static_cast<u32>(test)],
                         static_cast<float>(ref) / 255.0f);
        }
    }
}

TextureHandle OpenGLBackend::createTexture(const TextureDesc& desc) {
    auto& I = *m_impl;
    if (!hasGL()) return kNullTexture;

    GLuint tex = 0;
    p_glGenTextures(1, &tex);
    p_glBindTexture(GL_TEXTURE_2D, tex);

    GLenum internalFmt = GL_RGBA8;
    GLenum fmt = GL_RGBA;
    GLenum type = GL_UNSIGNED_BYTE;

    switch (desc.format) {
        case TextureFormat::PSMCT32:
            internalFmt = GL_RGBA8; fmt = GL_RGBA; type = GL_UNSIGNED_BYTE;
            break;
        case TextureFormat::PSMCT24:
            internalFmt = GL_RGB8; fmt = GL_RGB; type = GL_UNSIGNED_BYTE;
            break;
        case TextureFormat::PSMCT16:
        case TextureFormat::PSMCT16S:
            internalFmt = GL_RGBA4; fmt = GL_RGBA; type = GL_UNSIGNED_SHORT_4_4_4_4;
            break;
        default:
            internalFmt = GL_RGBA8; fmt = GL_RGBA; type = GL_UNSIGNED_BYTE;
            break;
    }

    if (desc.data && desc.dataSize > 0) {
        p_glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(internalFmt),
                        static_cast<GLsizei>(desc.width),
                        static_cast<GLsizei>(desc.height), 0,
                        fmt, type, desc.data);
        if (desc.generateMipmaps) {
            p_glGenerateMipmap(GL_TEXTURE_2D);
        }
    } else {
        p_glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                        static_cast<GLsizei>(desc.width),
                        static_cast<GLsizei>(desc.height), 0,
                        GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }

    p_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                       desc.generateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    p_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    p_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    p_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    TextureHandle id = I.nextTexId++;
    I.textures[id] = { tex };
    return id;
}

void OpenGLBackend::destroyTexture(TextureHandle handle) {
    auto& I = *m_impl;
    if (!hasGL() || handle == kNullTexture) return;
    auto it = I.textures.find(handle);
    if (it != I.textures.end()) {
        p_glDeleteTextures(1, &it->second.tex);
        I.textures.erase(it);
    }
}

void OpenGLBackend::bindTexture(TextureHandle handle, u32 slot) {
    auto& I = *m_impl;
    if (!hasGL()) return;
    if (I.boundTexture == handle) return;
    flushBatch();
    I.boundTexture = handle;

    if (p_glActiveTexture) {
        p_glActiveTexture(GL_TEXTURE0 + slot);
    }

    auto it = I.textures.find(handle);
    if (it != I.textures.end()) {
        p_glBindTexture(GL_TEXTURE_2D, it->second.tex);
        if (!I.isGL33) {
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        }
    } else {
        p_glBindTexture(GL_TEXTURE_2D, 0);
    }
}

RenderTargetHandle OpenGLBackend::createRenderTarget(u32 width, u32 height) {
    auto& I = *m_impl;
    if (!hasGL()) return kNullRenderTarget;

    RenderTargetGL rt{};
    rt.width = width;
    rt.height = height;

    p_glGenFramebuffers(1, &rt.fbo);
    p_glBindFramebuffer(GL_FRAMEBUFFER, rt.fbo);

    p_glGenTextures(1, &rt.colorTex);
    p_glBindTexture(GL_TEXTURE_2D, rt.colorTex);
    p_glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                    static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0,
                    GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    p_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    p_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    p_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    p_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    p_glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                              GL_TEXTURE_2D, rt.colorTex, 0);

    p_glGenRenderbuffers(1, &rt.depthRbo);
    p_glBindRenderbuffer(GL_RENDERBUFFER, rt.depthRbo);
    p_glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
                             static_cast<GLsizei>(width), static_cast<GLsizei>(height));
    p_glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                                 GL_RENDERBUFFER, rt.depthRbo);

    GLenum status = p_glCheckFramebufferStatus(GL_FRAMEBUFFER);
    p_glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::fprintf(stderr, "[render] FBO incomplete: 0x%X\n", status);
        p_glDeleteFramebuffers(1, &rt.fbo);
        p_glDeleteTextures(1, &rt.colorTex);
        p_glDeleteRenderbuffers(1, &rt.depthRbo);
        return kNullRenderTarget;
    }

    RenderTargetHandle id = I.nextRTId++;
    I.renderTargets[id] = rt;
    return id;
}

void OpenGLBackend::destroyRenderTarget(RenderTargetHandle handle) {
    auto& I = *m_impl;
    if (!hasGL() || handle == kNullRenderTarget) return;
    auto it = I.renderTargets.find(handle);
    if (it != I.renderTargets.end()) {
        p_glDeleteFramebuffers(1, &it->second.fbo);
        p_glDeleteTextures(1, &it->second.colorTex);
        p_glDeleteRenderbuffers(1, &it->second.depthRbo);
        I.renderTargets.erase(it);
    }
}

void OpenGLBackend::setRenderTarget(RenderTargetHandle handle) {
    auto& I = *m_impl;
    if (!hasGL()) return;
    if (I.currentRT == handle) return;
    flushBatch();
    I.currentRT = handle;

    if (handle == kNullRenderTarget) {
        p_glBindFramebuffer(GL_FRAMEBUFFER, 0);
        p_glViewport(0, 0, static_cast<GLsizei>(I.width), static_cast<GLsizei>(I.height));
    } else {
        auto it = I.renderTargets.find(handle);
        if (it != I.renderTargets.end()) {
            p_glBindFramebuffer(GL_FRAMEBUFFER, it->second.fbo);
            p_glViewport(0, 0,
                          static_cast<GLsizei>(it->second.width),
                          static_cast<GLsizei>(it->second.height));
        }
    }
}

void OpenGLBackend::setMatrices(const Matrix4x4& projection,
                                 const Matrix4x4& view,
                                 const Matrix4x4& model) {
    auto& I = *m_impl;
    if (!I.initialized) return;
    I.projMat = projection;
    I.viewMat = view;
    I.modelMat = model;
}

void OpenGLBackend::drawPrimitive(GSPrimitive primitive, RenderList list,
                                   const RenderVertex* vertices, u32 count,
                                   TextureHandle texture,
                                   u8 r, u8 g, u8 b, u8 a) {
    auto& I = *m_impl;
    if (!I.initialized || !vertices || count == 0) return;

    if (I.currentList != list) {
        flushBatch();
        I.currentList = list;
    }

    (void)primitive;

    for (u32 i = 0; i < count; ++i) {
        if (I.batchVertexCount >= kMaxBatchVertices) {
            flushBatch();
            I.drawCallCount++;
            I.triangleCount += I.batchVertexCount / 3;
        }
        BatchVertex bv{};
        bv.x = vertices[i].x;
        bv.y = vertices[i].y;
        bv.z = vertices[i].z;
        bv.nx = vertices[i].nx;
        bv.ny = vertices[i].ny;
        bv.nz = vertices[i].nz;
        bv.u = vertices[i].u;
        bv.v = vertices[i].v;
        bv.r = r;
        bv.g = g;
        bv.b = b;
        bv.a = a;
        I.batchVertices[I.batchVertexCount++] = bv;
    }

    (void)texture;
}

void OpenGLBackend::drawIndexed(GSPrimitive primitive, RenderList list,
                                 const u32* indices, u32 indexCount,
                                 const RenderVertex* vertices, u32 vertexOffset,
                                 TextureHandle texture,
                                 u8 r, u8 g, u8 b, u8 a) {
    auto& I = *m_impl;
    if (!I.initialized || !indices || indexCount == 0) return;

    if (I.currentList != list) {
        flushBatch();
        I.currentList = list;
    }

    (void)primitive;

    for (u32 i = 0; i < indexCount; ++i) {
        u32 vi = indices[i] + vertexOffset;
        if (I.batchVertexCount >= kMaxBatchVertices) {
            flushBatch();
            I.drawCallCount++;
            I.triangleCount += I.batchVertexCount / 3;
        }
        BatchVertex bv{};
        bv.x = vertices[vi].x;
        bv.y = vertices[vi].y;
        bv.z = vertices[vi].z;
        bv.nx = vertices[vi].nx;
        bv.ny = vertices[vi].ny;
        bv.nz = vertices[vi].nz;
        bv.u = vertices[vi].u;
        bv.v = vertices[vi].v;
        bv.r = r;
        bv.g = g;
        bv.b = b;
        bv.a = a;
        I.batchVertices[I.batchVertexCount++] = bv;
    }

    (void)texture;
}

void OpenGLBackend::drawSprite(float x, float y, float w, float h,
                                float u0, float v0, float u1, float v1,
                                TextureHandle texture,
                                u8 r, u8 g, u8 b, u8 a) {
    auto& I = *m_impl;
    if (!I.initialized) return;

    if (I.currentList != RenderList::UI) {
        flushBatch();
        I.currentList = RenderList::UI;
    }

    if (I.batchVertexCount + 4 > kMaxBatchVertices) {
        flushBatch();
    }

    Matrix4x4 mvp = Matrix4x4::multiply(I.projMat, Matrix4x4::multiply(I.viewMat, I.modelMat));
    float sx = mvp.m[0] * 2.0f / static_cast<float>(I.width);
    float sy = mvp.m[5] * 2.0f / static_cast<float>(I.height);
    float tx = mvp.m[12];
    float ty = mvp.m[13];

    u32 base = I.batchVertexCount;

    BatchVertex v00{};
    v00.x = x * sx + tx;
    v00.y = (y + h) * sy + ty;
    v00.z = 0.0f;
    v00.u = u0;
    v00.v = v1;
    v00.r = r; v00.g = g; v00.b = b; v00.a = a;
    I.batchVertices[I.batchVertexCount++] = v00;

    BatchVertex v10{};
    v10.x = (x + w) * sx + tx;
    v10.y = (y + h) * sy + ty;
    v10.z = 0.0f;
    v10.u = u1;
    v10.v = v1;
    v10.r = r; v10.g = g; v10.b = b; v10.a = a;
    I.batchVertices[I.batchVertexCount++] = v10;

    BatchVertex v11{};
    v11.x = (x + w) * sx + tx;
    v11.y = y * sy + ty;
    v11.z = 0.0f;
    v11.u = u1;
    v11.v = v0;
    v11.r = r; v11.g = g; v11.b = b; v11.a = a;
    I.batchVertices[I.batchVertexCount++] = v11;

    BatchVertex v01{};
    v01.x = x * sx + tx;
    v01.y = y * sy + ty;
    v01.z = 0.0f;
    v01.u = u0;
    v01.v = v0;
    v01.r = r; v01.g = g; v01.b = b; v01.a = a;
    I.batchVertices[I.batchVertexCount++] = v01;

    I.batchIndices.push_back(base + 0);
    I.batchIndices.push_back(base + 1);
    I.batchIndices.push_back(base + 2);
    I.batchIndices.push_back(base + 0);
    I.batchIndices.push_back(base + 2);
    I.batchIndices.push_back(base + 3);

    (void)texture;
}

void OpenGLBackend::drawSpriteGouraud(float x, float y, float w, float h,
                                       float u0, float v0, float u1, float v1,
                                       TextureHandle texture,
                                       const u8 cornerColors[4][4]) {
    auto& I = *m_impl;
    if (!I.initialized || !cornerColors) return;

    if (I.currentList != RenderList::UI) {
        flushBatch();
        I.currentList = RenderList::UI;
    }

    if (I.batchVertexCount + 4 > kMaxBatchVertices) {
        flushBatch();
    }

    Matrix4x4 mvp = Matrix4x4::multiply(I.projMat, Matrix4x4::multiply(I.viewMat, I.modelMat));
    float sx = mvp.m[0] * 2.0f / static_cast<float>(I.width);
    float sy = mvp.m[5] * 2.0f / static_cast<float>(I.height);
    float tx = mvp.m[12];
    float ty = mvp.m[13];

    u32 base = I.batchVertexCount;

    BatchVertex v00{};
    v00.x = x * sx + tx;
    v00.y = (y + h) * sy + ty;
    v00.z = 0.0f;
    v00.u = u0;
    v00.v = v1;
    v00.r = cornerColors[0][0]; v00.g = cornerColors[0][1];
    v00.b = cornerColors[0][2]; v00.a = cornerColors[0][3];
    I.batchVertices[I.batchVertexCount++] = v00;

    BatchVertex v10{};
    v10.x = (x + w) * sx + tx;
    v10.y = (y + h) * sy + ty;
    v10.z = 0.0f;
    v10.u = u1;
    v10.v = v1;
    v10.r = cornerColors[1][0]; v10.g = cornerColors[1][1];
    v10.b = cornerColors[1][2]; v10.a = cornerColors[1][3];
    I.batchVertices[I.batchVertexCount++] = v10;

    BatchVertex v11{};
    v11.x = (x + w) * sx + tx;
    v11.y = y * sy + ty;
    v11.z = 0.0f;
    v11.u = u1;
    v11.v = v0;
    v11.r = cornerColors[2][0]; v11.g = cornerColors[2][1];
    v11.b = cornerColors[2][2]; v11.a = cornerColors[2][3];
    I.batchVertices[I.batchVertexCount++] = v11;

    BatchVertex v01{};
    v01.x = x * sx + tx;
    v01.y = y * sy + ty;
    v01.z = 0.0f;
    v01.u = u0;
    v01.v = v0;
    v01.r = cornerColors[3][0]; v01.g = cornerColors[3][1];
    v01.b = cornerColors[3][2]; v01.a = cornerColors[3][3];
    I.batchVertices[I.batchVertexCount++] = v01;

    I.batchIndices.push_back(base + 0);
    I.batchIndices.push_back(base + 1);
    I.batchIndices.push_back(base + 2);
    I.batchIndices.push_back(base + 0);
    I.batchIndices.push_back(base + 2);
    I.batchIndices.push_back(base + 3);

    (void)texture;
}

void OpenGLBackend::beginPass(RenderList list) {
    auto& I = *m_impl;
    if (!hasGL()) return;
    if (I.currentList != list) {
        flushBatch();
        I.currentList = list;
    }
}

void OpenGLBackend::endPass() {
    auto& I = *m_impl;
    if (!hasGL()) return;
    flushBatch();
}

u32 OpenGLBackend::getWidth() const { return m_impl->width; }
u32 OpenGLBackend::getHeight() const { return m_impl->height; }

bool OpenGLBackend::hasGL() const { return m_impl->initialized && m_impl->display != nullptr; }

void OpenGLBackend::flushBatch() {
    auto& I = *m_impl;
    if (I.batchVertexCount == 0) return;
    if (!I.display) return;

    Matrix4x4 mvp = Matrix4x4::multiply(I.projMat, Matrix4x4::multiply(I.viewMat, I.modelMat));

    if (I.isGL33) {
        p_glUseProgram(I.currentProgram);
        if (I.mvpLoc >= 0) {
            p_glUniformMatrix4fv(I.mvpLoc, 1, GL_FALSE, mvp.m);
        }
    } else {
        glLoadMatrixf(mvp.m);
    }

    p_glBindVertexArray(I.batchVAO);
    p_glBindBuffer(GL_ARRAY_BUFFER, I.batchVBO);
    p_glBufferSubData(GL_ARRAY_BUFFER, 0,
                       static_cast<GLsizeiptr>(I.batchVertexCount * sizeof(BatchVertex)),
                       I.batchVertices);

    u32 indexCount = static_cast<u32>(I.batchIndices.size());
    p_glDrawElements(GL_TRIANGLES,
                      static_cast<GLsizei>(indexCount),
                      GL_UNSIGNED_INT, nullptr);

    I.batchVertexCount = 0;
    I.batchIndices.clear();
    I.drawCallCount++;
    I.triangleCount += indexCount / 3;
}

} // namespace ico::engine
