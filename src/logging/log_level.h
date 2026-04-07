/**
 * @file log_level.h
 * @brief 日志级别定义
 *
 * 定义 log_level 枚举和相关工具函数。
 * 级别从低到高：TRACE < DEBUG < INFO < WARN < ERROR < FATAL < OFF
 */
#pragma once
#include "../base/macros.h"

namespace zen {

// ============================================================================
// log_level 枚举
// ============================================================================
enum class log_level : int {
    TRACE = 0,  // 最详细的调试信息
    DEBUG = 1,  // 调试信息
    INFO  = 2,  // 普通信息（默认级别）
    WARN  = 3,  // 警告
    ERROR = 4,  // 错误（不中断运行）
    FATAL = 5,  // 致命错误
    OFF   = 6   // 关闭所有日志
};

// ============================================================================
// 工具函数
// ============================================================================

/**
 * @brief 将 log_level 转为字符串（大写简称）
 */
inline const char* level_to_string(log_level lvl) noexcept {
    switch (lvl) {
        case log_level::TRACE: return "TRACE";
        case log_level::DEBUG: return "DEBUG";
        case log_level::INFO:  return "INFO ";
        case log_level::WARN:  return "WARN ";
        case log_level::ERROR: return "ERROR";
        case log_level::FATAL: return "FATAL";
        case log_level::OFF:   return "OFF  ";
        default:               return "?????";
    }
}

/**
 * @brief 将 log_level 转为 ANSI 颜色前缀（终端彩色输出）
 */
inline const char* level_to_color(log_level lvl) noexcept {
    switch (lvl) {
        case log_level::TRACE: return "\033[37m";      // 白/灰
        case log_level::DEBUG: return "\033[36m";      // 青色
        case log_level::INFO:  return "\033[32m";      // 绿色
        case log_level::WARN:  return "\033[33m";      // 黄色
        case log_level::ERROR: return "\033[31m";      // 红色
        case log_level::FATAL: return "\033[35m";      // 品红
        default:               return "\033[0m";
    }
}

/** ANSI 重置颜色 */
inline const char* color_reset() noexcept { return "\033[0m"; }

/**
 * @brief 比较两个级别，返回 lvl >= threshold 时为 true
 */
inline bool level_enabled(log_level lvl, log_level threshold) noexcept {
    return static_cast<int>(lvl) >= static_cast<int>(threshold);
}

} // namespace zen
