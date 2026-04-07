#include "timer/timer_manager.h"
#include <algorithm>

namespace zen {

timer_manager::timer_manager() : next_id_(0) {
}

timer_id timer_manager::add_once(uint64_t delay_ms, timer_callback_t cb) {
    auto entry = zen::make_unique<timer_entry>();
    entry->id = ++next_id_;
    entry->type = timer_type::once;
    entry->delay_ms = delay_ms;
    entry->callback = cb;
    entry->next_fire = get_monotonic_time_ms() + delay_ms;
    
    timer_id id = entry->id;
    timers_.push_back(std::move(entry));
    std::push_heap(timers_.begin(), timers_.end(), entry_compare{});
    
    return id;
}

timer_id timer_manager::add_repeat(uint64_t interval_ms, timer_callback_t cb) {
    auto entry = zen::make_unique<timer_entry>();
    entry->id = ++next_id_;
    entry->type = timer_type::repeat;
    entry->delay_ms = interval_ms;
    entry->callback = cb;
    entry->next_fire = get_monotonic_time_ms() + interval_ms;
    
    timer_id id = entry->id;
    timers_.push_back(std::move(entry));
    std::push_heap(timers_.begin(), timers_.end(), entry_compare{});
    
    return id;
}

bool timer_manager::cancel(timer_id id) {
    for (auto& entry : timers_) {
        if (entry->id == id) {
            entry->cancelled = true;
            return true;
        }
    }
    return false;
}

void timer_manager::tick() {
    uint64_t now = get_monotonic_time_ms();
    
    while (!timers_.empty()) {
        auto& entry = timers_.front();
        
        if (entry->cancelled) {
            std::pop_heap(timers_.begin(), timers_.end(), entry_compare{});
            timers_.pop_back();
            continue;
        }
        
        if (entry->next_fire > now) {
            break;
        }
        
        if (entry->callback) {
            entry->callback();
        }
        
        if (entry->type == timer_type::repeat) {
            entry->next_fire = now + entry->delay_ms;
            std::push_heap(timers_.begin(), timers_.end(), entry_compare{});
        } else {
            std::pop_heap(timers_.begin(), timers_.end(), entry_compare{});
            timers_.pop_back();
        }
    }
}

void timer_manager::tick_until_empty(uint64_t poll_ms, uint64_t max_ms) {
    auto start = get_monotonic_time_ms();
    
    while (!timers_.empty() && !stop_requested_) {
        uint64_t now = get_monotonic_time_ms();
        
        if (max_ms > 0 && (now - start) >= max_ms) {
            break;
        }
        
        uint64_t next = next_expire_time_ms();
        if (next > poll_ms) {
            std::this_thread::sleep_for(std::chrono::milliseconds(std::min(poll_ms, next)));
        }
        
        tick();
    }
}

uint64_t timer_manager::next_expire_time_ms() const {
    if (timers_.empty()) return UINT64_MAX;
    
    uint64_t now = get_monotonic_time_ms();
    for (const auto& entry : timers_) {
        if (!entry->cancelled && entry->next_fire > now) {
            return entry->next_fire - now;
        }
    }
    return 0;
}

uint64_t timer_manager::get_monotonic_time_ms() const {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000ULL + ts.tv_nsec / 1000000ULL;
}

void timer_manager::stop() {
    stop_requested_ = true;
}

void timer_manager::clear() {
    timers_.clear();
}

size_t timer_manager::size() const {
    return timers_.size();
}

bool timer_manager::empty() const {
    return timers_.empty();
}

} // namespace zen
