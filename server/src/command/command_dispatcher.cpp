#include "command_dispatcher.h"
#include "../utils/logger/logger.h"
#include "../data_structures/global/globals.h"
#include "../data_structures/string.h"
#include "../data_structures/zset.h"
#include <iostream>

CommandDispatcher::CommandDispatcher()
{
    // 注册所有命令
    register_commands();
    Logger::debug("CommandDispatcher()  注册所有命令成功");
}

void CommandDispatcher::register_commands()
{
    registry_.clear();
    // get
    regiser_command(Command("GET", CommandType::GET, 2, 2, "GET key", &CommandDispatcher::handle_get));

    // set
    regiser_command(Command("SET", CommandType::SET, 3, 3, "SET key value", &CommandDispatcher::handle_set));

    // del
    regiser_command(Command("DEL", CommandType::DEL, 2, 2, "DEL key", &CommandDispatcher::handle_del));

    // zadd
    regiser_command(Command("ZADD", CommandType::ZADD, 4, 4, "ZADD key score member", &CommandDispatcher::handle_zadd));

    // zrem
    regiser_command(Command("ZREM", CommandType::ZREM, 3, 3, "ZREM key member", &CommandDispatcher::handle_zrem));

    // zscore
    regiser_command(Command("ZSCORE", CommandType::ZSCORE, 3, 3, "ZSCORE key member", &CommandDispatcher::handle_zscore));

    // zrank
    regiser_command(Command("ZRANK", CommandType::ZRANK, 3, 3, "ZRANK key member", &CommandDispatcher::handle_zrank));

    // zcard
    regiser_command(Command("ZCARD", CommandType::ZCARD, 2, 2, "ZCARD key", &CommandDispatcher::handle_zcard));

    // zrange
    regiser_command(Command("ZRANGE", CommandType::ZRANGE, 4, 4, "ZRANGE key min max", &CommandDispatcher::handle_zrange));

    // zall
    regiser_command(Command("ZALL", CommandType::ZALL, 2, 2, "ZALL key", &CommandDispatcher::handle_zall));

    //zdel
    regiser_command(Command("ZDEL", CommandType::ZDEL, 2, 2, "ZDEL key", &CommandDispatcher::handle_zdel));
}

void CommandDispatcher::regiser_command(const Command &cmd)
{
    registry_[cmd.name] = cmd;
}

Response CommandDispatcher::execute_command(const std::vector<std::string> &args)
{
    const std::string &name = args[0];
    auto it = registry_.find(name);
    if (it == registry_.end())
    {
        Logger::error("execute_command() 未知命令:" + name);
        return make_error_response("未知命令 '" + name + "'");
    }

    const Command &cmd = it->second;
    validationResult res = validate_args(cmd, args);
    if (!res.isvalid)
    {
        Logger::error("execute_command() 命令参数数量不对 '" + name + "'");
        return make_error_response(res.error_msg);
    }

    try
    {
        return cmd.handler(args);
    }
    catch (const std::exception &e)
    {
        Logger::error("execute_command() 命令执行出错 '" + name + "':" + e.what());
        return make_error_response(e.what());
    }
}

validationResult CommandDispatcher::validate_args(const Command &cmd, const std::vector<std::string> &args) const
{
    int32_t cmd_num = args.size();
    if (cmd_num < cmd.min_args || cmd_num > cmd.max_args)
        return {false, "'" + cmd.name + "'命令参数数量不对"};
    return {true, ""};
}

Response CommandDispatcher::make_error_response(const std::string &error_msg) const
{
    Response resp;
    resp.type = ResponseType::SIMPLE_STRING;
    resp.simple_string = error_msg;
    return resp;
}

Response CommandDispatcher::handle_get(const std::vector<std::string> &args)
{
    StringEntry _entry(args[1]);
    const std::string &value = _entry.get(HMap_string);
    // std::cout << value << std::endl;
    Response resp;
    resp.type = ResponseType::BULK_STRING;
    resp.bulk_string = value;
    return resp;
}

Response CommandDispatcher::handle_set(const std::vector<std::string> &args)
{
    StringEntry _entry(args[1], args[2]);
    _entry.set(HMap_string);
    Response resp;
    resp.type = ResponseType::SIMPLE_STRING;
    resp.simple_string = "OK";
    return resp;
}

