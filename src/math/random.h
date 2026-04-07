/**
 * @file random.h
 * @brief 随机数生成器实现
 * 
 * 手撕算法实现，不使用系统库
 * 
 * 算法说明：
 * 1. LCG (线性同余生成器) - 简单快速，周期较短
 * 2. Xorshift - 快速，无乘法，周期较长
 * 3. MT19937 (简化版) -高质量随机数，周期 2^19937-1
 */

#ifndef ZEN_MATH_RANDOM_H
#define ZEN_MATH_RANDOM_H

#include <cstdint>
#include <cstddef>
#include <climits>
#include <cmath>

namespace zen {

// ==================== 基础随机数引擎 ====================

/**
 * @brief 线性同余生成器 (Linear Congruential Generator)
 * 
 * 算法：X(n+1) = (a * X(n) + c) mod m
 * 
 * 参数选择：
 * - a = 1103515245 (乘数)
 * - c = 12345 (增量)
 * - m = 2^31 (模数)
 * 
 * 周期：约 2^31
 */
class lcg_engine {
private:
    uint32_t state_;  // 当前状态
    
    // LCG 参数
    static constexpr uint32_t a = 1103515245u;
    static constexpr uint32_t c = 12345u;
    static constexpr uint32_t m = 2147483647u;  // 2^31 - 1
    
public:
    using result_type = uint32_t;
    
    /**
     * @brief 默认构造函数，使用固定种子
     */
    lcg_engine() : state_(1) {}
    
    /**
     * @brief 构造函数
     * @param seed 种子值
     */
    explicit lcg_engine(uint32_t seed) : state_(seed) {}
    
    /**
     * @brief 生成下一个随机数
     * @return 随机整数 [0, 2^31)
     */
    result_type operator()() {
        state_ = (a * state_ + c) % m;
        return state_;
    }
    
    /**
     * @brief 跳到指定状态
     * @param s 新状态
     */
    void seed(uint32_t s) {
        state_ = (s == 0) ? 1 : s;
    }
    
    static constexpr result_type min() { return 0; }
    static constexpr result_type max() { return m - 1; }
};

/**
 * @brief Xorshift128+ 算法
 * 
 * 算法：使用三个位移和异或操作
 * 周期：2^128 - 1
 */
class xorshift_engine {
private:
    uint64_t state_[2];
    
    // 旋转常数
    static constexpr uint64_t ROTL(uint64_t x, int k) {
        return (x << k) | (x >> (64 - k));
    }
    
public:
    using result_type = uint64_t;
    
    xorshift_engine() : state_{123456789, 987654321} {}
    
    explicit xorshift_engine(uint64_t seed) {
        // 使用种子初始化两个状态
        state_[0] = seed;
        state_[1] = seed ^ 0x9e3779b97f4a7c15ULL;
    }
    
    result_type operator()() {
        uint64_t s1 = state_[0];
        const uint64_t s0 = state_[1];
        state_[0] = s0;
        s1 ^= s1 << 23;
        state_[1] = s1 ^ s0 ^ (s1 >> 18) ^ (s0 >> 5);
        return state_[1] + s0;
    }
    
    void seed(uint64_t s) {
        state_[0] = s;
        state_[1] = s ^ 0x9e3779b97f4a7c15ULL;
    }
    
    static constexpr result_type min() { return 0; }
    static constexpr result_type max() { return UINT64_MAX; }
};

// ==================== 默认引擎 ====================

/**
 * @brief 默认随机数引擎
 */
using default_random_engine = xorshift_engine;

// ==================== 均匀分布 ====================

/**
 * @brief 均匀分布 [min, max]
 * 
 * 算法思路：
 * 1. 从引擎获取原始随机数
 * 2. 映射到 [0, 1) 区间
 * 3. 缩放到 [min, max]
 */
template<typename T = double>
class uniform_real_distribution {
private:
    T min_;
    T max_;
    T range_;
    
public:
    using result_type = T;
    
    struct param_type {
        T min_val;
        T max_val;
    };
    
    uniform_real_distribution() : min_(0.0), max_(1.0), range_(1.0) {}
    
    uniform_real_distribution(T min, T max) : min_(min), max_(max), range_(max - min) {}
    
    uniform_real_distribution(const param_type& p) 
        : min_(p.min_val), max_(p.max_val), range_(p.max_val - p.min_val) {}
    
    template<typename Engine>
    result_type operator()(Engine& eng) {
        // 生成 [0, 1) 区间的浮点数
        result_type u = static_cast<result_type>(eng()) / static_cast<result_type>(Engine::max());
        return min_ + u * range_;
    }
    
    template<typename Engine>
    result_type operator()(Engine& eng, const param_type& p) {
        return uniform_real_distribution(p)(eng);
    }
    
    result_type min() const { return min_; }
    result_type max() const { return max_; }
    
    void reset() {}
};

/**
 * @brief 整数均匀分布 [min, max]
 */
template<typename T = int>
class uniform_int_distribution {
private:
    T min_;
    T max_;
    
public:
    using result_type = T;
    
