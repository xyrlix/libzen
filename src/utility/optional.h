#ifndef ZEN_UTILITY_OPTIONAL_H
#define ZEN_UTILITY_OPTIONAL_H

#include "../base/type_traits.h"
#include "swap.h"
// placement new 需要 <new>
#include <new>

namespace zen {

// ============================================================================
// nullopt_t / nullopt - 表示"无值"的标记类型
// ============================================================================

/**
 * @brief 表示 optional 无值状态的标记类型
 *
 * 类似 nullptr_t 对 nullptr 的关系，
 * nullopt_t 是 nullopt 的类型。
 */
struct nullopt_t {
    // 显式构造，防止被意外隐式使用
    explicit constexpr nullopt_t(int) noexcept {}
};

/**
 * @brief 用于构造空 optional 的常量
 *
 * 示例：
 * @code
 * zen::optional<int> opt = zen::nullopt;
 * @endcode
 */
inline constexpr nullopt_t nullopt{0};

// ============================================================================
// bad_optional_access - 访问空 optional 时的异常
// ============================================================================

// 简单异常标记（不依赖 std::exception）
struct bad_optional_access {};

// ============================================================================
// optional_storage - 内部存储
// ============================================================================

/**
 * @brief optional 的存储基类
 * @tparam T 存储的值类型
 *
 * 使用 union 实现：
 * - 若有值：union 中的 value_ 成员处于活跃状态
 * - 若无值：union 中的 dummy_ 成员处于活跃状态（零大小占位符）
 *
 * 这样 T 的构造函数只在"设值"时才被调用，
 * 析构函数由 optional 手动控制。
 */
template<typename T>
struct optional_storage {
    // 存储联合体（手动生命周期管理）
    union {
        char dummy_; // 无值时的占位，不触发 T 的构造
        T    value_; // 有值时的实际存储
    };
    bool has_value_; // 是否有值

    // 默认构造：无值状态
    constexpr optional_storage() noexcept
        : dummy_('\0'), has_value_(false) {
    }

    // 有值构造（完美转发）
    template<typename... Args>
    explicit optional_storage(Args&&... args)
        : has_value_(true) {
        // placement new 在 value_ 上构造 T
        ::new(static_cast<void*>(&value_)) T(static_cast<Args&&>(args)...);
    }

    // 析构函数（若有值，手动析构）
    ~optional_storage() {
        if (has_value_) {
            value_.~T();
        }
    }
};

// ============================================================================
// optional - 可选值包装器
// ============================================================================

/**
 * @brief 可选值容器（可能包含也可能不包含一个 T 类型的值）
 * @tparam T 包含的值类型（不能是引用、数组或 void）
 *
 * optional 表示一个"可能存在"的值，比裸指针或特殊哨兵值更安全。
 *
 * 主要特点：
 * - 无值时不构造 T 对象（节省资源）
 * - 提供类型安全的"有值/无值"检查
 * - 支持解引用、value_or、比较等操作
 *
 * 使用场景：
 * - 函数可能无返回值（替代 return nullptr 或特殊值）
 * - 延迟初始化
 * - 表示"没有找到"的搜索结果
 *
 * 示例：
 * @code
 * zen::optional<int> find_first_even(const int* arr, int n) {
 *     for (int i = 0; i < n; ++i) {
 *         if (arr[i] % 2 == 0) return arr[i];  // 有值
 *     }
 *     return zen::nullopt;  // 无值
 * }
 *
 * auto result = find_first_even(data, 10);
 * if (result) {
 *     printf("found: %d\n", *result);
 * }
 * int v = result.value_or(-1);  // 若无值则用 -1
 * @endcode
 */
template<typename T>
class optional {
private:
    optional_storage<T> storage_;

    // ========================================================================
    // 内部辅助
    // ========================================================================

    /**
     * @brief 在存储上原地构造 T
     */
    template<typename... Args>
    void construct(Args&&... args) {
        ::new(static_cast<void*>(&storage_.value_)) T(static_cast<Args&&>(args)...);
        storage_.has_value_ = true;
    }

