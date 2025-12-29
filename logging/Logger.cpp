#include "Logger.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

//TODO: Selective log levels

namespace ansi {
    constexpr auto reset   = "\033[0m";
    constexpr auto green   = "\033[32m";
    constexpr auto yellow  = "\033[33m";
    constexpr auto blue    = "\033[34m";
    constexpr auto red     = "\033[31m";
    constexpr auto magenta = "\033[35m";
    constexpr auto cyan    = "\033[36m";
}

LogLevel Logger::checkLevel(const std::string& className) const {
    if (levels.contains(className)) {
        return levels.at(className);
    }
    return baseLevel;
}

const char* Logger::levelToString(const LogLevel level) {
    switch (level) {
        case TRACE:    return "TRACE";
        case DEBUG:    return "DEBUG";
        case INFO:     return "INFO";
        case WARN:     return "WARN";
        case ERROR:    return "ERROR";
        case CRITICAL: return "CRITICAL";
        default:                 return "UNKNOWN";
    }
}

const char* Logger::levelColor(const LogLevel level) {
    switch (level) {
        case TRACE:    return ansi::cyan;
        case DEBUG:    return ansi::blue;
        case INFO:     return ansi::green;
        case WARN:     return ansi::yellow;
        case ERROR:    return ansi::red;
        case CRITICAL: return ansi::magenta;
        default:                 return ansi::reset;
    }
}

std::string Logger::iso8601Now() {
    using namespace std::chrono;
    const auto now = system_clock::now();
    auto time = system_clock::to_time_t(now);
    const auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    std::tm tm{};
#if defined(_WIN32)
    gmtime_s(&tm, &time);
#else
    gmtime_r(&time, &tm);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S")
        << '.' << std::setw(3) << std::setfill('0') << ms.count()
        << 'Z';
    return oss.str();
}

Logger &Logger::instance() {
    static Logger l;
    return l;
}

void Logger::trace(const std::string& className, const std::string& msg) const {
    if (checkLevel(className) <= TRACE) {
        writeLog(TRACE, className, msg);
    }
}

void Logger::debug(const std::string& className, const std::string& msg) const {
    if (checkLevel(className) <= DEBUG) {
        writeLog(DEBUG, className, msg);
    }
}

void Logger::info(const std::string& className, const std::string& msg) const {
    if (checkLevel(className) <= INFO) {
        writeLog(INFO, className, msg);
    }
}

void Logger::warn(const std::string& className, const std::string& msg) const {
    if (checkLevel(className) <= WARN) {
        writeLog(WARN, className, msg);
    }
}

void Logger::error(const std::string& className, const std::string& msg) const {
    if (checkLevel(className) <= ERROR) {
        writeLog(ERROR, className, msg);
    }
}

void Logger::critical(const std::string& className, const std::string& msg) const {
    if (checkLevel(className) <= CRITICAL) {
        writeLog(CRITICAL, className, msg);
    }
}

Logger::Logger() {
    levels["GnssReader"] = TRACE;
}

void Logger::writeLog(const LogLevel level, const std::string& className, const std::string& msg) {
    std::stringstream oss;
    oss << ansi::green << iso8601Now() << ansi::reset << " ";
    oss << levelColor(level)
        << std::left << std::setw(8) << levelToString(level)
        << ansi::reset << " ";
    oss << ansi::yellow << className << ansi::reset << ": ";
    oss << msg;
    std::cout << oss.str() << std::endl;
}
