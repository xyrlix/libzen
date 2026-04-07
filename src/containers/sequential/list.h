#ifndef ZEN_CONTAINERS_SEQUENTIAL_LIST_H
#define ZEN_CONTAINERS_SEQUENTIAL_LIST_H

#include "../../base/type_traits.h"
#include "../../memory/allocator.h"
#include "../../iterators/iterator_base.h"
#include "../../utility/swap.h"

namespace zen {

// ============================================================================
// list - 双向链表
// ============================================================================

/**
 * @brief 双向链表
 * @tparam T         元素类型
 * @tparam Allocator 内存分配器类型
 *
 * list 是双向链表，每个节点保存数据及前后两个指针。
 *
 * 设计特点：
 * - **哨兵节点**（sentinel）：head_ 是一个伪节点（不存储数据），
 *   head_->next 是第一个真实节点，head_->prev 是最后一个真实节点。
 *   这种设计消除了大量的空指针判断，使插入/删除代码更简洁统一。
 *
 * 时间复杂度：
 * - push_front / push_back / insert：O(1)
 * - pop_front / pop_back / erase：O(1)
 * - operator[] / at（不提供）：链表不支持随机访问
 * - size：O(1)（维护 size_ 成员）
 * - find / 其他需要遍历的操作：O(n)
 *
 * 内存布局（6 个元素的示意）：
 *
 *   head (sentinel)
 *     ← [prev] [next] →
 *                ↓
 *            node1 ←→ node2 ←→ node3 ←→ ... ←→ nodeN
 *                                                 ↓
 *                                           head (sentinel, 循环)
 *
 * 示例：
 * @code
 * zen::list<int> lst;
 * lst.push_back(1);
 * lst.push_back(2);
 * lst.push_front(0);
 *
 * for (auto& x : lst) { ... }   // 0, 1, 2
 * lst.erase(lst.begin());        // 删除 0
 * @endcode
 */
template<typename T, typename Allocator = allocator<T>>
class list {
private:
    // ========================================================================
    // 节点定义
    // ========================================================================

    /**
     * @brief 链表节点
     *
     * 哨兵节点也是 Node 类型，但其 data 域不会被访问。
     */
    struct Node {
        T     data; // 元素数据（哨兵节点此域未使用）
        Node* prev; // 前驱指针
        Node* next; // 后继指针

        Node() : prev(nullptr), next(nullptr) {}

        // 带数据的构造（placement new 使用）
        template<typename... Args>
        Node(Args&&... args)
            : data(static_cast<Args&&>(args)...),
              prev(nullptr), next(nullptr) {
        }
    };

    // 节点分配器：从 T 的分配器 rebind 到 Node
    using NodeAllocator = typename Allocator::template rebind<Node>::other;

    // ========================================================================
    // 成员变量
    // ========================================================================

    Node*         head_;  // 哨兵节点（不存储数据，head_->next 是第一个节点）
    size_t        size_;  // 元素个数
    NodeAllocator alloc_; // 节点分配器

    // ========================================================================
    // 内部辅助
    // ========================================================================

    /**
     * @brief 分配并构造一个新节点
     */
    template<typename... Args>
    Node* create_node(Args&&... args) {
        Node* n = alloc_.allocate(1);
        // 先构造 Node 的 data 成员，再设置指针
        alloc_.construct(n, static_cast<Args&&>(args)...);
        return n;
    }

    /**
     * @brief 析构并释放一个节点
     */
    void destroy_node(Node* n) noexcept {
        alloc_.destroy(n);
        alloc_.deallocate(n, 1);
    }

    /**
     * @brief 分配并初始化哨兵节点
     *
     * 哨兵的 prev 和 next 都指向自己（空链表时构成自环）
     */
    Node* create_sentinel() {
        Node* s = alloc_.allocate(1);
        // 只初始化指针，不构造 data
        s->prev = s;
        s->next = s;
        return s;
    }

    /**
     * @brief 将节点 n 插入到 pos 之前
     *
     * 链接操作：... ← [pos->prev] ↔ [n] ↔ [pos] → ...
     */
    void link_before(Node* pos, Node* n) noexcept {
        n->prev    = pos->prev;
        n->next    = pos;
        pos->prev->next = n;
        pos->prev       = n;
    }

