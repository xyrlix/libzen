#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cassert>
#include "../threading/sync/mutex.h"
#include "../threading/sync/lock_guard.h"

namespace zen {

// ============================================================================
// thread_pool - 固定大小线程池
// ============================================================================
/**
 * @brief 固定线程数的线程池，基于任务队列
 *
 * 设计：
 *  - N 个 worker 线程持续从任务队列取任务
 *  - 任务以函数指针 + void* 参数方式存储（类型擦除，避免 std::function）
 *  - 支持 submit_simple（无返回值任务）
 *  - 支持 shutdown（等待所有任务完成）
 *
 * 示例：
 * @code
 * zen::thread_pool pool(4);
 * pool.submit([]{ printf("task 1\n"); });
 * pool.submit([]{ printf("task 2\n"); });
 * pool.shutdown();
 * @endcode
 */

// -------------------------------------------------------------------------
// 内部：任务节点（无锁链表节点，避免依赖 std::function）
// -------------------------------------------------------------------------
struct pool_task {
    void  (*func)(void*); // 函数指针
    void*  arg;           // 参数（task wrapper 的指针）
    pool_task* next;      // 链表指针
};

// task_wrapper：持有真正的 callable 对象
template<typename F>
struct task_wrapper {
    F callable;
    explicit task_wrapper(F&& f) : callable(static_cast<F&&>(f)) {}
};

// 执行并释放包装器
template<typename F>
static void invoke_and_delete(void* ptr) {
    task_wrapper<F>* tw = static_cast<task_wrapper<F>*>(ptr);
    tw->callable();
    delete tw;
}

// -------------------------------------------------------------------------
// 任务队列（单链表 + mutex）
// -------------------------------------------------------------------------
class task_queue {
public:
    task_queue() : head_(nullptr), tail_(nullptr), size_(0), closed_(false) {}
    ~task_queue() { clear(); }

    // 入队（线程安全）
    bool push(pool_task* task) {
        lock_guard<mutex> lk(mtx_);
        if (closed_) return false;
        task->next = nullptr;
        if (tail_) {
            tail_->next = task;
        } else {
            head_ = task;
        }
        tail_ = task;
        ++size_;
        cv_.notify_one(lk);
        return true;
    }

    // 出队（阻塞，直到有任务或关闭）
    pool_task* pop() {
        unique_lock<mutex> lk(mtx_);
        while (head_ == nullptr && !closed_) {
            cv_.wait(lk);
        }
        if (head_ == nullptr) return nullptr; // 关闭且空
        pool_task* t = head_;
        head_ = head_->next;
        if (!head_) tail_ = nullptr;
        --size_;
        return t;
    }

    // 关闭队列，唤醒所有等待线程
    void close() {
        lock_guard<mutex> lk(mtx_);
        closed_ = true;
        cv_.notify_all();
    }

    bool empty() const {
        lock_guard<mutex> lk(mtx_);
        return size_ == 0;
    }

    size_t size() const {
        lock_guard<mutex> lk(mtx_);
        return size_;
    }

private:
    void clear() {
        while (head_) {
            pool_task* next = head_->next;
            // 注意：这里不执行任务，只回收节点内存
            // arg 的清理由调用者负责（已在 invoke_and_delete 中释放）
            delete head_;
            head_ = next;
        }
        head_ = tail_ = nullptr;
        size_ = 0;
    }

    mutable mutex mtx_;
    condition_variable cv_;
    pool_task* head_;
    pool_task* tail_;
    size_t     size_;
    bool       closed_;
};

// -------------------------------------------------------------------------
// worker 线程入口
// -------------------------------------------------------------------------
struct worker_arg {
    task_queue* queue;
};

#if defined(_WIN32)
#include <windows.h>
static DWORD WINAPI worker_entry(LPVOID arg) {
    worker_arg* wa = static_cast<worker_arg*>(arg);
    while (true) {
        pool_task* task = wa->queue->pop();
        if (!task) break;
        task->func(task->arg);
        delete task;
    }
    delete wa;
    return 0;
}
#else
#include <pthread.h>
static void* worker_entry(void* arg) {
    worker_arg* wa = static_cast<worker_arg*>(arg);
    while (true) {
        pool_task* task = wa->queue->pop();
        if (!task) break;
        task->func(task->arg);
        delete task;
    }
    delete wa;
    return nullptr;
}
#endif

// -------------------------------------------------------------------------
// thread_pool 主类
// -------------------------------------------------------------------------
class thread_pool {
public:
    /**
     * @brief 构造函数
     * @param num_threads 工作线程数（默认为 CPU 核数）
     */
    explicit thread_pool(size_t num_threads = 0)
        : shutdown_(false), active_tasks_(0) {
        if (num_threads == 0) {
            // 默认用 4 个线程
            num_threads = 4;
        }
        num_threads_ = num_threads;

#if defined(_WIN32)
        handles_ = new HANDLE[num_threads_];
        for (size_t i = 0; i < num_threads_; ++i) {
            worker_arg* wa = new worker_arg{&queue_};
            handles_[i] = CreateThread(nullptr, 0, worker_entry, wa, 0, nullptr);
        }
#else
        threads_ = new pthread_t[num_threads_];
        for (size_t i = 0; i < num_threads_; ++i) {
            worker_arg* wa = new worker_arg{&queue_};
            pthread_create(&threads_[i], nullptr, worker_entry, wa);
        }
#endif
    }

    ~thread_pool() {
        if (!shutdown_) {
            shutdown();
        }
#if defined(_WIN32)
        delete[] handles_;
#else
        delete[] threads_;
#endif
    }

    // 禁止拷贝
    thread_pool(const thread_pool&) = delete;
    thread_pool& operator=(const thread_pool&) = delete;

    /**
     * @brief 提交无返回值任务
     * @tparam F callable 类型
     * @param f  要执行的函数
     */
    template<typename F>
    bool submit(F&& f) {
        if (shutdown_) return false;
        using FD = typename remove_reference<F>::type;
        task_wrapper<FD>* tw = new task_wrapper<FD>(static_cast<F&&>(f));
        pool_task* t = new pool_task{invoke_and_delete<FD>, tw, nullptr};
        return queue_.push(t);
    }

    /**
     * @brief 等待所有任务完成并关闭线程池
     */
    void shutdown() {
        if (shutdown_) return;
        shutdown_ = true;
        queue_.close();
#if defined(_WIN32)
        WaitForMultipleObjects(static_cast<DWORD>(num_threads_), handles_, TRUE, INFINITE);
        for (size_t i = 0; i < num_threads_; ++i) {
            CloseHandle(handles_[i]);
        }
#else
        for (size_t i = 0; i < num_threads_; ++i) {
            pthread_join(threads_[i], nullptr);
        }
#endif
    }

    /** @brief 返回线程数 */
    size_t thread_count() const noexcept { return num_threads_; }

    /** @brief 返回当前队列中的任务数 */
    size_t pending_tasks() const { return queue_.size(); }

private:
    task_queue queue_;
    size_t     num_threads_;
    bool       shutdown_;
    size_t     active_tasks_;

#if defined(_WIN32)
    HANDLE* handles_;
#else
    pthread_t* threads_;
#endif
};

} // namespace zen
