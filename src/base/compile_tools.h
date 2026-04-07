#ifndef ZEN_BASE_COMPILE_TOOLS_H
#define ZEN_BASE_COMPILE_TOOLS_H

#include "type_traits.h"

// compile_tools.h 完全独立，不依赖任何系统头文件

namespace zen {

// ============================================================================
// 编译期整数序列
// ============================================================================

/**
 * @brief 编译期整数序列
 * @tparam T 整数类型
 * @tparam Iseq 整数序列值
 *
 * 用于实现变参模板的索引序列
 * 例如：index_sequence<0, 1, 2> 表示 0, 1, 2 的序列
 */
template<typename T, T... Iseq>
struct integer_sequence {
    /**
     * @brief 序列中元素的个数
     */
    static constexpr size_t size() noexcept {
        return sizeof...(Iseq);
    }

    /**
     * @brief 序列类型
     */
    using value_type = T;
};

/**
 * @brief 生成索引序列（size_t 的特化）
 * @tparam Iseq 整数序列
 */
template<size_t... Iseq>
using index_sequence = integer_sequence<size_t, Iseq...>;

// ============================================================================
// 整数序列生成器（递归实现）
// ============================================================================

// 主模板：生成 0 到 N-1 的序列
template<typename T, size_t N, T... Iseq>
struct make_integer_sequence_impl : make_integer_sequence_impl<T, N - 1, N - 1, Iseq...> {
    // 递归展开，每次在前方添加一个数
    // make_integer_sequence_impl<5> ->
    // make_integer_sequence_impl<4, 4> ->
    // make_integer_sequence_impl<3, 3, 4> ->
    // make_integer_sequence_impl<2, 2, 3, 4> ->
    // make_integer_sequence_impl<1, 1, 2, 3, 4> ->
    // make_integer_sequence_impl<0, 0, 1, 2, 3, 4> -> 基础模板
};

// 基础模板：递归终止条件，生成 0 的序列
template<typename T, T... Iseq>
struct make_integer_sequence_impl<T, 0, Iseq...> {
    using type = integer_sequence<T, Iseq...>;
};

/**
 * @brief 生成 0 到 N-1 的整数序列
 * @tparam T 整数类型
 * @tparam N 序列长度
 *
 * 示例：
 * make_integer_sequence<int, 3>::type => integer_sequence<int, 0, 1, 2>
 */
template<typename T, T N>
using make_integer_sequence = typename make_integer_sequence_impl<T, N>::type;

/**
 * @brief 生成索引序列的便捷别名
 * @tparam N 序列长度
 *
 * 示例：
 * make_index_sequence<3>::type => index_sequence<0, 1, 2>
 */
template<size_t N>
using make_index_sequence = make_integer_sequence<size_t, N>;

/**
 * @brief 为类型包生成索引序列
 * @tparam Types 类型包
 *
 * 示例：
 * index_sequence_for<int, double, char>::type => index_sequence<0, 1, 2>
 */
template<typename... Types>
using index_sequence_for = make_index_sequence<sizeof...(Types)>;

// ============================================================================
// 编译期常量
// ============================================================================

/**
 * @brief 编译期布尔常量
 */
template<bool B>
struct bool_constant {
    static constexpr bool value = B;

    constexpr operator bool() const noexcept { return value; }

    constexpr bool operator()() const noexcept { return value; }
};

// 便捷别名
using true_type = bool_constant<true>;
using false_type = bool_constant<false>;

// ============================================================================
// 编译期整数序列的辅助工具
// ============================================================================

/**
 * @brief 获取整数序列的第 N 个元素（编译期）
 * @tparam Seq 整数序列
 * @tparam N 索引
 *
 * 示例：
 * get_element<index_sequence<10, 20, 30>, 1>::value => 20
 */
template<typename Seq, size_t N>
struct get_element;

// 递归展开：先将参数包转换为可索引形式
// 辅助：从参数包中取第 I 个值
template<size_t I, size_t First, size_t... Rest>
struct nth_value_impl {
    static constexpr size_t value = nth_value_impl<I - 1, Rest...>::value;
};

template<size_t First, size_t... Rest>
struct nth_value_impl<0, First, Rest...> {
    static constexpr size_t value = First;
};

template<size_t... Iseq, size_t N>
struct get_element<index_sequence<Iseq...>, N> {
    /**
     * @brief 递归展开参数包，取第 N 个元素
     */
    static constexpr size_t value = nth_value_impl<N, Iseq...>::value;
};

// ============================================================================
// 编译期类型检查
// ============================================================================

/**
 * @brief 检查 From 类型是否可以隐式转换为 To 类型
 * @tparam From 源类型
 * @tparam To 目标类型
 *
 * 使用 SFINAE 技术：通过函数重载决议检测类型转换的可能性。
 */
template<typename From, typename To>
struct is_convertible {
private:
    // 如果 From 能隐式转换为 To，test(0) 可匹配
    template<typename F, typename T>
    static auto test(int) -> decltype(
        static_cast<void>(static_cast<T>(static_cast<F(*)()>(nullptr)())),
        true_type{}
    );

