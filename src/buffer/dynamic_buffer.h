/**
 * @file dynamic_buffer.h
 * @brief 动态缓冲区
 * 
 * dynamic_buffer 是动态扩容的字节缓冲区：
 * 
 * - push_back(): 添加字节
 * - append(): 追加数据
 * - data(): 获取数据指针
 * - size(): 获取数据大小
 * - capacity(): 获取容量
 * - reserve(): 预留容量
 * - clear(): 清空数据
 * - shrink_to_fit(): 缩小容量到实际大小
 * 
 * 特性：
 * - 自动扩容（指数增长）
 * - 移动语义
 * - 支持零拷贝操作
 * 
 * 示例：
 * @code
 * zen::dynamic_buffer buf;
 * 
 * buf.append("Hello", 5);
 * buf.append(" ", 1);
 * buf.append("World", 5);
 * 
 * std::cout << buf.data() << std::endl;  // "Hello World"
 * std::cout << buf.size() << std::endl;  // 11
 * @endcode
 */
#ifndef ZEN_BUFFER_DYNAMIC_BUFFER_H
#define ZEN_BUFFER_DYNAMIC_BUFFER_H

#include <cstdint>
#include <cstring>
#include <algorithm>
#include <initializer_list>

namespace zen {

// ============================================================================
// dynamic_buffer
// ============================================================================

/**
 * @brief 动态缓冲区
 */
class dynamic_buffer {
public:
    /**
     * @brief 默认构造
     */
    dynamic_buffer() : data_(nullptr), size_(0), capacity_(0) {}
    
    /**
     * @brief 指定初始容量构造
     */
    explicit dynamic_buffer(size_t initial_capacity) 
        : data_(nullptr), size_(0), capacity_(0) 
    {
        reserve(initial_capacity);
    }
    
    /**
     * @brief 从数据构造
     */
    dynamic_buffer(const void* data, size_t len) 
        : data_(nullptr), size_(0), capacity_(0) 
    {
        append(data, len);
    }
    
    /**
     * @brief 拷贝构造
     */
    dynamic_buffer(const dynamic_buffer& other) 
        : data_(nullptr), size_(0), capacity_(0) 
    {
        reserve(other.size_);
        append(other.data_, other.size_);
    }
    
    /**
     * @brief 移动构造
     */
    dynamic_buffer(dynamic_buffer&& other) noexcept
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_) 
    {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }
    
    /**
     * @brief 析构
     */
    ~dynamic_buffer() {
        deallocate();
    }
    
    /**
     * @brief 拷贝赋值
     */
    dynamic_buffer& operator=(const dynamic_buffer& other) {
        if (this != &other) {
            clear();
            reserve(other.size_);
            append(other.data_, other.size_);
        }
        return *this;
    }
    
    /**
     * @brief 移动赋值
     */
    dynamic_buffer& operator=(dynamic_buffer&& other) noexcept {
        if (this != &other) {
            deallocate();
            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }
    
    /**
     * @brief 获取数据指针
     */
    uint8_t* data() {
        return data_;
    }
    
    const uint8_t* data() const {
        return data_;
    }
    
    /**
     * @brief 获取数据大小
     */
    size_t size() const {
        return size_;
    }
    
    /**
     * @brief 获取容量
     */
    size_t capacity() const {
        return capacity_;
    }
    
    /**
     * @brief 检查是否为空
     */
    bool empty() const {
        return size_ == 0;
    }
    
    /**
     * @brief 检查是否为满
     */
    bool full() const {
        return size_ == capacity_;
    }
    
    /**
     * @brief 预留容量
     */
    void reserve(size_t new_capacity) {
        if (new_capacity <= capacity_) {
            return;
        }
        
        uint8_t* new_data = allocate(new_capacity);
        if (data_) {
            memcpy(new_data, data_, size_);
            deallocate();
        }
        data_ = new_data;
        capacity_ = new_capacity;
    }
    
    /**
     * @brief 调整大小
     */
    void resize(size_t new_size) {
        if (new_size > capacity_) {
            reserve(new_size);
        }
        size_ = new_size;
    }
    
    /**
     * @brief 清空数据（保留容量）
     */
    void clear() {
        size_ = 0;
    }
    
    /**
     * @brief 缩小容量到实际大小
     */
    void shrink_to_fit() {
        if (size_ == capacity_) {
            return;
        }
        
        if (size_ == 0) {
            deallocate();
            capacity_ = 0;
            return;
        }
        
        uint8_t* new_data = allocate(size_);
        memcpy(new_data, data_, size_);
        deallocate();
        data_ = new_data;
        capacity_ = size_;
    }
    
    /**
     * @brief 追加字节
     */
    void push_back(uint8_t value) {
        ensure_capacity(size_ + 1);
        data_[size_++] = value;
    }
    
    /**
     * @brief 追加数据
     */
    void append(const void* data, size_t len) {
        if (!data || len == 0) {
            return;
        }
        ensure_capacity(size_ + len);
        memcpy(data_ + size_, data, len);
        size_ += len;
    }
    
    /**
     * @brief 追加另一个缓冲区
     */
    void append(const dynamic_buffer& other) {
        append(other.data_, other.size_);
    }
    
    /**
     * @brief 填充数据
     */
    void fill(uint8_t value, size_t len) {
        resize(len);
        if (len > 0) {
            memset(data_, value, len);
        }
    }
    
    /**
     * @brief 读取指定偏移的数据
     */
    uint8_t read(size_t offset) const {
        if (offset >= size_) {
            return 0;
        }
        return data_[offset];
    }
    
    /**
     * @brief 写入指定偏移
     */
    void write(size_t offset, uint8_t value) {
        if (offset >= size_) {
            resize(offset + 1);
        }
        data_[offset] = value;
    }
    
    /**
     * @brief 移除前 n 个字节
     */
    void erase_front(size_t n) {
        if (n >= size_) {
            clear();
            return;
        }
        memmove(data_, data_ + n, size_ - n);
        size_ -= n;
    }
    
    /**
     * @brief 移除后 n 个字节
     */
    void erase_back(size_t n) {
        if (n >= size_) {
            clear();
            return;
        }
        size_ -= n;
    }
    
    /**
     * @brief 交换
     */
    void swap(dynamic_buffer& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

private:
    /**
     * @brief 确保有足够容量
     */
    void ensure_capacity(size_t required) {
        if (required <= capacity_) {
            return;
        }
        
        // 指数增长：至少翻倍
        size_t new_capacity = capacity_ == 0 ? 16 : capacity_ * 2;
        while (new_capacity < required) {
            new_capacity *= 2;
        }
        
        reserve(new_capacity);
    }
    
    /**
     * @brief 分配内存
     */
    static uint8_t* allocate(size_t size) {
        return new uint8_t[size];
    }
    
    /**
     * @brief 释放内存
     */
    void deallocate() {
        delete[] data_;
        data_ = nullptr;
    }
    
    uint8_t* data_;
    size_t size_;
    size_t capacity_;
};

} // namespace zen

#endif // ZEN_BUFFER_DYNAMIC_BUFFER_H
