#ifndef ZEN_CONTAINERS_ASSOCIATIVE_MAP_H
#define ZEN_CONTAINERS_ASSOCIATIVE_MAP_H

#include "../../base/type_traits.h"
#include "../../utility/swap.h"
#include "../../utility/pair.h"
#include "../../memory/allocator.h"
#include "../../iterators/iterator_base.h"

namespace zen {

// ============================================================================
// 比较器
// ============================================================================

/**
 * @brief 默认小于比较器
 */
template<typename T>
struct less {
    constexpr bool operator()(const T& a, const T& b) const noexcept {
        return a < b;
    }
};

// ============================================================================
// 红黑树节点颜色
// ============================================================================

enum class rb_color : unsigned char { RED = 0, BLACK = 1 };

// ============================================================================
// 红黑树节点
// ============================================================================

template<typename Key, typename Value>
struct rb_node {
    using value_type = pair<const Key, Value>;

    value_type  kv;         ///< 键值对（key 为 const 防止意外修改）
    rb_color    color;
    rb_node*    parent;
    rb_node*    left;
    rb_node*    right;

    template<typename... Args>
    explicit rb_node(Args&&... args)
        : kv(static_cast<Args&&>(args)...)
        , color(rb_color::RED)
        , parent(nullptr)
        , left(nullptr)
        , right(nullptr)
    {}
};

// ============================================================================
// 红黑树迭代器（中序遍历）
// ============================================================================

template<typename Key, typename Value>
struct rb_iterator {
    using node_type         = rb_node<Key, Value>;
    using value_type        = pair<const Key, Value>;
    using reference         = value_type&;
    using pointer           = value_type*;
    using difference_type   = decltype((char*)0 - (char*)0);
    using iterator_category = bidirectional_iterator_tag;

    node_type* node_;   ///< 当前节点（nullptr 表示 end()）
    node_type* nil_;    ///< 哨兵节点（NIL）

    rb_iterator() noexcept : node_(nullptr), nil_(nullptr) {}
    explicit rb_iterator(node_type* n, node_type* nil) noexcept
        : node_(n), nil_(nil) {}

    reference operator*()  const noexcept { return node_->kv; }
    pointer   operator->() const noexcept { return &node_->kv; }

    // 前缀++（中序后继）
    rb_iterator& operator++() noexcept {
        if (node_->right != nil_) {
            // 右子树最左节点
            node_ = node_->right;
            while (node_->left != nil_) {
                node_ = node_->left;
            }
        } else {
            // 往上找第一个"从左子树来"的祖先
            node_type* p = node_->parent;
            while (p != nil_ && node_ == p->right) {
                node_ = p;
                p = p->parent;
            }
            node_ = p;
        }
        return *this;
    }

    rb_iterator operator++(int) noexcept {
        rb_iterator tmp = *this;
        ++(*this);
        return tmp;
    }

    // 前缀--（中序前驱）
    rb_iterator& operator--() noexcept {
        if (node_ == nil_) {
            // end() -- 找最右节点
            node_type* cur = nil_->parent; // root
            while (cur != nil_ && cur->right != nil_) {
                cur = cur->right;
            }
            node_ = cur;
        } else if (node_->left != nil_) {
            node_ = node_->left;
            while (node_->right != nil_) {
                node_ = node_->right;
            }
        } else {
            node_type* p = node_->parent;
            while (p != nil_ && node_ == p->left) {
                node_ = p;
                p = p->parent;
            }
            node_ = p;
        }
        return *this;
    }

    rb_iterator operator--(int) noexcept {
        rb_iterator tmp = *this;
        --(*this);
        return tmp;
    }

    bool operator==(const rb_iterator& other) const noexcept {
        return node_ == other.node_;
    }
    bool operator!=(const rb_iterator& other) const noexcept {
        return node_ != other.node_;
    }
};

// const 迭代器
template<typename Key, typename Value>
struct rb_const_iterator {
    using node_type         = const rb_node<Key, Value>;
    using value_type        = const pair<const Key, Value>;
    using reference         = const pair<const Key, Value>&;
    using pointer           = const pair<const Key, Value>*;
    using difference_type   = decltype((char*)0 - (char*)0);
    using iterator_category = bidirectional_iterator_tag;

