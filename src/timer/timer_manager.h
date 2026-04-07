/**
 * @file timer_manager.h
 * @brief 定时器管理器
 *
 * 管理所有定时器的生命周期，提供：
 *   - add_once(delay_ms, cb, user_data) -> timer_id : 添加一次性定时器
 *   - add_repeat(interval_ms, cb, user_data) -> timer_id : 添加重复定时器
 *   - cancel(id)                               : 取消定时器
 *   - tick()                                   : 检查并触发到期定时器（需外部定期调用）
 *   - tick_until_empty(poll_ms)                : 轮询直到所有定时器触发完毕（测试用）
 *
 * 线程安全：tick() / add_once() / add_repeat() / cancel() 均加锁，
 * 可从多线程调用，但回调在调用 tick() 的线程中执行。
 *
 * 示例（单线程轮询）：
 * @code
 * zen::timer_manager tm;
 *
 * bool done = false;
 * tm.add_once(100, [](zen::timer_id, void* p){
 *     *static_cast<bool*>(p) = true;
 * }, &done);
 *
 * while (!done) {
 *     tm.tick();
 *     zen::this_thread::sleep_for(10);
 * }
 * @endcode
 */
#pragma once
#include "timer.h"
#include "timer_queue.h"
#include "../threading/sync/mutex.h"
#include "../threading/sync/lock_guard.h"
#include "../threading/thread/this_thread.h"

namespace zen {

class timer_manager {
public:
    timer_manager() noexcept : next_id_(1) {}

    // 禁止拷贝
    timer_manager(const timer_manager&) = delete;
    timer_manager& operator=(const timer_manager&) = delete;

    // ----------------------------------------------------------------
    // 添加定时器
    // ----------------------------------------------------------------

    /**
     * @brief 添加一次性定时器
     * @param delay_ms   延迟毫秒数（delay_ms=0 时下次 tick 即触发）
     * @param cb         回调函数（不可为 nullptr）
     * @param user_data  用户数据（可选）
     * @return 定时器 ID（INVALID_TIMER_ID 表示失败，如队列满）
     */
    timer_id add_once(unsigned long long delay_ms,
                      timer_callback cb,
                      void* user_data = nullptr) noexcept {
        return add_internal(timer_type::ONCE, delay_ms, delay_ms, cb, user_data);
    }

    /**
     * @brief 添加重复定时器（首次触发也是 interval_ms 之后）
     * @param interval_ms 间隔毫秒数（必须 > 0）
     * @param cb          回调函数
     * @param user_data   用户数据
     */
    timer_id add_repeat(unsigned long long interval_ms,
                        timer_callback cb,
                        void* user_data = nullptr) noexcept {
        if (interval_ms == 0) interval_ms = 1;
        return add_internal(timer_type::REPEAT, interval_ms, interval_ms, cb, user_data);
    }

    /**
     * @brief 添加一次性定时器（首次 delay，之后重复 interval，添加后只触发一次）
     *        等价于 add_once，提供此重载以保持接口对称。
     */
    timer_id add_once_at(unsigned long long expire_ms,
                         timer_callback cb,
                         void* user_data = nullptr) noexcept {
        if (!cb) return INVALID_TIMER_ID;
        lock_guard<mutex> lk(mtx_);
        timer_entry e;
        e.id          = next_id_++;
        e.type        = timer_type::ONCE;
        e.expire_ms   = expire_ms;
        e.interval_ms = 0;
        e.callback    = cb;
        e.user_data   = user_data;
        e.cancelled   = false;
        if (!queue_.push(e)) return INVALID_TIMER_ID;
        return e.id;
    }

    // ----------------------------------------------------------------
    // 取消
    // ----------------------------------------------------------------

    /**
     * @brief 取消一个定时器（惰性取消，不立即从堆中移除）
     * @return true 找到，false 未找到（可能已触发）
     */
    bool cancel(timer_id id) noexcept {
        if (id == INVALID_TIMER_ID) return false;
        lock_guard<mutex> lk(mtx_);
        return queue_.cancel(id);
    }

    // ----------------------------------------------------------------
    // 驱动
    // ----------------------------------------------------------------

    /**
     * @brief 检查并触发所有已到期的定时器
     * @return 本次触发的定时器数量
     */
    int tick() noexcept {
        unsigned long long now = this_thread::monotonic_ms();

        // 弹出到期条目（加锁）
        static const int MAX_BATCH = 32;
        timer_entry fired[MAX_BATCH];
        int count = 0;
        {
            lock_guard<mutex> lk(mtx_);
            count = queue_.pop_expired(now, fired, MAX_BATCH);
        }

        // 执行回调（不持锁，允许回调内部 add_once 等操作）
        for (int i = 0; i < count; ++i) {
            if (fired[i].callback) {
                fired[i].callback(fired[i].id, fired[i].user_data);
            }
            // 若是重复定时器，重新入队
            if (fired[i].type == timer_type::REPEAT) {
                lock_guard<mutex> lk(mtx_);
                timer_entry re = fired[i];
                re.expire_ms = now + re.interval_ms;
                re.cancelled = false;
                queue_.push(re);
            }
        }

        return count;
    }

    /**
     * @brief 轮询直到队列为空（用于测试）
     * @param poll_ms  每次 tick 之间的睡眠时间
     * @param max_ms   最长等待时间（超时返回）
     */
    void tick_until_empty(unsigned long poll_ms = 5,
                          unsigned long max_ms = 10000) noexcept {
        unsigned long long start = this_thread::monotonic_ms();
        while (true) {
            tick();
            {
                lock_guard<mutex> lk(mtx_);
                if (queue_.empty()) break;
            }
            unsigned long long elapsed = this_thread::monotonic_ms() - start;
            if (elapsed >= max_ms) break;
            this_thread::sleep_for(poll_ms);
        }
    }

    // ----------------------------------------------------------------
    // 查询
    // ----------------------------------------------------------------

    int  pending_count() const noexcept {
        lock_guard<mutex> lk(mtx_);
        return queue_.size();
    }

    bool empty() const noexcept {
        lock_guard<mutex> lk(mtx_);
        return queue_.empty();
    }

    /**
     * @brief 距下一个定时器到期还有多少毫秒（队列空时返回 UINT64_MAX）
     */
    unsigned long long time_until_next(unsigned long long now) const noexcept {
        lock_guard<mutex> lk(mtx_);
        unsigned long long nx = queue_.next_expire();
        if (nx == ~0ULL) return ~0ULL;
        return nx > now ? nx - now : 0;
    }

private:
    timer_id add_internal(timer_type type,
                          unsigned long long first_ms,
                          unsigned long long interval_ms,
                          timer_callback cb,
                          void* user_data) noexcept {
        if (!cb) return INVALID_TIMER_ID;
        unsigned long long now = this_thread::monotonic_ms();
        lock_guard<mutex> lk(mtx_);
        timer_entry e;
        e.id          = next_id_++;
        e.type        = type;
        e.expire_ms   = now + first_ms;
        e.interval_ms = interval_ms;
        e.callback    = cb;
        e.user_data   = user_data;
        e.cancelled   = false;
        if (!queue_.push(e)) return INVALID_TIMER_ID;
        return e.id;
    }

    mutable mutex  mtx_;
    timer_queue    queue_;
    timer_id       next_id_;
};

} // namespace zen
