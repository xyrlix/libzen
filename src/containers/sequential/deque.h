/**
 * @file deque.h
 * @brief 双端队列容器
 * @details 支持头部和尾部的高效插入和删除操作
 *          基于循环缓冲区实现
 */

#ifndef ZEN_CONTAINERS_DEQUE_H
#define ZEN_CONTAINERS_DEQUE_H

#include <initializer_list>
#include <stdexcept>
#include <algorithm>
#include "../memory/allocator.h"
#include "../iterators/iterator.h"

namespace zen {

// ============================================================================
// 双端队列迭代器
// ============================================================================

template <typename T>
class DequeIterator {
public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = T;
    using difference_type = ptrdiff_t;
    using pointer = T*;
    using reference = T&;

    DequeIterator() : data_(nullptr), index_(0), capacity_(0) {}
    DequeIterator(pointer data, size_t index, size_t capacity)
        : data_(data), index_(index), capacity_(capacity) {}

    reference operator*() const { return data_[index_]; }
    pointer operator->() const { return &data_[index_]; }

    DequeIterator& operator++() {
        index_ = (index_ + 1) % capacity_;
        return *this;
    }

    DequeIterator operator++(int) {
        DequeIterator tmp = *this;
        ++(*this);
        return tmp;
    }

    DequeIterator& operator--() {
        index_ = (index_ + capacity_ - 1) % capacity_;
        return *this;
    }

    DequeIterator operator--(int) {
        DequeIterator tmp = *this;
        --(*this);
        return tmp;
    }

    DequeIterator& operator+=(difference_type n) {
        index_ = (index_ + n + capacity_) % capacity_;
        return *this;
    }

    DequeIterator operator+(difference_type n) const {
        DequeIterator tmp = *this;
        tmp += n;
        return tmp;
    }

    DequeIterator& operator-=(difference_type n) {
        index_ = (index_ - n + capacity_) % capacity_;
        return *this;
    }

    DequeIterator operator-(difference_type n) const {
        DequeIterator tmp = *this;
        tmp -= n;
        return tmp;
    }

    reference operator[](difference_type n) const {
        return data_[(index_ + n + capacity_) % capacity_];
    }

    friend difference_type operator-(const DequeIterator& a, const DequeIterator& b) {
        return static_cast<difference_type>(a.index_) - static_cast<difference_type>(b.index_);
    }

    friend bool operator==(const DequeIterator& a, const DequeIterator& b) {
        return a.index_ == b.index_;
    }

    friend bool operator!=(const DequeIterator& a, const DequeIterator& b) {
        return a.index_ != b.index_;
    }

    friend bool operator<(const DequeIterator& a, const DequeIterator& b) {
        return a.index_ < b.index_;
    }

    friend bool operator>(const DequeIterator& a, const DequeIterator& b) {
        return a.index_ > b.index_;
    }

    friend bool operator<=(const DequeIterator& a, const DequeIterator& b) {
        return a.index_ <= b.index_;
    }

    friend bool operator>=(const DequeIterator& a, const DequeIterator& b) {
        return a.index_ >= b.index_;
    }

private:
    pointer data_;
    size_t index_;
    size_t capacity_;
};

// ============================================================================
// 双端队列
// ============================================================================

/**
 * @brief 双端队列容器
 * @tparam T 元素类型
 */
template <typename T>
class Deque {
public:
    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T&;
    using iterator = DequeIterator<T>;
    using const_iterator = DequeIterator<const T>;

    // ============================================================================
    // 构造函数
    // ============================================================================

    Deque() : data_(nullptr), capacity_(0), size_(0), head_(0), tail_(0) {}

    explicit Deque(size_type count) : Deque() {
        resize(count);
    }

    Deque(size_type count, const T& value) : Deque() {
        reserve(count * 2);
        for (size_type i = 0; i < count; ++i) {
            push_back(value);
        }
    }

    Deque(std::initializer_list<T> init) : Deque() {
        reserve(init.size() * 2);
        for (const auto& item : init) {
            push_back(item);
        }
    }

    Deque(const Deque& other) : Deque() {
        reserve(other.capacity_);
        for (size_type i = 0; i < other.size_; ++i) {
            push_back(other[i]);
        }
    }

    Deque(Deque&& other) noexcept
        : data_(other.data_)
        , capacity_(other.capacity_)
        , size_(other.size_)
        , head_(other.head_)
        , tail_(other.tail_) {
        other.data_ = nullptr;
        other.capacity_ = 0;
        other.size_ = 0;
        other.head_ = 0;
        other.tail_ = 0;
    }