    const rb_node<Key, Value>* node_;
    const rb_node<Key, Value>* nil_;

    rb_const_iterator() noexcept : node_(nullptr), nil_(nullptr) {}
    explicit rb_const_iterator(const rb_node<Key, Value>* n,
                                const rb_node<Key, Value>* nil) noexcept
        : node_(n), nil_(nil) {}

    // 从非 const 迭代器构造
    rb_const_iterator(const rb_iterator<Key, Value>& it) noexcept
        : node_(it.node_), nil_(it.nil_) {}

    reference operator*()  const noexcept { return node_->kv; }
    pointer   operator->() const noexcept { return &node_->kv; }

    rb_const_iterator& operator++() noexcept {
        if (node_->right != nil_) {
            node_ = node_->right;
            while (node_->left != nil_) node_ = node_->left;
        } else {
            const rb_node<Key, Value>* p = node_->parent;
            while (p != nil_ && node_ == p->right) {
                node_ = p;
                p = p->parent;
            }
            node_ = p;
        }
        return *this;
    }

    rb_const_iterator operator++(int) noexcept {
        rb_const_iterator tmp = *this;
        ++(*this);
        return tmp;
    }

    rb_const_iterator& operator--() noexcept {
        if (node_ == nil_) {
            const rb_node<Key, Value>* cur = nil_->parent;
            while (cur != nil_ && cur->right != nil_) cur = cur->right;
            node_ = cur;
        } else if (node_->left != nil_) {
            node_ = node_->left;
            while (node_->right != nil_) node_ = node_->right;
        } else {
            const rb_node<Key, Value>* p = node_->parent;
            while (p != nil_ && node_ == p->left) {
                node_ = p;
                p = p->parent;
            }
            node_ = p;
        }
        return *this;
    }

    rb_const_iterator operator--(int) noexcept {
        rb_const_iterator tmp = *this;
        --(*this);
        return tmp;
    }

    bool operator==(const rb_const_iterator& other) const noexcept {
        return node_ == other.node_;
    }
    bool operator!=(const rb_const_iterator& other) const noexcept {
        return node_ != other.node_;
    }
};

// ============================================================================
// map 容器（基于红黑树）
// ============================================================================

/**
 * @brief 有序关联容器，基于红黑树实现
 *
 * 红黑树性质：
 *  1. 每个节点是红色或黑色
 *  2. 根节点是黑色
 *  3. 所有叶节点（NIL）是黑色
 *  4. 红色节点的两个子节点都是黑色（红色节点不能相邻）
 *  5. 从任一节点到其叶节点的所有路径上黑色节点数相同
 *
 * 操作时间复杂度：
 *  - find / insert / erase：O(log n)
 *  - begin / end：O(1)
 *  - 中序遍历：O(n)
 *
 * @tparam Key     键类型
 * @tparam Value   值类型
 * @tparam Compare 比较器，默认为 less<Key>
 * @tparam Alloc   分配器，默认为 allocator<pair<const Key, Value>>
 */
template<typename Key,
         typename Value,
         typename Compare = less<Key>,
         typename Alloc = allocator<pair<const Key, Value>>>
class map {
public:
    // -------------------------------------------------------------------------
    // 类型定义
    // -------------------------------------------------------------------------
    using key_type        = Key;
    using mapped_type     = Value;
    using value_type      = pair<const Key, Value>;
    using size_type       = size_t;
    using difference_type = decltype((char*)0 - (char*)0);
    using key_compare     = Compare;
    using reference       = value_type&;
    using const_reference = const value_type&;
    using pointer         = value_type*;
    using const_pointer   = const value_type*;
    using iterator        = rb_iterator<Key, Value>;
    using const_iterator  = rb_const_iterator<Key, Value>;

private:
    // -------------------------------------------------------------------------
    // 内部类型
    // -------------------------------------------------------------------------
    using node_type  = rb_node<Key, Value>;
    using node_alloc = typename Alloc::template rebind<node_type>::other;

    // -------------------------------------------------------------------------
    // 成员变量
    // -------------------------------------------------------------------------
    node_type*  nil_;       ///< 哨兵节点（所有叶子、根的父）
    node_type*  root_;      ///< 根节点（初始时等于 nil_）
    size_type   size_;      ///< 节点数量
    Compare     comp_;
    node_alloc  alloc_;

