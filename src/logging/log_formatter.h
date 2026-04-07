/**
 * @file log_formatter.h
 * @brief 日志格式化器
 *
 * 将一条日志记录（log_record）格式化为字符串，写入固定大小的缓冲区。
 * 完全不依赖堆分配，无 std::string。
 *
 * 格式示例：
 *   [2026-03-22 14:01:23.456] [INFO ] [main.cpp:42] message here
 */
#pragma once
#include "log_level.h"
#include "../threading/thread/this_thread.h"  // monotonic_ms
#include <cstdio>   // snprintf
#include <cstring>  // strlen

namespace zen {

// ============================================================================
// log_record：一条日志的原始信息
// ============================================================================
struct log_record {
    log_level   level;
    const char* file;      // __FILE__ 或简化路径
    int         line;      // __LINE__
    const char* func;      // __func__
    const char* message;   // 日志正文（调用方管理生命周期）
    long long   time_ms;   // 单调时钟毫秒（相对启动）
    unsigned long thread_id; // 线程 id（简化为 unsigned long）
};

// ============================================================================
// 格式化选项
// ============================================================================
struct format_options {
    bool show_time      = true;
    bool show_level     = true;
    bool show_file      = true;
    bool show_func      = false;
    bool show_thread_id = false;
    bool use_color      = false;
};

// ============================================================================
// log_formatter：将 log_record 格式化到缓冲区
// ============================================================================
class log_formatter {
public:
    explicit log_formatter(format_options opts = {}) noexcept
        : opts_(opts) {}

    /**
     * @brief 将 log_record 格式化写入 buf[buf_size]
     * @return 写入的字节数（不含 '\0'）
     */
    int format(const log_record& rec, char* buf, int buf_size) const noexcept {
        int pos = 0;

        // 颜色前缀
        if (opts_.use_color) {
            const char* color = level_to_color(rec.level);
            pos += safe_copy(buf + pos, buf_size - pos, color);
        }

        // 时间戳
        if (opts_.show_time) {
            pos += snprintf(buf + pos, buf_size - pos > 0 ? buf_size - pos : 0,
                            "[%lld.%03lld] ", rec.time_ms / 1000, rec.time_ms % 1000);
        }

        // 日志级别
        if (opts_.show_level) {
            pos += snprintf(buf + pos, buf_size - pos > 0 ? buf_size - pos : 0,
                            "[%s] ", level_to_string(rec.level));
        }

        // 线程 id
        if (opts_.show_thread_id) {
            pos += snprintf(buf + pos, buf_size - pos > 0 ? buf_size - pos : 0,
                            "[T%lu] ", rec.thread_id);
        }

        // 文件:行
        if (opts_.show_file && rec.file) {
            // 只取文件名（最后一个 / 或 \ 之后）
            const char* basename = rec.file;
            for (const char* p = rec.file; *p; ++p) {
                if (*p == '/' || *p == '\\') basename = p + 1;
            }
            pos += snprintf(buf + pos, buf_size - pos > 0 ? buf_size - pos : 0,
                            "[%s:%d] ", basename, rec.line);
        }

        // 函数名
        if (opts_.show_func && rec.func) {
            pos += snprintf(buf + pos, buf_size - pos > 0 ? buf_size - pos : 0,
                            "[%s] ", rec.func);
        }

        // 消息正文
        if (rec.message) {
            int remain = buf_size - pos - 1;
            if (remain > 0) {
                pos += snprintf(buf + pos, remain + 1, "%s", rec.message);
            }
        }

        // 颜色重置
        if (opts_.use_color) {
            pos += safe_copy(buf + pos, buf_size - pos, color_reset());
        }

        // 换行
        if (pos < buf_size - 1) {
            buf[pos++] = '\n';
        }

        if (pos < buf_size) buf[pos] = '\0';
        else buf[buf_size - 1] = '\0';

        return pos;
    }

    void set_options(format_options opts) noexcept { opts_ = opts; }
    const format_options& options() const noexcept { return opts_; }

private:
    format_options opts_;

    static int safe_copy(char* dst, int dst_size, const char* src) noexcept {
        if (!dst || dst_size <= 0 || !src) return 0;
        int n = 0;
        while (*src && n < dst_size - 1) {
            dst[n++] = *src++;
        }
        dst[n] = '\0';
        return n;
    }
};

} // namespace zen
