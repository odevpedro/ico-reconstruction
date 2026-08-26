#pragma once

#include "engine/RenderBackend.h"
#include "ps2/Ps2Types.h"

#include <vector>

namespace ico::engine {

struct Tm2Texture {
    u32 width = 0;
    u32 height = 0;
    TextureFormat format = TextureFormat::PSMCT32;
    std::vector<u8> rgbaData;
    bool hasAlpha = false;
};

} // namespace ico::engine
