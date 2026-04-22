#pragma once

#include "Logger.h"

#ifdef NDEBUG
#define LOG_DEBUG(fmt, ...)
#else
#define LOG_DEBUG(fmt, ...) \
        Logger::Get().Log(LogLevel::Debug, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#endif

#define LOG_INFO(fmt, ...) \
    Logger::Get().Log(LogLevel::Info, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)

#define LOG_WARNING(fmt, ...) \
    Logger::Get().Log(LogLevel::Warning, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)

#define LOG_ERROR(fmt, ...) \
    Logger::Get().Log(LogLevel::Error, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)