/**
 * @file multimap.h
 * @brief 多值映射容器
 * @details 允许键重复的映射容器
 */

#ifndef ZEN_CONTAINERS_MULTIMAP_H
#define ZEN_CONTAINERS_MULTIMAP_H

#include "set.h"
#include "multiset.h"
#include <vector>

namespace zen {

/**
 * @brief 键值对
 */
template <typename Key, typename Value>
struct Pair {
    Key first;
    Value second;

    Pair() = default;
    Pair(const Key& k, const Value& v) : first(k), second(v) {}
    Pair(Key&& k, Value&& v) : first(std::move(k)), second(std::move(v)) {}
};

/**
 * @brief 多值映射容器
 * @tparam Key 键类型
 * @tparam Value 值类型
 * @tparam Compare 比较函数
 * @details 基于红黑树实现，允许键重复
 */
template <typename Key, typename Value, typename Compare = std::less<Key>>
class MultiMap {
public:
    using key_type = Key;
    using mapped_type = Value;
    using value_type = Pair<Key, Value>;
    using size_type = size_t;
    using iterator = typename std::vector<value_type>::iterator;
    using const_iterator = typename std::vector<value_type>::const_iterator;

    MultiMap() = default;

    MultiMap(std::initializer_list<value_type> init) {
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
    // 元素访问
    // ============================================================================

    /**
     * @brief 插入键值对
     */
    iterator insert(const value_type& value) {
        auto it = std::lower_bound(data_.begin(), data_.end(), value,
            [](const value_type& a, const value_type& b) {
                return Compare()(a.first, b.first);
            });
        return data_.insert(it, value);
    }

    iterator insert(value_type&& value) {
        auto it = std::lower_bound(data_.begin(), data_.end(), value,
            [](const value_type& a, const value_type& b) {
                return Compare()(a.first, b.first);
            });
        return data_.insert(it, std::move(value));
    }

    iterator insert(const Key& key, const Value& value) {
        return insert(value_type(key, value));
    }

    /**
     * @brief 删除一个键值对
     */
    size_type erase(const Key& key) {
        auto it = find(key);
        if (it != data_.end()) {
            data_.erase(it);
            return 1;
        }
        return 0;
    }

    iterator erase(const_iterator pos) {
        return data_.erase(pos);
    }

    /**
     * @brief 删除所有指定键的键值对
     */
    size_type erase_all(const Key& key) {
        auto range = equal_range(key);
        size_type count = std::distance(range.first, range.second);
        data_.erase(range.first, range.second);
        return count;
    }

    /**
     * @brief 清空映射
     */
    void clear() { data_.clear(); }

    /**
     * @brief 交换映射
     */
    void swap(MultiMap& other) noexcept {
        data_.swap(other.data_);
    }

    /**
     * @brief 查找键
     */
    iterator find(const Key& key) {
        auto it = std::lower_bound(data_.begin(), data_.end(), key,
            [](const value_type& a, const Key& k) {
                return Compare()(a.first, k);
            });
        if (it != data_.end() && it->first == key) {
            return it;
        }
        return data_.end();
    }

    const_iterator find(const Key& key) const {
        auto it = std::lower_bound(data_.cbegin(), data_.cend(), key,
            [](const value_type& a, const Key& k) {
                return Compare()(a.first, k);
            });
        if (it != data_.cend() && it->first == key) {
            return it;
        }
        return data_.cend();
    }

    /**
     * @brief 计算键的数量
     */
    size_type count(const Key& key) const {
        auto range = equal_range(key);
        return std::distance(range.first, range.second);
    }

    /**
     * @brief 获取键的范围
     */
    std::pair<iterator, iterator> equal_range(const Key& key) {
        return std::equal_range(data_.begin(), data_.end(), key,
            [](const value_type& a, const Key& k) {
                return Compare()(a.first, k);
            });
    }

    std::pair<const_iterator, const_iterator> equal_range(const Key& key) const {
        return std::equal_range(data_.cbegin(), data_.cend(), key,
            [](const value_type& a, const Key& k) {
                return Compare()(a.first, k);
            });
    }

    /**
     * @brief 获取下界迭代器
     */
    iterator lower_bound(const Key& key) {
        return std::lower_bound(data_.begin(), data_.end(), key,
            [](const value_type& a, const Key& k) {
                return Compare()(a.first, k);
            });
    }

    const_iterator lower_bound(const Key& key) const {
        return std::lower_bound(data_.cbegin(), data_.cend(), key,
            [](const value_type& a, const Key& k) {
                return Compare()(a.first, k);
            });
    }

    /**
     * @brief 获取上界迭代器
     */
    iterator upper_bound(const Key& key) {
        return std::upper_bound(data_.begin(), data_.end(), key,
            [](const Key& k, const value_type& a) {
                return Compare()(k, a.first);
            });
    }

    const_iterator upper_bound(const Key& key) const {
        return std::upper_bound(data_.cbegin(), data_.cend(), key,
            [](const Key& k, const value_type& a) {
                return Compare()(k, a.first);
            });
    }

    /**
     * @brief 获取所有指定键的值
     */
    std::vector<mapped_type> get_values(const Key& key) const {
        std::vector<mapped_type> result;
        auto range = equal_range(key);
        for (auto it = range.first; it != range.second; ++it) {
            result.push_back(it->second);
        }
        return result;
    }

private:
    std::vector<value_type> data_;
};

}  // namespace zen

#endif  // ZEN_CONTAINERS_MULTIMAP_H
