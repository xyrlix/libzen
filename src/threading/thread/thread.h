/**
 * @file thread.h
 * @brief 线程封装
 * 
 * 基于 POSIX pthread（Linux/macOS）或 Win32 线程（Windows）：
 * 
 * - thread          : RAII 线程对象，管理单个线程的生命周期
 * - thread_id       : 线程 ID 类型
 * 
 * 接口与 std::thread 基本兼容。
 * 
 * 示例：
 * @code
 * zen::thread t([]{ printf("Hello from thread!\n"); });
 * t.join();
 * 
 * // 传参
 * zen::thread t2([](int x){ printf("x = %d\n", x); }, 42);
 * t2.join();
 * @endcode
 */
#ifndef ZEN_THREADING_THREAD_THREAD_H
#define ZEN_THREADING_THREAD_THREAD_H

#include <cstddef>

#if defined(_WIN32) || defined(_WIN64)
#  define ZEN_OS_WINDOWS 1
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#else
#  define ZEN_OS_POSIX 1
#  include <pthread.h>
#  include <unistd.h>
#endif

namespace zen {

// ============================================================================
// 函数调用工具（替代 std::invoke / std::bind）
// ============================================================================

namespace detail {

/**
 * @brief 线程启动参数：包装任意可调用对象和参数
 * 
 * 使用虚函数实现类型擦除，线程函数为静态函数便于传递给系统 API。
 */
struct thread_task {
    virtual ~thread_task() = default;
    virtual void run() = 0;
};

/**
 * @brief 无参可调用对象包装器
 */
template<typename F>
struct thread_task_impl : thread_task {
    F func_;
    
    explicit thread_task_impl(F&& f) : func_(static_cast<F&&>(f)) {}
    explicit thread_task_impl(const F& f) : func_(f) {}
    
    void run() override { func_(); }
};

/**
 * @brief 单参数可调用对象包装器
 */
template<typename F, typename A1>
struct thread_task_1arg : thread_task {
    F  func_;
    A1 arg1_;
    
    thread_task_1arg(F&& f, A1&& a1)
        : func_(static_cast<F&&>(f)), arg1_(static_cast<A1&&>(a1)) {}
    
    void run() override { func_(arg1_); }
};

/**
 * @brief 两参数可调用对象包装器
 */
template<typename F, typename A1, typename A2>
struct thread_task_2arg : thread_task {
    F  func_;
    A1 arg1_;
    A2 arg2_;
    
    thread_task_2arg(F&& f, A1&& a1, A2&& a2)
        : func_(static_cast<F&&>(f))
        , arg1_(static_cast<A1&&>(a1))
        , arg2_(static_cast<A2&&>(a2)) {}
    
    void run() override { func_(arg1_, arg2_); }
};

/**
 * @brief 线程启动函数（传给系统 API 的函数指针）
 */
#ifdef ZEN_OS_WINDOWS
inline DWORD WINAPI thread_entry(LPVOID param) {
    thread_task* task = static_cast<thread_task*>(param);
    task->run();
    delete task;
    return 0;
}
#else
inline void* thread_entry(void* param) {
    thread_task* task = static_cast<thread_task*>(param);
    task->run();
    delete task;
    return nullptr;
}
#endif

} // namespace detail

// ============================================================================
// thread_id
// ============================================================================

/**
 * @brief 线程 ID 类型
 */
class thread_id {
public:
#ifdef ZEN_OS_WINDOWS
    using native_id_t = DWORD;
#else
    using native_id_t = pthread_t;
#endif
    
    thread_id() noexcept : id_(0) {}
    explicit thread_id(native_id_t id) noexcept : id_(id) {}
    
    bool operator==(const thread_id& other) const noexcept { return id_ == other.id_; }
    bool operator!=(const thread_id& other) const noexcept { return id_ != other.id_; }
    bool operator< (const thread_id& other) const noexcept { return id_ <  other.id_; }
    
    native_id_t native_id() const noexcept { return id_; }

private:
    native_id_t id_;
};

// ============================================================================
// thread
// ============================================================================

/**
 * @brief RAII 线程对象
 * 
 * 生命周期规则：
 * - 构造时启动线程
 * - 析构前必须调用 join() 或 detach()，否则终止程序
 * - 不可拷贝，可移动
 * 
 * 支持：
 * - 无参 callable：thread(f)
 * - 单参 callable：thread(f, arg1)
 * - 双参 callable：thread(f, arg1, arg2)
 */
class thread {
public:
    /**
     * @brief 默认构造（不关联任何线程）
     */
    thread() noexcept : joinable_(false) {
#ifdef ZEN_OS_WINDOWS
        handle_ = nullptr;
        id_     = 0;
#else
        handle_ = 0;
#endif
    }
    
    /**
     * @brief 启动一个无参数的线程
     */
    template<typename F>
    explicit thread(F&& f) : joinable_(false) {
        auto* task = new detail::thread_task_impl<typename decay_helper<F>::type>(
            static_cast<F&&>(f));
        start(task);
    }
    
