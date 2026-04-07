#ifndef ZEN_UTILITY_TUPLE_H
#define ZEN_UTILITY_TUPLE_H

#include "../base/type_traits.h"
#include "../base/compile_tools.h"
#include "swap.h"

namespace zen {

// ============================================================================
// tuple - 变参模板元组
// ============================================================================
//
// 实现策略：递归继承
//   tuple<int, double, char>
//     继承自 tuple_base<0, int>
//              tuple_base<1, double>
//              tuple_base<2, char>
//
// 通过 tuple_element_holder<N, T> 保存第 N 个元素，
// get<N>(t) 通过静态转型访问对应基类。
// ============================================================================

// ============================================================================
// 前向声明
// ============================================================================

template<typename... Types>
class tuple;

// ============================================================================
// tuple_element_holder - 保存第 N 个元素
// ============================================================================

/**
 * @brief 持有 tuple 中第 N 个位置元素的基类
 * @tparam N 元素在 tuple 中的索引
 * @tparam T 元素的类型
 *
 * 利用空基类优化（EBO）：若 T 是空类，编译器可能将其优化为 0 字节。
 * 通过不同的 N 区分相同类型的多个元素（避免多基类歧义）。
 */
template<size_t N, typename T>
struct tuple_element_holder {
    T value_; // 保存的元素值

    // 默认构造
    constexpr tuple_element_holder()
        : value_() {
    }

    // 从值构造（完美转发）
    template<typename U>
    constexpr explicit tuple_element_holder(U&& v)
        : value_(static_cast<U&&>(v)) {
    }

