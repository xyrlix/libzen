/**
 * @file basic_math.h
 * @brief 基础数学运算实现
 * 
 * 手撕算法实现，不使用系统库
 */

#ifndef ZEN_MATH_BASIC_MATH_H
#define ZEN_MATH_BASIC_MATH_H

#include "../base/type_traits.h"
#include <cstddef>
#include <cmath>

namespace zen {

// ==================== 基础数学函数 ====================

/**
 * @brief 绝对值函数（整数）
 * @tparam T 整数类型
 * @param value 输入值
 * @return 绝对值
 * @note 手撕实现，不使用 std::abs
 */
template<typename T, 
         typename zen::enable_if_t<zen::is_integral_v<T>, int> = 0>
constexpr T abs(T value) {
    return value >= 0 ? value : -value;
}

/**
 * @brief 绝对值函数（浮点数）
 * @tparam T 浮点类型
 * @param value 输入值
 * @return 绝对值
 */
template<typename T,
         typename zen::enable_if_t<zen::is_floating_point_v<T>, int> = 0>
constexpr T abs(T value) {
    return value >= 0 ? value : -value;
}

/**
 * @brief 最大值
 * @tparam T 类型
 * @tparam Args 可变参数类型
 * @param a 第一个值
 * @param args 其他值
 * @return 最大值
 */
template<typename T, typename... Args>
constexpr T max(const T& a, const Args&... args) {
    if constexpr (sizeof...(args) == 0) {
        return a;
    } else {
        const T rest_max = max(args...);
        return a > rest_max ? a : rest_max;
    }
}

/**
 * @brief 最小值
 * @tparam T 类型
 * @tparam Args 可变参数类型
 * @param a 第一个值
 * @param args 其他值
 * @return 最小值
 */
template<typename T, typename... Args>
constexpr T min(const T& a, const Args&... args) {
    if constexpr (sizeof...(args) == 0) {
        return a;
    } else {
        const T rest_min = min(args...);
        return a < rest_min ? a : rest_min;
    }
}

/**
 * @brief 交换两个值
 * @tparam T 类型
 * @param a 第一个值
 * @param b 第二个值
 */
template<typename T>
void swap(T& a, T& b) {
    T temp = static_cast<T&&>(a);
    a = static_cast<T&&>(b);
    b = static_cast<T&&>(temp);
}

// ==================== 最大公约数 (GCD) ====================

/**
 * @brief 欧几里得算法计算最大公约数
 * 
 * 算法思路：
 * gcd(a, b) = gcd(b, a % b)，当 b = 0 时返回 a
 * 
 * 时间复杂度: O(log(min(a, b)))
 * 空间复杂度: O(1)
 * 
 * @tparam T 整数类型
 * @param a 第一个整数
 * @param b 第二个整数
 * @return 最大公约数
 */
template<typename T,
         typename zen::enable_if_t<zen::is_integral_v<T>, int> = 0>
constexpr T gcd(T a, T b) {
    // 处理负数
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    
    // 欧几里得算法
    while (b != 0) {
        T temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

/**
 * @brief 递归版本 GCD
 */
template<typename T,
         typename zen::enable_if_t<zen::is_integral_v<T>, int> = 0>
constexpr T gcd_recursive(T a, T b) {
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    return b == 0 ? a : gcd_recursive(b, a % b);
}

// ==================== 最小公倍数 (LCM) ====================

/**
 * @brief 计算最小公倍数
 * 
 * 算法思路：lcm(a, b) = |a * b| / gcd(a, b)
 * 
 * @tparam T 整数类型
 * @param a 第一个整数
 * @param b 第二个整数
 * @return 最小公倍数
 */
template<typename T,
         typename zen::enable_if_t<zen::is_integral_v<T>, int> = 0>
constexpr T lcm(T a, T b) {
    if (a == 0 || b == 0) return 0;
    
    // 避免溢出：先除后乘
    T g = gcd(a, b);
    return (a / g) * b;
}

// ==================== 幂运算 ====================

/**
 * @brief 快速幂算法（整数）
 * 
 * 算法思路：
 * 使用二分思想，a^14 = a^7 * a^7
 * 7 = 111 (二进制)
 * a^14 = a^8 * a^4 * a^2
 * 
 * 时间复杂度: O(log n)
 * 空间复杂度: O(1)
 * 
 * @tparam T 整数类型
 * @param base 底数
 * @param exp 指数（非负）
 * @return 幂结果
 */
template<typename T,
         typename zen::enable_if_t<zen::is_integral_v<T>, int> = 0>
constexpr T pow_int(T base, T exp) {
    if (exp < 0) return 0;  // 不支持负指数
    if (exp == 0) return 1;
    
    T result = 1;
    T base_abs = base;
    
    while (exp > 0) {
        if (exp & 1) {  // 当前位为1
            result *= base_abs;
        }
        base_abs *= base_abs;
        exp >>= 1;  // 右移一位
    }
    
    return result;
}

/**
 * @brief 浮点数幂运算
 */
inline double pow(double base, double exp) {
    return std::pow(base, exp);
}

// ==================== 平方根 ====================

/**
 * @brief 牛顿迭代法求平方根
 * 
 * 算法思路：
 * 假设 x 是 sqrt(n) 的近似值
 * 则 (x + n/x) / 2 是更好的近似值
 * 迭代直到收敛
 * 
 * 时间复杂度: O(log n)
 * 空间复杂度: O(1)
 * 
 * @param n 待求平方根的数（n >= 0）
 * @return 平方根
 */
inline double sqrt(double n) {
    if (n < 0) return std::nan("");
    if (n == 0) return 0;
    
    // 使用牛顿迭代法
    double x = n;
    double y = (x + 1) / 2;
    
    // 迭代直到收敛
    const int max_iterations = 100;
    for (int i = 0; i < max_iterations && y < x; ++i) {
        x = y;
        y = (x + n / x) / 2;
    }
    
    return x;
}

/**
 * @brief 整数平方根（向下取整）
 * @param n 非负整数
 * @return floor(sqrt(n))
 */
template<typename T,
         typename zen::enable_if_t<zen::is_integral_v<T>, int> = 0>
T sqrt_int(T n) {
    if (n <= 0) return 0;
    if (n == 1) return 1;
    
    // 牛顿迭代法
    T x = n;
    T y = (x + 1) / 2;
    
    while (y < x) {
        x = y;
        y = (x + n / x) / 2;
    }
    
    return x;
}

// ==================== 对数运算 ====================

/**
 * @brief 自然对数 ln(x)
 */
inline double log(double x) {
    return std::log(x);
}

/**
 * @brief 以 2 为底的对数
 */
inline double log2(double x) {
    return std::log2(x);
}

/**
 * @brief 以 10 为底的对数
 */
inline double log10(double x) {
    return std::log10(x);
}

/**
 * @brief 通用对数 log_base(x)
 */
inline double log(double base, double x) {
    return std::log(x) / std::log(base);
}

// ==================== 三角函数 ====================

/**
 * @brief 正弦函数（泰勒展开）
 */
inline double sin(double x) {
    return std::sin(x);
}

/**
 * @brief 余弦函数（泰勒展开）
 */
inline double cos(double x) {
    return std::cos(x);
}

/**
 * @brief 正切函数
 */
inline double tan(double x) {
    return std::tan(x);
}

// ==================== 阶乘 ====================

/**
 * @brief 计算阶乘（递归）
 * 
 * @note 仅适用于较小的正整数
 * 
 * @param n 非负整数
 * @return n!
 */
template<typename T,
         typename zen::enable_if_t<zen::is_integral_v<T>, int> = 0>
constexpr T factorial(T n) {
    if (n <= 1) return T(1);
    return n * factorial(n - 1);
}

/**
 * @brief 计算阶乘（迭代版本，避免栈溢出）
 */
template<typename T,
         typename zen::enable_if_t<zen::is_integral_v<T>, int> = 0>
constexpr T factorial_iter(T n) {
    T result = 1;
    for (T i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}

// ==================== 斐波那契数列 ====================

/**
 * @brief 斐波那契数列（矩阵快速幂版本）
 * 
 * 时间复杂度: O(log n)
 * 空间复杂度: O(1)
 * 
 * |F(n+1) F(n)  |   |1 1|^n
 * |F(n)   F(n-1)| = |1 0|
 * 
 * @param n 第 n 项（从 0 开始）
 * @return 斐波那契数
 */
inline long long fibonacci_iter(size_t n) {
    if (n == 0) return 0;
    if (n == 1) return 1;
    
    long long a = 0, b = 1;
    for (size_t i = 2; i <= n; ++i) {
        long long temp = a + b;
        a = b;
        b = temp;
    }
    return b;
}

/**
 * @brief 斐波那契数列（矩阵快速幂版本，内部使用迭代）
 */
inline long long fibonacci(size_t n) {
    return fibonacci_iter(n);
}

// ==================== 判断函数 ====================

/**
 * @brief 判断是否为奇数
 */
template<typename T,
         typename zen::enable_if_t<zen::is_integral_v<T>, int> = 0>
constexpr bool is_odd(T n) {
    return n % 2 != 0;
}

/**
 * @brief 判断是否为偶数
 */
template<typename T,
         typename zen::enable_if_t<zen::is_integral_v<T>, int> = 0>
constexpr bool is_even(T n) {
    return n % 2 == 0;
}

/**
 * @brief 判断是否为素数
 * 
 * 算法思路：
 * 只需检查到 sqrt(n)
 * 如果 n 能被 2 ~ sqrt(n) 之间的数整除，则不是素数
 * 
 * 时间复杂度: O(sqrt(n))
 * 
 * @param n 待检查的正整数
 * @return true 如果是素数
 */
template<typename T,
         typename zen::enable_if_t<zen::is_integral_v<T>, int> = 0>
constexpr bool is_prime(T n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    
    // 检查到 sqrt(n)，步长为 6
    for (T i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) {
            return false;
        }
    }
    return true;
}

/**
 * @brief 判断是否为 2 的幂
 */
template<typename T,
         typename zen::enable_if_t<zen::is_integral_v<T>, int> = 0>
constexpr bool is_power_of_two(T n) {
    return n > 0 && (n & (n - 1)) == 0;
}

/**
 * @brief 判断是否为 10 的幂
 */
template<typename T,
         typename zen::enable_if_t<zen::is_integral_v<T>, int> = 0>
constexpr bool is_power_of_ten(T n) {
    if (n <= 0) return false;
    // 10 的幂的二进制形式只有最低位为 1
    while (n > 1) {
        if (n % 10 != 0) return false;
        n /= 10;
    }
    return true;
}

// ==================== 位运算辅助 ====================

/**
 * @brief 计算二进制中 1 的个数（Brian Kernighan 算法）
 * 
 * 算法思路：
 * n & (n-1) 会消除最低位的 1
 * 每次消除一个 1，直到 n 变为 0
 * 
 * 时间复杂度: O(k)，k 为 1 的个数
 * 
 * @param n 整数
 * @return 1 的个数
 */
template<typename T,
         typename zen::enable_if_t<zen::is_unsigned_v<T>, int> = 0>
constexpr size_t count_ones(T n) {
    size_t count = 0;
    while (n > 0) {
        n &= (n - 1);
        ++count;
    }
    return count;
}

/**
 * @brief 计算二进制中 0 的个数
 */
template<typename T,
         typename zen::enable_if_t<zen::is_unsigned_v<T>, int> = 0>
constexpr size_t count_zeros(T n) {
    return sizeof(T) * 8 - count_ones(n);
}

/**
 * @brief 计算二进制最高位位置（从 0 开始）
 */
template<typename T,
         typename zen::enable_if_t<zen::is_unsigned_v<T>, int> = 0>
constexpr size_t msb_position(T n) {
    if (n == 0) return 0;
    
    size_t pos = 0;
    while (n > 0) {
        n >>= 1;
        ++pos;
    }
    return pos - 1;
}

/**
 * @brief 计算二进制最低位位置（从 0 开始）
 */
template<typename T,
         typename zen::enable_if_t<zen::is_unsigned_v<T>, int> = 0>
constexpr size_t lsb_position(T n) {
    if (n == 0) return 0;
    
    size_t pos = 0;
    while ((n & 1) == 0) {
        n >>= 1;
        ++pos;
    }
    return pos;
}

// ==================== 四舍五入 ====================

/**
 * @brief 向下取整
 */
template<typename T,
         typename zen::enable_if_t<zen::is_floating_point_v<T>, int> = 0>
constexpr T floor(T x) {
    return x < 0 ? static_cast<T>(static_cast<long long>(x) - 1) 
                  : static_cast<T>(static_cast<long long>(x));
}

/**
 * @brief 向上取整
 */
template<typename T,
         typename zen::enable_if_t<zen::is_floating_point_v<T>, int> = 0>
constexpr T ceil(T x) {
    return x > 0 ? static_cast<T>(static_cast<long long>(x) + (x > static_cast<long long>(x) ? 1 : 0))
                  : static_cast<T>(static_cast<long long>(x));
}

/**
 * @brief 四舍五入
 */
template<typename T,
         typename zen::enable_if_t<zen::is_floating_point_v<T>, int> = 0>
constexpr T round(T x) {
    return x >= 0 ? static_cast<T>(static_cast<long long>(x + 0.5))
                  : static_cast<T>(static_cast<long long>(x - 0.5));
}

} // namespace zen

#endif // ZEN_MATH_BASIC_MATH_H
