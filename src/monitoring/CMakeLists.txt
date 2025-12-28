# 系统监控模块

# 源文件列表
set(MONITORING_SOURCES
    monitoring.cpp
)

# 头文件列表
set(MONITORING_HEADERS
    monitoring.h
    cpu.h
    memory.h
    disk.h
    network.h
    process.h
)

# 添加静态库
add_library(zen_monitoring STATIC ${MONITORING_SOURCES})

# 设置库的属性
set_target_properties(zen_monitoring PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME "zen_monitoring"
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 包含目录
target_include_directories(zen_monitoring PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/base
)

# 依赖关系
target_link_libraries(zen_monitoring PUBLIC
    pthread
)