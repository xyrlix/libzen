#include <zen/system.h>
#include <iostream>
#include <thread>
#include <chrono>

using namespace zen;
using namespace zen::system;

void system_info_example() {
    std::cout << "=== System Information ===" << std::endl;
    
    // CPU 信息
    int cpus = cpu_count();
    std::cout << "CPU cores: " << cpus << std::endl;
    
    // 内存信息
    uint64_t total_mem = total_memory();
    uint64_t avail_mem = available_memory();
    std::cout << "Total memory: " << total_mem / (1024 * 1024) << " MB" << std::endl;
    std::cout << "Available memory: " << avail_mem / (1024 * 1024) << " MB" << std::endl;
    std::cout << "Memory usage: " << (100 * (total_mem - avail_mem) / total_mem) << "%" << std::endl;
    
    // 主机信息
    std::string host = hostname();
    std::string user = username();
    std::cout << "Hostname: " << host << std::endl;
    std::cout << "Username: " << user << std::endl;
    
    // 进程信息
    int pid_val = pid();
    int ppid_val = ppid();
    int tid_val = tid();
    std::cout << "PID: " << pid_val << std::endl;
    std::cout << "PPID: " << ppid_val << std::endl;
    std::cout << "TID: " << tid_val << std::endl;
    
    // 可执行文件路径
    std::string exe_path = executable_path();
    std::cout << "Executable: " << exe_path << std::endl;
    
    // 当前工作目录
    std::string cwd = current_working_directory();
    std::cout << "CWD: " << cwd << std::endl;
    
    // 启动时间
    uint64_t boot = boot_time();
    std::cout << "Boot time: " << boot << " seconds ago" << std::endl;
    
    std::cout << std::endl;
}

void time_example() {
    std::cout << "=== Time Functions ===" << std::endl;
    
    // 获取各种时间
    uint64_t ticks = get_tick_count();
    double realtime = get_realtime();
    uint64_t monotonic = get_monotonic_time_ms();
    
    std::cout << "Tick count: " << ticks << std::endl;
    std::cout << "Realtime: " << realtime << " seconds" << std::endl;
    std::cout << "Monotonic: " << monotonic << " ms" << std::endl;
    
    // 测试 sleep
    std::cout << "\nTesting sleep functions:" << std::endl;
    
    auto start = std::chrono::steady_clock::now();
    sleep_ms(100);
    auto end = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "sleep_ms(100) took " << elapsed.count() << " ms" << std::endl;
    
    start = std::chrono::steady_clock::now();
    usleep(100000);  // 100000 microseconds = 100 ms
    end = std::chrono::steady_clock::now();
    elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "usleep(100000) took " << elapsed.count() << " ms" << std::endl;
    
    start = std::chrono::steady_clock::now();
    sleep(1);
    end = std::chrono::steady_clock::now();
    elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "sleep(1) took " << elapsed.count() << " ms" << std::endl;
    
    std::cout << std::endl;
}

void affinity_example() {
    std::cout << "=== CPU Affinity Example ===" << std::endl;
    
    int current_cpu = get_current_cpu();
    std::cout << "Current CPU: " << current_cpu << std::endl;
    
    // 设置 CPU 亲和性（绑定到 CPU 0）
    // set_cpu_affinity({0});
    
    std::cout << std::endl;
}

void load_average_example() {
    std::cout << "=== Load Average ===" << std::endl;
    
    double load[3];
    if (get_load_average(load)) {
        std::cout << "1 min:  " << load[0] << std::endl;
        std::cout << "5 min:  " << load[1] << std::endl;
        std::cout << "15 min: " << load[2] << std::endl;
    }
    
    std::cout << std::endl;
}

void high_resolution_example() {
    std::cout << "=== High Resolution Time ===" << std::endl;
    
    uint64_t start = get_high_resolution_time();
    
    // 执行一些工作
    volatile int sum = 0;
    for (int i = 0; i < 1000000; ++i) {
        sum += i;
    }
    
    uint64_t end = get_high_resolution_time();
    uint64_t elapsed = end - start;
    
    std::cout << "High resolution timer: " << elapsed << " ticks" << std::endl;
    std::cout << "Computed sum: " << sum << std::endl;
    
    std::cout << std::endl;
}

int main() {
    system_info_example();
    time_example();
    affinity_example();
    load_average_example();
    high_resolution_example();
    
    return 0;
}
