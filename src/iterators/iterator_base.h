#ifndef ZEN_ITERATORS_ITERATOR_BASE_H
#define ZEN_ITERATORS_ITERATOR_BASE_H

#include "../base/type_traits.h"

namespace zen {

// ============================================================================
// 迭代器类别标签（Iterator Category Tags）
// ============================================================================
//
// 标签继承关系（越靠下能力越强）：
//
//   input_iterator_tag    output_iterator_tag
//          ↓
//   forward_iterator_tag
//          ↓
//   bidirectional_iterator_tag
//          ↓
//   random_access_iterator_tag
//
// 算法可以通过 tag dispatching 为不同类别的迭代器选择最优实现。
// ============================================================================

/**
 * @brief 输入迭代器：只读，只能向前单步移动
 *
 * 支持：++, *, ->, ==, !=
 * 典型：std::istream_iterator
 */
struct input_iterator_tag {};

/**
 * @brief 输出迭代器：只写，只能向前单步移动
 *
 * 支持：++, * (赋值)
 * 典型：std::ostream_iterator
 */
struct output_iterator_tag {};

/**
 * @brief 前向迭代器：可读写，向前单步移动，支持多遍扫描
 *
 * 继承自 input_iterator_tag（满足所有输入迭代器要求）
 * 典型：std::forward_list::iterator
 */
struct forward_iterator_tag : input_iterator_tag {};

/**
 * @brief 双向迭代器：可读写，向前或向后单步移动
 *
 * 新增：--（前缀和后缀）
 * 典型：std::list::iterator, std::map::iterator
 */
struct bidirectional_iterator_tag : forward_iterator_tag {};

/**
 * @brief 随机访问迭代器：可读写，支持算术运算和下标访问
 *
 * 新增：+, -, +=, -=, [], <, >, <=, >=
 * 典型：指针、std::vector::iterator
 */
struct random_access_iterator_tag : bidirectional_iterator_tag {};

// ============================================================================
// iterator_traits - 迭代器类型萃取
// ============================================================================

/**
 * @brief 通用迭代器 traits，从迭代器类型中提取关联类型
 * @tparam Iterator 迭代器类型
 *
 * 每个自定义迭代器类型应定义以下成员类型：
 * - value_type
 * - difference_type
 * - pointer
 * - reference
 * - iterator_category
 *
 * 算法通过 iterator_traits 在不改变实现的情况下支持多种迭代器。
 */
template<typename Iterator>
struct iterator_traits {
    using difference_type   = typename Iterator::difference_type;
    using value_type        = typename Iterator::value_type;
    using pointer           = typename Iterator::pointer;
    using reference         = typename Iterator::reference;
    using iterator_category = typename Iterator::iterator_category;
};

/**
 * @brief 原始指针的 iterator_traits 特化
 *
 * T* 是最典型的随机访问迭代器
 */
template<typename T>
struct iterator_traits<T*> {
    using difference_type   = decltype((T*)0 - (T*)0); // ptrdiff_t
    using value_type        = T;
    using pointer           = T*;
    using reference         = T&;
    using iterator_category = random_access_iterator_tag;
};

/**
 * @brief const 指针的 iterator_traits 特化
 */
template<typename T>
struct iterator_traits<const T*> {
    using difference_type   = decltype((T*)0 - (T*)0);
    using value_type        = T;
    using pointer           = const T*;
    using reference         = const T&;
    using iterator_category = random_access_iterator_tag;
};

// ============================================================================
// iterator - 便捷基类，供自定义迭代器继承
// ============================================================================

/**
 * @brief 迭代器基类（辅助类，提供统一的类型定义）
 * @tparam Category   迭代器类别标签
 * @tparam T          值类型
 * @tparam Distance   差值类型（默认 ptrdiff_t）
 * @tparam Pointer    指针类型
 * @tparam Reference  引用类型
 *
 * 自定义迭代器可继承此类来获得所有必要的 typedef：
 * @code
 * class MyIterator : public zen::iterator<zen::forward_iterator_tag, int> {
 *     // 只需实现 ++, *, ==, != 等运算符
 * };
 * @endcode
 */
template<
    typename Category,
    typename T,
    typename Distance  = decltype((T*)0 - (T*)0),
    typename Pointer   = T*,
    typename Reference = T&
>
struct iterator {
    using iterator_category = Category;
    using value_type        = T;
    using difference_type   = Distance;
    using pointer           = Pointer;
    using reference         = Reference;
};

// ============================================================================
// 迭代器工具函数
// ============================================================================

/**
 * @brief 将迭代器向前推进 n 步（泛型版本）
 * @tparam InputIt 迭代器类型（至少为 input_iterator）
 * @param it 迭代器引用
 * @param n  步数（对于 input/forward_iterator 只能为正数）
 *
 * 复杂度：
 * - 随机访问迭代器：O(1)（直接 += n）
 * - 其他：O(n)（逐步 ++）
 */
// 随机访问版本：O(1)
template<typename RandomIt>
void advance_impl(RandomIt& it,
                  typename iterator_traits<RandomIt>::difference_type n,
                  random_access_iterator_tag) {
    it += n;
}

// 双向版本：O(|n|)，支持负数
template<typename BidirIt>
void advance_impl(BidirIt& it,
                  typename iterator_traits<BidirIt>::difference_type n,
                  bidirectional_iterator_tag) {
    if (n > 0) {
        while (n--) ++it;
    } else {
        while (n++) --it;
    }
}

// 输入/前向版本：O(n)，只能正数
template<typename InputIt>
void advance_impl(InputIt& it,
                  typename iterator_traits<InputIt>::difference_type n,
                  input_iterator_tag) {
    while (n-- > 0) ++it;
}

/**
 * @brief 将迭代器向前推进 n 步
 */
template<typename InputIt>
void advance(InputIt& it, typename iterator_traits<InputIt>::difference_type n) {
    advance_impl(it, n,
        typename iterator_traits<InputIt>::iterator_category{});
}

/**
 * @brief 返回两个迭代器之间的距离
 * @tparam InputIt 迭代器类型
 * @param first 起始迭代器
 * @param last  终止迭代器
 * @return 从 first 到 last 需要走的步数
 *
 * 复杂度：
 * - 随机访问迭代器：O(1)
 * - 其他：O(n)
 */
// 随机访问版本
template<typename RandomIt>
typename iterator_traits<RandomIt>::difference_type
distance_impl(RandomIt first, RandomIt last, random_access_iterator_tag) {
    return last - first;
}

// 输入迭代器版本
template<typename InputIt>
typename iterator_traits<InputIt>::difference_type
distance_impl(InputIt first, InputIt last, input_iterator_tag) {
    typename iterator_traits<InputIt>::difference_type n = 0;
    while (first != last) {
        ++first;
        ++n;
    }
    return n;
}

/**
 * @brief 计算两个迭代器之间的距离
 */
template<typename InputIt>
typename iterator_traits<InputIt>::difference_type
distance(InputIt first, InputIt last) {
    return distance_impl(first, last,
        typename iterator_traits<InputIt>::iterator_category{});
}

/**
 * @brief 返回 it 后移 n 步的迭代器（不修改 it 本身）
 */
template<typename InputIt>
InputIt next(InputIt it,
             typename iterator_traits<InputIt>::difference_type n = 1) {
    advance(it, n);
    return it;
}

/**
 * @brief 返回 it 前移 n 步的迭代器（需要双向迭代器）
 */
template<typename BidirIt>
BidirIt prev(BidirIt it,
             typename iterator_traits<BidirIt>::difference_type n = 1) {
    advance(it, -n);
    return it;
}

// ============================================================================
// reverse_iterator - 反向迭代器适配器
// ============================================================================

/**
 * @brief 将双向迭代器逆序包装
 * @tparam Iterator 被适配的迭代器类型（至少为双向迭代器）
 *
 * reverse_iterator(it) 实际指向 it 的前一个元素。
 * 也就是说：*reverse_iterator(it) == *(it - 1)
 *
 * 这是 rbegin/rend 的实现基础：
 *   container.rbegin() == reverse_iterator(container.end())
 *   container.rend()   == reverse_iterator(container.begin())
 *
 * 示例：
 * @code
 * int arr[] = {1, 2, 3, 4, 5};
 * auto rb = zen::reverse_iterator<int*>(arr + 5);  // 指向 arr[4]
 * auto re = zen::reverse_iterator<int*>(arr);       // 结束哨兵
 * for (auto it = rb; it != re; ++it) {
 *     // 输出: 5, 4, 3, 2, 1
 * }
 * @endcode
 */
template<typename Iterator>
class reverse_iterator {
public:
    using iterator_type     = Iterator;
    using iterator_category = typename iterator_traits<Iterator>::iterator_category;
    using value_type        = typename iterator_traits<Iterator>::value_type;
    using difference_type   = typename iterator_traits<Iterator>::difference_type;
    using pointer           = typename iterator_traits<Iterator>::pointer;
    using reference         = typename iterator_traits<Iterator>::reference;

protected:
    Iterator current_; // 包装的底层迭代器

public:
    // ========================================================================
    // 构造函数
    // ========================================================================

