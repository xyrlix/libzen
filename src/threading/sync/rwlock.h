/**
 * @file rwlock.h
 * @brief 读写锁实现
 * 
 * 读写锁（Reader-Writer Lock）允许：
 * - 多个读者同时持有读锁（共享访问）
 * - 只有一个写者可以持有写锁（独占访问）
 * - 读写互斥、写写互斥
 * 
 * 提供两个版本：
 * - rwlock        : 基于 POSIX pthread_rwlock_t / Win32 SRWLock
 * - shared_mutex  : 同义（STL 风格别名接口）
 * 
 * 配套 RAII 守卫：
 * - shared_lock   : 持有读锁
 * - write_lock    : 持有写锁（等价于 unique_lock）
 * 
 * 示例：
 * @code
 * zen::rwlock rw;
 * 
 * // 读者
 * zen::shared_lock<zen::rwlock> rl(rw);
 * // ... 只读操作 ...
 * 
 * // 写者
 * zen::write_lock<zen::rwlock> wl(rw);
 * // ... 写操作 ...
 * @endcode
 */
#ifndef ZEN_THREADING_SYNC_RWLOCK_H
#define ZEN_THREADING_SYNC_RWLOCK_H

#if defined(_WIN32) || defined(_WIN64)
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#else
#  include <pthread.h>
#endif

namespace zen {

// ============================================================================
// rwlock
// ============================================================================

/**
 * @brief 读写锁
 */
class rwlock {
public:
    rwlock() noexcept {
#if defined(_WIN32) || defined(_WIN64)
        InitializeSRWLock(&lock_);
#else
        pthread_rwlock_init(&lock_, nullptr);
#endif
    }
    
    ~rwlock() noexcept {
#if !defined(_WIN32) && !defined(_WIN64)
        pthread_rwlock_destroy(&lock_);
#endif
        // SRWLock 不需要显式销毁
    }
    
    rwlock(const rwlock&)            = delete;
    rwlock& operator=(const rwlock&) = delete;
    
    // ---- 读锁 ----
    
    /**
     * @brief 获取读锁（阻塞直到无写者）
     */
    void lock_shared() noexcept {
#if defined(_WIN32) || defined(_WIN64)
        AcquireSRWLockShared(&lock_);
#else
        pthread_rwlock_rdlock(&lock_);
#endif
    }
    
    /**
     * @brief 尝试获取读锁（非阻塞）
     */
    bool try_lock_shared() noexcept {
#if defined(_WIN32) || defined(_WIN64)
        return TryAcquireSRWLockShared(&lock_) != 0;
#else
        return pthread_rwlock_tryrdlock(&lock_) == 0;
#endif
    }
    
    /**
     * @brief 释放读锁
     */
    void unlock_shared() noexcept {
#if defined(_WIN32) || defined(_WIN64)
        ReleaseSRWLockShared(&lock_);
#else
        pthread_rwlock_unlock(&lock_);
#endif
    }
    
    // ---- 写锁 ----
    
    /**
     * @brief 获取写锁（阻塞直到无读者和写者）
     */
    void lock() noexcept {
#if defined(_WIN32) || defined(_WIN64)
        AcquireSRWLockExclusive(&lock_);
#else
        pthread_rwlock_wrlock(&lock_);
#endif
    }
    
    /**
     * @brief 尝试获取写锁（非阻塞）
     */
    bool try_lock() noexcept {
#if defined(_WIN32) || defined(_WIN64)
        return TryAcquireSRWLockExclusive(&lock_) != 0;
#else
        return pthread_rwlock_trywrlock(&lock_) == 0;
#endif
    }
    
    /**
     * @brief 释放写锁
     */
    void unlock() noexcept {
#if defined(_WIN32) || defined(_WIN64)
        ReleaseSRWLockExclusive(&lock_);
#else
        pthread_rwlock_unlock(&lock_);
#endif
    }
    
    /**
     * @brief 获取原生句柄
     */
#if defined(_WIN32) || defined(_WIN64)
    using native_handle_type = SRWLOCK*;
#else
    using native_handle_type = pthread_rwlock_t*;
#endif
    
    native_handle_type native_handle() noexcept { return &lock_; }

private:
#if defined(_WIN32) || defined(_WIN64)
    SRWLOCK lock_;
#else
    pthread_rwlock_t lock_;
#endif
};

// shared_mutex 是 rwlock 的别名（STL 风格）
using shared_mutex = rwlock;

// ============================================================================
// shared_lock（读锁 RAII 守卫）
// ============================================================================

/**
 * @brief 读锁 RAII 守卫
 * 
 * 构造时调用 lock_shared()，析构时调用 unlock_shared()。
 */
template<typename SharedMutex>
class shared_lock {
public:
    using mutex_type = SharedMutex;
    
    explicit shared_lock(SharedMutex& m) noexcept : mutex_(m), owns_(true) {
        mutex_.lock_shared();
    }
    
    ~shared_lock() noexcept {
        if (owns_) mutex_.unlock_shared();
    }
    
    shared_lock(const shared_lock&)            = delete;
    shared_lock& operator=(const shared_lock&) = delete;
    
    // 支持移动
    shared_lock(shared_lock&& other) noexcept
        : mutex_(other.mutex_), owns_(other.owns_) {
        other.owns_ = false;
    }
    
    void lock() noexcept {
        if (!owns_) {
            mutex_.lock_shared();
            owns_ = true;
        }
    }
    
    bool try_lock() noexcept {
        if (!owns_) {
            owns_ = mutex_.try_lock_shared();
        }
        return owns_;
    }
    
    void unlock() noexcept {
        if (owns_) {
            mutex_.unlock_shared();
            owns_ = false;
        }
    }
    
    bool owns_lock() const noexcept { return owns_; }
    explicit operator bool() const noexcept { return owns_; }
    SharedMutex* mutex() const noexcept { return &mutex_; }

private:
    SharedMutex& mutex_;
    bool         owns_;
};

// ============================================================================
// write_lock（写锁 RAII 守卫，与 unique_lock 等价）
// ============================================================================

/**
 * @brief 写锁 RAII 守卫
 * 
 * 构造时调用 lock()（独占写锁），析构时调用 unlock()。
 */
template<typename SharedMutex>
class write_lock {
public:
    using mutex_type = SharedMutex;
    
    explicit write_lock(SharedMutex& m) noexcept : mutex_(m), owns_(true) {
        mutex_.lock();
    }
    
    ~write_lock() noexcept {
        if (owns_) mutex_.unlock();
    }
    
    write_lock(const write_lock&)            = delete;
    write_lock& operator=(const write_lock&) = delete;
    
    write_lock(write_lock&& other) noexcept
        : mutex_(other.mutex_), owns_(other.owns_) {
        other.owns_ = false;
    }
    
    void lock() noexcept {
        if (!owns_) {
            mutex_.lock();
            owns_ = true;
        }
    }
    
    bool try_lock() noexcept {
        if (!owns_) {
            owns_ = mutex_.try_lock();
        }
        return owns_;
    }
    
    void unlock() noexcept {
        if (owns_) {
            mutex_.unlock();
            owns_ = false;
        }
    }
    
    bool owns_lock() const noexcept { return owns_; }
    explicit operator bool() const noexcept { return owns_; }

private:
    SharedMutex& mutex_;
    bool         owns_;
};

} // namespace zen

#endif // ZEN_THREADING_SYNC_RWLOCK_H
