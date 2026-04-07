/**
 * @file system_utils.h
 * @brief 系统工具函数
 * 
 * 提供：
 * - sleep(): 休眠指定秒数
 * - msleep(): 休眠指定毫秒数
 * - usleep(): 休眠指定微秒数
 * - nanosleep(): 休眠指定纳秒数
 * - get_tick_count(): 获取系统启动后的毫秒数
 * - get_monotonic_time(): 获取单调时间（秒）
 * - get_realtime(): 获取实际时间（秒）
 * 
 * 示例：
 * @code
 * zen::system::sleep(1);  // 休眠 1 秒
 * zen::system::msleep(100);  // 休眠 100 毫秒
 * 
 * uint64_t start = zen::system::get_tick_count();
 * // ... 做一些事情 ...
 * uint64_t elapsed = zen::system::get_tick_count() - start;
 * std::cout << "Elapsed: " << elapsed << " ms" << std::endl;
 * @endcode
 */
#ifndef ZEN_SYSTEM_SYSTEM_UTILS_H
#define ZEN_SYSTEM_SYSTEM_UTILS_H

#include <cstdint>

namespace zen {
namespace system {

/**
 * @brief 休眠指定秒数
 */
void sleep(unsigned int seconds);

/**
 * @brief 休眠指定毫秒数
 */
void msleep(unsigned int milliseconds);

/**
 * @brief 休眠指定微秒数
 */
void usleep(unsigned int microseconds);

/**
 * @brief 休眠指定纳秒数
 */
void nanosleep(uint64_t nanoseconds);

/**
 * @brief 获取系统启动后的毫秒数（单调时钟）
 */
uint64_t get_tick_count();

/**
 * @brief 获取单调时间（秒，从系统启动）
 */
double get_monotonic_time();

/**
 * @brief 获取实际时间（秒，从 Epoch）
 */
double get_realtime();

/**
 * @brief 获取高精度时间戳（纳秒）
 */
uint64_t get_high_resolution_time();

/**
 * @brief CPU 亲和性设置
 */
bool set_cpu_affinity(int cpu_id);

/**
 * @brief 获取当前 CPU ID
 */
int get_current_cpu();

/**
 * @brief 设置线程优先级
 */
bool set_thread_priority(int priority);

/**
 * @brief 获取系统负载
 */
bool get_load_average(double load_avg[3]);

} // namespace system
} // namespace zen

#endif // ZEN_SYSTEM_SYSTEM_UTILS_H
