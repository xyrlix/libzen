#pragma once

#include <string>
#include <functional>
#include <memory>
#include <unordered_map>

namespace zen {
namespace rpc {

// RPC 调用结果
enum class rpc_status {
    success,
    method_not_found,
    invalid_params,
    internal_error,
    timeout,
    network_error
};

// RPC 请求
struct rpc_request {
    std::string id;
    std::string method;
    std::string params_json;
    bool is_notification;  // 无返回值
};

// RPC 响应
struct rpc_response {
    std::string id;
    rpc_status status;
    std::string result_json;
    std::string error_message;
    int error_code;
};

// RPC 方法处理器
using rpc_method_handler = std::function<rpc_response(const rpc_request&)>;

// RPC 服务器
class rpc_server {
public:
    rpc_server();
    ~rpc_server();
    
    // 启动服务器
    bool start_tcp(uint16_t port);
    bool start_http(uint16_t port);
    void stop();
    
    // 注册方法
    void register_method(const std::string& name, rpc_method_handler handler);
    void unregister_method(const std::string& name);
    bool has_method(const std::string& name) const;
    
    // 中间件
    using middleware = std::function<bool(const rpc_request&, rpc_response&)>;
    void use(middleware m);
    
    // 批量处理
    std::vector<rpc_response> handle_batch(const std::vector<rpc_request>& requests);
    
    // 统计
    size_t get_call_count(const std::string& method) const;
    std::vector<std::string> get_methods() const;
    
private:
    rpc_response handle_request(const rpc_request& req);
    
    std::unordered_map<std::string, rpc_method_handler> methods_;
    std::vector<middleware> middlewares_;
    std::unordered_map<std::string, size_t> call_counts_;
    
    mutable threading::mutex server_mutex_;
};

// RPC 服务端（基于 HTTP）
class http_rpc_server {
public:
    http_rpc_server(uint16_t port);
    ~http_rpc_server();
    
    void run();
    void stop();
    
    // 注册方法
    void register_method(const std::string& name, rpc_method_handler handler);
    
private:
    http::http_server http_server_;
    rpc_server rpc_server_;
};

// 服务生成器（便捷宏）
#define ZEN_RPC_METHOD(server, name) \
    server.register_method(#name, \
        [](const rpc::rpc_request& req) -> rpc::rpc_response { \
            /* 实现 */ \
        })

} // namespace rpc
} // namespace zen
