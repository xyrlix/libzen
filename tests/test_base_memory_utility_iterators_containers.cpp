#ifndef ZEN_TEST_BASE_MEMORY_UTILITY_ITERATORS_CONTAINERS_CPP
#define ZEN_TEST_BASE_MEMORY_UTILITY_ITERATORS_CONTAINERS_CPP

// ============================================================================
// 综合测试：base / memory / utility / iterators / containers 模块
// ============================================================================

#include "../src/base/type_traits.h"
#include "../src/base/compile_tools.h"
#include "../src/memory/allocator.h"
#include "../src/memory/unique_ptr.h"
#include "../src/memory/shared_ptr.h"
#include "../src/memory/weak_ptr.h"
#include "../src/utility/swap.h"
#include "../src/utility/pair.h"
#include "../src/utility/tuple.h"
#include "../src/utility/optional.h"
#include "../src/iterators/iterator_base.h"
#include "../src/containers/sequential/vector.h"
#include "../src/containers/sequential/list.h"

#include <cstdio>
#include <cstring>
#include <cassert>

#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    printf("Running " #name "... "); \
    test_##name(); \
    printf("OK\n"); \
} while(0)

#define ASSERT_TRUE(cond) do { \
    if (!(cond)) { \
        printf("FAILED at line %d: %s\n", __LINE__, #cond); \
        assert(false); \
    } \
} while(0)

#define ASSERT_FALSE(cond) ASSERT_TRUE(!(cond))
#define ASSERT_EQ(a, b) ASSERT_TRUE((a) == (b))
#define ASSERT_NE(a, b) ASSERT_TRUE((a) != (b))
#define ASSERT_GE(a, b) ASSERT_TRUE((a) >= (b))
#define ASSERT_NE(a, b) ASSERT_TRUE((a) != (b))

