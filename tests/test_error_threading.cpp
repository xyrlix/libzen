/**
 * @file test_error_threading.cpp
 * @brief error 模块 + threading 模块测试
 */

// ============================================================================
// 极简测试框架
// ============================================================================

#include <cstdio>
#include <cstring>

static int g_total = 0;
static int g_passed = 0;
static bool g_current_failed = false;

#define TEST(name) static void name##_impl(); \
    static void name() { \
        g_current_failed = false; \
        name##_impl(); \
        ++g_total; \
        if (!g_current_failed) ++g_passed; \
    } \
    struct name##_reg { name##_reg() { name(); } } name##_instance; \
    static void name##_impl()

#define ASSERT_TRUE(expr) do { \
    if (!(expr)) { \
        std::printf("  FAIL: %s (line %d)\n", #expr, __LINE__); \
        g_current_failed = true; \
        return; \
    } \
} while(0)

#define ASSERT_EQ(a, b) ASSERT_TRUE((a) == (b))
#define ASSERT_NE(a, b) ASSERT_TRUE((a) != (b))
#define ASSERT_FALSE(expr) ASSERT_TRUE(!(expr))

// ============================================================================
// 包含模块头文件
// ============================================================================

#include "src/error/system_error.h"
#include "src/error/error_context.h"
#include "src/error/exception_chain.h"
#include "src/threading/sync/spinlock.h"
#include "src/threading/sync/mutex.h"
#include "src/threading/sync/lock_guard.h"
#include "src/threading/sync/rwlock.h"
#include "src/threading/sync/condition_variable.h"
#include "src/threading/thread/thread.h"
#include "src/threading/thread/this_thread.h"

// ============================================================================
// error_code 测试
// ============================================================================

TEST(test_error_code_default) {
    zen::error_code ec;
    ASSERT_FALSE(static_cast<bool>(ec));      // 无错误
    ASSERT_EQ(ec.value(), 0);
    ASSERT_EQ(std::strcmp(ec.message(), "Success"), 0);
}

TEST(test_error_code_from_errc) {
    auto ec = zen::make_error_code(zen::errc::not_found);
    ASSERT_TRUE(static_cast<bool>(ec));
    ASSERT_EQ(ec.value(), static_cast<int>(zen::errc::not_found));
    ASSERT_NE(std::strcmp(ec.message(), "Success"), 0);
}

TEST(test_error_code_equality) {
    auto e1 = zen::make_error_code(zen::errc::timeout);
    auto e2 = zen::make_error_code(zen::errc::timeout);
    auto e3 = zen::make_error_code(zen::errc::not_found);
    ASSERT_EQ(e1, e2);
    ASSERT_NE(e1, e3);
}

TEST(test_error_code_clear) {
    auto ec = zen::make_error_code(zen::errc::invalid_argument);
    ASSERT_TRUE(static_cast<bool>(ec));
    ec.clear();
    ASSERT_FALSE(static_cast<bool>(ec));
}

TEST(test_errc_messages) {
    // 检查所有常用错误码都有消息
    auto check = [](zen::errc e) -> bool {
        auto ec = zen::make_error_code(e);
        return ec.message() != nullptr && ec.message()[0] != '\0';
    };
    ASSERT_TRUE(check(zen::errc::invalid_argument));
    ASSERT_TRUE(check(zen::errc::out_of_range));
    ASSERT_TRUE(check(zen::errc::permission_denied));
    ASSERT_TRUE(check(zen::errc::out_of_memory));
    ASSERT_TRUE(check(zen::errc::io_error));
    ASSERT_TRUE(check(zen::errc::deadlock));
}

// ============================================================================
// error_context 测试
// ============================================================================

TEST(test_error_context_ok) {
    zen::error_context ec;
    ASSERT_FALSE(static_cast<bool>(ec));
    ASSERT_TRUE(ec.ok());
}

