#ifndef ZEN_MEMORY_SHARED_PTR_H
#define ZEN_MEMORY_SHARED_PTR_H

#include "../base/type_traits.h"
#include "../utility/swap.h"
// placement new 需要 <new>
#include <new>

namespace zen {

// ============================================================================
// 简单默认删除器（避免循环包含 unique_ptr.h）
// ============================================================================

/**
 * @brief shared_ptr 使用的默认删除器（delete 单个对象）
 */
template<typename T>
struct shared_default_delete {
    void operator()(T* p) const noexcept {
        delete p;
    }
};

// forward declaration
template<typename T>
class weak_ptr;

template<typename T>
class shared_ptr;

// ============================================================================
// control_block - 引用计数控制块
// ============================================================================

/**
 * @brief 共享引用计数控制块（基类）
 *
 * 控制块与托管对象分离，生命周期独立。
 * 当 shared_count 降为 0 时销毁对象，但控制块本身保留到
 * weak_count 也降为 0，才会释放控制块内存。
 *
 * 布局示意：
 *
 *   shared_ptr ──────────────────────────────────────────────────┐
 *                   ┌─────────────────────────────────────────┐  │
 *   weak_ptr ──────>│ control_block:                          │  │
 *                   │   shared_count  (强引用，管理对象生命)  │  │
 *                   │   weak_count    (弱引用，管理块生命)    │  │
 *                   └─────────────────────────────────────────┘  │
 *   T* ptr ─────────────────────────────────────────────────────>│  托管对象
 */
struct control_block {
    long shared_count; // 强引用计数（shared_ptr 数量）
    long weak_count;   // 弱引用计数（weak_ptr 数量，初始为 1，代表强引用组）

    control_block() noexcept
        : shared_count(1), weak_count(1) {
    }

    virtual ~control_block() noexcept = default;

    /**
     * @brief 销毁托管的对象（shared_count 降为 0 时调用）
     *
     * 子类重写此方法，以支持自定义删除器和分配器。
     */
    virtual void destroy_object() noexcept = 0;

    /**
     * @brief 释放控制块自身的内存（weak_count 降为 0 时调用）
     */
    virtual void destroy_block() noexcept = 0;

    // ========================================================================
    // 引用计数操作
    // ========================================================================

    /**
     * @brief 增加强引用
     */
    void add_ref() noexcept {
        ++shared_count;
        // weak_count 不需要递增，因为强引用组已经持有 1 个弱引用
    }

    /**
     * @brief 减少强引用，返回新计数
     */
    void release_ref() noexcept {
        if (--shared_count == 0) {
            // 强引用归零，销毁对象
            destroy_object();
            // 释放之前强引用组持有的那 1 个弱引用
            release_weak();
        }
    }

    /**
     * @brief 增加弱引用
     */
    void add_weak() noexcept {
        ++weak_count;
    }

    /**
     * @brief 减少弱引用
     */
    void release_weak() noexcept {
        if (--weak_count == 0) {
            // 没有任何引用，释放控制块
            destroy_block();
        }
    }

    /**
     * @brief 尝试将弱引用升级为强引用（lock() 使用）
     * @return 升级成功返回 true
     */
    bool try_add_ref() noexcept {
        if (shared_count == 0) {
            return false; // 对象已销毁
        }
        ++shared_count;
        return true;
    }
};

// ============================================================================
// control_block_impl - 带删除器的具体控制块
// ============================================================================

/**
 * @brief 持有托管指针和自定义删除器的控制块实现
 * @tparam T 托管对象类型
 * @tparam Deleter 删除器类型
 */
template<typename T, typename Deleter>
struct control_block_impl : control_block {
    T*      ptr_;     // 托管的原始指针
    Deleter deleter_; // 删除器

    control_block_impl(T* p, Deleter d) noexcept
        : control_block(), ptr_(p), deleter_(static_cast<Deleter&&>(d)) {
    }

    void destroy_object() noexcept override {
        if (ptr_ != nullptr) {
            deleter_(ptr_);
            ptr_ = nullptr;
        }
    }

    void destroy_block() noexcept override {
        // 调用自身析构函数，然后释放内存
        this->~control_block_impl();
        ::operator delete(this);
    }
};

// ============================================================================
// shared_ptr - 共享所有权智能指针
// ============================================================================

/**
 * @brief 共享所有权智能指针
 * @tparam T 托管的对象类型
 *
 * shared_ptr 通过引用计数实现多个所有者共享同一个对象。
 * 当最后一个 shared_ptr 销毁时，托管对象才被删除。
 *
 * 核心特性：
 * - 共享所有权：可以拷贝，多个 shared_ptr 指向同一对象
 * - 引用计数：使用控制块追踪共有者数量
 * - 线程安全的引用计数（当前为非原子实现，适合单线程）
 * - 支持自定义删除器
 * - 配合 weak_ptr 解决循环引用
 *
 * 内存布局：
 *   shared_ptr = { ptr_, ctrl_ }
 *   ctrl_ → control_block { shared_count, weak_count, ... }
 *   ptr_  → 托管对象
 *
 * 示例：
 * @code
 * auto sp1 = make_shared<int>(42);
 * auto sp2 = sp1;                      // 引用计数变为 2
 * sp1.reset();                         // 引用计数变为 1
 * // sp2 析构时引用计数降为 0，对象被删除
 * @endcode
 */
template<typename T>
class shared_ptr {
private:
    T*             ptr_;  // 指向托管对象（可能与 ctrl_ 分离）
    control_block* ctrl_; // 指向控制块

