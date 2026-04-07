#ifndef ZEN_ALGORITHMS_TRANSFORM_H
#define ZEN_ALGORITHMS_TRANSFORM_H

#include "../base/type_traits.h"

namespace zen {

// ============================================================================
// for_each（应用函数到每个元素）
// ============================================================================

/**
 * @brief 对区间每个元素应用函数
 *
 * 复杂度：O(n)
 * @return 函数对象（可用来捕获状态）
 */
template<typename InputIt, typename UnaryFunc>
UnaryFunc for_each(InputIt first, InputIt last, UnaryFunc f) {
    for (; first != last; ++first) {
        f(*first);
    }
    return f;
}

// ============================================================================
// transform（转换区间元素）
// ============================================================================

/**
 * @brief 一元转换：对输入区间的每个元素应用一元函数，写入输出区间
 *
 * d[i] = op(s[i])
 */
template<typename InputIt, typename OutputIt, typename UnaryOp>
OutputIt transform(InputIt first, InputIt last, OutputIt d_first, UnaryOp op) {
    for (; first != last; ++first, ++d_first) {
        *d_first = op(*first);
    }
    return d_first;
}

/**
 * @brief 二元转换：对两个输入区间的元素应用二元函数，写入输出区间
 *
 * d[i] = op(s1[i], s2[i])
 */
template<typename InputIt1, typename InputIt2, typename OutputIt, typename BinaryOp>
OutputIt transform(InputIt1 first1, InputIt1 last1,
                    InputIt2 first2, OutputIt d_first, BinaryOp op) {
    for (; first1 != last1; ++first1, ++first2, ++d_first) {
        *d_first = op(*first1, *first2);
    }
    return d_first;
}

// ============================================================================
// copy（拷贝区间）
// ============================================================================

/**
 * @brief 拷贝区间到输出区间
 */
template<typename InputIt, typename OutputIt>
OutputIt copy(InputIt first, InputIt last, OutputIt d_first) {
    for (; first != last; ++first, ++d_first) {
        *d_first = *first;
    }
    return d_first;
}

/**
 * @brief 条件拷贝：拷贝满足条件的元素
 */
template<typename InputIt, typename OutputIt, typename UnaryPred>
OutputIt copy_if(InputIt first, InputIt last, OutputIt d_first, UnaryPred pred) {
    for (; first != last; ++first) {
        if (pred(*first)) {
            *d_first = *first;
            ++d_first;
        }
    }
    return d_first;
}

/**
 * @brief 逆序拷贝
 */
template<typename BidirIt, typename OutputIt>
OutputIt copy_backward(BidirIt first, BidirIt last, OutputIt d_last) {
    while (first != last) {
        *(--d_last) = *(--last);
    }
    return d_last;
}

// ============================================================================
// move（移动区间）
// ============================================================================

/**
 * @brief 移动区间到输出区间
 */
template<typename InputIt, typename OutputIt>
OutputIt move(InputIt first, InputIt last, OutputIt d_first) {
    for (; first != last; ++first, ++d_first) {
        *d_first = zen::move(*first);
    }
    return d_first;
}

/**
 * @brief 逆序移动
 */
template<typename BidirIt, typename OutputIt>
OutputIt move_backward(BidirIt first, BidirIt last, OutputIt d_last) {
    while (first != last) {
        *(--d_last) = zen::move(*(--last));
    }
    return d_last;
}

// ============================================================================
// fill（填充区间）
// ============================================================================

/**
 * @brief 用指定值填充区间
 */
template<typename ForwardIt, typename T>
void fill(ForwardIt first, ForwardIt last, const T& value) {
    for (; first != last; ++first) {
        *first = value;
    }
}

/**
 * @brief 生成填充序列
 */
template<typename ForwardIt, typename Generator>
void generate(ForwardIt first, ForwardIt last, Generator g) {
    for (; first != last; ++first) {
        *first = g();
    }
}

/**
 * @brief 填充 n 个元素
 */
template<typename OutputIt, typename Size, typename T>
OutputIt fill_n(OutputIt first, Size n, const T& value) {
    for (Size i = 0; i < n; ++i, ++first) {
        *first = value;
    }
    return first;
}

/**
 * @brief 生成 n 个元素
 */
template<typename OutputIt, typename Size, typename Generator>
OutputIt generate_n(OutputIt first, Size n, Generator g) {
    for (Size i = 0; i < n; ++i, ++first) {
        *first = g();
    }
    return first;
}

// ============================================================================
// replace（替换）
// ============================================================================

/**
 * @brief 替换所有等于 old_value 的元素为 new_value
 */
template<typename ForwardIt, typename T>
void replace(ForwardIt first, ForwardIt last,
             const T& old_value, const T& new_value) {
    for (; first != last; ++first) {
        if (*first == old_value) {
            *first = new_value;
        }
    }
}

/**
 * @brief 条件替换
 */
template<typename ForwardIt, typename UnaryPred, typename T>
void replace_if(ForwardIt first, ForwardIt last,
                UnaryPred pred, const T& new_value) {
    for (; first != last; ++first) {
        if (pred(*first)) {
            *first = new_value;
        }
    }
}

/**
 * @brief 替换并拷贝到输出区间
 */
template<typename InputIt, typename OutputIt, typename T>
OutputIt replace_copy(InputIt first, InputIt last, OutputIt d_first,
                       const T& old_value, const T& new_value) {
    for (; first != last; ++first, ++d_first) {
        *d_first = (*first == old_value) ? new_value : *first;
    }
    return d_first;
}

// ============================================================================
// remove（移除）
// ============================================================================

/**
 * @brief 移除所有等于 value 的元素（不真正删除，只是移动到末尾）
 * @return 新的逻辑结束位置迭代器
 */
template<typename ForwardIt, typename T>
ForwardIt remove(ForwardIt first, ForwardIt last, const T& value) {
    ForwardIt result = first;
    for (; first != last; ++first) {
        if (!(*first == value)) {
            if (first != result) {
                *result = zen::move(*first);
            }
            ++result;
        }
    }
    return result;
}

/**
 * @brief 条件移除
 */
template<typename ForwardIt, typename UnaryPred>
ForwardIt remove_if(ForwardIt first, ForwardIt last, UnaryPred pred) {
    ForwardIt result = first;
    for (; first != last; ++first) {
        if (!pred(*first)) {
            if (first != result) {
                *result = zen::move(*first);
            }
            ++result;
        }
    }
    return result;
}

// ============================================================================
// unique（去重）
// ============================================================================

/**
 * @brief 移除相邻的重复元素
 * @return 新的逻辑结束位置迭代器
 */
template<typename ForwardIt>
ForwardIt unique(ForwardIt first, ForwardIt last) {
    if (first == last) return last;
    ForwardIt result = first;
    ++first;
    for (; first != last; ++first) {
        if (!(*result == *first)) {
            ++result;
            if (result != first) {
                *result = zen::move(*first);
            }
        }
    }
    return ++result;
}

/**
 * @brief 条件去重（使用二元谓词）
 */
template<typename ForwardIt, typename BinaryPred>
ForwardIt unique(ForwardIt first, ForwardIt last, BinaryPred pred) {
    if (first == last) return last;
    ForwardIt result = first;
    ++first;
    for (; first != last; ++first) {
        if (!pred(*result, *first)) {
            ++result;
            if (result != first) {
                *result = zen::move(*first);
            }
        }
    }
    return ++result;
}

// ============================================================================
// reverse（反转）
// ============================================================================

/**
 * @brief 反转区间
 */
template<typename BidirIt>
void reverse(BidirIt first, BidirIt last) {
    while (first != last && first != --last) {
        zen::swap(*first, *last);
        ++first;
    }
}

/**
 * @brief 逆序拷贝
 */
template<typename BidirIt, typename OutputIt>
OutputIt reverse_copy(BidirIt first, BidirIt last, OutputIt d_first) {
    while (first != last) {
        *d_first++ = *(--last);
    }
    return d_first;
}

// ============================================================================
// rotate（旋转）
// ============================================================================

/**
 * @brief 旋转区间：使 middle 成为新的第一个元素
 *
 * [first, middle, last) -> [middle, last, first)
 */
template<typename ForwardIt>
ForwardIt rotate(ForwardIt first, ForwardIt middle, ForwardIt last) {
    if (first == middle || middle == last) return first;

    ForwardIt next = middle;
    while (first != next) {
        zen::swap(*first, *next);
        ++first;
        ++next;
        if (next == last) {
            next = middle;
        } else if (first == middle) {
            middle = next;
        }
    }
    return first;
}

// ============================================================================
// shuffle（随机打乱）
// ============================================================================

/**
 * @brief 随机打乱区间（使用简单的线性同余生成器）
 *
 * 复杂度：O(n)
 */
template<typename RandomIt>
void shuffle(RandomIt first, RandomIt last) {
    if (first >= last) return;
    
    size_t n = static_cast<size_t>(last - first);
    if (n <= 1) return;
    
    // 简单的线性同余生成器（不依赖 random）
    unsigned long long seed = 123456789ULL;
    auto rand_next = [&]() -> size_t {
        seed = (seed * 1103515245ULL + 12345ULL) & 0x7fffffffULL;
        return static_cast<size_t>(seed);
    };
    
    for (size_t i = n - 1; i > 0; --i) {
        size_t j = rand_next() % (i + 1);
        if (i != j) {
            zen::swap(*(first + i), *(first + j));
        }
    }
}

// ============================================================================
// partition（分区）
// ============================================================================

/**
 * @brief 分区：使满足条件的元素在前，不满足的在后
 * @return 指向第一个不满足条件的元素的迭代器
 */
template<typename ForwardIt, typename UnaryPred>
ForwardIt partition(ForwardIt first, ForwardIt last, UnaryPred pred) {
    ForwardIt first_false = first;
    for (; first != last; ++first) {
        if (pred(*first)) {
            zen::swap(*first, *first_false);
            ++first_false;
        }
    }
    return first_false;
}

/**
 * @brief 稳定分区（保持相对顺序）
 */
template<typename BidirIt, typename UnaryPred>
BidirIt stable_partition(BidirIt first, BidirIt last, UnaryPred pred) {
    const size_t n = static_cast<size_t>(last - first);
    if (n <= 1) return (first == last || pred(*first)) ? last : first;

    BidirIt middle = first + n / 2;
    BidirIt left = stable_partition(first, middle, pred);
    BidirIt right = stable_partition(middle, last, pred);

    // 旋转中间区域
    return zen::rotate(left, middle, right);
}

// ============================================================================
// is_partitioned（检查是否已分区）
// ============================================================================

/**
 * @brief 检查区间是否已分区
 */
template<typename InputIt, typename UnaryPred>
bool is_partitioned(InputIt first, InputIt last, UnaryPred pred) {
    for (; first != last; ++first) {
        if (!pred(*first)) break;
    }
    for (; first != last; ++first) {
        if (pred(*first)) return false;
    }
    return true;
}

} // namespace zen

#endif // ZEN_ALGORITHMS_TRANSFORM_H
