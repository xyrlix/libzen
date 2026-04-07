#include <gtest/gtest.h>
#include <zen/system.h>

using namespace zen;
using namespace zen::system;

TEST(SystemInfoTest, CPUCount) {
    int cpus = cpu_count();
    EXPECT_GT(cpus, 0);
    EXPECT_LE(cpus, 256);
}

TEST(SystemInfoTest, MemoryInfo) {
    uint64_t total = total_memory();
    EXPECT_GT(total, 0);
    
    uint64_t avail = available_memory();
    EXPECT_GT(avail, 0);
    EXPECT_LE(avail, total);
}

TEST(SystemInfoTest, Hostname) {
    std::string host = hostname();
    EXPECT_FALSE(host.empty());
}

TEST(SystemInfoTest, Username) {
    std::string user = username();
    EXPECT_FALSE(user.empty());
}

TEST(SystemInfoTest, ProcessIDs) {
    int pid_val = pid();
    EXPECT_GT(pid_val, 0);
    
    int ppid_val = ppid();
    EXPECT_GT(ppid_val, 0);
    
    int tid_val = tid();
    EXPECT_GT(tid_val, 0);
}

TEST(SystemInfoTest, ExecutablePath) {
    std::string path = executable_path();
    EXPECT_FALSE(path.empty());
    EXPECT_TRUE(path[0] == '/');
}

TEST(SystemInfoTest, CurrentWorkingDirectory) {
    std::string cwd = current_working_directory();
    EXPECT_FALSE(cwd.empty());
    EXPECT_TRUE(cwd[0] == '/');
}

TEST(SystemInfoTest, BootTime) {
    uint64_t boot = boot_time();
    EXPECT_GT(boot, 0);
}

TEST(SystemUtilsTest, Sleep) {
    auto start = std::chrono::steady_clock::now();
    sleep_ms(100);
    auto end = std::chrono::steady_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    EXPECT_GE(duration.count(), 100);
}

TEST(SystemUtilsTest, TickCount) {
    uint64_t t1 = get_tick_count();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    uint64_t t2 = get_tick_count();
    
    EXPECT_GT(t2, t1);
}
