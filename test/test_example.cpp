/**
 * 单元测试示例
 * 
 * 编译: g++ -std=c++17 -I third_party/eigen -I third_party/spdlog/include -I third_party test_example.cpp -o test_example
 * 运行: ./test_example
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include <Eigen/Dense>
#include "nlohmann/json.hpp"

using json = nlohmann::json;
using namespace Eigen;

// 机器人状态结构（简化版，用于测试）
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

TEST_CASE("RobotState JSON 序列化") {
    RobotState state;
    state.position = Vector3d(1.0, 2.0, 3.0);
    state.velocity = Vector3d(0.1, 0.2, 0.3);
    state.timestamp = 1234.56;
    
    json j = state.to_json();
    RobotState restored = RobotState::from_json(j);
    
    CHECK(restored.position.isApprox(state.position));
    CHECK(restored.velocity.isApprox(state.velocity));
    CHECK(restored.timestamp == state.timestamp);
}

TEST_CASE("Eigen 矩阵运算") {
    Matrix3d rotation = AngleAxisd(M_PI/4, Vector3d::UnitZ()).toRotationMatrix();
    Vector3d point(1, 0, 0);
    Vector3d rotated = rotation * point;
    
    CHECK(rotated.norm() == doctest::Approx(1.0));
    CHECK(rotated.x() == doctest::Approx(std::sqrt(2.0)/2.0));
    CHECK(rotated.y() == doctest::Approx(std::sqrt(2.0)/2.0));
}

TEST_CASE("JSON 基本操作") {
    json j = {
        {"command", "move"},
        {"target", {1.0, 2.0, 3.0}},
        {"speed", 0.5}
    };
    
    CHECK(j["command"] == "move");
    CHECK(j["target"][0] == 1.0);
    CHECK(j["speed"] == 0.5);
    
    std::string json_str = j.dump();
    json parsed = json::parse(json_str);
    CHECK(parsed["command"] == "move");
}

