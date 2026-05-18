#include "Logger.h"

#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <utility>

namespace {
std::string CurrentTimeString()
{
    std::time_t now = std::time(nullptr);
    std::tm localTime = {};
    localtime_s(&localTime, &now);

    char buffer[32] = {};
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &localTime);
    return buffer;
}

class ConsoleLogSink : public ILogSink
{
public:
    void Write(LogLevel level, const std::string& message) override
    {
        std::printf("%s %s %s\n", CurrentTimeString().c_str(), LevelToString(level), message.c_str());
        std::fflush(stdout);
    }

private:
    static const char* LevelToString(LogLevel level)
    {
        switch (level) {
        case LogLevel::Debug: return "[DEBUG]";
        case LogLevel::Info: return "[INFO ]";
        case LogLevel::Warning: return "[WARN ]";
        case LogLevel::Error: return "[ERROR]";
        default: return "[UNKWN]";
        }
    }
};
}

Logger& Logger::Get()
{
    static Logger instance;
    return instance;
}

Logger::Logger()
{
    AddSink(std::make_unique<ConsoleLogSink>());
}

void Logger::Debug(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    WriteFormatted(LogLevel::Debug, format, args);
    va_end(args);
}

void Logger::Info(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    WriteFormatted(LogLevel::Info, format, args);
    va_end(args);
}

void Logger::Warning(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    WriteFormatted(LogLevel::Warning, format, args);
    va_end(args);
}

void Logger::Error(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    WriteFormatted(LogLevel::Error, format, args);
    va_end(args);
}

void Logger::AddSink(std::unique_ptr<ILogSink> sink)
{
    if (!sink) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex);
    sinks.push_back(std::move(sink));
}

void Logger::ClearSinks()
{
    std::lock_guard<std::mutex> lock(mutex);
    sinks.clear();
}

void Logger::Write(LogLevel level, const std::string& message)
{
    std::lock_guard<std::mutex> lock(mutex);
    for (const auto& sink : sinks) {
        sink->Write(level, message);
    }
}

void Logger::WriteFormatted(LogLevel level, const char* format, va_list args)
{
    if (format == nullptr) {
        return;
    }

    char buffer[1024] = {};
    vsnprintf(buffer, sizeof(buffer), format, args);
    Get().Write(level, buffer);
}
