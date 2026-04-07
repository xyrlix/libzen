#include "net/tcp/tcp_server.h"
#include <sys/socket.h>
#include <unistd.h>

namespace zen {
namespace net {

tcp_server::tcp_server(event_loop* loop)
    : loop_(loop), listen_fd_(-1), accept_handler_(nullptr) {
}

tcp_server::~tcp_server() {
    stop();
}

bool tcp_server::start(const std::string& ip, uint16_t port, int backlog) {
    listen_fd_ = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (listen_fd_ == -1) return false;

    int opt = 1;
    setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    addr.sin_port = htons(port);

    if (bind(listen_fd_, (sockaddr*)&addr, sizeof(addr)) == -1) {
        ::close(listen_fd_);
        listen_fd_ = -1;
        return false;
    }

    if (listen(listen_fd_, backlog) == -1) {
        ::close(listen_fd_);
        listen_fd_ = -1;
        return false;
    }

    accept_handler_ = loop_->add_io_event(
        listen_fd_,
        io_event::read,
        [this](int fd, io_event events) { handle_accept(); }
    );

    return accept_handler_ != nullptr;
}

void tcp_server::stop() {
    if (accept_handler_) {
        loop_->remove_io_event(accept_handler_);
        accept_handler_ = nullptr;
    }
    if (listen_fd_ != -1) {
        ::close(listen_fd_);
        listen_fd_ = -1;
    }
    connections_.clear();
}

void tcp_server::handle_accept() {
    sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    int client_fd = ::accept4(listen_fd_, (sockaddr*)&client_addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    
    if (client_fd != -1) {
        auto conn = zen::make_shared<tcp_connection>(loop_, client_fd);
        conn->set_connection_callback(connection_callback_);
        conn->set_message_callback(message_callback_);
        conn->set_write_complete_callback(write_complete_callback_);
        conn->set_close_callback([this](const tcp_connection_ptr& conn) {
            connections_.erase(conn->get_fd());
        });
        
        connections_[client_fd] = conn;
        
        if (connection_callback_) {
            connection_callback_(conn);
        }
        
        conn->established();
    }
}

void tcp_server::set_connection_callback(const connection_callback_t& cb) {
    connection_callback_ = cb;
}

void tcp_server::set_message_callback(const message_callback_t& cb) {
    message_callback_ = cb;
}

void tcp_server::set_write_complete_callback(const write_complete_callback_t& cb) {
    write_complete_callback_ = cb;
}

} // namespace net
} // namespace zen
