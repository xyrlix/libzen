/**
 * @file directory.h
 * @brief 目录遍历
 * 
 * directory 提供目录遍历功能：
 * 
 * - directory_iterator: 遍历目录中的条目（单向迭代器）
 * - recursive_directory_iterator: 递归遍历子目录
 * 
 * 示例：
 * @code
 * // 遍历当前目录
 * for (auto& entry : zen::directory_iterator(".")) {
 *     std::cout << entry.path() << std::endl;
 * }
 * 
 * // 递归遍历
 * for (auto& entry : zen::recursive_directory_iterator("/usr")) {
 *     std::cout << entry.path() << std::endl;
 * }
 * @endcode
 */
#ifndef ZEN_FILESYSTEM_DIRECTORY_H
#define ZEN_FILESYSTEM_DIRECTORY_H

#include "path.h"
#include <dirent.h>
#include <string>
#include <stack>

namespace zen {

// ============================================================================
// directory_entry
// ============================================================================

/**
 * @brief 目录条目
 */
class directory_entry {
public:
    directory_entry() = default;
    
    explicit directory_entry(const path& p) : path_(p) {}
    
    /**
     * @brief 获取路径
     */
    const path& path() const {
        return path_;
    }
    
    /**
     * @brief 获取文件名
     */
    std::string filename() const {
        return path_.filename().string();
    }
    
    /**
     * @brief 是否为目录
     */
    bool is_directory() const {
        return path_.is_directory();
    }
    
    /**
     * @brief 是否为普通文件
     */
    bool is_file() const {
        return path_.is_file();
    }
    
    /**
     * @brief 是否为符号链接
     */
    bool is_symlink() const;

    /**
     * @brief 文件大小
     */
    uintmax_t file_size() const;

    /**
     * @brief 最后修改时间
     */
    std::time_t last_write_time() const;

private:
    path path_;
};

// ============================================================================
// directory_iterator
// ============================================================================

/**
 * @brief 目录迭代器（单向迭代器）
 */
class directory_iterator {
public:
    using iterator_category = std::input_iterator_tag;
    using value_type = directory_entry;
    using difference_type = std::ptrdiff_t;
    using pointer = const directory_entry*;
    using reference = const directory_entry&;
    
    /**
     * @brief 默认构造（end 迭代器）
     */
    directory_iterator() : dir_(nullptr), entry_() {}
    
    /**
     * @brief 构造并打开目录
     */
    explicit directory_iterator(const path& p);
    
    /**
     * @brief 析构
     */
    ~directory_iterator();
    
    /**
     * @brief 拷贝/移动
     */
    directory_iterator(const directory_iterator& other);
    directory_iterator(directory_iterator&& other) noexcept;
    
    directory_iterator& operator=(const directory_iterator& other);
    directory_iterator& operator=(directory_iterator&& other) noexcept;
    
    /**
     * @brief 解引用
     */
    reference operator*() const {
        return entry_;
    }
    
    pointer operator->() const {
        return &entry_;
    }
    
    /**
     * @brief 前置递增
     */
    directory_iterator& operator++();
    
    /**
     * @brief 后置递增
     */
    directory_iterator operator++(int) {
        directory_iterator tmp = *this;
        ++(*this);
        return tmp;
    }
    
    /**
     * @brief 相等比较
     */
    bool operator==(const directory_iterator& other) const {
        if (!dir_ && !other.dir_) return true;
        if (!dir_ || !other.dir_) return false;
        return dir_ == other.dir_;
    }
    
    bool operator!=(const directory_iterator& other) const {
        return !(*this == other);
    }

private:
    void increment();
    
    DIR* dir_;
    directory_entry entry_;
};

// ============================================================================
// recursive_directory_iterator
// ============================================================================

/**
 * @brief 递归目录迭代器
 */
class recursive_directory_iterator {
public:
    using iterator_category = std::input_iterator_tag;
    using value_type = directory_entry;
    using difference_type = std::ptrdiff_t;
    using pointer = const directory_entry*;
    using reference = const directory_entry&;
    
    /**
     * @brief 默认构造（end 迭代器）
     */
    recursive_directory_iterator() : depth_(0) {}
    
    /**
     * @brief 构造并开始递归遍历
     */
    explicit recursive_directory_iterator(const path& p);
    
    /**
     * @brief 解引用
     */
    reference operator*() const {
        return current_;
    }
    
    pointer operator->() const {
        return &current_;
    }
    
    /**
     * @brief 前置递增
     */
    recursive_directory_iterator& operator++();
    
    /**
     * @brief 后置递增
     */
    recursive_directory_iterator operator++(int) {
        recursive_directory_iterator tmp = *this;
        ++(*this);
        return tmp;
    }
    
    /**
     * @brief 相等比较
     */
    bool operator==(const recursive_directory_iterator& other) const {
        return stack_.empty() && other.stack_.empty();
    }
    
    bool operator!=(const recursive_directory_iterator& other) const {
        return !(*this == other);
    }
    
    /**
     * @brief 当前深度
     */
    int depth() const {
        return depth_;
    }

private:
    void increment();
    
    std::stack<directory_iterator> stack_;
    directory_entry current_;
    int depth_;
};

// ============================================================================
// 实用函数
// ============================================================================

/**
 * @brief 创建目录
 */
bool create_directory(const path& p);

/**
 * @brief 递归创建目录
 */
bool create_directories(const path& p);

/**
 * @brief 删除目录（必须为空）
 */
bool remove_directory(const path& p);

/**
 * @brief 递归删除目录
 */
bool remove_all(const path& p);

/**
 * @brief 获取当前工作目录
 */
path current_path();

/**
 * @brief 设置当前工作目录
 */
bool current_path(const path& p);

/**
 * @brief 判断路径是否存在
 */
bool exists(const path& p);

} // namespace zen

#endif // ZEN_FILESYSTEM_DIRECTORY_H