    /**
     * @brief 将节点 n 从链表中断开
     */
    void unlink(Node* n) noexcept {
        n->prev->next = n->next;
        n->next->prev = n->prev;
        // 安全：不重置 n->prev/next（调用者负责销毁 n）
    }

public:
    // ========================================================================
    // 迭代器
    // ========================================================================

    /**
     * @brief 双向迭代器
     */
    class iterator {
    public:
        using iterator_category = bidirectional_iterator_tag;
        using value_type        = T;
        using difference_type   = decltype((T*)0 - (T*)0);
        using pointer           = T*;
        using reference         = T&;

        Node* node_; // 当前节点

        iterator() : node_(nullptr) {}
        explicit iterator(Node* n) : node_(n) {}

        reference operator*() const noexcept { return node_->data; }
        pointer   operator->() const noexcept { return &node_->data; }

        // 前缀 ++
        iterator& operator++() noexcept {
            node_ = node_->next;
            return *this;
        }
        // 后缀 ++
        iterator operator++(int) noexcept {
            iterator tmp = *this;
            node_ = node_->next;
            return tmp;
        }
        // 前缀 --
        iterator& operator--() noexcept {
            node_ = node_->prev;
            return *this;
        }
        // 后缀 --
        iterator operator--(int) noexcept {
            iterator tmp = *this;
            node_ = node_->prev;
            return tmp;
        }

        bool operator==(const iterator& other) const noexcept {
            return node_ == other.node_;
        }
        bool operator!=(const iterator& other) const noexcept {
            return node_ != other.node_;
        }
    };

    /**
     * @brief const 双向迭代器
     */
    class const_iterator {
    public:
        using iterator_category = bidirectional_iterator_tag;
        using value_type        = T;
        using difference_type   = decltype((T*)0 - (T*)0);
        using pointer           = const T*;
        using reference         = const T&;

        const Node* node_;

        const_iterator() : node_(nullptr) {}
        explicit const_iterator(const Node* n) : node_(n) {}

        // 从非 const 迭代器隐式转换
        const_iterator(const iterator& it) : node_(it.node_) {}

        reference operator*() const noexcept { return node_->data; }
        pointer   operator->() const noexcept { return &node_->data; }

        const_iterator& operator++() noexcept {
            node_ = node_->next;
            return *this;
        }
        const_iterator operator++(int) noexcept {
            const_iterator tmp = *this;
            node_ = node_->next;
            return tmp;
        }
        const_iterator& operator--() noexcept {
            node_ = node_->prev;
            return *this;
        }
        const_iterator operator--(int) noexcept {
            const_iterator tmp = *this;
            node_ = node_->prev;
            return tmp;
        }

        bool operator==(const const_iterator& other) const noexcept {
            return node_ == other.node_;
        }
        bool operator!=(const const_iterator& other) const noexcept {
            return node_ != other.node_;
        }
    };

    using reverse_iterator_type  = reverse_iterator<iterator>;
    using const_reverse_iterator = reverse_iterator<const_iterator>;
    using value_type             = T;
    using size_type              = size_t;
    using reference              = T&;
    using const_reference        = const T&;
    using pointer                = T*;
    using const_pointer          = const T*;
    using difference_type        = decltype((T*)0 - (T*)0);

    // ========================================================================
    // 构造函数
    // ========================================================================

    /**
     * @brief 默认构造：空链表
     */
    list()
        : size_(0), alloc_() {
        head_ = create_sentinel();
    }

    explicit list(const Allocator& alloc)
        : size_(0), alloc_(alloc) {
        head_ = create_sentinel();
    }

    /**
     * @brief 构造含 count 个 value 的链表
     */
    list(size_type count, const T& value,
         const Allocator& alloc = Allocator())
        : size_(0), alloc_(alloc) {
        head_ = create_sentinel();
        for (size_type i = 0; i < count; ++i) {
            push_back(value);
        }
    }

    /**
     * @brief 拷贝构造
     */
    list(const list& other)
        : size_(0), alloc_(other.alloc_) {
        head_ = create_sentinel();
        for (const auto& v : other) {
            push_back(v);
        }
    }

