/**
 * @file test_core_verify.cpp
 * @brief 核心模块的编译与功能验证测试
 *
 * 不依赖 GTest，直接使用断言验证，方便快速编译检查。
 */

// 核心头文件
#include "../src/base/type_traits.h"
#include "../src/base/compile_tools.h"
#include "../src/utility/swap.h"
#include "../src/utility/pair.h"
#include "../src/utility/tuple.h"
#include "../src/utility/optional.h"
#include "../src/memory/allocator.h"
#include "../src/memory/unique_ptr.h"
#include "../src/memory/shared_ptr.h"
#include "../src/memory/weak_ptr.h"
#include "../src/iterators/iterator_base.h"
#include "../src/containers/sequential/vector.h"
#include "../src/containers/sequential/list.h"

#include <cassert>
#include <cstdio>

// ============================================================================
// 辅助宏
// ============================================================================

#define CHECK(cond) do {                                         \
    if (!(cond)) {                                               \
        printf("FAIL [%s:%d]: %s\n", __FILE__, __LINE__, #cond);\
        return false;                                            \
    }                                                            \
} while(0)

#define TEST(name) static bool test_##name()
#define RUN(name) do {                              \
    if (test_##name()) printf("PASS: " #name "\n"); \
    else               { ++fail_count; }             \
} while(0)

// ============================================================================
// type_traits 测试
// ============================================================================

TEST(type_traits) {
    // is_integral
    CHECK(zen::is_integral<int>::value == true);
    CHECK(zen::is_integral<double>::value == false);
    CHECK(zen::is_integral<char>::value == true);
    CHECK(zen::is_integral<bool>::value == true);
    CHECK(zen::is_integral<float>::value == false);

    // is_floating_point
    CHECK(zen::is_floating_point<float>::value == true);
    CHECK(zen::is_floating_point<double>::value == true);
    CHECK(zen::is_floating_point<int>::value == false);

    // is_pointer
    CHECK(zen::is_pointer<int*>::value == true);
    CHECK(zen::is_pointer<int>::value == false);

    // is_reference
    CHECK(zen::is_reference<int&>::value == true);
    CHECK(zen::is_reference<int&&>::value == true);
    CHECK(zen::is_reference<int>::value == false);

    // is_const / is_volatile
    CHECK(zen::is_const<const int>::value == true);
    CHECK(zen::is_const<int>::value == false);
    CHECK(zen::is_volatile<volatile int>::value == true);

    // is_void
    CHECK(zen::is_void<void>::value == true);
    CHECK(zen::is_void<int>::value == false);

    // is_same
    CHECK((zen::is_same<int, int>::value) == true);
    CHECK((zen::is_same<int, double>::value) == false);

    // remove_reference
    CHECK((zen::is_same<zen::remove_reference_t<int&>, int>::value));
    CHECK((zen::is_same<zen::remove_reference_t<int&&>, int>::value));
    CHECK((zen::is_same<zen::remove_reference_t<int>, int>::value));

    // remove_cv
    CHECK((zen::is_same<zen::remove_cv_t<const volatile int>, int>::value));

    // conditional
    CHECK((zen::is_same<zen::conditional_t<true, int, double>, int>::value));
    CHECK((zen::is_same<zen::conditional_t<false, int, double>, double>::value));

    // enable_if
    CHECK((zen::is_same<zen::enable_if_t<true, int>, int>::value));

    return true;
}

// ============================================================================
// compile_tools 测试
// ============================================================================

TEST(compile_tools) {
    // integer_sequence
    using seq3 = zen::index_sequence<0, 1, 2>;
    CHECK(seq3::size() == 3);

    // make_index_sequence
    using mseq4 = zen::make_index_sequence<4>;
    CHECK(mseq4::size() == 4);

    // bool_constant
    CHECK(zen::bool_constant<true>::value == true);
    CHECK(zen::bool_constant<false>::value == false);
    CHECK(zen::true_type::value == true);
    CHECK(zen::false_type::value == false);

    // type_size / type_alignment
    CHECK(zen::type_size<int>::value == sizeof(int));
    CHECK(zen::type_alignment<double>::value == alignof(double));

    return true;
}

// ============================================================================
// swap / move / forward 测试
// ============================================================================

