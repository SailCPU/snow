# 第三方库说明

本目录包含机器人控制系统框架所需的 header-only 第三方库。

## 包含的库

### 1. Eigen (线性代数库)
- **位置**: `eigen/`
- **用途**: 矩阵运算、向量计算、机器人运动学/动力学计算
- **使用**: `#include <Eigen/Dense>` 或 `#include <Eigen/Core>`
- **编译选项**: 需要添加 `-I third_party/eigen` 到编译选项

### 2. nlohmann/json (JSON 解析库)
- **位置**: `nlohmann/json.hpp`
- **用途**: JSON 数据序列化/反序列化，用于跨进程消息传递
- **使用**: `#include "nlohmann/json.hpp"`
- **命名空间**: `using json = nlohmann::json;`

### 3. cpp-httplib (HTTP 服务器/客户端库)
- **位置**: `httplib.h`
- **用途**: HTTP 通信，用于跨进程 RESTful API 接口
- **使用**: `#include "httplib.h"`
- **依赖**: 需要链接 `-lpthread` (Linux) 或 `-pthread` (某些系统)

### 4. doctest (单元测试框架)
- **位置**: `doctest.h`
- **用途**: 单元测试和集成测试
- **使用**: `#include "doctest.h"`
- **定义**: 在某个源文件中定义 `#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN` 然后包含头文件

### 5. spdlog (日志库)
- **位置**: `spdlog/include/spdlog/`
- **用途**: 高性能日志记录
- **使用**: `#include <spdlog/spdlog.h>`
- **编译选项**: 需要添加 `-I third_party/spdlog/include` 到编译选项

## 编译选项示例

```bash
# GCC/Clang 编译示例
g++ -std=c++17 \
    -I third_party/eigen \
    -I third_party/spdlog/include \
    -I third_party \
    -pthread \
    your_source.cpp -o your_program
```

## CMakeLists.txt 示例

```cmake
cmake_minimum_required(VERSION 3.10)
project(RobotControlFramework)

set(CMAKE_CXX_STANDARD 17)

# 包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/third_party/eigen
    ${CMAKE_SOURCE_DIR}/third_party/spdlog/include
    ${CMAKE_SOURCE_DIR}/third_party
)

# 添加可执行文件
add_executable(robot_control main.cpp)

# 链接线程库（httplib 需要）
target_link_libraries(robot_control pthread)
```

## 使用示例

### 跨进程消息传递示例

```cpp
#include "httplib.h"
#include "nlohmann/json.hpp"
#include <spdlog/spdlog.h>

using json = nlohmann::json;

// 消息结构
struct RobotMessage {
    std::string command;
    double position[3];
    double velocity[3];
};

// 序列化
json to_json(const RobotMessage& msg) {
    return json{
        {"command", msg.command},
        {"position", msg.position},
        {"velocity", msg.velocity}
    };
}

// HTTP 服务器示例
int main() {
    spdlog::info("启动机器人控制系统...");
    
    httplib::Server svr;
    
    svr.Post("/robot/command", [](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = json::parse(req.body);
            RobotMessage msg;
            msg.command = j["command"];
            // ... 处理消息
            
            res.set_content(to_json(msg).dump(), "application/json");
        } catch (const std::exception& e) {
            spdlog::error("处理消息失败: {}", e.what());
            res.status = 400;
        }
    });
    
    svr.listen("0.0.0.0", 8080);
    return 0;
}
```

