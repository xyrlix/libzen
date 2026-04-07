/**
 * @file lock_guard.h
 * @brief RAII 锁守卫
 * 
 * 提供三种 RAII 锁管理器：
 * 
 * - lock_guard   : 最简版，构造时 lock，析构时 unlock，不可转移
 * - unique_lock  : 可延迟加锁、提前解锁、移动语义的灵活锁管理器
 * - scoped_lock  : 同时锁住多个 mutex（C++17 scoped_lock 的手工实现）
 * 
 * 所有 RAII 守卫均不可拷贝。
 */
#ifndef ZEN_THREADING_SYNC_LOCK_GUARD_H
#define ZEN_THREADING_SYNC_LOCK_GUARD_H

namespace zen {

// ============================================================================
// 辅助标签
// ============================================================================

/**
 * @brief 延迟加锁标签（构造时不立即加锁）
 */
struct defer_lock_t  { explicit defer_lock_t() = default; };

/**
 * @brief 已持有锁标签（构造时假设调用方已经 lock 了，不再重复 lock）
 */
struct adopt_lock_t  { explicit adopt_lock_t() = default; };

/**
 * @brief 尝试加锁标签（构造时调用 try_lock，不阻塞）
 */
struct try_to_lock_t { explicit try_to_lock_t() = default; };

inline constexpr defer_lock_t  defer_lock  {};
inline constexpr adopt_lock_t  adopt_lock  {};
inline constexpr try_to_lock_t try_to_lock {};

// ============================================================================
// lock_guard
// ============================================================================

/**
 * @brief 简单 RAII 锁守卫
 * 
 * 构造时立即 lock，析构时自动 unlock。
 * 不支持提前解锁或移动。
 * 
 * 示例：
 * @code
 * zen::mutex m;
 * {
 *     zen::lock_guard<zen::mutex> lg(m);
 *     // 临界区
 * } // 自动解锁
 * @endcode
 */
template<typename Mutex>
class lock_guard {
public:
    using mutex_type = Mutex;
    
    /**
     * @brief 构造并加锁
     */
    explicit lock_guard(Mutex& m) noexcept : mutex_(m) {
        mutex_.lock();
    }
    
    /**
     * @brief 接管已持有的锁（不再重复 lock）
     */
    lock_guard(Mutex& m, adopt_lock_t) noexcept : mutex_(m) {}
    
    /**
     * @brief 析构时自动解锁
     */
    ~lock_guard() noexcept {
        mutex_.unlock();
    }
    
    // 不可拷贝，不可移动
    lock_guard(const lock_guard&)            = delete;
    lock_guard& operator=(const lock_guard&) = delete;

private:
    Mutex& mutex_;
};

// ============================================================================
// unique_lock
// ============================================================================

/**
 * @brief 灵活 RAII 锁管理器
 * 
 * 与 lock_guard 相比，增加了：
 * - 延迟加锁（defer_lock）
 * - try_lock 尝试加锁
 * - 提前 unlock()/lock() 切换
 * - 移动语义（可以转移锁的所有权）
 * 
 * 示例：
 * @code
 * zen::mutex m;
 * zen::unique_lock<zen::mutex> ul(m, zen::defer_lock);
 * // ... 做点别的 ...
 * ul.lock();  // 手动加锁
 * // ... 临界区 ...
 * ul.unlock(); // 提前解锁
 * @endcode
 */
template<typename Mutex>
class unique_lock {
public:
    using mutex_type = Mutex;
    
    /**
     * @brief 默认构造（不持有任何 mutex）
     */
    unique_lock() noexcept : mutex_(nullptr), owns_(false) {}
    
    /**
     * @brief 构造并立即加锁
     */
    explicit unique_lock(Mutex& m) noexcept : mutex_(&m), owns_(false) {
        mutex_->lock();
        owns_ = true;
    }
    
    /**
     * @brief 延迟加锁构造
     */
    unique_lock(Mutex& m, defer_lock_t) noexcept : mutex_(&m), owns_(false) {}
    
    /**
     * @brief 接管已持有的锁
     */
    unique_lock(Mutex& m, adopt_lock_t) noexcept : mutex_(&m), owns_(true) {}
    
    /**
     * @brief 尝试加锁构造
     */
    unique_lock(Mutex& m, try_to_lock_t) noexcept : mutex_(&m), owns_(false) {
        owns_ = mutex_->try_lock();
    }
    
    /**
     * @brief 析构时如果持有锁则解锁
     */
    ~unique_lock() noexcept {
        if (owns_) mutex_->unlock();
    }
    
