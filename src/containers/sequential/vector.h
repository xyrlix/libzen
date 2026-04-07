#ifndef ZEN_CONTAINERS_SEQUENTIAL_VECTOR_H
#define ZEN_CONTAINERS_SEQUENTIAL_VECTOR_H

#include "../../base/type_traits.h"
#include "../../memory/allocator.h"
#include "../../iterators/iterator_base.h"
#include "../../utility/swap.h"

namespace zen {

// ============================================================================
// vector - 动态数组（连续内存）
// ============================================================================

/**
 * @brief 动态大小的连续内存数组
 * @tparam T         元素类型
 * @tparam Allocator 内存分配器类型
 *
 * vector 是最常用的序列容器，元素在内存中连续存储，
 * 支持 O(1) 随机访问和均摊 O(1) 的尾部插入。
 *
 * 核心设计：
 * - data_:     指向分配的内存块
 * - size_:     当前有效元素个数
 * - capacity_: 已分配的总容量
 *
 * 扩容策略：
 * - 容量不足时分配新内存（2 倍扩容）
 * - 将旧元素移动到新内存
 * - 释放旧内存
 *
 * 内存管理：
 * - 分配器负责内存的分配/释放
 * - placement new 负责元素的构造
 * - 显式析构函数调用负责元素的销毁
 *
 * 时间复杂度：
 * - operator[]、front、back：O(1)
 * - push_back：均摊 O(1)，最坏 O(n)（扩容时）
 * - insert/erase at end：O(1)
 * - insert/erase at middle：O(n)
 *
 * 示例：
 * @code
 * zen::vector<int> v;
 * v.push_back(1);
 * v.push_back(2);
 * v.push_back(3);
 *
 * for (auto x : v) { ... }    // 范围 for
 * v[0] = 10;                   // 随机访问
 * v.insert(v.begin(), 0);      // 头部插入 O(n)
 * v.erase(v.begin() + 1);      // 删除第二个元素
 * @endcode
 */
template<typename T, typename Allocator = allocator<T>>
class vector {
public:
    // ========================================================================
    // 类型定义
    // ========================================================================

    using value_type             = T;
    using allocator_type         = Allocator;
    using size_type              = decltype(sizeof(0));      // size_t
    using difference_type        = decltype((T*)0 - (T*)0); // ptrdiff_t
    using reference              = T&;
    using const_reference        = const T&;
    using pointer                = T*;
    using const_pointer          = const T*;

    // 使用原始指针作为迭代器（最简单且高效的实现）
    using iterator               = T*;
    using const_iterator         = const T*;
    using reverse_iterator_type  = reverse_iterator<iterator>;
    using const_reverse_iterator = reverse_iterator<const_iterator>;

private:
    T*         data_;      // 数据指针（指向分配的内存块）
    size_type  size_;      // 当前元素个数
    size_type  capacity_;  // 已分配的容量
    Allocator  alloc_;     // 分配器

    // ========================================================================
    // 内部辅助：内存管理
    // ========================================================================

    /**
     * @brief 分配并返回 new_cap 个 T 大小的内存块
     */
    T* alloc_memory(size_type new_cap) {
        return alloc_.allocate(new_cap);
    }

    /**
     * @brief 释放内存块
     */
    void free_memory(T* p, size_type n) noexcept {
        if (p != nullptr) {
            alloc_.deallocate(p, n);
        }
    }

    /**
     * @brief 销毁 [first, last) 范围内的所有元素（只调用析构，不释放内存）
     */
    void destroy_range(T* first, T* last) noexcept {
        for (T* p = first; p != last; ++p) {
            alloc_.destroy(p);
        }
    }

    /**
     * @brief 将 [src_first, src_last) 的元素移动构造到 dest 开始的位置
     *
     * 用于扩容时将旧元素"搬"到新内存。
     * 移动构造后旧元素处于有效但未指定的状态，需要再显式析构。
     */
    void move_construct_range(T* dest, T* src_first, T* src_last) {
        T* d = dest;
        for (T* s = src_first; s != src_last; ++s, ++d) {
            alloc_.construct(d, static_cast<T&&>(*s));
        }
    }

