#ifndef ZEN_UTILITY_STRING_VIEW_H
#define ZEN_UTILITY_STRING_VIEW_H

#pragma once
#include <cstddef>      // size_t, ptrdiff_t
#include <cstring>      // strlen, memcmp, memchr
#include <stdexcept>    // out_of_range

namespace zen {

// ============================================================================
// string_view - 非拥有字符串视图
// ============================================================================
/**
 * @brief 对字符串的轻量只读视图（不拥有内存，不分配内存）
 *
 * 等价于 std::string_view，但完全自实现。
 * 适合函数参数、字符串切片、零拷贝字符串处理。
 *
 * 内部仅持有：
 *   - const char* ptr_  指向字符串起始位置
 *   - size_t      len_  字符串长度（不含 '\0'）
 *
 * 示例：
 * @code
 * zen::string_view sv = "hello world";
 * zen::string_view sub = sv.substr(6, 5);  // "world"
 * bool found = sv.find("world") != zen::string_view::npos;
 * @endcode
 */
class string_view {
public:
    // -------------------------------------------------------------------------
    // 类型定义
    // -------------------------------------------------------------------------
    using value_type      = char;
    using pointer         = char*;
    using const_pointer   = const char*;
    using reference       = char&;
    using const_reference = const char&;
    using const_iterator  = const char*;
    using iterator        = const_iterator;
    using size_type       = size_t;
    using difference_type = ptrdiff_t;

    static constexpr size_type npos = static_cast<size_type>(-1);

    // -------------------------------------------------------------------------
    // 构造函数
    // -------------------------------------------------------------------------

    /** @brief 默认构造：空视图 */
    constexpr string_view() noexcept
        : ptr_(nullptr), len_(0) {}

    /** @brief 从 C 字符串构造（自动计算长度） */
    string_view(const char* s) noexcept
        : ptr_(s), len_(s ? strlen(s) : 0) {}

    /** @brief 从指针+长度构造 */
    constexpr string_view(const char* s, size_type count) noexcept
        : ptr_(s), len_(count) {}

    /** @brief 拷贝构造 */
    constexpr string_view(const string_view&) noexcept = default;

    /** @brief 赋值运算符 */
    string_view& operator=(const string_view&) noexcept = default;

    // -------------------------------------------------------------------------
    // 迭代器
    // -------------------------------------------------------------------------
    constexpr const_iterator begin()  const noexcept { return ptr_; }
    constexpr const_iterator end()    const noexcept { return ptr_ + len_; }
    constexpr const_iterator cbegin() const noexcept { return ptr_; }
    constexpr const_iterator cend()   const noexcept { return ptr_ + len_; }

    // -------------------------------------------------------------------------
    // 元素访问
    // -------------------------------------------------------------------------

    /** @brief 下标访问（无边界检查） */
    constexpr const_reference operator[](size_type pos) const noexcept {
        return ptr_[pos];
    }

    /** @brief 带边界检查的访问 */
    const_reference at(size_type pos) const {
        if (pos >= len_) throw std::out_of_range("string_view::at");
        return ptr_[pos];
    }

    constexpr const_reference front() const noexcept { return ptr_[0]; }
    constexpr const_reference back()  const noexcept { return ptr_[len_ - 1]; }
    constexpr const_pointer   data()  const noexcept { return ptr_; }

    // -------------------------------------------------------------------------
    // 容量
    // -------------------------------------------------------------------------
    constexpr size_type size()   const noexcept { return len_; }
    constexpr size_type length() const noexcept { return len_; }
    constexpr bool      empty()  const noexcept { return len_ == 0; }

    // -------------------------------------------------------------------------
    // 修改器（只修改视图范围，不修改内存）
    // -------------------------------------------------------------------------

    /** @brief 从前端移除 n 个字符 */
    void remove_prefix(size_type n) noexcept {
        ptr_ += n;
        len_ -= n;
    }

    /** @brief 从后端移除 n 个字符 */
    void remove_suffix(size_type n) noexcept {
        len_ -= n;
    }

    /** @brief 交换两个视图 */
    void swap(string_view& other) noexcept {
        const char* tmp_ptr = ptr_;
        size_type   tmp_len = len_;
        ptr_ = other.ptr_;
        len_ = other.len_;
        other.ptr_ = tmp_ptr;
        other.len_ = tmp_len;
    }

    // -------------------------------------------------------------------------
    // 子视图
    // -------------------------------------------------------------------------

