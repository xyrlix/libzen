/**
 * @file file_io.h
 * @brief 文件 I/O 封装
 *
 * 提供同步文件读写操作，不依赖 std::fstream。
 * 基于 POSIX FILE* / Win32 HANDLE（此处统一用 FILE*，跨平台更简单）。
 *
 * 核心类：
 *   - file        : RAII 文件句柄，支持读/写/定位
 *   - file_reader : 按行/按块读取帮助器
 *   - file_writer : 带缓冲写帮助器
 *
 * 工具函数：
 *   - read_all()  : 一次性读取整个文件到缓冲
 *   - write_all() : 一次性写入缓冲到文件
 *   - file_size() : 获取文件大小
 *   - file_exists(): 检查文件是否存在
 *
 * 示例：
 * @code
 * zen::file f("log.txt", zen::open_mode::WRITE_TRUNC);
 * f.write("hello\n", 6);
 * f.flush();
 *
 * char buf[256];
 * zen::file r("log.txt", zen::open_mode::READ);
 * zen::ssize_t n = r.read(buf, sizeof(buf));
 * @endcode
 */
#pragma once
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstddef>
#include <cstdint>

// stat 用于 file_size / file_exists
#if defined(_WIN32)
#  include <sys/stat.h>
#  define ZEN_STAT  _stat
#  define ZEN_STAT_T struct _stat
#else
#  include <sys/stat.h>
#  include <unistd.h>
#  define ZEN_STAT  stat
#  define ZEN_STAT_T struct stat
#endif

namespace zen {

using ssize_t = long long;

// ============================================================================
// 打开模式
// ============================================================================
enum class open_mode : int {
    READ        = 0,   // 只读（文件必须存在）
    WRITE_TRUNC = 1,   // 只写（清空或新建）
    WRITE_APPEND= 2,   // 追加写
    READ_WRITE  = 3,   // 读写（文件必须存在）
};

inline const char* open_mode_str(open_mode m) noexcept {
    switch (m) {
        case open_mode::READ:         return "rb";
        case open_mode::WRITE_TRUNC:  return "wb";
        case open_mode::WRITE_APPEND: return "ab";
        case open_mode::READ_WRITE:   return "r+b";
        default:                      return "rb";
    }
}

// ============================================================================
// file：RAII 文件句柄
// ============================================================================
class file {
public:
    /** 默认构造，未关联文件 */
    file() noexcept : fp_(nullptr) {}

    /**
     * @brief 打开文件
     * @param path 文件路径
     * @param mode 打开模式
     */
    file(const char* path, open_mode mode) noexcept
        : fp_(nullptr) {
        open(path, mode);
    }

    ~file() noexcept { close(); }

    // 禁止拷贝，允许移动
    file(const file&) = delete;
    file& operator=(const file&) = delete;

    file(file&& other) noexcept : fp_(other.fp_) { other.fp_ = nullptr; }
    file& operator=(file&& other) noexcept {
        if (this != &other) {
            close();
            fp_ = other.fp_;
            other.fp_ = nullptr;
        }
        return *this;
    }

    // ----------------------------------------------------------------
    // 打开 / 关闭
    // ----------------------------------------------------------------

    bool open(const char* path, open_mode mode) noexcept {
        close();
        fp_ = fopen(path, open_mode_str(mode));
        return fp_ != nullptr;
    }

    void close() noexcept {
        if (fp_) { fclose(fp_); fp_ = nullptr; }
    }

    bool is_open() const noexcept { return fp_ != nullptr; }
    explicit operator bool() const noexcept { return is_open(); }

    // ----------------------------------------------------------------
    // 读写
    // ----------------------------------------------------------------

    /**
     * @brief 从文件读取最多 n 个字节到 buf
     * @return 实际读取字节数，-1 表示错误
     */
    ssize_t read(void* buf, size_t n) noexcept {
        if (!fp_) return -1;
        size_t r = fread(buf, 1, n, fp_);
        if (r == 0 && ferror(fp_)) return -1;
        return static_cast<ssize_t>(r);
    }

    /**
     * @brief 向文件写入 n 个字节
     * @return 实际写入字节数，-1 表示错误
     */
    ssize_t write(const void* buf, size_t n) noexcept {
        if (!fp_) return -1;
        size_t w = fwrite(buf, 1, n, fp_);
        if (w < n) return -1;
        return static_cast<ssize_t>(w);
    }

    /** 写入 C 字符串（不含 '\0'） */
    ssize_t write_str(const char* s) noexcept {
        if (!s) return 0;
        return write(s, strlen(s));
    }

    // ----------------------------------------------------------------
    // 定位
    // ----------------------------------------------------------------

    enum class seek_origin : int {
        BEGIN   = SEEK_SET,
        CURRENT = SEEK_CUR,
        END     = SEEK_END,
    };

    bool seek(long long offset, seek_origin origin = seek_origin::BEGIN) noexcept {
        if (!fp_) return false;
#if defined(_WIN32)
        return _fseeki64(fp_, offset, static_cast<int>(origin)) == 0;
#else
        return fseeko(fp_, static_cast<off_t>(offset), static_cast<int>(origin)) == 0;
#endif
    }