    // 拷贝/移动由编译器自动生成
};

// ============================================================================
// tuple_impl - 展开变参的递归实现基类
// ============================================================================

// 主模板声明（实际用带 index_sequence 的偏特化）
template<typename IndexSeq, typename... Types>
struct tuple_impl;

/**
 * @brief tuple 的递归实现，通过 index_sequence 展开类型列表
 *
 * 继承多个 tuple_element_holder<N, T> 基类，
 * 每个基类保存一个元素。
 */
template<size_t... Ns, typename... Types>
struct tuple_impl<index_sequence<Ns...>, Types...>
    : tuple_element_holder<Ns, Types>... {

    // 默认构造
    constexpr tuple_impl() = default;

    // 从参数列表构造（完美转发每个元素）
    template<typename... Us>
    constexpr explicit tuple_impl(Us&&... vs)
        : tuple_element_holder<Ns, Types>(static_cast<Us&&>(vs))... {
    }

    // 拷贝构造
    constexpr tuple_impl(const tuple_impl&) = default;

    // 移动构造
    constexpr tuple_impl(tuple_impl&&) = default;

    // 赋值
    tuple_impl& operator=(const tuple_impl&) = default;
    tuple_impl& operator=(tuple_impl&&) = default;
};

// ============================================================================
// tuple - 主模板
// ============================================================================

/**
 * @brief 固定大小的异质容器
 * @tparam Types 各元素类型
 *
 * tuple 是 pair 的泛化，可以存储任意数量、任意类型的元素。
 *
 * 内部通过 tuple_impl（多重继承）实现元素存储，
 * get<N>() 通过静态转型取到对应的基类来访问元素。
 *
 * 示例：
 * @code
 * zen::tuple<int, double, const char*> t(1, 3.14, "hello");
 * int    i = zen::get<0>(t);   // 1
 * double d = zen::get<1>(t);   // 3.14
 * auto [x, y, z] = t;          // 结构化绑定（C++17）
 * @endcode
 */
template<typename... Types>
class tuple
    : private tuple_impl<make_index_sequence<sizeof...(Types)>, Types...> {

    using base = tuple_impl<make_index_sequence<sizeof...(Types)>, Types...>;

    // 让 get<N> 函数能访问私有基类
    template<size_t N, typename... Ts>
    friend constexpr auto& get(tuple<Ts...>& t) noexcept;

    template<size_t N, typename... Ts>
    friend constexpr const auto& get(const tuple<Ts...>& t) noexcept;

    template<size_t N, typename... Ts>
    friend constexpr auto&& get(tuple<Ts...>&& t) noexcept;

public:
    // ========================================================================
    // 构造函数
    // ========================================================================

    /**
     * @brief 默认构造：值初始化所有元素
     */
    constexpr tuple() : base() {}

    /**
     * @brief 从参数列表构造
     */
    template<typename... Us>
    constexpr explicit tuple(Us&&... vs)
        : base(static_cast<Us&&>(vs)...) {
    }

    /**
     * @brief 拷贝构造
     */
    constexpr tuple(const tuple&) = default;

    /**
     * @brief 移动构造
     */
    constexpr tuple(tuple&&) noexcept = default;

    // ========================================================================
    // 赋值运算符
    // ========================================================================

    tuple& operator=(const tuple&) = default;
    tuple& operator=(tuple&&) noexcept = default;

    // ========================================================================
    // 大小查询
    // ========================================================================

    /**
     * @brief 返回元素个数（编译期常量）
     */
    static constexpr size_t size() noexcept {
        return sizeof...(Types);
    }
};

// 零元素特化
template<>
class tuple<> {
public:
    constexpr tuple() noexcept = default;
    static constexpr size_t size() noexcept { return 0; }
};

// ============================================================================
// get<N> - 按索引访问 tuple 元素
// ============================================================================

/**
 * @brief 获取 tuple 第 N 个元素的类型
 */
template<size_t N, typename... Types>
struct tuple_element;

// 递归：剥掉第一个类型，N 递减
template<size_t N, typename Head, typename... Tail>
struct tuple_element<N, Head, Tail...> {
    using type = typename tuple_element<N - 1, Tail...>::type;
};

// 基础情况：N == 0，取第一个类型
template<typename Head, typename... Tail>
struct tuple_element<0, Head, Tail...> {
    using type = Head;
};

// 方便的别名
template<size_t N, typename... Types>
using tuple_element_t = typename tuple_element<N, Types...>::type;

/**
 * @brief 左值引用版 get<N>
 */
template<size_t N, typename... Types>
constexpr auto& get(tuple<Types...>& t) noexcept {
    using T = tuple_element_t<N, Types...>;
    // 静态转型到保存第 N 个元素的基类，获取 value_
    return static_cast<tuple_element_holder<N, T>&>(
        static_cast<typename tuple<Types...>::base&>(t)
    ).value_;
}

/**
 * @brief const 左值引用版 get<N>
 */
template<size_t N, typename... Types>
constexpr const auto& get(const tuple<Types...>& t) noexcept {
    using T = tuple_element_t<N, Types...>;
    return static_cast<const tuple_element_holder<N, T>&>(
        static_cast<const typename tuple<Types...>::base&>(t)
    ).value_;
}

/**
 * @brief 右值引用版 get<N>
 */
template<size_t N, typename... Types>
constexpr auto&& get(tuple<Types...>&& t) noexcept {
    using T = tuple_element_t<N, Types...>;
    return static_cast<T&&>(
        static_cast<tuple_element_holder<N, T>&>(
            static_cast<typename tuple<Types...>::base&>(t)
        ).value_
    );
}

// ============================================================================
// make_tuple - 工厂函数
// ============================================================================

/**
 * @brief 创建 tuple，自动推导类型
 * @param args 各元素值
 * @return tuple
 *
 * 示例：
 * @code
 * auto t = zen::make_tuple(1, 3.14, "hi");
 * // 类型为 tuple<int, double, const char*>
 * @endcode
 */
template<typename... Types>
constexpr tuple<typename remove_reference<Types>::type...>
make_tuple(Types&&... args) {
    return tuple<typename remove_reference<Types>::type...>(
        static_cast<Types&&>(args)...
    );
}

// ============================================================================
// tuple_size - 编译期获取 tuple 大小
// ============================================================================

template<typename T>
struct tuple_size;

template<typename... Types>
struct tuple_size<tuple<Types...>> {
    static constexpr size_t value = sizeof...(Types);
};

template<typename T>
constexpr size_t tuple_size_v = tuple_size<T>::value;

// ============================================================================
// tuple 比较运算符
// ============================================================================

// 辅助：逐元素比较
template<size_t N, typename... Types>
struct tuple_equal_impl {
    static bool apply(const tuple<Types...>& a, const tuple<Types...>& b) {
        return get<N - 1>(a) == get<N - 1>(b)
            && tuple_equal_impl<N - 1, Types...>::apply(a, b);
    }
};

template<typename... Types>
struct tuple_equal_impl<0, Types...> {
    static bool apply(const tuple<Types...>&, const tuple<Types...>&) {
        return true;
    }
};

template<typename... Types>
bool operator==(const tuple<Types...>& a, const tuple<Types...>& b) {
    return tuple_equal_impl<sizeof...(Types), Types...>::apply(a, b);
}

template<typename... Types>
bool operator!=(const tuple<Types...>& a, const tuple<Types...>& b) {
    return !(a == b);
}

// 辅助：字典序比较
template<size_t N, size_t Total, typename... Types>
struct tuple_less_impl {
    static bool apply(const tuple<Types...>& a, const tuple<Types...>& b) {
        if (get<N>(a) < get<N>(b)) return true;
        if (get<N>(b) < get<N>(a)) return false;
        return tuple_less_impl<N + 1, Total, Types...>::apply(a, b);
    }
};

template<size_t Total, typename... Types>
struct tuple_less_impl<Total, Total, Types...> {
    static bool apply(const tuple<Types...>&, const tuple<Types...>&) {
        return false; // 完全相等
    }
};

template<typename... Types>
bool operator<(const tuple<Types...>& a, const tuple<Types...>& b) {
    return tuple_less_impl<0, sizeof...(Types), Types...>::apply(a, b);
}

template<typename... Types>
bool operator>(const tuple<Types...>& a, const tuple<Types...>& b) {
    return b < a;
}

template<typename... Types>
bool operator<=(const tuple<Types...>& a, const tuple<Types...>& b) {
    return !(b < a);
}

template<typename... Types>
bool operator>=(const tuple<Types...>& a, const tuple<Types...>& b) {
    return !(a < b);
}

} // namespace zen

#endif // ZEN_UTILITY_TUPLE_H
