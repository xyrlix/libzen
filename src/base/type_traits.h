#ifndef ZEN_BASE_TYPE_TRAITS_H
#define ZEN_BASE_TYPE_TRAITS_H

// size_t 是 C 语言级别的基础类型，在 <cstddef> 中定义（不属于 std:: 命名空间下的 C++ 库）
#include <cstddef>

namespace zen {

// ============================================================================
// 基础类型判断 Traits
// ============================================================================

/**
 * @brief 判断类型是否为整数类型
 * @tparam T 要检查的类型
 */
template<typename T>
struct is_integral {
    static constexpr bool value = false;
};

// 特化：所有整数类型
template<> struct is_integral<bool> { static constexpr bool value = true; };
template<> struct is_integral<char> { static constexpr bool value = true; };
template<> struct is_integral<signed char> { static constexpr bool value = true; };
template<> struct is_integral<unsigned char> { static constexpr bool value = true; };
template<> struct is_integral<wchar_t> { static constexpr bool value = true; };
template<> struct is_integral<char16_t> { static constexpr bool value = true; };
template<> struct is_integral<char32_t> { static constexpr bool value = true; };
template<> struct is_integral<short> { static constexpr bool value = true; };
template<> struct is_integral<unsigned short> { static constexpr bool value = true; };
template<> struct is_integral<int> { static constexpr bool value = true; };
template<> struct is_integral<unsigned int> { static constexpr bool value = true; };
template<> struct is_integral<long> { static constexpr bool value = true; };
template<> struct is_integral<unsigned long> { static constexpr bool value = true; };
template<> struct is_integral<long long> { static constexpr bool value = true; };
template<> struct is_integral<unsigned long long> { static constexpr bool value = true; };

/**
 * @brief 判断类型是否为浮点类型
 */
template<typename T>
struct is_floating_point {
    static constexpr bool value = false;
};

template<> struct is_floating_point<float> { static constexpr bool value = true; };
template<> struct is_floating_point<double> { static constexpr bool value = true; };
template<> struct is_floating_point<long double> { static constexpr bool value = true; };

/**
 * @brief 判断类型是否为指针
 * @tparam T 要检查的类型
 *
 * 使用模板特化检测指针类型
 */
template<typename T>
struct is_pointer {
    static constexpr bool value = false;
};

template<typename T>
struct is_pointer<T*> {
    static constexpr bool value = true;
};

/**
 * @brief 判断类型是否为左值引用
 */
template<typename T>
struct is_lvalue_reference {
    static constexpr bool value = false;
};

template<typename T>
struct is_lvalue_reference<T&> {
    static constexpr bool value = true;
};

/**
 * @brief 判断类型是否为右值引用
 */
template<typename T>
struct is_rvalue_reference {
    static constexpr bool value = false;
};

template<typename T>
struct is_rvalue_reference<T&&> {
    static constexpr bool value = true;
};

/**
 * @brief 判断类型是否为引用（左值或右值）
 */
template<typename T>
struct is_reference {
    static constexpr bool value = is_lvalue_reference<T>::value ||
                                   is_rvalue_reference<T>::value;
};

/**
 * @brief 判断类型是否为 const 限定
 */
template<typename T>
struct is_const {
    static constexpr bool value = false;
};

template<typename T>
struct is_const<const T> {
    static constexpr bool value = true;
};

/**
 * @brief 判断类型是否为 volatile 限定
 */
template<typename T>
struct is_volatile {
    static constexpr bool value = false;
};

template<typename T>
struct is_volatile<volatile T> {
    static constexpr bool value = true;
};

/**
 * @brief 判断类型是否为 void
 */
template<typename T>
struct is_void {
    static constexpr bool value = false;
};

template<>
struct is_void<void> {
    static constexpr bool value = true;
};

/**
 * @brief 判断类型是否为 null 指针
 */
template<typename T>
struct is_null_pointer {
    static constexpr bool value = false;
};

template<>
struct is_null_pointer<decltype(nullptr)> {
    static constexpr bool value = true;
};

// ============================================================================
// 类型转换 Traits
// ============================================================================

/**
 * @brief 移除类型的引用
 *
 * remove_reference<int&>::type => int
 * remove_reference<int&&>::type => int
 * remove_reference<int>::type => int
 */
template<typename T>
struct remove_reference {
    using type = T;
};

template<typename T>
struct remove_reference<T&> {
    using type = T;
};

template<typename T>
struct remove_reference<T&&> {
    using type = T;
};

/**
 * @brief 移除类型的 const 限定
 */
template<typename T>
struct remove_const {
    using type = T;
};

template<typename T>
struct remove_const<const T> {
    using type = T;
};

/**
 * @brief 移除类型的 volatile 限定
 */
template<typename T>
struct remove_volatile {
    using type = T;
};

template<typename T>
struct remove_volatile<volatile T> {
    using type = T;
};

/**
 * @brief 移除类型的 const 和 volatile 限定
 */
template<typename T>
struct remove_cv {
    using type = typename remove_volatile<typename remove_const<T>::type>::type;
};

/**
 * @brief 移除类型的指针
 */
template<typename T>
struct remove_pointer {
    using type = T;
};

template<typename T>
struct remove_pointer<T*> {
    using type = T;
};

template<typename T>
struct remove_pointer<T* const> {
    using type = T;
};

template<typename T>
struct remove_pointer<T* volatile> {
    using type = T;
};

template<typename T>
struct remove_pointer<T* const volatile> {
    using type = T;
};

/**
 * @brief 添加指针
 */
template<typename T>
struct add_pointer {
    using type = typename remove_reference<T>::type*;
};

/**
 * @brief 添加左值引用
 */
template<typename T>
struct add_lvalue_reference {
    using type = T&;
};

template<typename T>
struct add_lvalue_reference<T&> {
    using type = T&;
};

/**
 * @brief 添加右值引用
 */
template<typename T>
struct add_rvalue_reference {
    using type = T&&;
};

// ============================================================================
// 类型关系 Traits
// ============================================================================

/**
 * @brief 判断两个类型是否相同
 */
template<typename T, typename U>
struct is_same {
    static constexpr bool value = false;
};

template<typename T>
struct is_same<T, T> {
    static constexpr bool value = true;
};

/**
 * @brief 判断类型是否为数组
 */
template<typename T>
struct is_array {
    static constexpr bool value = false;
};

template<typename T, size_t N>
struct is_array<T[N]> {
    static constexpr bool value = true;
};

template<typename T>
struct is_array<T[]> {
    static constexpr bool value = true;
};

/**
 * @brief 获取数组元素类型
 */
template<typename T>
struct remove_extent {
    using type = T;
};

template<typename T, size_t N>
struct remove_extent<T[N]> {
    using type = T;
};

template<typename T>
struct remove_extent<T[]> {
    using type = T;
};

/**
 * @brief 判断类型是否为类（非 union）
 */
template<typename T>
struct is_class {
private:
    // 使用 SFINAE 检测是否是类类型
    template<typename U>
    static constexpr auto test(int) -> decltype(sizeof(U) != 0, int()) {
        return 1;
    }