namespace zen_test {

using namespace zen;

// ============================================================================
// base/type_traits 测试
// ============================================================================

TEST(type_traits_integral) {
    ASSERT_TRUE(is_integral_v<int>);
    ASSERT_TRUE(is_integral_v<unsigned int>);
    ASSERT_TRUE(is_integral_v<char>);
    ASSERT_TRUE(is_integral_v<bool>);
    ASSERT_FALSE(is_integral_v<float>);
    ASSERT_FALSE(is_integral_v<double>);
}

TEST(type_traits_pointer) {
    ASSERT_TRUE(is_pointer_v<int*>);
    ASSERT_TRUE(is_pointer_v<const int*>);
    ASSERT_FALSE(is_pointer_v<int>);
    ASSERT_FALSE(is_pointer_v<int&>);
}

TEST(type_traits_reference) {
    ASSERT_TRUE(is_reference_v<int&>);
    ASSERT_TRUE(is_reference_v<int&&>);
    ASSERT_FALSE(is_reference_v<int*>);
    ASSERT_FALSE(is_reference_v<int>);
}

TEST(type_traits_const) {
    ASSERT_TRUE(is_const_v<const int>);
    ASSERT_FALSE(is_const_v<int>);
}

TEST(type_traits_void) {
    ASSERT_TRUE(is_void_v<void>);
    ASSERT_FALSE(is_void_v<int>);
}

TEST(type_traits_remove_reference) {
    ASSERT_TRUE((is_same<remove_reference_t<int&>, int>::value));
    ASSERT_TRUE((is_same<remove_reference_t<int&&>, int>::value));
    ASSERT_TRUE((is_same<remove_reference_t<int>, int>::value));
}

TEST(type_traits_remove_cv) {
    ASSERT_TRUE((is_same<remove_cv_t<const volatile int>, int>::value));
    ASSERT_TRUE((is_same<remove_cv_t<const int>, int>::value));
}

TEST(type_traits_remove_pointer) {
    ASSERT_TRUE((is_same<remove_pointer_t<int*>, int>::value));
    ASSERT_TRUE((is_same<remove_pointer_t<int**>, int*>::value));
}

TEST(type_traits_is_same) {
    ASSERT_TRUE(is_same_v<int, int>);
    ASSERT_FALSE(is_same_v<int, double>);
}

TEST(type_traits_array) {
    ASSERT_TRUE(is_array_v<int[10]>);
    ASSERT_TRUE(is_array_v<int[]>);
    ASSERT_FALSE(is_array_v<int*>);
}

TEST(type_traits_signed_unsigned) {
    ASSERT_TRUE(is_signed_v<int>);
    ASSERT_TRUE(is_signed_v<float>);
    ASSERT_TRUE(is_unsigned_v<unsigned int>);
    ASSERT_FALSE(is_unsigned_v<int>);
}

TEST(compile_tools_conditional) {
    ASSERT_TRUE((is_same<conditional_t<true, int, double>, int>::value));
    ASSERT_TRUE((is_same<conditional_t<false, int, double>, double>::value));
}

TEST(compile_tools_enable_if) {
    ASSERT_TRUE((is_same<enable_if_t<true, int>, int>::value));
    // enable_if<false, int>::type 不存在（编译期错误）
}

TEST(compile_tools_integer_sequence) {
    using seq = index_sequence<0, 1, 2, 3, 4>;
    ASSERT_EQ(seq::size(), 5);
}

TEST(compile_tools_make_integer_sequence) {
    using seq = make_index_sequence<5>;
    ASSERT_EQ(seq::size(), 5);
}

TEST(compile_tools_type_list) {
    using tl = type_list<int, double, char>;
    ASSERT_EQ(tl::size(), 3);
    ASSERT_TRUE((is_same<tl::nth_type<0>::type, int>::value));
    ASSERT_TRUE((is_same<tl::nth_type<1>::type, double>::value));
}

TEST(compile_tools_max_min_value) {
    ASSERT_EQ(max_value<int, 1, 5, 3>::value, 5);
    ASSERT_EQ(min_value<int, 1, 5, 3>::value, 1);
}

// ============================================================================
// memory/allocator 测试
// ============================================================================

TEST(allocator_allocate_deallocate) {
    allocator<int> alloc;
    int* p = alloc.allocate(10);
    ASSERT_TRUE(p != nullptr);
    alloc.deallocate(p, 10);
}

TEST(allocator_construct_destroy) {
    allocator<std::pair<int, double>> alloc;
    using PairType = std::pair<int, double>;
    PairType* p = alloc.allocate(1);
    alloc.construct(p, 42, 3.14);
    ASSERT_EQ(p->first, 42);
    ASSERT_TRUE(p->second > 3.0 && p->second < 3.2);
    alloc.destroy(p);
    alloc.deallocate(p, 1);
}

// ============================================================================
// memory/unique_ptr 测试
// ============================================================================

TEST(unique_ptr_basic) {
    unique_ptr<int> ptr(new int(42));
    ASSERT_EQ(*ptr, 42);
    ASSERT_TRUE(ptr.get() != nullptr);
}

TEST(unique_ptr_move) {
    unique_ptr<int> ptr1(new int(42));
    unique_ptr<int> ptr2 = zen::move(ptr1);
    ASSERT_EQ(*ptr2, 42);
    ASSERT_TRUE(ptr1.get() == nullptr);
}

TEST(unique_ptr_reset) {
    unique_ptr<int> ptr(new int(42));
    ptr.reset(new int(100));
    ASSERT_EQ(*ptr, 100);
}

TEST(unique_ptr_release) {
    unique_ptr<int> ptr(new int(42));
    int* raw = ptr.release();
    ASSERT_EQ(*raw, 42);
    ASSERT_TRUE(ptr.get() == nullptr);
    delete raw;
}

TEST(unique_ptr_make_unique) {
    auto ptr = make_unique<int>(42);
    ASSERT_EQ(*ptr, 42);
}

TEST(unique_ptr_array) {
    unique_ptr<int[]> arr(new int[5]);
    arr[0] = 1;
    arr[4] = 5;
    ASSERT_EQ(arr[0], 1);
    ASSERT_EQ(arr[4], 5);
}

// ============================================================================
// memory/shared_ptr 测试
// ============================================================================

TEST(shared_ptr_basic) {
    shared_ptr<int> sp(new int(42));
    ASSERT_EQ(*sp, 42);
    ASSERT_EQ(sp.use_count(), 1);
}

TEST(shared_ptr_copy) {
    shared_ptr<int> sp1(new int(42));
    shared_ptr<int> sp2 = sp1;
    ASSERT_EQ(sp1.use_count(), 2);
    ASSERT_EQ(sp2.use_count(), 2);
    ASSERT_EQ(*sp1, *sp2);
}

TEST(shared_ptr_move) {
    shared_ptr<int> sp1(new int(42));
    shared_ptr<int> sp2 = zen::move(sp1);
    ASSERT_TRUE(sp1.get() == nullptr);
    ASSERT_EQ(sp2.use_count(), 1);
    ASSERT_EQ(*sp2, 42);
}

TEST(shared_ptr_reset) {
    shared_ptr<int> sp(new int(42));
    sp.reset(new int(100));
    ASSERT_EQ(*sp, 100);
    ASSERT_EQ(sp.use_count(), 1);
}

TEST(shared_ptr_make_shared) {
    auto sp = make_shared<int>(42);
    ASSERT_EQ(*sp, 42);
    ASSERT_EQ(sp.use_count(), 1);
}

// ============================================================================
// memory/weak_ptr 测试
// ============================================================================

TEST(weak_ptr_basic) {
    auto sp = make_shared<int>(42);
    weak_ptr<int> wp = sp;
    ASSERT_FALSE(wp.expired());
    ASSERT_EQ(wp.use_count(), 1);
    
    auto sp2 = wp.lock();
    ASSERT_TRUE(sp2);
    ASSERT_EQ(*sp2, 42);
    ASSERT_EQ(wp.use_count(), 2);
}

TEST(weak_ptr_expired) {
    weak_ptr<int> wp;
    {
        auto sp = make_shared<int>(42);
        wp = sp;
        ASSERT_FALSE(wp.expired());
    }
    ASSERT_TRUE(wp.expired());
    auto sp2 = wp.lock();
    ASSERT_FALSE(sp2);
}

TEST(weak_ptr_lock) {
    auto sp = make_shared<int>(42);
    weak_ptr<int> wp = sp;
    auto locked = wp.lock();
    ASSERT_TRUE(locked);
    ASSERT_EQ(*locked, 42);
}

// ============================================================================
// utility/swap 测试
// ============================================================================

TEST(swap_int) {
    int a = 1, b = 2;
    zen::swap(a, b);
    ASSERT_EQ(a, 2);
    ASSERT_EQ(b, 1);
}

TEST(swap_move) {
    struct Test {
        int value;
        Test(int v) : value(v) {}
        Test(Test&& other) noexcept : value(other.value) {}
        Test& operator=(Test&& other) noexcept { value = other.value; return *this; }
    };
    Test a(1), b(2);
    zen::swap(a, b);
    ASSERT_EQ(a.value, 2);
    ASSERT_EQ(b.value, 1);
}

// ============================================================================
// utility/pair 测试
// ============================================================================

TEST(pair_basic) {
    pair<int, double> p(42, 3.14);
    ASSERT_EQ(p.first, 42);
    ASSERT_TRUE(p.second > 3.0);
}

TEST(make_pair) {
    auto p = make_pair(42, "hello");
    ASSERT_EQ(p.first, 42);
    ASSERT_TRUE(strcmp(p.second, "hello") == 0);
}

TEST(pair_comparison) {
    pair<int, int> p1(1, 2);
    pair<int, int> p2(1, 3);
    pair<int, int> p3(2, 1);
    
    ASSERT_TRUE(p1 < p2);
    ASSERT_TRUE(p2 < p3);
    ASSERT_TRUE(p1 == p1);
}

// ============================================================================
// utility/tuple 测试
// ============================================================================

TEST(tuple_basic) {
    tuple<int, double, const char*> t(42, 3.14, "hello");
    ASSERT_EQ(get<0>(t), 42);
    ASSERT_TRUE(get<1>(t) > 3.0);
    ASSERT_TRUE(strcmp(get<2>(t), "hello") == 0);
}

TEST(make_tuple) {
    auto t = make_tuple(42, 3.14, std::string("world"));
    ASSERT_EQ(get<0>(t), 42);
}

TEST(tuple_size) {
    tuple<int, double, char> t;
    ASSERT_EQ(t.size(), 3);
}

TEST(tuple_comparison) {
    tuple<int, int> t1(1, 2);
    tuple<int, int> t2(1, 3);
    tuple<int, int> t3(2, 1);
    
    ASSERT_TRUE(t1 < t2);
    ASSERT_TRUE(t2 < t3);
    ASSERT_TRUE(t1 == t1);
}

// ============================================================================
// utility/optional 测试
// ============================================================================

TEST(optional_basic) {
    optional<int> opt;
    ASSERT_FALSE(opt.has_value());
    
    opt = 42;
    ASSERT_TRUE(opt.has_value());
    ASSERT_EQ(*opt, 42);
}

TEST(optional_value_or) {
    optional<int> opt;
    ASSERT_EQ(opt.value_or(100), 100);
    
    opt = 42;
    ASSERT_EQ(opt.value_or(100), 42);
}

TEST(optional_nullopt) {
    optional<int> opt = nullopt;
    ASSERT_FALSE(opt.has_value());
}

TEST(optional_reset) {
    optional<int> opt = 42;
    opt.reset();
    ASSERT_FALSE(opt.has_value());
}

TEST(optional_emplace) {
    optional<int> opt;
    opt.emplace(42);
    ASSERT_EQ(*opt, 42);
}

// ============================================================================
// iterators/iterator_base 测试
// ============================================================================

TEST(iterator_traits_pointer) {
    using traits = iterator_traits<int*>;
    ASSERT_TRUE((is_same<traits::value_type, int>::value));
    ASSERT_TRUE((is_same<traits::pointer, int*>::value));
    ASSERT_TRUE((is_same<traits::reference, int&>::value));
    ASSERT_TRUE((is_same<traits::iterator_category, random_access_iterator_tag>::value));
}

TEST(iterator_advance) {
    int arr[] = {1, 2, 3, 4, 5};
    int* p = arr;
    advance(p, 3);
    ASSERT_EQ(*p, 4);
}

TEST(iterator_distance) {
    int arr[] = {1, 2, 3, 4, 5};
    auto dist = distance(arr, arr + 5);
    ASSERT_EQ(dist, 5);
}

TEST(reverse_iterator_basic) {
    int arr[] = {1, 2, 3, 4, 5};
    reverse_iterator<int*> rbegin(arr + 5);
    reverse_iterator<int*> rend(arr);
    
    int expected = 5;
    for (auto it = rbegin; it != rend; ++it) {
        ASSERT_EQ(*it, expected--);
    }
}

TEST(reverse_iterator_next_prev) {
    int arr[] = {1, 2, 3, 4, 5};
    reverse_iterator<int*> it(arr + 5);
    
    ASSERT_EQ(*it, 5);
    ASSERT_EQ(*next(it), 4);
    ASSERT_EQ(*prev(next(it)), 5);
}

// ============================================================================
// containers/vector 测试
// ============================================================================

TEST(vector_push_back) {
    vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    
    ASSERT_EQ(v.size(), 3);
    ASSERT_EQ(v[0], 1);
    ASSERT_EQ(v[1], 2);
    ASSERT_EQ(v[2], 3);
}

TEST(vector_resize) {
    vector<int> v;
    v.resize(5, 42);
    
    ASSERT_EQ(v.size(), 5);
    for (size_t i = 0; i < v.size(); ++i) {
        ASSERT_EQ(v[i], 42);
    }
}

TEST(vector_capacity) {
    vector<int> v;
    ASSERT_EQ(v.capacity(), 0);
    
    v.reserve(10);
    ASSERT_GE(v.capacity(), 10);
    ASSERT_EQ(v.size(), 0);
}

TEST(vector_clear) {
    vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.clear();
    
    ASSERT_EQ(v.size(), 0);
    ASSERT_TRUE(v.empty());
}

TEST(vector_iterator) {
    vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    
    int sum = 0;
    for (auto it = v.begin(); it != v.end(); ++it) {
        sum += *it;
    }
    ASSERT_EQ(sum, 6);
}

TEST(vector_erase) {
    vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    v.push_back(4);
    
    v.erase(v.begin() + 1); // 删除 2
    ASSERT_EQ(v.size(), 3);
    ASSERT_EQ(v[0], 1);
    ASSERT_EQ(v[1], 3);
    ASSERT_EQ(v[2], 4);
}

TEST(vector_insert) {
    vector<int> v;
    v.push_back(1);
    v.push_back(3);
    v.insert(v.begin() + 1, 2); // 在 1 和 3 之间插入 2
    
    ASSERT_EQ(v.size(), 3);
    ASSERT_EQ(v[0], 1);
    ASSERT_EQ(v[1], 2);
    ASSERT_EQ(v[2], 3);
}

// ============================================================================
// containers/list 测试
// ============================================================================

TEST(list_push_back) {
    list<int> lst;
    lst.push_back(1);
    lst.push_back(2);
    lst.push_back(3);
    
    ASSERT_EQ(lst.size(), 3);
    
    auto it = lst.begin();
    ASSERT_EQ(*it, 1);
    ++it;
    ASSERT_EQ(*it, 2);
    ++it;
    ASSERT_EQ(*it, 3);
}

TEST(list_push_front) {
    list<int> lst;
    lst.push_front(3);
    lst.push_front(2);
    lst.push_front(1);
    
    auto it = lst.begin();
    ASSERT_EQ(*it, 1);
    ++it;
    ASSERT_EQ(*it, 2);
    ++it;
    ASSERT_EQ(*it, 3);
}

TEST(list_erase) {
    list<int> lst;
    lst.push_back(1);
    lst.push_back(2);
    lst.push_back(3);
    
    lst.erase(lst.begin());
    ASSERT_EQ(lst.size(), 2);
    ASSERT_EQ(*lst.begin(), 2);
}

TEST(list_clear) {
    list<int> lst;
    lst.push_back(1);
    lst.push_back(2);
    lst.clear();
    
    ASSERT_EQ(lst.size(), 0);
    ASSERT_TRUE(lst.empty());
}

TEST(list_iterator) {
    list<int> lst;
    lst.push_back(1);
    lst.push_back(2);
    lst.push_back(3);
    
    int sum = 0;
    for (auto& x : lst) {
        sum += x;
    }
    ASSERT_EQ(sum, 6);
}

// ============================================================================
// 主函数
// ============================================================================

} // namespace zen_test

