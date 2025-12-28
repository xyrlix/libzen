#include <zen/net/tcp/tcp_server.h>
#include <zen/logging.h>
#include <string>

int main() {
    zen::logger& logger = zen::logger::get_instance();
    
    try {
        // Create a TCP server listening on port 8080
        zen::tcp_server server(8080);
        
        // Set connection handler
        server.set_connection_handler([&](zen::tcp_connection& conn) {
            logger.info("New connection from {}", conn.remote_address());
            
            // Set message handler
            conn.set_message_handler([&](const char* data, size_t len) {
                std::string message(data, len);
                logger.info("Received: {}", message);
                
                // Echo back the message
                conn.send(message.data(), message.size());
            });
            
            // Set disconnection handler
            conn.set_disconnect_handler([&]() {
                logger.info("Connection closed");
            });
        });
        
        logger.info("Server started on port 8080");
        logger.info("Press Ctrl+C to stop");
        
        // Start the server
        server.start();
        
    } catch (const std::exception& e) {
        logger.error("Error: {}", e.what());
        return 1;
    }
    
    return 0;
}