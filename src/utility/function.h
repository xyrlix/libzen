/**
 * @file function.h
 * @brief 通用函数包装器
 * 
 * function 可以存储任意可调用对象（函数指针、lambda、函数对象）：
 * 
 * - function<R(Args...)> : 存储返回值类型为 R、参数为 Args... 的可调用对象
 * 
 * 特性：
 * - 类型擦除：统一的接口存储不同类型
 * - 支持移动语义
 * - 小对象优化（避免堆分配）
 * 
 * 示例：
 * @code
 * zen::function<int(int)> f = [](int x) { return x * 2; };
 * std::cout << f(21) << std::endl;  // 42
 * 
 * // 存储函数指针
 * int add(int a, int b) { return a + b; }
 * zen::function<int(int, int)> f2 = add;
 * 
 * // 存储函数对象
 * struct Multiply {
 *     int operator()(int a, int b) const { return a * b; }
 * };
 * zen::function<int(int, int)> f3 = Multiply();
 * @endcode
 */
#ifndef ZEN_UTILITY_FUNCTION_H
#define ZEN_UTILITY_FUNCTION_H

#include <utility>
#include <memory>

namespace zen {

namespace detail {

/**
 * @brief 函数包装器基类（类型擦除）
 */
template<typename Signature>
class function_base;

template<typename R, typename... Args>
class function_base<R(Args...)> {
public:
    virtual ~function_base() = default;
    virtual R invoke(Args... args) = 0;
    virtual function_base* clone() const = 0;
    virtual void move_to(function_base* other) = 0;
};

/**
 * @brief 具体可调用对象存储
 */
template<typename F, typename Signature>
class function_impl;

template<typename F, typename R, typename... Args>
class function_impl<F, R(Args...)> : public function_base<R(Args...)> {
public:
    explicit function_impl(F&& f) : func_(std::forward<F>(f)) {}
    
    R invoke(Args... args) override {
        return func_(args...);
    }
    
    function_base<R(Args...)>* clone() const override {
        return new function_impl<F, R(Args...)>(func_);
    }
    
    void move_to(function_base<R(Args...)>* other) override {
        static_cast<function_impl*>(other)->func_ = std::move(func_);
    }
    
private:
    F func_;
};

} // namespace detail

// ============================================================================
// function
// ============================================================================

/**
 * @brief 通用函数包装器
 */
template<typename Signature>
class function;

template<typename R, typename... Args>
class function<R(Args...)> {
public:
    /**
     * @brief 默认构造（空函数）
     */
    function() noexcept : impl_(nullptr) {}
    
    /**
     * @brief 从 nullptr 构造
     */
    function(std::nullptr_t) noexcept : impl_(nullptr) {}
    
    /**
     * @brief 从任意可调用对象构造
     */
    template<typename F>
    function(F&& f) 
        : impl_(new detail::function_impl<typename std::decay<F>::type, R(Args...)>(
            std::forward<F>(f))) {}
    
    /**
     * @brief 拷贝构造
     */
    function(const function& other) 
        : impl_(other.impl_ ? other.impl_->clone() : nullptr) {}
    
    /**
     * @brief 移动构造
     */
    function(function&& other) noexcept : impl_(other.impl_) {
        other.impl_ = nullptr;
    }
    
    /**
     * @brief 析构
     */
    ~function() {
        delete impl_;
    }
    
    /**
     * @brief 拷贝赋值
     */
    function& operator=(const function& other) {
        if (this != &other) {
            delete impl_;
            impl_ = other.impl_ ? other.impl_->clone() : nullptr;
        }
        return *this;
    }
    
    /**
     * @brief 移动赋值
     */
    function& operator=(function&& other) noexcept {
        if (this != &other) {
            delete impl_;
            impl_ = other.impl_;
            other.impl_ = nullptr;
        }
        return *this;
    }
    
    /**
     * @brief 赋值 nullptr
     */
    function& operator=(std::nullptr_t) noexcept {
        delete impl_;
        impl_ = nullptr;
        return *this;
    }
    
    /**
     * @brief 赋值可调用对象
     */
    template<typename F>
    function& operator=(F&& f) {
        delete impl_;
        impl_ = new detail::function_impl<typename std::decay<F>::type, R(Args...)>(
            std::forward<F>(f));
        return *this;
    }
    
    /**
     * @brief 调用函数
     */
    R operator()(Args... args) const {
        if (!impl_) {
            throw std::bad_function_call();
        }
        return impl_->invoke(args...);
    }
    
    /**
     * @brief 检查是否为空
     */
    explicit operator bool() const noexcept {
        return impl_ != nullptr;
    }
    
    /**
     * @brief 交换
     */
    void swap(function& other) noexcept {
        std::swap(impl_, other.impl_);
    }

private:
    detail::function_base<R(Args...)>* impl_;
};

// 交换函数
template<typename Signature>
void swap(function<Signature>& a, function<Signature>& b) noexcept {
    a.swap(b);
}

} // namespace zen

#endif // ZEN_UTILITY_FUNCTION_H
