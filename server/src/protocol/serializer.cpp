#include "serializer.h"
#include "../utils/logger/logger.h"

/*
enum class ResponseType
{
    SIMPLE_STRING,   // +OK\r\n
    ERROR,           // -Error message\r\n
    INTEGER,         // :123\r\n
    BULK_STRING,     // $5\r\nhello\r\n
    ARRAY,           // *2\r\n$5\r\nhello\r\n$5\r\nworld\r\n
    NULL_BULK_STRING // $-1\r\n
};
*/

std::string Serializer::serialize_simple_string(const std::string &str)
{
    return "+" + str + "\r\n";
}

std::string Serializer::serialize_error(const std::string &error_msg)
{
    return "-" + error_msg + "\r\n";
}

std::string Serializer::serialize_integer(int32_t value)
{
    return ":" + std::to_string(value) + "\r\n";
}

std::string Serializer::serialize_bulk_string(const std::string &str)
{
    return "$" + std::to_string(str.size()) + "\r\n" + str + "\r\n";
}

std::string Serializer::serialize_null_bulk_string()
{
    return "$-1\r\n";
}

std::string Serializer::serialize_array(const std::vector<std::string> &elements)
{
    std::string res = "*";
    res += std::to_string(elements.size()) + "\r\n";

    for (auto &item : elements)
    {
        res += "$";
        res += std::to_string(item.size()) + "\r\n" + item + "\r\n";
    }
    return res;
}

std::string Serializer::serialize_array(const std::vector<int32_t> &elements)
{
    std::string res = "*";
    res += std::to_string(elements.size()) + "\r\n";

    for (auto &item : elements)
    {
        res += serialize_integer(item);
    }
    return res;
}

std::string Serializer::serialize(const Response &response)
{
    switch (response.type)
    {
        case ResponseType::SIMPLE_STRING:
            return serialize_simple_string(response.simple_string);

        case ResponseType::ERROR:
            return serialize_error(response.simple_string);

        case ResponseType::INTEGER:
            return serialize_integer(response.integer);

        case ResponseType::BULK_STRING:
            return serialize_bulk_string(response.bulk_string);

        case ResponseType::ARRAY:
        {
            std::string res = "*";
            res += std::to_string(response.array.size()) + "\r\n";

            for (auto &item : response.array)
            {
                res += serialize(item);
            }
            return res;
        }
        case ResponseType::NULL_BULK_STRING:
            return serialize_null_bulk_string();
        default:
        {
            Logger::error("serialize() 响应数据类型未知");
            break;
        }
    }
    return "";
}

std::string Serializer::ok()
{
    return serialize_simple_string("ok");
}

std::string Serializer::nil()
{
    return serialize_null_bulk_string();
}

std::string Serializer::integer_result(int32_t value)
{
    return serialize_integer(value);
}

std::string Serializer::string_result(const std::string &str)
{
    return serialize_bulk_string(str);
}

std::string Serializer::error_result(const std::string &msg)
{
    return serialize_error(msg);
}

std::string Serializer::array_result(const std::vector<std::string> &items)
{
    return serialize_array(items);
}

std::string Serializer::array_result(const std::vector<int32_t> &items)
{
    return serialize_array(items);
}