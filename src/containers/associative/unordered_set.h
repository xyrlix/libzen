/**
 * @file unordered_set.h
 * @brief 无序集合容器
 * @details 基于哈希表实现，提供平均 O(1) 的插入、删除和查找
 */

#ifndef ZEN_CONTAINERS_UNORDERED_SET_H
#define ZEN_CONTAINERS_UNORDERED_SET_H

#include <functional>
#include <vector>
#include <list>
#include <memory>
#include "../utility/utility.h"

namespace zen {

/**
 * @brief 哈希表迭代器
 */
template <typename T>
class UnorderedSetIterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = T;
    using difference_type = ptrdiff_t;
    using pointer = T*;
    using reference = T&;

    using BucketIterator = typename std::list<T>::iterator;

    UnorderedSetIterator() = default;
    UnorderedSetIterator(size_t bucket_index, BucketIterator it,
                         std::vector<std::list<T>>* buckets)
        : bucket_index_(bucket_index), it_(it), buckets_(buckets) {}

    reference operator*() const { return *it_; }
    pointer operator->() const { return &(*it_); }

    UnorderedSetIterator& operator++();
    UnorderedSetIterator operator++(int) {
        UnorderedSetIterator tmp = *this;
        ++(*this);
        return tmp;
    }

    friend bool operator==(const UnorderedSetIterator& a, const UnorderedSetIterator& b) {
        return a.bucket_index_ == b.bucket_index_ &&
               (!a.buckets_ || a.it_ == b.it_);
    }

    friend bool operator!=(const UnorderedSetIterator& a, const UnorderedSetIterator& b) {
        return !(a == b);
    }

private:
    size_t bucket_index_;
    BucketIterator it_;
    std::vector<std::list<T>>* buckets_;
};

template <typename T>
UnorderedSetIterator<T>& UnorderedSetIterator<T>::operator++() {
    if (!buckets_) return *this;

    ++it_;
    if (it_ == (*buckets_)[bucket_index_].end()) {
        // 移动到下一个非空桶
        for (++bucket_index_; bucket_index_ < buckets_->size(); ++bucket_index_) {
            if (!(*buckets_)[bucket_index_].empty()) {
                it_ = (*buckets_)[bucket_index_].begin();
                return *this;
            }
        }
        // 到达末尾
        bucket_index_ = buckets_->size();
        it_ = BucketIterator();
    }
    return *this;
}

/**
 * @brief 无序集合容器
 * @tparam T 元素类型
 * @tparam Hash 哈希函数
 * @tparam KeyEqual 相等比较函数
 */
template <
    typename T,
    typename Hash = std::hash<T>,
    typename KeyEqual = std::equal_to<T>
>
class UnorderedSet {
public:
    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = UnorderedSetIterator<T>;
    using const_iterator = UnorderedSetIterator<T>;

    /**
     * @brief 默认构造函数
     */
    UnorderedSet()
        : buckets_(new std::vector<std::list<T>>(8))
        , size_(0)
        , load_factor_threshold_(0.75f) {}

    explicit UnorderedSet(size_t bucket_count)
        : buckets_(new std::vector<std::list<T>>(bucket_count))
        , size_(0)
        , load_factor_threshold_(0.75f) {}

    UnorderedSet(std::initializer_list<T> init)
        : UnorderedSet() {
        for (const auto& item : init) {
            insert(item);
        }
    }

    ~UnorderedSet() = default;

    // 禁用拷贝，使用移动语义
    UnorderedSet(const UnorderedSet&) = delete;
    UnorderedSet& operator=(const UnorderedSet&) = delete;

    UnorderedSet(UnorderedSet&& other) noexcept = default;
    UnorderedSet& operator=(UnorderedSet&& other) noexcept = default;

    // ============================================================================
    // 迭代器
    // ============================================================================

    iterator begin() {
        for (size_t i = 0; i < buckets_->size(); ++i) {
            if (!(*buckets_)[i].empty()) {
                return iterator(i, (*buckets_)[i].begin(), buckets_.get());
            }
        }
        return end();
    }

    iterator end() {
        return iterator(buckets_->size(), typename std::list<T>::iterator(), buckets_.get());
    }

    const_iterator begin() const {
        return const_cast<UnorderedSet*>(this)->begin();
    }

    const_iterator end() const {
        return const_cast<UnorderedSet*>(this)->end();
    }

    const_iterator cbegin() const { return begin(); }
    const_iterator cend() const { return end(); }

    // ============================================================================
    // 容量
    // ============================================================================

