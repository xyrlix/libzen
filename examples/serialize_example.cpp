#include <zen/serialize.h>
#include <iostream>

using namespace zen;
using namespace zen::serialize;

// 可序列化的结构体
struct person {
    std::string name;
    int age;
    float height;
    std::vector<std::string> hobbies;
    
    template<typename Serializer>
    void serialize(Serializer& s) const {
        s(name);
        s(age);
        s(height);
        s(hobbies);
    }
    
    template<typename Deserializer>
    void deserialize(Deserializer& d) {
        d(name);
        d(age);
        d(height);
        d(hobbies);
    }
    
    void print() const {
        std::cout << "Person:" << std::endl;
        std::cout << "  Name: " << name << std::endl;
        std::cout << "  Age: " << age << std::endl;
        std::cout << "  Height: " << height << "m" << std::endl;
        std::cout << "  Hobbies: ";
        for (const auto& h : hobbies) {
            std::cout << h << " ";
        }
        std::cout << std::endl;
    }
};

void binary_serialize_example() {
    std::cout << "=== Binary Serialization Example ===" << std::endl;
    
    person p1;
    p1.name = "Alice";
    p1.age = 30;
    p1.height = 1.68f;
    p1.hobbies = {"reading", "coding", "hiking"};
    
    std::cout << "\nOriginal:" << std::endl;
    p1.print();
    
    // 二进制序列化
    binary_serializer bs;
    p1.serialize(bs);
    
    std::cout << "\nSerialized size: " << bs.size() << " bytes" << std::endl;
    std::cout << "Hex dump: ";
    for (size_t i = 0; i < std::min(size_t(32), bs.size()); ++i) {
        printf("%02x ", (unsigned char)(bs.data()[i]));
    }
    std::cout << std::endl;
    
    // 二进制反序列化
    binary_deserializer bd(bs.data(), bs.size());
    person p2;
    p2.deserialize(bd);
    
    std::cout << "\nDeserialized:" << std::endl;
    p2.print();
    
    std::cout << std::endl;
}

void compact_binary_example() {
    std::cout << "=== Compact Binary Example ===" << std::endl;
    
    compact_binary_serializer cbs;
    
    // 使用可变长度整数
    cbs.write_varint(42);
    cbs.write_varint(255);
    cbs.write_varint(65535);
    cbs.write_varint(-100);
    
    std::cout << "Serialized compact data: " << cbs.size() << " bytes" << std::endl;
    
    // 反序列化
    compact_binary_deserializer cbd(cbs.data(), cbs.size());
    
    std::cout << "Read values:" << std::endl;
    std::cout << "  " << cbd.read_varint<int32_t>() << std::endl;
    std::cout << "  " << cbd.read_varint<int32_t>() << std::endl;
    std::cout << "  " << cbd.read_varint<int32_t>() << std::endl;
    std::cout << "  " << cbd.read_varint<int32_t>() << std::endl;
    
    std::cout << std::endl;
}

void zigzag_example() {
    std::cout << "=== ZigZag Encoding Example ===" << std::endl;
    
    std::vector<int32_t> values = {0, -1, 1, -2, 2, -127, 127, -128, 128};
    
    std::cout << "Original -> ZigZag -> Decoded:" << std::endl;
    for (int32_t v : values) {
        uint32_t encoded = zigzag_encode(v);
        int32_t decoded = zigzag_decode(encoded);
        std::cout << "  " << v << " -> " << encoded << " -> " << decoded << std::endl;
    }
    
    std::cout << std::endl;
}

void json_serialize_example() {
    std::cout << "=== JSON Serialization Example ===" << std::endl;
    
    person p1;
    p1.name = "Bob";
    p1.age = 25;
    p1.height = 1.75f;
    p1.hobbies = {"gaming", "music", "travel"};
    
    std::cout << "\nOriginal:" << std::endl;
    p1.print();
    
    // JSON 序列化
    json_serializer js;
    p1.serialize(js);
    
    std::cout << "\nJSON:" << std::endl;
    std::cout << js.str() << std::endl;
    
    // JSON 反序列化
    json_deserializer jd(js.str());
    person p2;
    p2.deserialize(jd);
    
    std::cout << "\nDeserialized:" << std::endl;
    p2.print();
    
    std::cout << std::endl;
}

int main() {
    binary_serialize_example();
    compact_binary_example();
    zigzag_example();
    json_serialize_example();
    
    return 0;
}