    // -------------------------------------------------------------------------
    // 内部辅助
    // -------------------------------------------------------------------------

    /** 分配并构造节点 */
    template<typename... Args>
    node_type* make_node(Args&&... args) {
        node_type* n = alloc_.allocate(1);
        alloc_.construct(n, static_cast<Args&&>(args)...);
        n->left  = nil_;
        n->right = nil_;
        n->parent= nil_;
        n->color = rb_color::RED;
        return n;
    }

    /** 析构并释放节点 */
    void free_node(node_type* n) {
        alloc_.destroy(n);
        alloc_.deallocate(n, 1);
    }

    /** 递归释放子树（后序） */
    void free_subtree(node_type* x) {
        if (x == nil_) return;
        free_subtree(x->left);
        free_subtree(x->right);
        free_node(x);
    }

    /** 递归复制子树 */
    node_type* copy_subtree(node_type* x, node_type* src_nil) {
        if (x == src_nil) return nil_;
        node_type* n = make_node(x->kv);
        n->color = x->color;
        n->left  = copy_subtree(x->left,  src_nil);
        n->right = copy_subtree(x->right, src_nil);
        if (n->left  != nil_) n->left->parent  = n;
        if (n->right != nil_) n->right->parent = n;
        return n;
    }

    // -------------------------------------------------------------------------
    // 旋转
    // -------------------------------------------------------------------------

    /**
     * @brief 左旋：将 x 的右子节点 y 提升
     *
     *    x             y
     *   / \           / \
     *  A   y   =>   x   C
     *     / \      / \
     *    B   C    A   B
     */
    void left_rotate(node_type* x) {
        node_type* y = x->right;
        x->right = y->left;
        if (y->left != nil_) y->left->parent = x;

        y->parent = x->parent;
        if (x->parent == nil_) {
            root_ = y;
        } else if (x == x->parent->left) {
            x->parent->left = y;
        } else {
            x->parent->right = y;
        }

        y->left = x;
        x->parent = y;
    }

    /**
     * @brief 右旋：将 y 的左子节点 x 提升
     *
     *      y           x
     *     / \         / \
     *    x   C  =>  A   y
     *   / \            / \
     *  A   B          B   C
     */
    void right_rotate(node_type* y) {
        node_type* x = y->left;
        y->left = x->right;
        if (x->right != nil_) x->right->parent = y;

        x->parent = y->parent;
        if (y->parent == nil_) {
            root_ = x;
        } else if (y == y->parent->left) {
            y->parent->left = x;
        } else {
            y->parent->right = x;
        }

        x->right = y;
        y->parent = x;
    }

    // -------------------------------------------------------------------------
    // 插入修复（维持红黑性质）
    // -------------------------------------------------------------------------

