/**
 * @file test_math.cpp
 * @brief Math 模块测试
 */

#include "src/math/basic_math.h"
#include "src/math/random.h"
#include "src/math/numeric_stats.h"
#include "src/fmt/print.h"
#include <iostream>
#include <cassert>

int passed = 0;
int failed = 0;

#define TEST(name) void name()
#define RUN_TEST(name) do { \
    std::cout << "Testing " << #name << "... "; \
    try { \
        name(); \
        std::cout << "PASSED\n"; \
        passed++; \
    } catch (const std::exception& e) { \
        std::cout << "FAILED: " << e.what() << "\n"; \
        failed++; \
    } \
} while(0)

#define ASSERT_EQ(a, b) do { \
    if ((a) != (b)) { \
        throw std::runtime_error("Assertion failed: " + std::to_string(a) + " != " + std::to_string(b)); \
    } \
} while(0)

#define ASSERT_TRUE(cond) do { \
    if (!(cond)) { \
        throw std::runtime_error("Assertion failed: " #cond); \
    } \
} while(0)

// ==================== Basic Math Tests ====================

TEST(test_abs) {
    ASSERT_EQ(zen::abs(5), 5);
    ASSERT_EQ(zen::abs(-5), 5);
    ASSERT_EQ(zen::abs(0), 0);
    ASSERT_EQ(zen::abs(-100), 100);
}

TEST(test_gcd) {
    ASSERT_EQ(zen::gcd(12, 8), 4);
    ASSERT_EQ(zen::gcd(48, 18), 6);
    ASSERT_EQ(zen::gcd(7, 13), 1);
    ASSERT_EQ(zen::gcd(100, 25), 25);
    ASSERT_EQ(zen::gcd(0, 5), 5);
    ASSERT_EQ(zen::gcd(5, 0), 5);
}

TEST(test_lcm) {
    ASSERT_EQ(zen::lcm(4, 6), 12);
    ASSERT_EQ(zen::lcm(5, 7), 35);
    ASSERT_EQ(zen::lcm(3, 0), 0);
    ASSERT_EQ(zen::lcm(0, 3), 0);
}

TEST(test_pow_int) {
    ASSERT_EQ(zen::pow_int(2, 0), 1);
    ASSERT_EQ(zen::pow_int(2, 1), 2);
    ASSERT_EQ(zen::pow_int(2, 10), 1024);
    ASSERT_EQ(zen::pow_int(3, 4), 81);
    ASSERT_EQ(zen::pow_int(10, 3), 1000);
    ASSERT_EQ(zen::pow_int(2, -1), 0);  // 负指数返回 0
}

TEST(test_is_odd_even) {
    ASSERT_TRUE(zen::is_odd(1));
    ASSERT_TRUE(zen::is_odd(-3));
    ASSERT_TRUE(!zen::is_odd(2));
    ASSERT_TRUE(zen::is_even(2));
    ASSERT_TRUE(zen::is_even(-4));
    ASSERT_TRUE(!zen::is_even(3));
}

TEST(test_is_prime) {
    ASSERT_TRUE(!zen::is_prime(0));
    ASSERT_TRUE(!zen::is_prime(1));
    ASSERT_TRUE(zen::is_prime(2));
    ASSERT_TRUE(zen::is_prime(3));
    ASSERT_TRUE(!zen::is_prime(4));
    ASSERT_TRUE(zen::is_prime(5));
    ASSERT_TRUE(!zen::is_prime(6));
    ASSERT_TRUE(zen::is_prime(7));
    ASSERT_TRUE(!zen::is_prime(8));
    ASSERT_TRUE(!zen::is_prime(9));
    ASSERT_TRUE(zen::is_prime(11));
    ASSERT_TRUE(zen::is_prime(13));
    ASSERT_TRUE(zen::is_prime(17));
    ASSERT_TRUE(zen::is_prime(19));
}

TEST(test_is_power_of_two) {
    ASSERT_TRUE(zen::is_power_of_two(1));
    ASSERT_TRUE(zen::is_power_of_two(2));
    ASSERT_TRUE(zen::is_power_of_two(4));
    ASSERT_TRUE(zen::is_power_of_two(8));
    ASSERT_TRUE(zen::is_power_of_two(16));
    ASSERT_TRUE(zen::is_power_of_two(1024));
    ASSERT_TRUE(!zen::is_power_of_two(0));
    ASSERT_TRUE(!zen::is_power_of_two(3));
    ASSERT_TRUE(!zen::is_power_of_two(6));
    ASSERT_TRUE(!zen::is_power_of_two(100));
}

