#pragma once

#include "config_base.h"

#include <string>

namespace zen {
namespace config {

// JSON 配置文件解析
class json_config : public config_base {
public:
    json_config();
    ~json_config() override = default;
    
    // 加载/保存
    bool load(const std::string& path) override;
    bool save(const std::string& path) override;
    bool load_string(const std::string& content) override;
    std::string to_string() const override;
    
    // 访问配置
    config_value& root() override { return root_; }
    const config_value& root() const override { return root_; }
    
    // 美化输出
    std::string to_string(int indent) const;
    std::string to_string_pretty() const { return to_string(2); }
    
private:
    // JSON 解析
    bool parse(const std::string& json);
    config_value parse_value(const std::string& json, size_t& pos);
    config_value parse_object(const std::string& json, size_t& pos);
    config_value parse_array(const std::string& json, size_t& pos);
    config_value parse_string(const std::string& json, size_t& pos);
    config_value parse_number(const std::string& json, size_t& pos);
    config_value parse_literal(const std::string& json, size_t& pos);
    
    // JSON 格式化
    std::string format_value(const config_value& val, int indent = 0) const;
    std::string format_object(const config_value& val, int indent) const;
    std::string format_array(const config_value& val, int indent) const;
    std::string format_string(const std::string& str) const;
    
    // 工具函数
    void skip_whitespace(const std::string& json, size_t& pos) const;
    bool is_whitespace(char c) const;
    std::string escape_json_string(const std::string& str) const;
    std::string unescape_json_string(const std::string& str) const;
};

} // namespace config
} // namespace zen
