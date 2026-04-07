#include <gtest/gtest.h>
#include <zen/crypto.h>

using namespace zen;
using namespace zen::crypto;

TEST(HashTest, MD5) {
    std::string data = "hello world";
    std::string hash = md5::hash(data);
    
    EXPECT_EQ(hash.size(), 32);
    
    std::string expected = "5eb63bbbe01eeed093cb22bb8f5acdc3";
    EXPECT_EQ(hash, expected);
}

TEST(HashTest, SHA256) {
    std::string data = "hello world";
    std::string hash = sha256::hash(data);
    
    EXPECT_EQ(hash.size(), 64);
    
    std::string expected = "b94d27b9934d3e08a52e52d7da7dabfac484efe37a5380ee9088f7ace2efcde9";
    EXPECT_EQ(hash, expected);
}

TEST(HashTest, SHA512) {
    std::string data = "hello world";
    std::string hash = sha512::hash(data);
    
    EXPECT_EQ(hash.size(), 128);
}

TEST(HashTest, CRC32) {
    std::string data = "hello world";
    uint32_t crc = crc32::hash(data);
    
    EXPECT_NE(crc, 0);
}

TEST(HashTest, HMAC) {
    std::string key = "secret";
    std::string data = "message";
    std::string hmac = hmac<sha256>::hash(key, data);
    
    EXPECT_EQ(hmac.size(), 64);
}

TEST(SymmetricTest, AES128ECB) {
    std::string key = "0123456789ABCDEF";
    std::string plaintext = "Hello World!";
    
    aes aes_cipher(aes::key_size_128);
    
    std::string encrypted = aes_cipher.encrypt(plaintext, key, cipher_mode::ecb);
    EXPECT_NE(encrypted, plaintext);
    
    std::string decrypted = aes_cipher.decrypt(encrypted, key, cipher_mode::ecb);
    EXPECT_EQ(decrypted, plaintext);
}

TEST(SymmetricTest, AES256CBC) {
    std::string key = "0123456789ABCDEF0123456789ABCDEF";
    std::string iv = "1234567890ABCDEF";
    std::string plaintext = "Hello World!";
    
    aes aes_cipher(aes::key_size_256);
    
    std::string encrypted = aes_cipher.encrypt(plaintext, key, cipher_mode::cbc, iv);
    EXPECT_NE(encrypted, plaintext);
    
    std::string decrypted = aes_cipher.decrypt(encrypted, key, cipher_mode::cbc, iv);
    EXPECT_EQ(decrypted, plaintext);
}

TEST(Base64Test, EncodeDecode) {
    std::string data = "Hello World!";
    
    std::string encoded = base64::encode(data);
    EXPECT_NE(encoded, data);
    
    std::string decoded = base64::decode(encoded);
    EXPECT_EQ(decoded, data);
}

TEST(Base64Test, HexEncodeDecode) {
    std::string data = "Hello";
    
    std::string encoded = hex::encode(data);
    EXPECT_NE(encoded, data);
    
    std::string decoded = hex::decode(encoded);
    EXPECT_EQ(decoded, data);
}
