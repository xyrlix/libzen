/**
 * @file mutex.h
 * @brief 互斥锁实现
 * 
 * 基于 POSIX pthread（Linux/macOS）或 Win32 CRITICAL_SECTION/SRWLock（Windows）
 * 提供与 std::mutex 兼容的接口：
 * 
 * - mutex          : 普通互斥锁（非递归）
 * - recursive_mutex: 递归互斥锁
 * - timed_mutex    : 带超时的互斥锁
 * 
 * 遵守 BasicLockable 概念（提供 lock() / unlock()）。
 */
#ifndef ZEN_THREADING_SYNC_MUTEX_H
#define ZEN_THREADING_SYNC_MUTEX_H

#include <cstddef>

// 平台检测
#if defined(_WIN32) || defined(_WIN64)
#  define ZEN_OS_WINDOWS 1
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#else
#  define ZEN_OS_POSIX 1
#  include <pthread.h>
#  include <errno.h>
#  include <time.h>
#endif

namespace zen {

// ============================================================================
// mutex
// ============================================================================

/**
 * @brief 非递归互斥锁
 * 
 * 满足 Mutex 要求：
 *   - lock()    : 阻塞获取锁
 *   - unlock()  : 释放锁
 *   - try_lock(): 非阻塞尝试获取（成功返回 true）
 * 
 * 不可拷贝，不可移动。
 */
class mutex {
public:
    mutex() noexcept {
#ifdef ZEN_OS_WINDOWS
        InitializeCriticalSection(&cs_);
#else
        pthread_mutex_init(&mtx_, nullptr);
#endif
    }
    
    ~mutex() noexcept {
#ifdef ZEN_OS_WINDOWS
        DeleteCriticalSection(&cs_);
#else
        pthread_mutex_destroy(&mtx_);
#endif
    }
    
    // 不可拷贝、不可移动
    mutex(const mutex&)            = delete;
    mutex& operator=(const mutex&) = delete;
    mutex(mutex&&)                 = delete;
    mutex& operator=(mutex&&)      = delete;
    
    /**
     * @brief 阻塞获取锁
     */
    void lock() noexcept {
#ifdef ZEN_OS_WINDOWS
        EnterCriticalSection(&cs_);
#else
        pthread_mutex_lock(&mtx_);
#endif
    }
    
    /**
     * @brief 非阻塞尝试获取锁
     * @return 成功获取返回 true，锁已被占用返回 false
     */
    bool try_lock() noexcept {
#ifdef ZEN_OS_WINDOWS
        return TryEnterCriticalSection(&cs_) != 0;
#else
        return pthread_mutex_trylock(&mtx_) == 0;
#endif
    }
    
    /**
     * @brief 释放锁
     */
    void unlock() noexcept {
#ifdef ZEN_OS_WINDOWS
        LeaveCriticalSection(&cs_);
#else
        pthread_mutex_unlock(&mtx_);
#endif
    }
    
    /**
     * @brief 获取原生句柄（用于与系统 API 互操作）
     */
#ifdef ZEN_OS_WINDOWS
    using native_handle_type = CRITICAL_SECTION*;
#else
    using native_handle_type = pthread_mutex_t*;
#endif
    
    native_handle_type native_handle() noexcept {
#ifdef ZEN_OS_WINDOWS
        return &cs_;
#else
        return &mtx_;
#endif
    }

private:
#ifdef ZEN_OS_WINDOWS
    CRITICAL_SECTION cs_;
#else
    pthread_mutex_t mtx_;
#endif
};

// ============================================================================
// recursive_mutex
// ============================================================================

/**
 * @brief 递归互斥锁
 * 
 * 同一线程可以多次 lock()，对应 unlock() 次数必须相同。
 */
class recursive_mutex {
public:
    recursive_mutex() noexcept {
#ifdef ZEN_OS_WINDOWS
        InitializeCriticalSection(&cs_);
#else
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&mtx_, &attr);
        pthread_mutexattr_destroy(&attr);
#endif
    }
    
