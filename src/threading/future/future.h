/**
 * @file future.h
 * @brief 异步任务结果（future/promise）
 * 
 * future/promise 用于线程间传递异步任务结果：
 * 
 * - promise     : 设置异步任务的结果或异常
 * - future       : 获取异步任务的结果或异常
 * - packaged_task: 打包任务，便于异步执行
 * - shared_future: 多个 future 共享同一结果
 * 
 * 特性：
 * - 支持任意类型的返回值
 * - 支持异常传播
 * - 支持超时等待
 * - shared_future 可拷贝，future 不可拷贝
 * 
 * 示例：
 * @code
 * zen::promise<int> p;
 * zen::future<int>  f = p.get_future();
 * 
 * // 在另一个线程中设置结果
 * std::thread([&]{ p.set_value(42); }).detach();
 * 
 * // 获取结果（阻塞）
 * int result = f.get();  // 42
 * 
 * // packaged_task
 * zen::packaged_task<int()> task([]{ return 42; });
 * zen::future<int> f = task.get_future();
 * std::thread(std::move(task)).detach();
 * std::cout << f.get() << std::endl;
 * @endcode
 */
#ifndef ZEN_THREADING_FUTURE_FUTURE_H
#define ZEN_THREADING_FUTURE_FUTURE_H

#include "../sync/mutex.h"
#include "../sync/condition_variable.h"
#include "../sync/unique_lock.h"
#include "../sync/lock_guard.h"
#include "../../utility/optional.h"
#include "../../base/type_traits.h"
#include "../thread/thread.h"

#include <stdexcept>
#include <exception>
#include <memory>

namespace zen {

// ============================================================================
// future_state：存储异步结果或异常
// ============================================================================

namespace detail {

/**
 * @brief 异步结果状态
 * 
 * 存储计算结果或异常，线程安全。
 */
template<typename T>
class future_state {
public:
    future_state() : ready_(false) {}
    
    /**
     * @brief 设置值
     */
    void set_value(const T& value) {
        lock_guard<mutex> lock(mutex_);
        if (ready_) {
            throw std::future_error(std::future_errc::promise_already_satisfied);
        }
        value_ = value;
        ready_ = true;
        cond_.notify_all();
    }
    
    void set_value(T&& value) {
        lock_guard<mutex> lock(mutex_);
        if (ready_) {
            throw std::future_error(std::future_errc::promise_already_satisfied);
        }
        value_ = std::move(value);
        ready_ = true;
        cond_.notify_all();
    }
    
    /**
     * @brief 设置异常
     */
    void set_exception(std::exception_ptr ex) {
        lock_guard<mutex> lock(mutex_);
        if (ready_) {
            throw std::future_error(std::future_errc::promise_already_satisfied);
        }
        exception_ = ex;
        ready_ = true;
        cond_.notify_all();
    }
    
    /**
     * @brief 获取值（阻塞直到就绪）
     */
    T get() {
        unique_lock<mutex> lock(mutex_);
        cond_.wait(lock, [this] { return ready_; });
        
        if (exception_) {
            std::rethrow_exception(exception_);
        }
        
        return std::move(*value_);
    }
    
    /**
     * @brief 等待就绪
     */
    void wait() const {
        unique_lock<mutex> lock(mutex_);
        cond_.wait(lock, [this] { return ready_; });
    }
    
    /**
     * @brief 超时等待
     */
    template<typename Rep, typename Period>
    bool wait_for(const std::chrono::duration<Rep, Period>& timeout) const {
        unique_lock<mutex> lock(mutex_);
        return cond_.wait_for(lock, timeout, [this] { return ready_; });
    }
    
    /**
     * @brief 检查是否就绪
     */
    bool is_ready() const {
        lock_guard<mutex> lock(mutex_);
        return ready_;
    }
    
    /**
     * @brief 增加引用计数（用于 shared_future）
     */
    void add_ref() {
        lock_guard<mutex> lock(ref_mutex_);
        ++ref_count_;
    }
    
    /**
     * @brief 减少引用计数
     */
    void release() {
        lock_guard<mutex> lock(ref_mutex_);
        if (--ref_count_ == 0) {
            delete this;
        }
    }
    
    /**
     * @brief 获取引用计数
     */
    int ref_count() const {
        lock_guard<mutex> lock(ref_mutex_);
        return ref_count_;
    }

private:
    mutable mutex mutex_;
    mutable condition_variable cond_;
    optional<T> value_;
    std::exception_ptr exception_;
    bool ready_;
    
    // shared_future 引用计数
    mutable mutex ref_mutex_;
    int ref_count_{1};
};

// void 特化
template<>
class future_state<void> {
public:
    future_state() : ready_(false) {}
    
