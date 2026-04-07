#include "buffer/ring_buffer.h"
#include <cstring>
#include <algorithm>

namespace zen {

ring_buffer::ring_buffer(size_t capacity)
    : data_(nullptr), capacity_(capacity), read_pos_(0), write_pos_(0) {
    if (capacity > 0 && (capacity & (capacity - 1)) != 0) {
        capacity_ = 1;
        while (capacity_ < capacity) capacity_ <<= 1;
    }
    data_ = new char[capacity_];
}

ring_buffer::~ring_buffer() {
    delete[] data_;
}

ring_buffer::ring_buffer(ring_buffer&& other)
    : data_(other.data_), capacity_(other.capacity_),
      read_pos_(other.read_pos_), write_pos_(other.write_pos_) {
    other.data_ = nullptr;
    other.capacity_ = 0;
    other.read_pos_ = 0;
    other.write_pos_ = 0;
}

ring_buffer& ring_buffer::operator=(ring_buffer&& other) {
    if (this != &other) {
        delete[] data_;
        data_ = other.data_;
        capacity_ = other.capacity_;
        read_pos_ = other.read_pos_;
        write_pos_ = other.write_pos_;
        other.data_ = nullptr;
        other.capacity_ = 0;
        other.read_pos_ = 0;
        other.write_pos_ = 0;
    }
    return *this;
}

size_t ring_buffer::write(const void* data, size_t size) {
    size_t writable = std::min(size, capacity_ - size());
    
    size_t write_pos = write_pos_ & (capacity_ - 1);
    size_t readable = read_pos_ & (capacity_ - 1);
    
    if (write_pos >= readable) {
        size_t first = std::min(writable, capacity_ - write_pos);
        std::memcpy(data_ + write_pos, data, first);
        if (writable > first) {
            std::memcpy(data_, static_cast<const char*>(data) + first, writable - first);
        }
    } else {
        std::memcpy(data_ + write_pos, data, writable);
    }
    
    write_pos_ += writable;
    return writable;
}

size_t ring_buffer::read(void* data, size_t size) {
    size_t readable = std::min(size, this->size());
    
    if (readable == 0) return 0;
    
    size_t read_pos = read_pos_ & (capacity_ - 1);
    
    if (read_pos + readable <= capacity_) {
        std::memcpy(data, data_ + read_pos, readable);
    } else {
        size_t first = capacity_ - read_pos;
        std::memcpy(data, data_ + read_pos, first);
        std::memcpy(static_cast<char*>(data) + first, data_, readable - first);
    }
    
    read_pos_ += readable;
    return readable;
}

size_t ring_buffer::peek(void* data, size_t size) const {
    size_t readable = std::min(size, this->size());
    
    if (readable == 0) return 0;
    
    size_t read_pos = read_pos_ & (capacity_ - 1);
    
    if (read_pos + readable <= capacity_) {
        std::memcpy(data, data_ + read_pos, readable);
    } else {
        size_t first = capacity_ - read_pos;
        std::memcpy(data, data_ + read_pos, first);
        std::memcpy(static_cast<char*>(data) + first, data_, readable - first);
    }
    
    return readable;
}

void ring_buffer::skip(size_t size) {
    size_t skippable = std::min(size, this->size());
    read_pos_ += skippable;
}

size_t ring_buffer::read_ptr(void** ptr1, size_t* size1, void** ptr2, size_t* size2) const {
    size_t readable = this->size();
    
    if (readable == 0) {
        if (ptr1) *ptr1 = nullptr;
        if (size1) *size1 = 0;
        if (ptr2) *ptr2 = nullptr;
        if (size2) *size2 = 0;
        return 0;
    }
    
    size_t read_pos = read_pos_ & (capacity_ - 1);
    
    if (ptr1) *ptr1 = data_ + read_pos;
    
    if (read_pos + readable <= capacity_) {
        if (size1) *size1 = readable;
        if (ptr2) *ptr2 = nullptr;
        if (size2) *size2 = 0;
    } else {
        size_t first = capacity_ - read_pos;
        if (size1) *size1 = first;
        if (ptr2) *ptr2 = data_;
        if (size2) *size2 = readable - first;
    }
    
    return readable;
}

size_t ring_buffer::write_ptr(void** ptr1, size_t* size1, void** ptr2, size_t* size2) {
    size_t writable = capacity_ - size();
    
    if (writable == 0) {
        if (ptr1) *ptr1 = nullptr;
        if (size1) *size1 = 0;
        if (ptr2) *ptr2 = nullptr;
        if (size2) *size2 = 0;
        return 0;
    }
    
    size_t write_pos = write_pos_ & (capacity_ - 1);
    
    if (ptr1) *ptr1 = data_ + write_pos;
    
    if (write_pos + writable <= capacity_) {
        if (size1) *size1 = writable;
        if (ptr2) *ptr2 = nullptr;
        if (size2) *size2 = 0;
    } else {
        size_t first = capacity_ - write_pos;
        if (size1) *size1 = first;
        if (ptr2) *ptr2 = data_;
        if (size2) *size2 = writable - first;
    }
    
    return writable;
}

void ring_buffer::commit_read(size_t size) {
    size_t skippable = std::min(size, this->size());
    read_pos_ += skippable;
}

void ring_buffer::commit_write(size_t size) {
    size_t commitable = std::min(size, capacity_ - this->size());
    write_pos_ += commitable;
}

size_t ring_buffer::size() const {
    return write_pos_ - read_pos_;
}

size_t ring_buffer::capacity() const {
    return capacity_;
}

bool ring_buffer::empty() const {
    return read_pos_ == write_pos_;
}

bool ring_buffer::full() const {
    return size() == capacity_;
}

void ring_buffer::clear() {
    read_pos_ = 0;
    write_pos_ = 0;
}

} // namespace zen
