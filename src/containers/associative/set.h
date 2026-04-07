/**
 * @file set.h
 * @brief 有序集合容器
 * @details 基于 Red-Black Tree 实现，保证元素唯一且有序
 */

#ifndef ZEN_CONTAINERS_SET_H
#define ZEN_CONTAINERS_SET_H

#include <initializer_list>
#include <functional>
#include <utility>
#include "../memory/allocator.h"
#include "../iterators/iterator.h"

namespace zen {

/**
 * @brief 红黑树节点颜色
 */
enum class RBColor {
    RED,
    BLACK
};

/**
 * @brief 红黑树节点
 */
template <typename T>
struct RBNode {
    T value;
    RBNode* left;
    RBNode* right;
    RBNode* parent;
    RBColor color;

    explicit RBNode(const T& v)
        : value(v)
        , left(nullptr)
        , right(nullptr)
        , parent(nullptr)
        , color(RBColor::RED) {}

    explicit RBNode(T&& v)
        : value(std::move(v))
        , left(nullptr)
        , right(nullptr)
        , parent(nullptr)
        , color(RBColor::RED) {}
};

/**
 * @brief 红黑树迭代器
 */
template <typename T>
class SetIterator {
public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = T;
    using difference_type = ptrdiff_t;
    using pointer = const T*;
    using reference = const T&;

    SetIterator() : node_(nullptr) {}
    explicit SetIterator(RBNode<T>* node) : node_(node) {}

    reference operator*() const { return node_->value; }
    pointer operator->() const { return &node_->value; }

    SetIterator& operator++();
    SetIterator operator++(int) {
        SetIterator tmp = *this;
        ++(*this);
        return tmp;
    }

    SetIterator& operator--();
    SetIterator operator--(int) {
        SetIterator tmp = *this;
        --(*this);
        return tmp;
    }

    friend bool operator==(const SetIterator& a, const SetIterator& b) {
        return a.node_ == b.node_;
    }

    friend bool operator!=(const SetIterator& a, const SetIterator& b) {
        return a.node_ != b.node_;
    }

private:
    RBNode<T>* node_;

    template <typename, typename>
    friend class Set;

    RBNode<T>* get_minimum(RBNode<T>* node) {
        while (node && node->left) {
            node = node->left;
        }
        return node;
    }

    RBNode<T>* get_maximum(RBNode<T>* node) {
        while (node && node->right) {
            node = node->right;
        }
        return node;
    }
};

template <typename T>
SetIterator<T>& SetIterator<T>::operator++() {
    if (!node_) return *this;

    if (node_->right) {
        node_ = get_minimum(node_->right);
    } else {
        RBNode<T>* parent = node_->parent;
        while (parent && node_ == parent->right) {
            node_ = parent;
            parent = parent->parent;
        }
        node_ = parent;
    }
    return *this;
}

template <typename T>
SetIterator<T>& SetIterator<T>::operator--() {
    if (!node_) return *this;

    if (node_->left) {
        node_ = get_maximum(node_->left);
    } else {
        RBNode<T>* parent = node_->parent;
        while (parent && node_ == parent->left) {
            node_ = parent;
            parent = parent->parent;
        }
        node_ = parent;
    }
    return *this;
}

/**
 * @brief 红黑树基类
 */
template <typename T, typename Compare>
class RBTree {
public:
    using Node = RBNode<T>;
    using size_type = size_t;

    RBTree() : root_(nullptr), size_(0), sentinel_(&sentinel_node_) {}

    ~RBTree() {
        clear();
    }

    Node* find(const T& value) const {
        Node* node = root_;
        while (node != sentinel_) {
            if (compare_(value, node->value)) {
                node = node->left;
            } else if (compare_(node->value, value)) {
                node = node->right;
            } else {
                return node;
            }
        }
        return nullptr;
    }

    bool insert(const T& value) {
        return insert_node(new Node(value));
    }

    bool insert(T&& value) {
        return insert_node(new Node(std::move(value)));
    }

    bool erase(const T& value) {
        Node* node = find(value);
        if (!node) return false;

        delete_node(node);
        delete node;
        return true;
    }

    void clear() {
        clear_subtree(root_);
        root_ = nullptr;
        size_ = 0;
    }

    size_type size() const { return size_; }
    bool empty() const { return size_ == 0; }

    Node* minimum() const {
        return get_minimum_node(root_);
    }

    Node* maximum() const {
        return get_maximum_node(root_);
    }

protected:
    Node* root_;
    size_type size_;
    Compare compare_;

private:
    Node sentinel_node_;
    Node* sentinel_;

    void clear_subtree(Node* node) {
        if (!node || node == sentinel_) return;
        clear_subtree(node->left);
        clear_subtree(node->right);
        delete node;
    }