    /**
     * @brief 插入新节点 z 后，修复可能违反的红黑树性质
     *
     * 违反情况：z 和 z 的父节点都是红色（性质4）
     * 分三种情况处理，通过重新着色和旋转修复。
     */
    void insert_fixup(node_type* z) {
        while (z->parent->color == rb_color::RED) {
            if (z->parent == z->parent->parent->left) {
                // z 的父节点是祖父的左孩子
                node_type* uncle = z->parent->parent->right;
                if (uncle->color == rb_color::RED) {
                    // Case 1: 叔叔节点是红色 → 重新着色，问题上移
                    z->parent->color          = rb_color::BLACK;
                    uncle->color              = rb_color::BLACK;
                    z->parent->parent->color  = rb_color::RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->right) {
                        // Case 2: 叔叔是黑色，z 是右孩子 → 左旋变为 Case 3
                        z = z->parent;
                        left_rotate(z);
                    }
                    // Case 3: 叔叔是黑色，z 是左孩子 → 重新着色 + 右旋
                    z->parent->color         = rb_color::BLACK;
                    z->parent->parent->color = rb_color::RED;
                    right_rotate(z->parent->parent);
                }
            } else {
                // z 的父节点是祖父的右孩子（镜像情况）
                node_type* uncle = z->parent->parent->left;
                if (uncle->color == rb_color::RED) {
                    z->parent->color          = rb_color::BLACK;
                    uncle->color              = rb_color::BLACK;
                    z->parent->parent->color  = rb_color::RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->left) {
                        z = z->parent;
                        right_rotate(z);
                    }
                    z->parent->color         = rb_color::BLACK;
                    z->parent->parent->color = rb_color::RED;
                    left_rotate(z->parent->parent);
                }
            }
        }
        root_->color = rb_color::BLACK; // 确保根节点是黑色
    }

    // -------------------------------------------------------------------------
    // 删除辅助
    // -------------------------------------------------------------------------

    /**
     * @brief 将 v 替换 u 在树中的位置（仅修改父指针，不改子树）
     */
    void transplant(node_type* u, node_type* v) {
        if (u->parent == nil_) {
            root_ = v;
        } else if (u == u->parent->left) {
            u->parent->left = v;
        } else {
            u->parent->right = v;
        }
        v->parent = u->parent;
    }

    /**
     * @brief 找到以 x 为根的子树中的最小节点
     */
    node_type* tree_minimum(node_type* x) const noexcept {
        while (x->left != nil_) x = x->left;
        return x;
    }

    /**
     * @brief 删除节点 z 后，修复可能违反的红黑树性质
     *
     * 当删除的节点（或其后继）是黑色时，需要修复黑高不平衡。
     */
    void erase_fixup(node_type* x) {
        while (x != root_ && x->color == rb_color::BLACK) {
            if (x == x->parent->left) {
                node_type* w = x->parent->right; // 兄弟节点
                if (w->color == rb_color::RED) {
                    // Case 1: 兄弟是红色 → 变换为兄弟是黑色的情况
                    w->color          = rb_color::BLACK;
                    x->parent->color  = rb_color::RED;
                    left_rotate(x->parent);
                    w = x->parent->right;
                }
                if (w->left->color  == rb_color::BLACK &&
                    w->right->color == rb_color::BLACK) {
                    // Case 2: 兄弟的两个子节点都是黑色 → 兄弟变红，问题上移
                    w->color = rb_color::RED;
                    x = x->parent;
                } else {
                    if (w->right->color == rb_color::BLACK) {
                        // Case 3: 兄弟右孩子是黑色 → 右旋变为 Case 4
                        w->left->color  = rb_color::BLACK;
                        w->color        = rb_color::RED;
                        right_rotate(w);
                        w = x->parent->right;
                    }
                    // Case 4: 兄弟右孩子是红色 → 重新着色 + 左旋
                    w->color          = x->parent->color;
                    x->parent->color  = rb_color::BLACK;
                    w->right->color   = rb_color::BLACK;
                    left_rotate(x->parent);
                    x = root_; // 修复完成
                }
            } else {
                // 镜像情况
                node_type* w = x->parent->left;
                if (w->color == rb_color::RED) {
                    w->color          = rb_color::BLACK;
                    x->parent->color  = rb_color::RED;
                    right_rotate(x->parent);
                    w = x->parent->left;
                }
                if (w->right->color == rb_color::BLACK &&
                    w->left->color  == rb_color::BLACK) {
                    w->color = rb_color::RED;
                    x = x->parent;
                } else {
                    if (w->left->color == rb_color::BLACK) {
                        w->right->color = rb_color::BLACK;
                        w->color        = rb_color::RED;
                        left_rotate(w);
                        w = x->parent->left;
                    }
                    w->color          = x->parent->color;
                    x->parent->color  = rb_color::BLACK;
                    w->left->color    = rb_color::BLACK;
                    right_rotate(x->parent);
                    x = root_;
                }
            }
        }
        x->color = rb_color::BLACK;
    }

    // -------------------------------------------------------------------------
    // 查找内部实现
    // -------------------------------------------------------------------------

    node_type* find_node(const Key& key) const noexcept {
        node_type* cur = root_;
        while (cur != nil_) {
            if (comp_(key, cur->kv.first)) {
                cur = cur->left;
            } else if (comp_(cur->kv.first, key)) {
                cur = cur->right;
            } else {
                return cur; // 找到
            }
        }
        return nil_; // 未找到
    }

    // -------------------------------------------------------------------------
    // 初始化哨兵
    // -------------------------------------------------------------------------

    void init_nil() {
        nil_ = alloc_.allocate(1);
        // 不完全构造，只初始化必要字段（避免无效的 kv 构造）
        nil_->color  = rb_color::BLACK;
        nil_->parent = nil_;
        nil_->left   = nil_;
        nil_->right  = nil_;
    }

