// ============================================================================
// libzen 容器与算法测试
// ============================================================================

#include <iostream>
#include <cassert>
#include <string>

// 基础模块
#include "src/base/type_traits.h"
#include "src/base/compile_tools.h"
#include "src/utility/swap.h"
#include "src/utility/pair.h"
#include "src/utility/tuple.h"
#include "src/utility/optional.h"

// 内存模块
#include "src/memory/allocator.h"
#include "src/memory/unique_ptr.h"
#include "src/memory/shared_ptr.h"
#include "src/memory/weak_ptr.h"

// 迭代器
#include "src/iterators/iterator_base.h"

// 容器
#include "src/containers/sequential/vector.h"
#include "src/containers/sequential/list.h"
#include "src/containers/associative/map.h"
#include "src/containers/associative/unordered_map.h"

// 算法
#include "src/algorithms/sort.h"
#include "src/algorithms/find.h"
#include "src/algorithms/transform.h"
#include "src/algorithms/numeric.h"

using namespace zen;

// ============================================================================
// 测试工具
// ============================================================================

#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    std::cout << "TEST: " << #name << " ... "; \
    test_##name(); \
    std::cout << "PASS\n"; \
} while(0)

#define CHECK(cond) do { \
    if (!(cond)) { \
        std::cerr << "FAILED: " << #cond << " at line " << __LINE__ << "\n"; \
        throw std::runtime_error("CHECK failed"); \
    } \
} while(0)

// ============================================================================
// map 测试
// ============================================================================

TEST(map_basic) {
    zen::map<int, std::string> m;
    
    // insert
    m.insert({1, std::string("one")});
    m.insert(zen::make_pair(2, std::string("two")));
    m[3] = "three";
    
    CHECK(m.size() == 3);
    CHECK(m.find(1) != m.end());
    CHECK(m.find(2) != m.end());
    CHECK(m.find(3) != m.end());
    
    // find
    auto it = m.find(2);
    CHECK(it->second == "two");
    
    // erase
    m.erase(1);
    CHECK(m.size() == 2);
    CHECK(m.find(1) == m.end());
    
    // contains
    CHECK(m.contains(2));
    CHECK(!m.contains(1));
}

TEST(map_iteration) {
    zen::map<int, int> m;
    for (int i = 5; i >= 1; --i) {
        m[i] = i * 10;
    }
    
    // 验证中序遍历是有序的
    int prev = -1;
    for (auto& kv : m) {
        CHECK(prev < kv.first);
        prev = kv.first;
    }
}

TEST(map_order) {
    zen::map<std::string, int> m;
    m["banana"] = 2;
    m["apple"] = 1;
    m["cherry"] = 3;
    
    auto it = m.begin();
    CHECK(it->first == "apple");
    ++it;
    CHECK(it->first == "banana");
    ++it;
    CHECK(it->first == "cherry");
}

// ============================================================================
// set 测试
// ============================================================================

TEST(set_basic) {
    zen::set<int> s;
    s.insert(3);
    s.insert(1);
    s.insert(2);
    
    CHECK(s.size() == 3);
    CHECK(s.contains(1));
    CHECK(s.contains(2));
    CHECK(s.contains(3));
    CHECK(!s.contains(4));
    
    // 验证有序
    int expected = 1;
    for (auto& kv : s) {
        CHECK(kv.first == expected++);
    }
}

// ============================================================================
// unordered_map 测试
// ============================================================================

TEST(unordered_map_basic) {
    zen::unordered_map<int, std::string> m;
    
    m.insert({1, "one"});
    m[2] = "two";
    m.emplace(3, "three");
    
    CHECK(m.size() == 3);
    CHECK(m.find(1) != m.end());
    CHECK(m.find(2)->second == "two");
    
    m.erase(1);
    CHECK(m.size() == 2);
    CHECK(m.find(1) == m.end());
    
    CHECK(m.contains(2));
    CHECK(!m.contains(1));
}

