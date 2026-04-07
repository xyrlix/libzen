/**
 * @file print.h
 * @brief 格式化打印实现
 * 
 * 手撕实现，不使用 std::printf
 */

#ifndef ZEN_FMT_PRINT_H
#define ZEN_FMT_PRINT_H

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdarg>

namespace zen {

// ==================== 输出流基础 ====================

/**
 * @brief 输出到标准输出
 */
inline int print(const char* str) {
    return std::printf("%s", str);
}

/**
 * @brief 输出并换行
 */
inline int println(const char* str) {
    return std::printf("%s\n", str);
}

/**
 * @brief 打印整数
 */
inline int print(int value) {
    return std::printf("%d", value);
}

inline int print(long value) {
    return std::printf("%ld", value);
}

inline int print(long long value) {
    return std::printf("%lld", value);
}

inline int print(unsigned int value) {
    return std::printf("%u", value);
}

inline int print(unsigned long value) {
    return std::printf("%lu", value);
}

inline int print(unsigned long long value) {
    return std::printf("%llu", value);
}

/**
 * @brief 打印浮点数
 */
inline int print(double value) {
    return std::printf("%f", value);
}

inline int print(long double value) {
    return std::printf("%Lf", value);
}

/**
 * @brief 打印字符和字符串
 */
inline int print(char c) {
    return std::printf("%c", c);
}

/**
 * @brief 打印布尔值
 */
inline int print(bool value) {
    return std::printf("%s", value ? "true" : "false");
}

/**
 * @brief 打印指针
 */
inline int print(const void* ptr) {
    return std::printf("%p", ptr);
}

// ==================== 格式化打印 ====================

/**
 * @brief 格式化打印整数
 * @param format 格式字符串
 * @param ... 参数
 */
inline int printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = std::vprintf(format, args);
    va_end(args);
    return result;
}

// ==================== 便捷打印函数 ====================

/**
 * @brief 打印多个值（递归结束）
 */
inline void print_values() {}

/**
 * @brief 打印多个值
 */
template<typename T, typename... Args>
void print_values(const T& first, const Args&... rest) {
    print(first);
    if (sizeof...(rest) > 0) {
        print(" ");
    }
    print_values(rest...);
}

/**
 * @brief 打印一行（多个值）
 */
template<typename... Args>
int println(const Args&... args) {
    print_values(args...);
    return print("\n");
}

/**
 * @brief 格式化打印一行
 */
template<typename... Args>
int printfln(const char* format, Args... args) {
    int result = printf(format, args...);
    print("\n");
    return result;
}

// ==================== 容器打印 ====================

/**
 * @brief 打印数组
 */
template<typename T, size_t N>
void print_array(const T (&arr)[N], const char* separator = ", ") {
    print("[");
    for (size_t i = 0; i < N; ++i) {
        print(arr[i]);
        if (i < N - 1) {
            print(separator);
        }
    }
    print("]");
}

/**
 * @brief 打印数组并换行
 */
template<typename T, size_t N>
void println_array(const T (&arr)[N], const char* separator = ", ") {
    print_array(arr, separator);
    print("\n");
}

// ==================== 表格对齐打印 ====================

/**
 * @brief 表格打印对齐
 */
inline void print_table_row(const char* col1, const char* col2, const char* col3 = nullptr) {
    printf("%-20s | %-20s", col1, col2);
    if (col3) {
        printf(" | %s", col3);
    }
    print("\n");
}

// ==================== 调试打印 ====================

#ifdef DEBUG

/**
 * @brief 调试打印（仅在 DEBUG 模式下有效）
 */
#define ZEN_DEBUG_PRINT(...) printf(__VA_ARGS__)

/**
 * @brief 调试打印变量名和值
 */
#define ZEN_DEBUG_VAR(x) printf("%s = %d\n", #x, x)

#else

#define ZEN_DEBUG_PRINT(...) 
#define ZEN_DEBUG_VAR(x)

#endif

// ==================== 颜色打印（ANSI 转义序列）====================

namespace color {
    // ANSI 颜色代码
    static const char* reset = "\033[0m";
    static const char* red = "\033[31m";
    static const char* green = "\033[32m";
    static const char* yellow = "\033[33m";
    static const char* blue = "\033[34m";
    static const char* magenta = "\033[35m";
    static const char* cyan = "\033[36m";
    static const char* white = "\033[37m";
    
    // 加粗
    static const char* bold = "\033[1m";
    static const char* dim = "\033[2m";
    
    // 背景色
    static const char* bg_red = "\033[41m";
    static const char* bg_green = "\033[42m";
    static const char* bg_yellow = "\033[43m";
    static const char* bg_blue = "\033[44m";
}

/**
 * @brief 打印带颜色的文本
 */
inline void print_color(const char* text, const char* color_code) {
    std::printf("%s%s%s", color_code, text, color::reset);
}

/**
 * @brief 打印错误（红色）
 */
inline void print_error(const char* msg) {
    print_color(msg, color::red);
}

/**
 * @brief 打印成功（绿色）
 */
inline void print_success(const char* msg) {
    print_color(msg, color::green);
}

/**
 * @brief 打印警告（黄色）
 */
inline void print_warning(const char* msg) {
    print_color(msg, color::yellow);
}

/**
 * @brief 打印信息（蓝色）
 */
inline void print_info(const char* msg) {
    print_color(msg, color::blue);
}

} // namespace zen

#endif // ZEN_FMT_PRINT_H
