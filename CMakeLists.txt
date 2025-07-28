cmake_minimum_required(VERSION 3.16)

# 设置Qt路径 (根据您的实际安装路径)
set(QT_DIR "D:/softwareB/Qt/6.9.1/msvc2022_64")
set(CMAKE_PREFIX_PATH ${QT_DIR})

# 项目配置
project(ModbusQtTest1)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 开启Qt自动处理
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC OFF)  # 关闭资源编译

# 查找Qt组件
find_package(Qt6 REQUIRED COMPONENTS Core Widgets SerialBus)

# 添加可执行文件目标
add_executable(ModbusQtTest1
    ModbusQtTest1/main.cpp
    ModbusQtTest1/MainWindow.h
    ModbusQtTest1/MainWindow.cpp
    ModbusQtTest1/MainWindow.ui
)

# 链接Qt库
target_link_libraries(ModbusQtTest1 PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::SerialBus
)

# 仅用于Windows平台的配置
if(WIN32)
    # 设置输出目录
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/out/bin)
    set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/out/lib)
    
    # 禁用特定警告
    add_compile_options(/wd26812 /wd26495)
endif()