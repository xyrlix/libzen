#pragma once

#include <functional>
#include <memory>
#include <queue>
#include <vector>
#include <chrono>
#include <atomic>

namespace zen {
namespace pool {

// 连接接口
template<typename T>
class connection {
public:
    using ptr = std::shared_ptr<T>;
    
    virtual ~connection() = default;
    
    // 检查连接是否有效
    virtual bool is_valid() const = 0;
    
    // 重新连接
    virtual bool reconnect() = 0;
    
    // 关闭连接
    virtual void close() = 0;
    
    // 获取最后使用时间
    std::chrono::system_clock::time_point get_last_used() const { return last_used_; }
    void touch() { last_used_ = std::chrono::system_clock::now(); }
    
protected:
    std::chrono::system_clock::time_point last_used_;
};

// 连接工厂
template<typename T>
class connection_factory {
public:
    virtual ~connection_factory() = default;
    virtual std::shared_ptr<T> create() = 0;
};

// 连接池配置
struct connection_pool_config {
    size_t min_connections = 1;
    size_t max_connections = 10;
    std::chrono::seconds idle_timeout{300};  // 空闲超时
    std::chrono::seconds connection_timeout{30};  // 连接超时
    bool validate_on_borrow = true;
    bool validate_on_return = false;
    size_t max_idle_connections = 5;
};

// 连接池
template<typename T>
class connection_pool {
public:
    using ptr = std::shared_ptr<connection_pool<T>>;
    using connection_ptr = std::shared_ptr<T>;
    using factory_ptr = std::shared_ptr<connection_factory<T>>;
    
    connection_pool(factory_ptr factory, const connection_pool_config& config = {});
    ~connection_pool();
    
    // 禁止拷贝
    connection_pool(const connection_pool&) = delete;
    connection_pool& operator=(const connection_pool&) = delete;
    
    // 获取连接
    connection_ptr acquire();
    connection_ptr acquire(std::chrono::milliseconds timeout);
    
    // 归还连接
    void release(connection_ptr conn);
    
    // 获取当前连接数
    size_t get_size() const;
    size_t get_idle_size() const;
    size_t get_active_size() const;
    
    // 清理空闲连接
    void cleanup_idle();
    void cleanup_all();
    
    // 执行函数（自动管理连接）
    template<typename Func>
    auto execute(Func&& func) -> decltype(func(std::declval<connection_ptr>())) {
        auto conn = acquire();
        return func(conn);
    }
    
    // 检查健康
    void check_health();
    
    // 获取配置
    const connection_pool_config& get_config() const { return config_; }
    
private:
    // 创建连接
    connection_ptr create_connection();
    
    // 验证连接
    bool validate_connection(connection_ptr conn);
    
    // 清理线程
    void cleanup_thread_func();
    
    factory_ptr factory_;
    connection_pool_config config_;
    
    std::queue<connection_ptr> idle_connections_;
    std::vector<connection_ptr> active_connections_;
    
    threading::mutex pool_mutex_;
    threading::condition_variable available_cv_;
    
    std::atomic<size_t> total_connections_;
    
    std::thread cleanup_thread_;
    std::atomic<bool> cleanup_running_;
};

// 连接包装器（RAII 自动归还）
template<typename T>
class connection_holder {
public:
    connection_holder(typename connection_pool<T>::ptr pool, std::shared_ptr<T> conn)
        : pool_(pool), conn_(conn) {}
    
    ~connection_holder() {
        if (pool_ && conn_) {
            pool_->release(conn_);
        }
    }
    
    // 禁止拷贝
    connection_holder(const connection_holder&) = delete;
    connection_holder& operator=(const connection_holder&) = delete;
    
    // 允许移动
    connection_holder(connection_holder&& other) noexcept
        : pool_(std::move(other.pool_)), conn_(std::move(other.conn_)) {
        other.pool_ = nullptr;
        other.conn_ = nullptr;
    }
    
    connection_holder& operator=(connection_holder&& other) noexcept {
        if (this != &other) {
            if (pool_ && conn_) {
                pool_->release(conn_);
            }
            pool_ = std::move(other.pool_);
            conn_ = std::move(other.conn_);
            other.pool_ = nullptr;
            other.conn_ = nullptr;
        }
        return *this;
    }
    
    // 获取连接
    std::shared_ptr<T> get() const { return conn_; }
    T* operator->() const { return conn_.get(); }
    T& operator*() const { return *conn_; }
    
    // 释放所有权
    std::shared_ptr<T> release() {
        auto conn = std::move(conn_);
        pool_ = nullptr;
        return conn;
    }
    
private:
    typename connection_pool<T>::ptr pool_;
    std::shared_ptr<T> conn_;
};

} // namespace pool
} // namespace zen
