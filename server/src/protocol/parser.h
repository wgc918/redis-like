#pragma once
#include <vector>
#include <stdint.h>
#include <string>
#include "../utils/logger/logger.h"

/*数据格式*/
// +------+-----+------+-----+------+-----+-----+------+
// | nstr | len | str1 | len | str2 | ... | len | strn |
// +------+-----+------+-----+------+-----+-----+------+

class Parser
{
public:
    Parser(const uint8_t *request, uint32_t n);
    ~Parser();

    // 0表示解析成功 -1表示失败
    int32_t parser_req(std::vector<std::string> &res);

private:
    const uint8_t *cur;                 // 命令头指针
    const uint8_t *end;                 // 命令尾指针
    const uint32_t k_max_args = 1 << 5; // 最大命令数

    // 解析单个命令字长度
    bool parser_len(uint32_t &len);
    // 解析单个命令字
    bool parser_str(std::string &str, uint32_t len);
};