public:
    // -------------------------------------------------------------------------
    // 构造 / 析构
    // -------------------------------------------------------------------------

    /**
     * @brief 默认构造函数
     */
    map() : nil_(nullptr), root_(nullptr), size_(0), comp_(), alloc_() {
        init_nil();
        root_ = nil_;
        nil_->parent = nil_; // root 的 parent 指向 nil
    }

    /**
     * @brief 拷贝构造函数
     */
    map(const map& other) : nil_(nullptr), root_(nullptr), size_(0),
                             comp_(other.comp_), alloc_(other.alloc_) {
        init_nil();
        root_  = copy_subtree(other.root_, other.nil_);
        size_  = other.size_;
        if (root_ != nil_) {
            root_->parent = nil_;
        }
        nil_->parent = root_; // end() 的 -- 需要找到最大值
    }

    /**
     * @brief 移动构造函数
     */
    map(map&& other) noexcept
        : nil_(other.nil_), root_(other.root_), size_(other.size_),
          comp_(zen::move(other.comp_)), alloc_(zen::move(other.alloc_)) {
        // 给 other 一个新的合法空状态
        other.alloc_ = node_alloc{};
        other.init_nil();
        other.root_ = other.nil_;
        other.size_ = 0;
    }

    /**
     * @brief 析构函数
     */
    ~map() {
        free_subtree(root_);
        alloc_.deallocate(nil_, 1); // 释放哨兵（不调用析构，因为 kv 没有构造）
    }

    // -------------------------------------------------------------------------
    // 赋值运算符
    // -------------------------------------------------------------------------

    map& operator=(const map& other) {
        if (this != &other) {
            map tmp(other);
            zen::swap(nil_,   tmp.nil_);
            zen::swap(root_,  tmp.root_);
            zen::swap(size_,  tmp.size_);
            zen::swap(comp_,  tmp.comp_);
            zen::swap(alloc_, tmp.alloc_);
        }
        return *this;
    }

    map& operator=(map&& other) noexcept {
        if (this != &other) {
            free_subtree(root_);
            alloc_.deallocate(nil_, 1);

            nil_   = other.nil_;
            root_  = other.root_;
            size_  = other.size_;
            comp_  = zen::move(other.comp_);
            alloc_ = zen::move(other.alloc_);

            other.alloc_ = node_alloc{};
            other.init_nil();
            other.root_ = other.nil_;
            other.size_ = 0;
        }
        return *this;
    }

    // -------------------------------------------------------------------------
    // 容量
    // -------------------------------------------------------------------------

    bool      empty()    const noexcept { return size_ == 0; }
    size_type size()     const noexcept { return size_; }

    // -------------------------------------------------------------------------
    // 迭代器
    // -------------------------------------------------------------------------

    /**
     * @brief 返回指向最小元素的迭代器（中序遍历起点）
     */
    iterator begin() noexcept {
        if (root_ == nil_) return end();
        return iterator(tree_minimum(root_), nil_);
    }

    const_iterator begin() const noexcept {
        if (root_ == nil_) return end();
        return const_iterator(tree_minimum(root_), nil_);
    }

    const_iterator cbegin() const noexcept { return begin(); }

    /**
     * @brief 返回 end 迭代器（指向哨兵 nil_）
     */
    iterator       end()        noexcept { return iterator(nil_, nil_); }
    const_iterator end()  const noexcept { return const_iterator(nil_, nil_); }
    const_iterator cend() const noexcept { return end(); }

    // -------------------------------------------------------------------------
    // 查找
    // -------------------------------------------------------------------------

    /**
     * @brief 查找键，返回对应迭代器，若不存在返回 end()
     */
    iterator find(const Key& key) noexcept {
        node_type* n = find_node(key);
        return iterator(n, nil_);
    }

    const_iterator find(const Key& key) const noexcept {
        node_type* n = find_node(key);
        return const_iterator(n, nil_);
    }

    /**
     * @brief 判断键是否存在
     */
    bool contains(const Key& key) const noexcept {
        return find_node(key) != nil_;
    }

    size_type count(const Key& key) const noexcept {
        return contains(key) ? 1u : 0u;
    }

    // -------------------------------------------------------------------------
    // 下标访问
    // -------------------------------------------------------------------------

    /**
     * @brief operator[]：若键不存在则默认插入
     */
    Value& operator[](const Key& key) {
        node_type* n = find_node(key);
        if (n == nil_) {
            auto res = insert(value_type(key, Value{}));
            return res.first->second;
        }
        return n->kv.second;
    }

    Value& operator[](Key&& key) {
        node_type* n = find_node(key);
        if (n == nil_) {
            auto res = insert(value_type(zen::move(key), Value{}));
            return res.first->second;
        }
        return n->kv.second;
    }

    /**
     * @brief at()：若键不存在，行为未定义（与 std::map 不同，不抛异常）
     */
    Value& at(const Key& key) noexcept {
        return find_node(key)->kv.second;
    }

    const Value& at(const Key& key) const noexcept {
        return find_node(key)->kv.second;
    }

    // -------------------------------------------------------------------------
    // 插入
    // -------------------------------------------------------------------------

    /**
     * @brief 插入键值对
     * @return {迭代器, 是否插入成功}（若已存在则返回已有元素的迭代器 + false）
     */
    pair<iterator, bool> insert(const value_type& kv) {
        return insert_impl(kv.first, kv.second);
    }

    pair<iterator, bool> insert(value_type&& kv) {
        return insert_impl(zen::move(const_cast<Key&>(kv.first)),
                           zen::move(kv.second));
    }

    /**
     * @brief 就地构造并插入（emplace）
     */
    template<typename... Args>
    pair<iterator, bool> emplace(Args&&... args) {
        // 构造临时节点来获取 key
        node_type* z = make_node(static_cast<Args&&>(args)...);
        return insert_node(z);
    }

