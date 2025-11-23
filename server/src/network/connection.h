#pragma once
#include <vector>
#include <stdint.h>
#include "../utils/logger/logger.h"
#include"../command/command_dispatcher.h"

struct State
{
    bool is_read;
    bool is_write;
    bool is_close;
};

class Conntion
{
private:
    int fd; // 连接对应的文件描述
    std::vector<uint8_t> read_buffer;
    std::vector<uint8_t> write_buffer;

    State state;
    int uid;          // 每个连接的唯一id
    static int count; // 记录连接数量

public:
    Conntion(int fd);
    ~Conntion();

    void handle_read( CommandDispatcher& cmdDisp);
    void handle_write();
    bool try_one_request( CommandDispatcher &cmdDisp);

    int get_fd() { return fd; }
    State get_state() { return state; }
    int get_id() { return uid; }
};