# 声明可执行文件
add_executable(ModbusQtTest1
    modbusmanager.h
    modbusmanager.cpp
    CoilTabWidget.cpp
    CoilTabWidget.h
    MainWindow.cpp
    MainWindow.h
    main.cpp
)

# 显式设置C++标准 (兼容旧CMake版本)
if(CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET ModbusQtTest1 PROPERTY CXX_STANDARD 17)
endif()

# 链接Qt库
target_link_libraries(ModbusQtTest1 PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::SerialBus
)

# 可选：安装规则
# install(TARGETS ModbusQtTest1 RUNTIME DESTINATION bin)