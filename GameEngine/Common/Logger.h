#pragma once

#include <cstdarg>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

enum class LogLevel
{
    Debug,
    Info,
    Warning,
    Error
};

class ILogSink
{
public:
    virtual ~ILogSink() = default;
    virtual void Write(LogLevel level, const std::string& message) = 0;
};

class Logger
{
public:
    static Logger& Get();

    static void Debug(const char* format, ...);
    static void Info(const char* format, ...);
    static void Warning(const char* format, ...);
    static void Error(const char* format, ...);

    void AddSink(std::unique_ptr<ILogSink> sink);
    void ClearSinks();
    void Write(LogLevel level, const std::string& message);

private:
    Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    static void WriteFormatted(LogLevel level, const char* format, va_list args);

    std::vector<std::unique_ptr<ILogSink>> sinks;
    std::mutex mutex;
};
