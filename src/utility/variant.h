#ifndef ZEN_UTILITY_VARIANT_H
#define ZEN_UTILITY_VARIANT_H

#include <new>       // placement new
#include <stdexcept>
#include <utility>
#include <type_traits>
#include <tuple>
#include "../base/type_traits.h"

namespace zen {

// ============================================================================
// 异常类
// ============================================================================

class bad_variant_access : public std::exception {
public:
    bad_variant_access() noexcept = default;
    const char* what() const noexcept override {
        return "bad_variant_access";
    }
};

// ============================================================================
// Monostate
// ============================================================================

struct Monostate {
    bool operator==(const Monostate&) const noexcept { return true; }
    bool operator!=(const Monostate&) const noexcept { return false; }
};

// ============================================================================
// 简单辅助模板
// ============================================================================

template<typename Bool>
struct negation { static constexpr bool value = !Bool::value; };

template<typename... Bools>
struct conjunction {
    static constexpr bool value = (Bools::value && ...);
};

// ============================================================================
// variant 前向声明
// ============================================================================

template<typename... Types>
class variant;

// ============================================================================
// 获取第 I 个类型
// ============================================================================

template<size_t I, typename... Types>
struct variant_alternative {
    static_assert(I < sizeof...(Types), "Index out of bounds");
    using type = typename std::tuple_element<I, std::tuple<Types...>>::type;
};

template<size_t I, typename... Types>
using variant_alternative_t = typename variant_alternative<I, Types...>::type;

// ============================================================================
// variant_size
// ============================================================================

template<typename Variant>
struct variant_size;

template<typename... Types>
struct variant_size<variant<Types...>> {
    static constexpr size_t value = sizeof...(Types);
};

// ============================================================================
// 存储管理：使用 std::max_element 风格的模板递归
// ============================================================================

// 计算最大大小
template<typename... Types>
struct MaxSize;

template<>
struct MaxSize<> {
    static constexpr size_t value = 1;
};

template<typename Head, typename... Tail>
struct MaxSize<Head, Tail...> {
    static constexpr size_t head_size = sizeof(Head);
    static constexpr size_t tail_size = MaxSize<Tail...>::value;
    static constexpr size_t value = (head_size > tail_size) ? head_size : tail_size;
};

// 计算最大对齐
template<typename... Types>
struct MaxAlign;

template<>
struct MaxAlign<> {
    static constexpr size_t value = 1;
};

template<typename Head, typename... Tail>
struct MaxAlign<Head, Tail...> {
    static constexpr size_t head_align = alignof(Head);
    static constexpr size_t tail_align = MaxAlign<Tail...>::value;
    static constexpr size_t value = (head_align > tail_align) ? head_align : tail_align;
};

// ============================================================================
// 类型索引
// ============================================================================

template<typename T, typename... Types>
inline constexpr size_t type_index_v = []<size_t... Is>(std::index_sequence<Is...>) {
    size_t result = 9999;
    ((std::is_same_v<T, Types> ? (result = Is, true) : false), ...);
    return result;
}(std::index_sequence_for<Types...>{});

// ============================================================================
// variant 类
// ============================================================================

template<typename... Types>
class variant {
public:
    static_assert(sizeof...(Types) > 0, "variant must have at least one type");

    // -------------------------------------------------------------------------
    // 构造函数
    // -------------------------------------------------------------------------
    
    variant() : index_(0) {
        new (&storage_) variant_alternative_t<0, Types...>();
    }
    
    variant(const variant& other) : index_(other.index_) {
        copy_construct(other);
    }
    
    variant(variant&& other) : index_(other.index_) {
        move_construct(std::move(other));
    }
    
    template<typename T, typename = zen::enable_if_t<
        !zen::is_same_v<zen::remove_cvref_t<T>, variant>
    >>
    variant(T&& value) : index_(type_index_v<remove_cvref_t<T>, Types...>) {
        using ValueType = remove_cvref_t<T>;
        new (&storage_) ValueType(std::forward<T>(value));
    }
    
    // -------------------------------------------------------------------------
    // 赋值运算符
    // -------------------------------------------------------------------------
    
    variant& operator=(const variant& other) {
        if (this != &other) {
            destroy_current();
            index_ = other.index_;
            copy_construct(other);
        }
        return *this;
    }
    
    variant& operator=(variant&& other) {
        if (this != &other) {
            destroy_current();
            index_ = other.index_;
            move_construct(std::move(other));
        }
        return *this;
    }
    
    template<typename T, typename = zen::enable_if_t<
        !zen::is_same_v<zen::remove_cvref_t<T>, variant>
    >>
    variant& operator=(T&& value) {
        using ValueType = remove_cvref_t<T>;
        constexpr size_t new_idx = type_index_v<ValueType, Types...>;
        
        if (index_ == new_idx) {
            *get_value_ptr<ValueType>() = std::forward<T>(value);
        } else {
            destroy_current();
            index_ = new_idx;
            new (&storage_) ValueType(std::forward<T>(value));
        }
        return *this;
    }
    
