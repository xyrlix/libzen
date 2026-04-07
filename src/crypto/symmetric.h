#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace zen {
namespace crypto {

// 对称加密算法
enum class cipher_algorithm {
    aes,
    des,
    triple_des,
    rc4
};

// 加密模式
enum class cipher_mode {
    ecb,        // 电子密码本模式
    cbc,        // 密码分组链接模式
    cfb,        // 密文反馈模式
    ofb,        // 输出反馈模式
    ctr,        // 计数器模式
    gcm         // 认证加密模式
};

// 填充方式
enum class padding_mode {
    none,       // 无填充（数据必须是块大小的整数倍）
    pkcs7,      // PKCS#7 填充
    iso_10126,  // ISO 10126 填充
    ansi_x923,  // ANSI X9.23 填充
    zero        // 零填充
};

// 对称加密器
class symmetric_cipher {
public:
    symmetric_cipher(cipher_algorithm algo, cipher_mode mode, padding_mode padding);
    ~symmetric_cipher();
    
    // 设置密钥和初始化向量
    void set_key(const std::vector<uint8_t>& key);
    void set_key(const std::string& key);
    void set_iv(const std::vector<uint8_t>& iv);
    void set_iv(const std::string& iv);
    
    // 加密
    std::vector<uint8_t> encrypt(const void* data, size_t len);
    std::vector<uint8_t> encrypt(const std::string& str);
    
    // 解密
    std::vector<uint8_t> decrypt(const void* data, size_t len);
    std::vector<uint8_t> decrypt(const std::vector<uint8_t>& data);
    
    // 重置
    void reset();
    
    // 获取信息
    cipher_algorithm get_algorithm() const { return algorithm_; }
    cipher_mode get_mode() const { return mode_; }
    padding_mode get_padding() const { return padding_; }
    size_t get_block_size() const;
    size_t get_key_size() const;
    
    // 静态方法
    static std::vector<uint8_t> encrypt_aes_ecb(const std::vector<uint8_t>& data,
                                                  const std::vector<uint8_t>& key);
    static std::vector<uint8_t> decrypt_aes_ecb(const std::vector<uint8_t>& data,
                                                  const std::vector<uint8_t>& key);
    
    static std::vector<uint8_t> encrypt_aes_cbc(const std::vector<uint8_t>& data,
                                                  const std::vector<uint8_t>& key,
                                                  const std::vector<uint8_t>& iv);
    static std::vector<uint8_t> decrypt_aes_cbc(const std::vector<uint8_t>& data,
                                                  const std::vector<uint8_t>& key,
                                                  const std::vector<uint8_t>& iv);

private:
    cipher_algorithm algorithm_;
    cipher_mode mode_;
    padding_mode padding_;
    std::vector<uint8_t> key_;
    std::vector<uint8_t> iv_;
    void* context_;  // 加密库上下文
    
    // 填充/去填充
    void apply_padding(std::vector<uint8_t>& data, size_t block_size);
    void remove_padding(std::vector<uint8_t>& data, size_t block_size);
};

// AES 加密器（便捷封装）
class aes {
public:
    // 128/192/256 位密钥
    static const size_t key_size_128 = 16;
    static const size_t key_size_192 = 24;
    static const size_t key_size_256 = 32;
    static const size_t block_size = 16;
    
    // ECB 模式
    static std::vector<uint8_t> encrypt_ecb(const std::vector<uint8_t>& data,
                                             const std::vector<uint8_t>& key);
    static std::vector<uint8_t> decrypt_ecb(const std::vector<uint8_t>& data,
                                             const std::vector<uint8_t>& key);
    
    // CBC 模式
    static std::vector<uint8_t> encrypt_cbc(const std::vector<uint8_t>& data,
                                             const std::vector<uint8_t>& key,
                                             const std::vector<uint8_t>& iv);
    static std::vector<uint8_t> decrypt_cbc(const std::vector<uint8_t>& data,
                                             const std::vector<uint8_t>& key,
                                             const std::vector<uint8_t>& iv);
    
    // CTR 模式
    static std::vector<uint8_t> encrypt_ctr(const std::vector<uint8_t>& data,
                                             const std::vector<uint8_t>& key,
                                             const std::vector<uint8_t>& iv);
    static std::vector<uint8_t> decrypt_ctr(const std::vector<uint8_t>& data,
                                             const std::vector<uint8_t>& key,
                                             const std::vector<uint8_t>& iv);
    
    // GCM 模式（认证加密）
    static std::vector<uint8_t> encrypt_gcm(const std::vector<uint8_t>& data,
                                             const std::vector<uint8_t>& key,
                                             const std::vector<uint8_t>& iv,
                                             const std::vector<uint8_t>& aad = {});
    static bool decrypt_gcm(const std::vector<uint8_t>& data,
                            std::vector<uint8_t>& plaintext,
                            const std::vector<uint8_t>& key,
                            const std::vector<uint8_t>& iv,
                            const std::vector<uint8_t>& aad = {},
                            const std::vector<uint8_t>& tag = {});
    
    // 生成密钥
    static std::vector<uint8_t> generate_key(size_t bits = 256);
    static std::vector<uint8_t> generate_iv();
};

// RC4 流加密器
class rc4 {
public:
    rc4(const std::vector<uint8_t>& key);
    rc4(const std::string& key);
    
    void encrypt(const void* data, size_t len, uint8_t* out);
    void decrypt(const void* data, size_t len, uint8_t* out);
    
    std::vector<uint8_t> encrypt(const std::vector<uint8_t>& data);
    std::vector<uint8_t> decrypt(const std::vector<uint8_t>& data);
    
private:
    void init_key(const std::vector<uint8_t>& key);
    
    uint8_t s_[256];
    uint8_t i_;
    uint8_t j_;
};

} // namespace crypto
} // namespace zen
