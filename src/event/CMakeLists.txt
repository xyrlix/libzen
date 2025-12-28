# 数值与数学工具模块

# 源文件列表
set(EVENT_SOURCES
    event.cpp
)

# 头文件列表
set(EVENT_HEADERS
    callback.h
    event_handler.h
    event_loop.h
    signal_event.h
)

# 添加静态库
add_library(zen_event STATIC ${EVENT_SOURCES})

# 设置库的属性
set_target_properties(zen_event PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME "zen_event"
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 包含目录
target_include_directories(zen_event PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/base
    ${CMAKE_SOURCE_DIR}/src/error
)

# 依赖关系
target_link_libraries(zen_event PUBLIC
    pthread
)