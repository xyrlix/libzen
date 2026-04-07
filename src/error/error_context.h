/**
 * @file error_context.h
 * @brief 带上下文信息的错误描述
 * 
 * error_context 在 error_code 的基础上附加：
 * - 文件名、行号（__FILE__/__LINE__）
 * - 函数名（__func__）
 * - 用户提供的描述消息
 * 
 * 便于调试，用 ZEN_MAKE_ERROR 宏自动填充位置信息。
 */
#ifndef ZEN_ERROR_ERROR_CONTEXT_H
#define ZEN_ERROR_ERROR_CONTEXT_H

#include "system_error.h"
#include <cstring>
#include <cstdio>

namespace zen {

// ============================================================================
// 字符串工具（避免依赖 <string>）
// ============================================================================

namespace detail {

/**
 * @brief 固定大小字符串缓冲区（用于不依赖 std::string 存储消息）
 */
template<size_t N = 256>
class fixed_string {
public:
    fixed_string() noexcept { buf_[0] = '\0'; }
    
    explicit fixed_string(const char* str) noexcept {
        if (str) {
            size_t len = 0;
            while (str[len] && len < N - 1) ++len;
            for (size_t i = 0; i < len; ++i) buf_[i] = str[i];
            buf_[len] = '\0';
        } else {
            buf_[0] = '\0';
        }
    }
    
    const char* c_str() const noexcept { return buf_; }
    bool empty() const noexcept { return buf_[0] == '\0'; }
    
    void append(const char* str) noexcept {
        if (!str) return;
        size_t cur = 0;
        while (buf_[cur]) ++cur;
        while (*str && cur < N - 1) {
            buf_[cur++] = *str++;
        }
        buf_[cur] = '\0';
    }
    
    void append_char(char c) noexcept {
        size_t cur = 0;
        while (buf_[cur]) ++cur;
        if (cur < N - 1) {
            buf_[cur] = c;
            buf_[cur + 1] = '\0';
        }
    }

private:
    char buf_[N];
};

} // namespace detail

// ============================================================================
// error_context
// ============================================================================

/**
 * @brief 带上下文信息的错误
 * 
 * 示例：
 * @code
 * zen::error_context ec(zen::errc::not_found, "file not found", __FILE__, __LINE__, __func__);
 * ec.print();
 * @endcode
 * 
 * 或使用宏：
 * @code
 * auto ec = ZEN_MAKE_ERROR(zen::errc::not_found, "file not found");
 * @endcode
 */
class error_context {
public:
    static constexpr size_t MAX_MESSAGE_LEN = 256;
    static constexpr size_t MAX_FILE_LEN    = 128;
    static constexpr size_t MAX_FUNC_LEN    = 64;
    
    /**
     * @brief 默认构造（成功，无错误）
     */
    error_context() noexcept : code_() {
        file_[0] = '\0';
        func_[0] = '\0';
        message_[0] = '\0';
        line_ = 0;
    }
    
    /**
     * @brief 构造带完整上下文的错误
     * @param code  错误码
     * @param msg   错误描述（用户可读消息）
     * @param file  源文件路径（通常传 __FILE__）
     * @param line  行号（通常传 __LINE__）
     * @param func  函数名（通常传 __func__）
     */
    error_context(error_code code,
                  const char* msg  = nullptr,
                  const char* file = nullptr,
                  int         line = 0,
                  const char* func = nullptr) noexcept
        : code_(code), line_(line)
    {
        copy_str(message_, msg,  MAX_MESSAGE_LEN);
        copy_str(file_,    file, MAX_FILE_LEN);
        copy_str(func_,    func, MAX_FUNC_LEN);
    }
    
    /**
     * @brief 从 errc 直接构造
     */
    error_context(errc e,
                  const char* msg  = nullptr,
                  const char* file = nullptr,
                  int         line = 0,
                  const char* func = nullptr) noexcept
        : error_context(make_error_code(e), msg, file, line, func) {}
    
    // ---- 访问器 ----
    
    const error_code& code() const noexcept { return code_; }
    const char* message()  const noexcept { return message_[0] ? message_ : code_.message(); }
    const char* file()     const noexcept { return file_; }
    int         line()     const noexcept { return line_; }
    const char* function() const noexcept { return func_; }
    
    /**
     * @brief 是否有错误（code 非零）
     */
    explicit operator bool() const noexcept { return static_cast<bool>(code_); }
    bool ok() const noexcept { return !code_; }
    
    /**
     * @brief 打印错误信息到 stderr
     */
    void print() const noexcept {
        if (!code_) {
            std::fprintf(stderr, "[OK]\n");
            return;
        }
        
        if (file_[0] && func_[0]) {
            std::fprintf(stderr, "[ERROR] %s:%d in %s(): [%s/%d] %s\n",
                         file_, line_, func_,
                         code_.category().name(),
                         code_.value(),
                         message());
        } else {
            std::fprintf(stderr, "[ERROR] [%s/%d] %s\n",
                         code_.category().name(),
                         code_.value(),
                         message());
        }
    }
    
    /**
     * @brief 格式化到缓冲区
     * @param buf    目标缓冲区
     * @param buflen 缓冲区大小
     * @return 实际写入字符数
     */
    int format(char* buf, size_t buflen) const noexcept {
        if (!buf || buflen == 0) return 0;
        if (!code_) {
            return std::snprintf(buf, buflen, "[OK]");
        }
        if (file_[0] && func_[0]) {
            return std::snprintf(buf, buflen,
                                 "[ERROR] %s:%d in %s(): [%s/%d] %s",
                                 file_, line_, func_,
                                 code_.category().name(),
                                 code_.value(),
                                 message());
        } else {
            return std::snprintf(buf, buflen,
                                 "[ERROR] [%s/%d] %s",
                                 code_.category().name(),
                                 code_.value(),
                                 message());
        }
    }

private:
    static void copy_str(char* dst, const char* src, size_t max_len) noexcept {
        if (!src) { dst[0] = '\0'; return; }
        size_t i = 0;
        while (src[i] && i < max_len - 1) {
            dst[i] = src[i];
            ++i;
        }
        dst[i] = '\0';
    }
    
    error_code code_;
    int        line_;
    char       message_[MAX_MESSAGE_LEN];
    char       file_[MAX_FILE_LEN];
    char       func_[MAX_FUNC_LEN];
};

// ============================================================================
// 辅助宏
// ============================================================================

/**
 * @brief 创建带源码位置的 error_context
 * 
 * 用法: auto ec = ZEN_MAKE_ERROR(zen::errc::not_found, "key not found");
 */
#define ZEN_MAKE_ERROR(code, msg) \
    ::zen::error_context((code), (msg), __FILE__, __LINE__, __func__)

/**
 * @brief 创建成功的 error_context
 */
#define ZEN_OK() ::zen::error_context()

/**
 * @brief 如果 ec 有错误，立即返回 ec（用于函数中的 early return）
 */
#define ZEN_RETURN_IF_ERROR(ec)  \
    do {                         \
        if (ec) { return ec; }   \
    } while(0)

} // namespace zen

#endif // ZEN_ERROR_ERROR_CONTEXT_H
