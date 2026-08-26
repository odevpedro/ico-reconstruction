#pragma once

#include "ps2/Ps2Types.h"

class Platform {
public:
    Platform();
    ~Platform();

    bool initialize();
    void shutdown();

    bool isInitialized() const;

private:
    bool m_initialized;
};
