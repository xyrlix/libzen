/**
 * @file spinlock.h
 * @brief 自旋锁实现
 * 
 * 基于原子操作的自旋锁，适合锁持有时间极短的场景：
 * - spinlock    : 基于 CAS（Compare-And-Swap）的自旋锁
 * - ticket_lock : 基于 ticket 算法的公平自旋锁（先到先得）
 * 
 * 设计原则：
 * - 无系统调用，开销极低
 * - 自旋等待（busy-wait），适合微秒级临界区
 * - 不适合长时间持有锁的场景（会浪费 CPU）
 * 
 * 编译器 barrier：使用 __asm__ volatile("" ::: "memory") 阻止指令重排
 */
#ifndef ZEN_THREADING_SYNC_SPINLOCK_H
#define ZEN_THREADING_SYNC_SPINLOCK_H

#include <cstdint>

namespace zen {

// ============================================================================
// 原子操作原语（手工封装，不依赖 <atomic>）
// ============================================================================

namespace detail {

/**
 * @brief 编译器内存屏障（阻止编译器重排序）
 */
inline void compiler_barrier() noexcept {
#if defined(__GNUC__) || defined(__clang__)
    __asm__ volatile("" ::: "memory");
#elif defined(_MSC_VER)
    _ReadWriteBarrier();
#endif
}

/**
 * @brief CPU 内存屏障（全屏障）
 */
inline void memory_fence() noexcept {
#if defined(__GNUC__) || defined(__clang__)
#  if defined(__x86_64__) || defined(__i386__)
    __asm__ volatile("mfence" ::: "memory");
#  elif defined(__aarch64__) || defined(__arm__)
    __asm__ volatile("dmb ish" ::: "memory");
#  else
    __sync_synchronize();
#  endif
#elif defined(_MSC_VER)
    MemoryBarrier();
#endif
}

/**
 * @brief CPU 暂停指令（让出时间片，减少功耗）
 */
inline void cpu_relax() noexcept {
#if defined(__x86_64__) || defined(__i386__)
    __asm__ volatile("pause" ::: "memory");
#elif defined(__aarch64__)
    __asm__ volatile("yield" ::: "memory");
#elif defined(__arm__)
    __asm__ volatile("yield" ::: "memory");
#else
    // 其他平台：什么都不做
#endif
}

/**
 * @brief CAS：比较并交换（返回旧值是否等于期望值）
 * @param ptr      目标地址
 * @param expected 期望旧值
 * @param desired  新值
 * @return 交换成功（旧值 == expected）返回 true
 */
inline bool cas(volatile int* ptr, int expected, int desired) noexcept {
#if defined(__GNUC__) || defined(__clang__)
    return __sync_bool_compare_and_swap(ptr, expected, desired);
#elif defined(_MSC_VER)
    return InterlockedCompareExchange(reinterpret_cast<volatile LONG*>(ptr),
                                      desired, expected) == expected;
#else
    // fallback：不安全，仅用于单线程测试
    if (*ptr == expected) { *ptr = desired; return true; }
    return false;
#endif
}

/**
 * @brief 原子加法（返回旧值）
 */
inline int atomic_fetch_add(volatile int* ptr, int val) noexcept {
#if defined(__GNUC__) || defined(__clang__)
    return __sync_fetch_and_add(ptr, val);
#elif defined(_MSC_VER)
    return InterlockedExchangeAdd(reinterpret_cast<volatile LONG*>(ptr), val);
#else
    int old = *ptr;
    *ptr += val;
    return old;
#endif
}

/**
 * @brief 原子读取
 */
inline int atomic_load(const volatile int* ptr) noexcept {
    compiler_barrier();
    int val = *ptr;
    compiler_barrier();
    return val;
}

/**
 * @brief 原子写入
 */
inline void atomic_store(volatile int* ptr, int val) noexcept {
    compiler_barrier();
    *ptr = val;
    compiler_barrier();
}

} // namespace detail

// ============================================================================
// spinlock
// ============================================================================

/**
 * @brief TAS（Test-And-Set）自旋锁
 * 
 * 满足 Mutex 概念（BasicLockable），可与 lock_guard/unique_lock 配合使用。
 * 
 * 内部使用 CAS 原子操作：
 * - locked_ == 0 表示空闲
 * - locked_ == 1 表示被持有
 * 
 * 注意：非公平锁，在高竞争下可能出现饥饿。
 * 若需要公平性，使用 ticket_lock。
 * 
 * 示例：
 * @code
 * zen::spinlock sl;
 * zen::lock_guard<zen::spinlock> lg(sl);
 * // 临界区（建议极短）
 * @endcode
 */
class spinlock {
public:
    spinlock() noexcept : locked_(0) {}
    
