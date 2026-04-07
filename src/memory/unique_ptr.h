#ifndef ZEN_MEMORY_UNIQUE_PTR_H
#define ZEN_MEMORY_UNIQUE_PTR_H

#include "../base/type_traits.h"
#include "../utility/swap.h"
namespace zen {

// ============================================================================
// Default Deleter - 默认删除器
// ============================================================================

/**
 * @brief 默认删除器（使用 delete）
 * @tparam T 被删除的对象类型
 *
 * 使用 operator delete 释放单个对象
 */
template<typename T>
struct default_delete {
    /**
     * @brief 删除指针
     * @param ptr 要删除的指针
     */
    void operator()(T* ptr) const noexcept {
        static_assert(!is_void<T>::value,
                      "cannot delete void pointer");
        static_assert(sizeof(T) > 0,
                      "cannot delete incomplete type");
        delete ptr;
    }
};

/**
 * @brief 默认删除器的数组特化（使用 delete[]）
 * @tparam T 被删除的数组元素类型
 *
 * 使用 operator delete[] 释放数组
 */
template<typename T>
struct default_delete<T[]> {
    /**
     * @brief 删除数组指针
     * @param ptr 要删除的数组指针
     */
    void operator()(T* ptr) const noexcept {
        static_assert(sizeof(T) > 0,
                      "cannot delete incomplete array type");
        delete[] ptr;
    }
};

// ============================================================================
// Unique_Ptr - 独占所有权智能指针
// ============================================================================

/**
 * @brief 独占所有权智能指针
 * @tparam T 管理的对象类型
 * @tparam Deleter 删除器类型
 *
 * unique_ptr 是一个独占所有权的智能指针，它保证在任何时候只有一个
 * unique_ptr 拥有某个对象的所有权。当 unique_ptr 被销毁时，它管理的
 * 对象也会被自动删除。
 *
 * 核心特性：
 * - 独占所有权：不能拷贝，只能移动
 * - 零开销：与裸指针大小相同
 * - 自动内存管理：超出作用域自动删除对象
 * - 支持自定义删除器
 *
 * 使用场景：
 * - 需要独占资源所有权的情况
 * - 工厂函数返回动态分配的对象
 * - PIMPL (Pointer to Implementation) 模式
 *
 * 示例：
 * @code
 * // 创建 unique_ptr
 * unique_ptr<int> ptr1(new int(42));
 *
 * // 使用 make_unique 创建（推荐）
 * auto ptr2 = make_unique<int>(100);
 *
 * // 移动语义
 * unique_ptr<int> ptr3 = std::move(ptr1);  // ptr1 现在为空
 *
 * // 访问对象
 * *ptr3 = 200;
 * ptr3->some_method();
 *
 * // 重置
 * ptr3.reset(new int(300));
 *
 * // 释放所有权
 * int* raw = ptr3.release();  // ptr3 不再管理对象
 * delete raw;
 * @endcode
 */
template<typename T, typename Deleter = default_delete<T>>
class unique_ptr {
private:
    T* ptr_;          // 管理的指针
    Deleter deleter_; // 删除器

    /**
     * @brief 允许不同类型的 unique_ptr 互相访问（类型转换使用）
     */
    template<typename U, typename E>
    friend class unique_ptr;

public:
    // ========================================================================
    // 类型定义
    // ========================================================================

    using pointer = T*;
    using element_type = T;
    using deleter_type = Deleter;

    // ========================================================================
    // 构造函数
    // ========================================================================

    /**
     * @brief 默认构造函数
     *
     * 创建一个空的 unique_ptr，不管理任何对象
     */
    constexpr unique_ptr() noexcept
        : ptr_(nullptr), deleter_(Deleter()) {
    }

    /**
     * @brief 从 nullptr 构造
     */
    constexpr unique_ptr(nullptr_t) noexcept
        : ptr_(nullptr), deleter_(Deleter()) {
    }

    /**
     * @brief 从指针构造
     * @param p 要管理的指针
     *
     * 注意：此构造函数不检查指针是否为空
     */
    explicit unique_ptr(pointer p) noexcept
        : ptr_(p), deleter_(Deleter()) {
    }

    /**
     * @brief 从指针和删除器构造
     * @param p 要管理的指针
     * @param d 删除器
     */
    unique_ptr(pointer p, const Deleter& d) noexcept
        : ptr_(p), deleter_(d) {
    }

    unique_ptr(pointer p, Deleter&& d) noexcept
        : ptr_(p), deleter_(zen::move(d)) {
    }

