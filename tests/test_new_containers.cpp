/**
 * @file test_new_containers.cpp
 * @brief 新容器测试（deque/set/unordered_set/multiset/multimap）
 */

#include <iostream>
#include <cassert>
#include "containers/sequential/deque.h"
#include "containers/associative/set.h"
#include "containers/associative/unordered_set.h"
#include "containers/associative/multiset.h"
#include "containers/associative/multimap.h"

using namespace zen;

// ============================================================================
// Deque 测试
// ============================================================================

void test_deque_basic() {
    std::cout << "测试 Deque 基本操作...\n";

    Deque<int> d;
    d.push_back(1);
    d.push_back(2);
    d.push_back(3);

    assert(d.size() == 3);
    assert(d.front() == 1);
    assert(d.back() == 3);

    d.push_front(0);
    assert(d.front() == 0);
    assert(d.size() == 4);

    std::cout << "  ✓ Deque 基本操作测试通过\n";
}

void test_deque_push_pop() {
    std::cout << "测试 Deque push/pop 操作...\n";

    Deque<int> d;
    d.push_back(1);
    d.push_back(2);
    d.push_front(0);

    d.pop_back();
    assert(d.back() == 1);

    d.pop_front();
    assert(d.front() == 1);

    std::cout << "  ✓ Deque push/pop 操作测试通过\n";
}

void test_deque_random_access() {
    std::cout << "测试 Deque 随机访问...\n";

    Deque<int> d;
    for (int i = 0; i < 10; ++i) {
        d.push_back(i);
    }

    assert(d[0] == 0);
    assert(d[5] == 5);
    assert(d[9] == 9);

    d[3] = 100;
    assert(d[3] == 100);

    std::cout << "  ✓ Deque 随机访问测试通过\n";
}

void test_deque_iterator() {
    std::cout << "测试 Deque 迭代器...\n";

    Deque<int> d;
    for (int i = 0; i < 5; ++i) {
        d.push_back(i);
    }

    int sum = 0;
    for (auto it = d.begin(); it != d.end(); ++it) {
        sum += *it;
    }
    assert(sum == 10);

    std::cout << "  ✓ Deque 迭代器测试通过\n";
}

// ============================================================================
// Set 测试
// ============================================================================

void test_set_basic() {
    std::cout << "测试 Set 基本操作...\n";

    Set<int> s;
    s.insert(3);
    s.insert(1);
    s.insert(4);
    s.insert(1);

    assert(s.size() == 3);
    assert(s.contains(3));
    assert(!s.contains(2));

    std::cout << "  ✓ Set 基本操作测试通过\n";
}

void test_set_ordered() {
    std::cout << "测试 Set 有序性...\n";

    Set<int> s;
    s.insert(3);
    s.insert(1);
    s.insert(4);
    s.insert(2);

    auto it = s.begin();
    assert(*it == 1);
    ++it;
    assert(*it == 2);
    ++it;
    assert(*it == 3);
    ++it;
    assert(*it == 4);

    std::cout << "  ✓ Set 有序性测试通过\n";
}

void test_set_erase() {
    std::cout << "测试 Set 删除操作...\n";

    Set<int> s;
    s.insert(1);
    s.insert(2);
    s.insert(3);

    assert(s.erase(2) == 1);
    assert(!s.contains(2));
    assert(s.size() == 2);

    assert(s.erase(10) == 0);
    assert(s.size() == 2);

    std::cout << "  ✓ Set 删除操作测试通过\n";
}

void test_set_bounds() {
    std::cout << "测试 Set 边界操作...\n";

    Set<int> s;
    for (int i = 1; i <= 5; ++i) {
        s.insert(i);
    }

    auto lb = s.lower_bound(3);
    assert(*lb == 3);

    auto ub = s.upper_bound(3);
    assert(*ub == 4);

    std::cout << "  ✓ Set 边界操作测试通过\n";
}

// ============================================================================
// UnorderedSet 测试
// ============================================================================

void test_unordered_set_basic() {
    std::cout << "测试 UnorderedSet 基本操作...\n";

    UnorderedSet<int> s;
    s.insert(3);
    s.insert(1);
    s.insert(4);
    s.insert(1);

    assert(s.size() == 3);
    assert(s.contains(3));
    assert(!s.contains(2));

    std::cout << "  ✓ UnorderedSet 基本操作测试通过\n";
}

