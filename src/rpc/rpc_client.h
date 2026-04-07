#pragma once

#include "rpc_server.h"

#include <string>
#include <functional>
#include <memory>

namespace zen {
namespace rpc {

// RPC 客户端
class rpc_client {
public:
    using async_callback = std::function<void(const rpc_response&)>;
    using error_callback = std::function<void(const std::string&)>;
    
    rpc_client();
    ~rpc_client();
    
    // 连接
    bool connect_tcp(const std::string& host, uint16_t port);
    bool connect_http(const std::string& url);
    void disconnect();
    
    // 同步调用
    rpc_response call(const std::string& method, const std::string& params_json = "[]");
    rpc_response call(const std::string& method, 
                        const std::unordered_map<std::string, std::string>& params);
    
    // 异步调用
    void call_async(const std::string& method, const std::string& params_json,
                     async_callback callback, error_callback error_cb = nullptr);
    
    // 通知（无返回值）
    void notify(const std::string& method, const std::string& params_json = "[]");
    
    // 批量调用
    std::vector<rpc_response> call_batch(const std::vector<rpc_request>& requests);
    
    // 超时设置
    void set_timeout(int timeout_ms);
    
    // 认证
    void set_auth(const std::string& token);
    
private:
    std::string build_request(const std::string& method, const std::string& params,
                               bool is_notification = false);
    rpc_response parse_response(const std::string& json);
    
    std::string endpoint_;
    bool is_http_;
    int timeout_ms_;
    std::string auth_token_;
    
    // HTTP 客户端
    std::unique_ptr<http::http_client> http_client_;
    
    // TCP 连接
    std::unique_ptr<net::tcp_client> tcp_client_;
};

// RPC 客户端代理（类型安全）
template<typename Interface>
class rpc_client_proxy {
public:
    rpc_client_proxy(rpc_client* client) : client_(client) {}
    
    template<typename... Args>
    auto call(const std::string& method, Args... args) {
        // 实现类型安全的 RPC 调用
        return client_->call(method, /* 序列化参数 */);
    }
    
private:
    rpc_client* client_;
};

} // namespace rpc
} // namespace zen