    bool empty() const { return size_ == 0; }
    size_type size() const { return size_; }
    size_type bucket_count() const { return buckets_->size(); }

    /**
     * @brief 获取负载因子
     */
    float load_factor() const {
        return static_cast<float>(size_) / buckets_->size();
    }

    /**
     * @brief 设置最大负载因子
     */
    void max_load_factor(float lf) {
        load_factor_threshold_ = lf;
    }

    /**
     * @brief 重新哈希（指定桶数量）
     */
    void rehash(size_t count) {
        auto new_buckets = std::make_unique<std::vector<std::list<T>>>(count);
        size_t new_size = 0;

        // 重新插入所有元素
        for (auto& bucket : *buckets_) {
            for (auto& value : bucket) {
                size_t new_index = hash_(value) % count;
                (*new_buckets)[new_index].push_back(std::move(value));
                new_size++;
            }
        }

        buckets_ = std::move(new_buckets);
        size_ = new_size;
    }

    /**
     * @brief 预留空间
     */
    void reserve(size_type count) {
        size_t new_bucket_count = static_cast<size_t>(count / load_factor_threshold_) + 1;
        rehash(new_bucket_count);
    }

    // ============================================================================
    // 修改操作
    // ============================================================================

    /**
     * @brief 插入元素
     */
    std::pair<iterator, bool> insert(const T& value) {
        check_rehash();

        size_t index = hash_(value) % buckets_->size();
        auto& bucket = (*buckets_)[index];

        // 检查是否已存在
        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (key_equal_(*it, value)) {
                return {iterator(index, it, buckets_.get()), false};
            }
        }

        bucket.push_back(value);
        size_++;
        return {iterator(index, std::prev(bucket.end()), buckets_.get()), true};
    }

    std::pair<iterator, bool> insert(T&& value) {
        check_rehash();

        size_t index = hash_(value) % buckets_->size();
        auto& bucket = (*buckets_)[index];

        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (key_equal_(*it, value)) {
                return {iterator(index, it, buckets_.get()), false};
            }
        }

        bucket.push_back(std::move(value));
        size_++;
        return {iterator(index, std::prev(bucket.end()), buckets_.get()), true};
    }

    /**
     * @brief 删除元素
     */
    size_type erase(const T& value) {
        size_t index = hash_(value) % buckets_->size();
        auto& bucket = (*buckets_)[index];

        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (key_equal_(*it, value)) {
                bucket.erase(it);
                size_--;
                return 1;
            }
        }
        return 0;
    }

    iterator erase(const_iterator pos) {
        if (pos == end()) return end();

        size_t index = pos.bucket_index_;
        auto& bucket = (*buckets_)[index];
        auto it = pos.it_;
        auto next_it = bucket.erase(it);
        size_--;

        if (next_it != bucket.end()) {
            return iterator(index, next_it, buckets_.get());
        } else {
            return ++iterator(index, bucket.end(), buckets_.get());
        }
    }

    /**
     * @brief 清空集合
     */
    void clear() {
        buckets_ = std::make_unique<std::vector<std::list<T>>>(8);
        size_ = 0;
    }

    /**
     * @brief 交换集合
     */
    void swap(UnorderedSet& other) noexcept {
        std::swap(buckets_, other.buckets_);
        std::swap(size_, other.size_);
        std::swap(load_factor_threshold_, other.load_factor_threshold_);
    }

    /**
     * @brief 查找元素
     */
    iterator find(const T& value) {
        size_t index = hash_(value) % buckets_->size();
        auto& bucket = (*buckets_)[index];

        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (key_equal_(*it, value)) {
                return iterator(index, it, buckets_.get());
            }
        }
        return end();
    }

    const_iterator find(const T& value) const {
        return const_cast<UnorderedSet*>(this)->find(value);
    }

    /**
     * @brief 检查元素是否存在
     */
    bool contains(const T& value) const {
        return find(value) != end();
    }

    /**
     * @brief 计算元素数量
     */
    size_type count(const T& value) const {
        return contains(value) ? 1 : 0;
    }

private:
    std::unique_ptr<std::vector<std::list<T>>> buckets_;
    size_type size_;
    float load_factor_threshold_;
    Hash hash_;
    KeyEqual key_equal_;

    /**
     * @brief 检查是否需要重新哈希
     */
    void check_rehash() {
        if (load_factor() > load_factor_threshold_) {
            rehash(buckets_->size() * 2);
        }
    }
};

}  // namespace zen

#endif  // ZEN_CONTAINERS_UNORDERED_SET_H