private:
    template<typename K, typename V>
    pair<iterator, bool> insert_impl(K&& key, V&& val) {
        node_type* z = make_node(zen::forward<K>(key), zen::forward<V>(val));
        return insert_node(z);
    }

    /**
     * @brief 将已构造好的节点 z 插入红黑树
     */
    pair<iterator, bool> insert_node(node_type* z) {
        node_type* y = nil_;
        node_type* x = root_;

        // 标准 BST 插入：找到插入位置
        while (x != nil_) {
            y = x;
            if (comp_(z->kv.first, x->kv.first)) {
                x = x->left;
            } else if (comp_(x->kv.first, z->kv.first)) {
                x = x->right;
            } else {
                // 键已存在：释放新节点，返回已有迭代器
                free_node(z);
                return {iterator(x, nil_), false};
            }
        }

        z->parent = y;
        if (y == nil_) {
            root_ = z; // 树为空
        } else if (comp_(z->kv.first, y->kv.first)) {
            y->left = z;
        } else {
            y->right = z;
        }

        ++size_;
        insert_fixup(z);
        nil_->parent = root_; // 更新 end() 的 -- 支持
        return {iterator(z, nil_), true};
    }

public:
    // -------------------------------------------------------------------------
    // 删除
    // -------------------------------------------------------------------------

    /**
     * @brief 删除指定迭代器指向的元素
     * @param pos 指向要删除元素的迭代器（必须有效且非 end()）
     */
    iterator erase(iterator pos) {
        iterator next = pos;
        ++next;
        erase_node(pos.node_);
        return next;
    }

    /**
     * @brief 按键删除，返回删除的元素个数（0 或 1）
     */
    size_type erase(const Key& key) {
        node_type* n = find_node(key);
        if (n == nil_) return 0;
        erase_node(n);
        return 1;
    }

    /**
     * @brief 清除所有元素
     */
    void clear() noexcept {
        free_subtree(root_);
        root_ = nil_;
        nil_->parent = nil_;
        size_ = 0;
    }

