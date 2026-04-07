#pragma once

#include "http_message.h"
#include "../net/tcp/tcp_client.h"

#include <functional>
#include <memory>
#include <chrono>

namespace zen {
namespace http {

// HTTP 客户端
class http_client {
public:
    using response_callback = std::function<void(const http_response&)>;
    using error_callback = std::function<void(const std::string&)>;
    
    http_client();
    ~http_client();
    
    // 同步请求
    http_response get(const std::string& url, int timeout_ms = 5000);
    http_response post(const std::string& url, const std::string& data,
                        const std::string& content_type = "application/json",
                        int timeout_ms = 5000);
    http_response put(const std::string& url, const std::string& data,
                       const std::string& content_type = "application/json",
                       int timeout_ms = 5000);
    http_response delete_(const std::string& url, int timeout_ms = 5000);
    http_response head(const std::string& url, int timeout_ms = 5000);
    
    // 通用请求
    http_response request(const http_request& req, int timeout_ms = 5000);
    
    // 异步请求
    void async_get(const std::string& url, response_callback on_response,
                    error_callback on_error = nullptr);
    void async_post(const std::string& url, const std::string& data,
                     response_callback on_response, error_callback on_error = nullptr);
    void async_request(const http_request& req,
                        response_callback on_response,
                        error_callback on_error = nullptr);
    
    // 连接池
    void set_max_connections(size_t max);
    void set_connection_timeout(int timeout_ms);
    void set_read_timeout(int timeout_ms);
    
    // User Agent
    void set_user_agent(const std::string& ua);
    
    // 重定向
    void set_follow_redirects(bool follow);
    void set_max_redirects(int max);
    
    // Cookie
    void set_cookie(const std::string& name, const std::string& value);
    std::string get_cookie(const std::string& name) const;
    void clear_cookies();
    
    // 认证
    void set_basic_auth(const std::string& username, const std::string& password);
    void set_bearer_token(const std::string& token);
    
    // 代理
    void set_proxy(const std::string& proxy_url);
    void clear_proxy();
    
    // HTTPS（需要 SSL 支持）
    void set_verify_ssl(bool verify);
    void set_ca_path(const std::string& path);
    
private:
    // 解析 URL
    struct url_info {
        std::string protocol;
        std::string host;
        uint16_t port;
        std::string path;
        std::string query;
        bool is_https;
    };
    
    url_info parse_url(const std::string& url);
    
    // 构建 HTTP 请求
    std::string build_request(const http_request& req);
    
    // 解析 HTTP 响应
    http_response parse_response(const std::string& raw);
    
    // 连接管理
    class connection_pool {
    public:
        std::shared_ptr<net::tcp_client> acquire(const url_info& url);
        void release(const url_info& url, std::shared_ptr<net::tcp_client> conn);
        void cleanup();
    };
    
    size_t max_connections_;
    int connection_timeout_;
    int read_timeout_;
    
    std::string user_agent_;
    bool follow_redirects_;
    int max_redirects_;
    
    std::unordered_map<std::string, std::string> cookies_;
    
    std::string basic_auth_;
    std::string bearer_token_;
    
    std::string proxy_url_;
    bool verify_ssl_;
    std::string ca_path_;
    
    connection_pool pool_;
    
    mutable threading::mutex client_mutex_;
};

// REST 客户端（便捷封装）
class rest_client {
public:
    rest_client(const std::string& base_url);
    
    http_response get(const std::string& path);
    http_response post(const std::string& path, const std::string& data);
    http_response put(const std::string& path, const std::string& data);
    http_response delete_(const std::string& path);
    http_response patch(const std::string& path, const std::string& data);
    
    // JSON 便捷方法
    http_response get_json(const std::string& path);
    http_response post_json(const std::string& path, const std::string& json);
    http_response put_json(const std::string& path, const std::string& json);
    
    void set_header(const std::string& name, const std::string& value);
    void set_auth(const std::string& token);
    
private:
    std::string build_url(const std::string& path) const;
    
    std::string base_url_;
    http_client client_;
    http_request base_request_;
};

} // namespace http
} // namespace zen
