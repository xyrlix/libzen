/**
 * @file exception_chain.h
 * @brief 异常链（错误链）
 * 
 * 用链式结构记录错误传播路径：
 * - 每个节点是一个 error_context
 * - 新错误可以"包装"旧错误，形成因果链
 * - 可以遍历完整的错误调用链
 * 
 * 设计考量：
 * - 不使用异常机制（不依赖 try/catch）
 * - 适合不允许异常的场景
 * - 链表节点用简单数组存储（最多 MAX_DEPTH 层）
 * 
 * 示例：
 * @code
 * zen::error_chain chain;
 * chain.push(ZEN_MAKE_ERROR(zen::errc::io_error, "read failed"));
 * chain.push(ZEN_MAKE_ERROR(zen::errc::not_found, "config file missing"));
 * chain.print_all();
 * @endcode
 */
#ifndef ZEN_ERROR_EXCEPTION_CHAIN_H
#define ZEN_ERROR_EXCEPTION_CHAIN_H

#include "error_context.h"
#include <cstdio>

namespace zen {

// ============================================================================
// error_chain
// ============================================================================

/**
 * @brief 错误链
 * 
 * 以 LIFO 顺序存储一组 error_context，表示错误从底层到上层的传播链。
 * 最先发生的错误在链尾（最先被 push），最外层错误在链头（最后被 push）。
 */
class error_chain {
public:
    static constexpr size_t MAX_DEPTH = 16;
    
    /**
     * @brief 默认构造（空链）
     */
    error_chain() noexcept : size_(0) {}
    
    /**
     * @brief 从单个错误构造
     */
    explicit error_chain(const error_context& ctx) noexcept : size_(0) {
        push(ctx);
    }
    
    /**
     * @brief 从错误码直接构造
     */
    explicit error_chain(errc e, const char* msg = nullptr) noexcept : size_(0) {
        push(error_context(e, msg));
    }
    
    /**
     * @brief 将新错误压入链顶（最新发生的错误）
     * @param ctx 错误上下文
     * @return 自身引用（支持链式调用）
     */
    error_chain& push(const error_context& ctx) noexcept {
        if (size_ < MAX_DEPTH) {
            entries_[size_++] = ctx;
        }
        return *this;
    }
    
    /**
     * @brief 从链顶弹出最新错误
     */
    error_chain& pop() noexcept {
        if (size_ > 0) --size_;
        return *this;
    }
    
    /**
     * @brief 是否为空（无任何错误）
     */
    bool empty() const noexcept { return size_ == 0; }
    
    /**
     * @brief 链中错误数量
     */
    size_t size() const noexcept { return size_; }
    
    /**
     * @brief 最新的错误（链顶）
     */
    const error_context& top() const noexcept {
        static error_context empty_ctx;
        return size_ > 0 ? entries_[size_ - 1] : empty_ctx;
    }
    
    /**
     * @brief 最早的错误（根本原因，链底）
     */
    const error_context& root_cause() const noexcept {
        static error_context empty_ctx;
        return size_ > 0 ? entries_[0] : empty_ctx;
    }
    
    /**
     * @brief 按索引访问（0 = 最早）
     */
    const error_context& operator[](size_t i) const noexcept {
        static error_context empty_ctx;
        return i < size_ ? entries_[i] : empty_ctx;
    }
    
    /**
     * @brief 是否有错误（链不为空且顶层有错）
     */
    explicit operator bool() const noexcept {
        return size_ > 0 && static_cast<bool>(top());
    }
    
    /**
     * @brief 清空错误链
     */
    void clear() noexcept { size_ = 0; }
    
    /**
     * @brief 打印完整错误链到 stderr
     * 
     * 格式示例：
     * @code
     * Error chain (3 entries):
     *   [0] Root cause: [generic/30] I/O error (file.cpp:42 in open())
     *   [1] Caused by:  [generic/4]  Not found (loader.cpp:88 in load_config())
     *   [2] Final:      [generic/2]  Invalid argument (main.cpp:15 in main())
     * @endcode
     */
    void print_all() const noexcept {
        if (size_ == 0) {
            std::fprintf(stderr, "Error chain: (empty)\n");
            return;
        }
        
        std::fprintf(stderr, "Error chain (%zu %s):\n",
                     size_, size_ == 1 ? "entry" : "entries");
        
        for (size_t i = 0; i < size_; ++i) {
            const char* prefix;
            if      (i == 0 && size_ == 1) prefix = "Error:      ";
            else if (i == 0)               prefix = "Root cause: ";
            else if (i == size_ - 1)       prefix = "Final:      ";
            else                           prefix = "Caused by:  ";
            
            char buf[512];
            entries_[i].format(buf, sizeof(buf));
            std::fprintf(stderr, "  [%zu] %s%s\n", i, prefix, buf);
        }
    }
    
