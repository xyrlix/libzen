#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace zen {
namespace crypto {

// 非对称加密算法
enum class asymmetric_algorithm {
    rsa,
    ecc,
    dsa
};

// RSA 密钥
class rsa_key {
public:
    struct params {
        std::vector<uint8_t> n;  // 模数
        std::vector<uint8_t> e;  // 公指数
        std::vector<uint8_t> d;  // 私指数
        std::vector<uint8_t> p;  // 素数 p
        std::vector<uint8_t> q;  // 素数 q
        std::vector<uint8_t> dp; // d mod (p-1)
        std::vector<uint8_t> dq; // d mod (q-1)
        std::vector<uint8_t> inv_q; // q^(-1) mod p
    };
    
    rsa_key();
    rsa_key(size_t key_bits = 2048);
    ~rsa_key();
    
    // 生成密钥对
    bool generate(size_t key_bits = 2048);
    
    // 从 PEM/DER 导入
    bool load_public_pem(const std::string& pem);
    bool load_private_pem(const std::string& pem);
    bool load_public_der(const std::vector<uint8_t>& der);
    bool load_private_der(const std::vector<uint8_t>& der);
    
    // 导出到 PEM/DER
    std::string export_public_pem() const;
    std::string export_private_pem() const;
    std::vector<uint8_t> export_public_der() const;
    std::vector<uint8_t> export_private_der() const;
    
    // 获取参数
    const params& get_params() const { return params_; }
    params& get_params() { return params_; }
    
    // 密钥信息
    size_t get_key_bits() const;
    bool has_private_key() const;
    bool is_valid() const;
    
private:
    params params_;
    size_t key_bits_;
    void* key_;  // 加密库密钥结构
};

// RSA 加密器
class rsa {
public:
    enum class padding {
        pkcs1_v1_5,
        oaep,
        pss
    };
    
    // 公钥加密
    static std::vector<uint8_t> encrypt_public(const std::vector<uint8_t>& data,
                                                 const rsa_key& key,
                                                 padding pad = padding::pkcs1_v1_5);
    
    // 私钥解密
    static std::vector<uint8_t> decrypt_private(const std::vector<uint8_t>& ciphertext,
                                                 const rsa_key& key,
                                                 padding pad = padding::pkcs1_v1_5);
    
    // 私钥加密（签名）
    static std::vector<uint8_t> encrypt_private(const std::vector<uint8_t>& data,
                                                  const rsa_key& key,
                                                  padding pad = padding::pkcs1_v1_5);
    
    // 公钥解密（验签）
    static std::vector<uint8_t> decrypt_public(const std::vector<uint8_t>& ciphertext,
                                                 const rsa_key& key,
                                                 padding pad = padding::pkcs1_v1_5);
    
    // 签名/验签
    static std::vector<uint8_t> sign(const std::vector<uint8_t>& data,
                                       const rsa_key& private_key);
    static bool verify(const std::vector<uint8_t>& data,
                       const std::vector<uint8_t>& signature,
                       const rsa_key& public_key);
    
    // 混合加密（随机对称密钥 + RSA）
    static std::vector<uint8_t> encrypt_hybrid(const std::vector<uint8_t>& data,
                                                 const rsa_key& public_key);
    static std::vector<uint8_t> decrypt_hybrid(const std::vector<uint8_t>& ciphertext,
                                                 const rsa_key& private_key);
};

// ECC 椭圆曲线加密
class ecc {
public:
    // 曲线类型
    enum class curve {
        secp256r1,   // NIST P-256
        secp384r1,   // NIST P-384
        secp521r1,   // NIST P-521
        secp256k1    // Bitcoin curve
    };
    
    // ECC 密钥对
    struct key_pair {
        std::vector<uint8_t> private_key;
        std::vector<uint8_t> public_key;
    };
    
    // 生成密钥对
    static key_pair generate_key(curve c = curve::secp256r1);
    
    // ECDH 密钥交换
    static std::vector<uint8_t> derive_shared_secret(const std::vector<uint8_t>& my_private_key,
                                                       const std::vector<uint8_t>& peer_public_key,
                                                       curve c = curve::secp256r1);
    
    // ECDSA 签名/验签
    static std::vector<uint8_t> sign(const std::vector<uint8_t>& data,
                                       const std::vector<uint8_t>& private_key,
                                       curve c = curve::secp256r1);
    static bool verify(const std::vector<uint8_t>& data,
                        const std::vector<uint8_t>& signature,
                        const std::vector<uint8_t>& public_key,
                        curve c = curve::secp256r1);
};

} // namespace crypto
} // namespace zen
