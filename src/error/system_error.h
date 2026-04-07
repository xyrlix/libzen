/**
 * @file system_error.h
 * @brief 错误码与系统错误封装
 * 
 * 提供类似 C++11 <system_error> 的错误码机制：
 * - error_code: 轻量级错误码（值 + 类别）
 * - error_category: 错误类别抽象基类
 * - 内置通用错误码（generic_error_code）
 * 
 * 设计原则：
 * - 不抛异常，适合嵌入式/高性能场景
 * - 零开销（无堆分配）
 * - 可扩展（自定义 error_category）
 */
#ifndef ZEN_ERROR_SYSTEM_ERROR_H
#define ZEN_ERROR_SYSTEM_ERROR_H

#include <cstring>
#include <cstdio>

namespace zen {

// ============================================================================
// 错误类别基类
// ============================================================================

/**
 * @brief 错误类别抽象基类
 * 
 * 每种错误来源对应一个 error_category 单例。
 * 继承此类可以定义自定义错误域。
 */
class error_category {
public:
    virtual ~error_category() = default;
    
    /**
     * @brief 返回错误类别名称
     */
    virtual const char* name() const noexcept = 0;
    
    /**
     * @brief 将错误码转为人类可读的描述字符串
     * @param code 错误码值
     */
    virtual const char* message(int code) const noexcept = 0;
    
    bool operator==(const error_category& other) const noexcept {
        return this == &other;
    }
    
    bool operator!=(const error_category& other) const noexcept {
        return this != &other;
    }
};

// ============================================================================
// 内置通用错误码
// ============================================================================

/**
 * @brief 通用错误枚举
 */
enum class errc {
    success = 0,

    // 通用错误
    unknown                  = 1,
    invalid_argument         = 2,
    out_of_range             = 3,
    not_found                = 4,
    already_exists           = 5,
    permission_denied        = 6,
    not_supported            = 7,
    timeout                  = 8,
    resource_unavailable     = 9,
    overflow                 = 10,
    underflow                = 11,
    null_pointer             = 12,
    index_out_of_bounds      = 13,
    buffer_overflow          = 14,
    
    // 内存相关
    out_of_memory            = 20,
    bad_alloc                = 21,
    
    // IO 相关
    io_error                 = 30,
    end_of_file              = 31,
    broken_pipe              = 32,
    would_block              = 33,
    
    // 线程相关
    deadlock                 = 40,
    operation_not_permitted  = 41,
    interrupted              = 42,
};

/**
 * @brief 通用错误类别
 */
class generic_category_impl : public error_category {
public:
    const char* name() const noexcept override {
        return "generic";
    }
    
    const char* message(int code) const noexcept override {
        switch (static_cast<errc>(code)) {
            case errc::success:               return "Success";
            case errc::unknown:               return "Unknown error";
            case errc::invalid_argument:      return "Invalid argument";
            case errc::out_of_range:          return "Out of range";
            case errc::not_found:             return "Not found";
            case errc::already_exists:        return "Already exists";
            case errc::permission_denied:     return "Permission denied";
            case errc::not_supported:         return "Not supported";
            case errc::timeout:               return "Timeout";
            case errc::resource_unavailable:  return "Resource unavailable";
            case errc::overflow:              return "Overflow";
            case errc::underflow:             return "Underflow";
            case errc::null_pointer:          return "Null pointer";
            case errc::index_out_of_bounds:   return "Index out of bounds";
            case errc::buffer_overflow:       return "Buffer overflow";
            case errc::out_of_memory:         return "Out of memory";
            case errc::bad_alloc:             return "Bad allocation";
            case errc::io_error:              return "I/O error";
            case errc::end_of_file:           return "End of file";
            case errc::broken_pipe:           return "Broken pipe";
            case errc::would_block:           return "Would block";
            case errc::deadlock:              return "Deadlock";
            case errc::operation_not_permitted: return "Operation not permitted";
            case errc::interrupted:           return "Interrupted";
            default:                          return "Unknown error code";
        }
    }
};

/**
 * @brief 获取通用错误类别单例
 */
inline const error_category& generic_category() noexcept {
    static generic_category_impl instance;
    return instance;
}

// ============================================================================
// error_code
// ============================================================================

/**
 * @brief 轻量级错误码
 * 
 * 存储一个整型错误值 + 指向错误类别的指针。
 * 设计为值类型，可以零开销传递。
 * 
 * 用法示例：
 * @code
 * zen::error_code ec = zen::make_error_code(zen::errc::not_found);
 * if (ec) {
 *     printf("Error: %s\n", ec.message());
 * }
 * @endcode
 */
class error_code {
public:
    /**
     * @brief 默认构造（无错误）
     */
    error_code() noexcept
        : value_(0), category_(&generic_category()) {}
    