TEST(unordered_map_rehash) {
    zen::unordered_map<int, int> m;
    
    // 插入足够多元素触发 rehash
    for (int i = 0; i < 100; ++i) {
        m[i] = i * 2;
    }
    
    CHECK(m.size() == 100);
    
    // 验证所有元素都能找到
    for (int i = 0; i < 100; ++i) {
        auto it = m.find(i);
        CHECK(it != m.end());
        CHECK(it->second == i * 2);
    }
}

TEST(unordered_set_basic) {
    zen::unordered_set<int> s;
    s.insert(5);
    s.insert(3);
    s.insert(7);
    
    CHECK(s.size() == 3);
    CHECK(s.contains(5));
    CHECK(!s.contains(10));
}

// ============================================================================
// 算法测试 - 排序
// ============================================================================

TEST(sort_quick) {
    int arr[] = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    constexpr size_t N = sizeof(arr) / sizeof(arr[0]);
    
    zen::sort(arr, arr + N);
    
    for (size_t i = 1; i < N; ++i) {
        CHECK(arr[i-1] <= arr[i]);
    }
}

TEST(sort_stable) {
    // 稳定性测试：值相同的元素应保持原相对顺序
    // 这里简单验证 sort 能正常工作
    int arr[] = {3, 1, 4, 1, 5, 9, 2, 6};
    constexpr size_t N = sizeof(arr) / sizeof(arr[0]);
    
    zen::stable_sort(arr, arr + N);
    
    for (size_t i = 1; i < N; ++i) {
        CHECK(arr[i-1] <= arr[i]);
    }
}

TEST(sort_heap) {
    int arr[] = {5, 2, 8, 1, 9};
    constexpr size_t N = sizeof(arr) / sizeof(arr[0]);
    
    zen::heap_sort(arr, arr + N);
    
    for (size_t i = 1; i < N; ++i) {
        CHECK(arr[i-1] <= arr[i]);
    }
}

TEST(sort_is_sorted) {
    int arr1[] = {1, 2, 3, 4, 5};
    int arr2[] = {1, 3, 2, 4, 5};
    
    CHECK(zen::is_sorted(arr1, arr1 + 5));
    CHECK(!zen::is_sorted(arr2, arr2 + 5));
}

// ============================================================================
// 算法测试 - 查找
// ============================================================================

TEST(find_linear) {
    int arr[] = {3, 1, 4, 1, 5, 9, 2, 6};
    constexpr size_t N = sizeof(arr) / sizeof(arr[0]);
    
    auto it = zen::find(arr, arr + N, 5);
    CHECK(it != arr + N);
    CHECK(*it == 5);
    
    auto it2 = zen::find(arr, arr + N, 100);
    CHECK(it2 == arr + N);
}

TEST(find_binary) {
    int arr[] = {1, 3, 5, 7, 9};
    constexpr size_t N = sizeof(arr) / sizeof(arr[0]);
    
    CHECK(zen::binary_search(arr, arr + N, 5));
    CHECK(!zen::binary_search(arr, arr + N, 4));
    
    auto lb = zen::lower_bound(arr, arr + N, 5);
    CHECK(*lb == 5);
    
    auto ub = zen::upper_bound(arr, arr + N, 5);
    CHECK(*ub == 7 || ub == arr + N);
}

TEST(find_count) {
    int arr[] = {1, 2, 3, 2, 4, 2};
    constexpr size_t N = sizeof(arr) / sizeof(arr[0]);
    
    CHECK(zen::count(arr, arr + N, 2) == 3);
    CHECK(zen::count(arr, arr + N, 5) == 0);
    
    CHECK(zen::count_if(arr, arr + N, [](int x){ return x > 2; }) == 2);
}

// ============================================================================
// 算法测试 - 转换
// ============================================================================

TEST(transform_basic) {
    int src[] = {1, 2, 3, 4, 5};
    int dst[5];
    constexpr size_t N = 5;
    
    zen::transform(src, src + N, dst, [](int x){ return x * 2; });
    
    for (size_t i = 0; i < N; ++i) {
        CHECK(dst[i] == src[i] * 2);
    }
}

TEST(transform_copy) {
    int src[] = {1, 2, 3, 4, 5};
    int dst[5];
    constexpr size_t N = 5;
    
    zen::copy(src, src + N, dst);
    
    for (size_t i = 0; i < N; ++i) {
        CHECK(dst[i] == src[i]);
    }
}