    /**
     * @brief 移动构造函数
     * @param other 要移动的 unique_ptr
     *
     * 移动后，other 变为空，不再管理任何对象
     */
    unique_ptr(unique_ptr&& other) noexcept
        : ptr_(other.ptr_), deleter_(zen::move(other.deleter_)) {
        other.ptr_ = nullptr;
    }

    /**
     * @brief 从兼容类型的 unique_ptr 移动构造
     */
    template<typename U, typename E>
    unique_ptr(unique_ptr<U, E>&& other) noexcept
        : ptr_(other.ptr_), deleter_(zen::move(other.deleter_)) {
        other.ptr_ = nullptr;
    }

    /**
     * @brief 析构函数
     *
     * 删除管理的对象（如果 ptr_ 不为空）
     */
    ~unique_ptr() {
        if (ptr_ != nullptr) {
            deleter_(ptr_);
        }
    }

    // ========================================================================
    // 赋值运算符
    // ========================================================================

    /**
     * @brief 移动赋值运算符
     * @param other 要移动的 unique_ptr
     *
     * 移动前，先删除当前管理的对象
     * 移动后，other 变为空
     */
    unique_ptr& operator=(unique_ptr&& other) noexcept {
        if (this != &other) {
            reset();
            ptr_ = other.ptr_;
            deleter_ = zen::move(other.deleter_);
            other.ptr_ = nullptr;
        }
        return *this;
    }

    /**
     * @brief 从兼容类型的 unique_ptr 移动赋值
     */
    template<typename U, typename E>
    unique_ptr& operator=(unique_ptr<U, E>&& other) noexcept {
        if (this != static_cast<void*>(&other)) {
            reset();
            ptr_ = other.ptr_;
            deleter_ = zen::move(other.deleter_);
            other.ptr_ = nullptr;
        }
        return *this;
    }

    /**
     * @brief 赋值为 nullptr
     */
    unique_ptr& operator=(nullptr_t) noexcept {
        reset();
        return *this;
    }

    /**
     * @brief 删除拷贝构造和拷贝赋值（独占所有权）
     */
    unique_ptr(const unique_ptr&) = delete;
    unique_ptr& operator=(const unique_ptr&) = delete;

    // ========================================================================
    // 观察者
    // ========================================================================

    /**
     * @brief 获取管理的指针
     * @return 管理的指针
     */
    pointer get() const noexcept {
        return ptr_;
    }

    /**
     * @brief 获取删除器
     * @return 删除器的引用
     */
    Deleter& get_deleter() noexcept {
        return deleter_;
    }

    /**
     * @brief 获取删除器（const 版本）
     * @return 删除器的 const 引用
     */
    const Deleter& get_deleter() const noexcept {
        return deleter_;
    }

    /**
     * @brief 检查是否管理着对象
     * @return 如果不为空返回 true
     */
    explicit operator bool() const noexcept {
        return ptr_ != nullptr;
    }

    // ========================================================================
    // 修改器
    // ========================================================================

    /**
     * @brief 释放所有权
     * @return 原来管理的指针
     *
     * 释放后，unique_ptr 不再管理该对象，
     * 调用者需要手动删除该指针
     */
    pointer release() noexcept {
        pointer p = ptr_;
        ptr_ = nullptr;
        return p;
    }

    /**
     * @brief 重置管理的对象
     * @param p 新管理的指针（默认为 nullptr）
     *
     * 先删除当前管理的对象，然后管理新的指针
     */
    void reset(pointer p = pointer()) noexcept {
        pointer old = ptr_;
        ptr_ = p;
        if (old != nullptr) {
            deleter_(old);
        }
    }

    /**
     * @brief 交换两个 unique_ptr
     * @param other 另一个 unique_ptr
     */
    void swap(unique_ptr& other) noexcept {
        using zen::swap;
        swap(ptr_, other.ptr_);
        swap(deleter_, other.deleter_);
    }

    // ========================================================================
    // 解引用
    // ========================================================================

    /**
     * @brief 解引用运算符
     * @return 管理对象的引用
     * @pre ptr_ != nullptr
     */
    typename add_lvalue_reference<T>::type
    operator*() const noexcept(noexcept(*ptr_)) {
        return *ptr_;
    }

    /**
     * @brief 箭头运算符
     * @return 管理对象的指针
     * @pre ptr_ != nullptr
     */
    pointer operator->() const noexcept {
        return ptr_;
    }

