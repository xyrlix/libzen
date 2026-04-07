#include "event/event_loop.h"
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <string.h>

namespace zen {
namespace event {

namespace {
    uint32_t to_epoll_events(io_event events) {
        uint32_t result = 0;
        if (events & io_event::read) result |= EPOLLIN;
        if (events & io_event::write) result |= EPOLLOUT;
        return result;
    }

    io_event from_epoll_events(uint32_t events) {
        io_event result = io_event::none;
        if (events & EPOLLIN) result = result | io_event::read;
        if (events & EPOLLOUT) result = result | io_event::write;
        if (events & (EPOLLHUP | EPOLLERR)) result = result | io_event::error;
        return result;
    }
}

event_loop::event_loop() 
    : epoll_fd_(-1), wakeup_fd_(-1), running_(false), stop_requested_(false) {
    
    epoll_fd_ = epoll_create1(EPOLL_CLOEXEC);
    
    wakeup_fd_ = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    
    epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = wakeup_fd_;
    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, wakeup_fd_, &ev);
}

event_loop::~event_loop() {
    stop();
    
    if (wakeup_fd_ != -1) {
        close(wakeup_fd_);
    }
    
    if (epoll_fd_ != -1) {
        close(epoll_fd_);
    }
}

io_handler* event_loop::add_io_event(int fd, io_event events, io_callback_t cb) {
    auto handler = zen::make_unique<io_handler>();
    handler->fd = fd;
    handler->events = events;
    handler->callback = cb;
    
    epoll_event ev;
    ev.events = to_epoll_events(events) | EPOLLET;
    ev.data.ptr = handler.get();
    
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
        return nullptr;
    }
    
    io_handler* ptr = handler.get();
    io_handlers_[fd] = std::move(handler);
    return ptr;
}

bool event_loop::modify_io_event(io_handler* handler, io_event events) {
    if (!handler) return false;
    
    epoll_event ev;
    ev.events = to_epoll_events(events) | EPOLLET;
    ev.data.ptr = handler;
    
    handler->events = events;
    
    return epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, handler->fd, &ev) == 0;
}

bool event_loop::remove_io_event(io_handler* handler) {
    if (!handler) return false;
    
    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, handler->fd, nullptr);
    
    auto it = io_handlers_.find(handler->fd);
    if (it != io_handlers_.end()) {
        io_handlers_.erase(it);
        return true;
    }
    
    return false;
}

timer_handler* event_loop::add_timer(uint64_t delay_ms, timer_callback_t cb, bool repeat) {
    auto handler = zen::make_unique<timer_handler>();
    handler->callback = cb;
    handler->repeat = repeat;
    handler->interval_ms = delay_ms;
    handler->next_fire_ms = get_monotonic_time_ms() + delay_ms;
    
    timer_handler* ptr = handler.get();
    timers_.push_back(std::move(handler));
    std::push_heap(timers_.begin(), timers_.end(), timer_compare{});
    
    return ptr;
}

bool event_loop::cancel_timer(timer_handler* handler) {
    if (!handler) return false;
    
    handler->cancelled = true;
    return true;
}

signal_handler* event_loop::add_signal(int signum, signal_callback_t cb) {
    auto handler = zen::make_unique<signal_handler>();
    handler->signum = signum;
    handler->callback = cb;
    
    signal_handler* ptr = handler.get();
    signal_handlers_[signum] = std::move(handler);
    
    return ptr;
}

bool event_loop::remove_signal(signal_handler* handler) {
    if (!handler) return false;
    
    auto it = signal_handlers_.find(handler->signum);
    if (it != signal_handlers_.end()) {
        signal_handlers_.erase(it);
        return true;
    }
    
    return false;
}

void event_loop::run() {
    running_ = true;
    stop_requested_ = false;
    
    const int MAX_EVENTS = 64;
    epoll_event events[MAX_EVENTS];
    
    while (running_ && !stop_requested_) {
        int timeout_ms = calculate_next_timer_timeout();
        
        int n = epoll_wait(epoll_fd_, events, MAX_EVENTS, timeout_ms);
        
        if (n == -1 && errno != EINTR) {
            break;
        }
        
        for (int i = 0; i < n; ++i) {
            if (events[i].data.fd == wakeup_fd_) {
                handle_wakeup();
            } else {
                io_handler* handler = static_cast<io_handler*>(events[i].data.ptr);
                if (handler && handler->callback) {
                    io_event ev = from_epoll_events(events[i].events);
                    handler->callback(handler->fd, ev);
                }
            }
        }
        
        process_timers();
    }
    
    running_ = false;
}

void event_loop::stop() {
    stop_requested_ = true;
    wakeup();
}

void event_loop::wakeup() {
    uint64_t one = 1;
    write(wakeup_fd_, &one, sizeof(one));
}

void event_loop::handle_wakeup() {
    uint64_t val;
    read(wakeup_fd_, &val, sizeof(val));
}

int event_loop::calculate_next_timer_timeout() {
    while (!timers_.empty()) {
        auto& timer = timers_.front();
        if (timer->cancelled) {
            std::pop_heap(timers_.begin(), timers_.end(), timer_compare{});
            timers_.pop_back();
        } else {
            uint64_t now = get_monotonic_time_ms();
            if (timer->next_fire_ms <= now) {
                return 0;
            } else {
                return static_cast<int>(timer->next_fire_ms - now);
            }
        }
    }
    return -1;
}

void event_loop::process_timers() {
    uint64_t now = get_monotonic_time_ms();
    
    while (!timers_.empty()) {
        auto& timer = timers_.front();
        
        if (timer->cancelled) {
            std::pop_heap(timers_.begin(), timers_.end(), timer_compare{});
            timers_.pop_back();
            continue;
        }
        
        if (timer->next_fire_ms > now) {
            break;
        }
        
        if (timer->callback) {
            timer->callback();
        }
        
        if (timer->repeat) {
            timer->next_fire_ms += timer->interval_ms;
            std::push_heap(timers_.begin(), timers_.end(), timer_compare{});
        } else {
            std::pop_heap(timers_.begin(), timers_.end(), timer_compare{});
            timers_.pop_back();
        }
    }
}

uint64_t event_loop::get_monotonic_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000ULL + ts.tv_nsec / 1000000ULL;
}

} // namespace event
} // namespace zen
