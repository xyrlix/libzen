#pragma once

#include "connection.h"
#include "../pool/connection_pool.h"

#include <memory>

namespace zen {
namespace database {

// 数据库连接（继承 pool::connection）
class db_pool_connection : public pool::connection<db_connection> {
public:
    explicit db_pool_connection(std::unique_ptr<db_connection> conn);
    
    bool is_valid() const override;
    bool reconnect() override;
    void close() override;
    
    // 转发到实际连接
    void begin_transaction();
    void commit();
    void rollback();
    
    bool execute(const std::string& sql);
    std::unique_ptr<db_result> query(const std::string& sql);
    
    uint64_t get_last_insert_id();
    
private:
    std::unique_ptr<db_connection> connection_;
};

// 数据库连接池工厂
class db_connection_pool_factory : public pool::connection_factory<db_pool_connection> {
public:
    explicit db_connection_pool_factory(const db_config& config);
    
    std::shared_ptr<db_pool_connection> create() override;
    
private:
    db_config config_;
};

// 数据库连接池
using db_connection_pool = pool::connection_pool<db_pool_connection>;
using db_connection_holder = pool::connection_holder<db_pool_connection>;

// 数据库连接池（便捷封装）
class database {
public:
    database(const db_config& config, size_t pool_size = 10);
    ~database();
    
    // 获取连接
    db_connection_holder acquire();
    db_connection_holder acquire(std::chrono::milliseconds timeout);
    
    // 执行查询（自动管理连接）
    std::unique_ptr<db_result> query(const std::string& sql);
    bool execute(const std::string& sql);
    
    // ORM 辅助
    template<typename T>
    std::vector<T> select(const std::string& table, const std::string& where = "");
    
    template<typename T>
    bool insert(const std::string& table, const T& obj);
    
    template<typename T>
    bool update(const std::string& table, const T& obj, const std::string& where);
    
    template<typename T>
    bool remove(const std::string& table, const std::string& where);
    
    // 获取连接池
    db_connection_pool& get_pool() { return *pool_; }
    
private:
    db_config config_;
    std::unique_ptr<db_connection_pool> pool_;
};

} // namespace database
} // namespace zen
