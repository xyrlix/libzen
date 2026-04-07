#pragma once

#include "serialize_base.h"
#include <vector>

namespace zen {
namespace serialize {

// 二进制序列化器
class binary_serializer : public serializer {
public:
    binary_serializer(byte_order order = byte_order::network);
    ~binary_serializer() override = default;
    
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
    
    // 二进制数据
    void write_bytes(const void* data, size_t len) override;
    void write_bytes(const std::vector<uint8_t>& data) override;
    
    // 获取结果
    std::vector<uint8_t> get_data() override { return data_; }
    const uint8_t* get_data_ptr() override { return data_.data(); }
    size_t get_size() override { return data_.size(); }
    
    // 重置
    void reset() override { data_.clear(); }
    
private:
    template<typename T>
    void write_value(T value);
    
    byte_order order_;
    std::vector<uint8_t> data_;
};

// 二进制反序列化器
class binary_deserializer : public deserializer {
public:
    binary_deserializer(const void* data, size_t len, byte_order order = byte_order::network);
    binary_deserializer(const std::vector<uint8_t>& data, byte_order order = byte_order::network);
    ~binary_deserializer() override = default;
    
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
    
    // 二进制数据
    void read_bytes(void* data, size_t len) override;
    std::vector<uint8_t> read_bytes(size_t len) override;
    
    // 位置控制
    size_t get_position() override { return position_; }
    void set_position(size_t pos) override;
    void skip(size_t bytes) override;
    
    // 状态检查
    bool eof() override;
    size_t remaining() override;
    
private:
    template<typename T>
    T read_value();
    
    void check_available(size_t len) const;
    
    byte_order order_;
    const uint8_t* data_;
    size_t size_;
    size_t position_;
};

// 紧凑二进制序列化（节省空间）
class compact_binary_serializer : public binary_serializer {
public:
    compact_binary_serializer();
    
    // 可变长度整数
    void write_varint(uint64_t value);
    void write_varint(int64_t value);
    
    // ZigZag 编码（有符号）
    void write_zigzag(int64_t value);
};

class compact_binary_deserializer : public binary_deserializer {
public:
    using binary_deserializer::binary_deserializer;
    
    // 可变长度整数
    uint64_t read_varint();
    int64_t read_varint_signed();
    
    // ZigZag 解码
    int64_t read_zigzag();
};

} // namespace serialize
} // namespace zen
