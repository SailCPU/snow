# Snow - 机器人信息交换库

Snow 是一个轻量级的机器人信息交换库，提供跨进程通信、日志记录和基础工具功能，作为机器人系统的基础组件。

## 项目结构

```
snow/
├── include/             # 项目头文件
│   └── base_log.h      # 日志封装库（类似 glog 接口）
├── test/               # 测试文件
│   ├── test_base_log.cpp    # 日志功能测试
│   └── test_example.cpp     # 单元测试示例
├── third_party/        # 第三方 header-only 库
│   ├── eigen/         # Eigen 线性代数库
│   ├── nlohmann/      # nlohmann/json JSON 库
│   ├── httplib.h      # cpp-httplib HTTP 库
│   ├── doctest.h      # doctest 测试框架
│   ├── spdlog/        # spdlog 日志库
│   └── README.md      # 第三方库详细说明
├── example_usage.cpp   # 使用示例（HTTP 服务器）
├── CMakeLists.txt      # CMake 构建配置
└── README.md          # 本文件
```

## 快速开始

### 编译示例程序

```bash
# 使用 CMake
mkdir build && cd build
cmake ..
make

# 或直接使用 g++
g++ -std=c++17 \
    -I include \
    -I third_party/eigen \
    -I third_party/spdlog/include \
    -I third_party \
    -pthread \
    example_usage.cpp -o robot_control
```

### 运行示例

```bash
./robot_control
```

示例服务器将在 `http://0.0.0.0:8080` 启动，演示如何使用 Snow 进行信息交换。

### 测试信息交换 API

```bash
# 获取机器人状态信息
curl http://localhost:8080/robot/state

# 发送控制命令（JSON格式）
curl -X POST http://localhost:8080/robot/command \
  -H "Content-Type: application/json" \
  -d '{"command":"move","target":[1.0,2.0,3.0]}'
```

### 运行单元测试

```bash
# 编译测试文件
g++ -std=c++17 \
    -I include \
    -I third_party/eigen \
    -I third_party/spdlog/include \
    -I third_party \
    test/test_example.cpp -o test_example

# 运行测试
./test_example
```

## 功能特性

Snow 作为机器人信息交换的基础库，提供以下核心功能：

### 信息交换能力
- ✅ **HTTP RESTful API**: 基于 cpp-httplib 的跨进程通信支持
- ✅ **JSON 消息格式**: 使用 nlohmann/json 进行数据序列化/反序列化
- ✅ **跨语言兼容**: 标准 HTTP + JSON 协议，支持不同语言和进程间通信

### 基础工具库
- ✅ **日志系统**: 基于 spdlog 封装的日志库（base_log.h），提供类似 glog 的简洁接口
  - 支持流式输出：`LOG_INFO << "message"`
  - 日志格式参考 glog：`I20231224 09:30:45.123456 12345 file.cpp:123] message`
  - 支持控制台和文件输出
  - 线程安全
- ✅ **数学计算**: Eigen 线性代数库，用于机器人相关的数学运算
- ✅ **测试框架**: doctest 轻量级单元测试框架

## 作为信息交换库的使用

Snow 提供了基础的 HTTP 通信能力，可以用于：

- **机器人状态查询**: 通过 HTTP GET 请求获取机器人状态信息
- **命令发送**: 通过 HTTP POST 请求发送控制命令
- **数据交换**: 使用 JSON 格式在进程间传递结构化数据

消息格式使用 JSON，便于不同语言和进程间交换数据。

## 日志使用

### 基本使用

```cpp
#include "base_log.h"

int main() {
    // 初始化日志系统（只输出到控制台）
    snow::Logger::Init();
    
    // 基本日志输出
    LOG_INFO << "这是一条信息日志";
    LOG_WARN << "这是一条警告日志";
    LOG_ERR << "这是一条错误日志";
    LOG_FATAL << "这是一条致命错误日志（会终止程序）";
    
    // 流式输出
    int value = 42;
    LOG_INFO << "数值: " << value << ", 字符串: " << "test";
    
    // 条件日志
    bool condition = true;
    LOG_INFO_IF(condition) << "条件为真时的日志";
    LOG_ERR_IF(!condition) << "这条日志不会输出";
    
    // 文件日志
    snow::Logger::Init("app.log", 10*1024*1024, 5);  // 10MB文件，保留5个
    
    snow::Logger::Shutdown();
    return 0;
}
```

### 日志级别

- `LOG_INFO`: 信息日志
- `LOG_WARN`: 警告日志
- `LOG_ERR`: 错误日志
- `LOG_FATAL`: 致命错误（会调用 `std::abort()` 终止程序）

### 日志格式

日志输出格式参考 glog，包含以下信息：
- 级别字符（I/W/E/F）
- 日期时间（YYYYMMDD HH:MM:SS.uuuuuu）
- 线程ID
- 文件名:行号
- 日志消息

示例输出：
```
I20231224 09:30:45.123456 12345 main.cpp:42] 这是一条信息日志
```

## 设计理念

Snow 旨在作为机器人系统的基础信息交换库：

- **轻量级**: 所有第三方库均为 header-only，无需额外编译步骤
- **易集成**: 提供统一的 CMake 接口，方便集成到现有项目
- **跨平台**: 基于标准 C++17，支持 Linux、Windows、macOS
- **可扩展**: 作为基础库，可以在此基础上构建更复杂的机器人应用

## 依赖说明

所有第三方库都是 header-only 的，无需额外编译：
- **Eigen**: 线性代数库，用于数学计算
- **nlohmann/json**: JSON 序列化/反序列化库
- **cpp-httplib**: HTTP 服务器/客户端库（需要 pthread）
- **doctest**: 轻量级单元测试框架
- **spdlog**: 高性能日志库

详细说明请查看 `third_party/README.md`。

## 许可证

各第三方库的许可证请参考各自的源文件。