private:
    /**
     * @brief 从树中删除节点 z（标准红黑树删除算法）
     */
    void erase_node(node_type* z) {
        node_type* y = z;   // y 是实际被删除（或移走）的节点
        node_type* x;       // x 是替代 y 位置的节点
        rb_color   y_original_color = y->color;

        if (z->left == nil_) {
            // Case A: z 没有左孩子
            x = z->right;
            transplant(z, z->right);
        } else if (z->right == nil_) {
            // Case B: z 没有右孩子
            x = z->left;
            transplant(z, z->left);
        } else {
            // Case C: z 有两个孩子 → 用 z 的中序后继 y 替代
            y = tree_minimum(z->right);
            y_original_color = y->color;
            x = y->right;

            if (y->parent == z) {
                x->parent = y; // 避免 x == nil_ 时 parent 指向无效节点
            } else {
                transplant(y, y->right);
                y->right = z->right;
                y->right->parent = y;
            }
            transplant(z, y);
            y->left = z->left;
            y->left->parent = y;
            y->color = z->color;
        }

        free_node(z);
        --size_;

        if (y_original_color == rb_color::BLACK) {
            erase_fixup(x); // 黑色节点被删除，需要修复
        }

        nil_->parent = root_; // 保持 end()--.
    }

public:
    // -------------------------------------------------------------------------
    // 范围查找
    // -------------------------------------------------------------------------

    /**
     * @brief 返回第一个 key >= k 的迭代器（lower_bound）
     */
    iterator lower_bound(const Key& k) noexcept {
        node_type* result = nil_;
        node_type* cur    = root_;
        while (cur != nil_) {
            if (!comp_(cur->kv.first, k)) {
                result = cur;
                cur = cur->left;
            } else {
                cur = cur->right;
            }
        }
        return iterator(result, nil_);
    }

    const_iterator lower_bound(const Key& k) const noexcept {
        const node_type* result = nil_;
        const node_type* cur    = root_;
        while (cur != nil_) {
            if (!comp_(cur->kv.first, k)) {
                result = cur;
                cur = cur->left;
            } else {
                cur = cur->right;
            }
        }
        return const_iterator(result, nil_);
    }

    /**
     * @brief 返回第一个 key > k 的迭代器（upper_bound）
     */
    iterator upper_bound(const Key& k) noexcept {
        node_type* result = nil_;
        node_type* cur    = root_;
        while (cur != nil_) {
            if (comp_(k, cur->kv.first)) {
                result = cur;
                cur = cur->left;
            } else {
                cur = cur->right;
            }
        }
        return iterator(result, nil_);
    }

    /**
     * @brief 比较器访问
     */
    key_compare key_comp() const noexcept { return comp_; }
};

// ============================================================================
// set（基于红黑树，仅存储 key）
// ============================================================================

/**
 * @brief 有序集合，基于红黑树实现（map 的键值相同特化）
 *
 * 用 map<Key, unit_t> 实现，其中 unit_t 是空结构体。
 */
struct unit_t {};

template<typename Key, typename Compare = less<Key>>
class set {
public:
    using key_type       = Key;
    using value_type     = Key;
    using size_type      = size_t;
    using iterator       = rb_iterator<Key, unit_t>;
    using const_iterator = rb_const_iterator<Key, unit_t>;

private:
    map<Key, unit_t, Compare> tree_;

public:
    set() = default;

    bool      empty() const noexcept { return tree_.empty(); }
    size_type size()  const noexcept { return tree_.size(); }

    iterator       begin()  noexcept { return tree_.begin(); }
    const_iterator begin()  const noexcept { return tree_.begin(); }
    iterator       end()    noexcept { return tree_.end(); }
    const_iterator end()    const noexcept { return tree_.end(); }

    pair<iterator, bool> insert(const Key& key) {
        return tree_.insert({key, unit_t{}});
    }

    pair<iterator, bool> insert(Key&& key) {
        return tree_.insert({zen::move(key), unit_t{}});
    }

    size_type erase(const Key& key) { return tree_.erase(key); }
    iterator  erase(iterator pos)   { return tree_.erase(pos); }
    void      clear() noexcept      { tree_.clear(); }

    bool      contains(const Key& k) const noexcept { return tree_.contains(k); }
    size_type count(const Key& k)    const noexcept { return tree_.count(k); }

    iterator       find(const Key& k)       noexcept { return tree_.find(k); }
    const_iterator find(const Key& k) const noexcept { return tree_.find(k); }

    iterator       lower_bound(const Key& k)       noexcept { return tree_.lower_bound(k); }
    const_iterator lower_bound(const Key& k) const noexcept { return tree_.lower_bound(k); }
    iterator       upper_bound(const Key& k)       noexcept { return tree_.upper_bound(k); }
};

} // namespace zen

#endif // ZEN_CONTAINERS_ASSOCIATIVE_MAP_H