    /**
     * @brief 将 [src_first, src_last) 的元素拷贝构造到 dest 开始的位置
     */
    void copy_construct_range(T* dest, const T* src_first, const T* src_last) {
        T* d = dest;
        for (const T* s = src_first; s != src_last; ++s, ++d) {
            alloc_.construct(d, *s);
        }
    }

    /**
     * @brief 扩容到 new_cap
     *
     * 步骤：
     * 1. 分配新内存
     * 2. 移动旧元素到新内存
     * 3. 析构旧元素
     * 4. 释放旧内存
     * 5. 更新 data_ 和 capacity_
     */
    void reallocate(size_type new_cap) {
        T* new_data = alloc_memory(new_cap);

        // 移动旧元素（比拷贝更高效）
        move_construct_range(new_data, data_, data_ + size_);

        // 析构旧元素
        destroy_range(data_, data_ + size_);

        // 释放旧内存
        free_memory(data_, capacity_);

        // 更新指针和容量
        data_     = new_data;
        capacity_ = new_cap;
    }

    /**
     * @brief 根据当前 capacity_ 计算新容量（2 倍增长）
     */
    size_type grow_capacity() const noexcept {
        return capacity_ == 0 ? 1 : capacity_ * 2;
    }

    /**
     * @brief 确保至少有 min_cap 的容量，必要时扩容
     */
    void ensure_capacity(size_type min_cap) {
        if (min_cap <= capacity_) return;

        size_type new_cap = grow_capacity();
        if (new_cap < min_cap) new_cap = min_cap;

        reallocate(new_cap);
    }

public:
    // ========================================================================
    // 构造函数
    // ========================================================================

    /**
     * @brief 默认构造：空 vector
     */
    vector() noexcept
        : data_(nullptr), size_(0), capacity_(0), alloc_() {
    }

    /**
     * @brief 带分配器的默认构造
     */
    explicit vector(const Allocator& alloc) noexcept
        : data_(nullptr), size_(0), capacity_(0), alloc_(alloc) {
    }

    /**
     * @brief 构造含 count 个 value 的 vector
     * @param count 元素个数
     * @param value 初始值
     */
    explicit vector(size_type count, const T& value,
                    const Allocator& alloc = Allocator())
        : data_(nullptr), size_(0), capacity_(0), alloc_(alloc) {
        if (count > 0) {
            data_     = alloc_memory(count);
            capacity_ = count;
            for (size_type i = 0; i < count; ++i) {
                alloc_.construct(data_ + i, value);
            }
            size_ = count;
        }
    }

    /**
     * @brief 构造含 count 个默认值的 vector
     */
    explicit vector(size_type count,
                    const Allocator& alloc = Allocator())
        : data_(nullptr), size_(0), capacity_(0), alloc_(alloc) {
        if (count > 0) {
            data_     = alloc_memory(count);
            capacity_ = count;
            for (size_type i = 0; i < count; ++i) {
                alloc_.construct(data_ + i);
            }
            size_ = count;
        }
    }

    /**
     * @brief 拷贝构造
     */
    vector(const vector& other)
        : data_(nullptr), size_(0), capacity_(0), alloc_(other.alloc_) {
        if (other.size_ > 0) {
            data_     = alloc_memory(other.size_);
            capacity_ = other.size_;
            copy_construct_range(data_, other.data_, other.data_ + other.size_);
            size_ = other.size_;
        }
    }

    /**
     * @brief 移动构造：窃取资源，O(1)
     */
    vector(vector&& other) noexcept
        : data_(other.data_), size_(other.size_),
          capacity_(other.capacity_), alloc_(static_cast<Allocator&&>(other.alloc_)) {
        other.data_     = nullptr;
        other.size_     = 0;
        other.capacity_ = 0;
    }

    /**
     * @brief 析构：销毁所有元素并释放内存
     */
    ~vector() {
        destroy_range(data_, data_ + size_);
        free_memory(data_, capacity_);
    }

