# Net模块的CMake配置

add_library(zen_net INTERFACE)

# 设置包含目录
target_include_directories(zen_net
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/src
)

# 链接依赖
target_link_libraries(zen_net
    INTERFACE
        zen_base
        zen_event
)

# 设置编译选项
target_compile_options(zen_net INTERFACE
    -Wall
    -Wextra
    -Wpedantic
    -O2
)

# 安装头文件
install(DIRECTORY
    core
    reactor
    tcp
    DESTINATION include/zen/net
    FILES_MATCHING PATTERN "*.h"
)