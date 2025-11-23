#pragma once
#include <stdint.h>
#include <poll.h>
#include <vector>
#include <netinet/in.h>
#include "connection.h"
#include "../utils/logger/logger.h"
#include "../command/command_dispatcher.h"

class Server
{
public:
    Server();
    ~Server();
    void run();

private:
    int fd;                            // 监听连接文件描述
    struct sockaddr_in addr = {};      // 服务器ip
    std::vector<Conntion *> conn_pool; // 连接池
    std::vector<pollfd> pollfd_args;   // poll数组

    CommandDispatcher cmdDisp; // 命令分发管理器

    Conntion *handle_accept();
};