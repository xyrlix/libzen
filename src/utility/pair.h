#ifndef ZEN_UTILITY_PAIR_H
#define ZEN_UTILITY_PAIR_H

#include "../base/type_traits.h"
#include "../base/compile_tools.h"

namespace zen {

// ============================================================================
// Pair - 键值对容器
// ============================================================================

/**
 * @brief 键值对容器
 * @tparam T1 第一个元素类型
 * @tparam T2 第二个元素类型
 *
 * pair 是一个简单的容器，用于存储两个可能不同类型的值。
 * 它是 map、unordered_map 等关联容器的核心组件。
 *
 * 特性：
 * - 支持拷贝语义和移动语义
 * - 支持比较运算符
 * - 可以使用结构化绑定
 *
 * 示例：
 * @code
 * pair<int, string> p(1, "hello");
 * pair<int, string> p2 = make_pair(2, "world");
 * auto [first, second] = p;  // 结构化绑定
 * @endcode
 */
template<typename T1, typename T2>
struct pair {
    // 类型定义
    using first_type = T1;
    using second_type = T2;

    // 成员变量
    T1 first;   // 第一个元素
    T2 second;  // 第二个元素

    // ========================================================================
    // 构造函数
    // ========================================================================

    /**
     * @brief 默认构造函数
     *
     * 使用值初始化两个元素
     */
    constexpr pair()
        : first(), second() {
    }

    /**
     * @brief 拷贝构造函数
     */
    constexpr pair(const pair& other) = default;

    /**
     * @brief 移动构造函数
     */
    constexpr pair(pair&& other) noexcept = default;

    /**
     * @brief 从其他类型的 pair 拷贝构造
     */
    template<typename U1, typename U2>
    constexpr pair(const pair<U1, U2>& other)
        : first(other.first), second(other.second) {
    }

    /**
     * @brief 从其他类型的 pair 移动构造
     */
    template<typename U1, typename U2>
    constexpr pair(pair<U1, U2>&& other)
        : first(zen::move(other.first)), second(zen::move(other.second)) {
    }

    /**
     * @brief 从两个值构造（拷贝）
     */
    constexpr pair(const T1& x, const T2& y)
        : first(x), second(y) {
    }

    /**
     * @brief 从两个值构造（移动）
     */
    constexpr pair(T1&& x, T2&& y)
        noexcept(noexcept(T1(zen::move(x))) && noexcept(T2(zen::move(y))))
        : first(zen::move(x)), second(zen::move(y)) {
    }

    /**
     * @brief 从两个不同类型的值构造（完美转发）
     */
    template<typename U1, typename U2>
    constexpr pair(U1&& x, U2&& y)
        : first(zen::forward<U1>(x)), second(zen::forward<U2>(y)) {
    }

    // ========================================================================
    // 赋值运算符
    // ========================================================================

    /**
     * @brief 拷贝赋值运算符
     */
    pair& operator=(const pair& other) = default;

    /**
     * @brief 移动赋值运算符
     */
    pair& operator=(pair&& other) noexcept = default;

    /**
     * @brief 从其他类型的 pair 拷贝赋值
     */
    template<typename U1, typename U2>
    pair& operator=(const pair<U1, U2>& other) {
        first = other.first;
        second = other.second;
        return *this;
    }

    /**
     * @brief 从其他类型的 pair 移动赋值
     */
    template<typename U1, typename U2>
    pair& operator=(pair<U1, U2>&& other) {
        first = zen::move(other.first);
        second = zen::move(other.second);
        return *this;
    }

    // ========================================================================
    // 比较运算符
    // ========================================================================

    /**
     * @brief 相等比较
     */
    constexpr bool operator==(const pair& other) const {
        return first == other.first && second == other.second;
    }

    /**
     * @brief 不等比较
     */
    constexpr bool operator!=(const pair& other) const {
        return !(*this == other);
    }

    /**
     * @brief 小于比较
     *
     * 先比较 first，如果 first 相等，再比较 second
     */
    constexpr bool operator<(const pair& other) const {
        if (first < other.first) return true;
        if (other.first < first) return false;
        return second < other.second;
    }

    /**
     * @brief 大于比较
     */
    constexpr bool operator>(const pair& other) const {
        return other < *this;
    }

    /**
     * @brief 小于等于比较
     */
    constexpr bool operator<=(const pair& other) const {
        return !(other < *this);
    }

    /**
     * @brief 大于等于比较
     */
    constexpr bool operator>=(const pair& other) const {
        return !(*this < other);
    }

    /**
     * @brief 交换两个 pair 的内容
     */
    void swap(pair& other) {
        zen::swap(first, other.first);
        zen::swap(second, other.second);
    }
};

// ============================================================================
// 工厂函数
// ============================================================================

/**
 * @brief 创建 pair 的工厂函数
 * @tparam T1 第一个元素类型
 * @tparam T2 第二个元素类型
 * @param x 第一个元素
 * @param y 第二个元素
 * @return pair<T1, T2>
 *
 * 使用类型推导自动确定 pair 的类型，避免了手动指定类型
 *
 * 示例：
 * @code
 * auto p = make_pair(42, "hello");  // 类型为 pair<int, const char*>
 * auto p2 = make_pair(3.14, std::string("world"));  // 类型为 pair<double, std::string>
 * @endcode
 */
template<typename T1, typename T2>
constexpr pair<typename remove_reference<T1>::type, typename remove_reference<T2>::type>
make_pair(T1&& x, T2&& y) {
    return pair<typename remove_reference<T1>::type, typename remove_reference<T2>::type>(
        zen::forward<T1>(x),
        zen::forward<T2>(y)
    );
}

// ============================================================================
// 结构化绑定支持（C++17）
// ============================================================================

// 前向声明
template<typename T>
struct tuple_size;

/**
 * @brief 获取 pair 的元素数量（结构化绑定需要）
 */
template<typename T1, typename T2>
struct tuple_size<pair<T1, T2>> {
    static constexpr size_t value = 2;
};

/**
 * @brief 获取 pair 的第 I 个元素类型（结构化绑定需要）
 * 前向声明（变参版本，与 tuple.h 兼容）
 */
template<size_t I, typename... Types>
struct tuple_element;

template<typename T1, typename T2>
struct tuple_element<0, pair<T1, T2>> {
    using type = T1;
};

template<typename T1, typename T2>
struct tuple_element<1, pair<T1, T2>> {
    using type = T2;
};

/**
 * @brief 获取 pair 的元素（结构化绑定需要）
 */
template<size_t I, typename T1, typename T2>
constexpr auto& get(pair<T1, T2>& p) noexcept {
    static_assert(I < 2, "pair index out of bounds");
    if constexpr (I == 0) {
        return p.first;
    } else {
        return p.second;
    }
}

template<size_t I, typename T1, typename T2>
constexpr const auto& get(const pair<T1, T2>& p) noexcept {
    static_assert(I < 2, "pair index out of bounds");
    if constexpr (I == 0) {
        return p.first;
    } else {
        return p.second;
    }
}

template<size_t I, typename T1, typename T2>
constexpr auto&& get(pair<T1, T2>&& p) noexcept {
    static_assert(I < 2, "pair index out of bounds");
    if constexpr (I == 0) {
        return zen::move(p.first);
    } else {
        return zen::move(p.second);
    }
}

// ============================================================================
// 类型别名
// ============================================================================

// 常用 pair 类型别名
template<typename T1, typename T2>
using pair_t = pair<T1, T2>;

} // namespace zen

#endif // ZEN_UTILITY_PAIR_H