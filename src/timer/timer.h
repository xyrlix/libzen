/**
 * @file timer.h
 * @brief 定时器基础类型与接口定义
 *
 * 提供：
 *   - timer_id：定时器唯一标识符（uint64_t）
 *   - timer_callback：回调函数类型（void(*)(timer_id, void*)）
 *   - timer_entry：单个定时器的内部表示
 *
 * 与 timer_manager 配合使用：
 * @code
 * zen::timer_manager tm;
 * zen::timer_id id = tm.add_once(500, [](zen::timer_id, void*){
 *     printf("500ms later\n");
 * });
 * // 在事件循环或主线程中定期调用：
 * tm.tick();
 * @endcode
 */
#pragma once
#include <cstdint>
#include <cstddef>

namespace zen {

// ============================================================================
// 基础类型
// ============================================================================

/** 定时器唯一 ID（0 表示无效） */
using timer_id = uint64_t;
static const timer_id INVALID_TIMER_ID = 0;

/**
 * @brief 定时器回调：void callback(timer_id id, void* user_data)
 */
using timer_callback = void(*)(timer_id, void*);

/** 定时器类型 */
enum class timer_type : uint8_t {
    ONCE     = 0,  // 一次性定时器（触发后自动移除）
    REPEAT   = 1,  // 重复定时器（每隔 interval_ms 触发一次）
};

// ============================================================================
// timer_entry：单个定时器的完整状态
// ============================================================================
struct timer_entry {
    timer_id       id;            // 唯一 ID
    timer_type     type;          // 一次性 / 重复
    unsigned long long expire_ms; // 下次触发的绝对时间（单调毫秒）
    unsigned long long interval_ms; // REPEAT 模式下的间隔（ONCE 时忽略）
    timer_callback callback;      // 回调函数
    void*          user_data;     // 用户数据指针
    bool           cancelled;     // 是否已取消

    timer_entry() noexcept
        : id(INVALID_TIMER_ID), type(timer_type::ONCE),
          expire_ms(0), interval_ms(0),
          callback(nullptr), user_data(nullptr), cancelled(false) {}
};

} // namespace zen
