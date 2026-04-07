#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace zen {
namespace http {

// HTTP 版本
enum class version {
    http_1_0,
    http_1_1,
    http_2_0
};

// HTTP 方法
enum class method {
    get,
    post,
    put,
    delete_,
    head,
    options,
    patch,
    connect,
    trace
};

// HTTP 状态码
enum class status_code {
    continue_ = 100,
    switching_protocols = 101,
    
    ok = 200,
    created = 201,
    accepted = 202,
    non_authoritative_information = 203,
    no_content = 204,
    reset_content = 205,
    partial_content = 206,
    
    multiple_choices = 300,
    moved_permanently = 301,
    found = 302,
    see_other = 303,
    not_modified = 304,
    use_proxy = 305,
    temporary_redirect = 307,
    permanent_redirect = 308,
    
    bad_request = 400,
    unauthorized = 401,
    payment_required = 402,
    forbidden = 403,
    not_found = 404,
    method_not_allowed = 405,
    not_acceptable = 406,
    proxy_authentication_required = 407,
    request_timeout = 408,
    conflict = 409,
    gone = 410,
    length_required = 411,
    precondition_failed = 412,
    payload_too_large = 413,
    uri_too_long = 414,
    unsupported_media_type = 415,
    range_not_satisfiable = 416,
    expectation_failed = 417,
    
    internal_server_error = 500,
    not_implemented = 501,
    bad_gateway = 502,
    service_unavailable = 503,
    gateway_timeout = 504,
    http_version_not_supported = 505
};

// HTTP 消息基类
class http_message {
public:
    http_message() = default;
    virtual ~http_message() = default;
    
    // 头部操作
    void set_header(const std::string& name, const std::string& value);
    std::string get_header(const std::string& name, const std::string& default_val = "") const;
    bool has_header(const std::string& name) const;
    void remove_header(const std::string& name);
    void clear_headers();
    std::vector<std::string> get_header_names() const;
    
    // Body 操作
    void set_body(const std::string& body);
    void set_body(const std::vector<uint8_t>& body);
    std::string get_body_string() const;
    const std::vector<uint8_t>& get_body() const { return body_; }
    void clear_body();
    
    // 版本
    version get_version() const { return version_; }
    void set_version(version v) { version_ = v; }
    std::string get_version_string() const;
    
    // Content-Length
    size_t get_content_length() const;
    
protected:
    version version_;
    std::unordered_map<std::string, std::string> headers_;
    std::vector<uint8_t> body_;
    
    static std::string to_lower(const std::string& str);
    static method parse_method(const std::string& str);
    static std::string method_to_string(method m);
    static status_code parse_status_code(int code);
    static int status_code_to_int(status_code code);
    static std::string status_code_to_string(status_code code);
};

// HTTP 请求
class http_request : public http_message {
public:
    http_request() = default;
    http_request(method m, const std::string& url);
    
    // 请求行
    method get_method() const { return method_; }
    void set_method(method m) { method_ = m; }
    
    std::string get_url() const { return url_; }
    void set_url(const std::string& url);
    
    std::string get_path() const;
    std::string get_query_string() const;
    std::unordered_map<std::string, std::string> get_query_params() const;
    
    // Cookie
    std::string get_cookie(const std::string& name) const;
    void set_cookie(const std::string& name, const std::string& value);
    
    // 转换为字符串
    std::string to_string() const;
    
    // 解析
    static http_request parse(const std::string& raw);
    static http_request parse(const char* data, size_t len);
    
private:
    method method_;
    std::string url_;
};

// HTTP 响应
class http_response : public http_message {
public:
    http_response() = default;
    http_response(status_code code);
    
    // 状态行
    status_code get_status_code() const { return status_code_; }
    void set_status_code(status_code code) { status_code_ = code; }
    
    std::string get_status_message() const;
    
    // Cookie
    std::vector<std::string> get_set_cookie() const;
    void add_set_cookie(const std::string& cookie);
    
    // 转换为字符串
    std::string to_string() const;
    
    // 解析
    static http_response parse(const std::string& raw);
    static http_response parse(const char* data, size_t len);
    
    // 便捷构建
    static http_response ok(const std::string& body = "");
    static http_response not_found(const std::string& body = "");
    static http_response bad_request(const std::string& body = "");
    static http_response internal_server_error(const std::string& body = "");
    static http_response json(const std::string& json_str);
    static http_response redirect(const std::string& location);
    
private:
    status_code status_code_;
};

} // namespace http
} // namespace zen
