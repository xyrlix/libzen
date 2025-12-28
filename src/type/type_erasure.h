#pragma once

#include <memory>
#include <functional>
#include <type_traits>

namespace zen {

/**
 * @brief 类型擦除工具类
 * 封装多态类型，避免模板膨胀，适配回调/容器
 */
namespace type_erasure {

/**
 * @brief 可调用对象的类型擦除基类
 */
class any_callable {
public:
    virtual ~any_callable() = default;
    
    /**
     * @brief 克隆函数，用于复制可调用对象
     * @return 克隆后的可调用对象
     */
    virtual std::unique_ptr<any_callable> clone() const = 0;
};

/**
 * @brief 可调用对象的类型擦除实现类
 * @tparam F 可调用对象类型
 */
template <typename F>
class any_callable_impl : public any_callable {
public:
    /**
     * @brief 构造函数
     * @param func 可调用对象
     */
    explicit any_callable_impl(F&& func) : func_(std::forward<F>(func)) {}
    
    /**
     * @brief 构造函数（拷贝版本）
     * @param func 可调用对象
     */
    explicit any_callable_impl(const F& func) : func_(func) {}
    
    /**
     * @brief 克隆函数实现
     * @return 克隆后的可调用对象
     */
    std::unique_ptr<any_callable> clone() const override {
        return std::make_unique<any_callable_impl<F>>(func_);
    }
    
    /**
     * @brief 获取可调用对象
     * @return 可调用对象引用
     */
    F& get() { return func_; }
    
    /**
     * @brief 获取可调用对象（常量版本）
     * @return 可调用对象常量引用
     */
    const F& get() const { return func_; }
    
private:
    F func_; ///< 实际的可调用对象
};

/**
 * @brief 可调用对象的类型擦除封装
 */
class any_function {
public:
    any_function() = default;
    
    /**
     * @brief 构造函数
     * @tparam F 可调用对象类型
     * @param func 可调用对象
     */
    template <typename F>
    any_function(F&& func) : impl_(std::make_unique<any_callable_impl<typename std::decay<F>::type>>(std::forward<F>(func))) {}
    
    /**
     * @brief 拷贝构造函数
     * @param other 其他any_function对象
     */
    any_function(const any_function& other) {
        if (other.impl_) {
            impl_ = other.impl_->clone();
        }
    }
    
    /**
     * @brief 移动构造函数
     * @param other 其他any_function对象
     */
    any_function(any_function&& other) noexcept = default;
    
    /**
     * @brief 拷贝赋值运算符
     * @param other 其他any_function对象
     * @return 自身引用
     */
    any_function& operator=(const any_function& other) {
        if (this != &other) {
            if (other.impl_) {
                impl_ = other.impl_->clone();
            } else {
                impl_.reset();
            }
        }
        return *this;
    }
    
    /**
     * @brief 移动赋值运算符
     * @param other 其他any_function对象
     * @return 自身引用
     */
    any_function& operator=(any_function&& other) noexcept = default;
    
    /**
     * @brief 检查是否包含可调用对象
     * @return 是否包含可调用对象
     */
    explicit operator bool() const noexcept {
        return static_cast<bool>(impl_);
    }
    
    /**
     * @brief 获取可调用对象
     * @tparam F 可调用对象类型
     * @return 可调用对象引用
     */
    template <typename F>
    F& get() {
        auto* p = dynamic_cast<any_callable_impl<F>*>(impl_.get());
        if (!p) {
            throw std::bad_cast();
        }
        return p->get();
    }
    
    /**
     * @brief 获取可调用对象（常量版本）
     * @tparam F 可调用对象类型
     * @return 可调用对象常量引用
     */
    template <typename F>
    const F& get() const {
        auto* p = dynamic_cast<const any_callable_impl<F>*>(impl_.get());
        if (!p) {
            throw std::bad_cast();
        }
        return p->get();
    }
    
    /**
     * @brief 调用可调用对象
     * @tparam Args 参数类型
     * @param args 调用参数
     * @return 调用结果
     */
    template <typename... Args>
    auto operator()(Args&&... args) -> decltype(auto) {
        return invoke_impl<Args...>(impl_.get(), std::forward<Args>(args)...);
    }
    
    /**
     * @brief 调用可调用对象（常量版本）
     * @tparam Args 参数类型
     * @param args 调用参数
     * @return 调用结果
     */
    template <typename... Args>
    auto operator()(Args&&... args) const -> decltype(auto) {
        return invoke_impl<Args...>(impl_.get(), std::forward<Args>(args)...);
    }
    
private:
    /**
     * @brief 调用实现
     * @tparam Args 参数类型
     * @param impl 可调用对象实现
     * @param args 调用参数
     * @return 调用结果
     */
    template <typename... Args>
    static auto invoke_impl(any_callable* impl, Args&&... args) -> decltype(auto) {
        if (!impl) {
            throw std::bad_function_call();
        }
        
        // 这里简化实现，实际应该使用类型擦除的方式调用
        // 由于any_function没有指定返回类型和参数类型，这里无法直接调用
        // 实际使用时应该使用更具体的类型擦除，如function_view
        throw std::runtime_error("any_function cannot be invoked directly, use function_view instead");
    }
    
