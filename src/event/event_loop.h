/**
 * @file event_loop.h
 * @brief 统一事件循环（epoll/timer/signal）
 * 
 * event_loop 提供统一的事件驱动框架，支持：
 * - IO 事件（epoll）：可读、可写、错误事件
 * - 定时器事件：基于定时器队列
 * - 信号事件：信号处理
 * 
 * 特性：
 * - 单线程运行，避免锁竞争
 * - 高性能：使用 epoll 边缘触发
 * - 线程安全：其他线程可通过异步唤醒
 * 
 * 示例：
 * @code
 * zen::event_loop loop;
 * 
 * // 注册 IO 事件
 * int fd = socket(...);
 * loop.add_io_event(fd, ZEN_EVENT_READ, [](int events){
 *     if (events & ZEN_EVENT_READ) {
 *         // 读取数据
 *     }
 * });
 * 
 * // 添加定时器
 * loop.add_timer(1000, []{
 *     printf("1 second passed\n");
 * });
 * 
 * // 运行事件循环
 * loop.run();
 * @endcode
 */
#ifndef ZEN_EVENT_EVENT_LOOP_H
#define ZEN_EVENT_EVENT_LOOP_H

#include "../threading/sync/mutex.h"
#include "../threading/sync/condition_variable.h"
#include "../timer/timer_manager.h"
#include "../utility/function.h"
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <vector>
#include <map>

namespace zen {

// ============================================================================
// 事件类型
// ============================================================================

constexpr uint32_t ZEN_EVENT_READ  = EPOLLIN;   // 可读
constexpr uint32_t ZEN_EVENT_WRITE = EPOLLOUT;  // 可写
constexpr uint32_t ZEN_EVENT_ERROR = EPOLLERR;  // 错误
constexpr uint32_t ZEN_EVENT_HUP   = EPOLLHUP;  // 挂起

// ============================================================================
// event_loop
// ============================================================================

/**
 * @brief 统一事件循环
 */
class event_loop {
public:
    using io_callback = function<void(int, uint32_t)>;
    using timer_callback = function<void()>;
    using signal_callback = function<void(int)>;

    /**
     * @brief 构造
     */
    event_loop();
    
    /**
     * @brief 析构
     */
    ~event_loop();
    
    // 不可拷贝、不可移动
    event_loop(const event_loop&)            = delete;
    event_loop& operator=(const event_loop&) = delete;
    event_loop(event_loop&&)                 = delete;
    event_loop& operator=(event_loop&&)      = delete;
    
    /**
     * @brief 运行事件循环（阻塞）
     */
    void run();
    
    /**
     * @brief 退出事件循环
     */
    void stop();
    
    // ----------------------------------------------------------------
    // IO 事件
    // ----------------------------------------------------------------
    
    /**
     * @brief 添加/修改 IO 事件监听
     * @param fd 文件描述符
     * @param events 事件掩码（ZEN_EVENT_READ/WRITE/ERROR）
     * @param callback 回调函数（参数：fd, 实际触发的事件）
     * @return true 成功，false 失败
     */
    bool add_io_event(int fd, uint32_t events, io_callback callback);
    
    /**
     * @brief 修改 IO 事件监听
     */
    bool modify_io_event(int fd, uint32_t events, io_callback callback);
    
    /**
     * @brief 移除 IO 事件监听
     */
    bool remove_io_event(int fd);
    
    // ----------------------------------------------------------------
    // 定时器事件
    // ----------------------------------------------------------------
    
    /**
     * @brief 添加一次性定时器
     * @param delay_ms 延迟毫秒数
     * @param callback 回调函数
     * @return timer_id，失败返回 0
     */
    timer_id add_timer(unsigned long long delay_ms, timer_callback callback);
    
    /**
     * @brief 添加重复定时器
     * @param interval_ms 间隔毫秒数
     * @param callback 回调函数
     * @return timer_id，失败返回 0
     */
    timer_id add_repeat_timer(unsigned long long interval_ms, timer_callback callback);
    
    /**
     * @brief 取消定时器
     */
    bool cancel_timer(timer_id id);
    
    // ----------------------------------------------------------------
    // 信号事件
    // ----------------------------------------------------------------
    
    /**
     * @brief 添加信号监听
     * @param signum 信号编号
     * @param callback 回调函数（参数：信号编号）
     * @return true 成功，false 失败
     */
    bool add_signal(int signum, signal_callback callback);
    
    /**
     * @brief 移除信号监听
     */
    bool remove_signal(int signum);
    
    // ----------------------------------------------------------------
    // 异步唤醒
    // ----------------------------------------------------------------
    
    /**
     * @brief 从其他线程唤醒事件循环
     */
    void wakeup();

private:
    /**
     * @brief 处理 IO 事件
     */
    void handle_io_events(int timeout_ms);
    
    /**
     * @brief 处理定时器事件
     */
    void handle_timer_events();
    
    /**
     * @brief 创建唤醒用的 eventfd
     */
    bool create_wakeup_fd();
    
    // epoll 文件描述符
    int epoll_fd_;
    
    // 唤醒用的 eventfd
    int wakeup_fd_;
    
    // IO 事件映射：fd -> callback
    std::map<int, io_callback> io_handlers_;
    
    // 定时器管理器
    timer_manager timer_manager_;
    
    // 定时器回调映射：timer_id -> callback
    std::map<timer_id, timer_callback> timer_callbacks_;
    
