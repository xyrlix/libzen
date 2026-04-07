#include <gtest/gtest.h>
#include <zen/buffer.h>

using namespace zen;

TEST(DynamicBufferTest, PushBack) {
    dynamic_buffer buf;
    
    buf.push_back('H');
    buf.push_back('e');
    buf.push_back('l');
    buf.push_back('l');
    buf.push_back('o');
    
    EXPECT_EQ(buf.size(), 5);
    EXPECT_EQ(buf[0], 'H');
    EXPECT_EQ(buf[4], 'o');
}

TEST(DynamicBufferTest, Append) {
    dynamic_buffer buf;
    
    std::string data = "Hello World";
    buf.append(data.data(), data.size());
    
    EXPECT_EQ(buf.size(), data.size());
    EXPECT_EQ(std::string(buf.data(), buf.size()), data);
}

TEST(DynamicBufferTest, ReserveAndResize) {
    dynamic_buffer buf;
    
    buf.reserve(100);
    EXPECT_GE(buf.capacity(), 100);
    
    buf.resize(50);
    EXPECT_EQ(buf.size(), 50);
}

TEST(DynamicBufferTest, Erase) {
    dynamic_buffer buf;
    buf.append("Hello World", 11);
    
    buf.erase_front(6);
    EXPECT_EQ(std::string(buf.data(), buf.size()), "World");
    
    buf.erase_back(1);
    EXPECT_EQ(std::string(buf.data(), buf.size()), "Worl");
}

TEST(DynamicBufferTest, ShrinkToFit) {
    dynamic_buffer buf;
    buf.reserve(1000);
    buf.append("Hello", 5);
    
    buf.shrink_to_fit();
    EXPECT_EQ(buf.capacity(), 5);
}

TEST(RingBufferTest, BasicReadWrite) {
    ring_buffer buf(64);
    
    std::string data = "Hello World";
    size_t written = buf.write(data.data(), data.size());
    
    EXPECT_EQ(written, data.size());
    EXPECT_EQ(buf.size(), data.size());
    
    char read_buf[64];
    size_t read = buf.read(read_buf, sizeof(read_buf));
    
    EXPECT_EQ(read, data.size());
    EXPECT_EQ(std::string(read_buf, read), data);
    EXPECT_TRUE(buf.empty());
}

TEST(RingBufferTest, Peek) {
    ring_buffer buf(64);
    
    std::string data = "Hello";
    buf.write(data.data(), data.size());
    
    char peek_buf[64];
    size_t peeked = buf.peek(peek_buf, sizeof(peek_buf));
    
    EXPECT_EQ(peeked, data.size());
    EXPECT_EQ(std::string(peek_buf, peeked), data);
    EXPECT_FALSE(buf.empty());
}

TEST(RingBufferTest, Skip) {
    ring_buffer buf(64);
    
    std::string data = "Hello";
    buf.write(data.data(), data.size());
    
    buf.skip(3);
    EXPECT_EQ(buf.size(), 2);
}

TEST(RingBufferTest, WrapAround) {
    ring_buffer buf(16);
    
    for (int i = 0; i < 10; ++i) {
        buf.write("ABCDEFGHIJ", 10);
        char buf2[16];
        buf.read(buf2, 10);
    }
    
    EXPECT_TRUE(buf.empty());
}

TEST(RingBufferTest, ZeroCopyRead) {
    ring_buffer buf(64);
    
    std::string data = "Hello World";
    buf.write(data.data(), data.size());
    
    void* ptr1;
    size_t size1;
    void* ptr2;
    size_t size2;
    
    size_t total = buf.read_ptr(&ptr1, &size1, &ptr2, &size2);
    
    EXPECT_EQ(total, data.size());
    EXPECT_EQ(std::string(static_cast<char*>(ptr1), size1), data);
}
