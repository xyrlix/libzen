#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <stdexcept>

namespace zen {
namespace serialize {

// 序列化异常
class serialize_exception : public std::runtime_error {
public:
    explicit serialize_exception(const std::string& msg) : std::runtime_error(msg) {}
};

// 字节顺序
enum class byte_order {
    little_endian,
    big_endian,
    network  // 网络字节序（大端）
};

// 序列化接口
class serializer {
public:
    virtual ~serializer() = default;
    
    // 基本类型
    virtual void write_bool(bool value) = 0;
    virtual void write_int8(int8_t value) = 0;
    virtual void write_int16(int16_t value) = 0;
    virtual void write_int32(int32_t value) = 0;
    virtual void write_int64(int64_t value) = 0;
    virtual void write_uint8(uint8_t value) = 0;
    virtual void write_uint16(uint16_t value) = 0;
    virtual void write_uint32(uint32_t value) = 0;
    virtual void write_uint64(uint64_t value) = 0;
    virtual void write_float(float value) = 0;
    virtual void write_double(double value) = 0;
    virtual void write_string(const std::string& value) = 0;
    
    // 二进制数据
    virtual void write_bytes(const void* data, size_t len) = 0;
    virtual void write_bytes(const std::vector<uint8_t>& data) = 0;
    
    // 获取结果
    virtual std::vector<uint8_t> get_data() = 0;
    virtual const uint8_t* get_data_ptr() = 0;
    virtual size_t get_size() = 0;
    
    // 重置
    virtual void reset() = 0;
};

// 反序列化接口
class deserializer {
public:
    virtual ~deserializer() = default;
    
    // 基本类型
    virtual bool read_bool() = 0;
    virtual int8_t read_int8() = 0;
    virtual int16_t read_int16() = 0;
    virtual int32_t read_int32() = 0;
    virtual int64_t read_int64() = 0;
    virtual uint8_t read_uint8() = 0;
    virtual uint16_t read_uint16() = 0;
    virtual uint32_t read_uint32() = 0;
    virtual uint64_t read_uint64() = 0;
    virtual float read_float() = 0;
    virtual double read_double() = 0;
    virtual std::string read_string() = 0;
    
    // 二进制数据
    virtual void read_bytes(void* data, size_t len) = 0;
    virtual std::vector<uint8_t> read_bytes(size_t len) = 0;
    
    // 位置控制
    virtual size_t get_position() = 0;
    virtual void set_position(size_t pos) = 0;
    virtual void skip(size_t bytes) = 0;
    
    // 状态检查
    virtual bool eof() = 0;
    virtual size_t remaining() = 0;
};

// 可序列化接口
class serializable {
public:
    virtual ~serializable() = default;
    virtual void serialize(serializer& s) const = 0;
    virtual void deserialize(deserializer& d) = 0;
};

// 字节序转换
class byte_order_converter {
public:
    static uint16_t swap(uint16_t value);
    static uint32_t swap(uint32_t value);
    static uint64_t swap(uint64_t value);
    static int16_t swap(int16_t value);
    static int32_t swap(int32_t value);
    static int64_t swap(int64_t value);
    
    // 主机到网络
    static uint16_t host_to_network(uint16_t value);
    static uint32_t host_to_network(uint32_t value);
    static uint64_t host_to_network(uint64_t value);
    
    // 网络到主机
    static uint16_t network_to_host(uint16_t value);
    static uint32_t network_to_host(uint32_t value);
    static uint64_t network_to_host(uint64_t value);
};

} // namespace serialize
} // namespace zen
