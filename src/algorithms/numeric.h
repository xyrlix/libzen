#ifndef ZEN_ALGORITHMS_NUMERIC_H
#define ZEN_ALGORITHMS_NUMERIC_H

#include "../base/type_traits.h"
#include "../utility/pair.h"
#include "../iterators/iterator_base.h"
#include "comparators.h"

namespace zen {

// ============================================================================
// 累加（accumulate / reduce）
// ============================================================================

/**
 * @brief 累加：计算区间所有元素的和（或通过 binary_op 组合）
 *
 * 复杂度：O(n)
 *
 * @tparam InputIt  输入迭代器
 * @tparam T        初始值/累加器类型
 * @tparam BinaryOp 二元操作符
 * @return 累加结果
 */
template<typename InputIt, typename T, typename BinaryOp>
T accumulate(InputIt first, InputIt last, T init, BinaryOp op) {
    for (; first != last; ++first) {
        init = op(init, *first);
    }
    return init;
}

/**
 * @brief 累加（默认使用加法）
 */
template<typename InputIt, typename T>
T accumulate(InputIt first, InputIt last, T init) {
    return zen::accumulate(first, last, init, [](const T& a, const T& b){ return a + b; });
}

// ============================================================================
// 内积（inner_product）
// ============================================================================

/**
 * @brief 内积：对应元素相乘后累加
 *
 * result = init + sum(a[i] * b[i])
 */
template<typename InputIt1, typename InputIt2, typename T>
T inner_product(InputIt1 first1, InputIt1 last1, InputIt2 first2, T init) {
    for (; first1 != last1; ++first1, ++first2) {
        init = init + (*first1 * *first2);
    }
    return init;
}

/**
 * @brief 内积（自定义操作符）
 */
template<typename InputIt1, typename InputIt2, typename T,
         typename BinaryOp1, typename BinaryOp2>
T inner_product(InputIt1 first1, InputIt1 last1, InputIt2 first2,
                T init, BinaryOp1 op1, BinaryOp2 op2) {
    for (; first1 != last1; ++first1, ++first2) {
        init = op1(init, op2(*first1, *first2));
    }
    return init;
}

// ============================================================================
// 相邻差（adjacent_difference）
// ============================================================================

/**
 * @brief 相邻差：计算每个元素与前一个元素的差
 *
 * d[0] = a[0]
 * d[i] = a[i] - a[i-1]
 */
template<typename InputIt, typename OutputIt, typename BinaryOp>
OutputIt adjacent_difference(InputIt first, InputIt last, OutputIt d_first, BinaryOp op) {
    if (first == last) return d_first;
    
    *d_first = *first;
    auto prev = *first;
    ++first; ++d_first;
    
    for (; first != last; ++first, ++d_first) {
        *d_first = op(*first, prev);
        prev = *first;
    }
    return d_first;
}

/**
 * @brief 相邻差（默认使用减法）
 */
template<typename InputIt, typename OutputIt>
OutputIt adjacent_difference(InputIt first, InputIt last, OutputIt d_first) {
    return zen::adjacent_difference(first, last, d_first, [](const auto& a, const auto& b){ return a - b; });
}

// ============================================================================
// 部分和（partial_sum）
// ============================================================================

/**
 * @brief 部分和：计算累计和
 *
 * s[0] = a[0]
 * s[i] = s[i-1] + a[i]
 */
template<typename InputIt, typename OutputIt, typename BinaryOp>
OutputIt partial_sum(InputIt first, InputIt last, OutputIt d_first, BinaryOp op) {
    if (first == last) return d_first;
    
    typename zen::iterator_traits<InputIt>::value_type acc = *first;
    *d_first = acc;
    ++first; ++d_first;
    
    for (; first != last; ++first, ++d_first) {
        acc = op(acc, *first);
        *d_first = acc;
    }
    return d_first;
}

/**
 * @brief 部分和（默认使用加法）
 */
template<typename InputIt, typename OutputIt>
OutputIt partial_sum(InputIt first, InputIt last, OutputIt d_first) {
    return zen::partial_sum(first, last, d_first, [](const auto& a, const auto& b){ return a + b; });
}

// ============================================================================
// 计数（iota）- 填充递增序列
// ============================================================================

/**
 * @brief 填充递增序列：从 value 开始，每次 ++value
 */
template<typename ForwardIt, typename T>
void iota(ForwardIt first, ForwardIt last, T value) {
    for (; first != last; ++first, ++value) {
        *first = value;
    }
}

// ============================================================================
// 最大最小
// ============================================================================

/**
 * @brief 返回区间最小元素
 */
template<typename ForwardIt>
ForwardIt min_element(ForwardIt first, ForwardIt last) {
    if (first == last) return last;
    ForwardIt min_pos = first;
    ++first;
    for (; first != last; ++first) {
        if (*first < *min_pos) min_pos = first;
    }
    return min_pos;
}

/**
 * @brief 返回区间最小元素（使用比较函数）
 */
template<typename ForwardIt, typename Compare>
ForwardIt min_element(ForwardIt first, ForwardIt last, Compare comp) {
    if (first == last) return last;
    ForwardIt min_pos = first;
    ++first;
    for (; first != last; ++first) {
        if (comp(*first, *min_pos)) min_pos = first;
    }
    return min_pos;
}

/**
 * @brief 返回区间最大元素
 */
template<typename ForwardIt>
ForwardIt max_element(ForwardIt first, ForwardIt last) {
    if (first == last) return last;
    ForwardIt max_pos = first;
    ++first;
    for (; first != last; ++first) {
        if (*max_pos < *first) max_pos = first;
    }
    return max_pos;
}

/**
 * @brief 返回区间最大元素（使用比较函数）
 */
template<typename ForwardIt, typename Compare>
ForwardIt max_element(ForwardIt first, ForwardIt last, Compare comp) {
    if (first == last) return last;
    ForwardIt max_pos = first;
    ++first;
    for (; first != last; ++first) {
        if (comp(*max_pos, *first)) max_pos = first;
    }
    return max_pos;
}

/**
 * @brief 返回区间最小和最大元素
 */
template<typename ForwardIt>
zen::pair<ForwardIt, ForwardIt> minmax_element(ForwardIt first, ForwardIt last) {
    if (first == last) return {first, first};
    ForwardIt min_pos = first, max_pos = first;
    ++first;
    bool min_set = false, max_set = false;
    while (first != last) {
        if (*first < *min_pos) { min_pos = first; min_set = true; }
        if (*max_pos < *first) { max_pos = first; max_set = true; }
        ++first;
    }
    return {min_pos, max_pos};
}

/**
 * @brief 返回最小值（不使用迭代器）
 */
template<typename T>
const T& min(const T& a, const T& b) {
    return (b < a) ? b : a;
}

/**
 * @brief 返回最大值（不使用迭代器）
 */
template<typename T>
const T& max(const T& a, const T& b) {
    return (a < b) ? b : a;
}

/**
 * @brief 返回三个值的最小值
 */
template<typename T>
const T& min(const T& a, const T& b, const T& c) {
    return zen::min(zen::min(a, b), c);
}

/**
 * @brief 返回三个值的最大值
 */
template<typename T>
const T& max(const T& a, const T& b, const T& c) {
    return zen::max(zen::max(a, b), c);
}

// ============================================================================
// 简单运算辅助
// ============================================================================

/**
 * @brief 加法函数对象
 */
template<typename T = void>
struct plus {
    constexpr T operator()(const T& a, const T& b) const { return a + b; }
};

/**
 * @brief 减法函数对象
 */
template<typename T = void>
struct minus {
    constexpr T operator()(const T& a, const T& b) const { return a - b; }
};

/**
 * @brief 乘法函数对象
 */
template<typename T = void>
struct multiplies {
    constexpr T operator()(const T& a, const T& b) const { return a * b; }
};

/**
 * @brief 除法函数对象
 */
template<typename T = void>
struct divides {
    constexpr T operator()(const T& a, const T& b) const { return a / b; }
};

/**
 * @brief 取模函数对象
 */
template<typename T = void>
struct modulus {
    constexpr T operator()(const T& a, const T& b) const { return a % b; }
};

/**
 * @brief 负数函数对象
 */
template<typename T = void>
struct negate {
    constexpr T operator()(const T& a) const { return -a; }
};

} // namespace zen

#endif // ZEN_ALGORITHMS_NUMERIC_H
