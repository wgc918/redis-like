#pragma once
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>
#include "../protocol/serializer.h"

using CommandHandler = std::function<Response(const std::vector<std::string> &)>;

enum class CommandType
{
    // String
    GET,
    SET,
    DEL,
    // Zset
    ZADD,
    ZREM,
    ZSCORE, // 获取指定元素的分数
    ZRANK,  // 获取指定元素排名
    ZCARD,  // 获取集合元素个数
    ZRANGE,  // 获取指定排名范围内的元素
    ZALL,
    ZDEL
};

struct Command
{
    std::string name;       // 命令名称
    CommandType type;       // 命令类型
    int min_args;           // 最小参数个数（包含命令本身）
    int max_args;           // 最大参数个数（-1表示不限）
    std::string syntax;     // 语法说明
    CommandHandler handler; // 处理函数

    Command() : name(""), type(CommandType::GET), min_args(0), max_args(0), syntax(""), handler(nullptr) {}

    Command(const std::string &name, CommandType type, int min, int max, const std::string &syntax, CommandHandler handler) : name(name), type(type), min_args(min), max_args(max), syntax(syntax), handler(handler) {}
};

// 命令注册表类型
using CommandRegistry = std::unordered_map<std::string, Command>;