#pragma once

#include <string>
#include <fstream>
#include <mutex>

enum class LogLevel
{
    Debug,
    Info,
    Warning,
    Error
};

class Logger
{
public:
    static Logger& Get();

    void Initialize(const std::string& filename = "ShturmEngine.log");
    void Shutdown();

    void SetLevel(LogLevel level);

    void Log(LogLevel level,
        const char* file,
        int line,
        const char* function,
        const char* fmt, ...);

private:
    Logger() = default;
    ~Logger() = default;

    std::string FormatPrefix(LogLevel level,
        const char* file,
        int line,
        const char* function);

    std::string LevelToString(LogLevel level);
    std::string ExtractFileName(const std::string& path);

    std::ofstream _file;
    std::mutex _mutex;
    LogLevel _currentLevel = LogLevel::Debug;
};