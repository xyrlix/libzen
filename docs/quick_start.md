# 快速入门指南

## 简介

libzen是一个高效、可靠的C++基础库，提供了内存管理、容器、算法、并发编程、网络编程等核心功能。本指南将帮助您在5分钟内上手libzen。

## 安装

### 从源码编译

```bash
git clone https://github.com/xyrlix/libzen.git
cd libzen
bash scripts/build.sh
sudo bash scripts/install.sh
```

## 第一个程序

创建一个简单的C++文件 `hello_libzen.cpp`：

```cpp
#include <zen/containers/vector.h>
#include <zen/fmt.h>

int main() {
    zen::vector<int> vec = {1, 2, 3, 4, 5};
    zen::print("Hello, libzen! Vector elements: {}", vec);
    return 0;
}
```

编译并运行：

```bash
g++ -std=c++17 hello_libzen.cpp -o hello_libzen -lzen
./hello_libzen
```

输出：
```
Hello, libzen! Vector elements: [1, 2, 3, 4, 5]
```

## 核心功能示例

### 1. 容器使用

```cpp
#include <zen/containers/vector.h>
#include <zen/containers/unordered_map.h>
#include <zen/fmt.h>

int main() {
    // Vector
    zen::vector<std::string> fruits = {"apple", "banana", "cherry"};
    fruits.push_back("date");
    zen::print("Fruits: {}", fruits);
    
    // Unordered map
    zen::unordered_map<std::string, int> scores = { 
        {"Alice", 95}, 
        {"Bob", 88}, 
        {"Charlie", 92} 
    };
    scores["David"] = 85;
    zen::print("Scores: {}", scores);
    
    return 0;
}
```

### 2. 智能指针

```cpp
#include <zen/memory.h>
#include <zen/fmt.h>

class Person {
public:
    Person(const std::string& name, int age) : name_(name), age_(age) {
        zen::print("Person {} created", name_);
    }
    
    ~Person() {
        zen::print("Person {} destroyed", name_);
    }
    
    void say_hello() {
        zen::print("Hello, I'm {} and I'm {} years old", name_, age_);
    }
    
private:
    std::string name_;
    int age_;
};

int main() {
    // Unique pointer (exclusive ownership)
    zen::unique_ptr<Person> person1 = zen::make_unique<Person>("Alice", 25);
    person1->say_hello();
    
    // Shared pointer (shared ownership)
    zen::shared_ptr<Person> person2 = zen::make_shared<Person>("Bob", 30);
    zen::shared_ptr<Person> person3 = person2;
    zen::print("Shared pointer use count: {}", person2.use_count());
    
    return 0;
}
```

### 3. 日志系统

```cpp
#include <zen/logging.h>

int main() {
    zen::logger& logger = zen::logger::get_instance();
    
    logger.debug("This is a debug message");
    logger.info("This is an info message");
    logger.warn("This is a warning message");
    logger.error("This is an error message");
    logger.fatal("This is a fatal message");
    
    return 0;
}
```

### 4. 线程池

```cpp
#include <zen/threading/pool/thread_pool.h>
#include <zen/fmt.h>
#include <chrono>

int main() {
    // Create a thread pool with 4 threads
    zen::thread_pool pool(4);
    
    // Submit tasks
    std::vector<std::future<int>> futures;
    
    for (int i = 0; i < 8; ++i) {
        futures.emplace_back(pool.submit([i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            return i * i;
        }));
    }
    
    // Collect results
    int sum = 0;
    for (auto& future : futures) {
        sum += future.get();
    }
    
    zen::print("Sum of squares: {}", sum);
    
    return 0;
}
```

## 下一步学习

1. 查看更详细的 [API文档](./api/)
2. 了解各模块的 [设计文档](./design/)
3. 运行 [示例程序](../examples/)
4. 编写自己的测试用例
5. 阅读 [编译&安装文档](./build.md) 了解更多构建选项

## 常见问题

### 如何链接libzen？

在CMake项目中：
```cmake
find_package(zen REQUIRED)
target_link_libraries(your_target PRIVATE zen::zen)
```

手动编译：
```bash
g++ -std=c++17 your_file.cpp -o your_program -lzen
```

### 支持哪些编译器？

- GCC 7.1+
- Clang 5.0+
- MSVC 2019+

### 需要什么C++标准？

libzen需要C++17或更高版本。

## 联系与支持

- 提交Issue：https://github.com/xyrlix/libzen/issues
- 贡献代码：https://github.com/xyrlix/libzen/pulls
- 阅读完整文档：https://xyrlix.github.io/libzen/
