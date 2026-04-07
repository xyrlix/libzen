#include <zen/event.h>
#include <iostream>
#include <thread>
#include <unistd.h>
#include <fcntl.h>

using namespace zen;
using namespace zen::event;

void io_event_example() {
    std::cout << "=== IO Event Example ===" << std::endl;
    
    event_loop loop;
    
    // 创建一个 pipe
    int pipefd[2];
    pipe(pipefd);
    
    // 设置读端为非阻塞
    fcntl(pipefd[0], F_SETFL, O_NONBLOCK);
    
    // 添加可读事件
    auto handler = loop.add_io_event(pipefd[0], io_event::read, 
        [pipefd](int fd, io_event events) {
            char buf[1024];
            ssize_t n = read(fd, buf, sizeof(buf));
            if (n > 0) {
                buf[n] = '\0';
                std::cout << "Received: " << buf << std::endl;
            }
        });
    
    // 启动一个线程写入数据
    std::thread writer([pipefd]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        write(pipefd[1], "Hello from thread!", 18);
    });
    
    // 运行事件循环
    loop.run();
    
    writer.join();
    close(pipefd[0]);
    close(pipefd[1]);
    
    std::cout << std::endl;
}

void timer_example() {
    std::cout << "=== Timer Example ===" << std::endl;
    
    event_loop loop;
    
    // 添加一次性定时器
    loop.add_timer(500, []() {
        std::cout << "Once timer fired!" << std::endl;
    });
    
    // 添加重复定时器
    int count = 0;
    loop.add_repeat_timer(1000, [&count]() {
        count++;
        std::cout << "Repeat timer fired (" << count << "/3)" << std::endl;
        if (count >= 3) {
            std::cout << "Stopping event loop..." << std::endl;
            // 注意：这里需要获取 loop 的引用来调用 stop()
        }
    });
    
    loop.run();
    
    std::cout << std::endl;
}

void combined_example() {
    std::cout << "=== Combined Example ===" << std::endl;
    
    event_loop loop;
    
    int counter = 0;
    
    // 定时器
    loop.add_repeat_timer(1000, [&]() {
        counter++;
        std::cout << "Tick " << counter << std::endl;
        if (counter >= 5) {
            loop.stop();
        }
    });
    
    // IO 事件
    int pipefd[2];
    pipe(pipefd);
    fcntl(pipefd[0], F_SETFL, O_NONBLOCK);
    
    loop.add_io_event(pipefd[0], io_event::read, 
        [pipefd](int fd, io_event events) {
            char buf[1024];
            ssize_t n = read(fd, buf, sizeof(buf));
            if (n > 0) {
                buf[n] = '\0';
                std::cout << "IO: " << buf << std::endl;
            }
        });
    
    std::thread writer([pipefd]() {
        for (int i = 1; i <= 3; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1500));
            std::string msg = "Message " + std::to_string(i);
            write(pipefd[1], msg.c_str(), msg.size());
        }
    });
    
    loop.run();
    
    writer.join();
    close(pipefd[0]);
    close(pipefd[1]);
    
    std::cout << std::endl;
}

int main() {
    io_event_example();
    timer_example();
    combined_example();
    
    return 0;
}
