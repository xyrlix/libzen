// test_algorithms.cpp
// 测试 algorithms 模块（sort/find/numeric/transform）

#include "../src/algorithms/sort.h"
#include "../src/algorithms/find.h"
#include "../src/algorithms/numeric.h"
#include "../src/algorithms/transform.h"
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
// Sort 算法测试
// ===========================================================================

void test_quick_sort() {
    printf("test_quick_sort...\n");
    int arr[] = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    zen::quick_sort(arr, arr + 9);
    
    for (int i = 0; i < 9; ++i) {
        ASSERT_EQ(arr[i], i + 1);
    }
}

void test_merge_sort() {
    printf("test_merge_sort...\n");
    int arr[] = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    zen::merge_sort(arr, arr + 9);
    
    for (int i = 0; i < 9; ++i) {
        ASSERT_EQ(arr[i], i + 1);
    }
}

void test_heap_sort() {
    printf("test_heap_sort...\n");
    int arr[] = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    zen::heap_sort(arr, arr + 9);
    
    for (int i = 0; i < 9; ++i) {
        ASSERT_EQ(arr[i], i + 1);
    }
}

void test_insertion_sort() {
    printf("test_insertion_sort...\n");
    int arr[] = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    zen::insertion_sort(arr, arr + 9);
    
    for (int i = 0; i < 9; ++i) {
        ASSERT_EQ(arr[i], i + 1);
    }
}

void test_selection_sort() {
    printf("test_selection_sort...\n");
    int arr[] = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    zen::selection_sort(arr, arr + 9);
    
    for (int i = 0; i < 9; ++i) {
        ASSERT_EQ(arr[i], i + 1);
    }
}

void test_sort() {
    printf("test_sort...\n");
    int arr[] = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    zen::sort(arr, arr + 9);
    
    for (int i = 0; i < 9; ++i) {
        ASSERT_EQ(arr[i], i + 1);
    }
}

void test_stable_sort() {
    printf("test_stable_sort...\n");
    int arr[] = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    zen::stable_sort(arr, arr + 9);
    
    for (int i = 0; i < 9; ++i) {
        ASSERT_EQ(arr[i], i + 1);
    }
}

void test_partial_sort() {
    printf("test_partial_sort...\n");
    int arr[] = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    zen::partial_sort(arr, arr + 4, arr + 9); // 前 4 个最小
    
    ASSERT_EQ(arr[0], 1);
    ASSERT_EQ(arr[1], 2);
    ASSERT_EQ(arr[2], 3);
    ASSERT_EQ(arr[3], 4);
}

void test_nth_element() {
    printf("test_nth_element...\n");
    int arr[] = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    zen::nth_element(arr, arr + 4, arr + 9); // 第 4 个元素（下标 4）就位
    
    ASSERT_EQ(arr[4], 5); // 第 5 小的元素
}

void test_is_sorted() {
    printf("test_is_sorted...\n");
    int arr1[] = {1, 2, 3, 4, 5};
    ASSERT_TRUE(zen::is_sorted(arr1, arr1 + 5));
    
    int arr2[] = {1, 3, 2, 4, 5};
    ASSERT_FALSE(zen::is_sorted(arr2, arr2 + 5));
}

// ===========================================================================
// Find 算法测试
// ===========================================================================

void test_find() {
    printf("test_find...\n");
    int arr[] = {1, 3, 5, 7, 9};
    
    auto it1 = zen::find(arr, arr + 5, 5);
    ASSERT_TRUE(it1 != arr + 5);
    ASSERT_EQ(*it1, 5);
    
    auto it2 = zen::find(arr, arr + 5, 4);
    ASSERT_TRUE(it2 == arr + 5); // 未找到
}

void test_find_if() {
    printf("test_find_if...\n");
    int arr[] = {1, 3, 5, 7, 9};
    
    auto it = zen::find_if(arr, arr + 5, [](int x){ return x > 5; });
    ASSERT_EQ(*it, 7);
}

void test_binary_search() {
    printf("test_binary_search...\n");
    int arr[] = {1, 3, 5, 7, 9, 11, 13};
    
    ASSERT_TRUE(zen::binary_search(arr, arr + 7, 7));
    ASSERT_FALSE(zen::binary_search(arr, arr + 7, 6));
}