    /**
     * @brief 析构当前持有的值
     */
    void destroy() noexcept {
        if (storage_.has_value_) {
            storage_.value_.~T();
            storage_.has_value_ = false;
        }
    }

public:
    // ========================================================================
    // 类型定义
    // ========================================================================

    using value_type = T;

    // ========================================================================
    // 构造函数
    // ========================================================================

    /**
     * @brief 默认构造：无值状态
     */
    constexpr optional() noexcept
        : storage_() {
    }

    /**
     * @brief 从 nullopt 构造：无值状态
     */
    constexpr optional(nullopt_t) noexcept
        : storage_() {
    }

    /**
     * @brief 从值拷贝构造：有值状态
     */
    optional(const T& value) {
        construct(value);
    }

    /**
     * @brief 从值移动构造：有值状态
     */
    optional(T&& value) {
        construct(static_cast<T&&>(value));
    }

    /**
     * @brief 拷贝构造
     */
    optional(const optional& other) {
        if (other.has_value()) {
            construct(other.storage_.value_);
        }
        // 否则 storage_ 默认构造，has_value_ = false
    }

    /**
     * @brief 移动构造
     */
    optional(optional&& other) noexcept {
        if (other.has_value()) {
            construct(static_cast<T&&>(other.storage_.value_));
            // 移动后 other 保持有值但值未定义（符合移动语义）
        }
    }

    /**
     * @brief 析构
     */
    ~optional() {
        destroy();
    }

    // ========================================================================
    // 赋值运算符
    // ========================================================================

    /**
     * @brief 赋值为无值
     */
    optional& operator=(nullopt_t) noexcept {
        destroy();
        return *this;
    }

    /**
     * @brief 从值赋值
     */
    optional& operator=(const T& value) {
        if (has_value()) {
            storage_.value_ = value;
        } else {
            construct(value);
        }
        return *this;
    }

    /**
     * @brief 从值移动赋值
     */
    optional& operator=(T&& value) {
        if (has_value()) {
            storage_.value_ = static_cast<T&&>(value);
        } else {
            construct(static_cast<T&&>(value));
        }
        return *this;
    }

    /**
     * @brief 拷贝赋值
     */
    optional& operator=(const optional& other) {
        if (this == &other) return *this;

        if (other.has_value()) {
            if (has_value()) {
                storage_.value_ = other.storage_.value_;
            } else {
                construct(other.storage_.value_);
            }
        } else {
            destroy();
        }
        return *this;
    }

    /**
     * @brief 移动赋值
     */
    optional& operator=(optional&& other) noexcept {
        if (this == &other) return *this;

        if (other.has_value()) {
            if (has_value()) {
                storage_.value_ = static_cast<T&&>(other.storage_.value_);
            } else {
                construct(static_cast<T&&>(other.storage_.value_));
            }
        } else {
            destroy();
        }
        return *this;
    }

    // ========================================================================
    // 观察者
    // ========================================================================

    /**
     * @brief 检查是否有值
     */
    bool has_value() const noexcept {
        return storage_.has_value_;
    }

    /**
     * @brief 显式 bool 转换
     */
    explicit operator bool() const noexcept {
        return has_value();
    }

    /**
     * @brief 解引用（有值时有效）
     * @pre has_value() == true
     */
    T& operator*() & noexcept {
        return storage_.value_;
    }

    const T& operator*() const& noexcept {
        return storage_.value_;
    }

    T&& operator*() && noexcept {
        return static_cast<T&&>(storage_.value_);
    }

    /**
     * @brief 成员访问（有值时有效）
     * @pre has_value() == true
     */
    T* operator->() noexcept {
        return &storage_.value_;
    }

    const T* operator->() const noexcept {
        return &storage_.value_;
    }

