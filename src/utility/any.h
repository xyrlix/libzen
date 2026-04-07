#ifndef ZEN_UTILITY_ANY_H
#define ZEN_UTILITY_ANY_H

#pragma once
#include <new>          // placement new
#include <cstring>      // memcpy
#include "../base/type_traits.h"
#include "swap.h"

namespace zen {

// ============================================================================
// bad_any_cast - any 类型转换失败时的异常
// ============================================================================
struct bad_any_cast {};

// ============================================================================
// any - 类型安全的值容器（类似 std::any，不依赖 std::any）
// ============================================================================
/**
 * @brief 可存储任意可拷贝类型的容器
 *
 * 内部使用小对象优化（SOO）：
 *   - 对象 <= 小缓冲区大小（24字节）且对齐合法时，直接存在栈上
 *   - 否则，堆分配
 *
 * 通过类型擦除的虚函数表（vtable）实现拷贝/移动/析构/类型信息。
 *
 * 示例：
 * @code
 * zen::any a = 42;
 * zen::any b = 3.14;
 * int  v1 = zen::any_cast<int>(a);    // 42
 * auto v2 = zen::any_cast<double>(b); // 3.14
 * a = std::string("hello");           // 重新赋值
 * @endcode
 */
class any {
private:
    // -------------------------------------------------------------------------
    // 小对象优化缓冲区
    // -------------------------------------------------------------------------
    static constexpr size_t SBO_SIZE  = 24;
    static constexpr size_t SBO_ALIGN = alignof(void*);

    alignas(SBO_ALIGN) unsigned char buf_[SBO_SIZE];

    // -------------------------------------------------------------------------
    // 虚函数表（类型擦除的操作集合）
    // -------------------------------------------------------------------------
    struct vtable_t {
        // 类型 id（用指针地址做类型标识）
        const void* type_id;
        // 析构当前值（sbo=true 时 ptr 指向 buf_，否则指向堆）
        void (*destroy)(void* ptr, bool sbo);
        // 拷贝构造到目标
        void (*copy)(const void* src, void* dst_buf, bool src_sbo, void*& dst_heap);
        // 移动构造到目标
        void (*move_to)(void* src, void* dst_buf, bool src_sbo, void*& dst_heap);
        // 返回值的 void* 指针
        void* (*get_ptr)(void* buf, bool sbo, void* heap);
        const void* (*get_cptr)(const void* buf, bool sbo, const void* heap);
    };

    const vtable_t* vt_;
    bool            sbo_;     // 是否使用小对象优化
    void*           heap_;    // 堆分配的指针（仅 sbo_=false 时有效）

    // -------------------------------------------------------------------------
    // 模板工厂：为每个类型生成专属 vtable
    // -------------------------------------------------------------------------
    template<typename T>
    static const vtable_t* get_vtable() {
        // 类型标识符：用静态局部变量地址（不依赖 typeid）
        static const char type_tag = 0;

        static const vtable_t vt = {
            &type_tag,

            // destroy
            [](void* ptr, bool sbo) {
                T* p = sbo ? reinterpret_cast<T*>(ptr)
                           : reinterpret_cast<T*>(ptr); // 均用 ptr
                p->~T();
                if (!sbo) {
                    // 堆指针由 any 自己 delete，这里只析构
                }
            },

            // copy
            [](const void* src, void* dst_buf, bool src_sbo, void*& dst_heap) {
                const T* sp = reinterpret_cast<const T*>(src);
                if (sizeof(T) <= SBO_SIZE &&
                    alignof(T) <= SBO_ALIGN) {
                    ::new(dst_buf) T(*sp);
                    dst_heap = nullptr;
                } else {
                    T* np = new T(*sp);
                    dst_heap = np;
                }
            },

            // move_to
            [](void* src, void* dst_buf, bool src_sbo, void*& dst_heap) {
                T* sp = reinterpret_cast<T*>(src);
                if (sizeof(T) <= SBO_SIZE &&
                    alignof(T) <= SBO_ALIGN) {
                    ::new(dst_buf) T(static_cast<T&&>(*sp));
                    dst_heap = nullptr;
                } else {
                    // 堆分配场景：直接转移指针（由调用方处理 src heap ptr）
                    T* np = new T(static_cast<T&&>(*sp));
                    dst_heap = np;
                }
            },

            // get_ptr
            [](void* buf, bool sbo, void* heap) -> void* {
                return sbo ? buf : heap;
            },

            // get_cptr
            [](const void* buf, bool sbo, const void* heap) -> const void* {
                return sbo ? buf : heap;
            }
        };
        return &vt;
    }

