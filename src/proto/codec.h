#pragma once

#include "proto_base.h"
#include <vector>

namespace zen {
namespace proto {

// Protobuf 编码器
class proto_encoder {
public:
    proto_encoder();
    ~proto_encoder() = default;
    
    // 写入 Tag
    void write_tag(field_number number, wire_type type);
    void write_tag(field_number number, field_type type);
    
    // 写入字段
    void write_bool(field_number number, bool value);
    void write_int32(field_number number, int32_t value);
    void write_int64(field_number number, int64_t value);
    void write_uint32(field_number number, uint32_t value);
    void write_uint64(field_number number, uint64_t value);
    void write_sint32(field_number number, int32_t value);
    void write_sint64(field_number number, int64_t value);
    void write_fixed32(field_number number, uint32_t value);
    void write_fixed64(field_number number, uint64_t value);
    void write_sfixed32(field_number number, int32_t value);
    void write_sfixed64(field_number number, int64_t value);
    void write_float(field_number number, float value);
    void write_double(field_number number, double value);
    void write_string(field_number number, const std::string& value);
    void write_bytes(field_number number, const std::vector<uint8_t>& value);
    
    // 写入枚举
    void write_enum(field_number number, int32_t value);
    
    // 写入消息
    void write_message(field_number number, const std::function<void(proto_encoder&)>& writer);
    void write_message(field_number number, const std::vector<uint8_t>& message);
    
    // 写入打包数组（repeated）
    void write_packed_int32(field_number number, const std::vector<int32_t>& values);
    void write_packed_int64(field_number number, const std::vector<int64_t>& values);
    void write_packed_uint32(field_number number, const std::vector<uint32_t>& values);
    void write_packed_uint64(field_number number, const std::vector<uint64_t>& values);
    void write_packed_bool(field_number number, const std::vector<bool>& values);
    void write_packed_enum(field_number number, const std::vector<int32_t>& values);
    
    // 获取结果
    const std::vector<uint8_t>& get_data() const { return data_; }
    std::vector<uint8_t> release_data() { return std::move(data_); }
    size_t get_size() const { return data_.size(); }
    
    // 重置
    void clear() { data_.clear(); }
    
private:
    // Varint 编码
    void write_varint(uint64_t value);
    
    // ZigZag 编码（有符号）
    int32_t zigzag_encode(int32_t value);
    int64_t zigzag_encode(int64_t value);
    
    std::vector<uint8_t> data_;
};

// Protobuf 解码器
class proto_decoder {
public:
    proto_decoder(const void* data, size_t len);
    proto_decoder(const std::vector<uint8_t>& data);
    ~proto_decoder() = default;
    
    // 读取 Tag
    bool read_tag(field_number& number, wire_type& type);
    bool skip_field(wire_type type);
    
    // 读取字段
    bool read_bool(bool& value);
    bool read_int32(int32_t& value);
    bool read_int64(int64_t& value);
    bool read_uint32(uint32_t& value);
    bool read_uint64(uint64_t& value);
    bool read_sint32(int32_t& value);
    bool read_sint64(int64_t& value);
    bool read_fixed32(uint32_t& value);
    bool read_fixed64(uint64_t& value);
    bool read_sfixed32(int32_t& value);
    bool read_sfixed64(int64_t& value);
    bool read_float(float& value);
    bool read_double(double& value);
    bool read_string(std::string& value);
    bool read_bytes(std::vector<uint8_t>& value);
    bool read_enum(int32_t& value);
    
    // 读取消息
    bool read_message(const std::function<void(proto_decoder&)>& reader);
    std::vector<uint8_t> read_message_bytes();
    
    // 读取打包数组
    bool read_packed_int32(std::vector<int32_t>& values);
    bool read_packed_int64(std::vector<int64_t>& values);
    bool read_packed_uint32(std::vector<uint32_t>& values);
    bool read_packed_uint64(std::vector<uint64_t>& values);
    bool read_packed_bool(std::vector<bool>& values);
    bool read_packed_enum(std::vector<int32_t>& values);
    
    // 位置控制
    size_t get_position() const { return position_; }
    void set_position(size_t pos);
    void skip_bytes(size_t bytes);
    
    // 状态检查
    bool eof() const { return position_ >= size_; }
    size_t remaining() const { return size_ - position_; }
    
    // 获取原始数据
    const uint8_t* get_data() const { return data_; }
    size_t get_size() const { return size_; }
    
private:
    // Varint 解码
    bool read_varint(uint64_t& value);
    
    // ZigZag 解码（有符号）
    int32_t zigzag_decode(uint32_t value);
    int64_t zigzag_decode(uint64_t value);
    
    const uint8_t* data_;
    size_t size_;
    size_t position_;
};

} // namespace proto
} // namespace zen
