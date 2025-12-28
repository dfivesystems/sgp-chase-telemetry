#ifndef LOGGER_H
#define LOGGER_H
#include <string>

class Logger {
public:
    static Logger& instance();

    static void trace(const std::string& className, const std::string& msg);
    static void debug(const std::string& className, const std::string& msg);
    static void info(const std::string& className, const std::string& msg);
    static void warn(const std::string& className, const std::string& msg);
    static void error(const std::string& className, const std::string& msg);
    static void critical(const std::string& className, const std::string& msg);

private:
    Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    enum LogLevel {
        TRACE, DEBUG, INFO, WARN, ERROR, CRITICAL
    };

    static const char *levelToString(::Logger::LogLevel level);
    static const char *levelColor(::Logger::LogLevel level);
    static std::string iso8601Now();

    static void writeLog(LogLevel level, const std::string& className, const std::string& msg);
};

#endif //LOGGER_H