    ~recursive_mutex() noexcept {
#ifdef ZEN_OS_WINDOWS
        DeleteCriticalSection(&cs_);
#else
        pthread_mutex_destroy(&mtx_);
#endif
    }
    
    recursive_mutex(const recursive_mutex&)            = delete;
    recursive_mutex& operator=(const recursive_mutex&) = delete;
    
    void lock() noexcept {
#ifdef ZEN_OS_WINDOWS
        EnterCriticalSection(&cs_);
#else
        pthread_mutex_lock(&mtx_);
#endif
    }
    
    bool try_lock() noexcept {
#ifdef ZEN_OS_WINDOWS
        return TryEnterCriticalSection(&cs_) != 0;
#else
        return pthread_mutex_trylock(&mtx_) == 0;
#endif
    }
    
    void unlock() noexcept {
#ifdef ZEN_OS_WINDOWS
        LeaveCriticalSection(&cs_);
#else
        pthread_mutex_unlock(&mtx_);
#endif
    }

private:
#ifdef ZEN_OS_WINDOWS
    CRITICAL_SECTION cs_;
#else
    pthread_mutex_t mtx_;
#endif
};

// ============================================================================
// timed_mutex
// ============================================================================

/**
 * @brief 带超时功能的互斥锁
 * 
 * 额外提供：
 *   - try_lock_for(milliseconds)  : 最多等待指定毫秒数
 *   - try_lock_until(deadline_ms) : 等待到指定时刻（毫秒时间戳）
 */
class timed_mutex {
public:
    timed_mutex() noexcept {
#ifdef ZEN_OS_WINDOWS
        InitializeCriticalSection(&cs_);
#else
        pthread_mutex_init(&mtx_, nullptr);
#endif
    }
    
    ~timed_mutex() noexcept {
#ifdef ZEN_OS_WINDOWS
        DeleteCriticalSection(&cs_);
#else
        pthread_mutex_destroy(&mtx_);
#endif
    }
    
    timed_mutex(const timed_mutex&)            = delete;
    timed_mutex& operator=(const timed_mutex&) = delete;
    
    void lock() noexcept {
#ifdef ZEN_OS_WINDOWS
        EnterCriticalSection(&cs_);
#else
        pthread_mutex_lock(&mtx_);
#endif
    }
    
    bool try_lock() noexcept {
#ifdef ZEN_OS_WINDOWS
        return TryEnterCriticalSection(&cs_) != 0;
#else
        return pthread_mutex_trylock(&mtx_) == 0;
#endif
    }
    
    void unlock() noexcept {
#ifdef ZEN_OS_WINDOWS
        LeaveCriticalSection(&cs_);
#else
        pthread_mutex_unlock(&mtx_);
#endif
    }
    
    /**
     * @brief 尝试在指定毫秒内获取锁
     * @param timeout_ms 超时时间（毫秒）
     * @return 成功获取返回 true
     */
    bool try_lock_for(unsigned long timeout_ms) noexcept {
#ifdef ZEN_OS_WINDOWS
        // Windows: 用自旋等待模拟（简化）
        DWORD deadline = GetTickCount() + static_cast<DWORD>(timeout_ms);
        while (!TryEnterCriticalSection(&cs_)) {
            if (GetTickCount() >= deadline) return false;
            Sleep(1);
        }
        return true;
#else
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec  += timeout_ms / 1000;
        ts.tv_nsec += (timeout_ms % 1000) * 1000000L;
        if (ts.tv_nsec >= 1000000000L) {
            ++ts.tv_sec;
            ts.tv_nsec -= 1000000000L;
        }
        return pthread_mutex_timedlock(&mtx_, &ts) == 0;
#endif
    }

private:
#ifdef ZEN_OS_WINDOWS
    CRITICAL_SECTION cs_;
#else
    pthread_mutex_t mtx_;
#endif
};

} // namespace zen

#endif // ZEN_THREADING_SYNC_MUTEX_H
