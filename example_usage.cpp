/**
 * 机器人控制系统框架 - 使用示例
 * 
 * 演示如何使用第三方库进行跨进程消息传递
 */

#include <iostream>
#include <thread>
#include <chrono>

// 第三方库
#include <Eigen/Dense>
#include "nlohmann/json.hpp"
#include "httplib.h"
#include "base_log.h"  // 使用封装的日志库
using json = nlohmann::json;
using namespace Eigen;

// 机器人消息结构
struct RobotState {
    Vector3d position;
    Vector3d velocity;
    double timestamp;
    
    json to_json() const {
        return json{
            {"position", {position.x(), position.y(), position.z()}},
            {"velocity", {velocity.x(), velocity.y(), velocity.z()}},
            {"timestamp", timestamp}
        };
    }
    
    static RobotState from_json(const json& j) {
        RobotState state;
        auto pos = j["position"];
        auto vel = j["velocity"];
        state.position = Vector3d(pos[0], pos[1], pos[2]);
        state.velocity = Vector3d(vel[0], vel[1], vel[2]);
        state.timestamp = j["timestamp"];
        return state;
    }
};

// HTTP 服务器示例（跨进程通信）
void start_http_server() {
    httplib::Server svr;
    
    RobotState current_state;
    current_state.position = Vector3d(0, 0, 0);
    current_state.velocity = Vector3d(0, 0, 0);
    current_state.timestamp = 0.0;
    
    // 获取机器人状态
    svr.Get("/robot/state", [&current_state](const httplib::Request&, httplib::Response& res) {
        res.set_content(current_state.to_json().dump(), "application/json");
    });
    
    // 设置机器人命令
    svr.Post("/robot/command", [&current_state](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = json::parse(req.body);
            std::string command = j["command"];
            
            LOG_INFO << "收到命令: " << command;
            
            // 处理命令逻辑
            if (command == "move") {
                auto target = j["target"];
                current_state.position = Vector3d(target[0], target[1], target[2]);
                LOG_INFO << "移动到位置: (" 
                    << current_state.position.x() << ", "
                    << current_state.position.y() << ", "
                    << current_state.position.z() << ")";
            }
            
            res.set_content(json{{"status", "ok"}}.dump(), "application/json");
        } catch (const std::exception& e) {
            LOG_ERR << "处理命令失败: " << e.what();
            res.status = 400;
            res.set_content(json{{"error", e.what()}}.dump(), "application/json");
        }
    });
    
    LOG_INFO << "HTTP 服务器启动在 http://0.0.0.0:8080";
    svr.listen("0.0.0.0", 8080);
}

int main(int argc, char* argv[]) {
    // 初始化日志系统
    snow::Logger::Init();
    LOG_INFO << "机器人控制系统框架启动";
    
    // Eigen 使用示例
    Vector3d position(1.0, 2.0, 3.0);
    Vector3d velocity(0.1, 0.2, 0.3);
    Vector3d acceleration = velocity * 0.1;
    
    LOG_INFO << "位置: (" << position.x() << ", " << position.y() << ", " << position.z() << ")";
    LOG_INFO << "速度: (" << velocity.x() << ", " << velocity.y() << ", " << velocity.z() << ")";
    
    // JSON 使用示例
    RobotState state;
    state.position = position;
    state.velocity = velocity;
    state.timestamp = std::chrono::duration<double>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    json j = state.to_json();
    LOG_INFO << "状态 JSON: " << j.dump(2);
    
    // 启动 HTTP 服务器（用于跨进程通信）
    start_http_server();
    
    // 清理日志系统
    snow::Logger::Shutdown();
    return 0;
}

