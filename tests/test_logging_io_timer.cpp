/**
 * @file test_logging_io_timer.cpp
 * @brief logging / io / timer 模块测试
 *
 * 编译（从项目根目录）：
 *   g++ -std=c++11 -Iinclude -I. tests/test_logging_io_timer.cpp -o test_lit \
 *       -lpthread && ./test_lit
 */

// ============================================================================
// 极简测试框架
// ============================================================================
#include <cstdio>
#include <cstring>

static int g_total  = 0;
static int g_passed = 0;
static bool g_current_failed = false;

#define TEST(name) \
    static void name##_impl(); \
    static void name() { \
        g_current_failed = false; \
        printf("  %-50s", #name); \
        name##_impl(); \
        ++g_total; \
        if (!g_current_failed) { ++g_passed; printf("OK\n"); } \
    } \
    struct name##_reg { name##_reg() { name(); } } name##_instance; \
    static void name##_impl()

#define ASSERT_TRUE(expr) do { \
    if (!(expr)) { \
        printf("FAIL: %s (line %d)\n", #expr, __LINE__); \
        g_current_failed = true; return; \
    } \
} while(0)

#define ASSERT_EQ(a, b)  ASSERT_TRUE((a) == (b))
#define ASSERT_NE(a, b)  ASSERT_TRUE((a) != (b))
#define ASSERT_FALSE(expr) ASSERT_TRUE(!(expr))
#define ASSERT_GT(a, b) ASSERT_TRUE((a) > (b))
#define ASSERT_GE(a, b) ASSERT_TRUE((a) >= (b))

// ============================================================================
// 头文件
// ============================================================================
#include "src/logging/log_level.h"
#include "src/logging/log_formatter.h"
#include "src/logging/log_sink.h"
#include "src/logging/logger.h"
#include "src/io/file_io.h"
#include "src/timer/timer.h"
#include "src/timer/timer_queue.h"
#include "src/timer/timer_manager.h"

// ============================================================================
// ---- logging/log_level -----
// ============================================================================

TEST(test_level_to_string) {
    ASSERT_TRUE(strcmp(zen::level_to_string(zen::log_level::INFO), "INFO ") == 0);
    ASSERT_TRUE(strcmp(zen::level_to_string(zen::log_level::ERROR), "ERROR") == 0);
    ASSERT_TRUE(strcmp(zen::level_to_string(zen::log_level::TRACE), "TRACE") == 0);
}

TEST(test_level_enabled) {
    ASSERT_TRUE(zen::level_enabled(zen::log_level::WARN, zen::log_level::INFO));
    ASSERT_TRUE(zen::level_enabled(zen::log_level::INFO, zen::log_level::INFO));
    ASSERT_FALSE(zen::level_enabled(zen::log_level::DEBUG, zen::log_level::INFO));
}

// ============================================================================
// ---- log_formatter -----
// ============================================================================

TEST(test_formatter_basic) {
    zen::log_formatter fmt;
    zen::log_record rec;
    rec.level     = zen::log_level::INFO;
    rec.file      = "test.cpp";
    rec.line      = 42;
    rec.func      = "test_func";
    rec.message   = "hello world";
    rec.time_ms   = 1234;
    rec.thread_id = 99;

    char buf[512];
    int n = fmt.format(rec, buf, sizeof(buf));
    ASSERT_GT(n, 0);
    // 应包含 INFO 和 message
    ASSERT_TRUE(strstr(buf, "INFO") != nullptr);
    ASSERT_TRUE(strstr(buf, "hello world") != nullptr);
}

TEST(test_formatter_file_basename) {
    zen::format_options opts;
    opts.show_file = true;
    zen::log_formatter fmt(opts);

    zen::log_record rec;
    rec.level   = zen::log_level::DEBUG;
    rec.file    = "/path/to/myfile.cpp";
    rec.line    = 7;
    rec.func    = "f";
    rec.message = "msg";
    rec.time_ms = 0;
    rec.thread_id = 0;

    char buf[512];
    fmt.format(rec, buf, sizeof(buf));
    // 应只显示 myfile.cpp
    ASSERT_TRUE(strstr(buf, "myfile.cpp") != nullptr);
    ASSERT_TRUE(strstr(buf, "/path/to/") == nullptr);
}

TEST(test_formatter_no_file) {
    zen::format_options opts;
    opts.show_file = false;
    opts.show_time = false;
    zen::log_formatter fmt(opts);

    zen::log_record rec;
    rec.level   = zen::log_level::WARN;
    rec.file    = "skip.cpp";
    rec.line    = 1;
    rec.func    = "f";
    rec.message = "warn msg";
    rec.time_ms = 0;
    rec.thread_id = 0;

    char buf[512];
    fmt.format(rec, buf, sizeof(buf));
    ASSERT_TRUE(strstr(buf, "skip.cpp") == nullptr);
    ASSERT_TRUE(strstr(buf, "warn msg") != nullptr);
}

// ============================================================================
// ---- log_sink (memory_sink) -----
// ============================================================================

TEST(test_memory_sink_write) {
    zen::format_options opts;
    opts.show_time = false;
    opts.show_file = false;
    zen::log_sink ms = zen::make_memory_sink(opts, zen::log_level::TRACE);

    zen::log_record rec;
    rec.level   = zen::log_level::INFO;
    rec.file    = "f.cpp";
    rec.line    = 1;
    rec.func    = "g";
    rec.message = "sink_test_message";
    rec.time_ms = 0;
    rec.thread_id = 0;

    ms.write(rec);

    zen::memory_sink_impl* impl = zen::memory_sink_cast(ms);
    ASSERT_TRUE(impl->contains("sink_test_message"));

    ms.destroy();
}

TEST(test_memory_sink_level_filter) {
    zen::format_options opts;
    opts.show_time = false;
    opts.show_file = false;
    // min_level = WARN，DEBUG 应被过滤
    zen::log_sink ms = zen::make_memory_sink(opts, zen::log_level::WARN);

    zen::log_record rec;
    rec.level   = zen::log_level::DEBUG;
    rec.file    = "f.cpp";
    rec.line    = 1;
    rec.func    = "g";
    rec.message = "should_be_filtered";
    rec.time_ms = 0;
    rec.thread_id = 0;

    ms.write(rec);  // 应被过滤

    zen::memory_sink_impl* impl = zen::memory_sink_cast(ms);
    ASSERT_FALSE(impl->contains("should_be_filtered"));

    ms.destroy();
}

// ============================================================================
// ---- logger -----
// ============================================================================

TEST(test_logger_basic) {
    zen::logger log;
    zen::format_options opts;
    opts.show_time = false;
    opts.show_file = false;
    zen::log_sink ms = zen::make_memory_sink(opts);
    zen::memory_sink_impl* impl = zen::memory_sink_cast(ms);
    log.add_sink(ms);  // ms 所有权转移给 logger

    log.set_level(zen::log_level::DEBUG);
    ZEN_LOG_INFO(log, "logger_test_ok");

    ASSERT_TRUE(impl->contains("logger_test_ok"));
}

TEST(test_logger_level_filter) {
    zen::logger log;
    log.set_level(zen::log_level::WARN);

    zen::format_options opts;
    opts.show_time = false;
    opts.show_file = false;
    zen::log_sink ms = zen::make_memory_sink(opts, zen::log_level::TRACE);
    zen::memory_sink_impl* impl = zen::memory_sink_cast(ms);
    log.add_sink(ms);

    ZEN_LOG_DEBUG(log, "filtered_debug");
    ZEN_LOG_WARN(log, "visible_warn");

    ASSERT_FALSE(impl->contains("filtered_debug"));
    ASSERT_TRUE(impl->contains("visible_warn"));
}

TEST(test_logger_logf) {
    zen::logger log;
    zen::format_options opts;
    opts.show_time = false;
    opts.show_file = false;
    zen::log_sink ms = zen::make_memory_sink(opts, zen::log_level::TRACE);
    zen::memory_sink_impl* impl = zen::memory_sink_cast(ms);
    log.add_sink(ms);

    ZEN_LOGF_INFO(log, "value=%d", 42);

    ASSERT_TRUE(impl->contains("value=42"));
}

TEST(test_logger_multi_sink) {
    zen::logger log;
    zen::format_options opts;
    opts.show_time = false;
    opts.show_file = false;

    zen::log_sink ms1 = zen::make_memory_sink(opts, zen::log_level::TRACE);
    zen::log_sink ms2 = zen::make_memory_sink(opts, zen::log_level::TRACE);
    zen::memory_sink_impl* impl1 = zen::memory_sink_cast(ms1);
    zen::memory_sink_impl* impl2 = zen::memory_sink_cast(ms2);

    log.add_sink(ms1);
    log.add_sink(ms2);
    ASSERT_EQ(log.sink_count(), 2);

    ZEN_LOG_INFO(log, "broadcast_msg");

    ASSERT_TRUE(impl1->contains("broadcast_msg"));
    ASSERT_TRUE(impl2->contains("broadcast_msg"));
}

// ============================================================================
// ---- io / file_io -----
// ============================================================================

// 临时文件名（测试用）
static const char* TMP_FILE = "test_zen_io_tmp.bin";

TEST(test_file_write_read) {
    // 写
    zen::file fw(TMP_FILE, zen::open_mode::WRITE_TRUNC);
    ASSERT_TRUE(fw.is_open());
    zen::ssize_t w = fw.write("hello zen", 9);
    ASSERT_EQ(w, 9);
    fw.close();

    // 读
    zen::file fr(TMP_FILE, zen::open_mode::READ);
    ASSERT_TRUE(fr.is_open());
    char buf[64] = {};
    zen::ssize_t r = fr.read(buf, 64);
    ASSERT_EQ(r, 9);
    ASSERT_TRUE(strncmp(buf, "hello zen", 9) == 0);
}

TEST(test_file_size) {
    zen::file fw(TMP_FILE, zen::open_mode::WRITE_TRUNC);
    ASSERT_TRUE(fw.is_open());
    fw.write("12345", 5);
    fw.close();

    long long sz = zen::file_size(TMP_FILE);
    ASSERT_EQ(sz, 5);
}

TEST(test_file_exists) {
    ASSERT_TRUE(zen::file_exists(TMP_FILE));  // 前面测试已创建
    ASSERT_FALSE(zen::file_exists("nonexistent_xyz_abc.txt"));
}

TEST(test_read_all) {
    zen::file fw(TMP_FILE, zen::open_mode::WRITE_TRUNC);
    fw.write("readall_test", 12);
    fw.close();

    char buf[64] = {};
    zen::ssize_t n = zen::read_all(TMP_FILE, buf, sizeof(buf));
    ASSERT_EQ(n, 12);
    ASSERT_TRUE(strcmp(buf, "readall_test") == 0);
}

TEST(test_write_all) {
    zen::ssize_t n = zen::write_all(TMP_FILE, "write_all_ok", 12);
    ASSERT_EQ(n, 12);

    char buf[64] = {};
    zen::read_all(TMP_FILE, buf, sizeof(buf));
    ASSERT_TRUE(strcmp(buf, "write_all_ok") == 0);
}

TEST(test_file_seek_tell) {
    zen::file fw(TMP_FILE, zen::open_mode::WRITE_TRUNC);
    fw.write("ABCDE", 5);
    fw.close();

    zen::file fr(TMP_FILE, zen::open_mode::READ);
    ASSERT_TRUE(fr.seek(2));
    ASSERT_EQ(fr.tell(), 2);
    char c;
    fr.read(&c, 1);
    ASSERT_EQ(c, 'C');
}

TEST(test_file_append) {
    zen::write_all(TMP_FILE, "part1_", 6);
    zen::append_all(TMP_FILE, "part2", 5);

    char buf[64] = {};
    zen::read_all(TMP_FILE, buf, sizeof(buf));
    ASSERT_TRUE(strcmp(buf, "part1_part2") == 0);
}

TEST(test_file_reader_readline) {
    zen::write_all(TMP_FILE, "line1\nline2\nline3", 17);

    zen::file_reader reader(TMP_FILE);
    ASSERT_TRUE(reader.is_open());

    char line[64];
    zen::ssize_t n1 = reader.read_line(line, sizeof(line));
    ASSERT_GT(n1, 0);
    ASSERT_TRUE(strcmp(line, "line1") == 0);

    zen::ssize_t n2 = reader.read_line(line, sizeof(line));
    ASSERT_GT(n2, 0);
    ASSERT_TRUE(strcmp(line, "line2") == 0);

    zen::ssize_t n3 = reader.read_line(line, sizeof(line));
    ASSERT_GT(n3, 0);
    ASSERT_TRUE(strcmp(line, "line3") == 0);

    // EOF
    zen::ssize_t n4 = reader.read_line(line, sizeof(line));
    ASSERT_EQ(n4, 0);
}

TEST(test_file_writer_buffered) {
    zen::file_writer writer(TMP_FILE);
    ASSERT_TRUE(writer.is_open());
    writer.write_str("buffered_");
    writer.write_str("write_test");
    writer.flush();  // 显式刷盘后再读

    char buf[64] = {};
    zen::read_all(TMP_FILE, buf, sizeof(buf));
    ASSERT_TRUE(strcmp(buf, "buffered_write_test") == 0);
}

TEST(test_file_move_semantics) {
    zen::file f1(TMP_FILE, zen::open_mode::WRITE_TRUNC);
    ASSERT_TRUE(f1.is_open());

    zen::file f2(static_cast<zen::file&&>(f1));  // 移动构造
    ASSERT_FALSE(f1.is_open());
    ASSERT_TRUE(f2.is_open());

    f2.write("moved", 5);
    f2.close();

    char buf[16] = {};
    zen::read_all(TMP_FILE, buf, sizeof(buf));
    ASSERT_TRUE(strcmp(buf, "moved") == 0);
}

// ============================================================================
// ---- timer -----
// ============================================================================

TEST(test_timer_queue_push_pop) {
    zen::timer_queue q;
    ASSERT_TRUE(q.empty());

    zen::timer_entry e;
    e.id = 1;
    e.expire_ms = 1000;
    e.callback  = nullptr;
    e.cancelled = false;

    bool ok = q.push(e);
    ASSERT_TRUE(ok);
    ASSERT_EQ(q.size(), 1);

    // 尚未到期
    zen::timer_entry out[8];
    int n = q.pop_expired(999, out, 8);
    ASSERT_EQ(n, 0);
    ASSERT_EQ(q.size(), 1);

    // 到期
    n = q.pop_expired(1000, out, 8);
    ASSERT_EQ(n, 1);
    ASSERT_EQ(out[0].id, (zen::timer_id)1);
    ASSERT_TRUE(q.empty());
}

TEST(test_timer_queue_heap_order) {
    zen::timer_queue q;
    zen::timer_entry e;
    e.callback  = nullptr;
    e.cancelled = false;

    // 乱序插入
    for (int i = 9; i >= 1; --i) {
        e.id = (zen::timer_id)i;
        e.expire_ms = (unsigned long long)i * 100;
        q.push(e);
    }
    ASSERT_EQ(q.size(), 9);

    zen::timer_entry out[16];
    int n = q.pop_expired(900, out, 16);
    ASSERT_EQ(n, 9);
    // 应按到期时间升序弹出
    for (int i = 0; i < 8; ++i) {
        ASSERT_TRUE(out[i].expire_ms <= out[i+1].expire_ms);
    }
}

TEST(test_timer_queue_cancel) {
    zen::timer_queue q;
    zen::timer_entry e;
    e.callback  = nullptr;
    e.cancelled = false;
    e.id = 42;
    e.expire_ms = 100;
    q.push(e);

    bool cancelled = q.cancel(42);
    ASSERT_TRUE(cancelled);

    zen::timer_entry out[8];
    int n = q.pop_expired(200, out, 8);
    ASSERT_EQ(n, 0);  // 被取消，跳过
    ASSERT_TRUE(q.empty());
}

// -------- timer_manager --------

static int s_once_fired = 0;
static void once_cb(zen::timer_id, void*) { ++s_once_fired; }

TEST(test_timer_manager_once) {
    s_once_fired = 0;
    zen::timer_manager tm;
    zen::timer_id id = tm.add_once(10, once_cb);
    ASSERT_NE(id, zen::INVALID_TIMER_ID);

    // tick 几次，10ms 内可能未到期
    zen::this_thread::sleep_for(20);
    tm.tick();

    ASSERT_EQ(s_once_fired, 1);

    // 再 tick 不应再触发
    tm.tick();
    ASSERT_EQ(s_once_fired, 1);
}

static int s_repeat_count = 0;
static void repeat_cb(zen::timer_id, void*) { ++s_repeat_count; }

TEST(test_timer_manager_repeat) {
    s_repeat_count = 0;
    zen::timer_manager tm;
    zen::timer_id id = tm.add_repeat(20, repeat_cb);
    ASSERT_NE(id, zen::INVALID_TIMER_ID);

    // 等待约 70ms，应触发约 3 次
    for (int i = 0; i < 7; ++i) {
        zen::this_thread::sleep_for(10);
        tm.tick();
    }
    ASSERT_GE(s_repeat_count, 2);

    tm.cancel(id);
    int prev = s_repeat_count;
    zen::this_thread::sleep_for(30);
    tm.tick();
    ASSERT_EQ(s_repeat_count, prev);  // 取消后不再触发
}

TEST(test_timer_manager_cancel_before_fire) {
    int fired = 0;
    auto cb = [](zen::timer_id, void* p){ ++(*static_cast<int*>(p)); };
    zen::timer_manager tm;
    zen::timer_id id = tm.add_once(200, cb, &fired);

    ASSERT_NE(id, zen::INVALID_TIMER_ID);
    tm.cancel(id);

    zen::this_thread::sleep_for(250);
    tm.tick();
    ASSERT_EQ(fired, 0);
}

TEST(test_timer_manager_tick_until_empty) {
    int count = 0;
    auto cb = [](zen::timer_id, void* p){ ++(*static_cast<int*>(p)); };
    zen::timer_manager tm;
    tm.add_once(10,  cb, &count);
    tm.add_once(30,  cb, &count);
    tm.add_once(50,  cb, &count);

    tm.tick_until_empty(5, 2000);
    ASSERT_EQ(count, 3);
}

TEST(test_timer_manager_multi_timers) {
    static int order[4];
    static int idx = 0;
    idx = 0;

    auto make_cb = [](int v) -> zen::timer_callback {
        // 无法在 C++11 捕获 v 用于函数指针，用静态数组存
        return nullptr;  // placeholder
    };
    (void)make_cb;

    // 改用全局状态
    struct Ctx { int val; };
    static Ctx ctxs[4] = {{1},{2},{3},{4}};

    auto cb = [](zen::timer_id, void* p){
        order[idx++] = static_cast<Ctx*>(p)->val;
    };

    zen::timer_manager tm;
    tm.add_once(40,  cb, &ctxs[3]);  // 第4个触发
    tm.add_once(10,  cb, &ctxs[0]);  // 第1个触发
    tm.add_once(20,  cb, &ctxs[1]);  // 第2个触发
    tm.add_once(30,  cb, &ctxs[2]);  // 第3个触发

    tm.tick_until_empty(5, 2000);

    ASSERT_EQ(idx, 4);
    ASSERT_EQ(order[0], 1);
    ASSERT_EQ(order[1], 2);
    ASSERT_EQ(order[2], 3);
    ASSERT_EQ(order[3], 4);
}

// ============================================================================
// 主函数
// ============================================================================

int main() {
    printf("========================================\n");
    printf("  libzen - logging / io / timer 测试\n");
    printf("========================================\n\n");

    // 清理临时文件
    remove(TMP_FILE);

    printf("\n[logging 模块]\n");
    // 测试通过自动注册执行

    printf("\n[io 模块]\n");
    // 同上

    printf("\n[timer 模块]\n");
    // 同上

    printf("\n========================================\n");
    printf("  结果: %d / %d 通过\n", g_passed, g_total);
    printf("========================================\n");

    // 清理
    remove(TMP_FILE);

    return (g_passed == g_total) ? 0 : 1;
}
