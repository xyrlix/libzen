#pragma once

#include <string>

namespace zen {
namespace process {

// 守护进程选项
struct daemon_options {
    std::string work_dir;
    std::string pid_file;
    std::string stdout_file;
    std::string stderr_file;
    bool close_stdin;
    bool close_stdout;
    bool close_stderr;
    bool umask_0;
};

// 守护进程类
class daemon {
public:
    daemon();
    ~daemon();
    
    // 启动守护进程
    bool start(const std::function<void()>& main_func);
    bool start(const daemon_options& options, const std::function<void()>& main_func);
    
    // PID 文件管理
    bool write_pid_file(const std::string& path);
    bool read_pid_file(const std::string& path, pid_t& pid);
    bool lock_pid_file(const std::string& path);
    bool unlock_pid_file(const std::string& path);
    
    // 检查运行
    static bool is_running(pid_t pid);
    static pid_t get_pid_from_file(const std::string& path);
    
    // 日志重定向
    bool redirect_stdio(const std::string& stdout_path, const std::string& stderr_path);
    
    // 工作目录
    bool change_work_directory(const std::string& path);
    
    // 用户/组
    bool change_user(const std::string& username);
    bool change_group(const std::string& groupname);
    
private:
    void cleanup();
    
    std::string pid_file_path_;
    int pid_file_fd_;
};

// 双重分叉守护进程（更安全的守护进程创建）
class double_fork_daemon {
public:
    static bool daemonize(const daemon_options& options);
};

} // namespace process
} // namespace zen
