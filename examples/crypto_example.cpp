#include <zen/crypto.h>
#include <iostream>
#include <iomanip>

using namespace zen;
using namespace zen::crypto;

void hash_example() {
    std::cout << "=== Hash Examples ===" << std::endl;
    
    std::string data = "Hello, World!";
    
    // MD5
    std::string md5_hash = md5::hash(data);
    std::cout << "MD5: " << md5_hash << std::endl;
    
    // SHA1
    std::string sha1_hash = sha1::hash(data);
    std::cout << "SHA1: " << sha1_hash << std::endl;
    
    // SHA256
    std::string sha256_hash = sha256::hash(data);
    std::cout << "SHA256: " << sha256_hash << std::endl;
    
    // SHA512
    std::string sha512_hash = sha512::hash(data);
    std::cout << "SHA512: " << sha512_hash << std::endl;
    
    // CRC32
    uint32_t crc = crc32::hash(data);
    std::cout << "CRC32: " << std::hex << crc << std::dec << std::endl;
    
    // HMAC
    std::string key = "secret_key";
    std::string hmac = hmac<sha256>::hash(key, data);
    std::cout << "HMAC-SHA256: " << hmac << std::endl;
    
    std::cout << std::endl;
}

void symmetric_encryption_example() {
    std::cout << "=== Symmetric Encryption Examples ===" << std::endl;
    
    std::string plaintext = "This is a secret message!";
    
    // AES-128-ECB
    {
        std::string key = "0123456789ABCDEF";  // 16 bytes for AES-128
        aes cipher(aes::key_size_128);
        
        std::string encrypted = cipher.encrypt(plaintext, key, cipher_mode::ecb);
        std::string decrypted = cipher.decrypt(encrypted, key, cipher_mode::ecb);
        
        std::cout << "AES-128-ECB:" << std::endl;
        std::cout << "  Plaintext: " << plaintext << std::endl;
        std::cout << "  Encrypted: " << hex::encode(encrypted) << std::endl;
        std::cout << "  Decrypted: " << decrypted << std::endl;
    }
    
    // AES-256-CBC
    {
        std::string key = "0123456789ABCDEF0123456789ABCDEF";  // 32 bytes for AES-256
        std::string iv = "1234567890ABCDEF";  // 16 bytes IV
        
        aes cipher(aes::key_size_256);
        
        std::string encrypted = cipher.encrypt(plaintext, key, cipher_mode::cbc, iv);
        std::string decrypted = cipher.decrypt(encrypted, key, cipher_mode::cbc, iv);
        
        std::cout << "\nAES-256-CBC:" << std::endl;
        std::cout << "  Plaintext: " << plaintext << std::endl;
        std::cout << "  Encrypted: " << hex::encode(encrypted) << std::endl;
        std::cout << "  Decrypted: " << decrypted << std::endl;
    }
    
    std::cout << std::endl;
}

void base64_example() {
    std::cout << "=== Base64 Encoding Examples ===" << std::endl;
    
    std::string data = "Hello, libzen!";
    
    // Base64 编码
    std::string encoded = base64::encode(data);
    std::cout << "Original: " << data << std::endl;
    std::cout << "Base64:   " << encoded << std::endl;
    
    // Base64 解码
    std::string decoded = base64::decode(encoded);
    std::cout << "Decoded:  " << decoded << std::endl;
    
    std::cout << std::endl;
}

void hex_example() {
    std::cout << "=== Hex Encoding Examples ===" << std::endl;
    
    std::string data = "ABC";
    
    // Hex 编码
    std::string encoded = hex::encode(data);
    std::cout << "Original: " << data << std::endl;
    std::cout << "Hex:      " << encoded << std::endl;
    
    // Hex 解码
    std::string decoded = hex::decode(encoded);
    std::cout << "Decoded:  " << decoded << std::endl;
    
    std::cout << std::endl;
}

int main() {
    hash_example();
    symmetric_encryption_example();
    base64_example();
    hex_example();
    
    return 0;
}
