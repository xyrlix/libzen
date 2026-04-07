#pragma once

#include <functional>
#include <memory>
#include <stack>
#include <vector>
#include <atomic>

namespace zen {
namespace pool {

// 对象池配置
struct object_pool_config {
    size_t initial_size = 10;
    size_t max_size = 100;
    bool thread_safe = true;
    bool auto_expand = true;
    bool auto_shrink = false;
    float shrink_ratio = 0.5f;
    size_t shrink_threshold = 50;
};

// 对象池
template<typename T>
class object_pool {
public:
    using ptr = std::shared_ptr<object_pool<T>>;
    using object_ptr = std::unique_ptr<T>;
    using create_func = std::function<object_ptr()>;
    using reset_func = std::function<void(T*)>;
    
    object_pool(create_func creator, const object_pool_config& config = {});
    ~object_pool();
    
    // 禁止拷贝
    object_pool(const object_pool&) = delete;
    object_pool& operator=(const object_pool&) = delete;
    
    // 获取对象
    object_ptr acquire();
    
    // 归还对象
    void release(object_ptr obj);
    
    // 设置重置函数
    void set_reset_func(reset_func func) { reset_func_ = func; }
    
    // 获取统计
    size_t get_size() const;
    size_t get_active_count() const;
    size_t get_idle_count() const;
    
    // 预分配
    void preallocate(size_t count);
    
    // 清理
    void clear();
    void shrink();
    
    // 检查健康
    void check_health();
    
    // 获取配置
    const object_pool_config& get_config() const { return config_; }
    
private:
    // 创建对象
    object_ptr create_object();
    
    // 重置对象
    void reset_object(T* obj);
    
    create_func create_func_;
    reset_func reset_func_;
    object_pool_config config_;
    
    std::stack<object_ptr> idle_objects_;
    std::atomic<size_t> active_count_;
    
    mutable threading::mutex pool_mutex_;
};

// 对象包装器（RAII 自动归还）
template<typename T>
class object_holder {
public:
    using pool_ptr = std::shared_ptr<object_pool<T>>;
    
    object_holder(pool_ptr pool, std::unique_ptr<T> obj)
        : pool_(pool), obj_(std::move(obj)) {}
    
    ~object_holder() {
        if (pool_ && obj_) {
            pool_->release(std::move(obj_));
        }
    }
    
    // 禁止拷贝
    object_holder(const object_holder&) = delete;
    object_holder& operator=(const object_holder&) = delete;
    
    // 允许移动
    object_holder(object_holder&& other) noexcept
        : pool_(std::move(other.pool_)), obj_(std::move(other.obj_)) {
        other.pool_ = nullptr;
        other.obj_ = nullptr;
    }
    
    object_holder& operator=(object_holder&& other) noexcept {
        if (this != &other) {
            if (pool_ && obj_) {
                pool_->release(std::move(obj_));
            }
            pool_ = std::move(other.pool_);
            obj_ = std::move(other.obj_);
            other.pool_ = nullptr;
            other.obj_ = nullptr;
        }
        return *this;
    }
    
    // 获取对象
    T* get() const { return obj_.get(); }
    T* operator->() const { return obj_.get(); }
    T& operator*() const { return *obj_; }
    
    // 释放所有权
    std::unique_ptr<T> release() {
        auto obj = std::move(obj_);
        pool_ = nullptr;
        return obj;
    }
    
private:
    pool_ptr pool_;
    std::unique_ptr<T> obj_;
};

// 简单对象池（使用默认构造函数）
template<typename T>
class simple_object_pool {
public:
    simple_object_pool(size_t initial_size = 10, size_t max_size = 100)
        : pool_(
            []() { return std::make_unique<T>(); },
            {initial_size, max_size, true, true}
        ) {}
    
    auto acquire() { return pool_->acquire(); }
    
    size_t get_size() const { return pool_->get_size(); }
    size_t get_active_count() const { return pool_->get_active_count(); }
    
private:
    object_pool<T> pool_;
};

} // namespace pool
} // namespace zen
