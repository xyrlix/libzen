#ifndef ZEN_MEMORY_ALLOCATOR_H
#define ZEN_MEMORY_ALLOCATOR_H

#include "../base/type_traits.h"
// placement new (::new(void*)) 需要 <new> 头文件
#include <new>

namespace zen {

// ============================================================================
// allocator - 自定义内存分配器
// ============================================================================

/**
 * @brief 标准布局的内存分配器
 * @tparam T 分配的对象类型
 *
 * 封装 ::operator new / ::operator delete，提供标准的分配器接口。
 * 与容器配合使用，可以替换成自定义策略（如内存池）。
 *
 * 核心职责：
 * - allocate:   分配 n 个 T 的原始内存（不构造对象）
 * - deallocate: 释放由 allocate 分配的内存
 * - construct:  在指定地址用 placement new 构造对象
 * - destroy:    手动调用对象析构函数
 */
template<typename T>
class allocator {
public:
    // ========================================================================
    // 标准类型定义
    // ========================================================================

    using value_type      = T;
    using pointer         = T*;
    using const_pointer   = const T*;
    using reference       = T&;
    using const_reference = const T&;
    using size_type       = decltype(sizeof(0));   // size_t
    using difference_type = decltype((T*)0 - (T*)0); // ptrdiff_t

    // 允许将 allocator<T> 重新绑定为 allocator<U>
    template<typename U>
    struct rebind {
        using other = allocator<U>;
    };

    // ========================================================================
    // 构造与析构
    // ========================================================================

    allocator() noexcept = default;
    allocator(const allocator&) noexcept = default;

    // 跨类型拷贝构造（rebind 使用）
    template<typename U>
    allocator(const allocator<U>&) noexcept {}

    ~allocator() noexcept = default;

    // ========================================================================
    // 内存分配
    // ========================================================================

    /**
     * @brief 分配 n 个 T 的原始内存
     * @param n 需要分配的对象个数
     * @return 指向分配内存的指针
     *
     * 使用 ::operator new 分配裸内存，不调用构造函数。
     * 抛出 std::bad_alloc（由 operator new 触发）。
     */
    pointer allocate(size_type n) {
        // ::operator new 分配字节，返回 void*，强制转换为 T*
        return static_cast<pointer>(::operator new(n * sizeof(T)));
    }

    /**
     * @brief 释放由 allocate 分配的内存
     * @param p 要释放的指针
     * @param n 对象个数（与分配时一致，但此处不使用）
     *
     * 仅释放内存，不调用析构函数。
     */
    void deallocate(pointer p, size_type /*n*/) noexcept {
        ::operator delete(p);
    }

    // ========================================================================
    // 对象构造与析构
    // ========================================================================

    /**
     * @brief 在已分配的内存上构造对象（placement new）
     * @tparam U 对象类型
     * @tparam Args 构造函数参数类型
     * @param p 目标地址
     * @param args 构造函数参数
     *
     * 使用 placement new 在指定地址构造 U 对象，不分配额外内存。
     */
    template<typename U, typename... Args>
    void construct(U* p, Args&&... args) {
        // placement new: 在 p 指向的位置构造对象
        ::new(static_cast<void*>(p)) U(static_cast<Args&&>(args)...);
    }

    /**
     * @brief 显式调用对象的析构函数
     * @tparam U 对象类型
     * @param p 要析构的对象指针
     *
     * 只调用析构函数，不释放内存。
     */
    template<typename U>
    void destroy(U* p) noexcept {
        p->~U();
    }

    // ========================================================================
    // 辅助查询
    // ========================================================================

    /**
     * @brief 返回理论上可分配的最大对象个数
     */
    size_type max_size() const noexcept {
        // 整个地址空间除以单个对象大小
        return static_cast<size_type>(-1) / sizeof(T);
    }

    /**
     * @brief 获取对象的实际地址（忽略重载的 operator&）
     */
    pointer address(reference x) const noexcept {
        return reinterpret_cast<pointer>(
            &reinterpret_cast<char&>(x)
        );
    }

    const_pointer address(const_reference x) const noexcept {
        return reinterpret_cast<const_pointer>(
            &reinterpret_cast<const char&>(x)
        );
    }
};

// ============================================================================
// allocator<void> 特化 - void 不能被解引用，只用作 rebind
// ============================================================================

template<>
class allocator<void> {
public:
    using value_type = void;
    using pointer    = void*;

    template<typename U>
    struct rebind {
        using other = allocator<U>;
    };
};

// ============================================================================
// 比较运算符
// 同类型的 allocator 总是相等（无状态）
// ============================================================================

template<typename T, typename U>
inline bool operator==(const allocator<T>&, const allocator<U>&) noexcept {
    return true;
}

template<typename T, typename U>
inline bool operator!=(const allocator<T>&, const allocator<U>&) noexcept {
    return false;
}

// ============================================================================
// allocator_traits - 分配器的统一萃取接口
// ============================================================================

/**
 * @brief 分配器 traits，提供统一的分配器操作接口
 * @tparam Alloc 分配器类型
 *
 * 通过 traits 访问分配器，使容器代码不依赖具体的分配器实现。
 */
template<typename Alloc>
struct allocator_traits {
    using allocator_type  = Alloc;
    using value_type      = typename Alloc::value_type;
    using pointer         = typename Alloc::pointer;
    using const_pointer   = typename Alloc::const_pointer;
    using size_type       = typename Alloc::size_type;
    using difference_type = typename Alloc::difference_type;

    /**
     * @brief 分配内存
     */
    static pointer allocate(Alloc& a, size_type n) {
        return a.allocate(n);
    }

    /**
     * @brief 释放内存
     */
    static void deallocate(Alloc& a, pointer p, size_type n) {
        a.deallocate(p, n);
    }

    /**
     * @brief 构造对象
     */
    template<typename T, typename... Args>
    static void construct(Alloc& a, T* p, Args&&... args) {
        a.construct(p, static_cast<Args&&>(args)...);
    }

    /**
     * @brief 析构对象
     */
    template<typename T>
    static void destroy(Alloc& a, T* p) noexcept {
        a.destroy(p);
    }

    /**
     * @brief 最大分配数量
     */
    static size_type max_size(const Alloc& a) noexcept {
        return a.max_size();
    }
};

} // namespace zen

#endif // ZEN_MEMORY_ALLOCATOR_H
