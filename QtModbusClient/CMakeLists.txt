set(SOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Sources)
set(HEADERS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Headers)
set(UI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/UI)

file(GLOB SOURCES "${SOURCES_DIR}/*.cpp")
file(GLOB HEADERS "${HEADERS_DIR}/*.h")
file(GLOB FORMS "${UI_DIR}/*.ui")          

set(CMAKE_AUTOUIC_SEARCH_PATHS ${UI_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(QtModbusClient
    ${SOURCES}
    ${HEADERS}
    ${FORMS}
)

target_include_directories(QtModbusClient PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${HEADERS_DIR}
)

target_link_libraries(QtModbusClient PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::SerialBus
    Qt6::SerialPort
)