// test_containers_associative.cpp
// 测试 map/set/unordered_map/unordered_set 容器

#include "../src/containers/associative/map.h"
#include "../src/containers/associative/unordered_map.h"
#include <stdio.h>
#include <string>
#include <cassert>

#define ASSERT_TRUE(cond) do { \
    if (!(cond)) { \
        printf("FAILED at line %d: %s\n", __LINE__, #cond); \
        assert(false); \
    } \
} while(0)

#define ASSERT_FALSE(cond) ASSERT_TRUE(!(cond))
#define ASSERT_EQ(a, b) ASSERT_TRUE((a) == (b))
#define ASSERT_NE(a, b) ASSERT_TRUE((a) != (b))

using namespace zen;

// ===========================================================================
// map 测试
// ===========================================================================

void test_map_insert_find() {
    printf("test_map_insert_find...\n");
    map<int, std::string> m;
    
    auto res1 = m.insert({1, std::string("one")});
    ASSERT_TRUE(res1.second);
    ASSERT_EQ(res1.first->first, 1);
    ASSERT_EQ(res1.first->second, "one");
    
    auto res2 = m.insert({2, std::string("two")});
    ASSERT_TRUE(res2.second);
    
    auto res3 = m.insert({1, std::string("ONE")});
    ASSERT_FALSE(res3.second); // 重复插入
    ASSERT_EQ(res3.first->second, "one"); // 值不变
    
    ASSERT_EQ(m.size(), 2);
    ASSERT_FALSE(m.empty());
}

void test_map_operator_at() {
    printf("test_map_operator_at...\n");
    map<int, std::string> m;
    
    m[1] = "one";
    m[2] = "two";
    m[1] = "ONE";
    
    ASSERT_EQ(m[1], "ONE");
    ASSERT_EQ(m[2], "two");
    ASSERT_EQ(m[3], ""); // 默认构造
    
    ASSERT_EQ(m.size(), 3);
}

void test_map_erase() {
    printf("test_map_erase...\n");
    map<int, std::string> m;
    m.insert({1, "one"});
    m.insert({2, "two"});
    m.insert({3, "three"});
    
    ASSERT_EQ(m.erase(2), 1);
    ASSERT_EQ(m.size(), 2);
    ASSERT_FALSE(m.contains(2));
    
    ASSERT_EQ(m.erase(99), 0); // 不存在
    
    auto it = m.find(1);
    if (it != m.end()) {
        m.erase(it);
    }
    ASSERT_EQ(m.size(), 1);
    ASSERT_TRUE(m.contains(3));
}

void test_map_iterator() {
    printf("test_map_iterator...\n");
    map<int, std::string> m;
    m.insert({3, "three"});
    m.insert({1, "one"});
    m.insert({2, "two"});
    
    // 中序遍历（升序）
    int expected = 1;
    for (auto it = m.begin(); it != m.end(); ++it) {
        ASSERT_EQ(it->first, expected);
        ++expected;
    }
    
    // const 迭代器
    int count = 0;
    for (auto it = m.cbegin(); it != m.cend(); ++it) {
        ++count;
    }
    ASSERT_EQ(count, 3);
}

void test_map_clear() {
    printf("test_map_clear...\n");
    map<int, std::string> m;
    m[1] = "one";
    m[2] = "two";
    m.clear();
    
    ASSERT_TRUE(m.empty());
    ASSERT_EQ(m.size(), 0);
}

void test_map_copy_move() {
    printf("test_map_copy_move...\n");
    map<int, std::string> m1;
    m1[1] = "one";
    m1[2] = "two";
    
    // 拷贝构造
    map<int, std::string> m2(m1);
    ASSERT_EQ(m2.size(), 2);
    ASSERT_EQ(m2[1], "one");
    
    // 移动构造
    map<int, std::string> m3(zen::move(m1));
    ASSERT_EQ(m3.size(), 2);
    ASSERT_TRUE(m1.empty()); // m1 被移动后为空
    
    // 拷贝赋值
    map<int, std::string> m4;
    m4 = m2;
    ASSERT_EQ(m4.size(), 2);
}

void test_map_lower_upper_bound() {
    printf("test_map_lower_upper_bound...\n");
    map<int, std::string> m;
    m[1] = "one";
    m[3] = "three";
    m[5] = "five";
    
    auto lb = m.lower_bound(3);
    ASSERT_EQ(lb->first, 3);
    
    auto ub = m.upper_bound(3);
    ASSERT_EQ(ub->first, 5);
    
    auto lb2 = m.lower_bound(2);
    ASSERT_EQ(lb2->first, 3); // 第一个 >= 2 的
    
    auto ub2 = m.upper_bound(2);
    ASSERT_EQ(ub2->first, 3); // 第一个 > 2 的
}