    // 私有构造标记，用于 make_shared 等内部使用
    struct make_shared_tag {};

    // weak_ptr::lock() 和 make_shared 使用此私有构造
    shared_ptr(T* p, control_block* ctrl, make_shared_tag) noexcept
        : ptr_(p), ctrl_(ctrl) {
    }

    // 保留原始私有构造（weak_ptr::lock 使用）
    shared_ptr(T* p, control_block* ctrl) noexcept
        : ptr_(p), ctrl_(ctrl) {
    }

    friend class weak_ptr<T>;
    // 允许不同 T 的 shared_ptr 互相访问（类型转换使用）
    template<typename U> friend class shared_ptr;
    // make_shared 需要访问私有构造
    template<typename U, typename... A> friend shared_ptr<U> make_shared(A&&...);

public:
    // ========================================================================
    // 类型定义
    // ========================================================================

    using element_type = T;

    // ========================================================================
    // 构造函数
    // ========================================================================

    /**
     * @brief 默认构造：空指针
     */
    constexpr shared_ptr() noexcept
        : ptr_(nullptr), ctrl_(nullptr) {
    }

    /**
     * @brief 从 nullptr 构造
     */
    constexpr shared_ptr(decltype(nullptr)) noexcept
        : ptr_(nullptr), ctrl_(nullptr) {
    }

    /**
     * @brief 从原始指针构造（使用默认删除器）
     * @param p 要托管的指针
     *
     * 分配控制块，初始引用计数 = 1。
     */
    explicit shared_ptr(T* p)
        : ptr_(p), ctrl_(nullptr) {
        if (p != nullptr) {
            // 创建持有 shared_default_delete 的控制块
            auto* cb = static_cast<control_block_impl<T, shared_default_delete<T>>*>(
                ::operator new(sizeof(control_block_impl<T, shared_default_delete<T>>))
            );
            ::new(cb) control_block_impl<T, shared_default_delete<T>>(p, shared_default_delete<T>{});
            ctrl_ = cb;
        }
    }

    /**
     * @brief 从原始指针和自定义删除器构造
     */
    template<typename Deleter>
    shared_ptr(T* p, Deleter d)
        : ptr_(p), ctrl_(nullptr) {
        if (p != nullptr) {
            auto* cb = static_cast<control_block_impl<T, Deleter>*>(
                ::operator new(sizeof(control_block_impl<T, Deleter>))
            );
            ::new(cb) control_block_impl<T, Deleter>(p, static_cast<Deleter&&>(d));
            ctrl_ = cb;
        }
    }

    /**
     * @brief 拷贝构造：共享所有权（增加引用计数）
     */
    shared_ptr(const shared_ptr& other) noexcept
        : ptr_(other.ptr_), ctrl_(other.ctrl_) {
        if (ctrl_ != nullptr) {
            ctrl_->add_ref();
        }
    }

    /**
     * @brief 从兼容类型的 shared_ptr 拷贝构造（向上转型）
     */
    template<typename U>
    shared_ptr(const shared_ptr<U>& other) noexcept
        : ptr_(other.ptr_), ctrl_(other.ctrl_) {
        if (ctrl_ != nullptr) {
            ctrl_->add_ref();
        }
    }

    /**
     * @brief 移动构造：转移所有权（不改变引用计数）
     */
    shared_ptr(shared_ptr&& other) noexcept
        : ptr_(other.ptr_), ctrl_(other.ctrl_) {
        other.ptr_  = nullptr;
        other.ctrl_ = nullptr;
    }

    /**
     * @brief 从兼容类型的 shared_ptr 移动构造
     */
    template<typename U>
    shared_ptr(shared_ptr<U>&& other) noexcept
        : ptr_(other.ptr_), ctrl_(other.ctrl_) {
        other.ptr_  = nullptr;
        other.ctrl_ = nullptr;
    }

    /**
     * @brief 析构函数：减少引用计数
     *
     * 如果计数归零，销毁托管对象
     */
    ~shared_ptr() noexcept {
        if (ctrl_ != nullptr) {
            ctrl_->release_ref();
        }
    }

    // ========================================================================
    // 赋值运算符
    // ========================================================================

    /**
     * @brief 拷贝赋值
     */
    shared_ptr& operator=(const shared_ptr& other) noexcept {
        // copy-and-swap 保证异常安全和自赋值安全
        shared_ptr tmp(other);
        swap(tmp);
        return *this;
    }

