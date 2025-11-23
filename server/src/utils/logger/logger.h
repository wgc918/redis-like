#pragma once
#include <chrono>
#include <string>
#include <fstream>

enum class LogLevel
{
    DEBUG,   // 调试信息
    INFO,    // 一般信息
    WARNING, // 警告信息
    ERROR,   // 错误信息
    FATAL    // 严重错误
};

struct LogConfig
{
    LogLevel level = LogLevel::DEBUG;
    bool output_to_console = true;
    size_t max_file_size = 10 * 1024 * 1024;
};


class Logger
{
private:
    static std::fstream out;
    static LogConfig config;
    static std::string current_filename;

public:
    // 初始化配置
    static void init(const LogConfig &cfg);
    static void shutdown();

    // 不同级别的日志方法
    static void debug(const std::string &message);
    static void info(const std::string &message);
    static void warning(const std::string &message);
    static void error(const std::string &message);
    static void fatal(const std::string &message);

private:
    static void log(LogLevel level, const std::string &message);
    static bool shouldLog(LogLevel level);
    static std::string levelToString(LogLevel level);

    // 工具方法
    static std::string getFileName();
    static void ensureFileOpen();
};