// ===========================================================================
// set 测试
// ===========================================================================

void test_set_basic() {
    printf("test_set_basic...\n");
    set<int> s;
    
    ASSERT_TRUE(s.insert(1).second);
    ASSERT_TRUE(s.insert(3).second);
    ASSERT_TRUE(s.insert(5).second);
    
    ASSERT_FALSE(s.insert(3).second); // 重复
    ASSERT_EQ(s.size(), 3);
    ASSERT_TRUE(s.contains(3));
    ASSERT_FALSE(s.contains(99));
    
    ASSERT_EQ(s.erase(3), 1);
    ASSERT_FALSE(s.contains(3));
}

// ===========================================================================
// unordered_map 测试
// ===========================================================================

void test_unordered_map_basic() {
    printf("test_unordered_map_basic...\n");
    unordered_map<int, std::string> um;
    
    auto res1 = um.insert({1, std::string("one")});
    ASSERT_TRUE(res1.second);
    
    auto res2 = um.insert({2, std::string("two")});
    ASSERT_TRUE(res2.second);
    
    auto res3 = um.insert({1, std::string("ONE")});
    ASSERT_FALSE(res3.second);
    
    ASSERT_EQ(um.size(), 2);
    ASSERT_TRUE(um.contains(1));
    ASSERT_FALSE(um.contains(99));
    
    um[1] = "ONE";
    ASSERT_EQ(um[1], "ONE");
}

void test_unordered_map_iterator() {
    printf("test_unordered_map_iterator...\n");
    unordered_map<int, std::string> um;
    um[1] = "one";
    um[2] = "two";
    um[3] = "three";
    
    int count = 0;
    for (auto it = um.begin(); it != um.end(); ++it) {
        ++count;
    }
    ASSERT_EQ(count, 3);
    
    // 测试前向迭代器
    for (auto it = um.begin(); it != um.end(); ++it) {
        // 访问键值对
        auto key = it->first;
        auto val = it->second;
        (void)key; (void)val;
    }
}

void test_unordered_map_erase() {
    printf("test_unordered_map_erase...\n");
    unordered_map<int, std::string> um;
    um[1] = "one";
    um[2] = "two";
    um[3] = "three";
    
    ASSERT_EQ(um.erase(2), 1);
    ASSERT_FALSE(um.contains(2));
    ASSERT_EQ(um.size(), 2);
    
    auto it = um.find(1);
    if (it != um.end()) {
        um.erase(it);
    }
    ASSERT_FALSE(um.contains(1));
}

void test_unordered_map_rehash() {
    printf("test_unordered_map_rehash...\n");
    unordered_map<int, std::string> um;
    size_t initial_buckets = um.bucket_count();
    
    // 插入大量元素触发扩容
    for (int i = 0; i < 100; ++i) {
        um[i] = "value";
    }
    
    ASSERT_TRUE(um.bucket_count() > initial_buckets);
    ASSERT_TRUE(um.load_factor() <= 1.0f);
}

// ===========================================================================
// unordered_set 测试
// ===========================================================================

void test_unordered_set_basic() {
    printf("test_unordered_set_basic...\n");
    unordered_set<int> us;
    
    ASSERT_TRUE(us.insert(1).second);
    ASSERT_TRUE(us.insert(2).second);
    ASSERT_TRUE(us.insert(3).second);
    
    ASSERT_FALSE(us.insert(2).second);
    ASSERT_EQ(us.size(), 3);
    
    ASSERT_EQ(us.erase(2), 1);
    ASSERT_FALSE(us.contains(2));
}

// ===========================================================================
// 主函数
// ===========================================================================

int main() {
    printf("=== Containers Associative Tests ===\n\n");
    
    // map tests
    test_map_insert_find();
    test_map_operator_at();
    test_map_erase();
    test_map_iterator();
    test_map_clear();
    test_map_copy_move();
    test_map_lower_upper_bound();
    
    // set tests
    test_set_basic();
    
    // unordered_map tests
    test_unordered_map_basic();
    test_unordered_map_iterator();
    test_unordered_map_erase();
    test_unordered_map_rehash();
    
    // unordered_set tests
    test_unordered_set_basic();
    
    printf("\n=== All tests passed! ===\n");
    return 0;
}