    ~Deque() {
        clear();
        deallocate(data_, capacity_);
    }

    Deque& operator=(const Deque& other) {
        if (this != &other) {
            Deque tmp(other);
            swap(tmp);
        }
        return *this;
    }

    Deque& operator=(Deque&& other) noexcept {
        if (this != &other) {
            clear();
            deallocate(data_, capacity_);

            data_ = other.data_;
            capacity_ = other.capacity_;
            size_ = other.size_;
            head_ = other.head_;
            tail_ = other.tail_;

            other.data_ = nullptr;
            other.capacity_ = 0;
            other.size_ = 0;
            other.head_ = 0;
            other.tail_ = 0;
        }
        return *this;
    }

    Deque& operator=(std::initializer_list<T> init) {
        Deque tmp(init);
        swap(tmp);
        return *this;
    }

    // ============================================================================
    // 元素访问
    // ============================================================================

    reference operator[](size_type index) {
        return data_[(head_ + index) % capacity_];
    }

    const_reference operator[](size_type index) const {
        return data_[(head_ + index) % capacity_];
    }

    reference at(size_type index) {
        if (index >= size_) {
            throw std::out_of_range("Deque index out of range");
        }
        return operator[](index);
    }

    const_reference at(size_type index) const {
        if (index >= size_) {
            throw std::out_of_range("Deque index out of range");
        }
        return operator[](index);
    }

    reference front() {
        return data_[head_];
    }

    const_reference front() const {
        return data_[head_];
    }

    reference back() {
        return data_[(tail_ + capacity_ - 1) % capacity_];
    }

    const_reference back() const {
        return data_[(tail_ + capacity_ - 1) % capacity_];
    }

    // ============================================================================
    // 迭代器
    // ============================================================================

    iterator begin() { return iterator(data_, head_, capacity_); }
    iterator end() { return iterator(data_, tail_, capacity_); }

    const_iterator begin() const { return const_iterator(data_, head_, capacity_); }
    const_iterator end() const { return const_iterator(data_, tail_, capacity_); }

    const_iterator cbegin() const { return const_iterator(data_, head_, capacity_); }
    const_iterator cend() const { return const_iterator(data_, tail_, capacity_); }

    // ============================================================================
    // 容量
    // ============================================================================

    bool empty() const { return size_ == 0; }
    size_type size() const { return size_; }

    /**
     * @brief 设置容量大小
     */
    void resize(size_type new_size) {
        if (new_size > size_) {
            reserve(new_size * 2);
            while (size_ < new_size) {
                push_back(T());
            }
        } else {
            while (size_ > new_size) {
                pop_back();
            }
        }
    }

    /**
     * @brief 预留容量
     */
    void reserve(size_type new_capacity) {
        if (new_capacity <= capacity_) {
            return;
        }

        pointer new_data = allocate(new_capacity);

        // 拷贝现有元素
        for (size_type i = 0; i < size_; ++i) {
            construct(new_data + i, data_[(head_ + i) % capacity_]);
        }

        // 销毁旧元素
        for (size_type i = 0; i < size_; ++i) {
            destroy(data_[(head_ + i) % capacity_]);
        }

        deallocate(data_, capacity_);

        data_ = new_data;
        capacity_ = new_capacity;
        head_ = 0;
        tail_ = size_;
    }

    // ============================================================================
    // 修改操作
    // ============================================================================

    /**
     * @brief 在尾部添加元素
     */
    void push_back(const T& value) {
        if (size_ == capacity_) {
            reserve(capacity_ == 0 ? 4 : capacity_ * 2);
        }
        construct(data_ + tail_, value);
        tail_ = (tail_ + 1) % capacity_;
        size_++;
    }

    void push_back(T&& value) {
        if (size_ == capacity_) {
            reserve(capacity_ == 0 ? 4 : capacity_ * 2);
        }
        construct(data_ + tail_, std::move(value));
        tail_ = (tail_ + 1) % capacity_;
        size_++;
    }

    /**
     * @brief 在头部添加元素
     */
    void push_front(const T& value) {
        if (size_ == capacity_) {
            reserve(capacity_ == 0 ? 4 : capacity_ * 2);
        }
        head_ = (head_ + capacity_ - 1) % capacity_;
        construct(data_ + head_, value);
        size_++;
    }

    void push_front(T&& value) {
        if (size_ == capacity_) {
            reserve(capacity_ == 0 ? 4 : capacity_ * 2);
        }
        head_ = (head_ + capacity_ - 1) % capacity_;
        construct(data_ + head_, std::move(value));
        size_++;
    }

