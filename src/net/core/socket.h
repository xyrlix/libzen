#pragma once

#include <cstdint>
#include <string>

namespace zen {
namespace net {

// 地址族枚举
enum class address_family {
    ipv4 = AF_INET,
    ipv6 = AF_INET6,
    unspecified = AF_UNSPEC
};

// Socket 类型枚举
enum class socket_type {
    stream = SOCK_STREAM,    // TCP
    dgram = SOCK_DGRAM,      // UDP
    raw = SOCK_RAW
};

// 协议类型枚举
enum class protocol_type {
    tcp = IPPROTO_TCP,
    udp = IPPROTO_UDP,
    icmp = IPPROTO_ICMP,
    unspecified = 0
};

// Socket 异常类
class socket_exception : public std::runtime_error {
public:
    explicit socket_exception(const std::string& msg)
        : std::runtime_error(msg) {}
};

// Socket 基类
class socket {
public:
    socket();
    socket(address_family af, socket_type type, protocol_type proto = protocol_type::unspecified);
    explicit socket(int fd);
    
    // 禁止拷贝
    socket(const socket&) = delete;
    socket& operator=(const socket&) = delete;
    
    // 允许移动
    socket(socket&& other) noexcept;
    socket& operator=(socket&& other) noexcept;
    
    ~socket();
    
    // 连接服务器
    void connect(const std::string& ip, uint16_t port);
    void connect(uint32_t ip, uint16_t port);
    
    // 绑定地址
    void bind(const std::string& ip, uint16_t port);
    void bind(uint32_t ip, uint16_t port);
    void bind(uint16_t port); // 绑定到所有接口
    
    // 监听
    void listen(int backlog = 128);
    
    // 接受连接
    socket accept();
    bool accept(socket& client);
    
    // 发送/接收数据
    ssize_t send(const void* buf, size_t len, int flags = 0);
    ssize_t send(const std::string& msg, int flags = 0);
    
    ssize_t recv(void* buf, size_t len, int flags = 0);
    std::string recv(size_t len, int flags = 0);
    
    // UDP sendto/recvfrom
    ssize_t sendto(const void* buf, size_t len, const std::string& ip, uint16_t port);
    ssize_t recvfrom(void* buf, size_t len, std::string& ip, uint16_t& port);
    
    // 设置选项
    void set_reuse_addr(bool on = true);
    void set_reuse_port(bool on = true);
    void set_keep_alive(bool on = true);
    void set_non_blocking(bool on = true);
    void set_tcp_no_delay(bool on = true);
    
    // 获取选项
    bool get_reuse_addr() const;
    bool get_reuse_port() const;
    bool get_keep_alive() const;
    bool get_non_blocking() const;
    
    // 获取信息
    int get_fd() const { return fd_; }
    address_family get_address_family() const;
    socket_type get_socket_type() const;
    
    // 关闭
    void close();
    void shutdown_read();
    void shutdown_write();
    void shutdown();
    
    // 判断有效性
    explicit operator bool() const { return fd_ != -1; }
    bool is_valid() const { return fd_ != -1; }
    
    // 获取本地和对端地址
    std::string get_local_address() const;
    uint16_t get_local_port() const;
    std::string get_remote_address() const;
    uint16_t get_remote_port() const;

protected:
    int fd_;
    
private:
    static uint32_t string_to_ip(const std::string& ip);
    static std::string ip_to_string(uint32_t ip);
};

} // namespace net
} // namespace zen