    // -------------------------------------------------------------------------
    // 析构函数
    // -------------------------------------------------------------------------
    
    ~variant() {
        destroy_current();
    }
    
    // -------------------------------------------------------------------------
    // 属性
    // -------------------------------------------------------------------------
    
    constexpr size_t index() const noexcept {
        return index_;
    }
    
    template<typename T>
    constexpr bool holds_alternative() const noexcept {
        return index_ == type_index_v<T, Types...>;
    }
    
    // -------------------------------------------------------------------------
    // 访问
    // -------------------------------------------------------------------------
    
    template<size_t I>
    constexpr variant_alternative_t<I, Types...>& get() noexcept {
        static_assert(I < sizeof...(Types), "Index out of bounds");
        return *reinterpret_cast<variant_alternative_t<I, Types...>*>(&storage_);
    }
    
    template<size_t I>
    constexpr const variant_alternative_t<I, Types...>& get() const noexcept {
        static_assert(I < sizeof...(Types), "Index out of bounds");
        return *reinterpret_cast<const variant_alternative_t<I, Types...>*>(&storage_);
    }
    
    template<typename T>
    T& get() {
        constexpr size_t I = type_index_v<T, Types...>;
        static_assert(I < sizeof...(Types), "Type not in variant");
        if (index_ != I) throw bad_variant_access();
        return *reinterpret_cast<T*>(&storage_);
    }
    
    template<typename T>
    const T& get() const {
        constexpr size_t I = type_index_v<T, Types...>;
        static_assert(I < sizeof...(Types), "Type not in variant");
        if (index_ != I) throw bad_variant_access();
        return *reinterpret_cast<const T*>(&storage_);
    }
    
    template<typename T>
    T* get_if() noexcept {
        constexpr size_t I = type_index_v<T, Types...>;
        if (index_ == I) {
            return reinterpret_cast<T*>(&storage_);
        }
        return nullptr;
    }
    
    template<typename T>
    const T* get_if() const noexcept {
        constexpr size_t I = type_index_v<T, Types...>;
        if (index_ == I) {
            return reinterpret_cast<const T*>(&storage_);
        }
        return nullptr;
    }
    
    // -------------------------------------------------------------------------
    // 比较
    // -------------------------------------------------------------------------
    
    bool operator==(const variant& other) const {
        if (index_ != other.index_) return false;
        return compare_equal(other);
    }
    
    bool operator!=(const variant& other) const {
        return !(*this == other);
    }
    
    // -------------------------------------------------------------------------
    // emplace
    // -------------------------------------------------------------------------
    
    template<typename T, typename... Args>
    T& emplace(Args&&... args) {
        constexpr size_t I = type_index_v<T, Types...>;
        static_assert(I < sizeof...(Types), "Type not in variant");
        
        destroy_current();
        index_ = I;
        new (&storage_) T(std::forward<Args>(args)...);
        return *reinterpret_cast<T*>(&storage_);
    }
    
    // -------------------------------------------------------------------------
    // swap
    // -------------------------------------------------------------------------
    
    void swap(variant& other) {
        if (this == &other) return;
        
        if (index_ == other.index_) {
            swap_values(other);
        } else {
            variant tmp = std::move(*this);
            *this = std::move(other);
            other = std::move(tmp);
        }
    }

private:
    size_t index_;
    
    // 使用静态 constexpr 来计算最大大小
    static constexpr size_t max_size = MaxSize<Types...>::value;
    static constexpr size_t max_align = MaxAlign<Types...>::value;
    
    alignas(max_align) unsigned char storage_[max_size];
    
    // -------------------------------------------------------------------------
    // 内部辅助
    // -------------------------------------------------------------------------
    
    template<typename T>
    T* get_value_ptr() noexcept {
        return reinterpret_cast<T*>(&storage_);
    }
    
    template<typename T>
    const T* get_value_ptr() const noexcept {
        return reinterpret_cast<const T*>(&storage_);
    }
    
    // -------------------------------------------------------------------------
    // 析构
    // -------------------------------------------------------------------------
    
    void destroy_current() {
        switch (index_) {
            case 0:
                destroy_at<0>();
                break;
            case 1:
                destroy_at<1>();
                break;
            case 2:
                destroy_at<2>();
                break;
            case 3:
                destroy_at<3>();
                break;
            case 4:
                destroy_at<4>();
                break;
        }
    }
    
    template<size_t I>
    void destroy_at() {
        if constexpr (I < sizeof...(Types)) {
            using T = variant_alternative_t<I, Types...>;
            reinterpret_cast<T*>(&storage_)->~T();
        }
    }
    
    // -------------------------------------------------------------------------
    // 拷贝构造
    // -------------------------------------------------------------------------
    
