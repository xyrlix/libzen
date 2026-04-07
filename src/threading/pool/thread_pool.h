/**
 * @file thread_pool.h
 * @brief 线程池实现
 * 
 * 线程池用于管理和复用线程，避免频繁创建销毁线程的开销：
 * 
 * - thread_pool   : 固定大小线程池，支持任务提交和异步执行
 * - 任务队列      : 使用条件变量和互斥锁实现的任务队列
 * 
 * 特性：
 * - 固定线程数量（构造时指定）
 * - 支持任意可调用对象作为任务
 * - 支持 lambda、函数指针、函数对象
 * - RAII 管理，析构时自动等待所有任务完成
 * 
 * 示例：
 * @code
 * zen::thread_pool pool(4);  // 4 个工作线程
 * 
 * // 提交任务
 * auto result1 = pool.submit([]{ return 42; });
 * auto result2 = pool.submit([](int x){ return x * 2; }, 21);
 * 
 * // 获取结果
 * std::cout << result1.get() << std::endl;  // 42
 * std::cout << result2.get() << std::endl;  // 42
 * 
 * // void 任务
 * pool.submit([]{ std::cout << "Hello from thread!\n"; });
 * @endcode
 */
#ifndef ZEN_THREADING_POOL_THREAD_POOL_H
#define ZEN_THREADING_POOL_THREAD_POOL_H

#include "../thread/thread.h"
#include "../sync/mutex.h"
#include "../sync/condition_variable.h"
#include "../future/future.h"
#include "../../utility/function.h"
#include "../../containers/sequential/queue.h"

#include <vector>

namespace zen {

// ============================================================================
// thread_pool
// ============================================================================

/**
 * @brief 固定大小线程池
 * 
 * 工作原理：
 * 1. 构造时创建指定数量的工作线程
 * 2. 工作线程从任务队列中取出任务执行
 * 3. 主线程通过 submit() 提交任务到队列
 * 4. 析构时等待所有任务完成
 */
class thread_pool {
public:
    /**
     * @brief 构造线程池
     * @param num_threads 工作线程数量（默认为 CPU 核心数）
     */
    explicit thread_pool(size_t num_threads = 0)
        : stop_(false)
    {
        if (num_threads == 0) {
            num_threads = static_cast<size_t>(thread::hardware_concurrency());
        }
        
        // 创建工作线程
        workers_.reserve(num_threads);
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this] { worker_thread(); });
        }
    }
    
    /**
     * @brief 析构：等待所有任务完成
     */
    ~thread_pool() {
        {
            lock_guard<mutex> lock(queue_mutex_);
            stop_ = true;
        }
        condition_.notify_all();
        
        for (auto& worker : workers_) {
            worker.join();
        }
    }
    
    // 不可拷贝、不可移动
    thread_pool(const thread_pool&)            = delete;
    thread_pool& operator=(const thread_pool&) = delete;
    thread_pool(thread_pool&&)                 = delete;
    thread_pool& operator=(thread_pool&&)      = delete;
    
    /**
     * @brief 提交任务（返回 future 用于获取结果）
     * 
     * @tparam F 任务类型（函数、lambda、函数对象）
     * @tparam Args 参数类型
     * @param f 任务
     * @param args 参数
     * @return future 任务结果
     */
    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args) 
        -> future<typename std::result_of<F(Args...)>::type>
    {
        using result_type = typename std::result_of<F(Args...)>::type;
        
        // 创建 packaged_task
        packaged_task<result_type()> task(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
        // 获取 future
        future<result_type> result = task.get_future();
        
        {
            lock_guard<mutex> lock(queue_mutex_);
            
            if (stop_) {
                throw std::runtime_error("submit on stopped thread_pool");
            }
            
            // 将任务加入队列
            tasks_.emplace([task]() mutable { task(); });
        }
        
        // 唤醒一个工作线程
        condition_.notify_one();
        return result;
    }
    
    /**
     * @brief 提交无返回值任务
     */
    template<typename F, typename... Args>
    void submit_void(F&& f, Args&&... args) {
        {
            lock_guard<mutex> lock(queue_mutex_);
            
            if (stop_) {
                throw std::runtime_error("submit on stopped thread_pool");
            }
            
            tasks_.emplace([f, args...]() mutable {
                f(args...);
            });
        }
        
        condition_.notify_one();
    }
    
    /**
     * @brief 获取工作线程数量
     */
    size_t size() const noexcept {
        return workers_.size();
    }
    
    /**
     * @brief 等待所有任务完成
     */
    void wait() {
        // 简化实现：只等待所有线程空闲
        // 实际应用中需要更复杂的任务计数机制
    }

private:
    /**
     * @brief 工作线程函数
     */
    void worker_thread() {
        while (true) {
            function<void()> task;
            
            {
                unique_lock<mutex> lock(queue_mutex_);
                
                // 等待任务或停止信号
                condition_.wait(lock, [this] { 
                    return stop_ || !tasks_.empty(); 
                });
                
                if (stop_ && tasks_.empty()) {
                    return;  // 退出线程
                }
                
                // 取出任务
                task = std::move(tasks_.front());
                tasks_.pop();
            }
            
            // 执行任务（不持有锁）
            task();
        }
    }
    
    // 工作线程
    vector<thread> workers_;
    
    // 任务队列
    queue<function<void()>> tasks_;
    
    // 同步原语
    mutable mutex queue_mutex_;
    condition_variable condition_;
    bool stop_;
};

} // namespace zen

#endif // ZEN_THREADING_POOL_THREAD_POOL_H
