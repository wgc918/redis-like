#include "connection.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <errno.h>
#include <assert.h>
#include "../utils/buffer/bufferPool.h"
#include <string.h>
#include "../protocol/parser.h"
#include "../protocol/serializer.h"
#include <iostream>

int Conntion::count = 0;

Conntion::Conntion(int fd)
{
    this->fd = fd;
    this->state.is_read = true;
    count++;
    this->uid = count;
}

Conntion::~Conntion()
{
}

void Conntion::handle_read(CommandDispatcher &cmdDisp)
{
    if (fd < 0)
    {
        Logger::fatal("handle_read() 连接id为" + std::to_string(uid) + "的连接fd有误");
        state.is_close = true;
        return;
    }

    uint8_t buf[64 * 1024];
    ssize_t rv = read(fd, buf, sizeof(buf));
    if (rv < 0)
    {
        if (errno == EAGAIN)
        {
            Logger::debug("handle_read() 连接id为" + std::to_string(uid) + "的连接系统调用read()被延误");
            return;
        }
        else
        {
            Logger::error("handle_read() 连接id为" + std::to_string(uid) + "的连接从内核缓冲区读数据发生错误");
            state.is_close = true;
            return;
        }
    }
    if (rv == 0)
    {
        if (read_buffer.size() == 0)
        {
            Logger::info("handle_read() 连接id为" + std::to_string(uid) + "的连接客户端断开连接");
            state.is_close = true;
            return;
        }
        else
        {
            Logger::error("handle_read() 连接id为" + std::to_string(uid) + "的连接意外EFO");
            state.is_close = true;
            return;
        }
    }
    bufferPool read_bufferPool(read_buffer);
    read_bufferPool.buffer_append(buf, (uint32_t)rv);
    Logger::debug("handle_read() 缓冲区大小：" + std::to_string(read_buffer.size()));
    Logger::debug("handle_read() 入读数据长度：" + std::to_string(rv));
    // Logger::debug("handle_read() 入读数据：" + std::string(buf, buf + rv));

    while (try_one_request(cmdDisp))
    {
    }

    if (write_buffer.size() > 0)
    {
        state.is_read = false;
        state.is_write = true;
        handle_write();
    }
}

void Conntion::handle_write()
{
    if (fd < 0)
    {
        Logger::fatal("handle_write() 连接id为" + std::to_string(uid) + "的连接fd有误");
        state.is_close = true;
        return;
    }

    if (write_buffer.size() == 0)
    {
        Logger::debug("handle_write() 连接id为" + std::to_string(uid) + "的连接写入缓冲区为空");
        return;
    }

    ssize_t rv = write(fd, write_buffer.data(), write_buffer.size());
    if (rv < 0)
    {
        if (errno == EAGAIN)
        {
            Logger::debug("handle_write() 连接id为" + std::to_string(uid) + "的连接系统调用write()被延误");
            return;
        }
        else
        {
            Logger::error("handle_write() 连接id为" + std::to_string(uid) + "的连接将数据写入内核缓冲区时发生错误");
            state.is_close = true;
            return;
        }
    }

    bufferPool write_bufferPool(write_buffer);
    write_bufferPool.buffer_consume((uint32_t)rv);

    if (write_buffer.size() == 0)
    {
        state.is_write = false;
        state.is_read = true;
    }
}

bool Conntion::try_one_request(CommandDispatcher &cmdDisp)
{
    if (fd < 0)
    {
        Logger::fatal("try_one_request() 连接id为" + std::to_string(uid) + "的连接fd有误");
        return false;
    }

    // 检查输入缓冲区数据是否足够
    if (read_buffer.size() < 4)
    {
        Logger::error("try_one_request() 连接id为" + std::to_string(uid) + "的连接输入缓冲区数据不够首部长度");
        return false;
    }
    const int k_max_msg = 32 << 20;
    uint32_t len;
    memcpy(&len, read_buffer.data(), 4);
    if (len > k_max_msg)
    {
        Logger::error("try_one_request() 连接id为" + std::to_string(uid) + "的连接数据长度超过最大长度");
        return false;
    }

    if (read_buffer.size() < len + 4)
    {
        Logger::error("try_one_request() 连接id为" + std::to_string(uid) + "的连接输入缓冲区数据不够请求体长度");
        return false;
    }

    uint8_t *request_data = read_buffer.data() + 4;
    bufferPool read_bufferPool(read_buffer);
    read_bufferPool.buffer_consume(len + 4);

    std::vector<std::string> args;
    Parser p(request_data, len);
    if (!p.parser_req(args))
    {
        Logger::debug("try_one_request() 命令解析成功");
        for (auto &a : args)
        {
            std::cout << a << " ";
        }
        std::cout << std::endl;
        // 执行命令生成响应
        Response resp = cmdDisp.execute_command(args);
        // 将响应序列化
        const std::string &res = Serializer::serialize(resp);
        uint32_t resp_len = res.size();
        // 将序列化后的响应写入缓冲
        bufferPool write_bufferPool(write_buffer);
        write_bufferPool.buffer_append((const uint8_t *)&resp_len, 4);
        write_bufferPool.buffer_append((const uint8_t *)res.data(), resp_len);
        return true;
    }
    return false;
}
