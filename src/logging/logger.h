/**
 * @file logger.h
 * @brief 日志器主接口
 *
 * logger 持有若干 log_sink，每次记录日志时向所有 sink 分发。
 * 支持最多 ZEN_LOGGER_MAX_SINKS 个 sink（默认 8）。
 *
 * 全局便捷宏：
 *   ZEN_LOG_TRACE(logger, msg)
 *   ZEN_LOG_DEBUG(logger, msg)
 *   ZEN_LOG_INFO(logger, msg)
 *   ZEN_LOG_WARN(logger, msg)
 *   ZEN_LOG_ERROR(logger, msg)
 *   ZEN_LOG_FATAL(logger, msg)
 *
 * 格式化宏（snprintf 风格）：
 *   ZEN_LOGF_INFO(logger, fmt, ...)
 *   ZEN_LOGF_WARN(logger, fmt, ...)
 *   ...
 *
 * 全局单例（可选）：
 *   zen::logger& zen::global_logger()
 *
 * 示例：
 * @code
 * zen::logger log;
 * auto sink = zen::make_console_sink({.use_color = true});
 * log.add_sink(sink);
 * log.set_level(zen::log_level::DEBUG);
 *
 * ZEN_LOG_INFO(log, "server started");
 * ZEN_LOGF_WARN(log, "load = %d%%", 80);
 * @endcode
 */
#pragma once
#include "log_sink.h"
#include "../threading/sync/mutex.h"
#include "../threading/sync/lock_guard.h"
#include "../threading/thread/this_thread.h"
#include <cstdio>
#include <cstring>
#include <cstdarg>

namespace zen {

// ============================================================================
// logger：多 sink 日志器
// ============================================================================

#ifndef ZEN_LOGGER_MAX_SINKS
#define ZEN_LOGGER_MAX_SINKS 8
#endif

class logger {
public:
    logger() noexcept
        : level_(log_level::INFO), sink_count_(0) {}

    ~logger() {
        for (int i = 0; i < sink_count_; ++i) {
            sinks_[i].destroy();
        }
    }

    // 禁止拷贝（sink 所有权不共享）
    logger(const logger&) = delete;
    logger& operator=(const logger&) = delete;

    // ----------------------------------------------------------------
    // 配置
    // ----------------------------------------------------------------

    /** 设置全局最低日志级别（低于此级别的调用直接返回） */
    void set_level(log_level lvl) noexcept {
        lock_guard<mutex> lk(mtx_);
        level_ = lvl;
    }

    log_level level() const noexcept { return level_; }

    /**
     * @brief 添加一个 sink（最多 ZEN_LOGGER_MAX_SINKS 个）
     * @return true 成功，false 已满
     */
    bool add_sink(log_sink sink) {
        lock_guard<mutex> lk(mtx_);
        if (sink_count_ >= ZEN_LOGGER_MAX_SINKS) return false;
        sinks_[sink_count_++] = sink;
        return true;
    }

    /** 移除所有 sink 并 destroy */
    void clear_sinks() {
        lock_guard<mutex> lk(mtx_);
        for (int i = 0; i < sink_count_; ++i) {
            sinks_[i].destroy();
        }
        sink_count_ = 0;
    }

    // ----------------------------------------------------------------
    // 核心写入
    // ----------------------------------------------------------------

    /**
     * @brief 直接写入一条记录
     */
    void write(const log_record& rec) {
        if (!level_enabled(rec.level, level_)) return;
        lock_guard<mutex> lk(mtx_);
        for (int i = 0; i < sink_count_; ++i) {
            sinks_[i].write(rec);
        }
    }

    /**
     * @brief 最常用的入口：给定级别 + 消息 + 位置信息
     */
    void log(log_level lvl, const char* msg,
             const char* file = nullptr, int line = 0, const char* func = nullptr) {
        if (!level_enabled(lvl, level_)) return;
        log_record rec;
        rec.level     = lvl;
        rec.file      = file;
        rec.line      = line;
        rec.func      = func;
        rec.message   = msg;
        rec.time_ms   = (long long)this_thread::monotonic_ms();
        rec.thread_id = static_cast<unsigned long>(this_thread::get_id().native_id());
        write(rec);
    }