    template<typename, typename>
    static false_type test(...);

public:
    static constexpr bool value = decltype(test<From, To>(0))::value;
};

// ============================================================================
// 编译期递归终止检查
// ============================================================================

/**
 * @brief 检查是否为终止条件
 */
template<typename T>
struct is_terminal : false_type {};

// 如果是空指针类型，则认为是终止条件
template<>
struct is_terminal<decltype(nullptr)> : true_type {};

// ============================================================================
// 编译期类型列表
// ============================================================================

// 前向声明，供 type_list_nth 使用
template<typename... Types>
struct type_list;

/**
 * @brief 辅助结构：从类型列表取第 N 个类型（class-scope 特化 GCC 不允许，改为顶层）
 */
template<size_t N, typename... Types>
struct type_list_nth;

// 递归：剥掉第一个类型
template<size_t N, typename Head, typename... Tail>
struct type_list_nth<N, Head, Tail...> {
    using type = typename type_list_nth<N - 1, Tail...>::type;
};

// 基础情况：N==0，返回第一个类型
template<typename Head, typename... Tail>
struct type_list_nth<0, Head, Tail...> {
    using type = Head;
};

/**
 * @brief 类型列表（用于变参模板）
 * @tparam Types 类型包
 */
template<typename... Types>
struct type_list {
    /**
     * @brief 类型列表长度
     */
    static constexpr size_t size() noexcept {
        return sizeof...(Types);
    }

    /**
     * @brief 是否为空
     */
    static constexpr bool empty() noexcept {
        return sizeof...(Types) == 0;
    }

    /**
     * @brief 获取第 N 个类型
     */
    template<size_t N>
    using nth_type = type_list_nth<N, Types...>;

    /**
     * @brief 在头部添加类型
     */
    template<typename T>
    using push_front = type_list<T, Types...>;

    /**
     * @brief 在尾部添加类型
     */
    template<typename T>
    using push_back = type_list<Types..., T>;
};

// ============================================================================
// 编译期数学运算
// ============================================================================

// ============================================================================
// 编译期整型极值（不依赖 <limits>）
// ============================================================================

/**
 * @brief 整型极值辅助（仅 size_t 特化，供 max_value/min_value 使用）
 */
template<typename T>
struct integral_limits {
    // 通用实现：全 1 位表示无符号最大值
    static constexpr T max_val = static_cast<T>(-1); // 无符号类型
    static constexpr T min_val = static_cast<T>(0);
};

/**
 * @brief 编译期最大值
 */
template<typename T, T... Values>
struct max_value;

template<typename T>
struct max_value<T> {
    static constexpr T value = integral_limits<T>::min_val;
};

template<typename T, T First>
struct max_value<T, First> {
    static constexpr T value = First;
};

template<typename T, T First, T Second, T... Rest>
struct max_value<T, First, Second, Rest...> {
    static constexpr T value = (First >= Second) ?
        max_value<T, First, Rest...>::value :
        max_value<T, Second, Rest...>::value;
};

/**
 * @brief 编译期最小值
 */
template<typename T, T... Values>
struct min_value;

template<typename T>
struct min_value<T> {
    static constexpr T value = integral_limits<T>::max_val;
};

template<typename T, T First>
struct min_value<T, First> {
    static constexpr T value = First;
};

template<typename T, T First, T Second, T... Rest>
struct min_value<T, First, Second, Rest...> {
    static constexpr T value = (First <= Second) ?
        min_value<T, First, Rest...>::value :
        min_value<T, Second, Rest...>::value;
};

// ============================================================================
// 编译期选择
// ============================================================================

/**
 * @brief 编译期选择第 N 个值
 */
template<size_t N, typename T, T First, T... Rest>
struct select_value {
    static constexpr T value = select_value<N - 1, T, Rest...>::value;
};

template<typename T, T First, T... Rest>
struct select_value<0, T, First, Rest...> {
    static constexpr T value = First;
};

// ============================================================================
// 编译期元组辅助（类型安全的变参容器）
// ============================================================================

/**
 * @brief 编译期获取类型大小
 */
template<typename T>
struct type_size {
    static constexpr size_t value = sizeof(T);
};

/**
 * @brief 编译期获取类型对齐
 */
template<typename T>
struct type_alignment {
    static constexpr size_t value = alignof(T);
};

// ============================================================================
// 便捷别名
// ============================================================================

// 类型列表别名
template<typename... Types>
using type_list_t = type_list<Types...>;

// 整数序列别名
template<typename T, T... Iseq>
using integer_sequence_t = integer_sequence<T, Iseq...>;

template<size_t... Iseq>
using index_sequence_t = index_sequence<Iseq...>;

// 常量别名
template<bool B>
constexpr bool bool_constant_v = bool_constant<B>::value;

// is_convertible 值别名
template<typename From, typename To>
constexpr bool is_convertible_v = is_convertible<From, To>::value;

} // namespace zen

#endif // ZEN_BASE_COMPILE_TOOLS_H