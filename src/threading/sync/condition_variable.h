/**
 * @file condition_variable.h
 * @brief 条件变量实现
 * 
 * condition_variable 用于线程间的等待/通知机制：
 * - 等待线程调用 wait()，原子性地释放 mutex 并挂起
 * - 通知线程调用 notify_one()/notify_all() 唤醒等待者
 * - 等待者被唤醒后重新获取 mutex，再检查条件
 * 
 * 与 std::condition_variable 接口兼容。
 * 
 * 典型用法（生产者-消费者）：
 * @code
 * zen::mutex m;
 * zen::condition_variable cv;
 * bool ready = false;
 * 
 * // 消费者
 * zen::unique_lock<zen::mutex> ul(m);
 * cv.wait(ul, [&]{ return ready; });
 * 
 * // 生产者
 * {
 *     zen::lock_guard<zen::mutex> lg(m);
 *     ready = true;
 * }
 * cv.notify_one();
 * @endcode
 */
#ifndef ZEN_THREADING_SYNC_CONDITION_VARIABLE_H
#define ZEN_THREADING_SYNC_CONDITION_VARIABLE_H

#include "mutex.h"
#include "lock_guard.h"

namespace zen {

// ============================================================================
// condition_variable
// ============================================================================

/**
 * @brief 条件变量（与 mutex 配合使用）
 */
class condition_variable {
public:
    condition_variable() noexcept {
#ifdef ZEN_OS_WINDOWS
        InitializeConditionVariable(&cv_);
#else
        pthread_cond_init(&cond_, nullptr);
#endif
    }
    
    ~condition_variable() noexcept {
#ifdef ZEN_OS_POSIX
        pthread_cond_destroy(&cond_);
#endif
        // Windows 的 CONDITION_VARIABLE 不需要显式销毁
    }
    
    condition_variable(const condition_variable&)            = delete;
    condition_variable& operator=(const condition_variable&) = delete;
    
    // ---- 通知 ----
    
    /**
     * @brief 唤醒一个等待线程
     */
    void notify_one() noexcept {
#ifdef ZEN_OS_WINDOWS
        WakeConditionVariable(&cv_);
#else
        pthread_cond_signal(&cond_);
#endif
    }
    
    /**
     * @brief 唤醒所有等待线程
     */
    void notify_all() noexcept {
#ifdef ZEN_OS_WINDOWS
        WakeAllConditionVariable(&cv_);
#else
        pthread_cond_broadcast(&cond_);
#endif
    }
    
    // ---- 等待 ----
    
    /**
     * @brief 等待通知（无条件等待）
     * 
     * 原子性地：
     * 1. 释放 lock 持有的 mutex
     * 2. 挂起当前线程
     * 3. 被唤醒后重新获取 mutex
     * 
     * @param lock 持有关联 mutex 的 unique_lock
     */
    void wait(unique_lock<mutex>& lock) noexcept {
#ifdef ZEN_OS_WINDOWS
        SleepConditionVariableCS(&cv_, lock.mutex()->native_handle(), INFINITE);
#else
        pthread_cond_wait(&cond_, lock.mutex()->native_handle());
#endif
    }
    
    /**
     * @brief 带谓词的等待（防止虚假唤醒）
     * 
     * 等价于：while (!pred()) wait(lock);
     * 
     * @param lock  持有关联 mutex 的 unique_lock
     * @param pred  唤醒条件谓词（返回 true 时结束等待）
     */
    template<typename Predicate>
    void wait(unique_lock<mutex>& lock, Predicate pred) noexcept {
        while (!pred()) {
            wait(lock);
        }
    }
    
    /**
     * @brief 超时等待
     * 
     * @param lock       持有关联 mutex 的 unique_lock
     * @param timeout_ms 超时时间（毫秒）
     * @return 条件被触发返回 true，超时返回 false
     */
    bool wait_for(unique_lock<mutex>& lock, unsigned long timeout_ms) noexcept {
#ifdef ZEN_OS_WINDOWS
        return SleepConditionVariableCS(&cv_,
                                        lock.mutex()->native_handle(),
                                        static_cast<DWORD>(timeout_ms)) != 0;
#else
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec  += timeout_ms / 1000;
        ts.tv_nsec += (timeout_ms % 1000) * 1000000L;
        if (ts.tv_nsec >= 1000000000L) {
            ++ts.tv_sec;
            ts.tv_nsec -= 1000000000L;
        }
        return pthread_cond_timedwait(&cond_, lock.mutex()->native_handle(), &ts) == 0;
#endif
    }
    
    /**
     * @brief 带谓词的超时等待
     * @return 条件满足返回 true，超时返回 false
     */
    template<typename Predicate>
    bool wait_for(unique_lock<mutex>& lock, unsigned long timeout_ms, Predicate pred) noexcept {
        while (!pred()) {
            if (!wait_for(lock, timeout_ms)) {
                return pred(); // 超时时再检查一次条件
            }
        }
        return true;
    }
    
    /**
     * @brief 获取原生句柄
     */
#ifdef ZEN_OS_WINDOWS
    using native_handle_type = CONDITION_VARIABLE*;
#else
    using native_handle_type = pthread_cond_t*;
#endif
    
    native_handle_type native_handle() noexcept {
#ifdef ZEN_OS_WINDOWS
        return &cv_;
#else
        return &cond_;
#endif
    }

private:
#ifdef ZEN_OS_WINDOWS
    CONDITION_VARIABLE cv_;
#else
    pthread_cond_t cond_;
#endif
};

} // namespace zen

#endif // ZEN_THREADING_SYNC_CONDITION_VARIABLE_H
