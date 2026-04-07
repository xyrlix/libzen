#pragma once

#include "../core/epoll.h"
#include "../core/socket.h"
#include "../../event/event_loop.h"
#include "../../threading/sync/mutex.h"
#include "../../threading/sync/condition_variable.h"

#include <cstdint>
#include <functional>
#include <unordered_map>
#include <memory>
#include <atomic>

namespace zen {
namespace net {

// IO 事件类型
using io_event_handler = std::function<void(int fd, uint32_t events)>;
using connection_callback = std::function<void(int client_fd, const std::string& ip, uint16_t port)>;
using read_callback = std::function<void(int fd, const char* data, size_t len)>;
using write_callback = std::function<void(int fd)>;
using close_callback = std::function<void(int fd)>;

// Reactor 模式实现
class reactor {
public:
    reactor();
    ~reactor();
    
    // 启动事件循环
    void run();
    void stop();
    
    // 添加/修改/删除 IO 监听
    bool add_io_handler(int fd, uint32_t events, io_event_handler handler);
    bool modify_io_handler(int fd, uint32_t events);
    bool remove_io_handler(int fd);
    
    // TCP 服务器
    bool start_tcp_server(uint16_t port, connection_callback on_connect,
                          read_callback on_read, close_callback on_close);
    void stop_tcp_server();
    
    // TCP 客户端
    bool connect_tcp_server(const std::string& ip, uint16_t port,
                             read_callback on_read, close_callback on_close);
    void disconnect_tcp_server();
    
    // 定时器
    uint64_t add_timer(uint64_t interval_ms, std::function<void()> callback, bool repeat = false);
    void cancel_timer(uint64_t timer_id);
    
    // 唤醒事件循环
    void wakeup();
    
    // 获取线程安全的状态
    bool is_running() const { return running_; }
    
private:
    void handle_tcp_accept(int listen_fd);
    void handle_tcp_read(int fd);
    void handle_tcp_write(int fd);
    void handle_tcp_close(int fd);
    
    void on_io_event(int fd, uint32_t events);
    
    // 事件循环
    event_loop event_loop_;
    
    // TCP 服务器
    int tcp_listen_fd_;
    connection_callback on_connect_;
    read_callback on_read_;
    close_callback on_close_;
    
    // TCP 客户端
    int tcp_client_fd_;
    
    // IO 处理器映射
    std::unordered_map<int, io_event_handler> io_handlers_;
    threading::mutex io_handlers_mutex_;
    
    // 连接状态
    std::unordered_map<int, std::string> connection_ips_;
    std::unordered_map<int, uint16_t> connection_ports_;
    threading::mutex connection_mutex_;
    
    // 运行状态
    std::atomic<bool> running_;
    
    // 唤醒 fd
    int wakeup_fd_;
};

} // namespace net
} // namespace zen