TEST(swap_move) {
    // zen::move
    int a = 10;
    int&& moved = zen::move(a);
    CHECK(moved == 10);

    // zen::swap
    int x = 1, y = 2;
    zen::swap(x, y);
    CHECK(x == 2 && y == 1);

    // 数组 swap
    int arr1[] = {1, 2, 3};
    int arr2[] = {4, 5, 6};
    zen::swap(arr1, arr2);
    CHECK(arr1[0] == 4 && arr2[0] == 1);

    return true;
}

// ============================================================================
// pair 测试
// ============================================================================

TEST(pair) {
    zen::pair<int, double> p1(1, 3.14);
    CHECK(p1.first == 1);
    CHECK(p1.second == 3.14);

    // make_pair
    auto p2 = zen::make_pair(42, 'A');
    CHECK(p2.first == 42);
    CHECK(p2.second == 'A');

    // 比较
    zen::pair<int, int> pa(1, 2), pb(1, 3), pc(2, 0);
    CHECK(pa < pb);
    CHECK(pb < pc);
    {
        zen::pair<int,int> expected(1, 2);
        CHECK(pa == expected);
    }
    CHECK(pa != pb);

    // get
    CHECK(zen::get<0>(p1) == 1);
    CHECK(zen::get<1>(p1) == 3.14);

    // swap
    zen::pair<int,int> s1(1,2), s2(3,4);
    s1.swap(s2);
    CHECK(s1.first == 3 && s2.first == 1);

    return true;
}

// ============================================================================
// tuple 测试
// ============================================================================

TEST(tuple) {
    auto t = zen::make_tuple(1, 3.14, 'A');
    CHECK(zen::get<0>(t) == 1);
    CHECK(zen::get<1>(t) == 3.14);
    CHECK(zen::get<2>(t) == 'A');

    // 修改
    zen::get<0>(t) = 99;
    CHECK(zen::get<0>(t) == 99);

    // size
    CHECK(zen::tuple_size_v<decltype(t)> == 3);

    // 空 tuple
    zen::tuple<> empty;
    CHECK(empty.size() == 0);

    // 比较
    auto t1 = zen::make_tuple(1, 2);
    auto t2 = zen::make_tuple(1, 3);
    CHECK(t1 < t2);
    CHECK(t1 != t2);

    return true;
}

// ============================================================================
// optional 测试
// ============================================================================

TEST(optional) {
    // 无值状态
    zen::optional<int> opt;
    CHECK(!opt.has_value());
    CHECK(!opt);

    // 有值状态
    zen::optional<int> opt2(42);
    CHECK(opt2.has_value());
    CHECK(*opt2 == 42);

    // value_or
    CHECK(opt.value_or(-1) == -1);
    CHECK(opt2.value_or(-1) == 42);

    // 赋值
    opt = 100;
    CHECK(*opt == 100);

    // nullopt 赋值
    opt = zen::nullopt;
    CHECK(!opt);

    // emplace
    opt.emplace(200);
    CHECK(*opt == 200);

    // make_optional
    auto o = zen::make_optional(3);
    CHECK(*o == 3);

    // 比较
    zen::optional<int> a(1), b(2), none;
    CHECK(a < b);
    CHECK(a == zen::optional<int>(1));
    CHECK(none == zen::nullopt);
    CHECK(a != none);

    return true;
}

// ============================================================================
// allocator 测试
// ============================================================================

TEST(allocator) {
    zen::allocator<int> alloc;

    // 分配内存
    int* p = alloc.allocate(10);
    CHECK(p != nullptr);

    // 构造对象
    alloc.construct(p, 42);
    CHECK(*p == 42);

    // 析构对象
    alloc.destroy(p);

    // 释放内存
    alloc.deallocate(p, 10);

    // max_size
    CHECK(alloc.max_size() > 0);

    // address
    int val = 123;
    CHECK(alloc.address(val) == &val);

    return true;
}

// ============================================================================
// unique_ptr 测试
// ============================================================================

