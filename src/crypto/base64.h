#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace zen {
namespace crypto {

// Base64 编码/解码
class base64 {
public:
    // 标准编码
    static std::string encode(const void* data, size_t len);
    static std::string encode(const std::vector<uint8_t>& data);
    static std::string encode(const std::string& str);
    
    // 标准解码
    static std::vector<uint8_t> decode(const std::string& encoded);
    static std::string decode_to_string(const std::string& encoded);
    
    // URL 安全编码（替换 + 为 -，/ 为 _，去除 =）
    static std::string encode_url(const void* data, size_t len);
    static std::string encode_url(const std::vector<uint8_t>& data);
    static std::string encode_url(const std::string& str);
    
    // URL 安全解码
    static std::vector<uint8_t> decode_url(const std::string& encoded);
    static std::string decode_url_to_string(const std::string& encoded);
    
    // MIME 编码（76 字符换行）
    static std::string encode_mime(const void* data, size_t len);
    static std::string encode_mime(const std::vector<uint8_t>& data);
    static std::string encode_mime(const std::string& str);
    
    // MIME 解码
    static std::vector<uint8_t> decode_mime(const std::string& encoded);
    
    // 检查是否是有效的 Base64
    static bool is_valid(const std::string& str);
    static bool is_valid_url(const std::string& str);
    
private:
    static const char encoding_table_[];
    static const char decoding_table_[];
    static const size_t mod_table_[];
    
    static char encode_char(uint8_t value);
    static uint8_t decode_char(char c);
};

// Hex 编码/解码
class hex {
public:
    // 小写编码
    static std::string encode(const void* data, size_t len, bool uppercase = false);
    static std::string encode(const std::vector<uint8_t>& data, bool uppercase = false);
    
    // 解码（自动处理大小写）
    static std::vector<uint8_t> decode(const std::string& hex);
    
    // 转换为字符串表示（用于哈希显示）
    static std::string to_string(const std::vector<uint8_t>& data, bool uppercase = false);
    
private:
    static uint8_t decode_char(char c);
};

} // namespace crypto
} // namespace zen