    /**
     * @brief 移动构造
     */
    list(list&& other) noexcept
        : head_(other.head_), size_(other.size_), alloc_(static_cast<NodeAllocator&&>(other.alloc_)) {
        // 让 other 成为空链表
        other.head_ = other.create_sentinel();
        other.size_ = 0;
    }

    /**
     * @brief 析构：清空所有节点并释放哨兵
     */
    ~list() {
        clear();
        // 释放哨兵节点（只释放内存，不调用 data 析构）
        alloc_.deallocate(head_, 1);
    }

    // ========================================================================
    // 赋值运算符
    // ========================================================================

    list& operator=(const list& other) {
        if (this == &other) return *this;
        assign_from(other);
        return *this;
    }

    list& operator=(list&& other) noexcept {
        if (this == &other) return *this;
        clear();
        alloc_.deallocate(head_, 1);

        head_ = other.head_;
        size_ = other.size_;

        other.head_ = other.create_sentinel();
        other.size_ = 0;
        return *this;
    }

private:
    void assign_from(const list& other) {
        iterator it = begin();
        const_iterator oit = other.begin();

        // 复用已有节点（避免频繁分配）
        while (it != end() && oit != other.end()) {
            *it = *oit;
            ++it;
            ++oit;
        }

        // other 更长：追加
        while (oit != other.end()) {
            push_back(*oit);
            ++oit;
        }

        // 自身更长：删除多余
        while (it != end()) {
            it = erase(it);
        }
    }

public:
    // ========================================================================
    // 迭代器访问
    // ========================================================================

    iterator begin() noexcept {
        return iterator(head_->next);
    }
    const_iterator begin() const noexcept {
        return const_iterator(head_->next);
    }
    const_iterator cbegin() const noexcept {
        return const_iterator(head_->next);
    }

    // end() 指向哨兵节点（链表末尾之后）
    iterator end() noexcept {
        return iterator(head_);
    }
    const_iterator end() const noexcept {
        return const_iterator(head_);
    }
    const_iterator cend() const noexcept {
        return const_iterator(head_);
    }

    reverse_iterator_type rbegin() noexcept {
        return reverse_iterator_type(end());
    }
    reverse_iterator_type rend() noexcept {
        return reverse_iterator_type(begin());
    }

    // ========================================================================
    // 容量
    // ========================================================================

    bool empty() const noexcept {
        return size_ == 0;
    }

    size_type size() const noexcept {
        return size_;
    }

    // ========================================================================
    // 元素访问
    // ========================================================================

    T& front() noexcept {
        return head_->next->data;
    }

    const T& front() const noexcept {
        return head_->next->data;
    }

    T& back() noexcept {
        return head_->prev->data;
    }

    const T& back() const noexcept {
        return head_->prev->data;
    }

    // ========================================================================
    // 修改器
    // ========================================================================

    /**
     * @brief 在头部插入元素
     * @complexity O(1)
     */
    void push_front(const T& value) {
        Node* n = create_node(value);
        link_before(head_->next, n);
        ++size_;
    }

    void push_front(T&& value) {
        Node* n = create_node(static_cast<T&&>(value));
        link_before(head_->next, n);
        ++size_;
    }

    /**
     * @brief 在尾部插入元素
     * @complexity O(1)
     */
    void push_back(const T& value) {
        Node* n = create_node(value);
        link_before(head_, n); // 哨兵之前 = 链表末尾
        ++size_;
    }

    void push_back(T&& value) {
        Node* n = create_node(static_cast<T&&>(value));
        link_before(head_, n);
        ++size_;
    }

    /**
     * @brief 原地在尾部构造元素
     */
    template<typename... Args>
    reference emplace_back(Args&&... args) {
        Node* n = create_node(static_cast<Args&&>(args)...);
        link_before(head_, n);
        ++size_;
        return n->data;
    }

    /**
     * @brief 删除头部元素
     * @pre !empty()
     */
    void pop_front() noexcept {
        Node* n = head_->next;
        unlink(n);
        destroy_node(n);
        --size_;
    }

