#pragma once

#include <string>
#include <functional>
#include <memory>

namespace zen {
namespace database {

// 数据库类型
enum class db_type {
    mysql,
    postgresql,
    sqlite,
    oracle,
    mssql,
    mongodb
};

// 数据库连接配置
struct db_config {
    db_type type;
    std::string host;
    uint16_t port;
    std::string database;
    std::string username;
    std::string password;
    std::string charset;
    int timeout;
    bool auto_reconnect;
    int max_retries;
};

// 数据库异常
class db_exception : public std::runtime_error {
public:
    explicit db_exception(const std::string& msg) : std::runtime_error(msg) {}
};

// 数据库连接基类
class db_connection {
public:
    db_connection() = default;
    virtual ~db_connection() = default;
    
    // 连接/断开
    virtual bool connect(const db_config& config) = 0;
    virtual void disconnect() = 0;
    virtual bool is_connected() const = 0;
    virtual void ping() = 0;
    
    // 事务
    virtual void begin_transaction() = 0;
    virtual void commit() = 0;
    virtual void rollback() = 0;
    
    // 执行 SQL
    virtual bool execute(const std::string& sql) = 0;
    virtual size_t get_affected_rows() = 0;
    
    // 查询
    virtual std::unique_ptr<class db_result> query(const std::string& sql) = 0;
    
    // 获取信息
    virtual std::string get_last_error() const = 0;
    virtual uint64_t get_last_insert_id() = 0;
    
    // 转义
    virtual std::string escape(const std::string& str) = 0;
};

// 查询结果集
class db_result {
public:
    virtual ~db_result() = default;
    
    // 遍历
    virtual bool next() = 0;
    virtual void reset() = 0;
    virtual size_t get_row_count() = 0;
    virtual size_t get_column_count() = 0;
    
    // 获取列
    virtual std::vector<std::string> get_columns() const = 0;
    
    // 获取值
    virtual std::string get_string(size_t index) = 0;
    virtual int get_int(size_t index) = 0;
    virtual int64_t get_int64(size_t index) = 0;
    virtual double get_double(size_t index) = 0;
    virtual bool is_null(size_t index) = 0;
    
    // 按列名获取
    std::string get_string(const std::string& name);
    int get_int(const std::string& name);
    int64_t get_int64(const std::string& name);
    double get_double(const std::string& name);
};

// 连接工厂
class db_connection_factory {
public:
    static std::unique_ptr<db_connection> create_connection(db_type type);
};

} // namespace database
} // namespace zen
