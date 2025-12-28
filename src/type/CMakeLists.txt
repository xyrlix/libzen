# Type模块的CMake配置

add_library(zen_type INTERFACE)

# 设置包含目录
target_include_directories(zen_type
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/src
)

# 链接依赖
target_link_libraries(zen_type
    INTERFACE
        zen_base
)

# 设置编译选项
target_compile_options(zen_type INTERFACE
    -Wall
    -Wextra
    -Wpedantic
    -O2
)

# 安装头文件
install(FILES
    concepts.h
    ranges.h
    type_erasure.h
    type_convert.h
    DESTINATION include/zen/type
)
