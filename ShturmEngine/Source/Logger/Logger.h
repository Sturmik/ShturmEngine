#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <vector>

enum class LogLevel
{
    Debug,
    Info,
    Warning,
    Error
};

struct LogEntry
{
    LogLevel logLevel;
    std::string message;
};

class Logger
{
public:
    static Logger& Get();

    void Initialize(const std::string& filename = "ShturmEngineLog.txt");
    void Shutdown();

    void SetLevel(LogLevel level);

    void Log(LogLevel level,
        const char* file,
        int line,
        const char* function,
        const char* fmt, ...);

    const std::vector<LogEntry>& GetMessages() const;

private:
    Logger() = default;
    ~Logger();

    std::string FormatPrefix(LogLevel level,
        const char* file,
        int line,
        const char* function);

    std::string LevelToString(LogLevel level);
    std::string ExtractFileName(const std::string& path);

    std::ofstream _file;
    std::mutex _mutex;
    LogLevel _currentLevel = LogLevel::Debug;

    std::vector<LogEntry> _messages;
};