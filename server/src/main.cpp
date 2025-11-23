#include "./utils/logger/logger.h"
#include <iostream>
#include <string>
#include <chrono>
#include "network/server.h"

using namespace std;

int main()
{
    cout << "开始" << endl;
    LogConfig config;
    config.level = LogLevel::DEBUG;
    config.output_to_console = true;
    config.max_file_size = 10 * 1024 * 1024;

    Logger::init(config);

    Server server;
    server.run();

    Logger::shutdown();

    return 0;
}