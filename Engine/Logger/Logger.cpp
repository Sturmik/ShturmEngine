#include "Logger.h"

#include <iostream>
#include <cstdarg>
#include <cstdio>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#endif

Logger& Logger::Get()
{
    static Logger instance;
    return instance;
}

void Logger::Initialize(const std::string& filename)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _file.open(filename, std::ios::out | std::ios::trunc);
}

void Logger::Shutdown()
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (_file.is_open())
    {
        _file.close();
    }
}

void Logger::SetLevel(LogLevel level)
{
    _currentLevel = level;
}

void Logger::Log(LogLevel level,
    const char* fileName,
    int line,
    const char* function,
    const char* fmt, ...)
{
    if (level < _currentLevel)
    {
        return;
    }

    // Format user message (printf-style)
    char messageBuffer[2048];

    va_list args;
    va_start(args, fmt);
    vsnprintf(messageBuffer, sizeof(messageBuffer), fmt, args);
    va_end(args);

    // Build prefix (time + level + file info)
    std::string prefix = FormatPrefix(level, fileName, line, function);

    std::string finalMessage = prefix + messageBuffer;

    // Save log entry
    LogEntry logEntry = { level, finalMessage };
    _messages.emplace_back(logEntry);

    std::string coloredFinalMessage = finalMessage;

    // Make text green in case of information log
    if (level == LogLevel::Info)
    {
        coloredFinalMessage = "\033[32m" + finalMessage + "\033[0m";
    }
    // Make text yellow in case of warning level
    if (level == LogLevel::Warning)
    {
        coloredFinalMessage = "\033[33m" + finalMessage + "\033[0m";
    }
    // Make text red in case of error level
    if (level == LogLevel::Error)
    {
        coloredFinalMessage = "\033[31m" + finalMessage + "\033[0m";
    }
    std::cout << coloredFinalMessage << std::endl;

    // Thread-safe output
    std::lock_guard<std::mutex> lock(_mutex);

    if (_file.is_open())
    {
       _file << finalMessage << std::endl;
    }
}

const std::vector<LogEntry>& Logger::GetMessages() const
{
    return _messages;
}

Logger::~Logger()
{
    Shutdown();
}

std::string Logger::FormatPrefix(LogLevel level,
    const char* fileName,
    int line,
    const char* function)
{
    std::stringstream ss;

    // Time
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    time_t time = std::chrono::system_clock::to_time_t(now);

    std::tm localTime{};

#ifdef _WIN32
    localtime_s(&localTime, &time);
#else
    localtime_r(&time, &localTime);
#endif

    ss << "["
        << std::put_time(&localTime, "%H:%M:%S")
        << "]";

    // Level
    ss << "[" << LevelToString(level) << "]";

    // File:Line
    ss << "[" << ExtractFileName(fileName) << ":" << line << "]";

    // Function
    ss << "[" << function << "] ";

    return ss.str();
}

std::string Logger::LevelToString(LogLevel level)
{
    switch (level)
    {
        case LogLevel::Debug:   return "DEBUG";
        case LogLevel::Info:    return "INFO";
        case LogLevel::Warning: return "WARNING";
        case LogLevel::Error:   return "ERROR";
        default:                return "UNKNOWN";
    }
}

std::string Logger::ExtractFileName(const std::string& path)
{
    size_t pos = path.find_last_of("/\\");
    return (pos == std::string::npos) ? path : path.substr(pos + 1);
}