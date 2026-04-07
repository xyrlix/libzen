#pragma once

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <chrono>
#include <atomic>

namespace zen {
namespace monitoring {

// 监控指标

// 计数器
class counter {
public:
    counter(const std::string& name, const std::string& help = "");
    
    void increment(double delta = 1.0);
    double get_value() const;
    
    const std::string& get_name() const { return name_; }
    const std::string& get_help() const { return help_; }
    
    // 标签
    void with_label(const std::unordered_map<std::string, std::string>& labels);
    
private:
    std::string name_;
    std::string help_;
    std::atomic<double> value_;
    std::unordered_map<std::string, std::string> labels_;
};

// 仪表（Gauge）
class gauge {
public:
    gauge(const std::string& name, const std::string& help = "");
    
    void set(double value);
    void increment(double delta = 1.0);
    void decrement(double delta = 1.0);
    double get_value() const;
    
    const std::string& get_name() const { return name_; }
    
private:
    std::string name_;
    std::string help_;
    std::atomic<double> value_;
};

// 直方图（Histogram）
class histogram {
public:
    histogram(const std::string& name, const std::string& help = "",
               const std::vector<double>& buckets = {});
    
    void observe(double value);
    
    struct bucket_info {
        double upper_bound;
        double cumulative_count;
    };
    
    std::vector<bucket_info> get_buckets() const;
    double get_sum() const;
    double get_count() const;
    
    const std::string& get_name() const { return name_; }
    
private:
    std::string name_;
    std::string help_;
    std::vector<double> buckets_;
    std::vector<std::atomic<size_t>> bucket_counts_;
    std::atomic<double> sum_;
    std::atomic<size_t> count_;
};

// 摘要（Summary）
class summary {
public:
    summary(const std::string& name, const std::string& help = {},
             const std::vector<double>& quantiles = {0.5, 0.9, 0.99});
    
    void observe(double value);
    
    std::unordered_map<double, double> get_quantiles() const;
    double get_sum() const;
    double get_count() const;
    
    const std::string& get_name() const { return name_; }
    
private:
    std::string name_;
    std::string help_;
    std::vector<double> quantiles_;
    std::vector<double> samples_;
    std::atomic<double> sum_;
    std::atomic<size_t> count_;
    
    mutable threading::mutex samples_mutex_;
};

// 指标注册表
class metric_registry {
public:
    static metric_registry& get_default();
    
    // 注册指标
    void register_counter(const std::string& name, counter* counter);
    void register_gauge(const std::string& name, gauge* gauge);
    void register_histogram(const std::string& name, histogram* histogram);
    void register_summary(const std::string& name, summary* summary);
    
    // 获取指标
    counter* get_counter(const std::string& name) const;
    gauge* get_gauge(const std::string& name) const;
    histogram* get_histogram(const std::string& name) const;
    summary* get_summary(const std::string& name) const;
    
    // 导出
    std::string export_prometheus() const;
    std::string export_json() const;
    
    // 清除
    void clear();
    
    // 获取所有指标
    std::vector<std::string> get_metric_names() const;
    
private:
    std::unordered_map<std::string, counter*> counters_;
    std::unordered_map<std::string, gauge*> gauges_;
    std::unordered_map<std::string, histogram*> histograms_;
    std::unordered_map<std::string, summary*> summaries_;
    
    mutable threading::mutex registry_mutex_;
};

// 性能追踪器
class performance_tracer {
public:
    performance_tracer(const std::string& name);
    ~performance_tracer();
    
    void stop();
    double get_duration_ms() const;
    
    const std::string& get_name() const { return name_; }
    
private:
    std::string name_;
    std::chrono::steady_clock::time_point start_time_;
    bool stopped_;
    double duration_ms_;
};

// 性能分析器
class profiler {
public:
    struct sample {
        std::string name;
        double duration_ms;
        int64_t timestamp_ms;
    };
    
    profiler();
    ~profiler();
    
    void start();
    void stop();
    void reset();
    
    void add_sample(const sample& sample);
    
    std::vector<sample> get_samples() const;
    std::unordered_map<std::string, std::vector<sample>> get_samples_by_name() const;
    
    std::string generate_report() const;
    
private:
    std::vector<sample> samples_;
    bool running_;
    
    mutable threading::mutex profiler_mutex_;
};

// 日志监控
class log_monitor {
public:
    using log_handler = std::function<void(const std::string& level, const std::string& message)>;
    
    log_monitor();
    ~log_monitor();
    
    void start();
    void stop();
    
    void set_handler(log_handler handler);
    
    void alert_on_error(bool enable);
    void alert_on_warning(bool enable);
    void set_alert_threshold(size_t count, int64_t duration_ms);
    
private:
    std::thread monitor_thread_;
    std::atomic<bool> running_;
    log_handler handler_;
    bool alert_on_error_;
    bool alert_on_warning_;
    size_t alert_threshold_;
    int64_t alert_duration_ms_;
    
    void monitor_loop();
};

// 健康检查
class health_checker {
public:
    using check_function = std::function<bool()>;
    
    health_checker();
    ~health_checker();
    
    // 注册检查
    void register_check(const std::string& name, check_function check);
    
    // 执行检查
    bool check(const std::string& name);
    bool check_all();
    
    // 获取状态
    std::unordered_map<std::string, bool> get_status() const;
    std::string get_status_json() const;
    
    // 健康检查端点
    std::string get_http_response() const;
    
private:
    std::unordered_map<std::string, check_function> checks_;
    
    mutable threading::mutex checks_mutex_;
};

// 告警管理器
class alert_manager {
public:
    using alert_handler = std::function<void(const std::string& alert)>;
    
    alert_manager();
    ~alert_manager();
    
    void register_alert(const std::string& name, alert_handler handler);
    void trigger_alert(const std::string& name);
    
    void set_cooldown(int64_t cooldown_ms);
    
private:
    std::unordered_map<std::string, alert_handler> handlers_;
    std::unordered_map<std::string, int64_t> last_triggered_;
    int64_t cooldown_ms_;
    
    mutable threading::mutex alerts_mutex_;
};

} // namespace monitoring
} // namespace zen
