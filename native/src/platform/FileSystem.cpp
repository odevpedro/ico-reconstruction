#include "platform/FileSystem.h"
#include "runtime/Logger.h"

FileSystem::FileSystem()
    : m_initialized(false) {
}

FileSystem::~FileSystem() {
    shutdown();
}

bool FileSystem::initialize(const std::string& basePath) {
    if (m_initialized) {
        return true;
    }

    m_basePath = basePath;
    m_initialized = true;

    Logger::info("fs", "FileSystem stub initialized: base='%s'", m_basePath.c_str());
    return true;
}

void FileSystem::shutdown() {
    if (!m_initialized) {
        return;
    }

    m_initialized = false;
    Logger::info("fs", "FileSystem stub shutdown");
}

FILE* FileSystem::open(const char* path, const char* mode) {
    if (!m_initialized) return nullptr;

    std::string fullPath = m_basePath + "/" + path;
    FILE* file = std::fopen(fullPath.c_str(), mode);

    if (file) {
        Logger::debug("fs", "Opened: %s", fullPath.c_str());
    } else {
        Logger::debug("fs", "Failed to open: %s", fullPath.c_str());
    }

    return file;
}

void FileSystem::close(FILE* file) {
    if (file) {
        std::fclose(file);
    }
}

bool FileSystem::exists(const char* path) const {
    if (!m_initialized) return false;

    std::string fullPath = m_basePath + "/" + path;
    FILE* file = std::fopen(fullPath.c_str(), "r");
    if (file) {
        std::fclose(file);
        return true;
    }
    return false;
}

size_t FileSystem::size(const char* path) const {
    if (!m_initialized) return 0;

    std::string fullPath = m_basePath + "/" + path;
    FILE* file = std::fopen(fullPath.c_str(), "rb");
    if (!file) return 0;

    std::fseek(file, 0, SEEK_END);
    size_t sz = static_cast<size_t>(std::ftell(file));
    std::fclose(file);
    return sz;
}

const std::string& FileSystem::getBasePath() const {
    return m_basePath;
}