    /**
     * @brief 移动赋值
     */
    shared_ptr& operator=(shared_ptr&& other) noexcept {
        shared_ptr tmp(static_cast<shared_ptr&&>(other));
        swap(tmp);
        return *this;
    }

    /**
     * @brief 赋值 nullptr（相当于 reset()）
     */
    shared_ptr& operator=(decltype(nullptr)) noexcept {
        reset();
        return *this;
    }

    // ========================================================================
    // 修改器
    // ========================================================================

    /**
     * @brief 重置为空
     *
     * 释放当前持有的对象，引用计数减一
     */
    void reset() noexcept {
        shared_ptr tmp;
        swap(tmp);
    }

    /**
     * @brief 重置为新指针
     * @param p 新托管的指针
     */
    void reset(T* p) {
        shared_ptr tmp(p);
        swap(tmp);
    }

    /**
     * @brief 交换两个 shared_ptr
     */
    void swap(shared_ptr& other) noexcept {
        zen::swap(ptr_,  other.ptr_);
        zen::swap(ctrl_, other.ctrl_);
    }

    // ========================================================================
    // 观察者
    // ========================================================================

    /**
     * @brief 获取原始指针
     */
    T* get() const noexcept {
        return ptr_;
    }

    /**
     * @brief 解引用
     * @pre get() != nullptr
     */
    T& operator*() const noexcept {
        return *ptr_;
    }

    /**
     * @brief 成员访问
     * @pre get() != nullptr
     */
    T* operator->() const noexcept {
        return ptr_;
    }

    /**
     * @brief 显式 bool 转换
     */
    explicit operator bool() const noexcept {
        return ptr_ != nullptr;
    }

    /**
     * @brief 返回当前强引用计数
     *
     * 注意：线程不安全的读取，仅用于调试或测试
     */
    long use_count() const noexcept {
        return ctrl_ != nullptr ? ctrl_->shared_count : 0L;
    }

    /**
     * @brief 检查是否是唯一的所有者
     */
    bool unique() const noexcept {
        return use_count() == 1L;
    }
};

// ============================================================================
// make_shared - 推荐的创建方式（对象和控制块一次性分配）
// ============================================================================

/**
 * @brief 控制块与对象合并的版本（减少一次内存分配）
 * @tparam T 对象类型
 */
template<typename T>
struct control_block_inplace : control_block {
    // 使用对齐存储，避免直接构造
    alignas(T) char storage_[sizeof(T)];

    template<typename... Args>
    explicit control_block_inplace(Args&&... args)
        : control_block() {
        ::new(storage_) T(static_cast<Args&&>(args)...);
    }

    T* get() noexcept {
        return reinterpret_cast<T*>(storage_);
    }

    void destroy_object() noexcept override {
        // 显式调用析构函数，不释放内存
        reinterpret_cast<T*>(storage_)->~T();
    }

    void destroy_block() noexcept override {
        this->~control_block_inplace();
        ::operator delete(this);
    }
};

/**
 * @brief 一次内存分配同时创建对象和控制块
 * @tparam T 对象类型
 * @tparam Args 构造函数参数
 * @return shared_ptr<T>
 *
 * 优势：
 * - 只有一次 ::operator new（比 shared_ptr(new T()) 少一次）
 * - 对象和控制块在相邻内存，缓存友好
 *
 * 示例：
 * @code
 * auto sp = zen::make_shared<int>(42);
 * auto sp2 = zen::make_shared<MyClass>(arg1, arg2);
 * @endcode
 */
template<typename T, typename... Args>
shared_ptr<T> make_shared(Args&&... args) {
    // 一次分配控制块（内嵌对象）
    auto* cb = static_cast<control_block_inplace<T>*>(
        ::operator new(sizeof(control_block_inplace<T>))
    );
    ::new(cb) control_block_inplace<T>(static_cast<Args&&>(args)...);

    // 使用带 tag 的私有构造，避免与 Deleter 构造函数的歧义
    return shared_ptr<T>(cb->get(), cb, typename shared_ptr<T>::make_shared_tag{});
}

// ============================================================================
// 比较运算符
// ============================================================================

template<typename T, typename U>
bool operator==(const shared_ptr<T>& a, const shared_ptr<U>& b) noexcept {
    return a.get() == b.get();
}

template<typename T, typename U>
bool operator!=(const shared_ptr<T>& a, const shared_ptr<U>& b) noexcept {
    return a.get() != b.get();
}

template<typename T>
bool operator==(const shared_ptr<T>& a, decltype(nullptr)) noexcept {
    return a.get() == nullptr;
}

template<typename T>
bool operator==(decltype(nullptr), const shared_ptr<T>& a) noexcept {
    return a.get() == nullptr;
}

template<typename T>
bool operator!=(const shared_ptr<T>& a, decltype(nullptr)) noexcept {
    return a.get() != nullptr;
}

template<typename T>
bool operator!=(decltype(nullptr), const shared_ptr<T>& a) noexcept {
    return a.get() != nullptr;
}

} // namespace zen

#endif // ZEN_MEMORY_SHARED_PTR_H
