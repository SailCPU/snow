/**
 * @file base_log.h
 * @brief Logging wrapper library, providing glog-like logging interface
 * 
 * Usage example:
 *   LOG_INFO << "This is an info log";
 *   LOG_WARN << "This is a warning log";
 *   LOG_ERR << "This is an error log";
 *   LOG_FATAL << "This is a fatal error log";
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

// Log level enumeration, corresponding to glog levels
// Use LOG_LEVEL_ prefix to avoid conflicts with Windows system macros (e.g., ERROR, INFO, etc.)
enum LogSeverity {
    LOG_LEVEL_INFO = 0,
    LOG_LEVEL_WARNING = 1,
    LOG_LEVEL_ERROR = 2,
    LOG_LEVEL_FATAL = 3
};

// Log stream class for stream output
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

    // Stream output operator
    template<typename T>
    LogMessage& operator<<(const T& value) {
        message_ << value;
        return *this;
    }

    // Support std::endl and other manipulators
    LogMessage& operator<<(std::ostream& (*manip)(std::ostream&)) {
        message_ << manip;
        return *this;
    }

private:
    void Flush() {
        if (!logger_) {
            return;
        }

        // Get current time
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
        
        // Format date time: YYYYMMDD HH:MM:SS.uuuuuu
        char time_str[32];
        std::snprintf(time_str, sizeof(time_str), "%04d%02d%02d %02d:%02d:%02d.%06ld",
                     tm_buf.tm_year + 1900, tm_buf.tm_mon + 1, tm_buf.tm_mday,
                     tm_buf.tm_hour, tm_buf.tm_min, tm_buf.tm_sec, static_cast<long>(ms.count()));

        // Get thread ID
        std::thread::id thread_id = std::this_thread::get_id();
        std::ostringstream thread_id_str;
        thread_id_str << thread_id;
        size_t thread_hash = std::hash<std::thread::id>{}(thread_id);

        // Level character
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

        // Extract filename (without path)
        std::string filename = file_;
        size_t pos = filename.find_last_of("/\\");
        if (pos != std::string::npos) {
            filename = filename.substr(pos + 1);
        }

        // Build glog-style format: I20231224 09:30:45.123456 12345 file.cpp:123] message
        std::ostringstream formatted_msg;
        formatted_msg << level_char << time_str << " " 
                      << thread_hash << " " 
                      << filename << ":" << line_ << "] " 
                      << message_.str();
        
        // Use spdlog to record (use raw mode, format is already complete)
        logger_->log(spdlog_level, formatted_msg.str());
    }

    LogSeverity severity_;
    const char* file_;
    int line_;
    std::ostringstream message_;
    
    static std::shared_ptr<spdlog::logger> logger_;
    friend class Logger;
};

// Log manager class
class Logger {
public:
    /**
     * @brief Initialize logging system
     * @param log_file Log file path (optional, empty means console only)
     * @param max_file_size Maximum size of a single log file (bytes)
     * @param max_files Number of log files to keep
     * @param level Log level
     */
    static void Init(const std::string& log_file = "",
                     size_t max_file_size = 10 * 1024 * 1024,  // 10MB
                     size_t max_files = 5,
                     spdlog::level::level_enum level = spdlog::level::info) {
        std::vector<spdlog::sink_ptr> sinks;

        // Console output (with color)
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        // Format is already built in LogMessage::Flush(), here only output raw message
        console_sink->set_pattern("%v");
        sinks.push_back(console_sink);

        // File output (if log file is specified)
        if (!log_file.empty()) {
            try {
                auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                    log_file, max_file_size, max_files);
                // Format is already built in LogMessage::Flush(), here only output raw message
                file_sink->set_pattern("%v");
                sinks.push_back(file_sink);
            } catch (const spdlog::spdlog_ex& ex) {
                // If file creation fails, use console output only
                spdlog::warn("Failed to create log file: {}", ex.what());
            }
        }

        // Create logger
        LogMessage::logger_ = std::make_shared<spdlog::logger>("snow_logger", sinks.begin(), sinks.end());
        LogMessage::logger_->set_level(level);
        LogMessage::logger_->flush_on(spdlog::level::warn);  // WARNING and above levels flush immediately

        // Register as default logger
        spdlog::register_logger(LogMessage::logger_);
        spdlog::set_default_logger(LogMessage::logger_);
    }

    /**
     * @brief Set log level
     */
    static void SetLevel(spdlog::level::level_enum level) {
        if (LogMessage::logger_) {
            LogMessage::logger_->set_level(level);
        }
    }

    /**
     * @brief Flush log buffer
     */
    static void Flush() {
        if (LogMessage::logger_) {
            LogMessage::logger_->flush();
        }
    }

    /**
     * @brief Shutdown logging system
     */
    static void Shutdown() {
        if (LogMessage::logger_) {
            LogMessage::logger_->flush();
            spdlog::drop_all();
            LogMessage::logger_.reset();
        }
    }
};

// Static member initialization
std::shared_ptr<spdlog::logger> LogMessage::logger_ = nullptr;

} // namespace snow

// Log macro definitions, similar to glog style
// Note: Macro names remain LOG_INFO, LOG_WARN, etc., but internally use prefixed enum values to avoid conflicts
#define LOG_INFO  snow::LogMessage(snow::LOG_LEVEL_INFO, __FILE__, __LINE__)
#define LOG_WARN  snow::LogMessage(snow::LOG_LEVEL_WARNING, __FILE__, __LINE__)
#define LOG_ERR   snow::LogMessage(snow::LOG_LEVEL_ERROR, __FILE__, __LINE__)
#define LOG_FATAL snow::LogMessage(snow::LOG_LEVEL_FATAL, __FILE__, __LINE__)

// Conditional log macros
#define LOG_INFO_IF(condition)  \
    !(condition) ? (void)0 : snow::LogMessage(snow::LOG_LEVEL_INFO, __FILE__, __LINE__)
#define LOG_WARN_IF(condition) \
    !(condition) ? (void)0 : snow::LogMessage(snow::LOG_LEVEL_WARNING, __FILE__, __LINE__)
#define LOG_ERR_IF(condition)   \
    !(condition) ? (void)0 : snow::LogMessage(snow::LOG_LEVEL_ERROR, __FILE__, __LINE__)
#define LOG_FATAL_IF(condition) \
    !(condition) ? (void)0 : snow::LogMessage(snow::LOG_LEVEL_FATAL, __FILE__, __LINE__)


