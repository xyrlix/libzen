#pragma once

#include "../core/socket.h"
#include "../reactor/reactor.h"

#include <string>
#include <functional>
#include <memory>

namespace zen {
namespace net {

// TCP 客户端
class tcp_client {
public:
    using connect_callback = std::function<void(bool success)>;
    using read_callback = std::function<void(const char* data, size_t len)>;
    using write_callback = std::function<void()>;
    using close_callback = std::function<void()>;
    
    tcp_client();
    ~tcp_client();
    
    // 连接服务器
    bool connect(const std::string& ip, uint16_t port, int timeout_ms = 5000);
    bool async_connect(const std::string& ip, uint16_t port);
    void disconnect();
    
    // 发送/接收
    ssize_t send(const void* buf, size_t len);
    ssize_t send(const std::string& msg);
    std::string recv(size_t len);
    
    // 设置回调
    void set_connect_callback(connect_callback cb) { on_connect_ = cb; }
    void set_read_callback(read_callback cb) { on_read_ = cb; }
    void set_write_callback(write_callback cb) { on_write_ = cb; }
    void set_close_callback(close_callback cb) { on_close_ = cb; }
    
    // 运行事件循环
    void run();
    void stop_loop();
    
    // 获取信息
    std::string get_server_ip() const { return server_ip_; }
    uint16_t get_server_port() const { return server_port_; }
    std::string get_local_ip() const;
    uint16_t get_local_port() const;
    
    bool is_connected() const { return connected_; }
    bool is_running() const;
    
private:
    void on_read(int fd, const char* data, size_t len);
    void on_write(int fd);
    void on_close(int fd);
    
    void handle_connect();
    
    std::string server_ip_;
    uint16_t server_port_;
    bool connected_;
    bool connecting_;
    
    // Socket
    std::unique_ptr<socket> socket_;
    
    // Reactor
    std::unique_ptr<reactor> reactor_;
    
    // 回调
    connect_callback on_connect_;
    read_callback on_read_;
    write_callback on_write_;
    close_callback on_close_;
    
    // 连接状态
    int fd_;
};

} // namespace net
} // namespace zen
