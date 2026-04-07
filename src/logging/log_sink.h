/**
 * @file log_sink.h
 * @brief 日志输出端（Sink）
 *
 * Sink 是日志的输出目标，可以是标准输出、文件、内存缓冲等。
 * 采用 C 风格多态（函数指针），避免虚函数 / vtable。
 *
 * 内置提供：
 *   - console_sink    : 输出到 stdout / stderr
 *   - file_sink       : 输出到文件（追加模式）
 *   - memory_sink     : 输出到固定大小的环形内存缓冲（测试用）
 */
#pragma once
#include "log_formatter.h"
#include <cstdio>
#include <cstring>

namespace zen {

// ============================================================================
// sink_base：所有 sink 的基础接口（无虚函数，手动 vtable）
// ============================================================================
struct log_sink_vtable {
    void (*write)(void* self, const log_record& rec);
    void (*flush)(void* self);
    void (*destroy)(void* self);
};

struct log_sink {
    const log_sink_vtable* vtable;
    void*                  impl;
    log_level              min_level;  // 低于此级别的记录被忽略

    void write(const log_record& rec) {
        if (vtable && level_enabled(rec.level, min_level)) {
            vtable->write(impl, rec);
        }
    }
    void flush() {
        if (vtable) vtable->flush(impl);
    }
    void destroy() {
        if (vtable) vtable->destroy(impl);
    }
};

// ============================================================================
// console_sink：输出到 stdout（WARN 及以上输出到 stderr）
// ============================================================================
struct console_sink_impl {
    log_formatter formatter;
    char          buf[2048];
};

inline void console_sink_write(void* self, const log_record& rec) {
    console_sink_impl* s = static_cast<console_sink_impl*>(self);
    int n = s->formatter.format(rec, s->buf, sizeof(s->buf));
    FILE* fp = (static_cast<int>(rec.level) >= static_cast<int>(log_level::WARN))
               ? stderr : stdout;
    fwrite(s->buf, 1, n, fp);
}

inline void console_sink_flush(void* self) {
    (void)self;
    fflush(stdout);
    fflush(stderr);
}

inline void console_sink_destroy(void* self) {
    delete static_cast<console_sink_impl*>(self);
}

inline log_sink make_console_sink(format_options opts = {}, log_level min_level = log_level::INFO) {
    static const log_sink_vtable vtbl = {
        console_sink_write,
        console_sink_flush,
        console_sink_destroy
    };
    auto* impl = new console_sink_impl{log_formatter(opts)};
    return log_sink{&vtbl, impl, min_level};
}

// ============================================================================
// file_sink：输出到文件（追加模式）
// ============================================================================
struct file_sink_impl {
    FILE*         fp;
    bool          owns_fp;  // true 表示由此 sink 负责 fclose
    log_formatter formatter;
    char          buf[2048];
};

inline void file_sink_write(void* self, const log_record& rec) {
    file_sink_impl* s = static_cast<file_sink_impl*>(self);
    if (!s->fp) return;
    int n = s->formatter.format(rec, s->buf, sizeof(s->buf));
    fwrite(s->buf, 1, n, s->fp);
}

inline void file_sink_flush(void* self) {
    file_sink_impl* s = static_cast<file_sink_impl*>(self);
    if (s->fp) fflush(s->fp);
}

inline void file_sink_destroy(void* self) {
    file_sink_impl* s = static_cast<file_sink_impl*>(self);
    if (s->fp && s->owns_fp) fclose(s->fp);
    delete s;
}

/**
 * @brief 创建文件 sink
 * @param path     文件路径（追加模式）
 * @param opts     格式化选项
 * @param min_level 最低输出级别
 * @return log_sink（若文件打开失败则 impl 为 nullptr，write 为 no-op）
 */
inline log_sink make_file_sink(const char* path,
                                format_options opts = {},
                                log_level min_level = log_level::INFO) {
    static const log_sink_vtable vtbl = {
        file_sink_write,
        file_sink_flush,
        file_sink_destroy
    };
    FILE* fp = fopen(path, "a");
    auto* impl = new file_sink_impl{fp, true, log_formatter(opts)};
    return log_sink{&vtbl, impl, min_level};
}

// ============================================================================
// memory_sink：将日志追加到内存环形缓冲（主要用于测试/检查）
// ============================================================================
struct memory_sink_impl {
    static const int BUF_SIZE = 65536;  // 64KB 环形缓冲
    char          ring[BUF_SIZE];
    int           write_pos;            // 下次写入位置
    int           total_written;        // 累计写入字节数
    log_formatter formatter;
    char          fmt_buf[2048];        // 格式化临时缓冲

    memory_sink_impl() : write_pos(0), total_written(0) {
        ring[0] = '\0';
    }

    // 将 src[0..n) 追加到环形缓冲（可能覆盖旧内容）
    void append(const char* src, int n) {
        for (int i = 0; i < n; ++i) {
            ring[write_pos] = src[i];
            write_pos = (write_pos + 1) % BUF_SIZE;
        }
        total_written += n;
    }

    // 检查最后写入的内容中是否包含子串（从 ring 尾部逆向搜索最后一条）
    bool contains(const char* substr) const {
        // 简单线性扫描（用于测试）
        int slen = 0;
        while (substr[slen]) ++slen;
        if (slen == 0) return true;

        int limit = total_written < BUF_SIZE ? total_written : BUF_SIZE;
        // 从当前 write_pos 逆向重建线性视图（最多 limit 个字节）
        for (int start = 0; start <= limit - slen; ++start) {
            bool match = true;
            for (int j = 0; j < slen && match; ++j) {
                int idx = (write_pos - limit + start + j + BUF_SIZE * 2) % BUF_SIZE;
                if (ring[idx] != substr[j]) match = false;
            }
            if (match) return true;
        }
        return false;
    }
};

inline void memory_sink_write(void* self, const log_record& rec) {
    memory_sink_impl* s = static_cast<memory_sink_impl*>(self);
    int n = s->formatter.format(rec, s->fmt_buf, sizeof(s->fmt_buf));
    s->append(s->fmt_buf, n);
}
inline void memory_sink_flush(void* /*self*/) {}
inline void memory_sink_destroy(void* self) {
    delete static_cast<memory_sink_impl*>(self);
}

inline log_sink make_memory_sink(format_options opts = {}, log_level min_level = log_level::TRACE) {
    static const log_sink_vtable vtbl = {
        memory_sink_write,
        memory_sink_flush,
        memory_sink_destroy
    };
    auto* impl = new memory_sink_impl{};
    impl->formatter = log_formatter(opts);
    return log_sink{&vtbl, impl, min_level};
}

/** 从 memory_sink 的 impl 指针拿到 memory_sink_impl*（测试用）*/
inline memory_sink_impl* memory_sink_cast(log_sink& s) {
    return static_cast<memory_sink_impl*>(s.impl);
}

} // namespace zen
