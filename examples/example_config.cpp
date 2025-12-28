#include <zen/config.h>
#include <zen/logging.h>

int main() {
    zen::logger& logger = zen::logger::get_instance();
    
    try {
        // Load INI configuration
        zen::ini_config config;
        
        // Set some configuration values
        config.set("server", "port", 8080);
        config.set("server", "host", "localhost");
        config.set("database", "name", "mydb");
        config.set("database", "username", "admin");
        config.set("database", "password", "secret");
        config.set("logging", "level", "info");
        
        // Get configuration values
        int port = config.get<int>("server", "port");
        std::string host = config.get<std::string>("server", "host");
        std::string db_name = config.get<std::string>("database", "name");
        std::string db_user = config.get<std::string>("database", "username");
        std::string log_level = config.get<std::string>("logging", "level");
        
        // Log the configuration
        logger.info("Server Configuration:");
        logger.info("  Host: {}", host);
        logger.info("  Port: {}", port);
        
        logger.info("Database Configuration:");
        logger.info("  Name: {}", db_name);
        logger.info("  Username: {}", db_user);
        
        logger.info("Logging Configuration:");
        logger.info("  Level: {}", log_level);
        
        // Get default value if key doesn't exist
        int timeout = config.get<int>("server", "timeout", 30);
        logger.info("  Timeout: {}", timeout);
        
    } catch (const std::exception& e) {
        logger.error("Error: {}", e.what());
        return 1;
    }
    
    return 0;
}