# HTTP协议模块

# 源文件列表
set(HTTP_SOURCES
    http.cpp
)

# 头文件列表
set(HTTP_HEADERS
    http.h
    request.h
    response.h
    server.h
    client.h
    utils.h
)

# 添加静态库
add_library(zen_http STATIC ${HTTP_SOURCES})

# 设置库的属性
set_target_properties(zen_http PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME "zen_http"
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 包含目录
target_include_directories(zen_http PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/base
    ${CMAKE_SOURCE_DIR}/src/event
    ${CMAKE_SOURCE_DIR}/src/net
)

# 依赖关系
target_link_libraries(zen_http PUBLIC
    pthread
    zen_event
    zen_net
)