    /**
     * @brief snprintf 格式化版本
     */
    void logf(log_level lvl,
              const char* file, int line, const char* func,
              const char* fmt, ...) {
        if (!level_enabled(lvl, level_)) return;
        char buf[1024];
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(buf, sizeof(buf), fmt, ap);
        va_end(ap);
        log(lvl, buf, file, line, func);
    }

    /** 刷新所有 sink */
    void flush() {
        lock_guard<mutex> lk(mtx_);
        for (int i = 0; i < sink_count_; ++i) {
            sinks_[i].flush();
        }
    }

    /** 返回 sink 数量 */
    int sink_count() const noexcept { return sink_count_; }

private:
    mutable mutex mtx_;
    log_level     level_;
    int           sink_count_;
    log_sink      sinks_[ZEN_LOGGER_MAX_SINKS];
};

// ============================================================================
// 全局单例 logger（可选使用）
// ============================================================================
inline logger& global_logger() {
    static logger s_logger;
    return s_logger;
}

} // namespace zen

// ============================================================================
// 便捷宏（带位置信息）
// ============================================================================

/** 写入单条字符串日志 */
#define ZEN_LOG(logger_ref, level, msg) \
    (logger_ref).log((level), (msg), __FILE__, __LINE__, __func__)

#define ZEN_LOG_TRACE(logger_ref, msg) ZEN_LOG(logger_ref, zen::log_level::TRACE, msg)
#define ZEN_LOG_DEBUG(logger_ref, msg) ZEN_LOG(logger_ref, zen::log_level::DEBUG, msg)
#define ZEN_LOG_INFO(logger_ref, msg)  ZEN_LOG(logger_ref, zen::log_level::INFO,  msg)
#define ZEN_LOG_WARN(logger_ref, msg)  ZEN_LOG(logger_ref, zen::log_level::WARN,  msg)
#define ZEN_LOG_ERROR(logger_ref, msg) ZEN_LOG(logger_ref, zen::log_level::ERROR, msg)
#define ZEN_LOG_FATAL(logger_ref, msg) ZEN_LOG(logger_ref, zen::log_level::FATAL, msg)

/** 格式化日志（snprintf 风格） */
#define ZEN_LOGF(logger_ref, level, fmt, ...) \
    (logger_ref).logf((level), __FILE__, __LINE__, __func__, (fmt), ##__VA_ARGS__)

#define ZEN_LOGF_TRACE(logger_ref, fmt, ...) ZEN_LOGF(logger_ref, zen::log_level::TRACE, fmt, ##__VA_ARGS__)
#define ZEN_LOGF_DEBUG(logger_ref, fmt, ...) ZEN_LOGF(logger_ref, zen::log_level::DEBUG, fmt, ##__VA_ARGS__)
#define ZEN_LOGF_INFO(logger_ref, fmt, ...)  ZEN_LOGF(logger_ref, zen::log_level::INFO,  fmt, ##__VA_ARGS__)
#define ZEN_LOGF_WARN(logger_ref, fmt, ...)  ZEN_LOGF(logger_ref, zen::log_level::WARN,  fmt, ##__VA_ARGS__)
#define ZEN_LOGF_ERROR(logger_ref, fmt, ...) ZEN_LOGF(logger_ref, zen::log_level::ERROR, fmt, ##__VA_ARGS__)
#define ZEN_LOGF_FATAL(logger_ref, fmt, ...) ZEN_LOGF(logger_ref, zen::log_level::FATAL, fmt, ##__VA_ARGS__)

/** 全局 logger 简化宏 */
#define ZEN_GLOG_INFO(msg)  ZEN_LOG_INFO(zen::global_logger(), msg)
#define ZEN_GLOG_WARN(msg)  ZEN_LOG_WARN(zen::global_logger(), msg)
#define ZEN_GLOG_ERROR(msg) ZEN_LOG_ERROR(zen::global_logger(), msg)
#define ZEN_GLOGF_INFO(fmt, ...)  ZEN_LOGF_INFO(zen::global_logger(), fmt, ##__VA_ARGS__)
#define ZEN_GLOGF_WARN(fmt, ...)  ZEN_LOGF_WARN(zen::global_logger(), fmt, ##__VA_ARGS__)
#define ZEN_GLOGF_ERROR(fmt, ...) ZEN_LOGF_ERROR(zen::global_logger(), fmt, ##__VA_ARGS__)
