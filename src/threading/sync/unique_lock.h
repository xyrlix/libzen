/**
 * @file unique_lock.h
 * @brief 灵活 RAII 锁管理器（unique_lock）
 * 
 * unique_lock 与 lock_guard 相比增加了：
 * - 延迟加锁（defer_lock）
 * - 提前解锁（unlock）
 * - 重新加锁（lock）
 * - 移动语义
 * - try_lock 支持
 * 
 * 示例：
 * @code
 * zen::mutex m;
 * zen::unique_lock<zen::mutex> ul(m, zen::defer_lock);
 * // ... 做其他事情
 * ul.lock();  // 需要时加锁
 * // 临界区
 * ul.unlock();  // 提前解锁
 * @endcode
 */
#ifndef ZEN_THREADING_SYNC_UNIQUE_LOCK_H
#define ZEN_THREADING_SYNC_UNIQUE_LOCK_H

#include "lock_guard.h"

namespace zen {

template<typename Mutex>
class unique_lock {
public:
    using mutex_type = Mutex;
    
    /**
     * @brief 默认构造（不关联任何 mutex）
     */
    unique_lock() noexcept : mutex_(nullptr), owns_(false) {}
    
    /**
     * @brief 构造并加锁
     */
    explicit unique_lock(Mutex& m) : mutex_(&m), owns_(true) {
        m.lock();
    }
    
    /**
     * @brief 延迟加锁
     */
    unique_lock(Mutex& m, defer_lock_t) noexcept : mutex_(&m), owns_(false) {}
    
    /**
     * @brief 尝试加锁
     */
    unique_lock(Mutex& m, try_to_lock_t) : mutex_(&m), owns_(m.try_lock()) {}
    
    /**
     * @brief 接管已持有的锁
     */
    unique_lock(Mutex& m, adopt_lock_t) noexcept : mutex_(&m), owns_(true) {}
    
    /**
     * @brief 移动构造
     */
    unique_lock(unique_lock&& other) noexcept 
        : mutex_(other.mutex_), owns_(other.owns_) {
        other.mutex_ = nullptr;
        other.owns_ = false;
    }
    
    /**
     * @brief 析构：如果持有锁则解锁
     */
    ~unique_lock() noexcept {
        if (owns_ && mutex_) {
            mutex_->unlock();
        }
    }
    
    /**
     * @brief 移动赋值
     */
    unique_lock& operator=(unique_lock&& other) noexcept {
        if (this != &other) {
            if (owns_ && mutex_) {
                mutex_->unlock();
            }
            mutex_ = other.mutex_;
            owns_ = other.owns_;
            other.mutex_ = nullptr;
            other.owns_ = false;
        }
        return *this;
    }
    
    // 不可拷贝
    unique_lock(const unique_lock&)            = delete;
    unique_lock& operator=(const unique_lock&) = delete;
    
    /**
     * @brief 加锁
     */
    void lock() {
        if (!mutex_ || owns_) {
            throw std::system_error(std::make_error_code(std::errc::operation_not_permitted));
        }
        mutex_->lock();
        owns_ = true;
    }
    
    /**
     * @brief 尝试加锁
     */
    bool try_lock() {
        if (!mutex_ || owns_) {
            throw std::system_error(std::make_error_code(std::errc::operation_not_permitted));
        }
        owns_ = mutex_->try_lock();
        return owns_;
    }
    
    /**
     * @brief 解锁
     */
    void unlock() {
        if (!owns_ || !mutex_) {
            throw std::system_error(std::make_error_code(std::errc::operation_not_permitted));
        }
        mutex_->unlock();
        owns_ = false;
    }
    
    /**
     * @brief 交换
     */
    void swap(unique_lock& other) noexcept {
        std::swap(mutex_, other.mutex_);
        std::swap(owns_, other.owns_);
    }
    
    /**
     * @brief 释放 mutex（返回指针，不再自动解锁）
     */
    Mutex* release() noexcept {
        Mutex* m = mutex_;
        mutex_ = nullptr;
        owns_ = false;
        return m;
    }
    
    /**
     * @brief 是否持有锁
     */
    bool owns_lock() const noexcept {
        return owns_;
    }
    
    /**
     * @brief 是否持有锁（operator bool）
     */
    explicit operator bool() const noexcept {
        return owns_;
    }
    
    /**
     * @brief 获取关联的 mutex
     */
    Mutex* mutex() const noexcept {
        return mutex_;
    }

private:
    Mutex* mutex_;
    bool owns_;
};

} // namespace zen

#endif // ZEN_THREADING_SYNC_UNIQUE_LOCK_H
