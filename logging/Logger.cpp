#include "Logger.h"

#include <iostream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <iomanip>

namespace ansi {
    constexpr const char* reset   = "\033[0m";
    constexpr const char* green   = "\033[32m";
    constexpr const char* yellow  = "\033[33m";
    constexpr const char* blue    = "\033[34m";
    constexpr const char* red     = "\033[31m";
    constexpr const char* magenta = "\033[35m";
    constexpr const char* cyan    = "\033[36m";
}

const char* Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::TRACE:    return "TRACE";
        case LogLevel::DEBUG:    return "DEBUG";
        case LogLevel::INFO:     return "INFO";
        case LogLevel::WARN:     return "WARN";
        case LogLevel::ERROR:    return "ERROR";
        case LogLevel::CRITICAL: return "CRITICAL";
        default:                 return "UNKNOWN";
    }
}

const char* Logger::levelColor(LogLevel level) {
    switch (level) {
        case LogLevel::TRACE:    return ansi::cyan;
        case LogLevel::DEBUG:    return ansi::blue;
        case LogLevel::INFO:     return ansi::green;
        case LogLevel::WARN:     return ansi::yellow;
        case LogLevel::ERROR:    return ansi::red;
        case LogLevel::CRITICAL: return ansi::magenta;
        default:                 return ansi::reset;
    }
}

std::string Logger::iso8601Now() {
    using namespace std::chrono;
    auto now = system_clock::now();
    auto time = system_clock::to_time_t(now);
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

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

void Logger::trace(std::string className, std::string msg) {
    writeLog(LogLevel::TRACE, className, msg);
}

void Logger::debug(std::string className, std::string msg) {
    writeLog(LogLevel::DEBUG, className, msg);
}

void Logger::info(std::string className, std::string msg) {
    writeLog(LogLevel::INFO, className, msg);
}

void Logger::warn(std::string className, std::string msg) {
    writeLog(LogLevel::WARN, className, msg);
}

void Logger::error(std::string className, std::string msg) {
    writeLog(LogLevel::ERROR, className, msg);
}

void Logger::critical(std::string className, std::string msg) {
    writeLog(LogLevel::CRITICAL, className, msg);
}

void Logger::writeLog(LogLevel level, std::string className, std::string msg) {
    std::stringstream oss;
    oss << ansi::green << iso8601Now() << ansi::reset << " ";
    oss << levelColor(level)
        << std::left << std::setw(8) << levelToString(level)
        << ansi::reset << " ";
    oss << ansi::yellow << className << ansi::reset << ": ";
    oss << msg;
    std::cout << oss.str() << std::endl;
}
