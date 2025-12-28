# Process模块的CMake配置

add_library(zen_process STATIC
    process.cpp
)

# 设置包含目录
target_include_directories(zen_process
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
)

# 链接依赖
target_link_libraries(zen_process
    PRIVATE
        zen_base
        zen_error
)

# 设置编译选项
target_compile_options(zen_process PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -O2
)

# 安装配置
install(TARGETS zen_process
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# 安装头文件
install(FILES
    process.h
    daemon.h
    signal.h
    process_utils.h
    DESTINATION include/zen/process
)
