#include <zen/config.h>
#include <iostream>

using namespace zen;
using namespace zen::config;

int main() {
    // 创建 INI 配置
    ini_config cfg;
    
    // 设置配置项
    cfg.set("server", "host", "127.0.0.1");
    cfg.set("server", "port", "8080");
    cfg.set("server", "debug", "true");
    cfg.set("server", "max_connections", "100");
    
    cfg.set("database", "type", "mysql");
    cfg.set("database", "host", "localhost");
    cfg.set("database", "port", "3306");
    cfg.set("database", "name", "testdb");
    cfg.set("database", "user", "admin");
    cfg.set("database", "password", "secret");
    
    // 保存配置
    cfg.save("/tmp/example_config.ini");
    std::cout << "Config saved to /tmp/example_config.ini" << std::endl;
    
    // 加载配置
    ini_config loaded;
    if (loaded.load("/tmp/example_config.ini")) {
        std::cout << "\nLoaded configuration:\n" << std::endl;
        
        // 读取服务器配置
        std::string host = loaded.get("server", "host").as_string();
        int port = loaded.get("server", "port").as_int();
        bool debug = loaded.get("server", "debug").as_bool();
        int max_conn = loaded.get("server", "max_connections").as_int();
        
        std::cout << "[server]" << std::endl;
        std::cout << "  host = " << host << std::endl;
        std::cout << "  port = " << port << std::endl;
        std::cout << "  debug = " << (debug ? "true" : "false") << std::endl;
        std::cout << "  max_connections = " << max_conn << std::endl;
        
        // 读取数据库配置
        std::string db_type = loaded.get("database", "type").as_string();
        std::string db_host = loaded.get("database", "host").as_string();
        int db_port = loaded.get("database", "port").as_int();
        std::string db_name = loaded.get("database", "name").as_string();
        
        std::cout << "\n[database]" << std::endl;
        std::cout << "  type = " << db_type << std::endl;
        std::cout << "  host = " << db_host << std::endl;
        std::cout << "  port = " << db_port << std::endl;
        std::cout << "  name = " << db_name << std::endl;
    }
    
    // 遍历所有 section 和 keys
    std::cout << "\nAll sections and keys:" << std::endl;
    for (const auto& section : loaded.sections()) {
        std::cout << "[" << section << "]" << std::endl;
        for (const auto& key : loaded.keys(section)) {
            std::string value = loaded.get(section, key).as_string();
            std::cout << "  " << key << " = " << value << std::endl;
        }
    }
    
    return 0;
}
