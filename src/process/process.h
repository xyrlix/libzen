#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <functional>

namespace zen {
namespace process {

// 进程状态
enum class process_state {
    not_started,
    running,
    stopped,
    finished,
    error
};

// 进程信息
struct process_info {
    pid_t pid;
    pid_t ppid;
    std::string name;
    std::string path;
    std::vector<std::string> args;
    std::string working_dir;
    process_state state;
    int exit_code;
    uint64_t start_time;
    uint64_t cpu_time;
    uint64_t memory_used;
};

// 进程选项
struct process_options {
    std::string working_dir;
    std::vector<std::string> env;
    bool redirect_stdin;
    bool redirect_stdout;
    bool redirect_stderr;
    bool merge_output;
    bool create_new_console;
    bool detached;
    bool use_path;
};

// 回调类型
using output_callback = std::function<void(const std::string&)>;
using exit_callback = std::function<void(int exit_code)>;

// 进程类
class process {
public:
    process();
    explicit process(const std::string& command);
    process(const std::string& program, const std::vector<std::string>& args);
    ~process();
    
    // 禁止拷贝
    process(const process&) = delete;
    process& operator=(const process&) = delete;
    
    // 允许移动
    process(process&& other) noexcept;
    process& operator=(process&& other) noexcept;
    
    // 配置
    void set_program(const std::string& program);
    void set_arguments(const std::vector<std::string>& args);
    void set_working_directory(const std::string& dir);
    void set_environment(const std::vector<std::string>& env);
    void set_environment_variable(const std::string& name, const std::string& value);
    
    void set_redirect_stdin(bool redirect = true);
    void set_redirect_stdout(bool redirect = true);
    void set_redirect_stderr(bool redirect = true);
    void set_merge_output(bool merge = true);
    
    void set_detached(bool detached = true);
    void set_create_new_console(bool create = true);
    
    void set_output_callback(output_callback cb) { on_output_ = cb; }
    void set_error_callback(output_callback cb) { on_error_ = cb; }
    void set_exit_callback(exit_callback cb) { on_exit_ = cb; }
    
    // 启动/停止
    bool start();
    bool start(const process_options& options);
    bool detach();
    void stop();
    void terminate();
    void kill();
    void wait();
    bool wait_for(int timeout_ms);
    
    // 输入/输出
    bool write(const void* data, size_t len);
    bool write(const std::string& str);
    bool close_stdin();
    
    std::string read_stdout();
    std::string read_stderr();
    std::string read_all();
    
    // 状态查询
    bool is_running() const;
    bool is_finished() const;
    int get_exit_code() const;
    pid_t get_pid() const;
    process_state get_state() const;
    
    process_info get_info() const;
    
    // 静态方法
    static process_info get_current_process_info();
    static process_info get_process_info(pid_t pid);
    static std::vector<process_info> list_processes();
    static bool kill(pid_t pid, int signal = 9);
    static bool exists(pid_t pid);
    
private:
    void cleanup();
    void setup_redirects(const process_options& options);
    void run_output_thread();
    
    std::string program_;
    std::vector<std::string> args_;
    std::string working_dir_;
    std::vector<std::string> env_;
    
    bool redirect_stdin_;
    bool redirect_stdout_;
    bool redirect_stderr_;
    bool merge_output_;
    bool detached_;
    bool create_new_console_;
    
    output_callback on_output_;
    output_callback on_error_;
    exit_callback on_exit_;
    
    pid_t pid_;
    process_state state_;
    int exit_code_;
    
    int stdin_fd_;
    int stdout_fd_;
    int stderr_fd_;
    
    std::thread output_thread_;
    bool thread_running_;
    std::mutex output_mutex_;
    std::string stdout_buffer_;
    std::string stderr_buffer_;
};

// 进程池
class process_pool {
public:
    process_pool(size_t max_processes = 4);
    ~process_pool();
    
    // 提交进程
    pid_t execute(const std::string& command, output_callback cb = nullptr);
    pid_t execute(const std::string& program, const std::vector<std::string>& args,
                   output_callback cb = nullptr);
    
    // 管理
    void stop(pid_t pid);
    void stop_all();
    void wait_all();
    
    // 查询
    size_t get_running_count() const;
    size_t get_max_processes() const { return max_processes_; }
    std::vector<pid_t> get_running_pids() const;
    
private:
    void cleanup_finished();
    
    size_t max_processes_;
    std::unordered_map<pid_t, std::unique_ptr<process>> processes_;
    mutable threading::mutex processes_mutex_;
};

} // namespace process
} // namespace zen
