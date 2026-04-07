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
#include <string>

using namespace zen;

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

// ============================================================================
// base/type_traits 测试
// ============================================================================

void test_type_traits_integral() {
    ASSERT_TRUE(is_integral_v<int>);
    ASSERT_TRUE(is_integral_v<unsigned int>);
    ASSERT_FALSE(is_integral_v<float>);
}

void test_type_traits_pointer() {
    ASSERT_TRUE(is_pointer_v<int*>);
    ASSERT_FALSE(is_pointer_v<int>);
}

void test_type_traits_reference() {
    ASSERT_TRUE(is_reference_v<int&>);
    ASSERT_FALSE(is_reference_v<int>);
}

void test_type_traits_remove_reference() {
    ASSERT_TRUE((is_same<remove_reference_t<int&>, int>::value));
    ASSERT_TRUE((is_same<remove_reference_t<int&&>, int>::value));
}

void test_type_traits_is_same() {
    ASSERT_TRUE((is_same<int, int>::value));
    ASSERT_FALSE((is_same<int, double>::value));
}

void test_type_traits_array() {
    ASSERT_TRUE(is_array_v<int[10]>);
    ASSERT_FALSE(is_array_v<int*>);
}

// ============================================================================
// compile_tools 测试
// ============================================================================

void test_compile_tools_conditional() {
    ASSERT_TRUE((is_same<conditional_t<true, int, double>, int>::value));
    ASSERT_TRUE((is_same<conditional_t<false, int, double>, double>::value));
}

void test_compile_tools_integer_sequence() {
    using seq = index_sequence<0, 1, 2, 3, 4>;
    ASSERT_EQ(seq::size(), 5);
}

void test_compile_tools_make_integer_sequence() {
    using seq = make_index_sequence<5>;
    ASSERT_EQ(seq::size(), 5);
}

// ============================================================================
// memory/allocator 测试
// ============================================================================

void test_allocator_allocate_deallocate() {
    allocator<int> alloc;
    int* p = alloc.allocate(10);
    ASSERT_TRUE(p != nullptr);
    alloc.deallocate(p, 10);
}

// ============================================================================
// memory/unique_ptr 测试
// ============================================================================

void test_unique_ptr_basic() {
    unique_ptr<int> ptr(new int(42));
    ASSERT_EQ(*ptr, 42);
}

void test_unique_ptr_move() {
    unique_ptr<int> ptr1(new int(42));
    unique_ptr<int> ptr2 = zen::move(ptr1);
    ASSERT_EQ(*ptr2, 42);
    ASSERT_TRUE(ptr1.get() == nullptr);
}

void test_unique_ptr_reset() {
    unique_ptr<int> ptr(new int(42));
    ptr.reset(new int(100));
    ASSERT_EQ(*ptr, 100);
}

void test_unique_ptr_make_unique() {
    auto ptr = make_unique<int>(42);
    ASSERT_EQ(*ptr, 42);
}

// ============================================================================
// memory/shared_ptr 测试
// ============================================================================

void test_shared_ptr_basic() {
    shared_ptr<int> sp(new int(42));
    ASSERT_EQ(*sp, 42);
    ASSERT_EQ(sp.use_count(), 1);
}

void test_shared_ptr_copy() {
    shared_ptr<int> sp1(new int(42));
    shared_ptr<int> sp2 = sp1;
    ASSERT_EQ(sp1.use_count(), 2);
    ASSERT_EQ(sp2.use_count(), 2);
}

void test_shared_ptr_move() {
    shared_ptr<int> sp1(new int(42));
    shared_ptr<int> sp2 = zen::move(sp1);
    ASSERT_TRUE(sp1.get() == nullptr);
    ASSERT_EQ(sp2.use_count(), 1);
    ASSERT_EQ(*sp2, 42);
}

void test_shared_ptr_make_shared() {
    auto sp = make_shared<int>(42);
    ASSERT_EQ(*sp, 42);
    ASSERT_EQ(sp.use_count(), 1);
}

// ============================================================================
// memory/weak_ptr 测试
// ============================================================================

