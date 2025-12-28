#pragma once

#include <iterator>
#include <functional>
#include <vector>
#include <type_traits>

namespace zen {

/**
 * @brief 范围适配器工具类
 * 对标C++20 <ranges>，简化迭代器遍历、筛选、转换
 */
namespace ranges {

/**
 * @brief 范围包装类
 * 用于包装可迭代对象，提供链式调用的范围操作
 * @tparam Iterator 迭代器类型
 */
template <typename Iterator>
class range_view {
public:
    using value_type = typename std::iterator_traits<Iterator>::value_type;
    using reference = typename std::iterator_traits<Iterator>::reference;
    using pointer = typename std::iterator_traits<Iterator>::pointer;
    using difference_type = typename std::iterator_traits<Iterator>::difference_type;
    using iterator_category = typename std::iterator_traits<Iterator>::iterator_category;

    /**
     * @brief 构造函数
     * @param begin 起始迭代器
     * @param end 结束迭代器
     */
    range_view(Iterator begin, Iterator end) : begin_(begin), end_(end) {}

    /**
     * @brief 获取起始迭代器
     * @return 起始迭代器
     */
    Iterator begin() const { return begin_; }

    /**
     * @brief 获取结束迭代器
     * @return 结束迭代器
     */
    Iterator end() const { return end_; }

    /**
     * @brief 筛选操作
     * @tparam Predicate 谓词类型
     * @param pred 筛选条件
     * @return 筛选后的范围视图
     */
    template <typename Predicate>
    range_view filter(Predicate pred) const {
        // 这里简化实现，实际应该返回一个延迟计算的视图
        std::vector<value_type> result;
        for (auto it = begin_; it != end_; ++it) {
            if (pred(*it)) {
                result.push_back(*it);
            }
        }
        return range_view(result.begin(), result.end());
    }

    /**
     * @brief 转换操作
     * @tparam Transformer 转换函数类型
     * @param transform 转换函数
     * @return 转换后的范围视图
     */
    template <typename Transformer>
    auto transform(Transformer transform) const {
        using result_type = decltype(transform(*begin_));
        std::vector<result_type> result;
        for (auto it = begin_; it != end_; ++it) {
            result.push_back(transform(*it));
        }
        return range_view<typename std::vector<result_type>::iterator>(result.begin(), result.end());
    }

    /**
     * @brief 遍历操作
     * @tparam Func 函数类型
     * @param func 遍历函数
     */
    template <typename Func>
    void for_each(Func func) const {
        for (auto it = begin_; it != end_; ++it) {
            func(*it);
        }
    }

    /**
     * @brief 统计满足条件的元素个数
     * @tparam Predicate 谓词类型
     * @param pred 条件谓词
     * @return 满足条件的元素个数
     */
    template <typename Predicate>
    size_t count_if(Predicate pred) const {
        size_t count = 0;
        for (auto it = begin_; it != end_; ++it) {
            if (pred(*it)) {
                ++count;
            }
        }
        return count;
    }

    /**
     * @brief 统计元素总个数
     * @return 元素总个数
     */
    size_t size() const {
        return std::distance(begin_, end_);
    }

    /**
     * @brief 判断范围是否为空
     * @return 是否为空
     */
    bool empty() const {
        return begin_ == end_;
    }

private:
    Iterator begin_;
    Iterator end_;
};

/**
 * @brief 创建范围视图
 * @tparam Range 可迭代类型
 * @param range 可迭代对象
 * @return 范围视图
 */
template <typename Range>
range_view<typename Range::iterator> view(Range& range) {
    return range_view<typename Range::iterator>(range.begin(), range.end());
}

/**
 * @brief 创建常量范围视图
 * @tparam Range 可迭代类型
 * @param range 可迭代对象
 * @return 常量范围视图
 */
template <typename Range>
range_view<typename Range::const_iterator> view(const Range& range) {
    return range_view<typename Range::const_iterator>(range.begin(), range.end());
}

/**
 * @brief 创建基于数组的范围视图
 * @tparam T 元素类型
 * @tparam N 数组大小
 * @param arr 数组
 * @return 范围视图
 */
template <typename T, size_t N>
range_view<T*> view(T (&arr)[N]) {
    return range_view<T*>(arr, arr + N);
}

/**
 * @brief 筛选函数
 * @tparam Range 可迭代类型
 * @tparam Predicate 谓词类型
 * @param range 可迭代对象
 * @param pred 筛选条件
 * @return 筛选后的范围视图
 */
template <typename Range, typename Predicate>
auto filter(Range& range, Predicate pred) -> decltype(view(range).filter(pred)) {
    return view(range).filter(pred);
}

/**
 * @brief 筛选函数（常量版本）
 * @tparam Range 可迭代类型
 * @tparam Predicate 谓词类型
 * @param range 可迭代对象
 * @param pred 筛选条件
 * @return 筛选后的范围视图
 */
template <typename Range, typename Predicate>
auto filter(const Range& range, Predicate pred) -> decltype(view(range).filter(pred)) {
    return view(range).filter(pred);
}

/**
 * @brief 转换函数
 * @tparam Range 可迭代类型
 * @tparam Transformer 转换函数类型
 * @param range 可迭代对象
 * @param transform 转换函数
 * @return 转换后的范围视图
 */
template <typename Range, typename Transformer>
auto transform(Range& range, Transformer transform) -> decltype(view(range).transform(transform)) {
    return view(range).transform(transform);
}

/**
 * @brief 转换函数（常量版本）
 * @tparam Range 可迭代类型
 * @tparam Transformer 转换函数类型
 * @param range 可迭代对象
 * @param transform 转换函数
 * @return 转换后的范围视图
 */
template <typename Range, typename Transformer>
auto transform(const Range& range, Transformer transform) -> decltype(view(range).transform(transform)) {
    return view(range).transform(transform);
}

/**
 * @brief 遍历函数
 * @tparam Range 可迭代类型
 * @tparam Func 函数类型
 * @param range 可迭代对象
 * @param func 遍历函数
 */
template <typename Range, typename Func>
void for_each(Range& range, Func func) {
    view(range).for_each(func);
}

/**
 * @brief 遍历函数（常量版本）
 * @tparam Range 可迭代类型
 * @tparam Func 函数类型
 * @param range 可迭代对象
 * @param func 遍历函数
 */
template <typename Range, typename Func>
void for_each(const Range& range, Func func) {
    view(range).for_each(func);
}

} // namespace ranges

} // namespace zen
