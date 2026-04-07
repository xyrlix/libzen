#pragma once

#include "proto_base.h"
#include "message.h"
#include <functional>
#include <memory>

namespace zen {
namespace proto {

// Protobuf 反射

// 消息工厂
using message_factory = std::function<std::unique_ptr<proto_message>()>;

// 枚举值描述符
struct enum_value_descriptor {
    std::string name;
    int32_t number;
};

// 枚举描述符
struct enum_descriptor {
    std::string name;
    std::string full_name;
    std::vector<enum_value_descriptor> values;
    
    int32_t get_value_by_name(const std::string& name) const;
    std::string get_name_by_value(int32_t number) const;
};

// 消息描述符
struct message_descriptor {
    std::string name;
    std::string full_name;
    std::shared_ptr<message_descriptor> containing_type;
    
    std::unordered_map<field_number, field_descriptor> fields_by_number;
    std::unordered_map<std::string, field_descriptor*> fields_by_name;
    
    void add_field(const field_descriptor& field);
    const field_descriptor* find_field(field_number number) const;
    const field_descriptor* find_field(const std::string& name) const;
    
    // 创建消息
    std::unique_ptr<proto_message> new_message() const;
    void set_factory(message_factory factory);
    
private:
    message_factory factory_;
};

// 服务描述符（RPC）
struct service_descriptor {
    std::string name;
    std::string full_name;
    
    struct method_descriptor {
        std::string name;
        std::string full_name;
        const message_descriptor* input_type;
        const message_descriptor* output_type;
        bool client_streaming;
        bool server_streaming;
    };
    
    std::unordered_map<std::string, method_descriptor> methods;
    
    const method_descriptor* find_method(const std::string& name) const;
};

// 描述符池
class descriptor_pool {
public:
    descriptor_pool();
    ~descriptor_pool() = default;
    
    // 注册消息
    void add_message(const message_descriptor& descriptor);
    void add_enum(const enum_descriptor& descriptor);
    void add_service(const service_descriptor& descriptor);
    
    // 查找
    const message_descriptor* find_message(const std::string& full_name) const;
    const enum_descriptor* find_enum(const std::string& full_name) const;
    const service_descriptor* find_service(const std::string& full_name) const;
    
    // 创建消息
    std::unique_ptr<proto_message> new_message(const std::string& full_name) const;
    
    // 列表
    std::vector<const message_descriptor*> get_messages() const;
    std::vector<const enum_descriptor*> get_enums() const;
    std::vector<const service_descriptor*> get_services() const;
    
private:
    std::unordered_map<std::string, std::unique_ptr<message_descriptor>> messages_;
    std::unordered_map<std::string, std::unique_ptr<enum_descriptor>> enums_;
    std::unordered_map<std::string, std::unique_ptr<service_descriptor>> services_;
    
    mutable threading::mutex pool_mutex_;
};

// 全局描述符池
descriptor_pool& get_global_descriptor_pool();

// 辅助宏（简化消息定义）
#define ZEN_PROTO_MESSAGE(name) \
    public: \
        std::string get_message_name() const override { return #name; } \
        static std::string static_message_name() { return #name; }

#define ZEN_PROTO_FIELD(type, number, name) \
    type name##_() const { return name##_; } \
    void set_##name(type value) { name##_ = value; set_field_present(number); } \
    bool has_##name() const { return has_field(number); } \
    void clear_##name() { clear_field_present(number); name##_ = type{}; }

#define ZEN_PROTO_REPEATED(type, number, name) \
    const std::vector<type>& name() const { return name##_; } \
    void add_##name(type value) { name##_.push_back(value); set_field_present(number); } \
    size_t name##_size() const { return name##_.size(); } \
    void clear_##name() { name##_.clear(); clear_field_present(number); }

} // namespace proto
} // namespace zen
