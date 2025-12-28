#pragma once

#include <type_traits>

namespace zen {

/**
 * @brief 泛型约束工具类
 * 对标C++20 <concepts>，提供C++11/14兼容的泛型约束
 */
namespace concepts {

/**
 * @brief 检查类型是否为可复制构造的
 * @tparam T 待检查类型
 */
template <typename T>
struct is_copy_constructible : std::is_copy_constructible<T> {};

/**
 * @brief 检查类型是否为可移动构造的
 * @tparam T 待检查类型
 */
template <typename T>
struct is_move_constructible : std::is_move_constructible<T> {};

/**
 * @brief 检查类型是否为可复制赋值的
 * @tparam T 待检查类型
 */
template <typename T>
struct is_copy_assignable : std::is_copy_assignable<T> {};

/**
 * @brief 检查类型是否为可移动赋值的
 * @tparam T 待检查类型
 */
template <typename T>
struct is_move_assignable : std::is_move_assignable<T> {};

/**
 * @brief 检查类型是否为可默认构造的
 * @tparam T 待检查类型
 */
template <typename T>
struct is_default_constructible : std::is_default_constructible<T> {};

/**
 * @brief 检查类型是否为可析构的
 * @tparam T 待检查类型
 */
template <typename T>
struct is_destructible : std::is_destructible<T> {};

/**
 * @brief 检查类型是否为完整类型
 * @tparam T 待检查类型
 */
template <typename T>
struct is_complete {
private:
    template <typename U, size_t = sizeof(U)> static std::true_type check(U*);
    static std::false_type check(...);
public:
    static constexpr bool value = decltype(check(static_cast<T*>(nullptr)))::value;
};

/**
 * @brief 检查类型是否为算术类型
 * @tparam T 待检查类型
 */
template <typename T>
struct is_arithmetic : std::is_arithmetic<T> {};

/**
 * @brief 检查类型是否为整数类型
 * @tparam T 待检查类型
 */
template <typename T>
struct is_integral : std::is_integral<T> {};

/**
 * @brief 检查类型是否为浮点数类型
 * @tparam T 待检查类型
 */
template <typename T>
struct is_floating_point : std::is_floating_point<T> {};

/**
 * @brief 检查类型是否为指针类型
 * @tparam T 待检查类型
 */
template <typename T>
struct is_pointer : std::is_pointer<T> {};

/**
 * @brief 检查类型是否为引用类型
 * @tparam T 待检查类型
 */
template <typename T>
struct is_reference : std::is_reference<T> {};

/**
 * @brief 检查类型是否为左值引用
 * @tparam T 待检查类型
 */
template <typename T>
struct is_lvalue_reference : std::is_lvalue_reference<T> {};

/**
 * @brief 检查类型是否为右值引用
 * @tparam T 待检查类型
 */
template <typename T>
struct is_rvalue_reference : std::is_rvalue_reference<T> {};

/**
 * @brief 检查类型是否为数组类型
 * @tparam T 待检查类型
 */
template <typename T>
struct is_array : std::is_array<T> {};

/**
 * @brief 检查类型是否为函数类型
 * @tparam T 待检查类型
 */
template <typename T>
struct is_function : std::is_function<T> {};

/**
 * @brief 检查类型是否为类类型
 * @tparam T 待检查类型
 */
template <typename T>
struct is_class : std::is_class<T> {};

/**
 * @brief 检查类型是否为枚举类型
 * @tparam T 待检查类型
 */
template <typename T>
struct is_enum : std::is_enum<T> {};

/**
 * @brief 检查类型是否为联合类型
 * @tparam T 待检查类型
 */
template <typename T>
struct is_union : std::is_union<T> {};

/**
 * @brief 检查类型是否为可比较的（支持==和!=）
 * @tparam T 待检查类型
 */
template <typename T>
struct is_equality_comparable {
private:
    template <typename U>
    static auto check(U*) -> decltype(
        std::declval<U&>() == std::declval<U&>(),
        std::declval<U&>() != std::declval<U&>(),
        std::true_type{});
    static std::false_type check(...);
public:
    static constexpr bool value = decltype(check(static_cast<T*>(nullptr)))::value;
};

/**
 * @brief 检查类型是否为可排序的（支持<, <=, >, >=）
 * @tparam T 待检查类型
 */
template <typename T>
struct is_ordered {
private:
    template <typename U>
    static auto check(U*) -> decltype(
        std::declval<U&>() < std::declval<U&>(),
        std::declval<U&>() <= std::declval<U&>(),
        std::declval<U&>() > std::declval<U&>(),
        std::declval<U&>() >= std::declval<U&>(),
        std::true_type{});
    static std::false_type check(...);
public:
    static constexpr bool value = decltype(check(static_cast<T*>(nullptr)))::value;
};

/**
 * @brief 检查类型是否为可迭代的
 * @tparam T 待检查类型
 */
template <typename T>
struct is_iterable {
private:
    template <typename U>
    static auto check(U*) -> decltype(
        std::begin(std::declval<U&>()),
        std::end(std::declval<U&>()),
        std::true_type{});
    static std::false_type check(...);
public:
    static constexpr bool value = decltype(check(static_cast<T*>(nullptr)))::value;
};

/**
 * @brief 检查类型是否为可调用的
 * @tparam F 待检查类型
 * @tparam Args 参数类型
 */
template <typename F, typename... Args>
struct is_invocable {
private:
    template <typename U>
    static auto check(U*) -> decltype(
        std::declval<U>()(std::declval<Args>()...),
        std::true_type{});
    static std::false_type check(...);
public:
    static constexpr bool value = decltype(check(static_cast<F*>(nullptr)))::value;
};

/**
 * @brief 静态断言，用于检查类型约束
 * @tparam Condition 约束条件
 * @tparam Msg 错误信息
 */
template <bool Condition, typename Msg = void>
struct static_assertion {
    static_assert(Condition, "Type constraint violated");
    using type = void;
};

/**
 * @brief 类型约束检查宏
 * @param condition 约束条件
 */
#define ZEN_REQUIRES(condition) typename static_assertion<(condition)>::type

} // namespace concepts

} // namespace zen
