/**
 * @file base_log.h
 * @brief 日志封装库，提供类似 glog 的日志接口
 * 
 * 使用示例：
 *   LOG_INFO << "这是一条信息日志";
 *   LOG_WARN << "这是一条警告日志";
 *   LOG_ERR << "这是一条错误日志";
 *   LOG_FATAL << "这是一条致命错误日志";
 */

#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <memory>
#include <sstream>
#include <string>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <iomanip>
#include <ctime>

namespace snow {

// 日志级别枚举，对应 glog 的级别
// 使用 LOG_LEVEL_ 前缀避免与 Windows 系统宏冲突
enum LogSeverity {
    LOG_LEVEL_INFO = 0,
    LOG_LEVEL_WARNING = 1,
    LOG_LEVEL_ERROR = 2,
    LOG_LEVEL_FATAL = 3
};

// 日志流类，用于流式输出
class LogMessage {
public:
    LogMessage(LogSeverity severity, const char* file, int line)
        : severity_(severity), file_(file), line_(line) {
    }

    ~LogMessage() {
        if (!message_.str().empty()) {
            Flush();
        }
        if (severity_ == LOG_LEVEL_FATAL) {
            std::abort();
        }
    }

    // 流式输出操作符
    template<typename T>
    LogMessage& operator<<(const T& value) {
        message_ << value;
        return *this;
    }

    // 支持 std::endl 等操作符
    LogMessage& operator<<(std::ostream& (*manip)(std::ostream&)) {
        message_ << manip;
        return *this;
    }

private:
    void Flush() {
        if (!logger_) {
            return;
        }

        // 获取当前时间
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::microseconds>(
            now.time_since_epoch()) % 1000000;
        
        std::tm tm_buf;
        #ifdef _WIN32
            localtime_s(&tm_buf, &time_t);
        #else
            localtime_r(&time_t, &tm_buf);
        #endif
        
        // 格式化日期时间：YYYYMMDD HH:MM:SS.uuuuuu
        char time_str[32];
        std::snprintf(time_str, sizeof(time_str), "%04d%02d%02d %02d:%02d:%02d.%06ld",
                     tm_buf.tm_year + 1900, tm_buf.tm_mon + 1, tm_buf.tm_mday,
                     tm_buf.tm_hour, tm_buf.tm_min, tm_buf.tm_sec, static_cast<long>(ms.count()));

        // 获取线程ID
        std::thread::id thread_id = std::this_thread::get_id();
        std::ostringstream thread_id_str;
        thread_id_str << thread_id;
        size_t thread_hash = std::hash<std::thread::id>{}(thread_id);

        // 级别字符
        char level_char;
        spdlog::level::level_enum spdlog_level;
        switch (severity_) {
            case LOG_LEVEL_INFO:
                level_char = 'I';
                spdlog_level = spdlog::level::info;
                break;
            case LOG_LEVEL_WARNING:
                level_char = 'W';
                spdlog_level = spdlog::level::warn;
                break;
            case LOG_LEVEL_ERROR:
                level_char = 'E';
                spdlog_level = spdlog::level::err;
                break;
            case LOG_LEVEL_FATAL:
                level_char = 'F';
                spdlog_level = spdlog::level::critical;
                break;
            default:
                level_char = '?';
                spdlog_level = spdlog::level::info;
        }

        // 提取文件名（不含路径）
        std::string filename = file_;
        size_t pos = filename.find_last_of("/\\");
        if (pos != std::string::npos) {
            filename = filename.substr(pos + 1);
        }

        // 构建 glog 风格格式：I20231224 09:30:45.123456 12345 file.cpp:123] message
        std::ostringstream formatted_msg;
        formatted_msg << level_char << time_str << " " 
                      << thread_hash << " " 
                      << filename << ":" << line_ << "] " 
                      << message_.str();
        
        // 使用 spdlog 记录（使用 raw 模式，因为格式已经完整）
        logger_->log(spdlog_level, formatted_msg.str());
    }

