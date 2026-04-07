#include <zen/buffer.h>
#include <iostream>

using namespace zen;

void dynamic_buffer_example() {
    std::cout << "=== Dynamic Buffer Example ===" << std::endl;
    
    dynamic_buffer buf;
    
    // 追加数据
    buf.push_back('H');
    buf.push_back('e');
    buf.push_back('l');
    buf.push_back('l');
    buf.push_back('o');
    
    std::cout << "After push_back: " << std::string(buf.data(), buf.size()) << std::endl;
    
    // 批量追加
    std::string world = ", World!";
    buf.append(world.data(), world.size());
    
    std::cout << "After append: " << std::string(buf.data(), buf.size()) << std::endl;
    
    // 读取数据
    char read_buf[128];
    size_t n = buf.read(0, read_buf, sizeof(read_buf));
    std::cout << "Read " << n << " bytes: " << std::string(read_buf, n) << std::endl;
    
    // 删除数据
    buf.erase_front(7);
    std::cout << "After erase_front(7): " << std::string(buf.data(), buf.size()) << std::endl;
    
    buf.erase_back(1);
    std::cout << "After erase_back(1): " << std::string(buf.data(), buf.size()) << std::endl;
    
    // 容量操作
    std::cout << "\nBuffer info:" << std::endl;
    std::cout << "  Size: " << buf.size() << std::endl;
    std::cout << "  Capacity: " << buf.capacity() << std::endl;
    
    buf.reserve(1024);
    std::cout << "  After reserve(1024), capacity: " << buf.capacity() << std::endl;
    
    buf.shrink_to_fit();
    std::cout << "  After shrink_to_fit, capacity: " << buf.capacity() << std::endl;
    
    std::cout << std::endl;
}

void ring_buffer_example() {
    std::cout << "=== Ring Buffer Example ===" << std::endl;
    
    ring_buffer buf(16);
    
    std::cout << "Ring buffer capacity: " << buf.capacity() << std::endl;
    
    // 写入数据
    std::string msg1 = "Hello";
    std::string msg2 = " World!";
    
    size_t written1 = buf.write(msg1.data(), msg1.size());
    size_t written2 = buf.write(msg2.data(), msg2.size());
    
    std::cout << "Written: " << written1 << " + " << written2 << " bytes" << std::endl;
    std::cout << "Buffer size: " << buf.size() << std::endl;
    std::cout << "Buffer empty: " << (buf.empty() ? "true" : "false") << std::endl;
    std::cout << "Buffer full: " << (buf.full() ? "true" : "false") << std::endl;
    
    // 读取数据
    char read_buf[128];
    size_t read = buf.read(read_buf, sizeof(read_buf));
    
    std::cout << "Read " << read << " bytes: " << std::string(read_buf, read) << std::endl;
    std::cout << "After read, buffer size: " << buf.size() << std::endl;
    
    // 测试环绕
    std::cout << "\nTesting wrap-around:" << std::endl;
    for (int i = 0; i < 20; ++i) {
        std::string data = std::to_string(i);
        buf.write(data.data(), data.size());
        
        char temp[64];
        size_t n = buf.peek(temp, sizeof(temp));
        std::cout << "Write " << data << ", peek: " << std::string(temp, n) << std::endl;
        
        buf.skip(1);
    }
    
    std::cout << std::endl;
}

void zero_copy_example() {
    std::cout << "=== Zero-Copy Example ===" << std::endl;
    
    ring_buffer buf(4096);
    
    // 写入数据
    std::string data = "Hello, World! This is a zero-copy example.";
    buf.write(data.data(), data.size());
    
    // 零拷贝读取
    void* ptr1 = nullptr;
    size_t size1 = 0;
    void* ptr2 = nullptr;
    size_t size2 = 0;
    
    size_t total = buf.read_ptr(&ptr1, &size1, &ptr2, &size2);
    std::cout << "Zero-copy read pointers:" << std::endl;
    std::cout << "  Ptr1: " << ptr1 << ", size: " << size1 << std::endl;
    std::cout << "  Ptr2: " << ptr2 << ", size: " << size2 << std::endl;
    std::cout << "  Total: " << total << std::endl;
    
    if (ptr1 && size1 > 0) {
        std::cout << "  Data from ptr1: " << std::string(static_cast<char*>(ptr1), size1) << std::endl;
    }
    if (ptr2 && size2 > 0) {
        std::cout << "  Data from ptr2: " << std::string(static_cast<char*>(ptr2), size2) << std::endl;
    }
    
    // 提交读取
    buf.commit_read(total);
    std::cout << "After commit, buffer size: " << buf.size() << std::endl;
    
    std::cout << std::endl;
}

int main() {
    dynamic_buffer_example();
    ring_buffer_example();
    zero_copy_example();
    
    return 0;
}