void test_unordered_set_rehash() {
    std::cout << "测试 UnorderedSet rehash...\n";

    UnorderedSet<int> s;
    for (int i = 0; i < 100; ++i) {
        s.insert(i);
    }

    assert(s.size() == 100);
    assert(s.bucket_count() > 8);

    std::cout << "  ✓ UnorderedSet rehash 测试通过\n";
}

void test_unordered_set_reserve() {
    std::cout << "测试 UnorderedSet reserve...\n";

    UnorderedSet<int> s;
    s.reserve(1000);

    assert(s.bucket_count() >= 1000);

    std::cout << "  ✓ UnorderedSet reserve 测试通过\n";
}

// ============================================================================
// MultiSet 测试
// ============================================================================

void test_multiset_basic() {
    std::cout << "测试 MultiSet 基本操作...\n";

    MultiSet<int> ms;
    ms.insert(1);
    ms.insert(2);
    ms.insert(1);  // 重复元素
    ms.insert(3);

    assert(ms.size() == 4);
    assert(ms.count(1) == 2);

    std::cout << "  ✓ MultiSet 基本操作测试通过\n";
}

void test_multiset_equal_range() {
    std::cout << "测试 MultiSet equal_range...\n";

    MultiSet<int> ms;
    ms.insert(1);
    ms.insert(1);
    ms.insert(2);
    ms.insert(2);
    ms.insert(2);

    auto range = ms.equal_range(2);
    assert(std::distance(range.first, range.second) == 3);

    std::cout << "  ✓ MultiSet equal_range 测试通过\n";
}

void test_multiset_erase_all() {
    std::cout << "测试 MultiSet erase_all...\n";

    MultiSet<int> ms;
    ms.insert(1);
    ms.insert(1);
    ms.insert(2);

    assert(ms.erase_all(1) == 2);
    assert(ms.count(1) == 0);

    std::cout << "  ✓ MultiSet erase_all 测试通过\n";
}

// ============================================================================
// MultiMap 测试
// ============================================================================

void test_multimap_basic() {
    std::cout << "测试 MultiMap 基本操作...\n";

    MultiMap<std::string, int> mm;
    mm.insert("apple", 1);
    mm.insert("banana", 2);
    mm.insert("apple", 3);  // 重复键

    assert(mm.size() == 3);
    assert(mm.count("apple") == 2);

    std::cout << "  ✓ MultiMap 基本操作测试通过\n";
}

void test_multimap_get_values() {
    std::cout << "测试 MultiMap get_values...\n";

    MultiMap<std::string, int> mm;
    mm.insert("apple", 1);
    mm.insert("apple", 2);
    mm.insert("apple", 3);

    auto values = mm.get_values("apple");
    assert(values.size() == 3);
    assert(values[0] == 1);
    assert(values[1] == 2);
    assert(values[2] == 3);

    std::cout << "  ✓ MultiMap get_values 测试通过\n";
}

void test_multimap_erase() {
    std::cout << "测试 MultiMap erase...\n";

    MultiMap<std::string, int> mm;
    mm.insert("apple", 1);
    mm.insert("apple", 2);
    mm.insert("banana", 3);

    assert(mm.erase("apple") == 1);
    assert(mm.count("apple") == 1);

    assert(mm.erase_all("apple") == 1);
    assert(mm.count("apple") == 0);

    std::cout << "  ✓ MultiMap erase 测试通过\n";
}

// ============================================================================
// 主测试函数
// ============================================================================

int main() {
    std::cout << "========================================\n";
    std::cout << "    新容器测试\n";
    std::cout << "========================================\n\n";

    // Deque 测试
    std::cout << "--- Deque 测试 ---\n";
    test_deque_basic();
    test_deque_push_pop();
    test_deque_random_access();
    test_deque_iterator();

    // Set 测试
    std::cout << "\n--- Set 测试 ---\n";
    test_set_basic();
    test_set_ordered();
    test_set_erase();
    test_set_bounds();

    // UnorderedSet 测试
    std::cout << "\n--- UnorderedSet 测试 ---\n";
    test_unordered_set_basic();
    test_unordered_set_rehash();
    test_unordered_set_reserve();

    // MultiSet 测试
    std::cout << "\n--- MultiSet 测试 ---\n";
    test_multiset_basic();
    test_multiset_equal_range();
    test_multiset_erase_all();

    // MultiMap 测试
    std::cout << "\n--- MultiMap 测试 ---\n";
    test_multimap_basic();
    test_multimap_get_values();
    test_multimap_erase();

    std::cout << "\n========================================\n";
    std::cout << "    所有测试通过！\n";
    std::cout << "========================================\n";

    return 0;
}