    /**
     * @brief 从整型值和错误类别构造
     */
    error_code(int value, const error_category& cat) noexcept
        : value_(value), category_(&cat) {}
    
    /**
     * @brief 从 errc 枚举构造
     */
    explicit error_code(errc e) noexcept
        : value_(static_cast<int>(e)), category_(&generic_category()) {}
    
    /**
     * @brief 清除错误（置为成功）
     */
    void clear() noexcept {
        value_ = 0;
        category_ = &generic_category();
    }
    
    /**
     * @brief 赋值
     */
    void assign(int value, const error_category& cat) noexcept {
        value_ = value;
        category_ = &cat;
    }
    
    /**
     * @brief 错误码值
     */
    int value() const noexcept { return value_; }
    
    /**
     * @brief 所属错误类别
     */
    const error_category& category() const noexcept { return *category_; }
    
    /**
     * @brief 返回错误描述字符串
     */
    const char* message() const noexcept {
        return category_->message(value_);
    }
    
    /**
     * @brief 转为 bool，非零值（有错误）为 true
     */
    explicit operator bool() const noexcept {
        return value_ != 0;
    }
    
    bool operator==(const error_code& other) const noexcept {
        return value_ == other.value_ && category_ == other.category_;
    }
    
    bool operator!=(const error_code& other) const noexcept {
        return !(*this == other);
    }
    
    bool operator<(const error_code& other) const noexcept {
        if (category_ != other.category_) {
            return category_ < other.category_;
        }
        return value_ < other.value_;
    }

private:
    int value_;
    const error_category* category_;
};

/**
 * @brief 工厂函数：从 errc 创建 error_code
 */
inline error_code make_error_code(errc e) noexcept {
    return error_code(static_cast<int>(e), generic_category());
}

/**
 * @brief 表示"成功"的 error_code
 */
inline error_code ok() noexcept {
    return error_code();
}

// ============================================================================
// error_condition（逻辑错误条件，用于跨平台比较）
// ============================================================================

/**
 * @brief 错误条件（逻辑层面的错误，可跨平台比较）
 * 
 * error_code 表示具体的错误值（与平台/来源相关），
 * error_condition 表示抽象的错误语义（与平台无关）。
 */
class error_condition {
public:
    error_condition() noexcept
        : value_(0), category_(&generic_category()) {}
    
    error_condition(int value, const error_category& cat) noexcept
        : value_(value), category_(&cat) {}
    
    explicit error_condition(errc e) noexcept
        : value_(static_cast<int>(e)), category_(&generic_category()) {}
    
    int value() const noexcept { return value_; }
    const error_category& category() const noexcept { return *category_; }
    const char* message() const noexcept { return category_->message(value_); }
    
    explicit operator bool() const noexcept { return value_ != 0; }
    
    bool operator==(const error_condition& other) const noexcept {
        return value_ == other.value_ && category_ == other.category_;
    }
    
    bool operator!=(const error_condition& other) const noexcept {
        return !(*this == other);
    }

private:
    int value_;
    const error_category* category_;
};

inline error_condition make_error_condition(errc e) noexcept {
    return error_condition(static_cast<int>(e), generic_category());
}

} // namespace zen

#endif // ZEN_ERROR_SYSTEM_ERROR_H
