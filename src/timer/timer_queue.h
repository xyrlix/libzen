/**
 * @file timer_queue.h
 * @brief 最小堆定时器队列
 *
 * 按到期时间排序的最小堆（min-heap），支持：
 *   - push(entry)       : 插入新定时器，O(log n)
 *   - pop_expired(now)  : 弹出所有已到期的定时器，O(k log n)
 *   - cancel(id)        : 惰性取消（标记 cancelled，堆中仍保留直到弹出时跳过）
 *   - size() / empty()
 *
 * 内部使用固定大小的静态数组，避免堆分配（最多 MAX_TIMERS 个）。
 */
#pragma once
#include "timer.h"
#include <cstddef>

namespace zen {

// ============================================================================
// timer_queue
// ============================================================================

#ifndef ZEN_TIMER_QUEUE_MAX
#define ZEN_TIMER_QUEUE_MAX 256
#endif

class timer_queue {
public:
    timer_queue() noexcept : size_(0) {}

    /**
     * @brief 插入一个定时器，O(log n)
     * @return true 成功，false 队列已满
     */
    bool push(const timer_entry& entry) noexcept {
        if (size_ >= ZEN_TIMER_QUEUE_MAX) return false;
        heap_[size_] = entry;
        sift_up(size_);
        ++size_;
        return true;
    }

    /**
     * @brief 弹出所有 expire_ms <= now 的定时器（已取消的跳过）
     * @param now        当前单调毫秒
     * @param out        输出数组
     * @param out_max    out 数组最大容量
     * @return 实际弹出的有效（未取消）定时器数量
     */
    int pop_expired(unsigned long long now,
                    timer_entry* out, int out_max) noexcept {
        int count = 0;
        while (size_ > 0 && heap_[0].expire_ms <= now) {
            timer_entry top = heap_[0];
            remove_top();
            if (top.cancelled) continue;  // 惰性取消：跳过
            if (count < out_max) {
                out[count++] = top;
            }
        }
        return count;
    }

    /**
     * @brief 惰性取消：将对应 id 的条目标记为 cancelled
     * @return true 找到并标记，false 未找到（可能已触发或从未添加）
     */
    bool cancel(timer_id id) noexcept {
        for (int i = 0; i < size_; ++i) {
            if (heap_[i].id == id) {
                heap_[i].cancelled = true;
                return true;
            }
        }
        return false;
    }

    /**
     * @brief 返回堆顶（最近到期）的时间，队列为空时返回 UINT64_MAX
     */
    unsigned long long next_expire() const noexcept {
        if (size_ == 0) return ~0ULL;
        return heap_[0].expire_ms;
    }

    int  size()  const noexcept { return size_; }
    bool empty() const noexcept { return size_ == 0; }
    void clear() noexcept       { size_ = 0; }

private:
    // ----------------------------------------------------------------
    // 最小堆操作（按 expire_ms 排序）
    // ----------------------------------------------------------------

    void sift_up(int i) noexcept {
        while (i > 0) {
            int parent = (i - 1) / 2;
            if (heap_[parent].expire_ms > heap_[i].expire_ms) {
                swap_entries(parent, i);
                i = parent;
            } else {
                break;
            }
        }
    }

    void sift_down(int i) noexcept {
        while (true) {
            int smallest = i;
            int left  = 2 * i + 1;
            int right = 2 * i + 2;
            if (left  < size_ && heap_[left].expire_ms  < heap_[smallest].expire_ms) smallest = left;
            if (right < size_ && heap_[right].expire_ms < heap_[smallest].expire_ms) smallest = right;
            if (smallest == i) break;
            swap_entries(i, smallest);
            i = smallest;
        }
    }

    void remove_top() noexcept {
        if (size_ == 0) return;
        heap_[0] = heap_[--size_];
        sift_down(0);
    }

    void swap_entries(int a, int b) noexcept {
        timer_entry tmp = heap_[a];
        heap_[a] = heap_[b];
        heap_[b] = tmp;
    }

    timer_entry heap_[ZEN_TIMER_QUEUE_MAX];
    int         size_;
};

} // namespace zen