    // 信号回调映射：signum -> callback
    std::map<int, signal_callback> signal_handlers_;
    
    // 是否停止
    bool stopped_;
};

// ============================================================================
// 实现
// ============================================================================

inline event_loop::event_loop()
    : epoll_fd_(-1), wakeup_fd_(-1), stopped_(false)
{
    epoll_fd_ = epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd_ < 0) {
        throw std::runtime_error("epoll_create1 failed");
    }
    
    if (!create_wakeup_fd()) {
        close(epoll_fd_);
        throw std::runtime_error("create wakeup_fd failed");
    }
}

inline event_loop::~event_loop() {
    if (wakeup_fd_ >= 0) {
        close(wakeup_fd_);
    }
    if (epoll_fd_ >= 0) {
        close(epoll_fd_);
    }
}

inline void event_loop::run() {
    while (!stopped_) {
        // 计算下一次定时器到期时间
        unsigned long long now = timer_manager_.time_until_next(
            timer_manager_.pending_count() > 0 ? 0 : ~0ULL
        );
        
        int timeout = static_cast<int>(now);
        if (timeout > 10000) {
            timeout = 10000;  // 最多等待 10 秒
        }
        
        // 处理 IO 事件
        handle_io_events(timeout);
        
        // 处理定时器事件
        handle_timer_events();
    }
}

inline void event_loop::stop() {
    stopped_ = true;
    wakeup();
}

inline bool event_loop::add_io_event(int fd, uint32_t events, io_callback callback) {
    struct epoll_event ev;
    ev.events = events | EPOLLET;  // 边缘触发
    ev.data.fd = fd;
    
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) < 0) {
        return false;
    }
    
    io_handlers_[fd] = std::move(callback);
    return true;
}

inline bool event_loop::modify_io_event(int fd, uint32_t events, io_callback callback) {
    struct epoll_event ev;
    ev.events = events | EPOLLET;
    ev.data.fd = fd;
    
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev) < 0) {
        return false;
    }
    
    if (callback) {
        io_handlers_[fd] = std::move(callback);
    }
    return true;
}

inline bool event_loop::remove_io_event(int fd) {
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr) < 0) {
        return false;
    }
    
    io_handlers_.erase(fd);
    return true;
}

inline timer_id event_loop::add_timer(unsigned long long delay_ms, timer_callback callback) {
    auto wrapped_callback = [](timer_id id, void* user_data) {
        auto* cb = static_cast<timer_callback*>(user_data);
        if (cb) {
            (*cb)();
        }
    };
    
    timer_id id = timer_manager_.add_once(delay_ms, wrapped_callback, new timer_callback(callback));
    if (id != 0) {
        timer_callbacks_[id] = std::move(callback);
    }
    return id;
}

inline timer_id event_loop::add_repeat_timer(unsigned long long interval_ms, timer_callback callback) {
    auto wrapped_callback = [](timer_id id, void* user_data) {
        auto* cb = static_cast<timer_callback*>(user_data);
        if (cb) {
            (*cb)();
        }
    };
    
    timer_id id = timer_manager_.add_repeat(interval_ms, wrapped_callback, new timer_callback(callback));
    if (id != 0) {
        timer_callbacks_[id] = std::move(callback);
    }
    return id;
}

inline bool event_loop::cancel_timer(timer_id id) {
    auto it = timer_callbacks_.find(id);
    if (it != timer_callbacks_.end()) {
        delete &it->second;
        timer_callbacks_.erase(it);
    }
    return timer_manager_.cancel(id);
}

inline bool event_loop::add_signal(int signum, signal_callback callback) {
    // TODO: 实现信号处理（需要信号屏蔽和 signalfd）
    signal_handlers_[signum] = std::move(callback);
    return true;
}

inline bool event_loop::remove_signal(int signum) {
    signal_handlers_.erase(signum);
    return true;
}

inline void event_loop::wakeup() {
    uint64_t value = 1;
    write(wakeup_fd_, &value, sizeof(value));
}

inline void event_loop::handle_io_events(int timeout_ms) {
    const int MAX_EVENTS = 64;
    struct epoll_event events[MAX_EVENTS];
    
    int n = epoll_wait(epoll_fd_, events, MAX_EVENTS, timeout_ms);
    if (n < 0) {
        if (errno == EINTR) {
            return;  // 被信号中断
        }
        throw std::runtime_error("epoll_wait failed");
    }
    
    for (int i = 0; i < n; ++i) {
        int fd = events[i].data.fd;
        uint32_t revents = events[i].events;
        
        // 处理唤醒事件
        if (fd == wakeup_fd_) {
            uint64_t value;
            read(wakeup_fd_, &value, sizeof(value));
            continue;
        }
        
        // 处理 IO 事件
        auto it = io_handlers_.find(fd);
        if (it != io_handlers_.end()) {
            it->second(fd, revents);
        }
    }
}

inline void event_loop::handle_timer_events() {
    timer_manager_.tick();
}

inline bool event_loop::create_wakeup_fd() {
    wakeup_fd_ = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (wakeup_fd_ < 0) {
        return false;
    }
    
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = wakeup_fd_;
    
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, wakeup_fd_, &ev) < 0) {
        close(wakeup_fd_);
        wakeup_fd_ = -1;
        return false;
    }
    
    return true;
}

} // namespace zen

#endif // ZEN_EVENT_EVENT_LOOP_H
