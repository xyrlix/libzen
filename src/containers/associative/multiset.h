/**
 * @file multiset.h
 * @brief 多值有序集合容器
 * @details 允许重复元素的有序集合
 */

#ifndef ZEN_CONTAINERS_MULTISET_H
#define ZEN_CONTAINERS_MULTISET_H

#include "set.h"
#include <vector>

namespace zen {

/**
 * @brief 多值有序集合容器
 * @tparam T 元素类型
 * @tparam Compare 比较函数
 * @details 基于红黑树实现，允许重复元素
 */
template <typename T, typename Compare = std::less<T>>
class MultiSet {
public:
    using value_type = T;
    using size_type = size_t;
    using iterator = typename std::vector<T>::iterator;
    using const_iterator = typename std::vector<T>::const_iterator;

    MultiSet() = default;

    MultiSet(std::initializer_list<T> init) {
        for (const auto& item : init) {
            insert(item);
        }
    }

    // ============================================================================
    // 迭代器
    // ============================================================================

    iterator begin() { return data_.begin(); }
    iterator end() { return data_.end(); }

    const_iterator begin() const { return data_.begin(); }
    const_iterator end() const { return data_.end(); }

    const_iterator cbegin() const { return data_.cbegin(); }
    const_iterator cend() const { return data_.cend(); }

    // ============================================================================
    // 容量
    // ============================================================================

    bool empty() const { return data_.empty(); }
    size_type size() const { return data_.size(); }

    // ============================================================================
    // 修改操作
    // ============================================================================

    /**
     * @brief 插入元素（允许重复）
     */
    iterator insert(const T& value) {
        auto it = std::lower_bound(data_.begin(), data_.end(), value, compare_);
        return data_.insert(it, value);
    }

    iterator insert(T&& value) {
        auto it = std::lower_bound(data_.begin(), data_.end(), value, compare_);
        return data_.insert(it, std::move(value));
    }

    /**
     * @brief 删除一个元素
     */
    size_type erase(const T& value) {
        auto it = std::lower_bound(data_.begin(), data_.end(), value, compare_);
        if (it != data_.end() && !compare_(value, *it)) {
            data_.erase(it);
            return 1;
        }
        return 0;
    }

    iterator erase(const_iterator pos) {
        return data_.erase(pos);
    }

    /**
     * @brief 删除所有等于 value 的元素
     */
    size_type erase_all(const T& value) {
        auto range = equal_range(value);
        size_type count = std::distance(range.first, range.second);
        data_.erase(range.first, range.second);
        return count;
    }

    /**
     * @brief 清空集合
     */
    void clear() { data_.clear(); }

    /**
     * @brief 交换集合
     */
    void swap(MultiSet& other) noexcept {
        data_.swap(other.data_);
    }

    /**
     * @brief 查找元素
     */
    iterator find(const T& value) {
        auto range = equal_range(value);
        if (range.first != range.second) {
            return range.first;
        }
        return data_.end();
    }

    const_iterator find(const T& value) const {
        auto range = equal_range(value);
        if (range.first != range.second) {
            return range.first;
        }
        return data_.cend();
    }

    /**
     * @brief 计算元素数量
     */
    size_type count(const T& value) const {
        auto range = equal_range(value);
        return std::distance(range.first, range.second);
    }

    /**
     * @brief 获取元素范围
     */
    std::pair<iterator, iterator> equal_range(const T& value) {
        return std::equal_range(data_.begin(), data_.end(), value, compare_);
    }

    std::pair<const_iterator, const_iterator> equal_range(const T& value) const {
        return std::equal_range(data_.cbegin(), data_.cend(), value, compare_);
    }

    /**
     * @brief 获取下界迭代器
     */
    iterator lower_bound(const T& value) {
        return std::lower_bound(data_.begin(), data_.end(), value, compare_);
    }

    const_iterator lower_bound(const T& value) const {
        return std::lower_bound(data_.cbegin(), data_.cend(), value, compare_);
    }

    /**
     * @brief 获取上界迭代器
     */
    iterator upper_bound(const T& value) {
        return std::upper_bound(data_.begin(), data_.end(), value, compare_);
    }

    const_iterator upper_bound(const T& value) const {
        return std::upper_bound(data_.cbegin(), data_.cend(), value, compare_);
    }

private:
    std::vector<T> data_;
    Compare compare_;
};

}  // namespace zen

#endif  // ZEN_CONTAINERS_MULTISET_H
