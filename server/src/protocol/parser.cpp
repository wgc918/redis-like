#include "parser.h"
#include <string.h>

Parser::Parser(const uint8_t *request, uint32_t n)
{
    this->cur = request;
    this->end = request + n;
}

Parser::~Parser()
{

}

int32_t Parser::parser_req(std::vector<std::string> &res)
{
    uint32_t nstr = 0;
    if (!parser_len(nstr))
        return -1;

    if (nstr > k_max_args)
    {
        Logger::error("parser_req() 命令长度大于k_max_args");
        return -1;
    }

    res.resize(nstr);
    for (int i = 0; i < nstr; i++)
    {
        uint32_t len = 0;
        if (!parser_len(len))
            return -1;
        if (!parser_str(res[i], len))
            return -1;
    }

    if (cur != end)
    {
        Logger::error("parser_req() 解析失败");
        return -1;
    }
    return 0;
}

bool Parser::parser_len(uint32_t &len)
{
    if (cur + 4 > end)
    {
        Logger::error("parser_len() 解析失败");
        return false;
    }
    memcpy(&len, cur, 4);
    cur += 4;
    return true;
}

bool Parser::parser_str(std::string &str, uint32_t len)
{
    if (cur + len > end)
    {
        Logger::error("parser_str() 解析失败");
        return false;
    }
    str.assign(cur, cur + len);
    cur += len;
    return true;
}