    void copy_construct(const variant& other) {
        switch (other.index_) {
            case 0:
                copy_at<0>(other);
                break;
            case 1:
                copy_at<1>(other);
                break;
            case 2:
                copy_at<2>(other);
                break;
            case 3:
                copy_at<3>(other);
                break;
            case 4:
                copy_at<4>(other);
                break;
        }
    }
    
    template<size_t I>
    void copy_at(const variant& other) {
        if constexpr (I < sizeof...(Types)) {
            using T = variant_alternative_t<I, Types...>;
            new (&storage_) T(other.template get<I>());
        }
    }
    
    // -------------------------------------------------------------------------
    // 移动构造
    // -------------------------------------------------------------------------
    
    void move_construct(variant&& other) {
        switch (other.index_) {
            case 0:
                move_at<0>(std::move(other));
                break;
            case 1:
                move_at<1>(std::move(other));
                break;
            case 2:
                move_at<2>(std::move(other));
                break;
            case 3:
                move_at<3>(std::move(other));
                break;
            case 4:
                move_at<4>(std::move(other));
                break;
        }
    }
    
    template<size_t I>
    void move_at(variant&& other) {
        if constexpr (I < sizeof...(Types)) {
            using T = variant_alternative_t<I, Types...>;
            new (&storage_) T(std::move(other.template get<I>()));
        }
    }
    
    // -------------------------------------------------------------------------
    // 比较
    // -------------------------------------------------------------------------
    
    bool compare_equal(const variant& other) const {
        switch (index_) {
            case 0:
                return compare_at<0>(other);
            case 1:
                return compare_at<1>(other);
            case 2:
                return compare_at<2>(other);
            case 3:
                return compare_at<3>(other);
            case 4:
                return compare_at<4>(other);
        }
        return false;
    }
    
    template<size_t I>
    bool compare_at(const variant& other) const {
        if constexpr (I < sizeof...(Types)) {
            return this->template get<I>() == other.template get<I>();
        }
        return false;
    }
    
    // -------------------------------------------------------------------------
    // 交换
    // -------------------------------------------------------------------------
    
    void swap_values(variant& other) {
        switch (index_) {
            case 0:
                swap_at<0>(other);
                break;
            case 1:
                swap_at<1>(other);
                break;
            case 2:
                swap_at<2>(other);
                break;
            case 3:
                swap_at<3>(other);
                break;
            case 4:
                swap_at<4>(other);
                break;
        }
    }
    
    template<size_t I>
    void swap_at(variant& other) {
        if constexpr (I < sizeof...(Types)) {
            using T = variant_alternative_t<I, Types...>;
            T temp = std::move(this->template get<I>());
            this->template get<I>() = std::move(other.template get<I>());
            other.template get<I>() = std::move(temp);
        }
    }
};

// ============================================================================
// 便捷函数
// ============================================================================

template<size_t I, typename... Types>
constexpr variant_alternative_t<I, Types...>& get(variant<Types...>& v) {
    return v.template get<I>();
}

template<size_t I, typename... Types>
constexpr const variant_alternative_t<I, Types...>& get(const variant<Types...>& v) {
    return v.template get<I>();
}

template<typename T, typename... Types>
T& get(variant<Types...>& v) {
    return v.template get<T>();
}

template<typename T, typename... Types>
const T& get(const variant<Types...>& v) {
    return v.template get<T>();
}

template<typename T, typename... Types>
T* get_if(variant<Types...>* v) noexcept {
    return v ? v->template get_if<T>() : nullptr;
}

template<typename T, typename... Types>
const T* get_if(const variant<Types...>* v) noexcept {
    return v ? v->template get_if<T>() : nullptr;
}

// ============================================================================
// visit
// ============================================================================

template<typename Visitor, typename T0, typename... Types>
constexpr auto visit(Visitor&& vis, variant<T0, Types...>& v) {
    switch (v.index()) {
        case 0: return vis(v.template get<0>());
        case 1: if constexpr (sizeof...(Types) >= 1) return vis(v.template get<1>());
        case 2: if constexpr (sizeof...(Types) >= 2) return vis(v.template get<2>());
        case 3: if constexpr (sizeof...(Types) >= 3) return vis(v.template get<3>());
        case 4: if constexpr (sizeof...(Types) >= 4) return vis(v.template get<4>());
        default: throw bad_variant_access();
    }
}

template<typename Visitor, typename T0, typename... Types>
constexpr auto visit(Visitor&& vis, const variant<T0, Types...>& v) {
    switch (v.index()) {
        case 0: return vis(v.template get<0>());
        case 1: if constexpr (sizeof...(Types) >= 1) return vis(v.template get<1>());
        case 2: if constexpr (sizeof...(Types) >= 2) return vis(v.template get<2>());
        case 3: if constexpr (sizeof...(Types) >= 3) return vis(v.template get<3>());
        case 4: if constexpr (sizeof...(Types) >= 4) return vis(v.template get<4>());
        default: throw bad_variant_access();
    }
}

} // namespace zen

#endif // ZEN_UTILITY_VARIANT_H