    /**
     * @brief 数组下标运算符（仅数组特化）
     * @param i 索引
     * @return 数组元素的引用
     * @pre ptr_ != nullptr
     */
    typename add_lvalue_reference<T>::type
    operator[](size_t i) const noexcept(noexcept(ptr_[i])) {
        return ptr_[i];
    }
};

// ============================================================================
// Unique_Ptr 数组特化
// ============================================================================

/**
 * @brief unique_ptr 的数组特化
 * @tparam T 数组元素类型
 * @tparam Deleter 删除器类型
 *
 * 专门用于管理动态分配的数组
 */
template<typename T, typename Deleter>
class unique_ptr<T[], Deleter> {
private:
    T* ptr_;          // 管理的数组指针
    Deleter deleter_; // 删除器

public:
    using pointer = T*;
    using element_type = T;
    using deleter_type = Deleter;

    // ========================================================================
    // 构造函数
    // ========================================================================

    constexpr unique_ptr() noexcept
        : ptr_(nullptr), deleter_(Deleter()) {
    }

    constexpr unique_ptr(nullptr_t) noexcept
        : ptr_(nullptr), deleter_(Deleter()) {
    }

    explicit unique_ptr(pointer p) noexcept
        : ptr_(p), deleter_(Deleter()) {
    }

    unique_ptr(pointer p, const Deleter& d) noexcept
        : ptr_(p), deleter_(d) {
    }

    unique_ptr(pointer p, Deleter&& d) noexcept
        : ptr_(p), deleter_(zen::move(d)) {
    }

    unique_ptr(unique_ptr&& other) noexcept
        : ptr_(other.ptr_), deleter_(zen::move(other.deleter_)) {
        other.ptr_ = nullptr;
    }

    ~unique_ptr() {
        if (ptr_ != nullptr) {
            deleter_(ptr_);
        }
    }

    // ========================================================================
    // 赋值运算符
    // ========================================================================

    unique_ptr& operator=(unique_ptr&& other) noexcept {
        if (this != &other) {
            reset();
            ptr_ = other.ptr_;
            deleter_ = zen::move(other.deleter_);
            other.ptr_ = nullptr;
        }
        return *this;
    }

    unique_ptr& operator=(nullptr_t) noexcept {
        reset();
        return *this;
    }

    // 禁止拷贝
    unique_ptr(const unique_ptr&) = delete;
    unique_ptr& operator=(const unique_ptr&) = delete;

    // ========================================================================
    // 观察者
    // ========================================================================

    pointer get() const noexcept {
        return ptr_;
    }

    Deleter& get_deleter() noexcept {
        return deleter_;
    }

    const Deleter& get_deleter() const noexcept {
        return deleter_;
    }

    explicit operator bool() const noexcept {
        return ptr_ != nullptr;
    }

    // ========================================================================
    // 修改器
    // ========================================================================

    pointer release() noexcept {
        pointer p = ptr_;
        ptr_ = nullptr;
        return p;
    }

    void reset(pointer p = pointer()) noexcept {
        pointer old = ptr_;
        ptr_ = p;
        if (old != nullptr) {
            deleter_(old);
        }
    }

    void swap(unique_ptr& other) noexcept {
        using zen::swap;
        swap(ptr_, other.ptr_);
        swap(deleter_, other.deleter_);
    }

    // ========================================================================
    // 数组访问
    // ========================================================================

    typename add_lvalue_reference<T>::type
    operator[](size_t i) const noexcept(noexcept(ptr_[i])) {
        return ptr_[i];
    }

    // 禁止使用 * 和 -> 运算符（数组不适用）
    typename add_lvalue_reference<T>::type
    operator*() const = delete;

