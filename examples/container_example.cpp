#include <zen/containers/vector.h>
#include <zen/containers/list.h>
#include <zen/containers/map.h>
#include <zen/containers/unordered_map.h>
#include <iostream>
#include <algorithm>

using namespace zen;

void vector_example() {
    std::cout << "=== Vector Example ===" << std::endl;
    
    zen::vector<int> vec;
    
    // 插入元素
    vec.push_back(10);
    vec.push_back(20);
    vec.push_back(30);
    vec.push_back(40);
    vec.push_back(50);
    
    std::cout << "Vector elements: ";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i] << " ";
    }
    std::cout << std::endl;
    
    // 访问和修改
    vec[2] = 35;
    std::cout << "After modification: ";
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    
    // 在中间插入
    vec.insert(vec.begin() + 2, 25);
    std::cout << "After insert: ";
    for (const auto& v : vec) {
        std::cout << v << " ";
    }
    std::cout << std::endl;
    
    // 删除元素
    vec.erase(vec.begin() + 1);
    std::cout << "After erase: ";
    for (const auto& v : vec) {
        std::cout << v << " ";
    }
    std::cout << std::endl;
    
    // 容量和大小
    std::cout << "Size: " << vec.size() << std::endl;
    std::cout << "Capacity: " << vec.capacity() << std::endl;
    
    std::cout << std::endl;
}

void list_example() {
    std::cout << "=== List Example ===" << std::endl;
    
    zen::list<int> lst;
    
    // 插入元素
    lst.push_back(10);
    lst.push_back(20);
    lst.push_back(30);
    lst.push_front(5);
    
    std::cout << "List elements: ";
    for (auto it = lst.begin(); it != lst.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    
    // 在中间插入
    auto it = lst.begin();
    ++it;
    ++it;
    lst.insert(it, 15);
    
    std::cout << "After insert: ";
    for (const auto& v : lst) {
        std::cout << v << " ";
    }
    std::cout << std::endl;
    
    // 删除元素
    lst.erase(lst.begin());
    lst.pop_back();
    
    std::cout << "After erase: ";
    for (const auto& v : lst) {
        std::cout << v << " ";
    }
    std::cout << std::endl;
    
    // 大小
    std::cout << "Size: " << lst.size() << std::endl;
    
    std::cout << std::endl;
}

void map_example() {
    std::cout << "=== Map Example ===" << std::endl;
    
    zen::map<std::string, int> m;
    
    // 插入键值对
    m["apple"] = 5;
    m["banana"] = 3;
    m["cherry"] = 8;
    m["date"] = 2;
    
    std::cout << "Map contents:" << std::endl;
    for (const auto& kv : m) {
        std::cout << "  " << kv.first << ": " << kv.second << std::endl;
    }
    
    // 查找
    auto it = m.find("banana");
    if (it != m.end()) {
        std::cout << "\nFound banana: " << it->second << std::endl;
    }
    
    // 修改值
    m["banana"] = 10;
    std::cout << "After update, banana: " << m["banana"] << std::endl;
    
    // 删除
    m.erase("cherry");
    std::cout << "\nAfter erase cherry:" << std::endl;
    for (const auto& kv : m) {
        std::cout << "  " << kv.first << ": " << kv.second << std::endl;
    }
    
    // 大小
    std::cout << "\nSize: " << m.size() << std::endl;
    
    std::cout << std::endl;
}

void unordered_map_example() {
    std::cout << "=== Unordered Map Example ===" << std::endl;
    
    zen::unordered_map<int, std::string> m;
    
    // 插入键值对
    m[1] = "one";
    m[2] = "two";
    m[3] = "three";
    m[4] = "four";
    
    std::cout << "Unordered map contents:" << std::endl;
    for (const auto& kv : m) {
        std::cout << "  " << kv.first << ": " << kv.second << std::endl;
    }
    
    // 查找
    auto it = m.find(3);
    if (it != m.end()) {
        std::cout << "\nFound 3: " << it->second << std::endl;
    }
    
    // 负载因子
    std::cout << "\nLoad factor: " << m.load_factor() << std::endl;
    std::cout << "Bucket count: " << m.bucket_count() << std::endl;
    
    // 重哈希
    m.rehash(20);
    std::cout << "After rehash(20), bucket count: " << m.bucket_count() << std::endl;
    
    std::cout << std::endl;
}

int main() {
    vector_example();
    list_example();
    map_example();
    unordered_map_example();
    
    return 0;
}