    constexpr reverse_iterator() : current_() {}

    explicit constexpr reverse_iterator(Iterator x) : current_(x) {}

    template<typename U>
    constexpr reverse_iterator(const reverse_iterator<U>& other)
        : current_(other.base()) {
    }

    // ========================================================================
    // 访问底层迭代器
    // ========================================================================

    /**
     * @brief 返回底层迭代器（指向当前元素的下一位）
     */
    constexpr Iterator base() const {
        return current_;
    }

    // ========================================================================
    // 解引用（访问底层迭代器前一个元素）
    // ========================================================================

    /**
     * @brief 解引用：返回底层迭代器前一个位置的元素
     *
     * 实现：先将 current_ 复制，然后 -- 该副本，再解引用
     */
    constexpr reference operator*() const {
        Iterator tmp = current_;
        return *--tmp;
    }

    constexpr pointer operator->() const {
        Iterator tmp = current_;
        --tmp;
        return &(*tmp);
    }

    // ========================================================================
    // 移动运算符（逻辑是反的）
    // ========================================================================

    // 前缀 ++：逻辑上向前，底层向后
    constexpr reverse_iterator& operator++() {
        --current_;
        return *this;
    }

    // 后缀 ++
    constexpr reverse_iterator operator++(int) {
        reverse_iterator tmp = *this;
        --current_;
        return tmp;
    }

