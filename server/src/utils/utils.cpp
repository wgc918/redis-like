#include "utils.h"
#include<errno.h>
#include<fcntl.h>
#include"../utils/logger/logger.h"

void fd_set_nonblock(int fd)
{
    errno = 0;
    int flags = fcntl(fd, F_GETFL, 0);
    if(errno)
    {
        Logger::fatal("fd_set_nonblock() 设置fd为非阻塞模式失败");
        return;
    }

    flags |= O_NONBLOCK;

    errno = 0;
    fcntl(fd, F_SETFL, flags);
    if(errno)
    {
        Logger::fatal("fd_set_nonblock() 设置fd为非阻塞模式失败");
    }
}