void test_lower_upper_bound() {
    printf("test_lower_upper_bound...\n");
    int arr[] = {1, 3, 3, 5, 7, 9};
    
    auto lb = zen::lower_bound(arr, arr + 6, 3);
    ASSERT_EQ(lb - arr, 1); // 指向第一个 >= 3 的
    
    auto ub = zen::upper_bound(arr, arr + 6, 3);
    ASSERT_EQ(ub - arr, 3); // 指向第一个 > 3 的
    
    auto lb2 = zen::lower_bound(arr, arr + 6, 4);
    ASSERT_EQ(lb2 - arr, 3); // 4 不存在，指向第一个 > 4 的
    
    auto ub2 = zen::upper_bound(arr, arr + 6, 4);
    ASSERT_EQ(ub2 - arr, 3);
}

void test_adjacent_find() {
    printf("test_adjacent_find...\n");
    int arr1[] = {1, 3, 5, 5, 9};
    auto it1 = zen::adjacent_find(arr1, arr1 + 5);
    ASSERT_EQ(it1 - arr1, 2);
    
    int arr2[] = {1, 3, 5, 7, 9};
    auto it2 = zen::adjacent_find(arr2, arr2 + 5);
    ASSERT_TRUE(it2 == arr2 + 5);
}

void test_count() {
    printf("test_count...\n");
    int arr[] = {1, 3, 3, 5, 3, 7};
    
    size_t c = zen::count(arr, arr + 6, 3);
    ASSERT_EQ(c, 3);
    
    size_t c2 = zen::count_if(arr, arr + 6, [](int x){ return x > 3; });
    ASSERT_EQ(c2, 2);
}

void test_equal() {
    printf("test_equal...\n");
    int arr1[] = {1, 3, 5, 7};
    int arr2[] = {1, 3, 5, 7};
    int arr3[] = {1, 3, 5, 8};
    
    ASSERT_TRUE(zen::equal(arr1, arr1 + 4, arr2));
    ASSERT_FALSE(zen::equal(arr1, arr1 + 4, arr3));
}

void test_search() {
    printf("test_search...\n");
    int arr[] = {1, 2, 3, 4, 5, 1, 2, 3, 4, 5};
    int sub[] = {2, 3, 4};
    
    auto it = zen::search(arr, arr + 10, sub, sub + 3);
    ASSERT_EQ(it - arr, 1); // 找到子序列
}

// ===========================================================================
// Numeric 算法测试
// ===========================================================================

void test_accumulate() {
    printf("test_accumulate...\n");
    int arr[] = {1, 2, 3, 4, 5};
    
    int sum = zen::accumulate(arr, arr + 5, 0);
    ASSERT_EQ(sum, 15);
    
    int prod = zen::accumulate(arr, arr + 5, 1, [](int a, int b){ return a * b; });
    ASSERT_EQ(prod, 120);
}

void test_inner_product() {
    printf("test_inner_product...\n");
    int a[] = {1, 2, 3};
    int b[] = {4, 5, 6};
    
    int result = zen::inner_product(a, a + 3, b, 0);
    ASSERT_EQ(result, 1*4 + 2*5 + 3*6); // 32
}

void test_adjacent_difference() {
    printf("test_adjacent_difference...\n");
    int src[] = {1, 3, 6, 10, 15};
    int dst[5];
    
    zen::adjacent_difference(src, src + 5, dst);
    ASSERT_EQ(dst[0], 1);
    ASSERT_EQ(dst[1], 2);
    ASSERT_EQ(dst[2], 3);
    ASSERT_EQ(dst[3], 4);
    ASSERT_EQ(dst[4], 5);
}

void test_partial_sum() {
    printf("test_partial_sum...\n");
    int src[] = {1, 2, 3, 4, 5};
    int dst[5];
    
    zen::partial_sum(src, src + 5, dst);
    ASSERT_EQ(dst[0], 1);
    ASSERT_EQ(dst[1], 3);
    ASSERT_EQ(dst[2], 6);
    ASSERT_EQ(dst[3], 10);
    ASSERT_EQ(dst[4], 15);
}

void test_iota() {
    printf("test_iota...\n");
    int arr[5];
    zen::iota(arr, arr + 5, 10);
    
    ASSERT_EQ(arr[0], 10);
    ASSERT_EQ(arr[1], 11);
    ASSERT_EQ(arr[2], 12);
    ASSERT_EQ(arr[3], 13);
    ASSERT_EQ(arr[4], 14);
}

