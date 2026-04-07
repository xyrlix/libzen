/**
 * @file new_containers_example.cpp
 * @brief 新容器使用示例
 */

#include <iostream>
#include "containers/sequential/deque.h"
#include "containers/associative/set.h"
#include "containers/associative/unordered_set.h"
#include "containers/associative/multiset.h"
#include "containers/associative/multimap.h"

using namespace zen;

void example_deque() {
    std::cout << "=== Deque 示例 ===\n";

    Deque<int> dq;

    // 在两端添加元素
    dq.push_back(1);
    dq.push_back(2);
    dq.push_front(0);
    dq.push_back(3);

    std::cout << "当前队列: ";
    for (const auto& item : dq) {
        std::cout << item << " ";
    }
    std::cout << "\n";

    std::cout << "front: " << dq.front() << "\n";
    std::cout << "back: " << dq.back() << "\n";

    // 随机访问
    std::cout << "dq[2]: " << dq[2] << "\n";

    // 从两端删除
    dq.pop_front();
    dq.pop_back();

    std::cout << "删除两端后: ";
    for (const auto& item : dq) {
        std::cout << item << " ";
    }
    std::cout << "\n\n";
}

void example_set() {
    std::cout << "=== Set 示例 ===\n";

    Set<int> s;

    // 插入元素（自动去重）
    s.insert(3);
    s.insert(1);
    s.insert(4);
    s.insert(1);  // 重复元素会被忽略

    std::cout << "Set 大小: " << s.size() << "\n";
    std::cout << "Set 内容: ";
    for (const auto& item : s) {
        std::cout << item << " ";
    }
    std::cout << "\n";

    // 查找元素
    if (s.find(3) != s.end()) {
        std::cout << "找到元素 3\n";
    }

    if (!s.contains(10)) {
        std::cout << "未找到元素 10\n";
    }

    // 边界操作
    auto lb = s.lower_bound(2);
    std::cout << "lower_bound(2): " << *lb << "\n";

    auto ub = s.upper_bound(2);
    std::cout << "upper_bound(2): " << *ub << "\n";

    // 删除元素
    s.erase(1);
    std::cout << "删除 1 后的大小: " << s.size() << "\n\n";
}

void example_unordered_set() {
    std::cout << "=== UnorderedSet 示例 ===\n";

    UnorderedSet<std::string> us;

    // 插入元素
    us.insert("apple");
    us.insert("banana");
    us.insert("cherry");
    us.insert("apple");  // 重复元素会被忽略

    std::cout << "UnorderedSet 大小: " << us.size() << "\n";

    // 查找元素
    if (us.contains("banana")) {
        std::cout << "找到 banana\n";
    }

    // 遍历（顺序不确定）
    std::cout << "UnorderedSet 内容: ";
    for (const auto& item : us) {
        std::cout << item << " ";
    }
    std::cout << "\n";

    // 负载因子
    std::cout << "负载因子: " << us.load_factor() << "\n";
    std::cout << "桶数量: " << us.bucket_count() << "\n\n";
}

void example_multiset() {
    std::cout << "=== MultiSet 示例 ===\n";

    MultiSet<int> ms;

    // 插入元素（允许重复）
    ms.insert(1);
    ms.insert(2);
    ms.insert(1);
    ms.insert(3);
    ms.insert(1);

    std::cout << "MultiSet 大小: " << ms.size() << "\n";
    std::cout << "元素 1 的数量: " << ms.count(1) << "\n";

    std::cout << "MultiSet 内容: ";
    for (const auto& item : ms) {
        std::cout << item << " ";
    }
    std::cout << "\n";

    // 查找所有等于某个值的元素
    auto range = ms.equal_range(1);
    std::cout << "等于 1 的元素: ";
    for (auto it = range.first; it != range.second; ++it) {
        std::cout << *it << " ";
    }
    std::cout << "\n";

    // 删除所有等于某个值的元素
    ms.erase_all(1);
    std::cout << "删除所有 1 后的大小: " << ms.size() << "\n\n";
}

void example_multimap() {
    std::cout << "=== MultiMap 示例 ===\n";

    MultiMap<std::string, int> mm;

    // 插入键值对（允许键重复）
    mm.insert("apple", 10);
    mm.insert("banana", 20);
    mm.insert("apple", 15);
    mm.insert("cherry", 30);
    mm.insert("apple", 25);

    std::cout << "MultiMap 大小: " << mm.size() << "\n";
    std::cout << "键 \"apple\" 的数量: " << mm.count("apple") << "\n";

    // 获取某个键的所有值
    auto apple_values = mm.get_values("apple");
    std::cout << "\"apple\" 的所有值: ";
    for (const auto& value : apple_values) {
        std::cout << value << " ";
    }
    std::cout << "\n";

    // 遍历所有键值对
    std::cout << "所有键值对:\n";
    for (const auto& pair : mm) {
        std::cout << "  " << pair.first << ": " << pair.second << "\n";
    }

    // 删除一个键值对
    mm.erase("apple");
    std::cout << "删除一个 \"apple\" 后的大小: " << mm.size() << "\n";

    // 删除所有某个键的键值对
    mm.erase_all("apple");
    std::cout << "删除所有 \"apple\" 后的大小: " << mm.size() << "\n\n";
}

void example_container_comparison() {
    std::cout << "=== 容器对比示例 ===\n";

    // Set vs MultiSet
    std::cout << "Set vs MultiSet:\n";
    Set<int> s;
    s.insert(1);
    s.insert(1);
    std::cout << "  Set size after inserting 1 twice: " << s.size() << "\n";

    MultiSet<int> ms;
    ms.insert(1);
    ms.insert(1);
    std::cout << "  MultiSet size after inserting 1 twice: " << ms.size() << "\n";

    // Set vs UnorderedSet
    std::cout << "\nSet vs UnorderedSet:\n";
    Set<int> ordered_set;
    ordered_set.insert(3);
    ordered_set.insert(1);
    ordered_set.insert(2);
    std::cout << "  Set (有序): ";
    for (const auto& item : ordered_set) {
        std::cout << item << " ";
    }
    std::cout << "\n";

    UnorderedSet<int> unordered_set;
    unordered_set.insert(3);
    unordered_set.insert(1);
    unordered_set.insert(2);
    std::cout << "  UnorderedSet (无序): ";
    for (const auto& item : unordered_set) {
        std::cout << item << " ";
    }
    std::cout << "\n\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "    新容器示例\n";
    std::cout << "========================================\n\n";

    example_deque();
    example_set();
    example_unordered_set();
    example_multiset();
    example_multimap();
    example_container_comparison();

    return 0;
}
