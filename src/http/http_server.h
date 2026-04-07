#pragma once

#include "http_message.h"
#include "../net/tcp/tcp_server.h"

#include <functional>
#include <memory>

namespace zen {
namespace http {

// HTTP 请求处理器
using request_handler = std::function<http_response(const http_request&)>;
using request_handler_async = std::function<void(const http_request&, 
                                                   std::function<void(http_response)>)>;
using middleware_handler = std::function<bool(const http_request&, http_response&)>;

// HTTP 服务器
class http_server {
public:
    http_server();
    ~http_server();
    
    // 启动服务器
    bool start(uint16_t port);
    bool start(const std::string& ip, uint16_t port);
    void stop();
    
    // 运行事件循环
    void run();
    void run_in_thread();
    void stop_loop();
    
    // 路由注册
    void get(const std::string& path, request_handler handler);
    void post(const std::string& path, request_handler handler);
    void put(const std::string& path, request_handler handler);
    void delete_(const std::string& path, request_handler handler);
    void patch(const std::string& path, request_handler handler);
    void head(const std::string& path, request_handler handler);
    void options(const std::string& path, request_handler handler);
    
    // 通用路由
    void route(method m, const std::string& path, request_handler handler);
    void any(const std::string& path, request_handler handler);
    
    // 中间件
    void use(middleware_handler middleware);
    void use(const std::string& path, middleware_handler middleware);
    
    // 静态文件服务
    void serve_static(const std::string& mount_path, const std::string& directory);
    void enable_directory_listing(bool enable);
    
    // 服务器配置
    void set_max_connections(size_t max);
    void set_timeout(int timeout_ms);
    void set_max_body_size(size_t max_size);
    
    // Session
    void set_session_secret(const std::string& secret);
    void set_session_timeout(int timeout_seconds);
    
    // 模板引擎
    void set_template_engine(std::function<std::string(const std::string&, 
                                                        const std::unordered_map<std::string, std::string>&)> engine);
    
    // 获取信息
    std::string get_ip() const { return server_ip_; }
    uint16_t get_port() const { return port_; }
    bool is_running() const;
    
    // WebSocket 支持
    void enable_websocket(bool enable = true);
    
private:
    struct route_info {
        std::string pattern;
        request_handler handler;
        middleware_handler middleware;
    };
    
    struct static_route_info {
        std::string mount_path;
        std::string directory;
        bool list_directory;
    };
    
    void on_new_connection(net::tcp_connection::ptr conn);
    void on_request(net::tcp_connection::ptr conn, const http_request& req);
    
    http_response handle_request(const http_request& req);
    http_response route_request(const http_request& req);
    
    bool match_route(const std::string& pattern, const std::string& path,
                      std::unordered_map<std::string, std::string>& params);
    
    std::string serve_static_file(const std::string& path);
    
    // 会话管理
    std::string create_session(const http_request& req);
    std::string get_session_id(const http_request& req);
    std::unordered_map<std::string, std::string> get_session_data(const std::string& session_id);
    void set_session_data(const std::string& session_id, const std::unordered_map<std::string, std::string>& data);
    
    std::string server_ip_;
    uint16_t port_;
    bool running_;
    
    // TCP 服务器
    std::unique_ptr<net::tcp_server> tcp_server_;
    
    // 路由
    std::unordered_map<method, std::vector<route_info>> routes_;
    middleware_handler global_middleware_;
    
    // 静态文件
    std::vector<static_route_info> static_routes_;
    bool enable_directory_listing_;
    
    // 配置
    size_t max_connections_;
    int timeout_;
    size_t max_body_size_;
    
    // Session
    std::string session_secret_;
    int session_timeout_;
    std::unordered_map<std::string, 
        std::pair<std::unordered_map<std::string, std::string>, uint64_t>> sessions_;
    threading::mutex sessions_mutex_;
    
    // 模板引擎
    std::function<std::string(const std::string&, 
                               const std::unordered_map<std::string, std::string>&)> template_engine_;
    
    // WebSocket
    bool enable_websocket_;
    
    mutable threading::mutex server_mutex_;
};

// Web 框架（便捷封装）
class web_app {
public:
    web_app();
    
    // 路由
    void get(const std::string& path, request_handler handler);
    void post(const std::string& path, request_handler handler);
    void put(const std::string& path, request_handler handler);
    void delete_(const std::string& path, request_handler handler);
    
    // JSON 响应
    static http_response json_response(const std::string& json_str);
    static http_response error_response(int code, const std::string& message);
    
    // 启动
    void run(uint16_t port);
    
    // 中间件
    void use(middleware_handler middleware);
    
    // 静态文件
    void static_files(const std::string& path, const std::string& directory);
    
    // 获取服务器
    http_server& get_server() { return server_; }
    
private:
    http_server server_;
};

} // namespace http
} // namespace zen
