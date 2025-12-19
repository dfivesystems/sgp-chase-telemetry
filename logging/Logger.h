#ifndef LOGGER_H
#define LOGGER_H
#include <string>

class Logger {
public:
    static Logger& instance();

    void trace(std::string className, std::string msg);
    void debug(std::string className, std::string msg);
    void info(std::string className, std::string msg);
    void warn(std::string className, std::string msg);
    void error(std::string className, std::string msg);
    void critical(std::string className, std::string msg);

private:
    Logger() {};
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    enum LogLevel {
        TRACE, DEBUG, INFO, WARN, ERROR, CRITICAL
    };

    const char *levelToString(::Logger::LogLevel level);
    const char *levelColor(::Logger::LogLevel level);
    std::string iso8601Now();

    void writeLog(LogLevel level, std::string className, std::string msg);
};

#endif //LOGGER_H