    template<typename U>
    static constexpr auto test(...) -> char {
        return 0;
    }

public:
    static constexpr bool value = !is_integral<T>::value &&
                                   !is_floating_point<T>::value &&
                                   !is_pointer<T>::value &&
                                   !is_reference<T>::value &&
                                   !is_void<T>::value &&
                                   !is_array<T>::value;
};

// ============================================================================
// 辅助类型别名
// ============================================================================

// 提取 value 为 true/false
template<bool B, typename T = void>
struct enable_if {};

template<typename T>
struct enable_if<true, T> {
    using type = T;
};

// 条件类型选择
template<bool B, typename T, typename F>
struct conditional {
    using type = T;
};

template<typename T, typename F>
struct conditional<false, T, F> {
    using type = F;
};

// 常用类型别名简写
template<typename T>
using remove_reference_t = typename remove_reference<T>::type;

template<typename T>
using remove_cv_t = typename remove_cv<T>::type;

/**
 * @brief 移除引用和 cv 限定
 * 
 * remove_cvref<int&>::type => int
 * remove_cvref<const int>::type => int
 * remove_cvref<const int&>::type => int
 */
template<typename T>
struct remove_cvref {
    using type = typename remove_cv<typename remove_reference<T>::type>::type;
};

template<typename T>
using remove_cvref_t = typename remove_cvref<T>::type;

template<typename T>
using remove_pointer_t = typename remove_pointer<T>::type;

template<typename T>
using add_pointer_t = typename add_pointer<T>::type;

template<bool B, typename T = void>
using enable_if_t = typename enable_if<B, T>::type;

template<bool B, typename T, typename F>
using conditional_t = typename conditional<B, T, F>::type;

// 值类型简写
template<typename T>
constexpr bool is_integral_v = is_integral<T>::value;

template<typename T>
constexpr bool is_floating_point_v = is_floating_point<T>::value;

template<typename T>
constexpr bool is_pointer_v = is_pointer<T>::value;

template<typename T>
constexpr bool is_reference_v = is_reference<T>::value;

template<typename T>
constexpr bool is_const_v = is_const<T>::value;

template<typename T>
constexpr bool is_void_v = is_void<T>::value;

template<typename T, typename U>
constexpr bool is_same_v = is_same<T, U>::value;

template<typename T>
constexpr bool is_array_v = is_array<T>::value;

// ============================================================================
// 符号判断
// ============================================================================

/**
 * @brief 判断是否为有符号类型
 * 
 * 包括：signed char, short, int, long, long long
 */
template<typename T>
struct is_signed {
    static constexpr bool value = false;
};

template<>
struct is_signed<signed char> {
    static constexpr bool value = true;
};

template<>
struct is_signed<short> {
    static constexpr bool value = true;
};

template<>
struct is_signed<int> {
    static constexpr bool value = true;
};

template<>
struct is_signed<long> {
    static constexpr bool value = true;
};

template<>
struct is_signed<long long> {
    static constexpr bool value = true;
};

template<>
struct is_signed<float> {
    static constexpr bool value = true;
};

template<>
struct is_signed<double> {
    static constexpr bool value = true;
};

template<>
struct is_signed<long double> {
    static constexpr bool value = true;
};

/**
 * @brief 判断是否为无符号类型
 * 
 * 包括：unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long
 */
template<typename T>
struct is_unsigned {
    static constexpr bool value = false;
};

template<>
struct is_unsigned<unsigned char> {
    static constexpr bool value = true;
};

template<>
struct is_unsigned<unsigned short> {
    static constexpr bool value = true;
};

template<>
struct is_unsigned<unsigned int> {
    static constexpr bool value = true;
};

template<>
struct is_unsigned<unsigned long> {
    static constexpr bool value = true;
};

template<>
struct is_unsigned<unsigned long long> {
    static constexpr bool value = true;
};

// 值简写
template<typename T>
constexpr bool is_signed_v = is_signed<T>::value;

template<typename T>
constexpr bool is_unsigned_v = is_unsigned<T>::value;

} // namespace zen

#endif // ZEN_BASE_TYPE_TRAITS_H