    void set_value() {
        lock_guard<mutex> lock(mutex_);
        if (ready_) {
            throw std::future_error(std::future_errc::promise_already_satisfied);
        }
        ready_ = true;
        cond_.notify_all();
    }
    
    void set_exception(std::exception_ptr ex) {
        lock_guard<mutex> lock(mutex_);
        if (ready_) {
            throw std::future_error(std::future_errc::promise_already_satisfied);
        }
        exception_ = ex;
        ready_ = true;
        cond_.notify_all();
    }
    
    void get() {
        unique_lock<mutex> lock(mutex_);
        cond_.wait(lock, [this] { return ready_; });
        
        if (exception_) {
            std::rethrow_exception(exception_);
        }
    }
    
    void wait() const {
        unique_lock<mutex> lock(mutex_);
        cond_.wait(lock, [this] { return ready_; });
    }
    
    template<typename Rep, typename Period>
    bool wait_for(const std::chrono::duration<Rep, Period>& timeout) const {
        unique_lock<mutex> lock(mutex_);
        return cond_.wait_for(lock, timeout, [this] { return ready_; });
    }
    
    bool is_ready() const {
        lock_guard<mutex> lock(mutex_);
        return ready_;
    }
    
    void add_ref() {
        lock_guard<mutex> lock(ref_mutex_);
        ++ref_count_;
    }
    
    void release() {
        lock_guard<mutex> lock(ref_mutex_);
        if (--ref_count_ == 0) {
            delete this;
        }
    }
    
    int ref_count() const {
        lock_guard<mutex> lock(ref_mutex_);
        return ref_count_;
    }

private:
    mutable mutex mutex_;
    mutable condition_variable cond_;
    std::exception_ptr exception_;
    bool ready_;
    
    mutable mutex ref_mutex_;
    int ref_count_{1};
};

} // namespace detail

// ============================================================================
// future
// ============================================================================

/**
 * @brief 异步结果访问器
 * 
 * 不可拷贝，可移动。
 * 调用 get() 获取结果（阻塞）。
 */
template<typename T>
class future {
public:
    future() noexcept : state_(nullptr) {}
    
    future(future&& other) noexcept : state_(other.state_) {
        other.state_ = nullptr;
    }
    
    future& operator=(future&& other) noexcept {
        if (this != &other) {
            if (state_) {
                state_->release();
            }
            state_ = other.state_;
            other.state_ = nullptr;
        }
        return *this;
    }
    
    // 不可拷贝
    future(const future&)            = delete;
    future& operator=(const future&) = delete;
    
    ~future() {
        if (state_) {
            state_->release();
        }
    }
    
    /**
     * @brief 获取结果（阻塞）
     */
    T get() {
        if (!state_) {
            throw std::future_error(std::future_errc::no_state);
        }
        return state_->get();
    }
    
    /**
     * @brief 等待就绪
     */
    void wait() const {
        if (!state_) {
            throw std::future_error(std::future_errc::no_state);
        }
        state_->wait();
    }
    
    /**
     * @brief 超时等待
     */
    template<typename Rep, typename Period>
    bool wait_for(const std::chrono::duration<Rep, Period>& timeout) const {
        if (!state_) {
            throw std::future_error(std::future_errc::no_state);
        }
        return state_->wait_for(timeout);
    }
    
    /**
     * @brief 检查是否就绪
     */
    bool is_ready() const {
        return state_ ? state_->is_ready() : false;
    }
    
    /**
     * @brief 是否有效
     */
    bool valid() const noexcept {
        return state_ != nullptr;
    }
    
    /**
     * @brief 转换为 shared_future
     */
    shared_future<T> share() {
        if (!state_) {
            throw std::future_error(std::future_errc::no_state);
        }
        state_->add_ref();
        return shared_future<T>(state_);
    }

private:
    template<typename>
    friend class promise;
    template<typename>
    friend class packaged_task;
    
    explicit future(detail::future_state<T>* state) : state_(state) {}
    
    detail::future_state<T>* state_;
};

// void 特化
template<>
class future<void> {
public:
    future() noexcept : state_(nullptr) {}
    
    future(future&& other) noexcept : state_(other.state_) {
        other.state_ = nullptr;
    }
    
    future& operator=(future&& other) noexcept {
        if (this != &other) {
            if (state_) {
                state_->release();
            }
            state_ = other.state_;
            other.state_ = nullptr;
        }
        return *this;
    }
    
    future(const future&)            = delete;
    future& operator=(const future&) = delete;
    
    ~future() {
        if (state_) {
            state_->release();
        }
    }
    
