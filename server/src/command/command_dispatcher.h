#pragma once
#include "commands.h"
#include "../protocol/serializer.h"

struct validationResult
{
    bool isvalid;
    std::string error_msg;
};

class CommandDispatcher
{
private:
    CommandRegistry registry_;

public:
    CommandDispatcher();

    // 命令执行
    Response execute_command(const std::vector<std::string> &args);

protected:
    // 命令注册管理
    void register_commands();
    void regiser_command(const Command &cmd);

    validationResult validate_args(const Command &cmd, const std::vector<std::string> &args) const;

    // 错误响应生成
    Response make_error_response(const std::string &error_msg) const;

    // 命令处理器
    static Response handle_get(const std::vector<std::string> &args);
    static Response handle_set(const std::vector<std::string> &args);
    static Response handle_del(const std::vector<std::string> &args);

    static Response handle_zadd(const std::vector<std::string> &args);
    static Response handle_zrem(const std::vector<std::string> &args);
    static Response handle_zscore(const std::vector<std::string> &args);
    static Response handle_zrank(const std::vector<std::string> &args);
    static Response handle_zcard(const std::vector<std::string> &args);
    static Response handle_zrange(const std::vector<std::string> &args);
    static Response handle_zall(const std::vector<std::string> &args);
    static Response handle_zdel(const std::vector<std::string> &args);
};