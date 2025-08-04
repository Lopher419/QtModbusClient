# QtModbusClient - Modbus RTU 客户端工具

## 概述

这是一个基于 Qt 6.9.1 开发的 Modbus RTU 客户端工具，用于与 Modbus 设备通信。项目实现了对四种 Modbus 数据类型的支持：
- 线圈 (Coils)
- 离散输入 (Discrete Inputs)
- 输入寄存器 (Input Registers)
- 保持寄存器 (Holding Registers)

开发初衷是探索 Qt 6 中 Modbus API 的变化，并提供一个简单实用的调试工具。
欢迎提交错误报告和做出贡献！

## 功能

### 1. 连接管理
- 串口参数配置（端口、波特率、数据位、校验位、停止位）
- 从站 ID 设置

### 2. 数据操作
- **线圈 (Coils)**
  - 单个线圈读写
  - 批量线圈读写（支持全选操作）

- **离散输入 (DI)**
  - 单个离散输入读取
  - 批量离散输入读取

- **输入寄存器 (IR)**
  - 单个寄存器读取
  - 批量寄存器读取

- **保持寄存器 (HR)**
  - 单个寄存器读写
  - 批量寄存器读写

### 3. 其他特性
- 较为详细的调试日志输出
- 线程安全的 Modbus 操作

## 构建说明

### 环境要求
- Qt 6.9.1+
- CMake 3.21+
- 支持 C++20 的编译器

### 构建步骤
```bash
mkdir build
cd build
cmake ..
cmake --build .
