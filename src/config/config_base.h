#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <cstdint>

namespace zen {
namespace config {

// 配置异常
class config_exception : public std::runtime_error {
public:
    explicit config_exception(const std::string& msg) : std::runtime_error(msg) {}
};

// 配置值类型
enum class config_value_type {
    null,
    boolean,
    integer,
    float_,
    string,
    array,
    object
};

// 配置值封装类
class config_value {
public:
    config_value();
    explicit config_value(bool v);
    explicit config_value(int v);
    explicit config_value(int64_t v);
    explicit config_value(double v);
    explicit config_value(const std::string& v);
    explicit config_value(const char* v);
    
    // 访问方法
    bool is_null() const { return type_ == config_value_type::null; }
    bool is_bool() const { return type_ == config_value_type::boolean; }
    bool is_int() const { return type_ == config_value_type::integer; }
    bool is_float() const { return type_ == config_value_type::float_; }
    bool is_string() const { return type_ == config_value_type::string; }
    bool is_array() const { return type_ == config_value_type::array; }
    bool is_object() const { return type_ == config_value_type::object; }
    
    // 类型获取
    config_value_type get_type() const { return type_; }
    
    // 值获取
    bool as_bool() const;
    int as_int() const;
    int64_t as_int64() const;
    double as_float() const;
    std::string as_string() const;
    
    // 数组访问
    size_t size() const;
    config_value& operator[](size_t index);
    const config_value& operator[](size_t index) const;
    void push_back(const config_value& value);
    void clear();
    
    // 对象访问
    config_value& operator[](const std::string& key);
    const config_value& operator[](const std::string& key) const;
    bool has(const std::string& key) const;
    void remove(const std::string& key);
    std::vector<std::string> keys() const;
    
    // 字符串化
    std::string to_string() const;
    
private:
    config_value_type type_;
    bool bool_val_;
    int64_t int_val_;
    double float_val_;
    std::string string_val_;
    std::vector<config_value> array_val_;
    std::unordered_map<std::string, config_value> object_val_;
};

// 配置基类
class config_base {
public:
    config_base() = default;
    virtual ~config_base() = default;
    
    // 加载/保存
    virtual bool load(const std::string& path) = 0;
    virtual bool save(const std::string& path) = 0;
    virtual bool load_string(const std::string& content) = 0;
    virtual std::string to_string() const = 0;
    
    // 访问配置
    virtual config_value& root() = 0;
    virtual const config_value& root() const = 0;
    
    // 便捷访问
    bool has(const std::string& key) const;
    
    bool get_bool(const std::string& key, bool default_val = false) const;
    int get_int(const std::string& key, int default_val = 0) const;
    int64_t get_int64(const std::string& key, int64_t default_val = 0) const;
    double get_float(const std::string& key, double default_val = 0.0) const;
    std::string get_string(const std::string& key, const std::string& default_val = "") const;
    
    void set_bool(const std::string& key, bool value);
    void set_int(const std::string& key, int value);
    void set_int64(const std::string& key, int64_t value);
    void set_float(const std::string& key, double value);
    void set_string(const std::string& key, const std::string& value);
    
    // 数组访问
    size_t array_size(const std::string& key) const;
    bool get_bool_at(const std::string& key, size_t index, bool default_val = false) const;
    int get_int_at(const std::string& key, size_t index, int default_val = 0) const;
    std::string get_string_at(const std::string& key, size_t index, const std::string& default_val = "") const;
    
protected:
    // 按路径访问（支持 "section.key" 格式）
    config_value* get_value(const std::string& key);
    const config_value* get_value(const std::string& key) const;
    config_value& get_or_create(const std::string& key);
    
    config_value root_;
};

} // namespace config
} // namespace zen