void test_weak_ptr_basic() {
    auto sp = make_shared<int>(42);
    weak_ptr<int> wp = sp;
    ASSERT_FALSE(wp.expired());
    ASSERT_EQ(wp.use_count(), 1);
    
    auto sp2 = wp.lock();
    ASSERT_TRUE(sp2);
    ASSERT_EQ(*sp2, 42);
}

void test_weak_ptr_expired() {
    weak_ptr<int> wp;
    {
        auto sp = make_shared<int>(42);
        wp = sp;
        ASSERT_FALSE(wp.expired());
    }
    ASSERT_TRUE(wp.expired());
}

// ============================================================================
// utility/swap 测试
// ============================================================================

void test_swap_int() {
    int a = 1, b = 2;
    zen::swap(a, b);
    ASSERT_EQ(a, 2);
    ASSERT_EQ(b, 1);
}

// ============================================================================
// utility/pair 测试
// ============================================================================

void test_pair_basic() {
    pair<int, double> p(42, 3.14);
    ASSERT_EQ(p.first, 42);
    ASSERT_TRUE(p.second > 3.0);
}

void test_make_pair() {
    pair<int, std::string> p = zen::make_pair(42, std::string("hello"));
    ASSERT_EQ(p.first, 42);
    ASSERT_TRUE(p.second == "hello");
}

void test_pair_comparison() {
    pair<int, int> p1(1, 2);
    pair<int, int> p2(1, 3);
    ASSERT_TRUE(p1 < p2);
    ASSERT_TRUE(p1 == p1);
}

// ============================================================================
// utility/tuple 测试
// ============================================================================

void test_tuple_basic() {
    tuple<int, double, std::string> t(42, 3.14, "hello");
    ASSERT_EQ(get<0>(t), 42);
    ASSERT_TRUE(get<1>(t) > 3.0);
    ASSERT_TRUE(get<2>(t) == "hello");
}

void test_make_tuple() {
    tuple<int, double, std::string> t = zen::make_tuple(42, 3.14, std::string("world"));
    ASSERT_EQ(get<0>(t), 42);
}

void test_tuple_size() {
    tuple<int, double, char> t;
    ASSERT_EQ(t.size(), 3);
}

void test_tuple_comparison() {
    tuple<int, int> t1(1, 2);
    tuple<int, int> t2(1, 3);
    ASSERT_TRUE(t1 < t2);
}

// ============================================================================
// utility/optional 测试
// ============================================================================

void test_optional_basic() {
    optional<int> opt;
    ASSERT_FALSE(opt.has_value());
    
    opt = 42;
    ASSERT_TRUE(opt.has_value());
    ASSERT_EQ(*opt, 42);
}

void test_optional_value_or() {
    optional<int> opt;
    ASSERT_EQ(opt.value_or(100), 100);
    
    opt = 42;
    ASSERT_EQ(opt.value_or(100), 42);
}

void test_optional_nullopt() {
    optional<int> opt = nullopt;
    ASSERT_FALSE(opt.has_value());
}

void test_optional_reset() {
    optional<int> opt = 42;
    opt.reset();
    ASSERT_FALSE(opt.has_value());
}

void test_optional_emplace() {
    optional<int> opt;
    opt.emplace(42);
    ASSERT_EQ(*opt, 42);
}

// ============================================================================
// iterators 测试
// ============================================================================

void test_iterator_traits_pointer() {
    using traits = iterator_traits<int*>;
    ASSERT_TRUE((is_same<traits::value_type, int>::value));
    ASSERT_TRUE((is_same<traits::pointer, int*>::value));
}

void test_iterator_advance() {
    int arr[] = {1, 2, 3, 4, 5};
    int* p = arr;
    advance(p, 3);
    ASSERT_EQ(*p, 4);
}

void test_iterator_distance() {
    int arr[] = {1, 2, 3, 4, 5};
    auto dist = distance(arr, arr + 5);
    ASSERT_EQ(dist, 5);
}

void test_reverse_iterator_basic() {
    int arr[] = {1, 2, 3, 4, 5};
    reverse_iterator<int*> rbegin(arr + 5);
    reverse_iterator<int*> rend(arr);
    
    int expected = 5;
    for (auto it = rbegin; it != rend; ++it) {
        ASSERT_EQ(*it, expected--);
    }
}

// ============================================================================
// containers/vector 测试
// ============================================================================