    pointer operator->() const = delete;
};

// ============================================================================
// 工厂函数：make_unique
// ============================================================================

/**
 * @brief 创建 unique_ptr 的工厂函数
 * @tparam T 对象类型
 * @tparam Args 构造函数参数类型
 * @param args 构造函数参数
 * @return unique_ptr<T>
 *
 * make_unique 是创建 unique_ptr 的推荐方式，它具有以下优点：
 * - 异常安全：如果构造函数抛出异常，不会发生内存泄漏
 * - 类型安全：自动推导类型，避免重复
 * - 简洁代码：不需要显式 new 和 delete
 *
 * 示例：
 * @code
 * // 创建单个对象
 * auto ptr1 = make_unique<int>(42);
 * auto ptr2 = make_unique<std::string>("hello");
 * auto ptr3 = make_unique<MyClass>(arg1, arg2);
 *
 * // 创建数组（C++20）
 * auto arr = make_unique<int[]>(10);
 * @endcode
 */
template<typename T, typename... Args>
unique_ptr<T> make_unique(Args&&... args) {
    return unique_ptr<T>(new T(zen::forward<Args>(args)...));
}

/**
 * @brief 创建 unique_ptr 数组（仅声明，实际使用需要 C++20）
 * @tparam T 数组元素类型
 * @param size 数组大小
 * @return unique_ptr<T[]>
 */
template<typename T>
unique_ptr<T[]> make_unique(size_t size) {
    return unique_ptr<T[]>(new typename remove_extent<T>::type[size]());
}

// ============================================================================
// 比较运算符
// ============================================================================

/**
 * @brief 相等比较
 */
template<typename T1, typename D1, typename T2, typename D2>
bool operator==(const unique_ptr<T1, D1>& x, const unique_ptr<T2, D2>& y) {
    return x.get() == y.get();
}

/**
 * @brief 不等比较
 */
template<typename T1, typename D1, typename T2, typename D2>
bool operator!=(const unique_ptr<T1, D1>& x, const unique_ptr<T2, D2>& y) {
    return x.get() != y.get();
}

/**
 * @brief 小于比较（直接用 void* 比较，避免 common_type 依赖）
 */
template<typename T1, typename D1, typename T2, typename D2>
bool operator<(const unique_ptr<T1, D1>& x, const unique_ptr<T2, D2>& y) {
    return static_cast<const void*>(x.get()) < static_cast<const void*>(y.get());
}

/**
 * @brief 大于比较
 */
template<typename T1, typename D1, typename T2, typename D2>
bool operator>(const unique_ptr<T1, D1>& x, const unique_ptr<T2, D2>& y) {
    return y < x;
}

/**
 * @brief 小于等于比较
 */
template<typename T1, typename D1, typename T2, typename D2>
bool operator<=(const unique_ptr<T1, D1>& x, const unique_ptr<T2, D2>& y) {
    return !(y < x);
}

/**
 * @brief 大于等于比较
 */
template<typename T1, typename D1, typename T2, typename D2>
bool operator>=(const unique_ptr<T1, D1>& x, const unique_ptr<T2, D2>& y) {
    return !(x < y);
}

/**
 * @brief 与 nullptr 比较
 */
template<typename T, typename D>
bool operator==(const unique_ptr<T, D>& x, nullptr_t) noexcept {
    return x.get() == nullptr;
}

template<typename T, typename D>
bool operator==(nullptr_t, const unique_ptr<T, D>& x) noexcept {
    return x.get() == nullptr;
}

template<typename T, typename D>
bool operator!=(const unique_ptr<T, D>& x, nullptr_t) noexcept {
    return x.get() != nullptr;
}

template<typename T, typename D>
bool operator!=(nullptr_t, const unique_ptr<T, D>& x) noexcept {
    return x.get() != nullptr;
}

template<typename T, typename D>
bool operator<(const unique_ptr<T, D>& x, nullptr_t) noexcept {
    return x.get() < nullptr;
}

template<typename T, typename D>
bool operator<(nullptr_t, const unique_ptr<T, D>& x) noexcept {
    return nullptr < x.get();
}

template<typename T, typename D>
bool operator>(const unique_ptr<T, D>& x, nullptr_t) noexcept {
    return nullptr < x;
}

template<typename T, typename D>
bool operator>(nullptr_t, const unique_ptr<T, D>& x) noexcept {
    return x < nullptr;
}

template<typename T, typename D>
bool operator<=(const unique_ptr<T, D>& x, nullptr_t) noexcept {
    return !(nullptr < x);
}

template<typename T, typename D>
bool operator<=(nullptr_t, const unique_ptr<T, D>& x) noexcept {
    return !(x < nullptr);
}

template<typename T, typename D>
bool operator>=(const unique_ptr<T, D>& x, nullptr_t) noexcept {
    return !(x < nullptr);
}

template<typename T, typename D>
bool operator>=(nullptr_t, const unique_ptr<T, D>& x) noexcept {
    return !(nullptr < x);
}

// ============================================================================
// 类型别名
// ============================================================================

template<typename T, typename D = default_delete<T>>
using unique_ptr_t = unique_ptr<T, D>;

} // namespace zen

#endif // ZEN_MEMORY_UNIQUE_PTR_H