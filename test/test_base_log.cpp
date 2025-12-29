/**
 * @file test_base_log.cpp
 * @brief Test base_log.h logging functionality
 */

#include "base_log.h"
#include <thread>
#include <chrono>

int main() {
    // Initialize logging system (console output only)
    snow::Logger::Init();
    
    // Test basic log output
    LOG_INFO << "This is an info log";
    LOG_WARN << "This is a warning log";
    LOG_ERR << "This is an error log";
    
    // Test stream output
    int value = 42;
    LOG_INFO << "Value: " << value << ", String: " << "test";
    
    // Test conditional logging
    bool condition = true;
    LOG_INFO_IF(condition) << "Log when condition is true";
    LOG_INFO_IF(!condition) << "This log will not output";
    
    // Test multi-threading
    std::thread t1([]() {
        LOG_INFO << "Log from thread 1";
    });
    
    std::thread t2([]() {
        LOG_INFO << "Log from thread 2";
    });
    
    t1.join();
    t2.join();
    
    // Test file logging
    snow::Logger::Shutdown();
    snow::Logger::Init("test.log", 1024 * 1024, 3);  // 1MB file, keep 3 files
    LOG_INFO << "This log will be written to file";
    
    // Cleanup
    snow::Logger::Flush();
    snow::Logger::Shutdown();
    
    return 0;
}

