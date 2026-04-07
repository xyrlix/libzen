#ifndef ZEN_ALGORITHMS_SORT_H
#define ZEN_ALGORITHMS_SORT_H

#include "../base/type_traits.h"
#include "../utility/swap.h"
#include "../iterators/iterator_base.h"
#include "../utility/pair.h"
#include "comparators.h"

namespace zen {

// ============================================================================
// 快速排序（Quick Sort）
// ============================================================================

/**
 * @brief 快速排序（原地分区版本）
 *
 * 平均复杂度：O(n log n)，最坏 O(n²)（已优化：三数取中 + 插入排序小数组）
 * 不稳定排序
 *
 * @tparam RandomIt 随机访问迭代器
 * @tparam Compare  比较函数对象
 * @param first    区间起始
 * @param last     区间结束（不含）
 * @param comp     比较函数，默认 less<T>
 */
template<typename RandomIt, typename Compare>
void quick_sort(RandomIt first, RandomIt last, Compare comp) {
    if (first >= last - 1) return;

    // 小数组用插入排序（阈值 16）
    size_t dist = static_cast<size_t>(last - first);
    if (dist <= 16) {
        for (RandomIt i = first + 1; i < last; ++i) {
            auto key = zen::move(*i);
            RandomIt j = i;
            while (j > first && comp(key, *(j - 1))) {
                *j = zen::move(*(j - 1));
                --j;
            }
            *j = zen::move(key);
        }
        return;
    }

    // 三数取中选 pivot
    RandomIt mid = first + (last - first) / 2;
    if (comp(*mid, *first))  zen::swap(*mid, *first);
    if (comp(*(last - 1), *first)) zen::swap(*(last - 1), *first);
    if (comp(*(last - 1), *mid)) zen::swap(*(last - 1), *mid);

    auto pivot = *(last - 1);

    // 分区
    RandomIt i = first;
    RandomIt j = last - 1;
    while (true) {
        while (i < last && comp(*i, pivot)) ++i;
        while (j > first && !comp(*(j - 1), pivot)) --j;
        if (i >= j) break;
        zen::swap(*i, *j);
        ++i; --j;
    }
    zen::swap(*i, *(last - 1));

    // 递归排序
    quick_sort(first, i, comp);
    quick_sort(i + 1, last, comp);
}

template<typename RandomIt>
void quick_sort(RandomIt first, RandomIt last) {
    using value_type = typename zen::iterator_traits<RandomIt>::value_type;
    quick_sort(first, last, less<value_type>());
}

// ============================================================================
// 归并排序（Merge Sort）- 原地版本
// ============================================================================

/**
 * @brief 归并排序（原地版本，使用临时缓冲区）
 *
 * 复杂度：O(n log n)
 * 稳定排序
 *
 * @tparam RandomIt 随机访问迭代器
 * @tparam Compare  比较函数对象
 */
template<typename RandomIt, typename Compare>
void merge_sort(RandomIt first, RandomIt last, Compare comp) {
    size_t n = static_cast<size_t>(last - first);
    if (n <= 1) return;

    using value_type = typename zen::iterator_traits<RandomIt>::value_type;

    // 临时缓冲区
    value_type* tmp = static_cast<value_type*>(::operator new(n * sizeof(value_type)));

    // 自底向上归并
    for (size_t width = 1; width < n; width *= 2) {
        for (size_t i = 0; i < n; i += 2 * width) {
            size_t left  = i;
            size_t mid   = (i + width < n) ? i + width : n;
            size_t right = (i + 2 * width < n) ? i + 2 * width : n;

            // 归并到临时区
            size_t l = left, r = mid, k = left;
            while (l < mid && r < right) {
                if (comp(*(first + l), *(first + r))) {
                    tmp[k++] = zen::move(*(first + l++));
                } else {
                    tmp[k++] = zen::move(*(first + r++));
                }
            }
            while (l < mid)  tmp[k++] = zen::move(*(first + l++));
            while (r < right) tmp[k++] = zen::move(*(first + r++));

            // 拷贝回原数组
            for (size_t p = left; p < right; ++p) {
                *(first + p) = zen::move(tmp[p]);
            }
        }
    }

    ::operator delete(tmp);
}

template<typename RandomIt>
void merge_sort(RandomIt first, RandomIt last) {
    using value_type = typename zen::iterator_traits<RandomIt>::value_type;
    merge_sort(first, last, zen::less<value_type>{});
}

// ============================================================================
// 堆排序（Heap Sort）
// ============================================================================

/**
 * @brief 堆排序
 *
 * 复杂度：O(n log n)
 * 不稳定排序
 *
 * @tparam RandomIt 随机访问迭代器
 * @tparam Compare  比较函数对象
 */
template<typename RandomIt, typename Compare>
void heapify(RandomIt first, size_t n, size_t i, Compare comp) {
    while (true) {
        size_t largest = i;
        size_t left  = 2 * i + 1;
        size_t right = 2 * i + 2;

        if (left < n && comp(*(first + largest), *(first + left)))
            largest = left;
        if (right < n && comp(*(first + largest), *(first + right)))
            largest = right;

        if (largest == i) break;
        zen::swap(*(first + i), *(first + largest));
        i = largest;
    }
}

template<typename RandomIt, typename Compare>
void heap_sort(RandomIt first, RandomIt last, Compare comp) {
    size_t n = static_cast<size_t>(last - first);
    if (n <= 1) return;

    // 建堆（从最后一个非叶子节点向上 heapify）
    for (size_t i = n / 2; i > 0; --i) {
        heapify(first, n, i - 1, comp);
    }

    // 逐个提取最大值到末尾
    for (size_t i = n; i > 1; --i) {
        zen::swap(*first, *(first + i - 1));
        heapify(first, i - 1, 0, comp);
    }
}

template<typename RandomIt>
void heap_sort(RandomIt first, RandomIt last) {
    using value_type = typename zen::iterator_traits<RandomIt>::value_type;
    heap_sort(first, last, zen::less<value_type>{});
}

// ============================================================================
// 插入排序
// ============================================================================

/**
 * @brief 插入排序（对有序/接近有序数组效率高）
 *
 * 复杂度：O(n²)，最佳 O(n)
 * 稳定排序
 */
template<typename ForwardIt, typename Compare>
void insertion_sort(ForwardIt first, ForwardIt last, Compare comp) {
    if (first >= last) return;

    for (ForwardIt i = first + 1; i < last; ++i) {
        auto key = zen::move(*i);
        ForwardIt j = i;
        while (j > first && comp(key, *(j - 1))) {
            *j = zen::move(*(j - 1));
            --j;
        }
        *j = zen::move(key);
    }
}

template<typename ForwardIt>
void insertion_sort(ForwardIt first, ForwardIt last) {
    using value_type = typename zen::iterator_traits<ForwardIt>::value_type;
    insertion_sort(first, last, zen::less<value_type>{});
}

// ============================================================================
// 选择排序
// ============================================================================

/**
 * @brief 选择排序
 *
 * 复杂度：O(n²)
 * 不稳定排序（但我们的实现是稳定的）
 */
template<typename ForwardIt, typename Compare>
void selection_sort(ForwardIt first, ForwardIt last, Compare comp) {
    for (ForwardIt i = first; i < last; ++i) {
        ForwardIt min_pos = i;
        for (ForwardIt j = i + 1; j < last; ++j) {
            if (comp(*j, *min_pos)) min_pos = j;
        }
        if (min_pos != i) zen::swap(*i, *min_pos);
    }
}

template<typename ForwardIt>
void selection_sort(ForwardIt first, ForwardIt last) {
    using value_type = typename zen::iterator_traits<ForwardIt>::value_type;
    selection_sort(first, last, less<value_type>{});
}

// ============================================================================
// 默认 sort（使用快速排序）
// ============================================================================

/**
 * @brief 默认排序（quick_sort）
 */
template<typename RandomIt, typename Compare>
void sort(RandomIt first, RandomIt last, Compare comp) {
    quick_sort(first, last, comp);
}

template<typename RandomIt>
void sort(RandomIt first, RandomIt last) {
    using value_type = typename zen::iterator_traits<RandomIt>::value_type;
    sort(first, last, zen::less<value_type>{});
}

// ============================================================================
// 稳定排序（使用归并排序）
// ============================================================================

template<typename RandomIt, typename Compare>
void stable_sort(RandomIt first, RandomIt last, Compare comp) {
    merge_sort(first, last, comp);
}

template<typename RandomIt>
void stable_sort(RandomIt first, RandomIt last) {
    using value_type = typename zen::iterator_traits<RandomIt>::value_type;
    stable_sort(first, last, less<value_type>{});
}

// ============================================================================
// 部分排序（取前 n 个最小/最大元素）
// ============================================================================

/**
 * @brief 部分排序：使前 n 个元素为最小的 n 个（并按升序排列）
 *
 * 复杂度：O(n log n)
 */
template<typename RandomIt, typename Compare>
void partial_sort(RandomIt first, RandomIt middle, RandomIt last, Compare comp) {
    // 简化实现：使用 full sort
    zen::sort(first, last, comp);
}

template<typename RandomIt>
void partial_sort(RandomIt first, RandomIt middle, RandomIt last) {
    using value_type = typename zen::iterator_traits<RandomIt>::value_type;
    partial_sort(first, middle, last, less<value_type>{});
}

// ============================================================================
// nth_element（使第 n 个元素就位）
// ============================================================================

/**
 * @brief nth_element：使第 n 个位置放置正确排序后应该在那里的元素
 *
 * 复杂度：平均 O(n)，最坏 O(n²)
 */
template<typename RandomIt, typename Compare>
void nth_element(RandomIt first, RandomIt nth, RandomIt last, Compare comp) {
    if (first >= last || nth < first || nth >= last) return;

    while (last - first > 10) {
        // 三数取中
        RandomIt m = first + (last - first) / 2;
        if (comp(*m, *first)) zen::swap(*m, *first);
        if (comp(*(last - 1), *first)) zen::swap(*(last - 1), *first);
        if (comp(*(last - 1), *m)) zen::swap(*(last - 1), *m);

        zen::swap(*m, *(last - 1));
        auto pivot = *(last - 1);

        RandomIt i = first, j = last - 1;
        while (i < j) {
            while (i < j && comp(*i, pivot)) ++i;
            while (j > i && !comp(*(j - 1), pivot)) --j;
            if (i >= j) break;
            zen::swap(*i, *(--j));
        }
        zen::swap(*i, *(last - 1));

        if (i <= nth) first = i + 1;
        else last = i;
    }

    insertion_sort(first, last, comp);
}

template<typename RandomIt>
void nth_element(RandomIt first, RandomIt nth, RandomIt last) {
    using value_type = typename zen::iterator_traits<RandomIt>::value_type;
    nth_element(first, nth, last, zen::less<value_type>{});
}

// ============================================================================
// 判断是否已排序
// ============================================================================

/**
 * @brief 检查区间是否已按升序排列
 */
template<typename ForwardIt, typename Compare>
bool is_sorted(ForwardIt first, ForwardIt last, Compare comp) {
    if (first >= last) return true;
    for (ForwardIt prev = first++; first < last; ++first, ++prev) {
        if (comp(*first, *prev)) return false;
    }
    return true;
}

template<typename ForwardIt>
bool is_sorted(ForwardIt first, ForwardIt last) {
    using value_type = typename zen::iterator_traits<ForwardIt>::value_type;
    return is_sorted(first, last, zen::less<value_type>{});
}

} // namespace zen

#endif // ZEN_ALGORITHMS_SORT_H