void test_min_max_element() {
    printf("test_min_max_element...\n");
    int arr[] = {5, 2, 8, 1, 9, 3};
    
    auto min_it = zen::min_element(arr, arr + 6);
    ASSERT_EQ(*min_it, 1);
    
    auto max_it = zen::max_element(arr, arr + 6);
    ASSERT_EQ(*max_it, 9);
    
    auto p = zen::minmax_element(arr, arr + 6);
    ASSERT_EQ(*p.first, 1);
    ASSERT_EQ(*p.second, 9);
}

void test_min_max() {
    printf("test_min_max...\n");
    ASSERT_EQ(zen::min(3, 5), 3);
    ASSERT_EQ(zen::max(3, 5), 5);
    ASSERT_EQ(zen::min(1, 2, 3), 1);
    ASSERT_EQ(zen::max(1, 2, 3), 3);
}

// ===========================================================================
// Transform 算法测试
// ===========================================================================

void test_for_each() {
    printf("test_for_each...\n");
    int arr[] = {1, 2, 3};
    int sum = 0;
    
    zen::for_each(arr, arr + 3, [&](int x){ sum += x; });
    ASSERT_EQ(sum, 6);
}

void test_transform() {
    printf("test_transform...\n");
    int src[] = {1, 2, 3, 4, 5};
    int dst[5];
    
    zen::transform(src, src + 5, dst, [](int x){ return x * 2; });
    for (int i = 0; i < 5; ++i) {
        ASSERT_EQ(dst[i], (i + 1) * 2);
    }
}

void test_transform_binary() {
    printf("test_transform_binary...\n");
    int a[] = {1, 2, 3};
    int b[] = {4, 5, 6};
    int c[3];
    
    zen::transform(a, a + 3, b, c, [](int x, int y){ return x + y; });
    ASSERT_EQ(c[0], 5);
    ASSERT_EQ(c[1], 7);
    ASSERT_EQ(c[2], 9);
}

void test_copy() {
    printf("test_copy...\n");
    int src[] = {1, 2, 3};
    int dst[3];
    
    auto end = zen::copy(src, src + 3, dst);
    ASSERT_EQ(end - dst, 3);
    ASSERT_EQ(dst[0], 1);
    ASSERT_EQ(dst[1], 2);
    ASSERT_EQ(dst[2], 3);
}

void test_copy_if() {
    printf("test_copy_if...\n");
    int src[] = {1, 2, 3, 4, 5};
    int dst[5];
    int cnt = 0;
    
    auto end = zen::copy_if(src, src + 5, dst, [](int x){ return x % 2 == 0; });
    for (auto it = dst; it != end; ++it) {
        ++cnt;
    }
    ASSERT_EQ(cnt, 2);
}

void test_fill() {
    printf("test_fill...\n");
    int arr[5];
    zen::fill(arr, arr + 5, 42);
    
    for (int i = 0; i < 5; ++i) {
        ASSERT_EQ(arr[i], 42);
    }
}

void test_generate() {
    printf("test_generate...\n");
    int arr[5];
    int val = 0;
    zen::generate(arr, arr + 5, [&](){ return ++val; });
    
    ASSERT_EQ(arr[0], 1);
    ASSERT_EQ(arr[1], 2);
    ASSERT_EQ(arr[2], 3);
    ASSERT_EQ(arr[3], 4);
    ASSERT_EQ(arr[4], 5);
}

void test_replace() {
    printf("test_replace...\n");
    int arr[] = {1, 2, 1, 3, 1, 4};
    zen::replace(arr, arr + 6, 1, 99);
    
    ASSERT_EQ(arr[0], 99);
    ASSERT_EQ(arr[1], 2);
    ASSERT_EQ(arr[2], 99);
    ASSERT_EQ(arr[3], 3);
    ASSERT_EQ(arr[4], 99);
    ASSERT_EQ(arr[5], 4);
}

void test_replace_if() {
    printf("test_replace_if...\n");
    int arr[] = {1, 2, 3, 4, 5};
    zen::replace_if(arr, arr + 5, [](int x){ return x % 2 == 0; }, 99);
    
    ASSERT_EQ(arr[1], 99);
    ASSERT_EQ(arr[3], 99);
}