    // 前缀 --
    constexpr reverse_iterator& operator--() {
        ++current_;
        return *this;
    }

    // 后缀 --
    constexpr reverse_iterator operator--(int) {
        reverse_iterator tmp = *this;
        ++current_;
        return tmp;
    }

    // += / -=（随机访问迭代器）
    constexpr reverse_iterator& operator+=(difference_type n) {
        current_ -= n;
        return *this;
    }

    constexpr reverse_iterator& operator-=(difference_type n) {
        current_ += n;
        return *this;
    }

    // + / - 运算
    constexpr reverse_iterator operator+(difference_type n) const {
        return reverse_iterator(current_ - n);
    }

    constexpr reverse_iterator operator-(difference_type n) const {
        return reverse_iterator(current_ + n);
    }

    // 下标访问
    constexpr reference operator[](difference_type n) const {
        return *(*this + n);
    }
};

// ============================================================================
// reverse_iterator 比较运算符
// ============================================================================

template<typename It1, typename It2>
constexpr bool operator==(const reverse_iterator<It1>& a,
                           const reverse_iterator<It2>& b) {
    return a.base() == b.base();
}

template<typename It1, typename It2>
constexpr bool operator!=(const reverse_iterator<It1>& a,
                           const reverse_iterator<It2>& b) {
    return a.base() != b.base();
}

template<typename It1, typename It2>
constexpr bool operator<(const reverse_iterator<It1>& a,
                          const reverse_iterator<It2>& b) {
    return b.base() < a.base(); // 注意：方向相反
}

template<typename It1, typename It2>
constexpr bool operator>(const reverse_iterator<It1>& a,
                          const reverse_iterator<It2>& b) {
    return b < a;
}

template<typename It1, typename It2>
constexpr bool operator<=(const reverse_iterator<It1>& a,
                           const reverse_iterator<It2>& b) {
    return !(b < a);
}

template<typename It1, typename It2>
constexpr bool operator>=(const reverse_iterator<It1>& a,
                           const reverse_iterator<It2>& b) {
    return !(a < b);
}

// 迭代器之间的距离
template<typename It1, typename It2>
constexpr auto operator-(const reverse_iterator<It1>& a,
                          const reverse_iterator<It2>& b)
    -> decltype(b.base() - a.base()) {
    return b.base() - a.base();
}

// n + it
template<typename It>
constexpr reverse_iterator<It>
operator+(typename reverse_iterator<It>::difference_type n,
          const reverse_iterator<It>& it) {
    return it + n;
}

/**
 * @brief 创建 reverse_iterator 的辅助函数
 */
template<typename Iterator>
constexpr reverse_iterator<Iterator> make_reverse_iterator(Iterator i) {
    return reverse_iterator<Iterator>(i);
}

} // namespace zen

#endif // ZEN_ITERATORS_ITERATOR_BASE_H