    void get() {
        if (!state_) {
            throw std::future_error(std::future_errc::no_state);
        }
        state_->get();
    }
    
    void wait() const {
        if (!state_) {
            throw std::future_error(std::future_errc::no_state);
        }
        state_->wait();
    }
    
    template<typename Rep, typename Period>
    bool wait_for(const std::chrono::duration<Rep, Period>& timeout) const {
        if (!state_) {
            throw std::future_error(std::future_errc::no_state);
        }
        return state_->wait_for(timeout);
    }
    
    bool is_ready() const {
        return state_ ? state_->is_ready() : false;
    }
    
    bool valid() const noexcept {
        return state_ != nullptr;
    }
    
    shared_future<void> share() {
        if (!state_) {
            throw std::future_error(std::future_errc::no_state);
        }
        state_->add_ref();
        return shared_future<void>(state_);
    }

private:
    template<typename>
    friend class promise;
    template<typename>
    friend class packaged_task;
    
    explicit future(detail::future_state<void>* state) : state_(state) {}
    
    detail::future_state<void>* state_;
};

// ============================================================================
// shared_future
// ============================================================================

/**
 * @brief 可共享的异步结果访问器
 * 
 * 可拷贝，多个 shared_future 共享同一结果。
 */
template<typename T>
class shared_future {
public:
    shared_future() noexcept : state_(nullptr) {}
    
    shared_future(const shared_future& other) noexcept : state_(other.state_) {
        if (state_) {
            state_->add_ref();
        }
    }
    
    shared_future(shared_future&& other) noexcept : state_(other.state_) {
        other.state_ = nullptr;
    }
    
    ~shared_future() {
        if (state_) {
            state_->release();
        }
    }
    
    shared_future& operator=(const shared_future& other) noexcept {
        if (this != &other) {
            if (state_) {
                state_->release();
            }
            state_ = other.state_;
            if (state_) {
                state_->add_ref();
            }
        }
        return *this;
    }
    
    shared_future& operator=(shared_future&& other) noexcept {
        if (this != &other) {
            if (state_) {
                state_->release();
            }
            state_ = other.state_;
            other.state_ = nullptr;
        }
        return *this;
    }
    
    const T& get() const {
        if (!state_) {
            throw std::future_error(std::future_errc::no_state);
        }
        return state_->get();
    }
    
    void wait() const {
        if (!state_) {
            throw std::future_error(std::future_errc::no_state);
        }
        state_->wait();
    }
    
    template<typename Rep, typename Period>
    bool wait_for(const std::chrono::duration<Rep, Period>& timeout) const {
        if (!state_) {
            throw std::future_error(std::future_errc::no_state);
        }
        return state_->wait_for(timeout);
    }
    
    bool is_ready() const {
        return state_ ? state_->is_ready() : false;
    }
    
    bool valid() const noexcept {
        return state_ != nullptr;
    }

private:
    template<typename>
    friend class future;
    
    explicit shared_future(detail::future_state<T>* state) : state_(state) {}
    
    detail::future_state<T>* state_;
};

// ============================================================================
// promise
// ============================================================================

/**
 * @brief 异步结果提供者
 * 
 * promise 用于设置异步任务的结果或异常。
 * 每个 promise 只能设置一次结果。
 */
template<typename T>
class promise {
public:
    promise() : state_(new detail::future_state<T>()) {}
    
    promise(promise&& other) noexcept : state_(other.state_) {
        other.state_ = nullptr;
    }
    
    promise& operator=(promise&& other) noexcept {
        if (this != &other) {
            if (state_) {
                delete state_;
            }
            state_ = other.state_;
            other.state_ = nullptr;
        }
        return *this;
    }
    
    promise(const promise&)            = delete;
    promise& operator=(const promise&) = delete;
    
    ~promise() {
        if (state_) {
            // 如果未设置结果，设置一个异常
            if (!state_->is_ready()) {
                try {
                    throw std::future_error(std::future_errc::broken_promise);
                } catch (...) {
                    state_->set_exception(std::current_exception());
                }
            }
            state_->release();
        }
    }
    
    /**
     * @brief 设置值
     */
    void set_value(const T& value) {
        if (!state_) {
            throw std::future_error(std::future_errc::no_state);
        }
        state_->set_value(value);
    }
    
    void set_value(T&& value) {
        if (!state_) {
            throw std::future_error(std::future_errc::no_state);
        }
        state_->set_value(std::move(value));
    }
    
    /**
     * @brief 设置异常
     */
    void set_exception(std::exception_ptr ex) {
        if (!state_) {
            throw std::future_error(std::future_errc::no_state);
        }
        state_->set_exception(ex);
    }
    