    bool insert_node(Node* node) {
        Node* parent = nullptr;
        Node* current = root_;

        while (current != sentinel_) {
            parent = current;
            if (compare_(node->value, current->value)) {
                current = current->left;
            } else if (compare_(current->value, node->value)) {
                current = current->right;
            } else {
                delete node;
                return false;  // 已存在
            }
        }

        node->parent = parent;
        node->left = sentinel_;
        node->right = sentinel_;
        node->color = RBColor::RED;

        if (!parent) {
            root_ = node;
        } else if (compare_(node->value, parent->value)) {
            parent->left = node;
        } else {
            parent->right = node;
        }

        insert_fixup(node);
        size_++;
        return true;
    }

    void insert_fixup(Node* node) {
        while (node->parent && node->parent->color == RBColor::RED) {
            Node* grandparent = node->parent->parent;
            if (node->parent == grandparent->left) {
                Node* uncle = grandparent->right;
                if (uncle && uncle->color == RBColor::RED) {
                    node->parent->color = RBColor::BLACK;
                    uncle->color = RBColor::BLACK;
                    grandparent->color = RBColor::RED;
                    node = grandparent;
                } else {
                    if (node == node->parent->right) {
                        node = node->parent;
                        left_rotate(node);
                    }
                    node->parent->color = RBColor::BLACK;
                    grandparent->color = RBColor::RED;
                    right_rotate(grandparent);
                }
            } else {
                Node* uncle = grandparent->left;
                if (uncle && uncle->color == RBColor::RED) {
                    node->parent->color = RBColor::BLACK;
                    uncle->color = RBColor::BLACK;
                    grandparent->color = RBColor::RED;
                    node = grandparent;
                } else {
                    if (node == node->parent->left) {
                        node = node->parent;
                        right_rotate(node);
                    }
                    node->parent->color = RBColor::BLACK;
                    grandparent->color = RBColor::RED;
                    left_rotate(grandparent);
                }
            }
        }
        root_->color = RBColor::BLACK;
    }

    void delete_node(Node* node) {
        Node* y = node;
        Node* x;
        RBColor y_original_color = y->color;

        if (node->left == sentinel_) {
            x = node->right;
            transplant(node, node->right);
        } else if (node->right == sentinel_) {
            x = node->left;
            transplant(node, node->left);
        } else {
            y = get_minimum_node(node->right);
            y_original_color = y->color;
            x = y->right;
            if (y->parent == node) {
                if (x) x->parent = y;
            } else {
                transplant(y, y->right);
                y->right = node->right;
                if (y->right) y->right->parent = y;
            }
            transplant(node, y);
            y->left = node->left;
            y->left->parent = y;
            y->color = node->color;
        }

        if (y_original_color == RBColor::BLACK) {
            delete_fixup(x);
        }
        size_--;
    }

    void delete_fixup(Node* node) {
        while (node != root_ && (!node || node->color == RBColor::BLACK)) {
            if (node == node->parent->left) {
                Node* sibling = node->parent->right;
                if (sibling && sibling->color == RBColor::RED) {
                    sibling->color = RBColor::BLACK;
                    node->parent->color = RBColor::RED;
                    left_rotate(node->parent);
                    sibling = node->parent->right;
                }

                if ((!sibling->left || sibling->left->color == RBColor::BLACK) &&
                    (!sibling->right || sibling->right->color == RBColor::BLACK)) {
                    if (sibling) sibling->color = RBColor::RED;
                    node = node->parent;
                } else {
                    if (!sibling->right || sibling->right->color == RBColor::BLACK) {
                        if (sibling->left) sibling->left->color = RBColor::BLACK;
                        if (sibling) sibling->color = RBColor::RED;
                        right_rotate(sibling);
                        sibling = node->parent->right;
                    }
                    if (sibling) sibling->color = node->parent->color;
                    node->parent->color = RBColor::BLACK;
                    if (sibling->right) sibling->right->color = RBColor::BLACK;
                    left_rotate(node->parent);
                    node = root_;
                }
            } else {
                Node* sibling = node->parent->left;
                if (sibling && sibling->color == RBColor::RED) {
                    sibling->color = RBColor::BLACK;
                    node->parent->color = RBColor::RED;
                    right_rotate(node->parent);
                    sibling = node->parent->left;
                }

                if ((!sibling->right || sibling->right->color == RBColor::BLACK) &&
                    (!sibling->left || sibling->left->color == RBColor::BLACK)) {
                    if (sibling) sibling->color = RBColor::RED;
                    node = node->parent;
                } else {
                    if (!sibling->left || sibling->left->color == RBColor::BLACK) {
                        if (sibling->right) sibling->right->color = RBColor::BLACK;
                        if (sibling) sibling->color = RBColor::RED;
                        left_rotate(sibling);
                        sibling = node->parent->left;
                    }
                    if (sibling) sibling->color = node->parent->color;
                    node->parent->color = RBColor::BLACK;
                    if (sibling->left) sibling->left->color = RBColor::BLACK;
                    right_rotate(node->parent);
                    node = root_;
                }
            }
        }
        if (node) node->color = RBColor::BLACK;
    }

