#pragma once

#include "../core/socket.h"
#include "../reactor/reactor.h"

#include <string>
#include <functional>
#include <memory>

namespace zen {
namespace net {

// TCP 连接封装
class tcp_connection : public std::enable_shared_from_this<tcp_connection> {
public:
    using ptr = std::shared_ptr<tcp_connection>;
    
    tcp_connection(int fd, const std::string& ip, uint16_t port);
    ~tcp_connection();
    
    // 获取信息
    int get_fd() const { return fd_; }
    std::string get_remote_address() const { return remote_ip_; }
    uint16_t get_remote_port() const { return remote_port_; }
    
    // 发送/接收
    ssize_t send(const void* buf, size_t len);
    ssize_t send(const std::string& msg);
    std::string recv(size_t len);
    
    // 关闭
    void close();
    void shutdown_read();
    void shutdown_write();
    
    // 设置回调
    void set_read_callback(read_callback cb) { on_read_ = cb; }
    void set_write_callback(write_callback cb) { on_write_ = cb; }
    void set_close_callback(close_callback cb) { on_close_ = cb; }
    
    // 处理事件
    void handle_read();
    void handle_write();
    void handle_close();
    
    // 检查连接状态
    bool is_connected() const { return connected_; }
    
private:
    int fd_;
    std::string remote_ip_;
    uint16_t remote_port_;
    bool connected_;
    
    read_callback on_read_;
    write_callback on_write_;
    close_callback on_close_;
};

// TCP 服务器
class tcp_server {
public:
    using new_connection_callback = std::function<void(tcp_connection::ptr)>;
    using read_callback = std::function<void(tcp_connection::ptr, const char*, size_t)>;
    using write_callback = std::function<void(tcp_connection::ptr)>;
    using close_callback = std::function<void(tcp_connection::ptr)>;
    
    tcp_server();
    ~tcp_server();
    
    // 启动服务器
    bool start(const std::string& ip, uint16_t port);
    bool start(uint16_t port);
    void stop();
    
    // 设置回调
    void set_new_connection_callback(new_connection_callback cb) { on_new_connection_ = cb; }
    void set_read_callback(read_callback cb) { on_read_ = cb; }
    void set_write_callback(write_callback cb) { on_write_ = cb; }
    void set_close_callback(close_callback cb) { on_close_ = cb; }
    
    // 运行事件循环
    void run();
    void run_in_thread();
    void stop_loop();
    
    // 获取信息
    std::string get_ip() const { return ip_; }
    uint16_t get_port() const { return port_; }
    bool is_running() const;
    
    // 获取连接管理
    size_t get_connection_count() const;
    void broadcast(const char* data, size_t len);
    
private:
    void on_new_connection(int client_fd, const std::string& ip, uint16_t port);
    void on_connection_read(int fd, const char* data, size_t len);
    void on_connection_write(int fd);
    void on_connection_close(int fd);
    
    void remove_connection(int fd);
    tcp_connection::ptr find_connection(int fd);
    
    std::string ip_;
    uint16_t port_;
    bool running_;
    
    // Reactor
    std::unique_ptr<reactor> reactor_;
    
    // 连接管理
    std::unordered_map<int, tcp_connection::ptr> connections_;
    threading::mutex connections_mutex_;
    
    // 回调
    new_connection_callback on_new_connection_;
    read_callback on_read_;
    write_callback on_write_;
    close_callback on_close_;
};

} // namespace net
} // namespace zen
