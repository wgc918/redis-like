cmake_minimum_required(VERSION 3.12)
project(Test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# 查找所有源文件
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/command/*.cpp"
    "src/data_structures/*.cpp"
    "src/network/*.cpp"
    "src/protocol/*.cpp"
    "src/utils/*.cpp"
    "src/utils/buffer/*.cpp"
    "src/utils/logger/*.cpp"
)


message(STATUS "Sources: ${SOURCES}")

add_executable(test ${SOURCES})