void test_remove() {
    printf("test_remove...\n");
    int arr[] = {1, 2, 3, 2, 4, 2, 5};
    auto new_end = zen::remove(arr, arr + 7, 2);
    
    ASSERT_EQ(new_end - arr, 4);
    ASSERT_EQ(arr[0], 1);
    ASSERT_EQ(arr[1], 3);
    ASSERT_EQ(arr[2], 4);
    ASSERT_EQ(arr[3], 5);
}

void test_unique() {
    printf("test_unique...\n");
    int arr[] = {1, 2, 2, 3, 3, 3, 4};
    auto new_end = zen::unique(arr, arr + 7);
    
    ASSERT_EQ(new_end - arr, 4);
    ASSERT_EQ(arr[0], 1);
    ASSERT_EQ(arr[1], 2);
    ASSERT_EQ(arr[2], 3);
    ASSERT_EQ(arr[3], 4);
}

void test_reverse() {
    printf("test_reverse...\n");
    int arr[] = {1, 2, 3, 4, 5};
    zen::reverse(arr, arr + 5);
    
    ASSERT_EQ(arr[0], 5);
    ASSERT_EQ(arr[1], 4);
    ASSERT_EQ(arr[2], 3);
    ASSERT_EQ(arr[3], 2);
    ASSERT_EQ(arr[4], 1);
}

void test_rotate() {
    printf("test_rotate...\n");
    int arr[] = {1, 2, 3, 4, 5};
    zen::rotate(arr, arr + 2, arr + 5);
    
    ASSERT_EQ(arr[0], 3);
    ASSERT_EQ(arr[1], 4);
    ASSERT_EQ(arr[2], 5);
    ASSERT_EQ(arr[3], 1);
    ASSERT_EQ(arr[4], 2);
}

void test_shuffle() {
    printf("test_shuffle...\n");
    int arr[] = {1, 2, 3, 4, 5};
    int original[5];
    for (int i = 0; i < 5; ++i) original[i] = arr[i];
    
    zen::shuffle(arr, arr + 5);
    
    // 检查元素是否被打乱（有一定概率相同，忽略）
    // 这里只检查元素数量不变
    int sum = 0;
    for (int i = 0; i < 5; ++i) sum += arr[i];
    ASSERT_EQ(sum, 15);
}

void test_partition() {
    printf("test_partition...\n");
    int arr[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    auto pivot = zen::partition(arr, arr + 9, [](int x){ return x % 2 == 0; });
    
    // 偶数在前，奇数在后
    for (int i = 0; i < pivot - arr; ++i) {
        ASSERT_TRUE(arr[i] % 2 == 0);
    }
    for (auto it = pivot; it != arr + 9; ++it) {
        ASSERT_TRUE(*it % 2 == 1);
    }
}

void test_is_partitioned() {
    printf("test_is_partitioned...\n");
    int arr1[] = {2, 4, 6, 1, 3, 5};
    ASSERT_TRUE(zen::is_partitioned(arr1, arr1 + 6, [](int x){ return x % 2 == 0; }));
    
    int arr2[] = {2, 1, 4, 3, 6, 5};
    ASSERT_FALSE(zen::is_partitioned(arr2, arr2 + 6, [](int x){ return x % 2 == 0; }));
}

// ===========================================================================
// 主函数
// ===========================================================================

int main() {
    printf("=== Algorithms Tests ===\n\n");
    
    // Sort tests
    test_quick_sort();
    test_merge_sort();
    test_heap_sort();
    test_insertion_sort();
    test_selection_sort();
    test_sort();
    test_stable_sort();
    test_partial_sort();
    test_nth_element();
    test_is_sorted();
    
    // Find tests
    test_find();
    test_find_if();
    test_binary_search();
    test_lower_upper_bound();
    test_adjacent_find();
    test_count();
    test_equal();
    test_search();
    
    // Numeric tests
    test_accumulate();
    test_inner_product();
    test_adjacent_difference();
    test_partial_sum();
    test_iota();
    test_min_max_element();
    test_min_max();
    
    // Transform tests
    test_for_each();
    test_transform();
    test_transform_binary();
    test_copy();
    test_copy_if();
    test_fill();
    test_generate();
    test_replace();
    test_replace_if();
    test_remove();
    test_unique();
    test_reverse();
    test_rotate();
    test_shuffle();
    test_partition();
    test_is_partitioned();
    
    printf("\n=== All tests passed! ===\n");
    return 0;
}
