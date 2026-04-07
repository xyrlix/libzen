/**
 * @file system_info.h
 * @brief 系统信息获取
 * 
 * 提供：
 * - cpu_count(): CPU 核心数
 * - total_memory(): 总内存大小
 * - available_memory(): 可用内存大小
 * - hostname(): 主机名
 * - username(): 用户名
 * 
 * 示例：
 * @code
 * std::cout << "CPU count: " << zen::system::cpu_count() << std::endl;
 * std::cout << "Total memory: " << zen::system::total_memory() << std::endl;
 * std::cout << "Hostname: " << zen::system::hostname() << std::endl;
 * @endcode
 */
#ifndef ZEN_SYSTEM_SYSTEM_INFO_H
#define ZEN_SYSTEM_SYSTEM_INFO_H

#include <string>
#include <cstdint>

namespace zen {
namespace system {

/**
 * @brief 获取 CPU 核心数
 */
unsigned int cpu_count();

/**
 * @brief 获取总内存大小（字节）
 */
uint64_t total_memory();

/**
 * @brief 获取可用内存大小（字节）
 */
uint64_t available_memory();

/**
 * @brief 获取主机名
 */
std::string hostname();

/**
 * @brief 获取用户名
 */
std::string username();

/**
 * @brief 获取当前进程 ID
 */
int pid();

/**
 * @brief 获取父进程 ID
 */
int ppid();

/**
 * @brief 获取当前线程 ID
 */
uint64_t tid();

/**
 * @brief 获取进程可执行文件路径
 */
std::string executable_path();

/**
 * @brief 获取当前工作目录
 */
std::string current_working_directory();

/**
 * @brief 获取系统启动时间（时间戳）
 */
uint64_t boot_time();

} // namespace system
} // namespace zen

#endif // ZEN_SYSTEM_SYSTEM_INFO_H