    /**
     * @brief 在尾部移除元素
     */
    void pop_back() {
        if (empty()) {
            throw std::out_of_range("Deque is empty");
        }
        tail_ = (tail_ + capacity_ - 1) % capacity_;
        destroy(data_ + tail_);
        size_--;
    }

    /**
     * @brief 在头部移除元素
     */
    void pop_front() {
        if (empty()) {
            throw std::out_of_range("Deque is empty");
        }
        destroy(data_ + head_);
        head_ = (head_ + 1) % capacity_;
        size_--;
    }

    /**
     * @brief 清空队列
     */
    void clear() {
        while (!empty()) {
            pop_front();
        }
    }

    /**
     * @brief 交换两个队列
     */
    void swap(Deque& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(capacity_, other.capacity_);
        std::swap(size_, other.size_);
        std::swap(head_, other.head_);
        std::swap(tail_, other.tail_);
    }

    /**
     * @brief 生成 emplace 辅助函数
     */
    template <typename... Args>
    void emplace_back(Args&&... args) {
        push_back(T(std::forward<Args>(args)...));
    }

    template <typename... Args>
    void emplace_front(Args&&... args) {
        push_front(T(std::forward<Args>(args)...));
    }

    template <typename... Args>
    iterator emplace(const_iterator pos, Args&&... args) {
        size_type index = pos - begin();
        if (index == size_) {
            emplace_back(std::forward<Args>(args)...);
        } else if (index == 0) {
            emplace_front(std::forward<Args>(args)...);
        } else {
            // 简单实现：先在头部插入，然后交换到正确位置
            emplace_front(std::forward<Args>(args)...);
            for (size_type i = 0; i < index; ++i) {
                std::swap(front(), (*this)[i + 1]);
            }
        }
        return begin() + index;
    }

    /**
     * @brief 删除指定位置的元素
     */
    iterator erase(const_iterator pos) {
        size_type index = pos - begin();

        // 将元素向前移动
        for (size_type i = index; i < size_ - 1; ++i) {
            data_[(head_ + i) % capacity_] = std::move(data_[(head_ + i + 1) % capacity_]);
        }

        tail_ = (tail_ + capacity_ - 1) % capacity_;
        destroy(data_ + tail_);
        size_--;

        return begin() + index;
    }

    iterator erase(const_iterator first, const_iterator last) {
        size_type first_index = first - begin();
        size_type last_index = last - begin();
        size_type count = last_index - first_index;

        // 将元素向前移动
        for (size_type i = first_index; i < size_ - count; ++i) {
            data_[(head_ + i) % capacity_] = std::move(data_[(head_ + i + count) % capacity_]);
        }

        // 销毁尾部元素
        for (size_type i = 0; i < count; ++i) {
            tail_ = (tail_ + capacity_ - 1) % capacity_;
            destroy(data_ + tail_);
        }
        size_ -= count;

        return begin() + first_index;
    }

    /**
     * @brief 在指定位置插入元素
     */
    iterator insert(const_iterator pos, const T& value) {
        size_type index = pos - begin();
        if (index >= size_) {
            push_back(value);
            return end() - 1;
        } else if (index == 0) {
            push_front(value);
            return begin();
        } else {
            // 先在尾部添加，然后旋转元素
            push_back(value);
            for (size_type i = size_ - 1; i > index; --i) {
                std::swap((*this)[i], (*this)[i - 1]);
            }
            return begin() + index;
        }
    }

    iterator insert(const_iterator pos, T&& value) {
        size_type index = pos - begin();
        if (index >= size_) {
            push_back(std::move(value));
            return end() - 1;
        } else if (index == 0) {
            push_front(std::move(value));
            return begin();
        } else {
            push_back(std::move(value));
            for (size_type i = size_ - 1; i > index; --i) {
                std::swap((*this)[i], (*this)[i - 1]);
            }
            return begin() + index;
        }
    }

private:
    pointer data_;
    size_type capacity_;
    size_type size_;
    size_type head_;
    size_type tail_;

    static pointer allocate(size_type n) {
        return static_cast<pointer>(::operator new(n * sizeof(T)));
    }

    static void deallocate(pointer p, size_type) {
        ::operator delete(p);
    }

    static void construct(pointer p, const T& value) {
        ::new (static_cast<void*>(p)) T(value);
    }

    static void construct(pointer p, T&& value) {
        ::new (static_cast<void*>(p)) T(std::move(value));
    }

    static void destroy(pointer p) {
        p->~T();
    }
};

}  // namespace zen

#endif  // ZEN_CONTAINERS_DEQUE_H
