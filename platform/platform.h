#pragma once

#include "platform/fs.h"
#include "platform/time.h"
#include "platform/input.h"
#include "platform/render.h"
#include "platform/audio.h"

namespace platform {

class Platform {
public:
    virtual ~Platform() = default;
    
    virtual void initialize(const std::string& title, int width, int height, bool fullscreen) = 0;
    virtual void shutdown() = 0;
    
    virtual void update() = 0;
    virtual bool isRunning() const = 0;
    virtual void requestQuit() = 0;
    
    virtual FileSystem& getFileSystem() = 0;
    virtual GameClock& getClock() = 0;
    virtual Input& getInput() = 0;
    virtual Renderer& getRenderer() = 0;
    virtual Audio& getAudio() = 0;
    
    virtual void setTitle(const std::string& title) = 0;
    virtual void setIcon(const std::string& path) = 0;
};

std::unique_ptr<Platform> createPlatform();

} // namespace platform