    struct param_type {
        T min_val;
        T max_val;
    };
    
    uniform_int_distribution() : min_(0), max_(INT_MAX) {}
    
    uniform_int_distribution(T min, T max) : min_(min), max_(max) {}
    
    uniform_int_distribution(const param_type& p) : min_(p.min_val), max_(p.max_val) {}
    
    template<typename Engine>
    result_type operator()(Engine& eng) {
        // 简化实现：直接取模
        using engine_result = typename Engine::result_type;
        
        // 计算范围
        uint64_t range = static_cast<uint64_t>(max_) - static_cast<uint64_t>(min_) + 1;
        
        // 获取随机数并映射到范围内
        engine_result r = eng();
        uint64_t result = r % range;
        
        return static_cast<result_type>(result + min_);
    }
    
    result_type min() const { return min_; }
    result_type max() const { return max_; }
    
    void reset() {}
};

// ==================== 正态分布 ====================

/**
 * @brief 正态分布（高斯分布）
 * 
 * 算法：Box-Muller 变换
 * 
 * 算法思路：
 * 1. 生成两个独立的 [0, 1) 均匀分布 u1, u2
 * 2. 计算 z0 = sqrt(-2 * ln(u1)) * cos(2 * pi * u2)
 * 3. z0 服从标准正态分布 N(0, 1)
 * 4. 转换到 N(mean, stddev^2): x = mean + z0 * stddev
 */
class normal_distribution {
private:
    double mean_;
    double stddev_;
    double next_normal_;  // Box-Muller 产生两个数，缓存一个
    bool has_next_;
    bool use_next_;
    
public:
    using result_type = double;
    
    struct param_type {
        double mean;
        double stddev;
    };
    
    normal_distribution() : mean_(0.0), stddev_(1.0), has_next_(false), use_next_(false) {}
    
    normal_distribution(double mean, double stddev) 
        : mean_(mean), stddev_(stddev), has_next_(false), use_next_(false) {}
    
    normal_distribution(const param_type& p) 
        : mean_(p.mean), stddev_(p.stddev), has_next_(false), use_next_(false) {}
    
    template<typename Engine>
    result_type operator()(Engine& eng) {
        // Box-Muller 变换
        if (use_next_ && has_next_) {
            use_next_ = false;
            return next_normal_;
        }
        
        // 生成两个均匀分布
        double u1 = static_cast<double>(eng()) / static_cast<double>(Engine::max());
        double u2 = static_cast<double>(eng()) / static_cast<double>(Engine::max());
        
        // 避免 u1 为 0
        while (u1 == 0.0) {
            u1 = static_cast<double>(eng()) / static_cast<double>(Engine::max());
        }
        
        // Box-Muller 变换
        double z0 = std::sqrt(-2.0 * std::log(u1)) * std::cos(2.0 * 3.14159265358979323846 * u2);
        
        // 缓存第二个值
        double z1 = std::sqrt(-2.0 * std::log(u1)) * std::sin(2.0 * 3.14159265358979323846 * u2);
        next_normal_ = mean_ + z1 * stddev_;
        has_next_ = true;
        
        return mean_ + z0 * stddev_;
    }
    
    double mean() const { return mean_; }
    double stddev() const { return stddev_; }
    double variance() const { return stddev_ * stddev_; }
    
    void reset() { has_next_ = false; }
};

// ==================== 便捷函数 ====================

/**
 * @brief 生成随机整数 [min, max]
 */
template<typename T = int>
T random_int(T min, T max) {
    static default_random_engine eng;
    static uniform_int_distribution<T> dist(min, max);
    return dist(eng);
}

/**
 * @brief 生成随机浮点数 [min, max)
 */
template<typename T = double>
T random_real(T min, T max) {
    static default_random_engine eng;
    static uniform_real_distribution<T> dist(min, max);
    return dist(eng);
}

/**
 * @brief 生成标准正态分布随机数
 */
double random_normal() {
    static default_random_engine eng;
    static normal_distribution dist;
    return dist(eng);
}

/**
 * @brief 生成随机布尔值
 */
bool random_bool(double probability = 0.5) {
    return random_real(0.0, 1.0) < probability;
}

/**
 * @brief 随机打乱数组（Fisher-Yates 洗牌算法）
 * 
 * 算法思路：
 * 从最后一个元素开始，逐个与前面随机位置的元素交换
 * 
 * 时间复杂度: O(n)
 */
template<typename T>
void shuffle(T* arr, size_t n) {
    if (n <= 1) return;
    
    default_random_engine eng;
    uniform_int_distribution<size_t> dist(0, n - 1);
    
    for (size_t i = n - 1; i > 0; --i) {
        size_t j = dist(eng) % (i + 1);  // 随机选择 [0, i]
        // 交换 arr[i] 和 arr[j]
        T temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

} // namespace zen

#endif // ZEN_MATH_RANDOM_H
