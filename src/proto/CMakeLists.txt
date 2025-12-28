# 轻量级结构化序列化模块

# 源文件列表
set(PROTO_SOURCES
    proto.cpp
)

# 头文件列表
set(PROTO_HEADERS
    proto_base.h
    message.h
    codec.h
    reflection.h
)

# 添加静态库
add_library(zen_proto STATIC ${PROTO_SOURCES})

# 设置库的属性
set_target_properties(zen_proto PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME "zen_proto"
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 包含目录
target_include_directories(zen_proto PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/base
)

# 依赖关系
target_link_libraries(zen_proto PUBLIC
    pthread
)