    /**
     * @brief 打印链顶（最新）错误
     */
    void print() const noexcept {
        top().print();
    }
    
    /**
     * @brief 合并另一条错误链（追加到末尾）
     */
    error_chain& merge(const error_chain& other) noexcept {
        for (size_t i = 0; i < other.size_ && size_ < MAX_DEPTH; ++i) {
            entries_[size_++] = other.entries_[i];
        }
        return *this;
    }

private:
    error_context entries_[MAX_DEPTH];
    size_t        size_;
};

// ============================================================================
// result<T>：携带值或错误的返回类型
// ============================================================================

/**
 * @brief 类似 Rust Result<T, E> 的返回类型
 * 
 * 要么包含正常值 T，要么包含错误 error_context。
 * 避免使用异常，提供明确的错误处理路径。
 * 
 * 示例：
 * @code
 * zen::result<int> parse_int(const char* str) {
 *     int val = 0;
 *     // ... 解析 ...
 *     if (failed)
 *         return zen::result<int>::err(zen::errc::invalid_argument, "not a number");
 *     return zen::result<int>::ok(val);
 * }
 * 
 * auto r = parse_int("42");
 * if (r) {
 *     printf("value = %d\n", *r);
 * } else {
 *     r.error().print();
 * }
 * @endcode
 */
template<typename T>
class result {
public:
    /**
     * @brief 构造成功结果
     */
    static result ok(const T& value) noexcept {
        result r;
        r.has_value_ = true;
        r.value_     = value;
        return r;
    }
    
    static result ok(T&& value) noexcept {
        result r;
        r.has_value_ = true;
        r.value_     = static_cast<T&&>(value);
        return r;
    }
    
    /**
     * @brief 构造错误结果
     */
    static result err(const error_context& ctx) noexcept {
        result r;
        r.has_value_ = false;
        r.ctx_       = ctx;
        return r;
    }
    
    static result err(errc e, const char* msg = nullptr) noexcept {
        return err(error_context(e, msg));
    }
    
    static result err(error_code code, const char* msg = nullptr) noexcept {
        return err(error_context(code, msg));
    }
    
    // ---- 访问 ----
    
    /**
     * @brief 是否成功
     */
    explicit operator bool() const noexcept { return has_value_; }
    bool is_ok()  const noexcept { return has_value_; }
    bool is_err() const noexcept { return !has_value_; }
    
    /**
     * @brief 获取值（确保成功时调用）
     */
    T& value() noexcept { return value_; }
    const T& value() const noexcept { return value_; }
    T& operator*()  noexcept { return value_; }
    const T& operator*() const noexcept { return value_; }
    T* operator->() noexcept { return &value_; }
    const T* operator->() const noexcept { return &value_; }
    
    /**
     * @brief 获取错误上下文（确保失败时调用）
     */
    const error_context& error() const noexcept { return ctx_; }
    
    /**
     * @brief 获取值，失败时返回默认值
     */
    T value_or(const T& default_val) const noexcept {
        return has_value_ ? value_ : default_val;
    }

private:
    result() noexcept : has_value_(false) {}
    
    bool          has_value_;
    T             value_{};
    error_context ctx_;
};

/**
 * @brief result<void> 特化（用于只需要成功/失败的函数）
 */
template<>
class result<void> {
public:
    static result ok() noexcept {
        result r;
        r.has_value_ = true;
        return r;
    }
    
    static result err(const error_context& ctx) noexcept {
        result r;
        r.has_value_ = false;
        r.ctx_       = ctx;
        return r;
    }
    
    static result err(errc e, const char* msg = nullptr) noexcept {
        return err(error_context(e, msg));
    }
    
    explicit operator bool() const noexcept { return has_value_; }
    bool is_ok()  const noexcept { return has_value_; }
    bool is_err() const noexcept { return !has_value_; }
    
    const error_context& error() const noexcept { return ctx_; }

private:
    result() noexcept : has_value_(false) {}
    bool          has_value_;
    error_context ctx_;
};

} // namespace zen

#endif // ZEN_ERROR_EXCEPTION_CHAIN_H
