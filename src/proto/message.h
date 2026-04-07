#pragma once

#include "proto_base.h"
#include "codec.h"
#include "../serialize/serialize_base.h"
#include <unordered_map>
#include <memory>

namespace zen {
namespace proto {

// Protobuf 消息基类
class proto_message : public serialize::serializable {
public:
    virtual ~proto_message() = default;
    
    // 序列化到二进制
    std::vector<uint8_t> serialize() const;
    bool serialize(std::vector<uint8_t>& output) const;
    bool serialize(void* buffer, size_t buffer_size, size_t& actual_size) const;
    
    // 从二进制反序列化
    bool deserialize(const void* data, size_t len);
    bool deserialize(const std::vector<uint8_t>& data);
    
    // 获取大小
    size_t byte_size() const;
    
    // 检查字段是否存在
    bool has_field(field_number number) const;
    
    // 字符串化（用于调试）
    virtual std::string to_string() const;
    virtual std::string to_debug_string() const;
    
protected:
    // 编码/解码接口（由子类实现）
    virtual void encode(proto_encoder& encoder) const = 0;
    virtual bool decode(proto_decoder& decoder) = 0;
    
    // 字段存在性
    void set_field_present(field_number number);
    void clear_field_present(field_number number);
    
    std::unordered_set<field_number> present_fields_;
};

// 字段描述符
struct field_descriptor {
    field_number number;
    field_type type;
    std::string name;
    bool repeated;
    bool packed;
    std::shared_ptr<field_descriptor> message_type;  // 嵌套消息
    std::shared_ptr<field_descriptor> enum_type;     // 枚举
    
    int32_t default_value_int;
    uint32_t default_value_uint;
    bool default_value_bool;
    std::string default_value_string;
    
    field_descriptor()
        : number(0), type(field_type::int32), repeated(false), packed(false)
        , default_value_int(0), default_value_uint(0), default_value_bool(false) {}
};

// 反射消息（动态支持）
class reflected_message : public proto_message {
public:
    reflected_message() = default;
    ~reflected_message() override = default;
    
    // 设置值
    void set_bool(field_number number, bool value);
    void set_int32(field_number number, int32_t value);
    void set_int64(field_number number, int64_t value);
    void set_uint32(field_number number, uint32_t value);
    void set_uint64(field_number number, uint64_t value);
    void set_float(field_number number, float value);
    void set_double(field_number number, double value);
    void set_string(field_number number, const std::string& value);
    void set_bytes(field_number number, const std::vector<uint8_t>& value);
    void set_message(field_number number, const proto_message& msg);
    
    // 获取值
    bool get_bool(field_number number, bool& value) const;
    bool get_int32(field_number number, int32_t& value) const;
    bool get_int64(field_number number, int64_t& value) const;
    bool get_uint32(field_number number, uint32_t& value) const;
    bool get_uint64(field_number number, uint64_t& value) const;
    bool get_float(field_number number, float& value) const;
    bool get_double(field_number number, double& value) const;
    bool get_string(field_number number, std::string& value) const;
    bool get_bytes(field_number number, std::vector<uint8_t>& value) const;
    
    // 数组操作
    void add_bool(field_number number, bool value);
    void add_int32(field_number number, int32_t value);
    void add_string(field_number number, const std::string& value);
    
    size_t get_array_size(field_number number) const;
    bool get_bool_at(field_number number, size_t index, bool& value) const;
    bool get_int32_at(field_number number, size_t index, int32_t& value) const;
    
    // 清除字段
    void clear_field(field_number number);
    
    // 字段描述符
    void add_descriptor(const field_descriptor& desc);
    const field_descriptor* get_descriptor(field_number number) const;
    std::vector<const field_descriptor*> get_descriptors() const;
    
protected:
    void encode(proto_encoder& encoder) const override;
    bool decode(proto_decoder& decoder) override;
    
private:
    // 字段存储
    struct field_value {
        field_type type;
        bool bool_val;
        int64_t int_val;
        double float_val;
        std::string string_val;
        std::vector<uint8_t> bytes_val;
        std::shared_ptr<proto_message> message_val;
        
        bool is_array;
        std::vector<field_value> array_values;
    };
    
    std::unordered_map<field_number, field_value> fields_;
    std::unordered_map<field_number, field_descriptor> descriptors_;
};

} // namespace proto
} // namespace zen