int main() {
    using namespace zen_test;
    
    printf("=== Base / Type Traits Tests ===\n");
    RUN_TEST(type_traits_integral);
    RUN_TEST(type_traits_pointer);
    RUN_TEST(type_traits_reference);
    RUN_TEST(type_traits_const);
    RUN_TEST(type_traits_void);
    RUN_TEST(type_traits_remove_reference);
    RUN_TEST(type_traits_remove_cv);
    RUN_TEST(type_traits_remove_pointer);
    RUN_TEST(type_traits_is_same);
    RUN_TEST(type_traits_array);
    RUN_TEST(type_traits_signed_unsigned);
    
    printf("\n=== Compile Tools Tests ===\n");
    RUN_TEST(compile_tools_conditional);
    RUN_TEST(compile_tools_enable_if);
    RUN_TEST(compile_tools_integer_sequence);
    RUN_TEST(compile_tools_make_integer_sequence);
    RUN_TEST(compile_tools_type_list);
    RUN_TEST(compile_tools_max_min_value);
    
    printf("\n=== Memory / Allocator Tests ===\n");
    RUN_TEST(allocator_allocate_deallocate);
    RUN_TEST(allocator_construct_destroy);
    
    printf("\n=== Memory / Unique_Ptr Tests ===\n");
    RUN_TEST(unique_ptr_basic);
    RUN_TEST(unique_ptr_move);
    RUN_TEST(unique_ptr_reset);
    RUN_TEST(unique_ptr_release);
    RUN_TEST(unique_ptr_make_unique);
    RUN_TEST(unique_ptr_array);
    
    printf("\n=== Memory / Shared_Ptr Tests ===\n");
    RUN_TEST(shared_ptr_basic);
    RUN_TEST(shared_ptr_copy);
    RUN_TEST(shared_ptr_move);
    RUN_TEST(shared_ptr_reset);
    RUN_TEST(shared_ptr_make_shared);
    
    printf("\n=== Memory / Weak_Ptr Tests ===\n");
    RUN_TEST(weak_ptr_basic);
    RUN_TEST(weak_ptr_expired);
    RUN_TEST(weak_ptr_lock);
    
    printf("\n=== Utility / Swap Tests ===\n");
    RUN_TEST(swap_int);
    RUN_TEST(swap_move);
    
    printf("\n=== Utility / Pair Tests ===\n");
    RUN_TEST(pair_basic);
    RUN_TEST(make_pair);
    RUN_TEST(pair_comparison);
    
    printf("\n=== Utility / Tuple Tests ===\n");
    RUN_TEST(tuple_basic);
    RUN_TEST(make_tuple);
    RUN_TEST(tuple_size);
    RUN_TEST(tuple_comparison);
    
    printf("\n=== Utility / Optional Tests ===\n");
    RUN_TEST(optional_basic);
    RUN_TEST(optional_value_or);
    RUN_TEST(optional_nullopt);
    RUN_TEST(optional_reset);
    RUN_TEST(optional_emplace);
    
    printf("\n=== Iterators Tests ===\n");
    RUN_TEST(iterator_traits_pointer);
    RUN_TEST(iterator_advance);
    RUN_TEST(iterator_distance);
    RUN_TEST(reverse_iterator_basic);
    RUN_TEST(reverse_iterator_next_prev);
    
    printf("\n=== Containers / Vector Tests ===\n");
    RUN_TEST(vector_push_back);
    RUN_TEST(vector_resize);
    RUN_TEST(vector_capacity);
    RUN_TEST(vector_clear);
    RUN_TEST(vector_iterator);
    RUN_TEST(vector_erase);
    RUN_TEST(vector_insert);
    
    printf("\n=== Containers / List Tests ===\n");
    RUN_TEST(list_push_back);
    RUN_TEST(list_push_front);
    RUN_TEST(list_erase);
    RUN_TEST(list_clear);
    RUN_TEST(list_iterator);
    
    printf("\n=== All Tests Passed! ===\n");
    return 0;
}

#endif // ZEN_TEST_BASE_MEMORY_UTILITY_ITERATORS_CONTAINERS_CPP