    LogSeverity severity_;
    const char* file_;
    int line_;
    std::ostringstream message_;
    
    static std::shared_ptr<spdlog::logger> logger_;
    friend class Logger;
};

// 日志管理器类
class Logger {
public:
    /**
     * @brief 初始化日志系统
     * @param log_file 日志文件路径（可选，为空则只输出到控制台）
     * @param max_file_size 单个日志文件最大大小（字节）
     * @param max_files 保留的日志文件数量
     * @param level 日志级别
     */
    static void Init(const std::string& log_file = "",
                     size_t max_file_size = 10 * 1024 * 1024,  // 10MB
                     size_t max_files = 5,
                     spdlog::level::level_enum level = spdlog::level::info) {
        std::vector<spdlog::sink_ptr> sinks;

        // 控制台输出（带颜色）
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        // 格式已经在 LogMessage::Flush() 中构建，这里只输出原始消息
        console_sink->set_pattern("%v");
        sinks.push_back(console_sink);

        // 文件输出（如果指定了日志文件）
        if (!log_file.empty()) {
            try {
                auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                    log_file, max_file_size, max_files);
                // 格式已经在 LogMessage::Flush() 中构建，这里只输出原始消息
                file_sink->set_pattern("%v");
                sinks.push_back(file_sink);
            } catch (const spdlog::spdlog_ex& ex) {
                // 如果文件创建失败，只使用控制台输出
                spdlog::warn("Failed to create log file: {}", ex.what());
            }
        }

        // 创建 logger
        LogMessage::logger_ = std::make_shared<spdlog::logger>("snow_logger", sinks.begin(), sinks.end());
        LogMessage::logger_->set_level(level);
        LogMessage::logger_->flush_on(spdlog::level::warn);  // WARNING 及以上级别立即刷新

        // 注册为默认 logger
        spdlog::register_logger(LogMessage::logger_);
        spdlog::set_default_logger(LogMessage::logger_);
    }

    /**
     * @brief 设置日志级别
     */
    static void SetLevel(spdlog::level::level_enum level) {
        if (LogMessage::logger_) {
            LogMessage::logger_->set_level(level);
        }
    }

    /**
     * @brief 刷新日志缓冲区
     */
    static void Flush() {
        if (LogMessage::logger_) {
            LogMessage::logger_->flush();
        }
    }

    /**
     * @brief 关闭日志系统
     */
    static void Shutdown() {
        if (LogMessage::logger_) {
            LogMessage::logger_->flush();
            spdlog::drop_all();
            LogMessage::logger_.reset();
        }
    }
};

// 静态成员初始化
std::shared_ptr<spdlog::logger> LogMessage::logger_ = nullptr;

} // namespace snow

// 日志宏定义，类似 glog 风格
#define LOG_INFO  snow::LogMessage(snow::LOG_LEVEL_INFO, __FILE__, __LINE__)
#define LOG_WARN  snow::LogMessage(snow::LOG_LEVEL_WARNING, __FILE__, __LINE__)
#define LOG_ERR   snow::LogMessage(snow::LOG_LEVEL_ERROR, __FILE__, __LINE__)
#define LOG_FATAL snow::LogMessage(snow::LOG_LEVEL_FATAL, __FILE__, __LINE__)

// 条件日志宏
#define LOG_INFO_IF(condition)  \
    !(condition) ? (void)0 : snow::LogMessage(snow::LOG_LEVEL_INFO, __FILE__, __LINE__)
#define LOG_WARN_IF(condition) \
    !(condition) ? (void)0 : snow::LogMessage(snow::LOG_LEVEL_WARNING, __FILE__, __LINE__)
#define LOG_ERR_IF(condition)   \
    !(condition) ? (void)0 : snow::LogMessage(snow::LOG_LEVEL_ERROR, __FILE__, __LINE__)
#define LOG_FATAL_IF(condition) \
    !(condition) ? (void)0 : snow::LogMessage(snow::LOG_LEVEL_FATAL, __FILE__, __LINE__)


