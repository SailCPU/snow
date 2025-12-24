cmake_minimum_required(VERSION 3.10)
project(snow)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 创建统一的 snow 接口库，包含所有第三方库
add_library(snow INTERFACE)
add_library(snow::snow ALIAS snow)

# 设置包含目录（所有第三方库的头文件路径 + snow 自己的 include 目录）
target_include_directories(snow INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/eigen
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/spdlog/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

# 链接线程库（httplib 需要）
target_link_libraries(snow INTERFACE pthread)