    /**
     * @brief 获取关联的 future
     */
    future<T> get_future() {
        if (!state_ || future_obtained_) {
            throw std::future_error(std::future_errc::future_already_retrieved);
        }
        future_obtained_ = true;
        return future<T>(state_);
    }

private:
    detail::future_state<T>* state_;
    bool future_obtained_{false};
};

// void 特化
template<>
class promise<void> {
public:
    promise() : state_(new detail::future_state<void>()) {}
    
    promise(promise&& other) noexcept : state_(other.state_) {
        other.state_ = nullptr;
    }
    
    promise& operator=(promise&& other) noexcept {
        if (this != &other) {
            if (state_) {
                delete state_;
            }
            state_ = other.state_;
            other.state_ = nullptr;
        }
        return *this;
    }
    
    promise(const promise&)            = delete;
    promise& operator=(const promise&) = delete;
    
    ~promise() {
        if (state_) {
            if (!state_->is_ready()) {
                try {
                    throw std::future_error(std::future_errc::broken_promise);
                } catch (...) {
                    state_->set_exception(std::current_exception());
                }
            }
            state_->release();
        }
    }
    
    void set_value() {
        if (!state_) {
            throw std::future_error(std::future_errc::no_state);
        }
        state_->set_value();
    }
    
    void set_exception(std::exception_ptr ex) {
        if (!state_) {
            throw std::future_error(std::future_errc::no_state);
        }
        state_->set_exception(ex);
    }
    
    future<void> get_future() {
        if (!state_ || future_obtained_) {
            throw std::future_error(std::future_errc::future_already_retrieved);
        }
        future_obtained_ = true;
        return future<void>(state_);
    }

private:
    detail::future_state<void>* state_;
    bool future_obtained_{false};
};

// ============================================================================
// packaged_task
// ============================================================================

/**
 * @brief 可调用的任务包装器
 * 
 * packaged_task 将任务和结果存储在一起，
 * 可将任务移交给其他线程执行，并通过 future 获取结果。
 */
template<typename>
class packaged_task;  // 前置声明

template<typename R, typename... Args>
class packaged_task<R(Args...)> {
public:
    packaged_task() noexcept : state_(nullptr) {}
    
    template<typename F>
    explicit packaged_task(F&& f) 
        : state_(new detail::packaged_task_state_impl<F>(std::forward<F>(f))) {}
    
    packaged_task(packaged_task&& other) noexcept : state_(other.state_) {
        other.state_ = nullptr;
    }
    
    packaged_task& operator=(packaged_task&& other) noexcept {
        if (this != &other) {
            if (state_) {
                delete state_;
            }
            state_ = other.state_;
            other.state_ = nullptr;
        }
        return *this;
    }
    
    packaged_task(const packaged_task&)            = delete;
    packaged_task& operator=(const packaged_task&) = delete;
    
    ~packaged_task() {
        if (state_) {
            state_->release();
        }
    }
    
    /**
     * @brief 执行任务
     */
    void operator()(Args... args) {
        if (!state_) {
            throw std::future_error(std::future_errc::no_state);
        }
        state_->invoke(args...);
    }
    
    /**
     * @brief 获取关联的 future
     */
    future<R> get_future() {
        if (!state_ || future_obtained_) {
            throw std::future_error(std::future_errc::future_already_retrieved);
        }
        future_obtained_ = true;
        return future<R>(state_);
    }
    
    /**
     * @brief 检查是否有效
     */
    bool valid() const noexcept {
        return state_ != nullptr;
    }

private:
    struct packaged_task_state_base {
        virtual ~packaged_task_state_base() = default;
        virtual void invoke(Args...) = 0;
        virtual detail::future_state<R>* get_future_state() = 0;
        virtual void add_ref() = 0;
        virtual void release() = 0;
    };
    
    template<typename F>
    struct packaged_task_state_impl : packaged_task_state_base {
        F func_;
        detail::future_state<R> future_state_;
        int ref_count_{1};
        
        explicit packaged_task_state_impl(F&& f) : func_(std::forward<F>(f)) {}
        
        void invoke(Args... args) override {
            try {
                future_state_.set_value(func_(args...));
            } catch (...) {
                future_state_.set_exception(std::current_exception());
            }
        }
        
        detail::future_state<R>* get_future_state() override {
            return &future_state_;
        }
        
        void add_ref() override { ++ref_count_; }
        void release() override {
            if (--ref_count_ == 0) {
                delete this;
            }
        }
    };
    
    packaged_task_state_base* state_;
    bool future_obtained_{false};
};

} // namespace zen

#endif // ZEN_THREADING_FUTURE_FUTURE_H
