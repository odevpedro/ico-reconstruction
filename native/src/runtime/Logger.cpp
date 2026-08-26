#include "runtime/Logger.h"
#include <cstdio>
#include <cstdarg>

Logger::Level Logger::s_level = Logger::Level::Info;

void Logger::setLevel(Level level) {
    s_level = level;
}

Logger::Level Logger::getLevel() {
    return s_level;
}

void Logger::debug(const char* category, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log(Level::Debug, category, fmt, args);
    va_end(args);
}

void Logger::info(const char* category, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log(Level::Info, category, fmt, args);
    va_end(args);
}

void Logger::warn(const char* category, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log(Level::Warn, category, fmt, args);
    va_end(args);
}

void Logger::error(const char* category, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log(Level::Error, category, fmt, args);
    va_end(args);
}

void Logger::log(Level level, const char* category, const char* fmt, va_list args) {
    if (level < s_level) {
        return;
    }

    const char* levelStr = "";
    switch (level) {
        case Level::Debug: levelStr = "DEBUG"; break;
        case Level::Info:  levelStr = "INFO";  break;
        case Level::Warn:  levelStr = "WARN";  break;
        case Level::Error: levelStr = "ERROR"; break;
    }

    fprintf(stderr, "[%s] [%s] ", levelStr, category);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
}
