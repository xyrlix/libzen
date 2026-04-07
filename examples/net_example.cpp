#include <zen/net.h>
#include <iostream>
#include <thread>

using namespace zen;
using namespace zen::net;

// TCP Echo 服务器示例
void tcp_server_example() {
    event::event_loop loop;
    tcp_server server(&loop);
    
    server.set_connection_callback([](const tcp_connection_ptr& conn) {
        std::cout << "New connection from " << conn->peer_address().ip() 
                  << ":" << conn->peer_address().port() << std::endl;
    });
    
    server.set_message_callback([](const tcp_connection_ptr& conn, 
                                   zen::net::buffer* buf) {
        std::string data(buf->read_ptr(), buf->readable_bytes());
        std::cout << "Received: " << data << std::endl;
        conn->send("Echo: " + data);
    });
    
    if (server.start("0.0.0.0", 8888)) {
        std::cout << "TCP Server listening on port 8888" << std::endl;
        loop.run();
    }
}

// TCP 客户端示例
void tcp_client_example() {
    event::event_loop loop;
    tcp_client client(&loop);
    
    client.set_connect_callback([](bool success) {
        if (success) {
            std::cout << "Connected to server!" << std::endl;
        } else {
            std::cout << "Connection failed!" << std::endl;
        }
    });
    
    client.set_message_callback([](const tcp_connection_ptr& conn, 
                                   zen::net::buffer* buf) {
        std::string data(buf->read_ptr(), buf->readable_bytes());
        std::cout << "Server response: " << data << std::endl;
        conn->send("Hello again!");
    });
    
    client.connect("127.0.0.1", 8888);
    
    // 等待连接建立后发送消息
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if (client.is_connected()) {
        client.send("Hello Server!");
    }
    
    loop.run();
}

int main() {
    std::cout << "Choose mode:" << std::endl;
    std::cout << "1. TCP Server" << std::endl;
    std::cout << "2. TCP Client" << std::endl;
    
    int choice;
    std::cin >> choice;
    
    if (choice == 1) {
        tcp_server_example();
    } else if (choice == 2) {
        tcp_client_example();
    }
    
    return 0;
}
