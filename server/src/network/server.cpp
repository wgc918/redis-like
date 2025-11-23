#include "server.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include "../utils/utils.h"
#include <string>
#include "../data_structures/global/globals.h"

HMap HMap_string = HMap();

Server::Server()
{
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(0);

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        Logger::fatal("Server() 监听fd无效");
    }

    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    int rv = bind(fd, (const sockaddr *)&addr, sizeof(addr));
    if (rv)
    {
        Logger::fatal("Server() 绑定fd失败");
    }

    // 设置监听套接字为非阻塞模式
    fd_set_nonblock(fd);

    rv = listen(fd, SOMAXCONN);
    if (rv)
    {
        Logger::fatal("Server() 监听失败");
    }
}

Server::~Server()
{
}

void Server::run()
{
    Logger::debug("Redis 服务器开始运行");
    while (true)
    {
        pollfd_args.clear();
        struct pollfd pfd = {fd, POLLIN, 0};
        // 监听fd位于poll数组起始位置
        pollfd_args.push_back(pfd);

        // 将旧连接加入poll数组以待重新轮询
        for (Conntion *conn : conn_pool)
        {
            if (!conn)
                continue;
            pfd = {conn->get_fd(), POLLERR, 0};
            State conn_state = conn->get_state();
            if (conn_state.is_read)
                pfd.events |= POLLIN;
            if (conn_state.is_write)
                pfd.events |= POLLOUT;
            pollfd_args.push_back(pfd);
        }

        int rv = poll(pollfd_args.data(), (nfds_t)pollfd_args.size(), -1);
        if (rv < 0 && errno == EINTR)
            continue;
        if (rv < 0)
            Logger::fatal("run() poll轮询失败");

        // 处理监听套接字
        if (pollfd_args[0].revents == POLLIN)
        {
            Conntion *conn = handle_accept();
            if (!conn)
                continue;
            if (conn_pool.size() <= conn->get_fd())
                conn_pool.resize(conn->get_fd() + 1);
            conn_pool[conn->get_fd()] = conn;
        }

        // 处理连接套接字
        for (size_t i = 1; i < pollfd_args.size(); i++)
        {
            uint32_t revents = pollfd_args[i].revents;
            if (revents == 0)
                continue;
            Conntion *conn = conn_pool[pollfd_args[i].fd];
            if (revents & POLLIN)
                conn->handle_read(cmdDisp);
            if (revents & POLLOUT)
                conn->handle_write();
            // 连接状态设置为关闭的逻辑有问题，不能实现客户端通过一个连接连续请求  先将其注释掉以实现客户端通过一个连接连续请求
            // 导致现在服务端无法正确处理客户端主动断开连接的情况
            if ((revents & POLLERR) /*|| conn->get_state().is_close*/)
            {
                close(conn->get_fd());
                Logger::debug("连接关闭 id为" + std::to_string(conn->get_id()) + " fd为" + std::to_string(conn->get_fd()));
                conn_pool[conn->get_fd()] = NULL;
                delete conn;
            }
        }
    }
}

Conntion *Server::handle_accept()
{
    struct sockaddr_in client_addr = {};
    socklen_t client_addr_len = sizeof(client_addr);
    int connfd = accept(fd, (sockaddr *)&client_addr, &client_addr_len);

    if (connfd < 0)
    {
        Logger::error("获取新连接fd失败");
        return NULL;
    }
    uint32_t ip = client_addr.sin_addr.s_addr;

    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, INET_ADDRSTRLEN);
    std::string ip_string(ip_str);
    std::string port_string = std::to_string(ntohs(client_addr.sin_port));

    Logger::debug("handle_accept() 新连接ip:" + ip_string + " 端口:" + port_string);

    // 设置新连接fd为非阻塞模式
    fd_set_nonblock(connfd);

    Conntion *conn = new Conntion(connfd);
    return conn;
}
