#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <memory>

namespace platform {

enum class FileMode : uint8_t {
    Read,
    Write,
    ReadWrite
};

enum class SeekOrigin : uint8_t {
    Begin,
    Current,
    End
};

class File {
public:
    virtual ~File() = default;
    
    virtual size_t read(void* buffer, size_t size) = 0;
    virtual size_t write(const void* buffer, size_t size) = 0;
    virtual bool seek(int64_t offset, SeekOrigin origin) = 0;
    virtual int64_t tell() = 0;
    virtual bool eof() = 0;
    virtual size_t size() = 0;
    
    virtual void close() = 0;
};

class FileSystem {
public:
    virtual ~FileSystem() = default;
    
    virtual std::unique_ptr<File> open(const std::string& path, FileMode mode) = 0;
    virtual bool exists(const std::string& path) = 0;
    virtual size_t size(const std::string& path) = 0;
    virtual bool mkdir(const std::string& path) = 0;
    virtual bool rmdir(const std::string& path) = 0;
    virtual bool remove(const std::string& path) = 0;
    virtual std::vector<std::string> list(const std::string& path) = 0;
    
    virtual void setBasePath(const std::string& path) = 0;
    virtual std::string getBasePath() const = 0;
};

std::unique_ptr<FileSystem> createFileSystem(const std::string& basePath);

} // namespace platform
