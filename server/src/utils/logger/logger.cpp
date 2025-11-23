#include "logger.h"
#include <iomanip>
#include <iostream>
#include <sstream>

LogConfig CFG;

std::fstream Logger::out;
LogConfig Logger::config;
std::string Logger::current_filename;

void Logger::init(const LogConfig &cfg)
{
    config = cfg;
    ensureFileOpen();
}

void Logger::shutdown()
{
    if (out.is_open())
    {
        out.close();
    }
}

bool Logger::shouldLog(LogLevel level)
{
    return static_cast<int>(level) >= static_cast<int>(config.level);
}

std::string Logger::levelToString(LogLevel level)
{
    switch (level)
    {
    case LogLevel::DEBUG:
        return "DEBUG";
    case LogLevel::INFO:
        return "INFO";
    case LogLevel::WARNING:
        return "WARNING";
    case LogLevel::ERROR:
        return "ERROR";
    case LogLevel::FATAL:
        return "FATAL";
    default:
        return "UNKNOWN";
    }
}

std::string Logger::getFileName()
{
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    tm *timeinfo = std::localtime(&time_t);

    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "redis_%Y_%m_%d.log", timeinfo);
    //这里使用的相对路径 根据可执行文件的路径
    return "../src/utils/log/" + std::string(buffer);
}

void Logger::ensureFileOpen()
{
    std::string new_filename = getFileName();
    if (new_filename != current_filename)
    {
        if (out.is_open())
        {
            out.close();
        }
        out.open(new_filename, std::ios::app | std::ios::out);
        current_filename = new_filename;
        std::cout << "文件名" << current_filename << std::endl;
        if(out.is_open())
        {
            std::cout << "成功打开文件" << std::endl;
        }
    }
}

void Logger::log(LogLevel level, const std::string &message)
{
    if (!shouldLog(level))
    {
        return;
    }

    ensureFileOpen();

    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);

    std::string level_str = levelToString(level);
    std::stringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    std::string timestamp = oss.str();

    std::string log_entry = "[" + timestamp + "] [" + level_str + "]: " + message + "\n";

    if (config.output_to_console)
    {
        std::cout << log_entry;
    }

    if (out.is_open())
    {
        out << log_entry;
        out.flush(); // 确保日志及时写入文件
    }
}

void Logger::debug(const std::string &message) { log(LogLevel::DEBUG, message); }
void Logger::info(const std::string &message) { log(LogLevel::INFO, message); }
void Logger::warning(const std::string &message) { log(LogLevel::WARNING, message); }
void Logger::error(const std::string &message) { log(LogLevel::ERROR, message); }
void Logger::fatal(const std::string &message) { log(LogLevel::FATAL, message); }