TEST(test_error_context_with_message) {
    zen::error_context ec(zen::errc::not_found, "key missing");
    ASSERT_TRUE(static_cast<bool>(ec));
    ASSERT_EQ(std::strcmp(ec.message(), "key missing"), 0);
}

TEST(test_error_context_with_location) {
    zen::error_context ec(zen::errc::io_error, "file read failed",
                          "test.cpp", 42, "read_file");
    ASSERT_TRUE(static_cast<bool>(ec));
    ASSERT_EQ(ec.line(), 42);
    ASSERT_EQ(std::strcmp(ec.file(), "test.cpp"), 0);
    ASSERT_EQ(std::strcmp(ec.function(), "read_file"), 0);
}

TEST(test_zen_make_error_macro) {
    auto ec = ZEN_MAKE_ERROR(zen::errc::timeout, "network timeout");
    ASSERT_TRUE(static_cast<bool>(ec));
    ASSERT_EQ(std::strcmp(ec.message(), "network timeout"), 0);
    // 行号应该是非零
    ASSERT_NE(ec.line(), 0);
}

TEST(test_error_context_format) {
    zen::error_context ec(zen::errc::not_found, "not found",
                          "file.cpp", 10, "func");
    char buf[512];
    int len = ec.format(buf, sizeof(buf));
    ASSERT_TRUE(len > 0);
    // 检查 buf 包含关键信息
    bool has_error = std::strstr(buf, "ERROR") != nullptr ||
                     std::strstr(buf, "error") != nullptr;
    ASSERT_TRUE(has_error);
}

// ============================================================================
// error_chain 测试
// ============================================================================

TEST(test_error_chain_empty) {
    zen::error_chain chain;
    ASSERT_TRUE(chain.empty());
    ASSERT_EQ(chain.size(), 0u);
    ASSERT_FALSE(static_cast<bool>(chain));
}

TEST(test_error_chain_push_pop) {
    zen::error_chain chain;
    chain.push(zen::error_context(zen::errc::io_error, "read failed"));
    chain.push(zen::error_context(zen::errc::not_found, "file missing"));
    
    ASSERT_EQ(chain.size(), 2u);
    ASSERT_TRUE(static_cast<bool>(chain));
    
    // 顶层是最后 push 的
    ASSERT_EQ(chain.top().code().value(), static_cast<int>(zen::errc::not_found));
    
    // 根因是最先 push 的
    ASSERT_EQ(chain.root_cause().code().value(), static_cast<int>(zen::errc::io_error));
    
    chain.pop();
    ASSERT_EQ(chain.size(), 1u);
}

TEST(test_error_chain_clear) {
    zen::error_chain chain;
    chain.push(zen::error_context(zen::errc::timeout, "timeout"));
    chain.clear();
    ASSERT_TRUE(chain.empty());
}

// ============================================================================
// result<T> 测试
// ============================================================================

TEST(test_result_ok) {
    auto r = zen::result<int>::ok(42);
    ASSERT_TRUE(r.is_ok());
    ASSERT_FALSE(r.is_err());
    ASSERT_EQ(*r, 42);
    ASSERT_EQ(r.value(), 42);
}

TEST(test_result_err) {
    auto r = zen::result<int>::err(zen::errc::not_found, "not found");
    ASSERT_FALSE(r.is_ok());
    ASSERT_TRUE(r.is_err());
    ASSERT_TRUE(static_cast<bool>(r.error()));
}

TEST(test_result_value_or) {
    auto ok  = zen::result<int>::ok(10);
    auto err = zen::result<int>::err(zen::errc::unknown);
    ASSERT_EQ(ok.value_or(0), 10);
    ASSERT_EQ(err.value_or(99), 99);
}

TEST(test_result_void_ok) {
    auto r = zen::result<void>::ok();
    ASSERT_TRUE(r.is_ok());
}

TEST(test_result_void_err) {
    auto r = zen::result<void>::err(zen::errc::permission_denied);
    ASSERT_TRUE(r.is_err());
}

