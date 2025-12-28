# RPC模块

# 源文件列表
set(RPC_SOURCES
    rpc.cpp
)

# 头文件列表
set(RPC_HEADERS
    rpc.h
    service.h
    client.h
    server.h
    protocol.h
    serialization.h
    error.h
)

# 添加静态库
add_library(zen_rpc STATIC ${RPC_SOURCES})

# 设置库的属性
set_target_properties(zen_rpc PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME "zen_rpc"
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 包含目录
target_include_directories(zen_rpc PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/base
    ${CMAKE_SOURCE_DIR}/src/event
    ${CMAKE_SOURCE_DIR}/src/net
    ${CMAKE_SOURCE_DIR}/src/proto
    ${CMAKE_SOURCE_DIR}/src/error
)

# 依赖关系
target_link_libraries(zen_rpc PUBLIC
    pthread
    zen_event
    zen_net
    zen_proto
    zen_error
)