#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace zen {
namespace crypto {

// 哈希算法类型
enum class hash_type {
    md5,
    sha1,
    sha256,
    sha512,
    crc32
};

// MD5 哈希
class md5 {
public:
    md5();
    
    // 更新哈希
    void update(const void* data, size_t len);
    void update(const std::string& str);
    
    // 获取结果
    std::vector<uint8_t> digest();
    std::string digest_hex();
    std::string digest_base64();
    
    // 静态方法
    static std::vector<uint8_t> hash(const void* data, size_t len);
    static std::vector<uint8_t> hash(const std::string& str);
    static std::string hash_hex(const std::string& str);
    
private:
    void transform(const uint8_t block[64]);
    void finalize();
    
    uint32_t state_[4];
    uint64_t count_[2];
    uint8_t buffer_[64];
    bool finalized_;
};

// SHA-1 哈希
class sha1 {
public:
    sha1();
    
    void update(const void* data, size_t len);
    void update(const std::string& str);
    
    std::vector<uint8_t> digest();
    std::string digest_hex();
    std::string digest_base64();
    
    static std::vector<uint8_t> hash(const void* data, size_t len);
    static std::vector<uint8_t> hash(const std::string& str);
    static std::string hash_hex(const std::string& str);
    
private:
    void transform(const uint32_t block[80]);
    void finalize();
    
    uint32_t state_[5];
    uint64_t count_;
    uint8_t buffer_[64];
    bool finalized_;
};

// SHA-256 哈希
class sha256 {
public:
    sha256();
    
    void update(const void* data, size_t len);
    void update(const std::string& str);
    
    std::vector<uint8_t> digest();
    std::string digest_hex();
    std::string digest_base64();
    
    static std::vector<uint8_t> hash(const void* data, size_t len);
    static std::vector<uint8_t> hash(const std::string& str);
    static std::string hash_hex(const std::string& str);
    
private:
    void transform(const uint32_t block[64]);
    void finalize();
    
    uint32_t state_[8];
    uint64_t count_;
    uint8_t buffer_[64];
    bool finalized_;
};

// SHA-512 哈希
class sha512 {
public:
    sha512();
    
    void update(const void* data, size_t len);
    void update(const std::string& str);
    
    std::vector<uint8_t> digest();
    std::string digest_hex();
    std::string digest_base64();
    
    static std::vector<uint8_t> hash(const void* data, size_t len);
    static std::vector<uint8_t> hash(const std::string& str);
    static std::string hash_hex(const std::string& str);
    
private:
    void transform(const uint64_t block[128]);
    void finalize();
    
    uint64_t state_[8];
    uint64_t count_[2];
    uint8_t buffer_[128];
    bool finalized_;
};

// CRC32 哈希
class crc32 {
public:
    crc32(uint32_t init_crc = 0xFFFFFFFF);
    
    void update(const void* data, size_t len);
    void update(const std::string& str);
    
    uint32_t digest();
    std::string digest_hex();
    
    static uint32_t hash(const void* data, size_t len);
    static uint32_t hash(const std::string& str);
    static std::string hash_hex(const std::string& str);
    
private:
    uint32_t crc_;
};

// HMAC 哈希消息认证码
template<typename HashType>
class hmac {
public:
    hmac(const std::vector<uint8_t>& key);
    hmac(const std::string& key);
    
    void update(const void* data, size_t len);
    void update(const std::string& str);
    
    std::vector<uint8_t> digest();
    std::string digest_hex();
    
    static std::vector<uint8_t> hash(const std::vector<uint8_t>& key, const void* data, size_t len);
    static std::vector<uint8_t> hash(const std::string& key, const std::string& str);
    static std::string hash_hex(const std::string& key, const std::string& str);
    
private:
    std::vector<uint8_t> key_;
    HashType hash_;
};

} // namespace crypto
} // namespace zen