    // ========================================================================
    // 赋值运算符
    // ========================================================================

    /**
     * @brief 拷贝赋值
     */
    vector& operator=(const vector& other) {
        if (this == &other) return *this;

        // 销毁旧元素
        destroy_range(data_, data_ + size_);

        // 如果容量足够，直接拷贝构造
        if (other.size_ <= capacity_) {
            copy_construct_range(data_, other.data_,
                                 other.data_ + other.size_);
            size_ = other.size_;
        } else {
            // 容量不足，释放旧内存并重新分配
            free_memory(data_, capacity_);
            data_     = alloc_memory(other.size_);
            capacity_ = other.size_;
            copy_construct_range(data_, other.data_,
                                 other.data_ + other.size_);
            size_ = other.size_;
        }
        return *this;
    }

    /**
     * @brief 移动赋值：窃取资源，O(1)
     */
    vector& operator=(vector&& other) noexcept {
        if (this == &other) return *this;

        // 销毁当前内容
        destroy_range(data_, data_ + size_);
        free_memory(data_, capacity_);

        // 窃取 other 的资源
        data_     = other.data_;
        size_     = other.size_;
        capacity_ = other.capacity_;

        other.data_     = nullptr;
        other.size_     = 0;
        other.capacity_ = 0;

        return *this;
    }

    // ========================================================================
    // 元素访问
    // ========================================================================

    /**
     * @brief 下标访问（不检查边界）
     * @pre pos < size()
     */
    reference operator[](size_type pos) noexcept {
        return data_[pos];
    }

    const_reference operator[](size_type pos) const noexcept {
        return data_[pos];
    }

    /**
     * @brief 带边界检查的元素访问
     * @throw 若 pos >= size()，行为未定义（此实现无异常）
     */
    reference at(size_type pos) noexcept {
        return data_[pos]; // 简化：不抛异常
    }

    const_reference at(size_type pos) const noexcept {
        return data_[pos];
    }

    /**
     * @brief 访问第一个元素
     * @pre !empty()
     */
    reference front() noexcept {
        return data_[0];
    }

    const_reference front() const noexcept {
        return data_[0];
    }

    /**
     * @brief 访问最后一个元素
     * @pre !empty()
     */
    reference back() noexcept {
        return data_[size_ - 1];
    }

    const_reference back() const noexcept {
        return data_[size_ - 1];
    }

    /**
     * @brief 返回底层数组指针
     */
    T* data() noexcept {
        return data_;
    }

    const T* data() const noexcept {
        return data_;
    }

    // ========================================================================
    // 迭代器
    // ========================================================================

    iterator begin() noexcept { return data_; }
    const_iterator begin() const noexcept { return data_; }
    const_iterator cbegin() const noexcept { return data_; }

    iterator end() noexcept { return data_ + size_; }
    const_iterator end() const noexcept { return data_ + size_; }
    const_iterator cend() const noexcept { return data_ + size_; }

    reverse_iterator_type rbegin() noexcept {
        return reverse_iterator_type(end());
    }
    const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(end());
    }