    // -------------------------------------------------------------------------
    // 内部辅助
    // -------------------------------------------------------------------------
    void reset() noexcept {
        if (vt_) {
            void* ptr = sbo_ ? static_cast<void*>(buf_) : heap_;
            vt_->destroy(ptr, sbo_);
            if (!sbo_ && heap_) {
                // 这里需要知道类型大小来 delete，用 operator delete
                ::operator delete(heap_);
            }
            vt_   = nullptr;
            heap_ = nullptr;
        }
    }

    template<typename T>
    static bool use_sbo() {
        return sizeof(T) <= SBO_SIZE && alignof(T) <= SBO_ALIGN;
    }

public:
    // =========================================================================
    // 构造函数
    // =========================================================================

    /** @brief 默认构造：空状态 */
    any() noexcept
        : vt_(nullptr), sbo_(false), heap_(nullptr) {
        // buf_ 不需要初始化
    }

    /** @brief 拷贝构造 */
    any(const any& other) : vt_(other.vt_), sbo_(false), heap_(nullptr) {
        if (other.vt_) {
            const void* src = other.vt_->get_cptr(other.buf_, other.sbo_, other.heap_);
            other.vt_->copy(src, buf_, other.sbo_, heap_);
            sbo_ = use_sbo_from_vt(other);
        }
    }

    /** @brief 移动构造 */
    any(any&& other) noexcept : vt_(other.vt_), sbo_(other.sbo_), heap_(other.heap_) {
        if (other.vt_ && other.sbo_) {
            // SBO：需要把 buf_ 内容搬过来，无法直接 memcpy 有析构的对象
            // 使用 move_to 重新在 buf_ 上构造
            void* src = other.vt_->get_ptr(other.buf_, true, nullptr);
            void* dummy_heap = nullptr;
            other.vt_->move_to(src, buf_, true, dummy_heap);
            // 析构 other 的旧对象
            other.vt_->destroy(src, true);
        }
        other.vt_   = nullptr;
        other.heap_ = nullptr;
    }

    /** @brief 从值构造 */
    template<typename T,
             typename = typename enable_if<!is_same<typename remove_cvref<T>::type, any>::value>::type>
    any(T&& value) : vt_(nullptr), sbo_(false), heap_(nullptr) {
        using ValueT = typename remove_cvref<T>::type;
        emplace<ValueT>(static_cast<T&&>(value));
    }

    /** @brief 析构 */
    ~any() {
        reset();
    }

    // =========================================================================
    // 赋值运算符
    // =========================================================================

    any& operator=(const any& other) {
        if (this != &other) {
            any tmp(other);
            swap(tmp);
        }
        return *this;
    }

    any& operator=(any&& other) noexcept {
        if (this != &other) {
            reset();
            vt_   = other.vt_;
            sbo_  = other.sbo_;
            heap_ = other.heap_;
            if (other.vt_ && other.sbo_) {
                void* src = other.vt_->get_ptr(other.buf_, true, nullptr);
                void* dummy = nullptr;
                other.vt_->move_to(src, buf_, true, dummy);
                other.vt_->destroy(src, true);
            }
            other.vt_   = nullptr;
            other.heap_ = nullptr;
        }
        return *this;
    }