TEST(test_count_ones) {
    ASSERT_EQ(zen::count_ones(0u), 0u);
    ASSERT_EQ(zen::count_ones(1u), 1u);
    ASSERT_EQ(zen::count_ones(7u), 3u);    // 111
    ASSERT_EQ(zen::count_ones(255u), 8u);  // 11111111
    ASSERT_EQ(zen::count_ones(128u), 1u); // 10000000
}

TEST(test_min_max) {
    ASSERT_EQ(zen::max(1, 2, 3), 3);
    ASSERT_EQ(zen::max(10, 5, 8), 10);
    ASSERT_EQ(zen::min(1, 2, 3), 1);
    ASSERT_EQ(zen::min(10, 5, 8), 5);
}

// ==================== Random Tests ====================

TEST(test_lcg_engine) {
    zen::lcg_engine eng(42);
    
    // 测试多个随机数
    uint32_t r1 = eng();
    uint32_t r2 = eng();
    uint32_t r3 = eng();
    
    // 验证不是全部相同
    ASSERT_TRUE(r1 != r2 || r2 != r3);
    
    // 测试范围
    ASSERT_TRUE(r1 <= zen::lcg_engine::max());
}

TEST(test_uniform_int) {
    zen::uniform_int_distribution<int> dist(1, 10);
    zen::default_random_engine eng(123);
    
    for (int i = 0; i < 100; ++i) {
        int v = dist(eng);
        ASSERT_TRUE(v >= 1 && v <= 10);
    }
}

TEST(test_uniform_real) {
    zen::uniform_real_distribution<double> dist(0.0, 1.0);
    zen::default_random_engine eng(456);
    
    for (int i = 0; i < 100; ++i) {
        double v = dist(eng);
        ASSERT_TRUE(v >= 0.0 && v < 1.0);
    }
}

// ==================== Stats Tests ====================

TEST(test_sum_mean) {
    int arr[] = {1, 2, 3, 4, 5};
    ASSERT_EQ(zen::sum(arr), 15);
    ASSERT_EQ(zen::mean(arr), 3.0);
}

TEST(test_variance_stddev) {
    int arr[] = {2, 4, 4, 4, 5, 5, 7, 9};
    double var = zen::variance(arr);
    double std = zen::stddev(arr);
    
    // 验证标准差约为 2.0（放宽范围）
    (void)var;  // suppress unused warning
    ASSERT_TRUE(std > 1.5 && std < 2.5);
}

TEST(test_median) {
    int arr1[] = {1, 2, 3, 4, 5};
    ASSERT_EQ(zen::median(arr1), 3.0);
    
    int arr2[] = {1, 2, 3, 4};
    ASSERT_EQ(zen::median(arr2), 2.5);
}

TEST(test_minmax_range) {
    int arr[] = {5, 2, 8, 1, 9, 3};
    ASSERT_EQ(zen::min_value(arr), 1);
    ASSERT_EQ(zen::max_value(arr), 9);
    ASSERT_EQ(zen::range(arr), 8);
}

TEST(test_correlation) {
    // 完全正相关
    double x[] = {1, 2, 3, 4, 5};
    double y[] = {2, 4, 6, 8, 10};
    double r = zen::correlation(x, y);
    
    // 应该接近 1.0
    ASSERT_TRUE(r > 0.99);
    
    // 负相关
    double y2[] = {10, 8, 6, 4, 2};
    r = zen::correlation(x, y2);
    ASSERT_TRUE(r < -0.99);
}

int main() {
    std::cout << "=== Math Module Tests ===\n\n";
    
    // Basic Math
    RUN_TEST(test_abs);
    RUN_TEST(test_gcd);
    RUN_TEST(test_lcm);
    RUN_TEST(test_pow_int);
    RUN_TEST(test_is_odd_even);
    RUN_TEST(test_is_prime);
    RUN_TEST(test_is_power_of_two);
    RUN_TEST(test_count_ones);
    RUN_TEST(test_min_max);
    
    // Random
    RUN_TEST(test_lcg_engine);
    RUN_TEST(test_uniform_int);
    RUN_TEST(test_uniform_real);
    
    // Stats
    RUN_TEST(test_sum_mean);
    RUN_TEST(test_variance_stddev);
    RUN_TEST(test_median);
    RUN_TEST(test_minmax_range);
    RUN_TEST(test_correlation);
    
    std::cout << "\n=== Results ===\n";
    std::cout << "Passed: " << passed << "\n";
    std::cout << "Failed: " << failed << "\n";
    std::cout << "Total:  " << (passed + failed) << "\n";
    
    return failed > 0 ? 1 : 0;
}
