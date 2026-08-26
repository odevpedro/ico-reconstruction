#pragma once

#include "ps2/Ps2Types.h"
#include <string>
#include <cstdio>

class FileSystem {
public:
    FileSystem();
    ~FileSystem();

    bool initialize(const std::string& basePath = ".");
    void shutdown();

    FILE* open(const char* path, const char* mode);
    void close(FILE* file);

    bool exists(const char* path) const;
    size_t size(const char* path) const;

    const std::string& getBasePath() const;

private:
    std::string m_basePath;
    bool m_initialized;
};