    reverse_iterator_type rend() noexcept {
        return reverse_iterator_type(begin());
    }
    const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(begin());
    }

    // ========================================================================
    // 容量
    // ========================================================================

    /**
     * @brief 是否为空
     */
    bool empty() const noexcept {
        return size_ == 0;
    }

    /**
     * @brief 当前元素个数
     */
    size_type size() const noexcept {
        return size_;
    }

    /**
     * @brief 当前已分配容量
     */
    size_type capacity() const noexcept {
        return capacity_;
    }

    /**
     * @brief 预留至少 new_cap 的容量（不减小）
     */
    void reserve(size_type new_cap) {
        if (new_cap > capacity_) {
            reallocate(new_cap);
        }
    }

    /**
     * @brief 释放多余容量（shrink to fit）
     *
     * 将 capacity 缩减到与 size 相等，节省内存。
     */
    void shrink_to_fit() {
        if (size_ < capacity_) {
            if (size_ == 0) {
                free_memory(data_, capacity_);
                data_     = nullptr;
                capacity_ = 0;
            } else {
                reallocate(size_);
            }
        }
    }

    // ========================================================================
    // 修改器
    // ========================================================================

    /**
     * @brief 清空所有元素（不释放内存）
     */
    void clear() noexcept {
        destroy_range(data_, data_ + size_);
        size_ = 0;
    }

    /**
     * @brief 在尾部追加元素（拷贝版本）
     *
     * 若容量不足，扩容为原来的 2 倍；
     * 在 data_[size_] 上 placement new 构造元素
     */
    void push_back(const T& value) {
        ensure_capacity(size_ + 1);
        alloc_.construct(data_ + size_, value);
        ++size_;
    }

    /**
     * @brief 在尾部追加元素（移动版本）
     */
    void push_back(T&& value) {
        ensure_capacity(size_ + 1);
        alloc_.construct(data_ + size_, static_cast<T&&>(value));
        ++size_;
    }

    /**
     * @brief 在尾部原地构造元素（emplace_back）
     */
    template<typename... Args>
    reference emplace_back(Args&&... args) {
        ensure_capacity(size_ + 1);
        alloc_.construct(data_ + size_, static_cast<Args&&>(args)...);
        ++size_;
        return back();
    }

    /**
     * @brief 移除最后一个元素
     * @pre !empty()
     */
    void pop_back() noexcept {
        --size_;
        alloc_.destroy(data_ + size_);
    }

    /**
     * @brief 在 pos 之前插入元素（拷贝）
     * @param pos 插入位置的迭代器
     * @param value 要插入的值
     * @return 指向新插入元素的迭代器
     *
     * 插入步骤：
     * 1. 扩容（若需要）
     * 2. 从 pos 开始的元素向后移动一位（为新元素腾位置）
     * 3. 在 pos 位置构造新元素
     */
    iterator insert(const_iterator pos, const T& value) {
        // 计算插入位置索引（扩容前保存，因为扩容会使 pos 失效）
        size_type idx = static_cast<size_type>(pos - data_);

        ensure_capacity(size_ + 1);

        // 从末尾向前移动元素，为新元素腾出空间
        // 注意：需要从右往左移动，避免覆盖
        if (idx < size_) {
            // 在最后一位后面构造（相当于移动到新位置）
            alloc_.construct(data_ + size_, static_cast<T&&>(data_[size_ - 1]));
            // 将 [idx, size_-1) 范围内的元素向右移动一位
            for (size_type i = size_ - 1; i > idx; --i) {
                data_[i] = static_cast<T&&>(data_[i - 1]);
            }
            // 在 idx 处赋值（已构造，直接赋值）
            data_[idx] = value;
        } else {
            // 插入到末尾
            alloc_.construct(data_ + size_, value);
        }

        ++size_;
        return data_ + idx;
    }

    /**
     * @brief 在 pos 之前插入元素（移动）
     */
    iterator insert(const_iterator pos, T&& value) {
        size_type idx = static_cast<size_type>(pos - data_);

        ensure_capacity(size_ + 1);

        if (idx < size_) {
            alloc_.construct(data_ + size_, static_cast<T&&>(data_[size_ - 1]));
            for (size_type i = size_ - 1; i > idx; --i) {
                data_[i] = static_cast<T&&>(data_[i - 1]);
            }
            data_[idx] = static_cast<T&&>(value);
        } else {
            alloc_.construct(data_ + size_, static_cast<T&&>(value));
        }

        ++size_;
        return data_ + idx;
    }

    /**
     * @brief 删除 pos 处的元素
     * @param pos 要删除的元素位置
     * @return 指向被删除元素之后元素的迭代器
     *
     * 删除步骤：
     * 1. 将 [pos+1, end) 的元素向前移动一位
     * 2. 析构最后一个元素（已被移走）
     */
    iterator erase(const_iterator pos) {
        size_type idx = static_cast<size_type>(pos - data_);

        // 向前移动后续元素
        for (size_type i = idx; i + 1 < size_; ++i) {
            data_[i] = static_cast<T&&>(data_[i + 1]);
        }

        // 析构最后一个元素
        --size_;
        alloc_.destroy(data_ + size_);

        return data_ + idx;
    }

    /**
     * @brief 删除 [first, last) 范围内的元素
     * @return 指向 last 原来位置的迭代器
     */
    iterator erase(const_iterator first, const_iterator last) {
        size_type idx_first = static_cast<size_type>(first - data_);
        size_type idx_last  = static_cast<size_type>(last - data_);
        size_type count     = idx_last - idx_first;

        if (count == 0) return data_ + idx_first;

        // 移动后续元素填补空缺
        for (size_type i = idx_first; i + count < size_; ++i) {
            data_[i] = static_cast<T&&>(data_[i + count]);
        }

        // 析构末尾 count 个元素
        for (size_type i = size_ - count; i < size_; ++i) {
            alloc_.destroy(data_ + i);
        }

        size_ -= count;
        return data_ + idx_first;
    }

    /**
     * @brief 调整大小为 count
     *
     * - 若 count > size_：在末尾追加默认构造的元素
     * - 若 count < size_：删除末尾多余的元素
     */
    void resize(size_type count) {
        if (count > size_) {
            ensure_capacity(count);
            for (size_type i = size_; i < count; ++i) {
                alloc_.construct(data_ + i);
            }
            size_ = count;
        } else if (count < size_) {
            destroy_range(data_ + count, data_ + size_);
            size_ = count;
        }
    }

    /**
     * @brief 调整大小为 count，新元素用 value 填充
     */
    void resize(size_type count, const T& value) {
        if (count > size_) {
            ensure_capacity(count);
            for (size_type i = size_; i < count; ++i) {
                alloc_.construct(data_ + i, value);
            }
            size_ = count;
        } else if (count < size_) {
            destroy_range(data_ + count, data_ + size_);
            size_ = count;
        }
    }

    /**
     * @brief 填充 count 个 value（清空现有内容）
     */
    void assign(size_type count, const T& value) {
        clear();
        ensure_capacity(count);
        for (size_type i = 0; i < count; ++i) {
            alloc_.construct(data_ + i, value);
        }
        size_ = count;
    }

    /**
     * @brief 交换两个 vector 的内容（O(1)）
     */
    void swap(vector& other) noexcept {
        zen::swap(data_,     other.data_);
        zen::swap(size_,     other.size_);
        zen::swap(capacity_, other.capacity_);
        zen::swap(alloc_,    other.alloc_);
    }
};

