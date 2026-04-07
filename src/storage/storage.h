#pragma once

#include <string>
#include <vector>
#include <functional>

namespace zen {
namespace storage {

// 文件存储
class file_storage {
public:
    // 写入文件
    static bool write_file(const std::string& path, const std::string& content);
    static bool write_file(const std::string& path, const std::vector<uint8_t>& data);
    static bool write_file(const std::string& path, const void* data, size_t len);
    
    // 读取文件
    static std::string read_file_string(const std::string& path);
    static std::vector<uint8_t> read_file_bytes(const std::string& path);
    static bool read_file(const std::string& path, void* buffer, size_t len);
    
    // 追加文件
    static bool append_file(const std::string& path, const std::string& content);
    static bool append_file(const std::string& path, const std::vector<uint8_t>& data);
    
    // 文件信息
    static bool exists(const std::string& path);
    static size_t get_size(const std::string& path);
    static uint64_t get_modified_time(const std::string& path);
    
    // 文件操作
    static bool remove(const std::string& path);
    static bool rename(const std::string& old_path, const std::string& new_path);
    static bool copy(const std::string& src, const std::string& dst);
    
    // 目录操作
    static bool create_directory(const std::string& path);
    static bool create_directories(const std::string& path);
    static bool remove_directory(const std::string& path);
    static bool remove_all(const std::string& path);
    
    // 文件遍历
    static std::vector<std::string> list_files(const std::string& path);
    static std::vector<std::string> list_files_recursive(const std::string& path);
};

// 键值存储
class kv_store {
public:
    kv_store();
    explicit kv_store(const std::string& db_path);
    ~kv_store();
    
    // 打开/关闭
    bool open(const std::string& db_path);
    void close();
    bool is_open() const;
    
    // 键值操作
    bool put(const std::string& key, const std::string& value);
    bool put(const std::string& key, const std::vector<uint8_t>& value);
    bool get(const std::string& key, std::string& value);
    bool get(const std::string& key, std::vector<uint8_t>& value);
    bool remove(const std::string& key);
    bool exists(const std::string& key) const;
    
    // 批量操作
    bool put_batch(const std::unordered_map<std::string, std::string>& items);
    bool remove_batch(const std::vector<std::string>& keys);
    
    // 遍历
    using iterator_callback = std::function<bool(const std::string& key, const std::string& value)>;
    void for_each(iterator_callback callback);
    
    // 前缀搜索
    std::vector<std::string> search_prefix(const std::string& prefix) const;
    
    // 持久化
    bool flush();
    bool compact();
    
private:
    class impl;
    std::unique_ptr<impl> impl_;
};

// 对象存储
class object_storage {
public:
    // 对象元数据
    struct object_metadata {
        std::string key;
        size_t size;
        std::string content_type;
        std::string etag;
        uint64_t last_modified;
        std::unordered_map<std::string, std::string> user_metadata;
    };
    
    object_storage();
    ~object_storage();
    
    // 存储桶操作
    bool create_bucket(const std::string& bucket_name);
    bool delete_bucket(const std::string& bucket_name);
    bool bucket_exists(const std::string& bucket_name) const;
    std::vector<std::string> list_buckets() const;
    
    // 对象操作
    bool put_object(const std::string& bucket, const std::string& key,
                     const std::vector<uint8_t>& data,
                     const std::unordered_map<std::string, std::string>& metadata = {});
    bool get_object(const std::string& bucket, const std::string& key,
                     std::vector<uint8_t>& data);
    bool delete_object(const std::string& bucket, const std::string& key);
    bool object_exists(const std::string& bucket, const std::string& key) const;
    
    // 元数据
    object_metadata get_object_metadata(const std::string& bucket, const std::string& key) const;
    
    // 列表
    std::vector<std::string> list_objects(const std::string& bucket,
                                            const std::string& prefix = "",
                                            size_t limit = 1000) const;
    
    // 复制/移动
    bool copy_object(const std::string& src_bucket, const std::string& src_key,
                      const std::string& dst_bucket, const std::string& dst_key);
    bool move_object(const std::string& src_bucket, const std::string& src_key,
                      const std::string& dst_bucket, const std::string& dst_key);
    
    // URL（预签名）
    std::string get_presigned_url(const std::string& bucket, const std::string& key,
                                   int expires_seconds = 3600);
    
private:
    class impl;
    std::unique_ptr<impl> impl_;
};

} // namespace storage
} // namespace zen