    template<typename T,
             typename = typename enable_if<!is_same<typename remove_cvref<T>::type, any>::value>::type>
    any& operator=(T&& value) {
        using ValueT = typename remove_cvref<T>::type;
        reset();
        emplace<ValueT>(static_cast<T&&>(value));
        return *this;
    }

    // =========================================================================
    // 修改器
    // =========================================================================

    /** @brief 原地构造新值 */
    template<typename T, typename... Args>
    T& emplace(Args&&... args) {
        reset();
        vt_ = get_vtable<T>();
        if (use_sbo<T>()) {
            sbo_  = true;
            heap_ = nullptr;
            ::new(static_cast<void*>(buf_)) T(static_cast<Args&&>(args)...);
            return *reinterpret_cast<T*>(buf_);
        } else {
            sbo_  = false;
            heap_ = ::operator new(sizeof(T));
            ::new(heap_) T(static_cast<Args&&>(args)...);
            return *reinterpret_cast<T*>(heap_);
        }
    }

    /** @brief 重置为空 */
    void reset() noexcept {
        if (vt_) {
            void* ptr = sbo_ ? static_cast<void*>(buf_) : heap_;
            vt_->destroy(ptr, sbo_);
            if (!sbo_ && heap_) {
                ::operator delete(heap_);
            }
            vt_   = nullptr;
            heap_ = nullptr;
        }
    }

    /** @brief 交换 */
    void swap(any& other) noexcept {
        any tmp(static_cast<any&&>(*this));
        *this = static_cast<any&&>(other);
        other = static_cast<any&&>(tmp);
    }

    // =========================================================================
    // 观察者
    // =========================================================================

    /** @brief 是否有值 */
    bool has_value() const noexcept { return vt_ != nullptr; }
    explicit operator bool() const noexcept { return has_value(); }

    /** @brief 获取类型 id（用于类型检查） */
    const void* type_id() const noexcept {
        return vt_ ? vt_->type_id : nullptr;
    }

    // =========================================================================
    // 友元：any_cast
    // =========================================================================
    template<typename T>
    friend T* any_cast(any* a) noexcept;

    template<typename T>
    friend const T* any_cast(const any* a) noexcept;

private:
    bool use_sbo_from_vt(const any& other) const {
        // 根据 vtable 推断 sbo（通过对比 get_ptr 返回值）
        if (!other.vt_) return false;
        const void* p = other.vt_->get_cptr(other.buf_, true, nullptr);
        // 若 get_ptr(buf_, true, nullptr) == buf_，则是 SBO
        return (p == static_cast<const void*>(other.buf_)) || other.sbo_;
    }
};

// ============================================================================
// any_cast - 类型安全的值提取
// ============================================================================

template<typename T>
T* any_cast(any* a) noexcept {
    if (!a || !a->vt_) return nullptr;
    if (a->vt_->type_id != any::get_vtable<T>()->type_id) return nullptr;
    void* ptr = a->vt_->get_ptr(a->buf_, a->sbo_, a->heap_);
    return reinterpret_cast<T*>(ptr);
}

template<typename T>
const T* any_cast(const any* a) noexcept {
    if (!a || !a->vt_) return nullptr;
    if (a->vt_->type_id != any::get_vtable<T>()->type_id) return nullptr;
    const void* ptr = a->vt_->get_cptr(a->buf_, a->sbo_, a->heap_);
    return reinterpret_cast<const T*>(ptr);
}

template<typename T>
T any_cast(any& a) {
    T* p = any_cast<typename remove_reference<T>::type>(&a);
    if (!p) throw bad_any_cast{};
    return *p;
}

template<typename T>
T any_cast(const any& a) {
    const T* p = any_cast<T>(&a);
    if (!p) throw bad_any_cast{};
    return *p;
}

// 非成员 swap
inline void swap(any& a, any& b) noexcept { a.swap(b); }

// make_any 工厂函数
template<typename T, typename... Args>
any make_any(Args&&... args) {
    any a;
    a.emplace<T>(static_cast<Args&&>(args)...);
    return a;
}

} // namespace zen

#endif // ZEN_UTILITY_ANY_H
