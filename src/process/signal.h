#pragma once

#include <string>
#include <functional>
#include <unordered_map>

namespace zen {
namespace process {

// 信号处理
class signal_handler {
public:
    using signal_callback = std::function<void(int signal)>;
    
    signal_handler();
    ~signal_handler();
    
    // 注册信号处理
    bool register_signal(int signal, signal_callback callback);
    bool unregister_signal(int signal);
    
    // 阻塞/解除阻塞
    void block_signal(int signal);
    void unblock_signal(int signal);
    void block_all();
    void unblock_all();
    
    // 发送信号
    static bool send(pid_t pid, int signal);
    static bool send_group(pid_t pgid, int signal);
    
    // 常用信号
    static const int SIG_HUP = 1;    // 挂起
    static const int SIG_INT = 2;    // 中断（Ctrl+C）
    static const int SIG_QUIT = 3;   // 退出
    static const int SIG_ILL = 4;    // 非法指令
    static const int SIG_TRAP = 5;   // 跟踪断点
    static const int SIG_ABRT = 6;   // 中止
    static const int SIG_BUS = 7;    // 总线错误
    static const int SIG_FPE = 8;    // 浮点异常
    static const int SIG_KILL = 9;   // 强制终止
    static const int SIG_USR1 = 10;   // 用户定义 1
    static const int SIG_USR2 = 12;   // 用户定义 2
    static const int SIG_SEGV = 11;  // 段错误
    static const int SIG_PIPE = 13;  // 管道破裂
    static const int SIG_ALRM = 14;  // 定时器
    static const int SIG_TERM = 15;  // 终止
    static const int SIG_CHLD = 17;  // 子进程状态改变
    static const int SIG_CONT = 18;  // 继续执行
    static const int SIG_STOP = 19;  // 暂停执行
    static const int SIG_TSTP = 20;  // 终端暂停（Ctrl+Z）
    
    // 信号名称
    static std::string get_signal_name(int signal);
    
private:
    static void signal_handler_wrapper(int signal);
    
    std::unordered_map<int, signal_callback> callbacks_;
    threading::mutex callbacks_mutex_;
    static signal_handler* instance_;
};

// 信号集
class signal_set {
public:
    signal_set();
    ~signal_set();
    
    void add(int signal);
    void remove(int signal);
    void clear();
    
    bool contains(int signal) const;
    size_t size() const;
    
    // 阻塞等待
    int wait();
    bool wait(int timeout_ms, int& signal);
    
    // sigprocmask
    void block();
    void unblock();
    bool is_blocked(int signal) const;
    
private:
    sigset_t set_;
};

} // namespace process
} // namespace zen