    /**
     * @brief 返回子字符串视图
     * @param pos 起始位置
     * @param count 长度（默认到末尾）
     */
    string_view substr(size_type pos = 0, size_type count = npos) const {
        if (pos > len_) throw std::out_of_range("string_view::substr");
        size_type rlen = (count == npos || count > len_ - pos) ? (len_ - pos) : count;
        return string_view(ptr_ + pos, rlen);
    }

    // -------------------------------------------------------------------------
    // 比较
    // -------------------------------------------------------------------------

    int compare(string_view other) const noexcept {
        size_type min_len = len_ < other.len_ ? len_ : other.len_;
        int r = memcmp(ptr_, other.ptr_, min_len);
        if (r != 0) return r;
        if (len_ < other.len_) return -1;
        if (len_ > other.len_) return  1;
        return 0;
    }

    int compare(size_type pos, size_type count, string_view other) const {
        return substr(pos, count).compare(other);
    }

    // -------------------------------------------------------------------------
    // 查找
    // -------------------------------------------------------------------------

    /**
     * @brief 查找子串，返回第一次出现的位置
     * @return 找到时返回位置，找不到返回 npos
     */
    size_type find(string_view sv, size_type pos = 0) const noexcept {
        if (sv.empty()) return pos <= len_ ? pos : npos;
        if (sv.len_ > len_) return npos;

        for (size_type i = pos; i + sv.len_ <= len_; ++i) {
            if (memcmp(ptr_ + i, sv.ptr_, sv.len_) == 0) {
                return i;
            }
        }
        return npos;
    }

    /** @brief 查找单个字符 */
    size_type find(char c, size_type pos = 0) const noexcept {
        for (size_type i = pos; i < len_; ++i) {
            if (ptr_[i] == c) return i;
        }
        return npos;
    }

    /**
     * @brief 反向查找子串
     */
    size_type rfind(string_view sv, size_type pos = npos) const noexcept {
        if (sv.empty()) return len_;
        if (sv.len_ > len_) return npos;

        size_type start = len_ - sv.len_;
        if (pos < start) start = pos;

        for (size_type i = start + 1; i-- > 0; ) {
            if (memcmp(ptr_ + i, sv.ptr_, sv.len_) == 0) {
                return i;
            }
        }
        return npos;
    }

    /** @brief 反向查找字符 */
    size_type rfind(char c, size_type pos = npos) const noexcept {
        size_type start = (pos >= len_) ? len_ : pos + 1;
        for (size_type i = start; i-- > 0; ) {
            if (ptr_[i] == c) return i;
        }
        return npos;
    }

    /** @brief 查找第一个属于字符集的字符 */
    size_type find_first_of(string_view chars, size_type pos = 0) const noexcept {
        for (size_type i = pos; i < len_; ++i) {
            for (size_type j = 0; j < chars.len_; ++j) {
                if (ptr_[i] == chars.ptr_[j]) return i;
            }
        }
        return npos;
    }

    /** @brief 查找第一个不属于字符集的字符 */
    size_type find_first_not_of(string_view chars, size_type pos = 0) const noexcept {
        for (size_type i = pos; i < len_; ++i) {
            bool found = false;
            for (size_type j = 0; j < chars.len_; ++j) {
                if (ptr_[i] == chars.ptr_[j]) { found = true; break; }
            }
            if (!found) return i;
        }
        return npos;
    }

    // -------------------------------------------------------------------------
    // 前缀/后缀检测
    // -------------------------------------------------------------------------

    bool starts_with(string_view sv) const noexcept {
        return len_ >= sv.len_ && memcmp(ptr_, sv.ptr_, sv.len_) == 0;
    }

    bool ends_with(string_view sv) const noexcept {
        return len_ >= sv.len_ &&
               memcmp(ptr_ + len_ - sv.len_, sv.ptr_, sv.len_) == 0;
    }

    bool contains(string_view sv) const noexcept {
        return find(sv) != npos;
    }

    // -------------------------------------------------------------------------
    // 比较运算符
    // -------------------------------------------------------------------------
    bool operator==(string_view other) const noexcept {
        return len_ == other.len_ && (len_ == 0 || memcmp(ptr_, other.ptr_, len_) == 0);
    }
    bool operator!=(string_view other) const noexcept { return !(*this == other); }
    bool operator< (string_view other) const noexcept { return compare(other) < 0; }
    bool operator> (string_view other) const noexcept { return compare(other) > 0; }
    bool operator<=(string_view other) const noexcept { return compare(other) <= 0; }
    bool operator>=(string_view other) const noexcept { return compare(other) >= 0; }

private:
    const char* ptr_;
    size_type   len_;
};

// 非成员 swap
inline void swap(string_view& a, string_view& b) noexcept { a.swap(b); }

} // namespace zen

#endif // ZEN_UTILITY_STRING_VIEW_H