    /**
     * @brief 安全取值（若无值，抛出 bad_optional_access）
     */
    T& value() & {
        if (!has_value()) {
            // 不依赖 std::throw，直接用 throw 关键字
            throw bad_optional_access{};
        }
        return storage_.value_;
    }

    const T& value() const& {
        if (!has_value()) {
            throw bad_optional_access{};
        }
        return storage_.value_;
    }

    T&& value() && {
        if (!has_value()) {
            throw bad_optional_access{};
        }
        return static_cast<T&&>(storage_.value_);
    }

    /**
     * @brief 若无值，返回默认值
     * @param default_val 无值时的默认返回值
     */
    template<typename U>
    T value_or(U&& default_val) const& {
        if (has_value()) {
            return storage_.value_;
        }
        return static_cast<T>(static_cast<U&&>(default_val));
    }

    template<typename U>
    T value_or(U&& default_val) && {
        if (has_value()) {
            return static_cast<T&&>(storage_.value_);
        }
        return static_cast<T>(static_cast<U&&>(default_val));
    }

    // ========================================================================
    // 修改器
    // ========================================================================

    /**
     * @brief 重置为无值
     */
    void reset() noexcept {
        destroy();
    }

    /**
     * @brief 原地构造值
     * @param args 构造函数参数
     * @return 引用新构造的值
     */
    template<typename... Args>
    T& emplace(Args&&... args) {
        destroy();
        construct(static_cast<Args&&>(args)...);
        return storage_.value_;
    }

    /**
     * @brief 交换两个 optional
     */
    void swap(optional& other) noexcept {
        if (has_value() && other.has_value()) {
            zen::swap(storage_.value_, other.storage_.value_);
        } else if (has_value()) {
            other.construct(static_cast<T&&>(storage_.value_));
            destroy();
        } else if (other.has_value()) {
            construct(static_cast<T&&>(other.storage_.value_));
            other.destroy();
        }
    }
};

// ============================================================================
// make_optional - 工厂函数
// ============================================================================

/**
 * @brief 创建含有值的 optional
 * @param value 初始值
 * @return optional<T>
 */
template<typename T>
optional<typename remove_reference<T>::type>
make_optional(T&& value) {
    return optional<typename remove_reference<T>::type>(
        static_cast<T&&>(value)
    );
}

template<typename T, typename... Args>
optional<T> make_optional(Args&&... args) {
    optional<T> opt;
    opt.emplace(static_cast<Args&&>(args)...);
    return opt;
}

// ============================================================================
// 比较运算符
// ============================================================================

// optional == optional
template<typename T>
bool operator==(const optional<T>& a, const optional<T>& b) {
    if (a.has_value() != b.has_value()) return false;
    if (!a.has_value()) return true; // 两者都无值
    return *a == *b;
}

template<typename T>
bool operator!=(const optional<T>& a, const optional<T>& b) {
    return !(a == b);
}

template<typename T>
bool operator<(const optional<T>& a, const optional<T>& b) {
    if (!b.has_value()) return false;
    if (!a.has_value()) return true;
    return *a < *b;
}

// optional == nullopt
template<typename T>
bool operator==(const optional<T>& a, nullopt_t) noexcept {
    return !a.has_value();
}

template<typename T>
bool operator==(nullopt_t, const optional<T>& a) noexcept {
    return !a.has_value();
}

template<typename T>
bool operator!=(const optional<T>& a, nullopt_t) noexcept {
    return a.has_value();
}

// optional == T
template<typename T>
bool operator==(const optional<T>& a, const T& val) {
    return a.has_value() && (*a == val);
}

template<typename T>
bool operator==(const T& val, const optional<T>& a) {
    return a.has_value() && (val == *a);
}

template<typename T>
bool operator!=(const optional<T>& a, const T& val) {
    return !a.has_value() || (*a != val);
}

// 非成员 swap
template<typename T>
void swap(optional<T>& a, optional<T>& b) noexcept {
    a.swap(b);
}

} // namespace zen

#endif // ZEN_UTILITY_OPTIONAL_H
