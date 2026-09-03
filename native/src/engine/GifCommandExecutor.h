#pragma once

#include "engine/GifCommandBuffer.h"
#include "engine/RenderBackend.h"

#include <unordered_map>

namespace ico::engine {

class GifCommandExecutor {
public:
    explicit GifCommandExecutor(RenderBackend& backend);

    void execute(const GifCommandBuffer& buffer);
    void executeCommand(const RenderCmd& cmd);

private:
    /* Resolves the buffer's virtual texture handles to real backend textures,
       creating and binding them (cached) as needed. */
    void resolveTextureHandles(RenderCmd& cmd, const GifCommandBuffer& buffer);

    RenderBackend& m_backend;
    std::unordered_map<TextureHandle, TextureHandle> m_virtualToReal;
};

} // namespace ico::engine