TEST(transform_fill) {
    int arr[5];
    zen::fill(arr, arr + 5, 42);
    
    for (int x : arr) {
        CHECK(x == 42);
    }
}

TEST(transform_remove) {
    int arr[] = {1, 2, 3, 2, 4};
    constexpr size_t N = sizeof(arr) / sizeof(arr[0]);
    
    auto end = zen::remove(arr, arr + N, 2);
    CHECK(end - arr == 3);
    CHECK(arr[0] == 1);
    CHECK(arr[1] == 3);
    CHECK(arr[2] == 4);
}

TEST(transform_reverse) {
    int arr[] = {1, 2, 3, 4, 5};
    zen::reverse(arr, arr + 5);
    
    CHECK(arr[0] == 5);
    CHECK(arr[1] == 4);
    CHECK(arr[2] == 3);
    CHECK(arr[3] == 2);
    CHECK(arr[4] == 1);
}

// ============================================================================
// 算法测试 - 数值
// ============================================================================

TEST(numeric_accumulate) {
    int arr[] = {1, 2, 3, 4, 5};
    constexpr size_t N = sizeof(arr) / sizeof(arr[0]);
    
    int sum = zen::accumulate(arr, arr + N, 0);
    CHECK(sum == 15);
    
    int product = zen::accumulate(arr, arr + N, 1, [](int a, int b){ return a * b; });
    CHECK(product == 120);
}

TEST(numeric_iota) {
    int arr[5];
    zen::iota(arr, arr + 5, 10);
    
    CHECK(arr[0] == 10);
    CHECK(arr[1] == 11);
    CHECK(arr[2] == 12);
    CHECK(arr[3] == 13);
    CHECK(arr[4] == 14);
}

TEST(numeric_minmax) {
    int arr[] = {5, 2, 8, 1, 9, 3};
    constexpr size_t N = sizeof(arr) / sizeof(arr[0]);
    
    auto min_it = zen::min_element(arr, arr + N);
    CHECK(*min_it == 1);
    
    auto max_it = zen::max_element(arr, arr + N);
    CHECK(*max_it == 9);
    
    auto [min_p, max_p] = zen::minmax_element(arr, arr + N);
    CHECK(*min_p == 1);
    CHECK(*max_p == 9);
}

TEST(numeric_partial_sum) {
    int arr[] = {1, 2, 3, 4, 5};
    int result[5];
    constexpr size_t N = sizeof(arr) / sizeof(arr[0]);
    
    zen::partial_sum(arr, arr + N, result);
    
    CHECK(result[0] == 1);
    CHECK(result[1] == 3);
    CHECK(result[2] == 6);
    CHECK(result[3] == 10);
    CHECK(result[4] == 15);
}

// ============================================================================
// 主函数
// ============================================================================

int main() {
    std::cout << "=== libzen Container & Algorithm Tests ===\n\n";
    
    // map
    RUN_TEST(map_basic);
    RUN_TEST(map_iteration);
    RUN_TEST(map_order);
    
    // set
    RUN_TEST(set_basic);
    
    // unordered_map
    RUN_TEST(unordered_map_basic);
    RUN_TEST(unordered_map_rehash);
    RUN_TEST(unordered_set_basic);
    
    // 算法 - 排序
    RUN_TEST(sort_quick);
    RUN_TEST(sort_stable);
    RUN_TEST(sort_heap);
    RUN_TEST(sort_is_sorted);
    
    // 算法 - 查找
    RUN_TEST(find_linear);
    RUN_TEST(find_binary);
    RUN_TEST(find_count);
    
    // 算法 - 转换
    RUN_TEST(transform_basic);
    RUN_TEST(transform_copy);
    RUN_TEST(transform_fill);
    RUN_TEST(transform_remove);
    RUN_TEST(transform_reverse);
    
    // 算法 - 数值
    RUN_TEST(numeric_accumulate);
    RUN_TEST(numeric_iota);
    RUN_TEST(numeric_minmax);
    RUN_TEST(numeric_partial_sum);
    
    std::cout << "\n=== ALL TESTS PASSED ===\n";
    return 0;
}