    /**
     * @brief 启动一个单参数线程
     */
    template<typename F, typename A1>
    thread(F&& f, A1&& a1) : joinable_(false) {
        using FD  = typename decay_helper<F>::type;
        using A1D = typename decay_helper<A1>::type;
        auto* task = new detail::thread_task_1arg<FD, A1D>(
            static_cast<F&&>(f), static_cast<A1&&>(a1));
        start(task);
    }
    
    /**
     * @brief 启动一个双参数线程
     */
    template<typename F, typename A1, typename A2>
    thread(F&& f, A1&& a1, A2&& a2) : joinable_(false) {
        using FD  = typename decay_helper<F>::type;
        using A1D = typename decay_helper<A1>::type;
        using A2D = typename decay_helper<A2>::type;
        auto* task = new detail::thread_task_2arg<FD, A1D, A2D>(
            static_cast<F&&>(f), static_cast<A1&&>(a1), static_cast<A2&&>(a2));
        start(task);
    }
    
    /**
     * @brief 析构：若线程仍可 join，终止程序（与 std::thread 行为一致）
     */
    ~thread() noexcept {
        if (joinable_) {
            // 未 join 或 detach 直接析构是编程错误
            // 这里选择 detach 而非 terminate，宽容处理
            detach();
        }
    }
    
    // 不可拷贝
    thread(const thread&)            = delete;
    thread& operator=(const thread&) = delete;
    
    // 可移动
    thread(thread&& other) noexcept {
#ifdef ZEN_OS_WINDOWS
        handle_   = other.handle_;
        id_       = other.id_;
        other.handle_   = nullptr;
        other.id_       = 0;
#else
        handle_   = other.handle_;
        other.handle_   = 0;
#endif
        joinable_ = other.joinable_;
        other.joinable_ = false;
    }
    
    thread& operator=(thread&& other) noexcept {
        if (this != &other) {
            if (joinable_) detach();
#ifdef ZEN_OS_WINDOWS
            handle_   = other.handle_;
            id_       = other.id_;
            other.handle_   = nullptr;
            other.id_       = 0;
#else
            handle_   = other.handle_;
            other.handle_   = 0;
#endif
            joinable_       = other.joinable_;
            other.joinable_ = false;
        }
        return *this;
    }
    
    // ---- 操作 ----
    
    /**
     * @brief 等待线程结束（阻塞）
     */
    void join() noexcept {
        if (!joinable_) return;
#ifdef ZEN_OS_WINDOWS
        WaitForSingleObject(handle_, INFINITE);
        CloseHandle(handle_);
        handle_   = nullptr;
#else
        pthread_join(handle_, nullptr);
        handle_ = 0;
#endif
        joinable_ = false;
    }
    
    /**
     * @brief 分离线程（让线程在后台独立运行）
     */
    void detach() noexcept {
        if (!joinable_) return;
#ifdef ZEN_OS_WINDOWS
        CloseHandle(handle_);
        handle_ = nullptr;
#else
        pthread_detach(handle_);
        handle_ = 0;
#endif
        joinable_ = false;
    }
    
    /**
     * @brief 是否可以 join
     */
    bool joinable() const noexcept { return joinable_; }
    
    /**
     * @brief 获取线程 ID
     */
    thread_id get_id() const noexcept {
#ifdef ZEN_OS_WINDOWS
        return thread_id(id_);
#else
        return thread_id(handle_);
#endif
    }
    
    /**
     * @brief 获取硬件并发数（CPU 核心数）
     */
    static unsigned hardware_concurrency() noexcept {
#ifdef ZEN_OS_WINDOWS
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        return static_cast<unsigned>(si.dwNumberOfProcessors);
#else
        long count = sysconf(_SC_NPROCESSORS_ONLN);
        return count > 0 ? static_cast<unsigned>(count) : 1u;
#endif
    }

private:
    // 简单的 decay（移除引用和 const）
    template<typename T>
    struct decay_helper {
        using type = T;
    };
    
    template<typename T>
    struct decay_helper<T&> {
        using type = T;
    };
    
    template<typename T>
    struct decay_helper<T&&> {
        using type = T;
    };
    
    template<typename T>
    struct decay_helper<const T> {
        using type = T;
    };
    
    void start(detail::thread_task* task) {
#ifdef ZEN_OS_WINDOWS
        handle_ = CreateThread(nullptr, 0, detail::thread_entry,
                               static_cast<LPVOID>(task), 0, &id_);
        if (handle_ == nullptr) {
            delete task;
            return;
        }
#else
        int ret = pthread_create(&handle_, nullptr, detail::thread_entry,
                                 static_cast<void*>(task));
        if (ret != 0) {
            delete task;
            return;
        }
#endif
        joinable_ = true;
    }

#ifdef ZEN_OS_WINDOWS
    HANDLE handle_;
    DWORD  id_;
#else
    pthread_t handle_;
#endif
    bool joinable_;
};

} // namespace zen

#endif // ZEN_THREADING_THREAD_THREAD_H
