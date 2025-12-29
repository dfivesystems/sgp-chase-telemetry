#ifndef LOGGER_H
#define LOGGER_H
#include <map>
#include <string>

enum LogLevel {
    TRACE, DEBUG, INFO, WARN, ERROR, CRITICAL
};

class Logger {
public:
    static Logger& instance();

    void trace(const std::string& className, const std::string& msg) const;
    void debug(const std::string& className, const std::string& msg) const;
    void info(const std::string& className, const std::string& msg) const;
    void warn(const std::string& className, const std::string& msg) const;
    void error(const std::string& className, const std::string& msg) const;
    void critical(const std::string& className, const std::string& msg) const;

private:
    Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    LogLevel baseLevel = INFO;
    std::map<std::string, LogLevel> levels;

    [[nodiscard]] LogLevel checkLevel(const std::string& className) const;

    static const char *levelToString(LogLevel level);
    static const char *levelColor(LogLevel level);
    static std::string iso8601Now();

    static void writeLog(LogLevel level, const std::string& className, const std::string& msg);
};

#endif //LOGGER_H
