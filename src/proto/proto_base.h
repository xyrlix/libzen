#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>

namespace zen {
namespace proto {

// Protobuf 兼容的轻量级序列化

// 字段类型
enum class field_type {
    double_ = 1,
    float_ = 2,
    int64 = 3,
    uint64 = 4,
    int32 = 5,
    fixed64 = 6,
    fixed32 = 7,
    bool_ = 8,
    string = 9,
    group = 10,   // 废弃
    message = 11,
    bytes = 12,
    uint32 = 13,
    enum_ = 14,
    sfixed32 = 15,
    sfixed64 = 16,
    sint32 = 17,
    sint64 = 18
};

// 字段编号
using field_number = uint32_t;

// Tag 编码（wire_type + field_number << 3）
enum class wire_type : uint32_t {
    varint = 0,
    fixed64 = 1,
    length_delimited = 2,
    start_group = 3,    // 废弃
    end_group = 4,      // 废弃
    fixed32 = 5
};

// 编码 Tag
inline uint32_t encode_tag(field_number number, wire_type type) {
    return (static_cast<uint32_t>(number) << 3) | static_cast<uint32_t>(type);
}

// 解码 Tag
inline std::pair<field_number, wire_type> decode_tag(uint32_t tag) {
    return {tag >> 3, static_cast<wire_type>(tag & 0x07)};
}

// Protobuf 异常
class proto_exception : public std::runtime_error {
public:
    explicit proto_exception(const std::string& msg) : std::runtime_error(msg) {}
};

} // namespace proto
} // namespace zen