TEST(unique_ptr) {
    // 基本创建与访问
    zen::unique_ptr<int> p(new int(42));
    CHECK(p);
    CHECK(*p == 42);

    // make_unique
    auto p2 = zen::make_unique<int>(100);
    CHECK(*p2 == 100);

    // release
    int* raw = p.release();
    CHECK(!p);
    CHECK(*raw == 42);
    delete raw;

    // reset
    p2.reset(new int(200));
    CHECK(*p2 == 200);
    p2.reset();
    CHECK(!p2);

    // 移动
    auto p3 = zen::make_unique<int>(999);
    auto p4 = zen::move(p3);
    CHECK(!p3);
    CHECK(*p4 == 999);

    // 数组
    zen::unique_ptr<int[]> arr(new int[5]);
    for (int i = 0; i < 5; ++i) arr[i] = i;
    CHECK(arr[3] == 3);

    // nullptr 比较
    zen::unique_ptr<int> empty;
    CHECK(empty == nullptr);
    CHECK(nullptr == empty);
    CHECK(p4 != nullptr);

    return true;
}

// ============================================================================
// shared_ptr 测试
// ============================================================================

TEST(shared_ptr) {
    // 基本使用
    zen::shared_ptr<int> sp1(new int(42));
    CHECK(sp1);
    CHECK(*sp1 == 42);
    CHECK(sp1.use_count() == 1);

    // 拷贝（引用计数增加）
    zen::shared_ptr<int> sp2 = sp1;
    CHECK(sp1.use_count() == 2);
    CHECK(sp2.use_count() == 2);
    CHECK(*sp2 == 42);

    // make_shared
    auto sp3 = zen::make_shared<int>(100);
    CHECK(*sp3 == 100);
    CHECK(sp3.use_count() == 1);

    // reset
    sp1.reset();
    CHECK(!sp1);
    CHECK(sp2.use_count() == 1); // sp2 仍持有

    // 移动
    zen::shared_ptr<int> sp4 = zen::move(sp2);
    CHECK(!sp2);
    CHECK(*sp4 == 42);

    // 比较
    CHECK(sp3 != sp4);
    zen::shared_ptr<int> sp5 = sp3;
    CHECK(sp5 == sp3);

    return true;
}

// ============================================================================
// weak_ptr 测试
// ============================================================================

TEST(weak_ptr) {
    auto sp = zen::make_shared<int>(42);

    // 创建 weak_ptr
    zen::weak_ptr<int> wp = sp;
    CHECK(!wp.expired());
    CHECK(wp.use_count() == 1);

    // lock
    {
        auto locked = wp.lock();
        CHECK(locked);
        CHECK(*locked == 42);
        CHECK(wp.use_count() == 2);
    }
    CHECK(wp.use_count() == 1);

    // 对象销毁后 weak_ptr 过期
    sp.reset();
    CHECK(wp.expired());
    CHECK(wp.use_count() == 0);

    // lock 失败
    auto locked2 = wp.lock();
    CHECK(!locked2);

    return true;
}

// ============================================================================
// iterator 测试
// ============================================================================

TEST(iterators) {
    int arr[] = {10, 20, 30, 40, 50};

    // advance
    int* p = arr;
    zen::advance(p, 2);
    CHECK(*p == 30);

    // distance
    CHECK(zen::distance(arr, arr + 5) == 5);

    // next / prev
    CHECK(*zen::next(arr, 3) == 40);
    CHECK(*zen::prev(arr + 4, 2) == 30);

    // reverse_iterator
    auto rb = zen::make_reverse_iterator(arr + 5); // 指向 arr[4]
    auto re = zen::make_reverse_iterator(arr);
    CHECK(*rb == 50);
    ++rb;
    CHECK(*rb == 40);
    CHECK(zen::distance(rb, re) == 4);

    return true;
}

// ============================================================================
// vector 测试
// ============================================================================