    // 不可拷贝
    unique_lock(const unique_lock&)            = delete;
    unique_lock& operator=(const unique_lock&) = delete;
    
    /**
     * @brief 移动构造（转移锁的所有权）
     */
    unique_lock(unique_lock&& other) noexcept
        : mutex_(other.mutex_), owns_(other.owns_) {
        other.mutex_ = nullptr;
        other.owns_  = false;
    }
    
    /**
     * @brief 移动赋值
     */
    unique_lock& operator=(unique_lock&& other) noexcept {
        if (this != &other) {
            if (owns_) mutex_->unlock();
            mutex_       = other.mutex_;
            owns_        = other.owns_;
            other.mutex_ = nullptr;
            other.owns_  = false;
        }
        return *this;
    }
    
    // ---- 锁操作 ----
    
    /**
     * @brief 加锁（必须当前未持有锁）
     */
    void lock() noexcept {
        if (mutex_ && !owns_) {
            mutex_->lock();
            owns_ = true;
        }
    }
    
    /**
     * @brief 尝试加锁
     * @return 成功获取返回 true
     */
    bool try_lock() noexcept {
        if (mutex_ && !owns_) {
            owns_ = mutex_->try_lock();
            return owns_;
        }
        return false;
    }
    
    /**
     * @brief 解锁
     */
    void unlock() noexcept {
        if (mutex_ && owns_) {
            mutex_->unlock();
            owns_ = false;
        }
    }
    
    /**
     * @brief 释放 mutex 关联（不解锁，由调用方负责）
     */
    Mutex* release() noexcept {
        Mutex* m = mutex_;
        mutex_ = nullptr;
        owns_  = false;
        return m;
    }
    
    // ---- 状态查询 ----
    
    Mutex* mutex() const noexcept { return mutex_; }
    bool   owns_lock() const noexcept { return owns_; }
    explicit operator bool() const noexcept { return owns_; }

private:
    Mutex* mutex_;
    bool   owns_;
};

// ============================================================================
// scoped_lock（锁住多个 mutex，避免死锁）
// ============================================================================

/**
 * @brief 同时锁住多个 mutex 的 RAII 守卫
 * 
 * 使用按地址排序（lock ordering）避免死锁。
 * 
 * 示例：
 * @code
 * zen::mutex m1, m2;
 * zen::scoped_lock<zen::mutex, zen::mutex> sl(m1, m2);
 * // m1, m2 均已锁定，析构时自动解锁
 * @endcode
 */
template<typename... Mutexes>
class scoped_lock;

// 零个 mutex 的特化
template<>
class scoped_lock<> {
public:
    explicit scoped_lock() noexcept = default;
    ~scoped_lock() noexcept = default;
    scoped_lock(const scoped_lock&)            = delete;
    scoped_lock& operator=(const scoped_lock&) = delete;
};

// 单个 mutex 的特化（等价于 lock_guard）
template<typename Mutex>
class scoped_lock<Mutex> {
public:
    using mutex_type = Mutex;
    
    explicit scoped_lock(Mutex& m) noexcept : mutex_(m) {
        mutex_.lock();
    }
    
    scoped_lock(Mutex& m, adopt_lock_t) noexcept : mutex_(m) {}
    
    ~scoped_lock() noexcept {
        mutex_.unlock();
    }
    
    scoped_lock(const scoped_lock&)            = delete;
    scoped_lock& operator=(const scoped_lock&) = delete;

private:
    Mutex& mutex_;
};

// 两个 mutex 的特化（按指针地址排序加锁，避免死锁）
template<typename Mutex1, typename Mutex2>
class scoped_lock<Mutex1, Mutex2> {
public:
    scoped_lock(Mutex1& m1, Mutex2& m2) noexcept : m1_(m1), m2_(m2) {
        // 按指针地址排序，低地址先加锁，避免死锁
        void* p1 = static_cast<void*>(&m1);
        void* p2 = static_cast<void*>(&m2);
        if (p1 < p2) {
            m1_.lock();
            m2_.lock();
        } else {
            m2_.lock();
            m1_.lock();
        }
    }
    
    ~scoped_lock() noexcept {
        m1_.unlock();
        m2_.unlock();
    }
    
    scoped_lock(const scoped_lock&)            = delete;
    scoped_lock& operator=(const scoped_lock&) = delete;

private:
    Mutex1& m1_;
    Mutex2& m2_;
};

} // namespace zen

#endif // ZEN_THREADING_SYNC_LOCK_GUARD_H