Response CommandDispatcher::handle_del(const std::vector<std::string> &args)
{
    StringEntry _entry(args[1]);
    bool success = _entry.del(HMap_string);
    Response resp;
    resp.type = ResponseType::INTEGER;
    resp.integer = success ? 1 : -1;
    return resp;
}
// ZADD key score member
Response CommandDispatcher::handle_zadd(const std::vector<std::string> &args)
{
    // 首先获取顶级哈希表中的集合对象节点值指针
    Zset node(args[1]);
    Value *value = node.exsit(HMap_string);
    if (!value)
        value = node.create(HMap_string);

    ZsetEntry _entry(std::stod(args[2]), args[3]);
    bool success = _entry.zadd(value);

    Response resp;
    resp.type = ResponseType::INTEGER;
    resp.integer = success ? 1 : -1;

    return resp;
}
// ZREM key member
Response CommandDispatcher::handle_zrem(const std::vector<std::string> &args)
{
    // 首先获取顶级哈希表中的集合对象节点值指针
    Zset node(args[1]);
    Value *value = node.exsit(HMap_string);
    if (!value)
        value = node.create(HMap_string);
    ZsetEntry _entry(args[2]);
    bool success = _entry.zrem(value);

    Response resp;
    resp.type = ResponseType::INTEGER;
    resp.integer = success ? 1 : -1;

    return resp;
}
// ZSCORE key member
Response CommandDispatcher::handle_zscore(const std::vector<std::string> &args)
{
    Zset node(args[1]);
    Value *value = node.exsit(HMap_string);
    if (!value)
        value = node.create(HMap_string);

    ZsetEntry _entry(args[2]);
    bool success = false;
    double score = _entry.zscore(value, success);

    Response resp;

    if (success)
    {
        resp.type = ResponseType::SIMPLE_STRING;
        resp.simple_string = std::to_string(score);
    }
    else
    {
        resp.type = ResponseType::ERROR;
        resp.simple_string = "nil()";
    }

    return resp;
}
// ZRANK key member
Response CommandDispatcher::handle_zrank(const std::vector<std::string> &args)
{
    Zset node(args[1]);
    Value *value = node.exsit(HMap_string);
    if (!value)
        value = node.create(HMap_string);

    ZsetEntry _entry(args[2]);
    bool success = false;
    int rank = _entry.zrank(value, success);

    Response resp;

    if (success)
    {
        resp.type = ResponseType::INTEGER;
        resp.integer = rank;
    }
    else
    {
        resp.type = ResponseType::ERROR;
        resp.simple_string = "nil()";
    }
    return resp;
}
// ZCARD key
Response CommandDispatcher::handle_zcard(const std::vector<std::string> &args)
{
    Zset node(args[1]);
    Value *value = node.exsit(HMap_string);
    if (!value)
        value = node.create(HMap_string);

    ZsetEntry _entry("");
    int num = _entry.zcard(value);

    Response resp;
    resp.type = ResponseType::INTEGER;
    resp.integer = num;
    return resp;
}
// ZRANGE key min max
Response CommandDispatcher::handle_zrange(const std::vector<std::string> &args)
{
    Zset node(args[1]);
    Value *value = node.exsit(HMap_string);
    if (!value)
        value = node.create(HMap_string);

    ZsetEntry _entry("");
    std::vector<std::pair<std::string, double>> result = _entry.zrange(value, std::stoi(args[2]), std::stoi(args[3]));
    // std::cout << "-------" << std::endl;
    std::vector<Response> resp_result;
    Response resp_name, resp_score;
    for (auto &item : result)
    {
        resp_name.type = ResponseType::BULK_STRING;
        resp_name.bulk_string = item.first;
        resp_result.push_back(resp_name);

        resp_score.type = ResponseType::SIMPLE_STRING;
        resp_score.simple_string = std::to_string(item.second);
        resp_result.push_back(resp_score);
    }
    Response resp;
    resp.type = ResponseType::ARRAY;
    resp.array = resp_result;
    return resp;
}

Response CommandDispatcher::handle_zall(const std::vector<std::string> &args)
{
    Zset node(args[1]);
    Value *value = node.exsit(HMap_string);
    if (!value)
        value = node.create(HMap_string);

    ZsetEntry _entry("");
    std::vector<std::pair<std::string, double>> result = _entry.zall(value);
    std::vector<Response> resp_result;
    Response resp_name, resp_score;
    for (auto &item : result)
    {
        resp_name.type = ResponseType::BULK_STRING;
        resp_name.bulk_string = item.first;
        resp_result.push_back(resp_name);

        resp_score.type = ResponseType::SIMPLE_STRING;
        resp_score.simple_string = std::to_string(item.second);
        resp_result.push_back(resp_score);
    }
    Response resp;
    resp.type = ResponseType::ARRAY;
    resp.array = resp_result;
    return resp;
}

Response CommandDispatcher::handle_zdel(const std::vector<std::string> &args)
{
    Zset node(args[1]);
    bool success = node.zdel(HMap_string);
    Response resp;

    if (success)
    {
        resp.type = ResponseType::SIMPLE_STRING;
        resp.simple_string = "成功";
    }
    else
    {
        resp.type = ResponseType::ERROR;
        resp.simple_string = "nil()";
    }
    return resp;
}
