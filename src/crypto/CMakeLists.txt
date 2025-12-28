# Crypto模块的CMake配置

add_library(zen_crypto STATIC
    crypto.cpp
)

# 设置包含目录
target_include_directories(zen_crypto
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
)

# 链接依赖
target_link_libraries(zen_crypto
    PRIVATE
        zen_base
        zen_memory
        zen_error
)

# 设置编译选项
target_compile_options(zen_crypto PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -O2
)

# 安装配置
install(TARGETS zen_crypto
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# 安装头文件
install(FILES
    hash.h
    symmetric.h
    asymmetric.h
    base64.h
    DESTINATION include/zen/crypto
)