// ============================================================================
// spinlock 测试
// ============================================================================

TEST(test_spinlock_basic) {
    zen::spinlock sl;
    ASSERT_FALSE(sl.is_locked());
    
    sl.lock();
    ASSERT_TRUE(sl.is_locked());
    
    sl.unlock();
    ASSERT_FALSE(sl.is_locked());
}

TEST(test_spinlock_try_lock) {
    zen::spinlock sl;
    
    ASSERT_TRUE(sl.try_lock());    // 未被锁，应该成功
    ASSERT_FALSE(sl.try_lock());   // 已被锁，应该失败
    
    sl.unlock();
    ASSERT_TRUE(sl.try_lock());    // 再次可以获取
    sl.unlock();
}

TEST(test_spinlock_with_lock_guard) {
    zen::spinlock sl;
    {
        zen::lock_guard<zen::spinlock> lg(sl);
        ASSERT_TRUE(sl.is_locked());
    }
    ASSERT_FALSE(sl.is_locked());
}

TEST(test_ticket_lock_basic) {
    zen::ticket_lock tl;
    
    ASSERT_TRUE(tl.try_lock());
    ASSERT_FALSE(tl.try_lock());
    tl.unlock();
    ASSERT_TRUE(tl.try_lock());
    tl.unlock();
}

// ============================================================================
// mutex 测试
// ============================================================================

TEST(test_mutex_basic) {
    zen::mutex m;
    m.lock();
    // 成功持有锁
    ASSERT_FALSE(m.try_lock()); // 不可重入，应失败（注意：Linux pthread 下可能不保证）
    m.unlock();
    ASSERT_TRUE(m.try_lock());
    m.unlock();
}

TEST(test_mutex_lock_guard) {
    zen::mutex m;
    {
        zen::lock_guard<zen::mutex> lg(m);
        // 锁住了
    }
    // 应该能重新 lock
    ASSERT_TRUE(m.try_lock());
    m.unlock();
}

TEST(test_unique_lock_defer) {
    zen::mutex m;
    zen::unique_lock<zen::mutex> ul(m, zen::defer_lock);
    ASSERT_FALSE(ul.owns_lock());
    
    ul.lock();
    ASSERT_TRUE(ul.owns_lock());
    
    ul.unlock();
    ASSERT_FALSE(ul.owns_lock());
}

TEST(test_unique_lock_try) {
    zen::mutex m;
    zen::unique_lock<zen::mutex> ul(m, zen::try_to_lock);
    ASSERT_TRUE(ul.owns_lock());
    // 再尝试（已持有）
}

TEST(test_unique_lock_move) {
    zen::mutex m;
    zen::unique_lock<zen::mutex> ul1(m);
    ASSERT_TRUE(ul1.owns_lock());
    
    zen::unique_lock<zen::mutex> ul2(static_cast<zen::unique_lock<zen::mutex>&&>(ul1));
    ASSERT_FALSE(ul1.owns_lock());
    ASSERT_TRUE(ul2.owns_lock());
}

// ============================================================================
// rwlock 测试
// ============================================================================

TEST(test_rwlock_basic_read) {
    zen::rwlock rw;
    rw.lock_shared();
    // 多个读锁可以共存（仅验证接口正常）
    rw.unlock_shared();
    ASSERT_TRUE(rw.try_lock_shared());
    rw.unlock_shared();
}

TEST(test_rwlock_basic_write) {
    zen::rwlock rw;
    rw.lock();
    rw.unlock();
    ASSERT_TRUE(rw.try_lock());
    rw.unlock();
}

TEST(test_shared_lock_raii) {
    zen::rwlock rw;
    {
        zen::shared_lock<zen::rwlock> sl(rw);
        ASSERT_TRUE(sl.owns_lock());
    }
    // 析构后应可再次获取
    ASSERT_TRUE(rw.try_lock());
    rw.unlock();
}

