#pragma once
#include <string>
#include <vector>

enum class ResponseType
{
    SIMPLE_STRING,   // +OK\r\n
    ERROR,           // -Error message\r\n
    INTEGER,         // :123\r\n
    BULK_STRING,     // $5\r\nhello\r\n
    ARRAY,           // *2\r\n$5\r\nhello\r\n$5\r\nworld\r\n
    NULL_BULK_STRING // $-1\r\n
};

struct Response
{
    ResponseType type;
    std::string simple_string;   // 用于SIMPLE_STRING和ERROR
    int64_t integer;             // 用于INTEGER
    std::string bulk_string;     // 用于BULK_STRING
    std::vector<Response> array; // 用于ARRAY
    bool is_null;                // 用于NULL类型
};

class Serializer
{
private:
    // 基础类型序列化
    static std::string serialize_simple_string(const std::string &str);
    static std::string serialize_error(const std::string &error_msg);
    static std::string serialize_integer(int32_t value);
    static std::string serialize_bulk_string(const std::string &str);
    static std::string serialize_null_bulk_string();
    static std::string serialize_array(const std::vector<std::string> &elements);
    static std::string serialize_array(const std::vector<int32_t> &elements);

public:
    // 通用序列化方法
    static std::string serialize(const Response &response);

    // 便捷方法（对应Redis命令响应）
    static std::string ok();
    static std::string nil();
    static std::string integer_result(int32_t value);
    static std::string string_result(const std::string &str);
    static std::string error_result(const std::string &msg);
    static std::string array_result(const std::vector<std::string> &items);
    static std::string array_result(const std::vector<int32_t> &items);
};