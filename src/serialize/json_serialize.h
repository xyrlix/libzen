#pragma once

#include "serialize_base.h"
#include "../config/config_base.h"

#include <vector>

namespace zen {
namespace serialize {

// JSON 序列化器
class json_serializer : public serializer {
public:
    json_serializer(bool pretty = false, int indent = 2);
    ~json_serializer() override = default;
    
    // 基本类型
    void write_bool(bool value) override;
    void write_int8(int8_t value) override;
    void write_int16(int16_t value) override;
    void write_int32(int32_t value) override;
    void write_int64(int64_t value) override;
    void write_uint8(uint8_t value) override;
    void write_uint16(uint16_t value) override;
    void write_uint32(uint32_t value) override;
    void write_uint64(uint64_t value) override;
    void write_float(float value) override;
    void write_double(double value) override;
    void write_string(const std::string& value) override;
    
    // 二进制数据（Base64）
    void write_bytes(const void* data, size_t len) override;
    void write_bytes(const std::vector<uint8_t>& data) override;
    
    // JSON 特有
    void write_null();
    
    // 数组操作
    void start_array();
    void end_array();
    
    // 对象操作
    void start_object();
    void end_object();
    void write_key(const std::string& key);
    
    // 获取结果
    std::vector<uint8_t> get_data() override;
    const uint8_t* get_data_ptr() override;
    size_t get_size() override;
    
    // 获取 JSON 字符串
    std::string get_json_string() const;
    
    // 重置
    void reset() override;
    
private:
    void append_comma();
    void append_indent();
    
    bool pretty_;
    int indent_;
    int depth_;
    bool need_comma_;
    
    std::string json_;
};

// JSON 反序列化器
class json_deserializer : public deserializer {
public:
    json_deserializer(const std::string& json);
    json_deserializer(const char* json, size_t len);
    ~json_deserializer() override = default;
    
    // 基本类型
    bool read_bool() override;
    int8_t read_int8() override;
    int16_t read_int16() override;
    int32_t read_int32() override;
    int64_t read_int64() override;
    uint8_t read_uint8() override;
    uint16_t read_uint16() override;
    uint32_t read_uint32() override;
    uint64_t read_uint64() override;
    float read_float() override;
    double read_double() override;
    std::string read_string() override;
    
    // 二进制数据（Base64）
    void read_bytes(void* data, size_t len) override;
    std::vector<uint8_t> read_bytes(size_t len) override;
    std::vector<uint8_t> read_bytes();
    
    // JSON 特有
    bool is_null();
    
    // 数组操作
    bool start_array();
    void end_array();
    size_t get_array_size();
    bool has_next();
    
    // 对象操作
    bool start_object();
    void end_object();
    bool has_key(const std::string& key);
    std::string read_key();
    std::vector<std::string> get_keys();
    
    // 位置控制
    size_t get_position() override;
    void set_position(size_t pos) override;
    void skip(size_t bytes) override;
    
    // 状态检查
    bool eof() override;
    size_t remaining() override;
    
private:
    void skip_whitespace();
    char peek();
    char get();
    
    bool match(const char* str);
    
    std::string read_string_internal();
    double read_number_internal();
    bool read_bool_internal();
    void read_null_internal();
    
    config::config_value root_;
    config::config_value* current_;
    std::vector<config::config_value*> stack_;
    
    const char* json_;
    size_t length_;
    size_t position_;
};

} // namespace serialize
} // namespace zen
