#pragma once

#include "ps2/Ps2Types.h"
#include <cstdio>
#include <cstdarg>

class Logger {
public:
    enum class Level : u8 {
        Debug,
        Info,
        Warn,
        Error
    };

    static void setLevel(Level level);
    static Level getLevel();

    static void debug(const char* category, const char* fmt, ...);
    static void info(const char* category, const char* fmt, ...);
    static void warn(const char* category, const char* fmt, ...);
    static void error(const char* category, const char* fmt, ...);

private:
    static void log(Level level, const char* category, const char* fmt, va_list args);
    static Level s_level;
};
