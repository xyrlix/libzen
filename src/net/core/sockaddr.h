#pragma once

#include <cstdint>
#include <string>
#include <cstring>

namespace zen {
namespace net {

// IPv4 地址结构
struct ipv4_address {
    uint32_t addr;
    
    ipv4_address() : addr(0) {}
    explicit ipv4_address(uint32_t a) : addr(a) {}
    explicit ipv4_address(const std::string& ip) { addr = string_to_ip(ip); }
    
    std::string to_string() const { return ip_to_string(addr); }
    
    static uint32_t string_to_ip(const std::string& ip);
    static std::string ip_to_string(uint32_t addr);
};

// IPv6 地址结构
struct ipv6_address {
    uint8_t addr[16];
    
    ipv6_address() { memset(addr, 0, 16); }
    explicit ipv6_address(const uint8_t* a) { memcpy(addr, a, 16); }
    
    std::string to_string() const;
};

// Socket 地址（通用）
class socket_address {
public:
    socket_address();
    explicit socket_address(uint16_t port);
    socket_address(const std::string& ip, uint16_t port);
    
    void set_ip(const std::string& ip);
    void set_port(uint16_t port);
    
    std::string get_ip() const;
    uint16_t get_port() const;
    
    bool is_ipv4() const;
    bool is_ipv6() const;
    
    // 获取原生地址结构
    const struct sockaddr* get_addr() const;
    struct sockaddr* get_addr();
    socklen_t get_addr_len() const;
    
    // 从原生地址结构构造
    static socket_address from_sockaddr(const struct sockaddr* addr, socklen_t len);

private:
    union {
        struct sockaddr_in addr4_;
        struct sockaddr_in6 addr6_;
    } addr_;
    bool is_ipv4_;
};

} // namespace net
} // namespace zen
