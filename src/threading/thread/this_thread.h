/**
 * @file this_thread.h
 * @brief 当前线程操作
 * 
 * 提供操作当前线程的工具函数：
 * - get_id()     : 返回当前线程的 thread_id
 * - sleep_for()  : 让当前线程睡眠指定时间（毫秒）
 * - sleep_until(): 让当前线程睡眠到指定时刻
 * - yield()      : 让出时间片，提示调度器切换到其他线程
 * 
 * 与 std::this_thread 命名空间功能对应。
 */
#ifndef ZEN_THREADING_THREAD_THIS_THREAD_H
#define ZEN_THREADING_THREAD_THIS_THREAD_H

#include "thread.h"

#if defined(_WIN32) || defined(_WIN64)
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#else
#  include <pthread.h>
#  include <time.h>
#  include <sched.h>
#  include <unistd.h>
#endif

namespace zen {
namespace this_thread {

/**
 * @brief 返回当前线程的 ID
 */
inline thread_id get_id() noexcept {
#ifdef ZEN_OS_WINDOWS
    return thread_id(GetCurrentThreadId());
#else
    return thread_id(pthread_self());
#endif
}

/**
 * @brief 当前线程睡眠指定毫秒数
 * @param milliseconds 睡眠时间（毫秒）
 */
inline void sleep_for(unsigned long milliseconds) noexcept {
#ifdef ZEN_OS_WINDOWS
    Sleep(static_cast<DWORD>(milliseconds));
#else
    struct timespec ts;
    ts.tv_sec  = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000L;
    nanosleep(&ts, nullptr);
#endif
}

/**
 * @brief 当前线程睡眠指定微秒数
 * @param microseconds 睡眠时间（微秒）
 */
inline void sleep_for_us(unsigned long microseconds) noexcept {
#ifdef ZEN_OS_WINDOWS
    Sleep(static_cast<DWORD>(microseconds / 1000 + (microseconds % 1000 ? 1 : 0)));
#else
    struct timespec ts;
    ts.tv_sec  = microseconds / 1000000;
    ts.tv_nsec = (microseconds % 1000000) * 1000L;
    nanosleep(&ts, nullptr);
#endif
}

/**
 * @brief 让出当前线程时间片
 * 
 * 提示操作系统调度器将 CPU 分配给其他线程。
 * 常用于自旋等待中减少 CPU 空转：
 * @code
 * while (!ready) {
 *     zen::this_thread::yield();
 * }
 * @endcode
 */
inline void yield() noexcept {
#ifdef ZEN_OS_WINDOWS
    SwitchToThread();
#else
    sched_yield();
#endif
}

/**
 * @brief 获取当前时间戳（毫秒，单调时钟）
 * @return 毫秒时间戳
 */
inline unsigned long long monotonic_ms() noexcept {
#ifdef ZEN_OS_WINDOWS
    return static_cast<unsigned long long>(GetTickCount64());
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<unsigned long long>(ts.tv_sec) * 1000ULL
           + static_cast<unsigned long long>(ts.tv_nsec) / 1000000ULL;
#endif
}

/**
 * @brief 获取当前时间戳（微秒，单调时钟）
 * @return 微秒时间戳
 */
inline unsigned long long monotonic_us() noexcept {
#ifdef ZEN_OS_WINDOWS
    LARGE_INTEGER freq, count;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&count);
    return static_cast<unsigned long long>(count.QuadPart) * 1000000ULL
           / static_cast<unsigned long long>(freq.QuadPart);
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<unsigned long long>(ts.tv_sec) * 1000000ULL
           + static_cast<unsigned long long>(ts.tv_nsec) / 1000ULL;
#endif
}

} // namespace this_thread
} // namespace zen

#endif // ZEN_THREADING_THREAD_THIS_THREAD_H