    /**
     * @brief 删除尾部元素
     * @pre !empty()
     */
    void pop_back() noexcept {
        Node* n = head_->prev;
        unlink(n);
        destroy_node(n);
        --size_;
    }

    /**
     * @brief 在 pos 之前插入元素
     * @return 指向新插入元素的迭代器
     * @complexity O(1)
     */
    iterator insert(const_iterator pos, const T& value) {
        Node* n = create_node(value);
        // 需要 non-const 节点指针
        Node* p = const_cast<Node*>(pos.node_);
        link_before(p, n);
        ++size_;
        return iterator(n);
    }

    iterator insert(const_iterator pos, T&& value) {
        Node* n = create_node(static_cast<T&&>(value));
        Node* p = const_cast<Node*>(pos.node_);
        link_before(p, n);
        ++size_;
        return iterator(n);
    }

    /**
     * @brief 删除 pos 处的元素
     * @return 指向被删除元素之后的迭代器
     * @complexity O(1)
     */
    iterator erase(const_iterator pos) {
        Node* n    = const_cast<Node*>(pos.node_);
        Node* next = n->next;
        unlink(n);
        destroy_node(n);
        --size_;
        return iterator(next);
    }

    /**
     * @brief 删除 [first, last) 范围内的元素
     */
    iterator erase(const_iterator first, const_iterator last) {
        while (first != last) {
            first = erase(first);
        }
        return iterator(const_cast<Node*>(last.node_));
    }

    /**
     * @brief 清空所有元素
     */
    void clear() noexcept {
        Node* cur = head_->next;
        while (cur != head_) {
            Node* next = cur->next;
            destroy_node(cur);
            cur = next;
        }
        // 重置哨兵
        head_->next = head_;
        head_->prev = head_;
        size_ = 0;
    }

    /**
     * @brief 交换两个链表
     * @complexity O(1)
     */
    void swap(list& other) noexcept {
        zen::swap(head_, other.head_);
        zen::swap(size_, other.size_);
        zen::swap(alloc_, other.alloc_);
    }

    // ========================================================================
    // 链表特有操作
    // ========================================================================

    /**
     * @brief 翻转链表
     * @complexity O(n)
     */
    void reverse() noexcept {
        Node* cur = head_;
        do {
            zen::swap(cur->prev, cur->next);
            cur = cur->prev; // 翻转后，prev 是原来的 next
        } while (cur != head_);
    }

    /**
     * @brief 删除所有等于 value 的元素
     * @complexity O(n)
     */
    void remove(const T& value) {
        iterator it = begin();
        while (it != end()) {
            if (*it == value) {
                it = erase(it);
            } else {
                ++it;
            }
        }
    }

    /**
     * @brief 删除所有满足谓词的元素
     */
    template<typename Predicate>
    void remove_if(Predicate pred) {
        iterator it = begin();
        while (it != end()) {
            if (pred(*it)) {
                it = erase(it);
            } else {
                ++it;
            }
        }
    }

    /**
     * @brief 删除连续重复元素（只保留第一个）
     * @complexity O(n)
     *
     * 注意：只删除相邻的重复元素。
     * 要删除所有重复元素，需要先排序。
     */
    void unique() {
        if (size_ <= 1) return;
        iterator cur  = begin();
        iterator next_it = cur;
        ++next_it;
        while (next_it != end()) {
            if (*cur == *next_it) {
                next_it = erase(next_it);
            } else {
                cur = next_it;
                ++next_it;
            }
        }
    }
};

// ============================================================================
// 非成员函数
// ============================================================================

template<typename T, typename A>
void swap(list<T, A>& a, list<T, A>& b) noexcept {
    a.swap(b);
}

template<typename T, typename A>
bool operator==(const list<T, A>& a, const list<T, A>& b) {
    if (a.size() != b.size()) return false;
    auto it1 = a.begin();
    auto it2 = b.begin();
    while (it1 != a.end()) {
        if (*it1 != *it2) return false;
        ++it1;
        ++it2;
    }
    return true;
}

template<typename T, typename A>
bool operator!=(const list<T, A>& a, const list<T, A>& b) {
    return !(a == b);
}

} // namespace zen

#endif // ZEN_CONTAINERS_SEQUENTIAL_LIST_H