    void transplant(Node* u, Node* v) {
        if (!u->parent) {
            root_ = v;
        } else if (u == u->parent->left) {
            u->parent->left = v;
        } else {
            u->parent->right = v;
        }
        if (v) v->parent = u->parent;
    }

    void left_rotate(Node* x) {
        Node* y = x->right;
        x->right = y->left;
        if (y->left != sentinel_) {
            y->left->parent = x;
        }
        y->parent = x->parent;
        if (!x->parent) {
            root_ = y;
        } else if (x == x->parent->left) {
            x->parent->left = y;
        } else {
            x->parent->right = y;
        }
        y->left = x;
        x->parent = y;
    }

    void right_rotate(Node* y) {
        Node* x = y->left;
        y->left = x->right;
        if (x->right != sentinel_) {
            x->right->parent = y;
        }
        x->parent = y->parent;
        if (!y->parent) {
            root_ = x;
        } else if (y == y->parent->right) {
            y->parent->right = x;
        } else {
            y->parent->left = x;
        }
        x->right = y;
        y->parent = x;
    }

    Node* get_minimum_node(Node* node) const {
        while (node && node->left && node->left != sentinel_) {
            node = node->left;
        }
        return node;
    }

    Node* get_maximum_node(Node* node) const {
        while (node && node->right && node->right != sentinel_) {
            node = node->right;
        }
        return node;
    }
};

// ============================================================================
// 有序集合
// ============================================================================

/**
 * @brief 有序集合容器
 * @tparam T 元素类型
 * @tparam Compare 比较函数
 */
template <typename T, typename Compare = std::less<T>>
class Set {
public:
    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = SetIterator<T>;
    using const_iterator = SetIterator<T>;

    Set() = default;

    Set(std::initializer_list<T> init) {
        for (const auto& item : init) {
            insert(item);
        }
    }

    Set(const Set& other) = delete;
    Set& operator=(const Set& other) = delete;

    Set(Set&& other) noexcept = default;
    Set& operator=(Set&& other) noexcept = default;

    ~Set() = default;

    // ============================================================================
    // 迭代器
    // ============================================================================

    iterator begin() { return iterator(tree_.minimum()); }
    iterator end() { return iterator(nullptr); }

    const_iterator begin() const { return const_iterator(tree_.minimum()); }
    const_iterator end() const { return const_iterator(nullptr); }

    const_iterator cbegin() const { return const_iterator(tree_.minimum()); }
    const_iterator cend() const { return const_iterator(nullptr); }

    // ============================================================================
    // 容量
    // ============================================================================

    bool empty() const { return tree_.empty(); }
    size_type size() const { return tree_.size(); }

    // ============================================================================
    // 修改操作
    // ============================================================================

    /**
     * @brief 插入元素
     */
    std::pair<iterator, bool> insert(const T& value) {
        bool inserted = tree_.insert(value);
        auto it = find(value);
        return {it, inserted};
    }

    std::pair<iterator, bool> insert(T&& value) {
        bool inserted = tree_.insert(std::move(value));
        auto it = find(value);
        return {it, inserted};
    }

    /**
     * @brief 删除元素
     */
    size_type erase(const T& value) {
        return tree_.erase(value) ? 1 : 0;
    }

    iterator erase(const_iterator pos) {
        if (pos == end()) return end();
        T value = *pos;
        tree_.erase(value);
        return find(value);
    }

    /**
     * @brief 清空集合
     */
    void clear() { tree_.clear(); }

    /**
     * @brief 交换集合
     */
    void swap(Set& other) noexcept {
        std::swap(tree_, other.tree_);
    }

    /**
     * @brief 查找元素
     */
    iterator find(const T& value) {
        auto node = tree_.find(value);
        return node ? iterator(node) : end();
    }

    const_iterator find(const T& value) const {
        auto node = tree_.find(value);
        return node ? const_iterator(node) : end();
    }

    /**
     * @brief 检查元素是否存在
     */
    bool contains(const T& value) const {
        return tree_.find(value) != nullptr;
    }

    /**
     * @brief 计算元素数量
     */
    size_type count(const T& value) const {
        return contains(value) ? 1 : 0;
    }

    /**
     * @brief 获取下界迭代器
     */
    iterator lower_bound(const T& value) {
        auto it = begin();
        while (it != end() && compare_(*it, value)) {
            ++it;
        }
        return it;
    }

    const_iterator lower_bound(const T& value) const {
        auto it = cbegin();
        while (it != cend() && compare_(*it, value)) {
            ++it;
        }
        return it;
    }

    /**
     * @brief 获取上界迭代器
     */
    iterator upper_bound(const T& value) {
        auto it = begin();
        while (it != end() && !compare_(value, *it)) {
            ++it;
        }
        return it;
    }

    const_iterator upper_bound(const T& value) const {
        auto it = cbegin();
        while (it != cend() && !compare_(value, *it)) {
            ++it;
        }
        return it;
    }

private:
    RBTree<T, Compare> tree_;
    Compare compare_;
};

}  // namespace zen

#endif  // ZEN_CONTAINERS_SET_H
