#include "deserializer.h"
#include <iostream>
#include <sstream>
#include <cctype>

Deserializer::Deserializer() : pos_(0) {}

void Deserializer::reset()
{
    buffer_.clear();
    pos_ = 0;
}

void Deserializer::set_data(const std::string &data)
{
    buffer_ = data;
    pos_ = 0;
}

void Deserializer::append_data(const std::string &data)
{
    buffer_ += data;
}

std::string Deserializer::read_line()
{
    if (pos_ >= buffer_.size())
    {
        throw std::runtime_error("Unexpected end of data");
    }

    size_t end_pos = buffer_.find("\r\n", pos_);
    if (end_pos == std::string::npos)
    {
        throw std::runtime_error("Incomplete data: missing CRLF");
    }

    std::string line = buffer_.substr(pos_, end_pos - pos_);
    pos_ = end_pos + 2; // 跳过\r\n
    return line;
}

void Deserializer::skip_crlf()
{
    if (pos_ + 1 >= buffer_.size() ||
        buffer_[pos_] != '\r' ||
        buffer_[pos_ + 1] != '\n')
    {
        throw std::runtime_error("Expected CRLF");
    }
    pos_ += 2;
}

bool Deserializer::has_more_data() const
{
    return pos_ < buffer_.size();
}

std::string Deserializer::get_remaining_data() const
{
    if (pos_ >= buffer_.size())
    {
        return "";
    }
    return buffer_.substr(pos_);
}

bool Deserializer::parse(Response &response)
{
    if (!has_more_data())
    {
        return false;
    }

    try
    {
        char type_char = buffer_[pos_++];

        switch (type_char)
        {
        case '+':
            return parse_simple_string(response);
        case '-':
            return parse_error(response);
        case ':':
            return parse_integer(response);
        case '$':
            return parse_bulk_string(response);
        case '*':
            return parse_array(response);
        default:
            throw std::runtime_error("Unknown response type: " + std::string(1, type_char));
        }
    }
    catch (const std::exception &e)
    {
        // 重置位置到解析开始前，以便重试
        pos_ = 0;
        throw;
    }
}

bool Deserializer::parse_simple_string(Response &response)
{
    std::string line = read_line();
    response.type = ResponseType::SIMPLE_STRING;
    response.simple_string = line;
    response.is_null = false;
    return true;
}

bool Deserializer::parse_error(Response &response)
{
    std::string line = read_line();
    response.type = ResponseType::ERROR;
    response.simple_string = line;
    response.is_null = false;
    return true;
}

bool Deserializer::parse_integer(Response &response)
{
    std::string line = read_line();
    try
    {
        response.type = ResponseType::INTEGER;
        response.integer = std::stoll(line);
        response.is_null = false;
        return true;
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error("Invalid integer format: " + line);
    }
}

bool Deserializer::parse_bulk_string(Response &response)
{
    std::string length_str = read_line();

    if (length_str == "-1")
    {
        response.type = ResponseType::NULL_BULK_STRING;
        response.is_null = true;
        response.bulk_string.clear();
        return true;
    }

    try
    {
        int64_t length = std::stoll(length_str);
        if (length < 0)
        {
            throw std::runtime_error("Invalid bulk string length: " + length_str);
        }

        // 检查是否有足够的数据
        if (pos_ + length + 2 > buffer_.size())
        {
            // 数据不完整，回退位置到类型字符之前
            pos_ -= (length_str.length() + 3); // 回退到 '$' 之前
            return false;
        }

        response.type = ResponseType::BULK_STRING;
        response.bulk_string = buffer_.substr(pos_, length);
        response.is_null = false;
        pos_ += length;
        skip_crlf(); // 跳过结尾的\r\n

        return true;
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error("Invalid bulk string length: " + length_str);
    }
}

bool Deserializer::parse_array(Response &response)
{
    std::string length_str = read_line();

    if (length_str == "-1")
    {
        // NULL array (在Redis中，nil数组用*-1\r\n表示)
        response.type = ResponseType::ARRAY;
        response.array.clear();
        response.is_null = true;
        return true;
    }

    try
    {
        int64_t length = std::stoll(length_str);
        if (length < 0)
        {
            throw std::runtime_error("Invalid array length: " + length_str);
        }

        response.type = ResponseType::ARRAY;
        response.array.clear();
        response.is_null = false;

        // 保存当前位置，以便在解析失败时回退
        size_t saved_pos = pos_;

        // 递归解析数组元素
        for (int64_t i = 0; i < length; ++i)
        {
            Response element;
            if (!parse(element))
            {
                // 数据不完整，回退到数组开始位置
                pos_ = saved_pos - (length_str.length() + 3); // 回退到 '*' 之前
                return false;
            }
            response.array.push_back(element);
        }

        return true;
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error("Invalid array length: " + length_str);
    }
}

Response Deserializer::parse(const std::string &data)
{
    Deserializer deserializer;
    deserializer.set_data(data);
    Response response;
    if (!deserializer.parse(response))
    {
        throw std::runtime_error("Failed to parse RESP data");
    }

    // 检查是否解析了所有数据
    if (deserializer.has_more_data())
    {
        std::cout << "Warning: Extra data remaining after parsing: "
                  << deserializer.get_remaining_data() << std::endl;
    }

    return response;
}