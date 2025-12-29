/**
 * Robot control system framework - Usage example
 * 
 * Demonstrates how to use third-party libraries for inter-process message passing
 */

#include <iostream>
#include <thread>
#include <chrono>

// Third-party libraries
#include <Eigen/Dense>
#include "nlohmann/json.hpp"
#include "httplib.h"
#include "base_log.h"  // Using encapsulated logging library
using json = nlohmann::json;
using namespace Eigen;

// Robot message structure
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

// HTTP server example (inter-process communication)
void start_http_server() {
    httplib::Server svr;
    
    RobotState current_state;
    current_state.position = Vector3d(0, 0, 0);
    current_state.velocity = Vector3d(0, 0, 0);
    current_state.timestamp = 0.0;
    
    // Get robot state
    svr.Get("/robot/state", [&current_state](const httplib::Request&, httplib::Response& res) {
        res.set_content(current_state.to_json().dump(), "application/json");
    });
    
    // Set robot command
    svr.Post("/robot/command", [&current_state](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = json::parse(req.body);
            std::string command = j["command"];
            
            LOG_INFO << "Received command: " << command;
            
            // Process command logic
            if (command == "move") {
                auto target = j["target"];
                current_state.position = Vector3d(target[0], target[1], target[2]);
                LOG_INFO << "Move to position: (" 
                    << current_state.position.x() << ", "
                    << current_state.position.y() << ", "
                    << current_state.position.z() << ")";
            }
            
            res.set_content(json{{"status", "ok"}}.dump(), "application/json");
        } catch (const std::exception& e) {
            LOG_ERR << "Failed to process command: " << e.what();
            res.status = 400;
            res.set_content(json{{"error", e.what()}}.dump(), "application/json");
        }
    });
    
    LOG_INFO << "HTTP server started on http://0.0.0.0:8080";
    svr.listen("0.0.0.0", 8080);
}

int main(int argc, char* argv[]) {
    // Initialize logging system
    snow::Logger::Init();
    LOG_INFO << "Robot control system framework starting";
    
    // Eigen usage example
    Vector3d position(1.0, 2.0, 3.0);
    Vector3d velocity(0.1, 0.2, 0.3);
    Vector3d acceleration = velocity * 0.1;
    
    LOG_INFO << "Position: (" << position.x() << ", " << position.y() << ", " << position.z() << ")";
    LOG_INFO << "Velocity: (" << velocity.x() << ", " << velocity.y() << ", " << velocity.z() << ")";
    
    // JSON usage example
    RobotState state;
    state.position = position;
    state.velocity = velocity;
    state.timestamp = std::chrono::duration<double>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    json j = state.to_json();
    LOG_INFO << "State JSON: " << j.dump(2);
    
    // Start HTTP server (for inter-process communication)
    start_http_server();
    
    // Cleanup logging system
    snow::Logger::Shutdown();
    return 0;
}

