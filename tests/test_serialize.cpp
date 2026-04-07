#include <gtest/gtest.h>
#include <zen/serialize.h>

using namespace zen;
using namespace zen::serialize;

struct test_struct {
    int a;
    float b;
    std::string c;
    
    template<typename Serializer>
    void serialize(Serializer& s) const {
        s(a);
        s(b);
        s(c);
    }
    
    template<typename Deserializer>
    void deserialize(Deserializer& d) {
        d(a);
        d(b);
        d(c);
    }
    
    bool operator==(const test_struct& other) const {
        return a == other.a && b == other.b && c == other.c;
    }
};

TEST(SerializeTest, Binary) {
    test_struct original{42, 3.14f, "hello"};
    
    binary_serializer bs;
    original.serialize(bs);
    
    binary_deserializer bd(bs.data(), bs.size());
    test_struct loaded;
    loaded.deserialize(bd);
    
    EXPECT_EQ(loaded, original);
}

TEST(SerializeTest, CompactIntegers) {
    compact_binary_serializer bs;
    
    bs.write_varint(42);
    bs.write_varint(255);
    bs.write_varint(65535);
    bs.write_varint(-100);
    
    compact_binary_deserializer bd(bs.data(), bs.size());
    
    EXPECT_EQ(bd.read_varint<int32_t>(), 42);
    EXPECT_EQ(bd.read_varint<int32_t>(), 255);
    EXPECT_EQ(bd.read_varint<int32_t>(), 65535);
    EXPECT_EQ(bd.read_varint<int32_t>(), -100);
}

TEST(SerializeTest, ZigZag) {
    EXPECT_EQ(zigzag_encode(0), 0);
    EXPECT_EQ(zigzag_encode(-1), 1);
    EXPECT_EQ(zigzag_encode(1), 2);
    EXPECT_EQ(zigzag_encode(-2), 3);
    EXPECT_EQ(zigzag_encode(2), 4);
    
    EXPECT_EQ(zigzag_decode(0), 0);
    EXPECT_EQ(zigzag_decode(1), -1);
    EXPECT_EQ(zigzag_decode(2), 1);
    EXPECT_EQ(zigzag_decode(3), -2);
    EXPECT_EQ(zigzag_decode(4), 2);
}

TEST(SerializeTest, JSON) {
    test_struct original{42, 3.14f, "hello"};
    
    json_serializer js;
    original.serialize(js);
    
    json_deserializer jd(js.str());
    test_struct loaded;
    loaded.deserialize(jd);
    
    EXPECT_EQ(loaded, original);
}
