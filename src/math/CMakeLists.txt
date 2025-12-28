# Math模块的CMake配置

add_library(zen_math STATIC
    math.cpp
)

# 设置包含目录
target_include_directories(zen_math
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
)

# 链接依赖
target_link_libraries(zen_math
    PRIVATE
        zen_base
        zen_utility
)

# 设置编译选项
target_compile_options(zen_math PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -O2
)

# 安装配置
install(TARGETS zen_math
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# 安装头文件
install(FILES
    basic_math.h
    random.h
    numeric_stats.h
    fixed_point.h
    DESTINATION include/zen/math
)