TEST(test_write_lock_raii) {
    zen::rwlock rw;
    {
        zen::write_lock<zen::rwlock> wl(rw);
        ASSERT_TRUE(wl.owns_lock());
    }
    ASSERT_TRUE(rw.try_lock_shared());
    rw.unlock_shared();
}

// ============================================================================
// thread 测试
// ============================================================================

TEST(test_thread_basic) {
    volatile bool ran = false;
    zen::thread t([&ran]{ ran = true; });
    t.join();
    ASSERT_TRUE(ran);
}

TEST(test_thread_with_arg) {
    volatile int result = 0;
    zen::thread t([&result](int x){ result = x * 2; }, 21);
    t.join();
    ASSERT_EQ(result, 42);
}

TEST(test_thread_hardware_concurrency) {
    unsigned n = zen::thread::hardware_concurrency();
    ASSERT_TRUE(n >= 1);
}

TEST(test_thread_detach) {
    // detach 后不应该 crash
    zen::thread t([]{ zen::this_thread::sleep_for(1); });
    t.detach();
    ASSERT_FALSE(t.joinable());
    // 短暂等待让 detached 线程完成
    zen::this_thread::sleep_for(10);
}

// ============================================================================
// this_thread 测试
// ============================================================================

TEST(test_this_thread_get_id) {
    auto id1 = zen::this_thread::get_id();
    auto id2 = zen::this_thread::get_id();
    ASSERT_EQ(id1, id2); // 同线程 ID 相同
}

TEST(test_this_thread_sleep) {
    auto t0 = zen::this_thread::monotonic_ms();
    zen::this_thread::sleep_for(20);
    auto t1 = zen::this_thread::monotonic_ms();
    ASSERT_TRUE(t1 - t0 >= 15); // 允许 5ms 误差
}

TEST(test_this_thread_monotonic) {
    auto t0 = zen::this_thread::monotonic_us();
    zen::this_thread::sleep_for(1);
    auto t1 = zen::this_thread::monotonic_us();
    ASSERT_TRUE(t1 > t0);
}

// ============================================================================
// 并发测试（mutex + thread 协作）
// ============================================================================

TEST(test_mutex_concurrent_counter) {
    zen::mutex m;
    volatile int counter = 0;
    const int N = 10;
    const int ROUNDS = 1000;
    
    zen::thread threads[N];
    
    for (int i = 0; i < N; ++i) {
        threads[i] = zen::thread([&m, &counter, ROUNDS] {
            for (int j = 0; j < ROUNDS; ++j) {
                zen::lock_guard<zen::mutex> lg(m);
                ++counter;
            }
        });
    }
    
    for (int i = 0; i < N; ++i) {
        threads[i].join();
    }
    
    ASSERT_EQ(counter, N * ROUNDS);
}

TEST(test_condition_variable_producer_consumer) {
    zen::mutex m;
    zen::condition_variable cv;
    volatile bool ready = false;
    volatile int value  = 0;
    
    zen::thread producer([&m, &cv, &ready, &value] {
        zen::this_thread::sleep_for(5);
        {
            zen::lock_guard<zen::mutex> lg(m);
            value = 123;
            ready = true;
        }
        cv.notify_one();
    });
    
    {
        zen::unique_lock<zen::mutex> ul(m);
        cv.wait(ul, [&ready]{ return static_cast<bool>(ready); });
    }
    
    ASSERT_EQ(value, 123);
    producer.join();
}

// ============================================================================
// main
// ============================================================================

int main() {
    std::printf("=== Error + Threading Module Tests ===\n");
    std::printf("%d/%d PASSED\n", g_passed, g_total);
    
    if (g_passed == g_total) {
        std::printf("All tests passed!\n");
        return 0;
    } else {
        std::printf("%d FAILED\n", g_total - g_passed);
        return 1;
    }
}
