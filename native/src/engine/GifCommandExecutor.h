#pragma once

#include "engine/GifCommandBuffer.h"
#include "engine/RenderBackend.h"

namespace ico::engine {

class GifCommandExecutor {
public:
    explicit GifCommandExecutor(RenderBackend& backend);

    void execute(const GifCommandBuffer& buffer);
    void executeCommand(const RenderCmd& cmd);

private:
    RenderBackend& m_backend;
};

} // namespace ico::engine
