/**
 * @file test_base_log.cpp
 * @brief 测试 base_log.h 日志功能
 */

#include "base_log.h"
#include <thread>
#include <chrono>

int main() {
    // 初始化日志系统（只输出到控制台）
    snow::Logger::Init();
    
    // 测试基本日志输出
    LOG_INFO << "这是一条信息日志";
    LOG_WARN << "这是一条警告日志";
    LOG_ERR << "这是一条错误日志";
    
    // 测试流式输出
    int value = 42;
    LOG_INFO << "数值: " << value << ", 字符串: " << "test";
    
    // 测试条件日志
    bool condition = true;
    LOG_INFO_IF(condition) << "条件为真时的日志";
    LOG_INFO_IF(!condition) << "这条日志不会输出";
    
    // 测试多线程
    std::thread t1([]() {
        LOG_INFO << "来自线程1的日志";
    });
    
    std::thread t2([]() {
        LOG_INFO << "来自线程2的日志";
    });
    
    t1.join();
    t2.join();
    
    // 测试文件日志
    snow::Logger::Shutdown();
    snow::Logger::Init("test.log", 1024 * 1024, 3);  // 1MB 文件，保留3个
    LOG_INFO << "这条日志会写入文件";
    
    // 清理
    snow::Logger::Flush();
    snow::Logger::Shutdown();
    
    return 0;
}

