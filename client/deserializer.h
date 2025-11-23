#pragma once
#include <string>
#include <vector>
#include <stdexcept>
#include "../server/src/protocol/serializer.h" 

class Deserializer
{
private:
    std::string buffer_;
    size_t pos_;

    // 读取一行（以\r\n结尾）
    std::string read_line();

    // 跳过\r\n
    void skip_crlf();

    // 检查是否还有数据
    bool has_more_data() const;

public:
    Deserializer();

    // 重置解析状态
    void reset();

    // 设置要解析的数据
    void set_data(const std::string &data);

    // 追加数据（用于流式解析）
    void append_data(const std::string &data);

    // 主解析方法
    bool parse(Response &response);

    // 便捷方法：直接解析完整数据
    static Response parse(const std::string &data);

    // 获取剩余未解析的数据
    std::string get_remaining_data() const;

private:
    // 具体类型的解析方法
    bool parse_simple_string(Response &response);
    bool parse_error(Response &response);
    bool parse_integer(Response &response);
    bool parse_bulk_string(Response &response);
    bool parse_array(Response &response);
    bool parse_null_bulk_string(Response &response);
};


