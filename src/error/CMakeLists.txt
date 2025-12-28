# Error模块的CMake配置

add_library(zen_error STATIC
    error.cpp
)

# 设置包含目录
target_include_directories(zen_error
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
)

# 链接依赖
target_link_libraries(zen_error
    PRIVATE
        zen_base
        zen_utility
)

# 设置编译选项
target_compile_options(zen_error PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -O2
)

# 安装配置
install(TARGETS zen_error
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# 安装头文件
install(FILES
    system_error.h
    exception_chain.h
    error_context.h
    error_handler.h
    DESTINATION include/zen/error
)
