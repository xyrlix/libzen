#ifndef ZEN_UTILITY_SWAP_H
#define ZEN_UTILITY_SWAP_H

#include "../base/type_traits.h"

// swap.h 完全自实现，不依赖任何系统头文件

namespace zen {

// ============================================================================
// Move - 移动语义
// ============================================================================

/**
 * @brief 将左值转换为右值引用
 * @tparam T 类型
 * @param t 要移动的值
 * @return T&&
 *
 * move 是实现移动语义的核心工具，它告诉编译器可以将对象视为临时对象，
 * 从而启用移动构造函数和移动赋值运算符。
 *
 * 原理：
 * - 对于左值引用 T&，move 返回 T&&（右值引用）
 * - 对于右值引用 T&&，move 返回 T&&（保持不变）
 *
 * 使用场景：
 * - 当我们知道对象不再需要时，可以将其资源"窃取"给另一个对象
 * - 避免不必要的拷贝，提高性能
 *
 * 示例：
 * @code
 * std::string str1 = "Hello";
 * std::string str2 = std::move(str1);  // str1 的资源被移动到 str2
 * // 此时 str1 处于有效但未定义的状态
 * @endcode
 */
template<typename T>
constexpr typename remove_reference<T>::type&&
move(T&& t) noexcept {
    return static_cast<typename remove_reference<T>::type&&>(t);
}

// ============================================================================
// Forward - 完美转发
// ============================================================================

/**
 * @brief 完美转发
 * @tparam T 类型
 * @param t 要转发的值
 * @return T&&
 *
 * forward 是实现完美转发的核心工具，它保留了参数的值类别（左值或右值）。
 *
 * 原理：
 * - 如果是左值 T&，forward 返回 T&（左值引用）
 * - 如果是右值 T&&，forward 返回 T&&（右值引用）
 *
 * 使用场景：
 * - 在模板函数中，需要将参数按原始类别传递给其他函数
 * - 配合 universal reference (T&&) 使用
 *
 * 示例：
 * @code
 * template<typename T>
 * void wrapper(T&& arg) {
 *     // 完美转发 arg 到 target 函数
 *     target(std::forward<T>(arg));
 * }
 * @endcode
 */
template<typename T>
constexpr T&&
forward(typename remove_reference<T>::type& t) noexcept {
    return static_cast<T&&>(t);
}

template<typename T>
constexpr T&&
forward(typename remove_reference<T>::type&& t) noexcept {
    static_assert(!is_lvalue_reference<T>::value,
                  "cannot forward an rvalue as an lvalue");
    return static_cast<T&&>(t);
}

// ============================================================================
// Swap - 交换
// ============================================================================

/**
 * @brief 交换两个对象的值
 * @tparam T 类型
 * @param a 第一个对象
 * @param b 第二个对象
 *
 * swap 通过移动语义高效地交换两个对象的值。
 *
 * 实现原理：
 * 1. 创建临时对象，移动构造 a 的值
 * 2. 将 b 的值移动赋值给 a
 * 3. 将临时对象的值移动赋值给 b
 *
 * 复杂度：
 * - 如果类型支持移动语义：O(1)（只涉及指针或资源交换）
 * - 如果类型只支持拷贝语义：O(n)（需要拷贝数据）
 *
 * 异常安全：
 * - 如果移动操作是 noexcept，则 swap 也是 noexcept
 *
 * 示例：
 * @code
 * int a = 1, b = 2;
 * zen::swap(a, b);  // a = 2, b = 1
 *
 * std::string s1 = "hello", s2 = "world";
 * zen::swap(s1, s2);  // s1 = "world", s2 = "hello"
 * @endcode
 */
template<typename T>
void swap(T& a, T& b) noexcept(
    noexcept(zen::move(a)) &&
    noexcept(zen::move(b))
) {
    T temp = zen::move(a);
    a = zen::move(b);
    b = zen::move(temp);
}

// ============================================================================
// Swap 数组版本
// ============================================================================

/**
 * @brief 交换两个数组的内容
 * @tparam T 数组元素类型
 * @tparam N 数组大小
 * @param a 第一个数组
 * @param b 第二个数组
 *
 * 逐元素交换数组内容
 *
 * 复杂度：O(n)，其中 n 是数组大小
 */
template<typename T, size_t N>
void swap(T (&a)[N], T (&b)[N]) noexcept(noexcept(zen::swap(*a, *b))) {
    for (size_t i = 0; i < N; ++i) {
        zen::swap(a[i], b[i]);
    }
}

// ============================================================================
// Move_if_noexcept - 条件移动
// ============================================================================

// ============================================================================
// is_nothrow_move_constructible - 编译期 noexcept 检测
// ============================================================================

/**
 * @brief 检测类型的移动构造函数是否为 noexcept
 */
template<typename T>
struct is_nothrow_move_constructible {
    static constexpr bool value = noexcept(T(zen::move(*(T*)nullptr)));
};

// ============================================================================
// Move_if_noexcept - 条件移动
// ============================================================================

/**
 * @brief 如果类型的移动构造函数是 noexcept，则移动，否则拷贝
 * @tparam T 类型
 * @param x 要移动或拷贝的值
 * @return 根据条件选择移动或拷贝
 *
 * 这是一个用于实现强异常安全保证的工具。
 * 当移动操作可能抛出异常时，使用拷贝操作代替。
 */
template<typename T>
constexpr typename conditional<
    is_nothrow_move_constructible<T>::value,
    T&&,
    const T&
>::type
move_if_noexcept(T& x) noexcept {
    return zen::move(x);
}

// ============================================================================
// 辅助类型
// ============================================================================

/**
 * @brief 删除器基类（用于智能指针）
 *
 * 注意：此基类版本与 memory/unique_ptr.h 中的模板 default_delete 不同，
 * 仅作为非模板的通用删除器基类。
 */
struct default_delete_base {
    /**
     * @brief 删除指针
     * @tparam T 指针类型
     * @param ptr 要删除的指针
     */
    template<typename T>
    void operator()(T* ptr) const noexcept {
        static_assert(!is_void<T>::value,
                      "cannot delete void pointer");
        static_assert(sizeof(T) > 0,
                      "cannot delete incomplete type");
        delete ptr;
    }
};

// ============================================================================
// 类型别名
// ============================================================================

// move 的类型别名
template<typename T>
using move_t = typename remove_reference<T>::type&&;

// forward 的类型别名
template<typename T>
using forward_t = T&&;

} // namespace zen

#endif // ZEN_UTILITY_SWAP_H