    long long tell() noexcept {
        if (!fp_) return -1;
#if defined(_WIN32)
        return _ftelli64(fp_);
#else
        return static_cast<long long>(ftello(fp_));
#endif
    }

    // ----------------------------------------------------------------
    // 其他
    // ----------------------------------------------------------------

    void flush() noexcept { if (fp_) fflush(fp_); }
    bool eof() const noexcept { return fp_ && feof(fp_); }
    bool error() const noexcept { return fp_ && ferror(fp_); }

    FILE* native() const noexcept { return fp_; }

private:
    FILE* fp_;
};

// ============================================================================
// 工具函数
// ============================================================================

/**
 * @brief 获取文件大小（字节），文件不存在返回 -1
 */
inline long long file_size(const char* path) noexcept {
    ZEN_STAT_T st;
    if (ZEN_STAT(path, &st) != 0) return -1LL;
    return static_cast<long long>(st.st_size);
}

/**
 * @brief 检查文件是否存在
 */
inline bool file_exists(const char* path) noexcept {
    ZEN_STAT_T st;
    return ZEN_STAT(path, &st) == 0;
}

/**
 * @brief 将整个文件内容读入调用方提供的缓冲区
 * @param path     文件路径
 * @param buf      输出缓冲
 * @param buf_size 缓冲区大小
 * @return 实际读入字节数；-1 表示打开失败；-2 表示文件太大（超过 buf_size-1）
 */
inline ssize_t read_all(const char* path, char* buf, size_t buf_size) noexcept {
    file f(path, open_mode::READ);
    if (!f) return -1;
    long long sz = file_size(path);
    if (sz < 0) return -1;
    if (static_cast<size_t>(sz) >= buf_size) return -2;
    ssize_t n = f.read(buf, static_cast<size_t>(sz));
    if (n >= 0) buf[n] = '\0';
    return n;
}

/**
 * @brief 将缓冲区内容写入文件（覆盖）
 * @return 写入字节数；-1 表示失败
 */
inline ssize_t write_all(const char* path, const void* data, size_t n) noexcept {
    file f(path, open_mode::WRITE_TRUNC);
    if (!f) return -1;
    return f.write(data, n);
}

/**
 * @brief 将缓冲区内容追加到文件末尾
 */
inline ssize_t append_all(const char* path, const void* data, size_t n) noexcept {
    file f(path, open_mode::WRITE_APPEND);
    if (!f) return -1;
    return f.write(data, n);
}

// ============================================================================
// file_reader：按行读取帮助器
// ============================================================================
class file_reader {
public:
    static const int LINE_BUF = 4096;

    explicit file_reader(const char* path) noexcept
        : f_(path, open_mode::READ) {}

    bool is_open() const noexcept { return f_.is_open(); }

    /**
     * @brief 读取下一行（不含换行符，末尾 '\0'）
     * @param buf      输出缓冲
     * @param buf_size 缓冲区大小
     * @return 读取字节数（>=0），0 表示 EOF，-1 表示错误
     */
    ssize_t read_line(char* buf, int buf_size) noexcept {
        if (!f_ || buf_size <= 0) return -1;
        int pos = 0;
        while (pos < buf_size - 1) {
            char c;
            ssize_t r = f_.read(&c, 1);
            if (r <= 0) {
                // EOF 或错误
                if (pos == 0) return (r == 0) ? 0 : -1;
                break;
            }
            if (c == '\n') break;
            if (c != '\r') buf[pos++] = c;  // 忽略 \r
        }
        buf[pos] = '\0';
        return pos;
    }

    bool eof() const noexcept { return f_.eof(); }

private:
    file f_;
};

// ============================================================================
// file_writer：带小缓冲的写帮助器
// ============================================================================
class file_writer {
public:
    static const int FLUSH_SIZE = 4096;

    explicit file_writer(const char* path, open_mode mode = open_mode::WRITE_TRUNC) noexcept
        : f_(path, mode), buf_pos_(0) {}

    ~file_writer() noexcept { flush(); }

    bool is_open() const noexcept { return f_.is_open(); }

    /** 写字节块 */
    bool write(const void* data, size_t n) noexcept {
        const char* p = static_cast<const char*>(data);
        while (n > 0) {
            size_t room = FLUSH_SIZE - buf_pos_;
            size_t copy = n < room ? n : room;
            memcpy(buf_ + buf_pos_, p, copy);
            buf_pos_ += copy;
            p += copy;
            n -= copy;
            if (buf_pos_ >= FLUSH_SIZE) {
                if (!flush()) return false;
            }
        }
        return true;
    }

    bool write_str(const char* s) noexcept {
        return s && write(s, strlen(s));
    }

    bool flush() noexcept {
        if (buf_pos_ > 0) {
            if (f_.write(buf_, buf_pos_) < 0) return false;
            buf_pos_ = 0;
            f_.flush();
        }
        return true;
    }

private:
    file   f_;
    char   buf_[FLUSH_SIZE];
    size_t buf_pos_;
};

} // namespace zen
