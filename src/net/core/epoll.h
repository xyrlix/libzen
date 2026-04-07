#pragma once

#include <sys/epoll.h>
#include <cstdint>
#include <functional>

namespace zen {
namespace net {

// IO 事件类型
enum class io_event : uint32_t {
    in = EPOLLIN,
    out = EPOLLOUT,
    rdhup = EPOLLRDHUP,
    pri = EPOLLPRI,
    err = EPOLLERR,
    hup = EPOLLHUP
};

// IO 事件回调类型
using io_callback = std::function<void(int fd, uint32_t events)>;

// Epoll 封装类
class epoll {
public:
    epoll();
    explicit epoll(int size_hint);
    ~epoll();
    
    // 禁止拷贝
    epoll(const epoll&) = delete;
    epoll& operator=(const epoll&) = delete;
    
    // 允许移动
    epoll(epoll&& other) noexcept;
    epoll& operator=(epoll&& other) noexcept;
    
    // 添加/修改/删除监听
    bool add_fd(int fd, uint32_t events, io_callback cb);
    bool modify_fd(int fd, uint32_t events);
    bool remove_fd(int fd);
    
    // 等待事件
    int wait(int timeout_ms = -1);
    
    // 获取事件数量
    int event_count() const { return event_count_; }
    
    // 获取事件
    const struct epoll_event* get_events() const { return events_.get(); }
    
    // 获取回调
    io_callback get_callback(int fd) const;
    
    // 获取文件描述符
    int get_fd() const { return epoll_fd_; }
    
    // 关闭
    void close();

private:
    int epoll_fd_;
    int max_events_;
    int event_count_;
    std::unique_ptr<struct epoll_event[]> events_;
    
    // 回调映射
    std::unordered_map<int, io_callback> callbacks_;
};

} // namespace net
} // namespace zen