    std::unique_ptr<any_callable> impl_; ///< 可调用对象的类型擦除实现
};

/**
 * @brief 函数视图的类型擦除，支持调用
 * @tparam Signature 函数签名
 */
template <typename Signature>
class function_view;

/**
 * @brief 函数视图的类型擦除，支持调用（特化版本）
 * @tparam R 返回类型
 * @tparam Args 参数类型
 */
template <typename R, typename... Args>
class function_view<R(Args...)> {
public:
    /**
     * @brief 构造函数
     */
    function_view() noexcept : invoke_(nullptr), data_(nullptr) {}
    
    /**
     * @brief 构造函数
     * @tparam F 可调用对象类型
     * @param func 可调用对象
     */
    template <typename F,
              typename = typename std::enable_if<!std::is_same<typename std::decay<F>::type, function_view>::value>::type>
    function_view(F&& func) noexcept {
        using Functor = typename std::decay<F>::type;
        
        static_assert(std::is_invocable_r<R, Functor, Args...>::value, "Functor must be callable with Args... and return R");
        
        data_ = &func;
        invoke_ = [](void* data, Args&&... args) -> R {
            return (*static_cast<Functor*>(data))(std::forward<Args>(args)...);
        };
    }
    
    /**
     * @brief 调用函数
     * @param args 调用参数
     * @return 调用结果
     */
    R operator()(Args... args) const {
        if (!invoke_) {
            throw std::bad_function_call();
        }
        return invoke_(data_, std::forward<Args>(args)...);
    }
    
    /**
     * @brief 检查是否包含函数
     * @return 是否包含函数
     */
    explicit operator bool() const noexcept {
        return invoke_ != nullptr;
    }
    
private:
    using invoke_func = R(*)(void*, Args&&...);
    
    invoke_func invoke_; ///< 调用函数指针
    void* data_;         ///< 可调用对象指针
};

/**
 * @brief 任意类型的类型擦除封装
 */
class any {
public:
    any() noexcept : type_(nullptr), data_(nullptr), destroy_(nullptr), clone_(nullptr) {}
    
    /**
     * @brief 构造函数
     * @tparam T 类型
     * @param value 值
     */
    template <typename T,
              typename = typename std::enable_if<!std::is_same<typename std::decay<T>::type, any>::value>::type>
    any(T&& value) {
        using U = typename std::decay<T>::type;
        
        data_ = new U(std::forward<T>(value));
        type_ = &typeid(U);
        destroy_ = [](void* data) {
            delete static_cast<U*>(data);
        };
        clone_ = [](void* data) -> void* {
            return new U(*static_cast<U*>(data));
        };
    }
    
    /**
     * @brief 拷贝构造函数
     * @param other 其他any对象
     */
    any(const any& other) {
        if (other.data_) {
            data_ = other.clone_(other.data_);
            type_ = other.type_;
            destroy_ = other.destroy_;
            clone_ = other.clone_;
        } else {
            data_ = nullptr;
            type_ = nullptr;
            destroy_ = nullptr;
            clone_ = nullptr;
        }
    }
    
    /**
     * @brief 移动构造函数
     * @param other 其他any对象
     */
    any(any&& other) noexcept : any() {
        swap(*this, other);
    }
    
    /**
     * @brief 析构函数
     */
    ~any() {
        if (data_) {
            destroy_(data_);
        }
    }
    
    /**
     * @brief 拷贝赋值运算符
     * @param other 其他any对象
     * @return 自身引用
     */
    any& operator=(const any& other) {
        any(other).swap(*this);
        return *this;
    }
    
    /**
     * @brief 移动赋值运算符
     * @param other 其他any对象
     * @return 自身引用
     */
    any& operator=(any&& other) noexcept {
        any().swap(*this).swap(other);
        return *this;
    }
    
    /**
     * @brief 交换两个any对象
     * @param lhs 左操作数
     * @param rhs 右操作数
     */
    friend void swap(any& lhs, any& rhs) noexcept {
        using std::swap;
        swap(lhs.type_, rhs.type_);
        swap(lhs.data_, rhs.data_);
        swap(lhs.destroy_, rhs.destroy_);
        swap(lhs.clone_, rhs.clone_);
    }
    
    /**
     * @brief 检查是否包含值
     * @return 是否包含值
     */
    bool has_value() const noexcept {
        return data_ != nullptr;
    }
    
    /**
     * @brief 获取类型信息
     * @return 类型信息
     */
    const std::type_info& type() const noexcept {
        return data_ ? *type_ : typeid(void);
    }
    
    /**
     * @brief 重置any对象
     */
    void reset() noexcept {
        any().swap(*this);
    }
    
    /**
     * @brief 获取值
     * @tparam T 类型
     * @return 值的引用
     */
    template <typename T>
    T& get() {
        if (!data_ || typeid(T) != *type_) {
            throw std::bad_cast();
        }
        return *static_cast<T*>(data_);
    }
    
    /**
     * @brief 获取值（常量版本）
     * @tparam T 类型
     * @return 值的常量引用
     */
    template <typename T>
    const T& get() const {
        if (!data_ || typeid(T) != *type_) {
            throw std::bad_cast();
        }
        return *static_cast<T*>(data_);
    }
    
private:
    const std::type_info* type_;
    void* data_;
    void (*destroy_)(void*);
    void* (*clone_)(void*);
};

} // namespace type_erasure

} // namespace zen
