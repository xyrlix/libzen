#include "buffer/dynamic_buffer.h"
#include <cstring>
#include <algorithm>

namespace zen {

dynamic_buffer::dynamic_buffer(size_t initial_capacity)
    : data_(nullptr), size_(0), capacity_(0) {
    reserve(initial_capacity);
}

dynamic_buffer::~dynamic_buffer() {
    if (data_) {
        ::free(data_);
    }
}

dynamic_buffer::dynamic_buffer(dynamic_buffer&& other) 
    : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
    other.data_ = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;
}

dynamic_buffer& dynamic_buffer::operator=(dynamic_buffer&& other) {
    if (this != &other) {
        if (data_) {
            ::free(data_);
        }
        data_ = other.data_;
        size_ = other.size_;
        capacity_ = other.capacity_;
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }
    return *this;
}

void dynamic_buffer::reserve(size_t new_capacity) {
    if (new_capacity <= capacity_) return;
    
    size_t actual_capacity = std::max(new_capacity, capacity_ * 2);
    if (actual_capacity < 64) actual_capacity = 64;
    
    void* new_data = ::realloc(data_, actual_capacity);
    if (!new_data) {
        throw std::bad_alloc();
    }
    
    data_ = static_cast<char*>(new_data);
    capacity_ = actual_capacity;
}

void dynamic_buffer::resize(size_t new_size) {
    if (new_size > capacity_) {
        reserve(new_size);
    }
    size_ = new_size;
}

void dynamic_buffer::push_back(char c) {
    if (size_ + 1 > capacity_) {
        reserve(size_ + 1);
    }
    data_[size_++] = c;
}

void dynamic_buffer::append(const void* data, size_t size) {
    if (size_ + size > capacity_) {
        reserve(size_ + size);
    }
    std::memcpy(data_ + size_, data, size);
    size_ += size;
}

void dynamic_buffer::append(const dynamic_buffer& other) {
    append(other.data_, other.size_);
}

size_t dynamic_buffer::read(size_t offset, void* buf, size_t size) const {
    if (offset >= size_) return 0;
    size_t readable = std::min(size, size_ - offset);
    std::memcpy(buf, data_ + offset, readable);
    return readable;
}

size_t dynamic_buffer::write(size_t offset, const void* buf, size_t size) {
    size_t required = offset + size;
    if (required > capacity_) {
        reserve(required);
    }
    std::memcpy(data_ + offset, buf, size);
    if (required > size_) {
        size_ = required;
    }
    return size;
}

void dynamic_buffer::erase_front(size_t count) {
    if (count >= size_) {
        size_ = 0;
        return;
    }
    size_t remain = size_ - count;
    std::memmove(data_, data_ + count, remain);
    size_ = remain;
}

void dynamic_buffer::erase_back(size_t count) {
    if (count >= size_) {
        size_ = 0;
        return;
    }
    size_ -= count;
}

void dynamic_buffer::clear() {
    size_ = 0;
}

void dynamic_buffer::shrink_to_fit() {
    if (size_ < capacity_) {
        if (size_ == 0) {
            ::free(data_);
            data_ = nullptr;
            capacity_ = 0;
        } else {
            void* new_data = ::realloc(data_, size_);
            if (new_data) {
                data_ = static_cast<char*>(new_data);
                capacity_ = size_;
            }
        }
    }
}

char* dynamic_buffer::data() {
    return data_;
}

const char* dynamic_buffer::data() const {
    return data_;
}

size_t dynamic_buffer::size() const {
    return size_;
}

size_t dynamic_buffer::capacity() const {
    return capacity_;
}

bool dynamic_buffer::empty() const {
    return size_ == 0;
}

char& dynamic_buffer::operator[](size_t index) {
    return data_[index];
}

const char& dynamic_buffer::operator[](size_t index) const {
    return data_[index];
}

} // namespace zen