// ============================================================================
// 非成员函数
// ============================================================================

/**
 * @brief 非成员 swap
 */
template<typename T, typename A>
void swap(vector<T, A>& a, vector<T, A>& b) noexcept {
    a.swap(b);
}

/**
 * @brief 相等比较
 */
template<typename T, typename A>
bool operator==(const vector<T, A>& a, const vector<T, A>& b) {
    if (a.size() != b.size()) return false;
    for (decltype(a.size()) i = 0; i < a.size(); ++i) {
        if (a[i] != b[i]) return false;
    }
    return true;
}

template<typename T, typename A>
bool operator!=(const vector<T, A>& a, const vector<T, A>& b) {
    return !(a == b);
}

/**
 * @brief 字典序比较
 */
template<typename T, typename A>
bool operator<(const vector<T, A>& a, const vector<T, A>& b) {
    decltype(a.size()) i = 0;
    for (; i < a.size() && i < b.size(); ++i) {
        if (a[i] < b[i]) return true;
        if (b[i] < a[i]) return false;
    }
    return i == a.size() && i < b.size();
}

template<typename T, typename A>
bool operator>(const vector<T, A>& a, const vector<T, A>& b) {
    return b < a;
}

template<typename T, typename A>
bool operator<=(const vector<T, A>& a, const vector<T, A>& b) {
    return !(b < a);
}

template<typename T, typename A>
bool operator>=(const vector<T, A>& a, const vector<T, A>& b) {
    return !(a < b);
}

} // namespace zen

#endif // ZEN_CONTAINERS_SEQUENTIAL_VECTOR_H
