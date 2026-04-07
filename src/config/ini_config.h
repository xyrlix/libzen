#pragma once

#include "config_base.h"

#include <string>
#include <unordered_map>

namespace zen {
namespace config {

// INI 配置文件解析
class ini_config : public config_base {
public:
    ini_config();
    ~ini_config() override = default;
    
    // 加载/保存
    bool load(const std::string& path) override;
    bool save(const std::string& path) override;
    bool load_string(const std::string& content) override;
    std::string to_string() const override;
    
    // 访问配置
    config_value& root() override { return root_; }
    const config_value& root() const override { return root_; }
    
    // INI 特有方法
    bool has_section(const std::string& section) const;
    bool has_key(const std::string& section, const std::string& key) const;
    
    std::string get(const std::string& section, const std::string& key,
                    const std::string& default_val = "") const;
    int get_int(const std::string& section, const std::string& key,
                int default_val = 0) const;
    bool get_bool(const std::string& section, const std::string& key,
                  bool default_val = false) const;
    double get_float(const std::string& section, const std::string& key,
                     double default_val = 0.0) const;
    
    void set(const std::string& section, const std::string& key, const std::string& value);
    void set_int(const std::string& section, const std::string& key, int value);
    void set_bool(const std::string& section, const std::string& key, bool value);
    void set_float(const std::string& section, const std::string& key, double value);
    
    void remove_key(const std::string& section, const std::string& key);
    void remove_section(const std::string& section);
    
    std::vector<std::string> sections() const;
    std::vector<std::string> keys(const std::string& section) const;
    
    // 清空
    void clear();
    
private:
    // 解析 INI 行
    bool parse_line(const std::string& line, std::string& current_section);
    
    // 格式化 INI
    std::string format_section(const std::string& section) const;
    std::string format_key_value(const std::string& key, const std::string& value) const;
    
    // 转义/反转义
    std::string escape_value(const std::string& value) const;
    std::string unescape_value(const std::string& value) const;
};

} // namespace config
} // namespace zen
