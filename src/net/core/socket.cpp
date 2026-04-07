#include "net/core/socket.h"
#include <cstring>
#include <unistd.h>
#include <fcntl.h>

namespace zen {
namespace net {

namespace {
    int to_os_family(address_family family) {
        switch (family) {
            case address_family::ipv4: return AF_INET;
            case address_family::ipv6: return AF_INET6;
            case address_family::unix: return AF_UNIX;
            default: return AF_INET;
        }
    }

    int to_os_type(socket_type type) {
        switch (type) {
            case socket_type::stream: return SOCK_STREAM;
            case socket_type::dgram: return SOCK_DGRAM;
            case socket_type::raw: return SOCK_RAW;
            default: return SOCK_STREAM;
        }
    }

    int to_os_protocol(protocol_type proto) {
        switch (proto) {
            case protocol_type::tcp: return IPPROTO_TCP;
            case protocol_type::udp: return IPPROTO_UDP;
            default: return 0;
        }
    }
}

socket::~socket() {
    close();
}

bool socket::create(address_family family, socket_type type, protocol_type proto) {
    fd_ = ::socket(to_os_family(family), to_os_type(type) | SOCK_CLOEXEC, to_os_protocol(proto));
    return fd_ != -1;
}

bool socket::connect(const socket_address& addr) {
    return ::connect(fd_, addr.addr(), addr.len()) == 0;
}

bool socket::bind(const socket_address& addr) {
    return ::bind(fd_, addr.addr(), addr.len()) == 0;
}

bool socket::listen(int backlog) {
    return ::listen(fd_, backlog) == 0;
}

int socket::accept(socket_address* client_addr) {
    sockaddr_storage addr;
    socklen_t len = sizeof(addr);
    int client_fd = ::accept4(fd_, (sockaddr*)&addr, &len, SOCK_CLOEXEC);
    if (client_fd != -1 && client_addr) {
        client_addr->assign((sockaddr*)&addr, len);
    }
    return client_fd;
}

ssize_t socket::send(const void* data, size_t size, int flags) {
    return ::send(fd_, data, size, flags);
}

ssize_t socket::recv(void* buf, size_t size, int flags) {
    return ::recv(fd_, buf, size, flags);
}

bool socket::close() {
    if (fd_ == -1) return false;
    int ret = ::close(fd_);
    fd_ = -1;
    return ret == 0;
}

bool socket::set_reuse_addr(bool enable) {
    int opt = enable ? 1 : 0;
    return setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == 0;
}

bool socket::set_non_blocking(bool enable) {
    int flags = fcntl(fd_, F_GETFL, 0);
    if (flags == -1) return false;
    return fcntl(fd_, F_SETFL, enable ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK)) != -1;
}

bool socket::set_tcp_no_delay(bool enable) {
    int opt = enable ? 1 : 0;
    return setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) == 0;
}

int socket::get_fd() const {
    return fd_;
}

bool socket::is_valid() const {
    return fd_ != -1;
}

} // namespace net
} // namespace zen
