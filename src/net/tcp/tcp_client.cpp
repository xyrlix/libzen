#include "net/tcp/tcp_client.h"
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

namespace zen {
namespace net {

tcp_client::tcp_client(event_loop* loop)
    : loop_(loop), connect_fd_(-1), connected_(false) {
}

tcp_client::~tcp_client() {
    disconnect();
}

bool tcp_client::connect(const std::string& ip, uint16_t port) {
    connect_fd_ = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (connect_fd_ == -1) return false;

    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    addr.sin_port = htons(port);

    int ret = ::connect(connect_fd_, (sockaddr*)&addr, sizeof(addr));
    
    if (ret == 0) {
        connected_ = true;
        connection_ = zen::make_shared<tcp_connection>(loop_, connect_fd_);
        setup_connection();
        return true;
    } else if (errno == EINPROGRESS) {
        connect_handler_ = loop_->add_io_event(
            connect_fd_,
            io_event::write,
            [this](int fd, io_event events) { handle_connect(); }
        );
        return true;
    } else {
        ::close(connect_fd_);
        connect_fd_ = -1;
        return false;
    }
}

void tcp_client::disconnect() {
    if (connect_handler_) {
        loop_->remove_io_event(connect_handler_);
        connect_handler_ = nullptr;
    }
    connection_.reset();
    if (connect_fd_ != -1) {
        ::close(connect_fd_);
        connect_fd_ = -1;
    }
    connected_ = false;
}

void tcp_client::handle_connect() {
    int error = 0;
    socklen_t len = sizeof(error);
    if (getsockopt(connect_fd_, SOL_SOCKET, SO_ERROR, &error, &len) == 0 && error == 0) {
        connected_ = true;
        connection_ = zen::make_shared<tcp_connection>(loop_, connect_fd_);
        setup_connection();
        
        loop_->remove_io_event(connect_handler_);
        connect_handler_ = nullptr;
        
        if (connect_callback_) {
            connect_callback_(true);
        }
    } else {
        if (connect_callback_) {
            connect_callback_(false);
        }
        disconnect();
    }
}

void tcp_client::setup_connection() {
    connection_->set_message_callback(message_callback_);
    connection_->set_write_complete_callback(write_complete_callback_);
    connection_->set_close_callback([this](const tcp_connection_ptr& conn) {
        connected_ = false;
        disconnect();
        if (close_callback_) {
            close_callback_();
        }
    });
    connection_->established();
}

void tcp_client::set_connect_callback(const connect_callback_t& cb) {
    connect_callback_ = cb;
}

void tcp_client::set_message_callback(const message_callback_t& cb) {
    message_callback_ = cb;
}

void tcp_client::set_write_complete_callback(const write_complete_callback_t& cb) {
    write_complete_callback_ = cb;
}

void tcp_client::set_close_callback(const close_callback_t& cb) {
    close_callback_ = cb;
}

void tcp_client::send(const void* data, size_t size) {
    if (connection_) {
        connection_->send(data, size);
    }
}

bool tcp_client::is_connected() const {
    return connected_;
}

tcp_connection_ptr tcp_client::get_connection() const {
    return connection_;
}

} // namespace net
} // namespace zen