TEST(vector) {
    zen::vector<int> v;
    CHECK(v.empty());
    CHECK(v.size() == 0);

    // push_back
    for (int i = 0; i < 10; ++i) {
        v.push_back(i);
    }
    CHECK(v.size() == 10);
    CHECK(v[0] == 0);
    CHECK(v[9] == 9);
    CHECK(v.front() == 0);
    CHECK(v.back() == 9);

    // 扩容：再插入更多元素
    for (int i = 10; i < 100; ++i) v.push_back(i);
    CHECK(v.size() == 100);
    CHECK(v[50] == 50);

    // pop_back
    v.pop_back();
    CHECK(v.size() == 99);
    CHECK(v.back() == 98);

    // insert at begin
    v.insert(v.begin(), -1);
    CHECK(v.front() == -1);
    CHECK(v.size() == 100);

    // erase
    v.erase(v.begin());
    CHECK(v.front() == 0);
    CHECK(v.size() == 99);

    // range for
    int sum = 0;
    for (auto x : v) sum += x;
    // 0..98 之和 = 98*99/2 = 4851
    CHECK(sum == 4851);

    // reserve
    v.reserve(200);
    CHECK(v.capacity() >= 200);
    CHECK(v.size() == 99);

    // resize
    v.resize(50);
    CHECK(v.size() == 50);
    v.resize(60, -1);
    CHECK(v.size() == 60);
    CHECK(v[59] == -1);

    // clear
    v.clear();
    CHECK(v.empty());

    // copy constructor
    zen::vector<int> v2;
    v2.push_back(1);
    v2.push_back(2);
    v2.push_back(3);
    zen::vector<int> v3 = v2;
    CHECK(v3.size() == 3);
    CHECK(v3[1] == 2);

    // move constructor
    zen::vector<int> v4 = zen::move(v2);
    CHECK(v4.size() == 3);
    CHECK(v2.empty()); // v2 被移走

    // comparison
    CHECK(v3 == v4);
    v3.push_back(4);
    CHECK(v3 != v4);
    CHECK(v4 < v3);

    // reverse iterators
    zen::vector<int> rv;
    rv.push_back(1); rv.push_back(2); rv.push_back(3);
    int back_sum = 0;
    for (auto it = rv.rbegin(); it != rv.rend(); ++it) back_sum += *it;
    CHECK(back_sum == 6);

    return true;
}

// ============================================================================
// list 测试
// ============================================================================

TEST(list) {
    zen::list<int> lst;
    CHECK(lst.empty());

    // push_back / push_front
    lst.push_back(2);
    lst.push_back(3);
    lst.push_front(1);
    lst.push_front(0);
    CHECK(lst.size() == 4);
    CHECK(lst.front() == 0);
    CHECK(lst.back() == 3);

    // 遍历验证
    int expected[] = {0, 1, 2, 3};
    int idx = 0;
    for (auto& x : lst) {
        CHECK(x == expected[idx++]);
    }

    // pop_front / pop_back
    lst.pop_front();
    lst.pop_back();
    CHECK(lst.front() == 1);
    CHECK(lst.back() == 2);
    CHECK(lst.size() == 2);

    // insert / erase
    auto it = lst.begin();
    ++it; // 指向 2
    lst.insert(it, 99);
    // 1, 99, 2
    CHECK(lst.size() == 3);
    int lst_arr[] = {1, 99, 2};
    idx = 0;
    for (auto& x : lst) CHECK(x == lst_arr[idx++]);

    it = lst.begin(); ++it; // 指向 99
    it = lst.erase(it);     // 删除 99，it 现在指向 2
    CHECK(*it == 2);
    CHECK(lst.size() == 2);

    // reverse
    lst.push_back(3);
    lst.push_front(0);
    // 0, 1, 2, 3
    lst.reverse();
    // 3, 2, 1, 0
    CHECK(lst.front() == 3);
    CHECK(lst.back() == 0);

    // remove
    lst.push_back(1);
    lst.push_back(1);
    // 3, 2, 1, 0, 1, 1
    lst.remove(1);
    // 3, 2, 0
    CHECK(lst.size() == 3);
    CHECK(lst.front() == 3);

    // unique
    zen::list<int> u;
    u.push_back(1); u.push_back(1); u.push_back(2); u.push_back(2); u.push_back(3);
    u.unique();
    CHECK(u.size() == 3);

    // copy ctor
    zen::list<int> cp = u;
    CHECK(cp.size() == u.size());

    // move ctor
    zen::list<int> mv = zen::move(cp);
    CHECK(mv.size() == 3);
    CHECK(cp.empty());

    // clear
    mv.clear();
    CHECK(mv.empty());

    return true;
}

// ============================================================================
// main
// ============================================================================

int main() {
    int fail_count = 0;

    RUN(type_traits);
    RUN(compile_tools);
    RUN(swap_move);
    RUN(pair);
    RUN(tuple);
    RUN(optional);
    RUN(allocator);
    RUN(unique_ptr);
    RUN(shared_ptr);
    RUN(weak_ptr);
    RUN(iterators);
    RUN(vector);
    RUN(list);

    if (fail_count == 0) {
        printf("\n=== ALL TESTS PASSED ===\n");
        return 0;
    } else {
        printf("\n=== %d TEST(S) FAILED ===\n", fail_count);
        return 1;
    }
}
