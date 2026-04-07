/**
 * @file path.h
 * @brief 文件系统路径操作
 * 
 * path 提供跨平台的文件系统路径操作：
 * 
 * - 构造：path(p), path("/a/b/c"), path("a/b/c")
 * - 组合：operator/(), append()
 * - 分解：filename(), parent_path(), extension(), stem()
 * - 查询：exists(), is_directory(), is_file(), is_absolute(), is_relative()
 * - 操作：absolute(), relative()
 * 
 * 特性：
 * - 自动处理路径分隔符（/ 和 \）
 * - 规范化路径（去除 . 和 ..）
 * - 跨平台（Linux/Windows）
 * 
 * 示例：
 * @code
 * zen::path p1("/usr/local/bin");
 * zen::path p2("file.txt");
 * 
 * zen::path p3 = p1 / p2;  // /usr/local/bin/file.txt
 * 
 * std::cout << p3.filename() << std::endl;  // file.txt
 * std::cout << p3.extension() << std::endl;  // .txt
 * std::cout << p3.stem() << std::endl;      // file
 * @endcode
 */
#ifndef ZEN_FILESYSTEM_PATH_H
#define ZEN_FILESYSTEM_PATH_H

#include <string>
#include <vector>

namespace zen {

// ============================================================================
// path
// ============================================================================

/**
 * @brief 文件系统路径
 */
class path {
public:
    /**
     * @brief 默认构造
     */
    path() = default;
    
    /**
     * @brief 从字符串构造
     */
    path(const std::string& p) : path_(p) {
        normalize();
    }
    
    path(std::string&& p) : path_(std::move(p)) {
        normalize();
    }
    
    path(const char* p) : path_(p) {
        normalize();
    }
    
    /**
     * @brief 拷贝/移动
     */
    path(const path&) = default;
    path(path&&) = default;
    path& operator=(const path&) = default;
    path& operator=(path&&) = default;
    
    /**
     * @brief 赋值字符串
     */
    path& operator=(const std::string& p) {
        path_ = p;
        normalize();
        return *this;
    }
    
    path& operator=(std::string&& p) {
        path_ = std::move(p);
        normalize();
        return *this;
    }
    
    path& operator=(const char* p) {
        path_ = p;
        normalize();
        return *this;
    }
    
    /**
     * @brief 隐式转换为字符串
     */
    operator std::string() const {
        return path_;
    }
    
    /**
     * @brief 获取字符串
     */
    const std::string& string() const {
        return path_;
    }
    
    const char* c_str() const {
        return path_.c_str();
    }
    
    /**
     * @brief 路径拼接（operator/）
     */
    path operator/(const path& other) const {
        path result = *this;
        result.append(other.path_);
        return result;
    }
    
    path& operator/=(const path& other) {
        append(other.path_);
        return *this;
    }
    
    /**
     * @brief 追加路径
     */
    path& append(const std::string& p) {
        if (!path_.empty() && path_.back() != separator()) {
            path_ += separator();
        }
        path_ += p;
        normalize();
        return *this;
    }
    
    /**
     * @brief 获取文件名（最后部分）
     */
    path filename() const {
        size_t pos = path_.find_last_of(separator());
        if (pos == std::string::npos) {
            return path_;
        }
        return path(path_.substr(pos + 1));
    }
    
    /**
     * @brief 获取父路径
     */
    path parent_path() const {
        size_t pos = path_.find_last_of(separator());
        if (pos == std::string::npos) {
            return path(".");
        }
        if (pos == 0) {
            return path(separator());
        }
        return path(path_.substr(0, pos));
    }
    
    /**
     * @brief 获取扩展名（包含点）
     */
    path extension() const {
        std::string name = filename().path_;
        size_t pos = name.find_last_of('.');
        if (pos == std::string::npos || pos == 0) {
            return path();
        }
        return path(name.substr(pos));
    }
    
    /**
     * @brief 获取主干（文件名不含扩展名）
     */
    path stem() const {
        std::string name = filename().path_;
        size_t pos = name.find_last_of('.');
        if (pos == std::string::npos || pos == 0) {
            return path(name);
        }
        return path(name.substr(0, pos));
    }
    
    /**
     * @brief 转换为绝对路径
     */
    path absolute() const;
    
    /**
     * @brief 转换为相对路径（相对于 base）
     */
    path relative(const path& base) const;
    
    /**
     * @brief 检查路径是否存在
     */
    bool exists() const;
    
    /**
     * @brief 检查是否为目录
     */
    bool is_directory() const;
    
    /**
     * @brief 检查是否为普通文件
     */
    bool is_file() const;
    
    /**
     * @brief 检查是否为绝对路径
     */
    bool is_absolute() const {
        if (path_.empty()) return false;
        return path_[0] == separator();
    }
    
    /**
     * @brief 检查是否为相对路径
     */
    bool is_relative() const {
        return !is_absolute();
    }
    
    /**
     * @brief 判断路径是否为空
     */
    bool empty() const {
        return path_.empty();
    }
    
    /**
     * @brief 清空路径
     */
    void clear() {
        path_.clear();
    }
    
    /**
     * @brief 路径分隔符
     */
    static constexpr char separator() {
#ifdef _WIN32
        return '\\';
#else
        return '/';
#endif
    }
    
    /**
     * @brief 比较路径
     */
    bool operator==(const path& other) const {
        return path_ == other.path_;
    }
    
    bool operator!=(const path& other) const {
        return path_ != other.path_;
    }
    
    bool operator<(const path& other) const {
        return path_ < other.path_;
    }

private:
    /**
     * @brief 规范化路径（处理 . 和 ..）
     */
    void normalize();

    std::string path_;
};

} // namespace zen

#endif // ZEN_FILESYSTEM_PATH_H
