/**
 * @file ring_buffer.h
 * @brief 环形缓冲区
 * 
 * ring_buffer 是固定大小的环形缓冲区：
 * 
 * - write(): 写入数据
 * - read(): 读取数据
 * - peek(): 查看数据但不消费
 * - size(): 可读数据大小
 * - capacity(): 缓冲区容量
 * - clear(): 清空缓冲区
 * 
 * 特性：
 * - 固定容量，无需内存分配
 * - 零拷贝操作
 * - 线程不安全（需要外部同步）
 * 
 * 示例：
 * @code
 * zen::ring_buffer<1024> buf;
 * 
 * buf.write("Hello", 5);
 * buf.write(" ", 1);
 * buf.write("World", 5);
 * 
 * char output[11];
 * buf.read(output, 11);
 * 
 * std::cout << output << std::endl;  // "Hello World"
 * @endcode
 */
#ifndef ZEN_BUFFER_RING_BUFFER_H
#define ZEN_BUFFER_RING_BUFFER_H

#include <cstdint>
#include <cstring>
#include <algorithm>

namespace zen {

// ============================================================================
// ring_buffer
// ============================================================================

/**
 * @brief 环形缓冲区
 * 
 * @tparam Capacity 缓冲区容量（字节数）
 */
template<size_t Capacity>
class ring_buffer {
public:
    /**
     * @brief 默认构造
     */
    ring_buffer() : read_pos_(0), write_pos_(0), size_(0) {}
    
    /**
     * @brief 获取可读数据大小
     */
    size_t size() const {
        return size_;
    }
    
    /**
     * @brief 获取可写空间大小
     */
    size_t writable_size() const {
        return Capacity - size_;
    }
    
    /**
     * @brief 获取容量
     */
    size_t capacity() const {
        return Capacity;
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
        return size_ == Capacity;
    }
    
    /**
     * @brief 清空缓冲区
     */
    void clear() {
        read_pos_ = 0;
        write_pos_ = 0;
        size_ = 0;
    }
    
    /**
     * @brief 写入数据
     * @param data 数据指针
     * @param len 数据长度
     * @return 实际写入的字节数
     */
    size_t write(const void* data, size_t len) {
        if (!data || len == 0 || full()) {
            return 0;
        }
        
        len = std::min(len, writable_size());
        const uint8_t* src = static_cast<const uint8_t*>(data);
        
        // 可能需要分两次写入（绕回到开头）
        size_t first_chunk = std::min(len, Capacity - write_pos_);
        memcpy(buffer_ + write_pos_, src, first_chunk);
        
        if (len > first_chunk) {
            memcpy(buffer_, src + first_chunk, len - first_chunk);
        }
        
        write_pos_ = (write_pos_ + len) % Capacity;
        size_ += len;
        
        return len;
    }
    
    /**
     * @brief 读取数据
     * @param data 数据指针
     * @param len 数据长度
     * @return 实际读取的字节数
     */
    size_t read(void* data, size_t len) {
        if (!data || len == 0 || empty()) {
            return 0;
        }
        
        len = std::min(len, size_);
        uint8_t* dst = static_cast<uint8_t*>(data);
        
        // 可能需要分两次读取（绕回到开头）
        size_t first_chunk = std::min(len, Capacity - read_pos_);
        memcpy(dst, buffer_ + read_pos_, first_chunk);
        
        if (len > first_chunk) {
            memcpy(dst + first_chunk, buffer_, len - first_chunk);
        }
        
        read_pos_ = (read_pos_ + len) % Capacity;
        size_ -= len;
        
        return len;
    }
    
    /**
     * @brief 查看数据但不消费
     * @param data 数据指针
     * @param len 数据长度
     * @return 实际查看的字节数
     */
    size_t peek(void* data, size_t len, size_t offset = 0) const {
        if (!data || len == 0 || empty() || offset >= size_) {
            return 0;
        }
        
        size_t available = size_ - offset;
        len = std::min(len, available);
        uint8_t* dst = static_cast<uint8_t*>(data);
        
        size_t start = (read_pos_ + offset) % Capacity;
        
        size_t first_chunk = std::min(len, Capacity - start);
        memcpy(dst, buffer_ + start, first_chunk);
        
        if (len > first_chunk) {
            memcpy(dst + first_chunk, buffer_, len - first_chunk);
        }
        
        return len;
    }
    
    /**
     * @brief 跳过指定字节数
     */
    size_t skip(size_t len) {
        if (empty()) {
            return 0;
        }
        
        len = std::min(len, size_);
        read_pos_ = (read_pos_ + len) % Capacity;
        size_ -= len;
        
        return len;
    }
    
    /**
     * @brief 获取连续可读指针（可能需要两次读取）
     * @param ptr 输出指针
     * @param len 输出长度
     * @return 是否还有数据（true 表示需要第二次读取）
     */
    bool read_ptr(const uint8_t*& ptr, size_t& len) const {
        if (empty()) {
            ptr = nullptr;
            len = 0;
            return false;
        }
        
        ptr = buffer_ + read_pos_;
        len = std::min(size_, Capacity - read_pos_);
        
        return size_ > len;
    }
    
    /**
     * @brief 获取连续可写指针（可能需要两次写入）
     * @param ptr 输出指针
     * @param len 输出长度
     * @return 是否还有空间（true 表示需要第二次写入）
     */
    bool write_ptr(uint8_t*& ptr, size_t& len) {
        if (full()) {
            ptr = nullptr;
            len = 0;
            return false;
        }
        
        ptr = buffer_ + write_pos_;
        len = std::min(writable_size(), Capacity - write_pos_);
        
        return writable_size() > len;
    }
    
    /**
     * @brief 提交写入（调用 write_ptr 后调用）
     * @param len 提交的字节数
     */
    void commit_write(size_t len) {
        len = std::min(len, writable_size());
        write_pos_ = (write_pos_ + len) % Capacity;
        size_ += len;
    }
    
    /**
     * @brief 提交读取（调用 read_ptr 后调用）
     * @param len 提交的字节数
     */
    void commit_read(size_t len) {
        len = std::min(len, size_);
        read_pos_ = (read_pos_ + len) % Capacity;
        size_ -= len;
    }

private:
    uint8_t buffer_[Capacity];
    size_t read_pos_;
    size_t write_pos_;
    size_t size_;
};

} // namespace zen

#endif // ZEN_BUFFER_RING_BUFFER_H
