#ifndef ZEN_ALGORITHMS_FIND_H
#define ZEN_ALGORITHMS_FIND_H

#include "../base/type_traits.h"
#include "../utility/swap.h"
#include "../utility/pair.h"
#include "../iterators/iterator_base.h"
#include "comparators.h"

namespace zen {

// ============================================================================
// 线性查找
// ============================================================================

/**
 * @brief 查找第一个匹配的元素（线性扫描）
 *
 * 复杂度：O(n)
 *
 * @tparam InputIt   输入迭代器
 * @tparam T         查找的值类型
 * @tparam BinaryPred 一元谓词（检查元素是否匹配）
 * @return 找到返回迭代器，否则返回 last
 */
template<typename InputIt, typename T>
InputIt find(InputIt first, InputIt last, const T& value) {
    for (; first != last; ++first) {
        if (*first == value) return first;
    }
    return last;
}

/**
 * @brief 使用谓词查找第一个满足条件的元素
 */
template<typename InputIt, typename UnaryPred>
InputIt find_if(InputIt first, InputIt last, UnaryPred pred) {
    for (; first != last; ++first) {
        if (pred(*first)) return first;
    }
    return last;
}

/**
 * @brief 查找第一个不满足条件的元素
 */
template<typename InputIt, typename UnaryPred>
InputIt find_if_not(InputIt first, InputIt last, UnaryPred pred) {
    for (; first != last; ++first) {
        if (!pred(*first)) return first;
    }
    return last;
}

/**
 * @brief 查找第一个满足条件的元素（简化版）
 */
template<typename InputIt, typename UnaryPred>
InputIt find_first_of(InputIt first1, InputIt last1,
                      InputIt first2, InputIt last2) {
    for (; first1 != last1; ++first1) {
        for (auto it = first2; it != last2; ++it) {
            if (*first1 == *it) return first1;
        }
    }
    return last1;
}

// ============================================================================
// 二分查找（要求区间已排序）
// ============================================================================

/**
 * @brief 二分查找：查找第一个不小于 value 的元素位置
 *
 * 复杂度：O(log n)
 * 区间必须已升序排列
 *
 * @return 迭代器指向第一个满足 !comp(*it, value) 的元素，或 last
 */
template<typename RandomIt, typename T, typename Compare>
RandomIt lower_bound(RandomIt first, RandomIt last, const T& value, Compare comp) {
    size_t n = static_cast<size_t>(last - first);
    size_t left = 0;
    while (n > 0) {
        size_t mid = n / 2;
        size_t cur = left + mid;
        if (comp(*(first + cur), value)) {
            left = cur + 1;
            n = n - mid - 1;
        } else {
            n = mid;
        }
    }
    return first + left;
}

/**
 * @brief 二分查找：查找第一个大于 value 的元素位置
 */
template<typename RandomIt, typename T, typename Compare>
RandomIt upper_bound(RandomIt first, RandomIt last, const T& value, Compare comp) {
    size_t n = static_cast<size_t>(last - first);
    size_t left = 0;
    while (n > 0) {
        size_t mid = n / 2;
        size_t cur = left + mid;
        if (!comp(value, *(first + cur))) {
            left = cur + 1;
            n = n - mid - 1;
        } else {
            n = mid;
        }
    }
    return first + left;
}

/**
 * @brief 二分查找：查找等于 value 的元素范围
 *
 * @return pair<first, last>，first 指向第一个 >= value，last 指向第一个 > value
 */
template<typename RandomIt, typename T, typename Compare>
pair<RandomIt, RandomIt> equal_range(RandomIt first, RandomIt last,
                                      const T& value, Compare comp) {
    return {
        zen::lower_bound(first, last, value, comp),
        zen::upper_bound(first, last, value, comp)
    };
}

template<typename RandomIt, typename T>
RandomIt lower_bound(RandomIt first, RandomIt last, const T& value) {
    using value_type = typename zen::iterator_traits<RandomIt>::value_type;
    return zen::lower_bound(first, last, value, zen::less<value_type>{});
}

template<typename RandomIt, typename T>
RandomIt upper_bound(RandomIt first, RandomIt last, const T& value) {
    using value_type = typename zen::iterator_traits<RandomIt>::value_type;
    return zen::upper_bound(first, last, value, zen::less<value_type>{});
}

template<typename RandomIt, typename T>
pair<RandomIt, RandomIt> equal_range(RandomIt first, RandomIt last, const T& value) {
    using value_type = typename zen::iterator_traits<RandomIt>::value_type;
    return zen::equal_range(first, last, value, zen::less<value_type>{});
}

/**
 * @brief 二分查找：查找元素是否存在
 */
template<typename RandomIt, typename T, typename Compare>
bool binary_search(RandomIt first, RandomIt last, const T& value, Compare comp) {
    auto it = zen::lower_bound(first, last, value, comp);
    return it != last && !comp(value, *it);
}

template<typename RandomIt, typename T>
bool binary_search(RandomIt first, RandomIt last, const T& value) {
    using value_type = typename zen::iterator_traits<RandomIt>::value_type;
    return binary_search(first, last, value, less<value_type>{});
}

// ============================================================================
// 搜索（子序列查找）
// ============================================================================

/**
 * @brief 简单搜索：查找子序列第一次出现的位置
 *
 * 复杂度：O(n*m)，暴力搜索
 */
template<typename ForwardIt1, typename ForwardIt2>
ForwardIt1 search(ForwardIt1 first1, ForwardIt1 last1,
                   ForwardIt2 first2, ForwardIt2 last2) {
    if (first2 == last2) return first1;

    for (auto it1 = first1; it1 != last1; ++it1) {
        auto it1_cur = it1;
        auto it2_cur = first2;
        while (it1_cur != last1 && it2_cur != last2) {
            if (*it1_cur != *it2_cur) break;
            ++it1_cur; ++it2_cur;
        }
        if (it2_cur == last2) return it1;
    }
    return last1;
}

/**
 * @brief 查找第一个满足条件的元素（使用二元谓词）
 */
template<typename ForwardIt, typename BinaryPred>
ForwardIt find_first_of(ForwardIt first1, ForwardIt last1,
                         ForwardIt first2, ForwardIt last2,
                         BinaryPred pred) {
    for (; first1 != last1; ++first1) {
        for (auto it = first2; it != last2; ++it) {
            if (pred(*first1, *it)) return first1;
        }
    }
    return last1;
}

// ============================================================================
// 相邻查找
// ============================================================================

/**
 * @brief 查找第一对相邻的相等于值
 */
template<typename ForwardIt>
ForwardIt adjacent_find(ForwardIt first, ForwardIt last) {
    if (first == last) return last;
    auto prev = first;
    for (++first; first != last; ++first, ++prev) {
        if (*prev == *first) return prev;
    }
    return last;
}

/**
 * @brief 查找第一对满足二元谓词的相邻元素
 */
template<typename ForwardIt, typename BinaryPred>
ForwardIt adjacent_find(ForwardIt first, ForwardIt last, BinaryPred pred) {
    if (first == last) return last;
    auto prev = first;
    for (++first; first != last; ++first, ++prev) {
        if (pred(*prev, *first)) return prev;
    }
    return last;
}

// ============================================================================
// 计数
// ============================================================================

/**
 * @brief 统计等于给定值的元素个数
 */
template<typename InputIt, typename T>
size_t count(InputIt first, InputIt last, const T& value) {
    size_t n = 0;
    for (; first != last; ++first) {
        if (*first == value) ++n;
    }
    return n;
}

/**
 * @brief 统计满足条件的元素个数
 */
template<typename InputIt, typename UnaryPred>
size_t count_if(InputIt first, InputIt last, UnaryPred pred) {
    size_t n = 0;
    for (; first != last; ++first) {
        if (pred(*first)) ++n;
    }
    return n;
}

// ============================================================================
// 比较
// ============================================================================

/**
 * @brief 检查两个区间是否相等
 */
template<typename InputIt1, typename InputIt2>
bool equal(InputIt1 first1, InputIt1 last1, InputIt2 first2) {
    for (; first1 != last1; ++first1, ++first2) {
        if (*first1 != *first2) return false;
    }
    return true;
}

/**
 * @brief 检查两个区间是否相等（使用谓词）
 */
template<typename InputIt1, typename InputIt2, typename BinaryPred>
bool equal(InputIt1 first1, InputIt1 last1, InputIt2 first2, BinaryPred pred) {
    for (; first1 != last1; ++first1, ++first2) {
        if (!pred(*first1, *first2)) return false;
    }
    return true;
}

/**
 * @brief 字典序比较（<）
 */
template<typename InputIt1, typename InputIt2, typename Compare>
bool lexicographical_compare(InputIt1 first1, InputIt1 last1,
                             InputIt2 first2, InputIt2 last2,
                             Compare comp) {
    for (; first1 != last1 && first2 != last2; ++first1, ++first2) {
        if (comp(*first1, *first2)) return true;
        if (comp(*first2, *first1)) return false;
    }
    return (first1 == last1) && (first2 != last2);
}

template<typename InputIt1, typename InputIt2>
bool lexicographical_compare(InputIt1 first1, InputIt1 last1,
                             InputIt2 first2, InputIt2 last2) {
    using value_type = typename zen::iterator_traits<InputIt1>::value_type;
    return lexicographical_compare(first1, last1, first2, last2, less<value_type>{});
}

} // namespace zen

#endif // ZEN_ALGORITHMS_FIND_H
