#pragma once

#include <string>
#include <vector>

namespace zen {
namespace process {

// 进程工具函数
class process_utils {
public:
    // 获取当前进程 ID
    static pid_t get_pid();
    static pid_t get_ppid();
    
    // 获取进程组 ID
    static pid_t get_pgid();
    static pid_t get_pgid(pid_t pid);
    
    // 设置进程组
    static bool set_pgid(pid_t pid, pid_t pgid);
    static bool setpgid();
    
    // 会话管理
    static pid_t getsid();
    static pid_t getsid(pid_t pid);
    static pid_t setsid();
    
    // 用户/组
    static uid_t get_uid();
    static uid_t get_euid();
    static gid_t get_gid();
    static gid_t get_egid();
    
    static bool set_uid(uid_t uid);
    static bool set_gid(gid_t gid);
    static bool set_euid(uid_t uid);
    static bool set_egid(gid_t gid);
    
    static bool set_user(const std::string& username);
    static bool set_group(const std::string& groupname);
    
    // 进程名
    static std::string get_process_name();
    static std::string get_process_name(pid_t pid);
    static bool set_process_name(const std::string& name);
    
    // 可执行文件路径
    static std::string get_executable_path();
    static std::string get_executable_path(pid_t pid);
    
    // 参数
    static std::vector<std::string> get_arguments();
    static std::vector<std::string> get_arguments(pid_t pid);
    
    // 环境变量
    static std::vector<std::string> get_environment();
    static std::vector<std::string> get_environment(pid_t pid);
    
    static std::string get_env(const std::string& name);
    static bool set_env(const std::string& name, const std::string& value, bool overwrite = true);
    static bool unset_env(const std::string& name);
    static void clear_env();
    
    // 工作目录
    static std::string get_current_directory();
    static bool set_current_directory(const std::string& path);
    
    // 优先级
    static int get_priority();
    static int get_priority(pid_t pid);
    static bool set_priority(int priority);
    static bool set_priority(pid_t pid, int priority);
    
    // nice 值（Linux）
    static int get_nice();
    static int get_nice(pid_t pid);
    static bool set_nice(int increment);
    
    // CPU 亲和性
    static std::vector<int> get_cpu_affinity();
    static bool set_cpu_affinity(const std::vector<int>& cpus);
    static bool set_cpu_affinity(pid_t pid, const std::vector<int>& cpus);
    
    // 资源限制
    struct rlimit_info {
        rlim_t cur;
        rlim_t max;
    };
    
    static rlimit_info get_rlimit(int resource);
    static bool set_rlimit(int resource, rlim_t cur, rlim_t max = RLIM_INFINITY);
    
    static rlimit_info get_file_descriptor_limit();
    static bool set_file_descriptor_limit(rlim_t max);
    
    // 打开的文件描述符
    static int get_open_file_count();
    static std::vector<int> get_open_fds();
    static bool close_all_fds();
    static bool close_fds_above(int min_fd);
    
    // 进程时间
    struct process_time {
        uint64_t user_time;   // 用户态时间（纳秒）
        uint64_t system_time; // 内核态时间（纳秒）
        uint64_t real_time;   // 真实时间（纳秒）
    };
    
    static process_time get_process_time();
    static process_time get_process_time(pid_t pid);
    
    // 内存使用
    struct memory_info {
        uint64_t rss;      // 常驻集大小
        uint64_t vms;      // 虚拟内存大小
        uint64_t shared;   // 共享内存
        uint64_t text;     // 代码段
        uint64_t data;     // 数据段
        uint64_t lib;      // 库
    };
    
    static memory_info get_memory_info();
    static memory_info get_memory_info(pid_t pid);
    
    // CPU 使用率
    static double get_cpu_usage();
    static double get_cpu_usage(pid_t pid);
    
    // 线程
    static pid_t get_tid();
    static std::vector<pid_t> get_threads();
    static std::vector<pid_t> get_threads(pid_t pid);
    
    // 检查
    static bool is_alive(pid_t pid);
    static bool is_zombie(pid_t pid);
    static bool is_stopped(pid_t pid);
};

} // namespace process
} // namespace zen
