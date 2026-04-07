#ifndef ZEN_MEMORY_WEAK_PTR_H
#define ZEN_MEMORY_WEAK_PTR_H

#include "shared_ptr.h"

namespace zen {

// ============================================================================
// weak_ptr - 弱引用智能指针
// ============================================================================

/**
 * @brief 弱引用智能指针（不影响对象生命周期）
 * @tparam T 托管对象的类型
 *
 * weak_ptr 持有对 shared_ptr 托管对象的弱引用。
 * 它不增加强引用计数，因此不会阻止对象被销毁。
 *
 * 主要用途：
 * 1. **打破循环引用**：两个对象通过 shared_ptr 互相持有时，
 *    将其中一个改为 weak_ptr 可避免内存泄漏。
 * 2. **缓存/观察者模式**：持有对象引用而不影响其生命周期。
 *
 * 使用方式：
 * - 通过 lock() 尝试升级为 shared_ptr（若对象已销毁返回空 shared_ptr）
 * - expired() 检查对象是否已销毁
 *
 * 示例（打破循环引用）：
 * @code
 * struct Node {
 *     zen::shared_ptr<Node> next;    // 强引用 → 持有下一个节点
 *     zen::weak_ptr<Node>   prev;    // 弱引用 → 不持有上一个节点
 * };
 *
 * auto a = zen::make_shared<Node>();
 * auto b = zen::make_shared<Node>();
 * a->next = b;
 * b->prev = a;  // 使用 weak_ptr，不会循环引用
 * @endcode
 */
template<typename T>
class weak_ptr {
private:
    T*             ptr_;  // 指向托管对象（不保证有效）
    control_block* ctrl_; // 指向控制块

public:
    // ========================================================================
    // 构造函数
    // ========================================================================

    /**
     * @brief 默认构造：空弱指针
     */
    constexpr weak_ptr() noexcept
        : ptr_(nullptr), ctrl_(nullptr) {
    }

    /**
     * @brief 从 shared_ptr 构造弱引用
     * @param sp 要观察的 shared_ptr
     *
     * 增加控制块的弱引用计数，不影响强引用计数。
     */
    weak_ptr(const shared_ptr<T>& sp) noexcept
        : ptr_(sp.ptr_), ctrl_(sp.ctrl_) {
        if (ctrl_ != nullptr) {
            ctrl_->add_weak();
        }
    }

    /**
     * @brief 拷贝构造
     */
    weak_ptr(const weak_ptr& other) noexcept
        : ptr_(other.ptr_), ctrl_(other.ctrl_) {
        if (ctrl_ != nullptr) {
            ctrl_->add_weak();
        }
    }

    /**
     * @brief 移动构造（转移弱引用，不改变计数）
     */
    weak_ptr(weak_ptr&& other) noexcept
        : ptr_(other.ptr_), ctrl_(other.ctrl_) {
        other.ptr_  = nullptr;
        other.ctrl_ = nullptr;
    }

    /**
     * @brief 析构：减少弱引用计数
     *
     * 若弱引用归零，控制块内存被释放（对象可能已提前销毁）
     */
    ~weak_ptr() noexcept {
        if (ctrl_ != nullptr) {
            ctrl_->release_weak();
        }
    }

    // ========================================================================
    // 赋值运算符
    // ========================================================================

    /**
     * @brief 从 shared_ptr 赋值
     */
    weak_ptr& operator=(const shared_ptr<T>& sp) noexcept {
        weak_ptr tmp(sp);
        swap(tmp);
        return *this;
    }

    /**
     * @brief 拷贝赋值
     */
    weak_ptr& operator=(const weak_ptr& other) noexcept {
        weak_ptr tmp(other);
        swap(tmp);
        return *this;
    }

    /**
     * @brief 移动赋值
     */
    weak_ptr& operator=(weak_ptr&& other) noexcept {
        weak_ptr tmp(static_cast<weak_ptr&&>(other));
        swap(tmp);
        return *this;
    }

    // ========================================================================
    // 修改器
    // ========================================================================

    /**
     * @brief 重置为空弱引用
     */
    void reset() noexcept {
        weak_ptr tmp;
        swap(tmp);
    }

    /**
     * @brief 交换两个 weak_ptr
     */
    void swap(weak_ptr& other) noexcept {
        zen::swap(ptr_,  other.ptr_);
        zen::swap(ctrl_, other.ctrl_);
    }

    // ========================================================================
    // 观察者
    // ========================================================================

    /**
     * @brief 返回当前强引用计数（即观察的对象被多少 shared_ptr 持有）
     */
    long use_count() const noexcept {
        return ctrl_ != nullptr ? ctrl_->shared_count : 0L;
    }

    /**
     * @brief 检查托管对象是否已销毁
     * @return 若对象已销毁（强引用计数为 0）或 weak_ptr 为空，返回 true
     */
    bool expired() const noexcept {
        return use_count() == 0L;
    }

    /**
     * @brief 尝试升级为 shared_ptr
     * @return 若对象仍然存活，返回共享该对象的 shared_ptr；
     *         若对象已销毁，返回空的 shared_ptr
     *
     * lock() 是 weak_ptr 的核心操作。使用前必须通过 lock() 检查对象是否有效：
     * @code
     * zen::weak_ptr<Foo> wp = get_weak();
     * if (auto sp = wp.lock()) {
     *     sp->use();   // 对象仍然有效
     * }
     * // 若 lock() 返回空，说明对象已被销毁
     * @endcode
     */
    shared_ptr<T> lock() const noexcept {
        if (ctrl_ == nullptr || !ctrl_->try_add_ref()) {
            return shared_ptr<T>(); // 返回空 shared_ptr
        }
        // 成功增加强引用，构造 shared_ptr
        // 使用友元访问私有构造函数
        return shared_ptr<T>(ptr_, ctrl_);
    }
};

// ============================================================================
// 非成员 swap
// ============================================================================

template<typename T>
void swap(weak_ptr<T>& a, weak_ptr<T>& b) noexcept {
    a.swap(b);
}

} // namespace zen

#endif // ZEN_MEMORY_WEAK_PTR_H