void test_vector_push_back() {
    vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    
    ASSERT_EQ(v.size(), 3);
    ASSERT_EQ(v[0], 1);
    ASSERT_EQ(v[1], 2);
    ASSERT_EQ(v[2], 3);
}

void test_vector_resize() {
    vector<int> v;
    v.resize(5, 42);
    
    ASSERT_EQ(v.size(), 5);
    for (size_t i = 0; i < v.size(); ++i) {
        ASSERT_EQ(v[i], 42);
    }
}

void test_vector_capacity() {
    vector<int> v;
    ASSERT_EQ(v.capacity(), 0);
    
    v.reserve(10);
    ASSERT_GE(v.capacity(), 10);
}

void test_vector_clear() {
    vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.clear();
    
    ASSERT_EQ(v.size(), 0);
}

void test_vector_iterator() {
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

void test_vector_erase() {
    vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    v.push_back(4);
    
    v.erase(v.begin() + 1);
    ASSERT_EQ(v.size(), 3);
    ASSERT_EQ(v[0], 1);
    ASSERT_EQ(v[1], 3);
}

void test_vector_insert() {
    vector<int> v;
    v.push_back(1);
    v.push_back(3);
    v.insert(v.begin() + 1, 2);
    
    ASSERT_EQ(v.size(), 3);
    ASSERT_EQ(v[0], 1);
    ASSERT_EQ(v[1], 2);
    ASSERT_EQ(v[2], 3);
}

// ============================================================================
// containers/list 测试
// ============================================================================

void test_list_push_back() {
    list<int> lst;
    lst.push_back(1);
    lst.push_back(2);
    lst.push_back(3);
    
    ASSERT_EQ(lst.size(), 3);
    
    auto it = lst.begin();
    ASSERT_EQ(*it, 1);
    ++it;
    ASSERT_EQ(*it, 2);
}

void test_list_push_front() {
    list<int> lst;
    lst.push_front(3);
    lst.push_front(2);
    lst.push_front(1);
    
    auto it = lst.begin();
    ASSERT_EQ(*it, 1);
    ++it;
    ASSERT_EQ(*it, 2);
}

void test_list_erase() {
    list<int> lst;
    lst.push_back(1);
    lst.push_back(2);
    lst.push_back(3);
    
    lst.erase(lst.begin());
    ASSERT_EQ(lst.size(), 2);
    ASSERT_EQ(*lst.begin(), 2);
}

void test_list_clear() {
    list<int> lst;
    lst.push_back(1);
    lst.push_back(2);
    lst.clear();
    
    ASSERT_EQ(lst.size(), 0);
}

void test_list_iterator() {
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

int main() {
    printf("=== Base / Type Traits Tests ===\n");
    RUN_TEST(type_traits_integral);
    RUN_TEST(type_traits_pointer);
    RUN_TEST(type_traits_reference);
    RUN_TEST(type_traits_remove_reference);
    RUN_TEST(type_traits_is_same);
    RUN_TEST(type_traits_array);
    
    printf("\n=== Compile Tools Tests ===\n");
    RUN_TEST(compile_tools_conditional);
    RUN_TEST(compile_tools_integer_sequence);
    RUN_TEST(compile_tools_make_integer_sequence);
    
    printf("\n=== Memory / Allocator Tests ===\n");
    RUN_TEST(allocator_allocate_deallocate);
    
    printf("\n=== Memory / Unique_Ptr Tests ===\n");
    RUN_TEST(unique_ptr_basic);
    RUN_TEST(unique_ptr_move);
    RUN_TEST(unique_ptr_reset);
    RUN_TEST(unique_ptr_make_unique);
    
    printf("\n=== Memory / Shared_Ptr Tests ===\n");
    RUN_TEST(shared_ptr_basic);
    RUN_TEST(shared_ptr_copy);
    RUN_TEST(shared_ptr_move);
    RUN_TEST(shared_ptr_make_shared);
    
    printf("\n=== Memory / Weak_Ptr Tests ===\n");
    RUN_TEST(weak_ptr_basic);
    RUN_TEST(weak_ptr_expired);
    
    printf("\n=== Utility / Swap Tests ===\n");
    RUN_TEST(swap_int);
    
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
