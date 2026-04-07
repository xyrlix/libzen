/**
 * @file numeric_stats.h
 * @brief 数值统计实现
 * 
 * 手撕算法实现，不使用系统库
 */

#ifndef ZEN_MATH_NUMERIC_STATS_H
#define ZEN_MATH_NUMERIC_STATS_H

#include "basic_math.h"
#include <cstddef>

namespace zen {

// ==================== 统计工具函数 ====================

/**
 * @brief 计算数组元素的和
 * 
 * @tparam T 元素类型
 * @tparam N 数组大小
 * @param arr 输入数组
 * @return 所有元素的和
 */
template<typename T, size_t N>
T sum(const T (&arr)[N]) {
    T result = 0;
    for (size_t i = 0; i < N; ++i) {
        result += arr[i];
    }
    return result;
}

/**
 * @brief 计算容器元素的和
 */
template<typename Container>
auto sum(const Container& c) -> typename Container::value_type {
    using T = typename Container::value_type;
    T result = 0;
    for (const auto& elem : c) {
        result += elem;
    }
    return result;
}

/**
 * @brief 计算均值（算术平均）
 * 
 * 公式：mean = (x1 + x2 + ... + xn) / n
 * 
 * @tparam T 元素类型
 * @tparam N 数组大小
 * @param arr 输入数组
 * @return 均值
 */
template<typename T, size_t N>
double mean(const T (&arr)[N]) {
    if (N == 0) return 0.0;
    return static_cast<double>(sum(arr)) / N;
}

template<typename Container>
double mean(const Container& c) {
    if (c.empty()) return 0.0;
    return static_cast<double>(sum(c)) / c.size();
}

/**
 * @brief 计算几何平均
 * 
 * 公式：geometric_mean = (x1 * x2 * ... * xn)^(1/n)
 * 
 * @note 所有元素必须为正数
 */
template<typename T, size_t N>
double geometric_mean(const T (&arr)[N]) {
    if (N == 0) return 0.0;
    
    double product = 1.0;
    for (size_t i = 0; i < N; ++i) {
        if (arr[i] <= 0) return 0.0;
        product *= arr[i];
    }
    return pow(product, 1.0 / N);
}

/**
 * @brief 计算调和平均
 * 
 * 公式：harmonic_mean = n / (1/x1 + 1/x2 + ... + 1/xn)
 * 
 * @note 所有元素必须非零
 */
template<typename T, size_t N>
double harmonic_mean(const T (&arr)[N]) {
    if (N == 0) return 0.0;
    
    double sum_reciprocal = 0.0;
    for (size_t i = 0; i < N; ++i) {
        if (arr[i] == 0) return 0.0;
        sum_reciprocal += 1.0 / arr[i];
    }
    
    if (sum_reciprocal == 0) return 0.0;
    return N / sum_reciprocal;
}

// ==================== 方差与标准差 ====================

/**
 * @brief 计算方差（样本方差）
 * 
 * 公式：var = sum((xi - mean)^2) / (n - 1)
 * 
 * 使用 Welfold 算法避免数值不稳定：
 * var = var + (xi - mean_n) * (xi - mean_n+1)
 * 其中 mean_n 是前 n 个数的均值
 */
template<typename T, size_t N>
double variance(const T (&arr)[N]) {
    if (N <= 1) return 0.0;
    
    double mean_val = mean(arr);
    double sum_sq_diff = 0.0;
    
    for (size_t i = 0; i < N; ++i) {
        double diff = static_cast<double>(arr[i]) - mean_val;
        sum_sq_diff += diff * diff;
    }
    
    return sum_sq_diff / (N - 1);  // 样本方差
}

/**
 * @brief 计算总体方差
 * 
 * 公式：var = sum((xi - mean)^2) / n
 */
template<typename T, size_t N>
double population_variance(const T (&arr)[N]) {
    if (N == 0) return 0.0;
    
    double mean_val = mean(arr);
    double sum_sq_diff = 0.0;
    
    for (size_t i = 0; i < N; ++i) {
        double diff = static_cast<double>(arr[i]) - mean_val;
        sum_sq_diff += diff * diff;
    }
    
    return sum_sq_diff / N;
}

/**
 * @brief 计算标准差
 */
template<typename T, size_t N>
double stddev(const T (&arr)[N]) {
    return sqrt(variance(arr));
}

/**
 * @brief 计算总体标准差
 */
template<typename T, size_t N>
double population_stddev(const T (&arr)[N]) {
    return sqrt(population_variance(arr));
}

/**
 * @brief 计算变异系数（CV）
 * 
 * 公式：CV = (stddev / mean) * 100%
 */
template<typename T, size_t N>
double coefficient_of_variation(const T (&arr)[N]) {
    double mean_val = mean(arr);
    if (mean_val == 0) return 0.0;
    return (stddev(arr) / mean_val) * 100.0;
}

// ==================== 极值 ====================

/**
 * @brief 找到最小值
 */
template<typename T, size_t N>
T min_value(const T (&arr)[N]) {
    if (N == 0) return T();
    
    T min_val = arr[0];
    for (size_t i = 1; i < N; ++i) {
        if (arr[i] < min_val) min_val = arr[i];
    }
    return min_val;
}

/**
 * @brief 找到最大值
 */
template<typename T, size_t N>
T max_value(const T (&arr)[N]) {
    if (N == 0) return T();
    
    T max_val = arr[0];
    for (size_t i = 1; i < N; ++i) {
        if (arr[i] > max_val) max_val = arr[i];
    }
    return max_val;
}

/**
 * @brief 同时找到最小值和最大值
 */
template<typename T, size_t N>
void minmax_value(const T (&arr)[N], T& min_val, T& max_val) {
    if (N == 0) return;
    
    min_val = max_val = arr[0];
    for (size_t i = 1; i < N; ++i) {
        if (arr[i] < min_val) min_val = arr[i];
        if (arr[i] > max_val) max_val = arr[i];
    }
}

/**
 * @brief 计算极差（最大值 - 最小值）
 */
template<typename T, size_t N>
T range(const T (&arr)[N]) {
    return max_value(arr) - min_value(arr);
}

/**
 * @brief 计算四分位距 (IQR)
 * 
 * IQR = Q3 - Q1
 */
template<typename T, size_t N>
double interquartile_range(const T (&arr)[N]) {
    if (N < 4) return 0.0;
    
    // 复制数组并排序
    T sorted[N];
    for (size_t i = 0; i < N; ++i) sorted[i] = arr[i];
    
    // 简单排序
    for (size_t i = 0; i < N - 1; ++i) {
        for (size_t j = i + 1; j < N; ++j) {
            if (sorted[j] < sorted[i]) {
                T temp = sorted[i];
                sorted[i] = sorted[j];
                sorted[j] = temp;
            }
        }
    }
    
    // 计算 Q1 和 Q3
    double q1, q3;
    size_t mid = N / 2;
    
    if (N % 2 == 0) {
        q1 = static_cast<double>(sorted[mid / 2 - 1] + sorted[mid / 2]) / 2;
        q3 = static_cast<double>(sorted[mid + mid / 2 - 1] + sorted[mid + mid / 2]) / 2;
    } else {
        q1 = static_cast<double>(sorted[mid / 2]);
        q3 = static_cast<double>(sorted[mid + mid / 2 + 1]);
    }
    
    return q3 - q1;
}

// ==================== 中位数 ====================

/**
 * @brief 计算中位数
 * 
 * 算法思路：
 * 1. 复制数组
 * 2. 使用选择算法找中位数（O(n)），或者简单排序（O(n log n)）
 */
template<typename T, size_t N>
double median(const T (&arr)[N]) {
    if (N == 0) return 0.0;
    if (N == 1) return static_cast<double>(arr[0]);
    
    // 复制并排序
    T sorted[N];
    for (size_t i = 0; i < N; ++i) sorted[i] = arr[i];
    
    // 冒泡排序
    for (size_t i = 0; i < N - 1; ++i) {
        for (size_t j = 0; j < N - i - 1; ++j) {
            if (sorted[j] > sorted[j + 1]) {
                T temp = sorted[j];
                sorted[j] = sorted[j + 1];
                sorted[j + 1] = temp;
            }
        }
    }
    
    // 中位数
    if (N % 2 == 0) {
        return static_cast<double>(sorted[N / 2 - 1] + sorted[N / 2]) / 2;
    } else {
        return static_cast<double>(sorted[N / 2]);
    }
}

// ==================== 百分位数 ====================

/**
 * @brief 计算百分位数
 * 
 * @param p 百分位 [0, 100]
 * 
 * 使用线性插值：
 * rank = p/100 * (n - 1)
 * 如果 rank 是整数，直接返回
 * 否则，使用相邻值的线性插值
 */
template<typename T, size_t N>
double percentile(const T (&arr)[N], double p) {
    if (N == 0 || p < 0 || p > 100) return 0.0;
    
    // 复制并排序
    T sorted[N];
    for (size_t i = 0; i < N; ++i) sorted[i] = arr[i];
    
    // 简单排序
    for (size_t i = 0; i < N - 1; ++i) {
        for (size_t j = i + 1; j < N; ++j) {
            if (sorted[j] < sorted[i]) {
                T temp = sorted[i];
                sorted[i] = sorted[j];
                sorted[j] = temp;
            }
        }
    }
    
    double rank = p / 100.0 * (N - 1);
    size_t lower = static_cast<size_t>(rank);
    size_t upper = lower + 1;
    
    if (upper >= N) return static_cast<double>(sorted[N - 1]);
    if (lower == upper) return static_cast<double>(sorted[lower]);
    
    // 线性插值
    double fraction = rank - lower;
    return static_cast<double>(sorted[lower]) * (1 - fraction) + 
           static_cast<double>(sorted[upper]) * fraction;
}

// ==================== 偏度与峰度 ====================

/**
 * @brief 计算偏度（Skewness）
 * 
 * 衡量分布的不对称程度
 * - 正偏度：右偏（长尾在右侧）
 * - 负偏度：左偏（长尾在左侧）
 */
template<typename T, size_t N>
double skewness(const T (&arr)[N]) {
    if (N < 3) return 0.0;
    
    double mean_val = mean(arr);
    double stddev_val = stddev(arr);
    
    if (stddev_val == 0) return 0.0;
    
    double sum_cubed_diff = 0.0;
    for (size_t i = 0; i < N; ++i) {
        double diff = static_cast<double>(arr[i]) - mean_val;
        sum_cubed_diff += diff * diff * diff;
    }
    
    double n = static_cast<double>(N);
    double term = n / ((n - 1) * (n - 2));
    double stddev_cubed = stddev_val * stddev_val * stddev_val;
    
    return term * sum_cubed_diff / stddev_cubed;
}

/**
 * @brief 计算峰度（Kurtosis）
 * 
 * 衡量分布的尖峰程度
 * - 正峰度：尖峰（比正态分布更集中）
 * - 负峰度：平峰（比正态分布更分散）
 */
template<typename T, size_t N>
double kurtosis(const T (&arr)[N]) {
    if (N < 4) return 0.0;
    
    double mean_val = mean(arr);
    double stddev_val = stddev(arr);
    
    if (stddev_val == 0) return 0.0;
    
    double sum_quartic_diff = 0.0;
    for (size_t i = 0; i < N; ++i) {
        double diff = static_cast<double>(arr[i]) - mean_val;
        sum_quartic_diff += diff * diff * diff * diff;
    }
    
    double n = static_cast<double>(N);
    double term1 = n * (n + 1) / ((n - 1) * (n - 2) * (n - 3));
    double term2 = 3 * (n - 1) * (n - 1) / ((n - 2) * (n - 3));
    double stddev_quartic = stddev_val * stddev_val * stddev_val * stddev_val;
    
    return term1 * sum_quartic_diff / stddev_quartic - term2;
}

// ==================== 相关系数 ====================

/**
 * @brief 计算皮尔逊相关系数
 * 
 * 公式：r = sum((xi - x_mean)(yi - y_mean)) / 
 *       sqrt(sum((xi - x_mean)^2) * sum((yi - y_mean)^2))
 * 
 * 范围：[-1, 1]
 * - 1: 完全正相关
 * - -1: 完全负相关
 * - 0: 不相关
 */
template<typename T, size_t N>
double correlation(const T (&x)[N], const T (&y)[N]) {
    if (N == 0) return 0.0;
    
    double x_mean = mean(x);
    double y_mean = mean(y);
    
    double sum_xy_diff = 0.0;
    double sum_x_sq_diff = 0.0;
    double sum_y_sq_diff = 0.0;
    
    for (size_t i = 0; i < N; ++i) {
        double x_diff = static_cast<double>(x[i]) - x_mean;
        double y_diff = static_cast<double>(y[i]) - y_mean;
        
        sum_xy_diff += x_diff * y_diff;
        sum_x_sq_diff += x_diff * x_diff;
        sum_y_sq_diff += y_diff * y_diff;
    }
    
    double denominator = sqrt(sum_x_sq_diff * sum_y_sq_diff);
    if (denominator == 0) return 0.0;
    
    return sum_xy_diff / denominator;
}

/**
 * @brief 计算协方差
 */
template<typename T, size_t N>
double covariance(const T (&x)[N], const T (&y)[N]) {
    if (N == 0) return 0.0;
    
    double x_mean = mean(x);
    double y_mean = mean(y);
    
    double sum_xy_diff = 0.0;
    for (size_t i = 0; i < N; ++i) {
        sum_xy_diff += (static_cast<double>(x[i]) - x_mean) * 
                       (static_cast<double>(y[i]) - y_mean);
    }
    
    return sum_xy_diff / (N - 1);
}

} // namespace zen

#endif // ZEN_MATH_NUMERIC_STATS_H