    // 不可拷贝
    spinlock(const spinlock&)            = delete;
    spinlock& operator=(const spinlock&) = delete;
    
    /**
     * @brief 自旋等待直到获取锁
     * 
     * 先使用 load 检查（快路径），减少对总线的占用（TTAS 优化）。
     */
    void lock() noexcept {
        while (true) {
            // 快速路径：如果锁空闲，尝试 CAS 获取
            if (detail::atomic_load(&locked_) == 0 &&
                detail::cas(&locked_, 0, 1)) {
                return;
            }
            // 锁被占用，自旋等待
            while (detail::atomic_load(&locked_) != 0) {
                detail::cpu_relax();
            }
        }
    }
    
    /**
     * @brief 非阻塞尝试获取锁
     * @return 成功获取返回 true，已被占用返回 false
     */
    bool try_lock() noexcept {
        return detail::atomic_load(&locked_) == 0 &&
               detail::cas(&locked_, 0, 1);
    }
    
    /**
     * @brief 释放锁
     */
    void unlock() noexcept {
        detail::memory_fence();
        detail::atomic_store(&locked_, 0);
    }
    
    /**
     * @brief 是否被锁定（非原子，仅供诊断）
     */
    bool is_locked() const noexcept {
        return detail::atomic_load(&locked_) != 0;
    }

private:
    volatile int locked_;
    // 填充到 64 字节，避免 false sharing
    char padding_[64 - sizeof(int)];
};

// ============================================================================
// ticket_lock（公平自旋锁）
// ============================================================================

/**
 * @brief Ticket 自旋锁（先到先得，公平性保证）
 * 
 * 算法：
 * - 每个等待线程原子地获取一个 ticket（编号）
 * - 当 serving == ticket 时，该线程获得锁
 * - unlock 时 serving++，通知下一个等待者
 * 
 * 优点：严格 FIFO，无饥饿
 * 缺点：在多核高竞争下，所有等待者都在轮询 serving，
 *        可能导致缓存行 bouncing（MCS Lock 可解决此问题）
 */
class ticket_lock {
public:
    ticket_lock() noexcept : next_ticket_(0), serving_(0) {}
    
    ticket_lock(const ticket_lock&)            = delete;
    ticket_lock& operator=(const ticket_lock&) = delete;
    
    /**
     * @brief 获取 ticket 并自旋等待轮到自己
     */
    void lock() noexcept {
        // 原子获取 ticket
        int my_ticket = detail::atomic_fetch_add(&next_ticket_, 1);
        
        // 等待 serving 达到我的 ticket
        while (detail::atomic_load(&serving_) != my_ticket) {
            detail::cpu_relax();
        }
    }
    
    /**
     * @brief 尝试获取锁（当且仅当没有其他等待者时成功）
     */
    bool try_lock() noexcept {
        int ticket  = detail::atomic_load(&next_ticket_);
        int serving = detail::atomic_load(&serving_);
        if (ticket == serving) {
            return detail::cas(&next_ticket_, ticket, ticket + 1);
        }
        return false;
    }
    
    /**
     * @brief 释放锁，通知下一个等待者
     */
    void unlock() noexcept {
        detail::memory_fence();
        detail::atomic_fetch_add(&serving_, 1);
    }

private:
    volatile int next_ticket_;
    volatile int serving_;
    char padding_[64 - 2 * sizeof(int)];
};

} // namespace zen

#endif // ZEN_THREADING_SYNC_SPINLOCK_H
