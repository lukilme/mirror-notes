#pragma once

#include "ofMain.h"

#include <sstream>
#include <string>

class Logger {
public:
    enum class Level {
        Debug = 0,
        Info = 1,
        Warning = 2,
        Error = 3,
        Critical = 4
    };

    static Level& minimumLevel() {
        static Level level = Level::Info;
        return level;
    }

    static void setMinimumLevel(Level level) {
        minimumLevel() = level;
    }

    static bool shouldLog(Level level) {
        return static_cast<int>(level) >= static_cast<int>(minimumLevel());
    }

    static std::string colorize(const std::string& text, const std::string& colorCode) {
        return colorCode + text + "\033[0m";
    }

    static std::string levelLabel(Level level) {
        switch (level) {
            case Level::Info:     return "INFO";
            case Level::Debug:   return "DEBUG";
            case Level::Warning: return "WARNING";
            case Level::Error:   return "ERROR";
            case Level::Critical:return "CRITICAL";
        }
        return "INFO";
    }

    static std::string levelColor(Level level) {
        switch (level) {
            case Level::Info:     return "\033[32m"; // verde
            case Level::Debug:    return "\033[36m"; // ciano
            case Level::Warning:  return "\033[33m"; // amarelo
            case Level::Error:    return "\033[31m"; // vermelho
            case Level::Critical: return "\033[35m"; // magenta
        }
        return "\033[37m";
    }

    static void emit(Level level, const std::string& message) {
        if (!shouldLog(level)) {
            return;
        }

        std::cout << "[" << colorize(levelLabel(level), levelColor(level)) << "] "
                  << message << std::endl;
    }

    template<typename... Args>
    static void log(Level level, Args&&... args) {
        std::ostringstream oss;
        (oss << ... << std::forward<Args>(args));
        emit(level, oss.str());
    }

    template<typename... Args>
    static void info(Args&&... args) {
        log(Level::Info, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void debug(Args&&... args) {
        log(Level::Debug, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void warning(Args&&... args) {
        log(Level::Warning, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void error(Args&&... args) {
        log(Level::Error, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void critical(Args&&... args) {
        log(Level::Critical, std::forward<Args>(args)...);
    }
};
