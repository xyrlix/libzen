#include <zen/timer.h>
#include <iostream>
#include <thread>
#include <chrono>

using namespace zen;

void basic_timer_example() {
    std::cout << "=== Basic Timer Example ===" << std::endl;
    
    timer_manager mgr;
    
    // 添加一次性定时器
    bool fired1 = false;
    timer_id id1 = mgr.add_once(500, [&]() {
        fired1 = true;
        std::cout << "Once timer fired!" << std::endl;
    });
    
    // 添加重复定时器
    int count = 0;
    timer_id id2 = mgr.add_repeat(200, [&]() {
        count++;
        std::cout << "Repeat timer fired (" << count << "/3)" << std::endl;
    });
    
    // 运行定时器
    auto start = std::chrono::steady_clock::now();
    while (count < 3) {
        mgr.tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    auto end = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Elapsed: " << elapsed.count() << " ms" << std::endl;
    std::cout << "Once timer fired: " << (fired1 ? "true" : "false") << std::endl;
    
    std::cout << std::endl;
}

void timer_queue_example() {
    std::cout << "=== Timer Queue Example ===" << std::endl;
    
    timer_queue queue;
    
    int count = 0;
    timer_callback_t cb = [&]() { count++; };
    
    // 添加多个定时器
    queue.push(300, cb);
    queue.push(100, cb);
    queue.push(200, cb);
    
    std::cout << "Next expire: " << queue.next_expire() << " ms" << std::endl;
    
    // 弹出到期的定时器
    auto expired = queue.pop_expired(100);
    std::cout << "Expired at 100ms: " << expired.size() << " timers" << std::endl;
    
    expired = queue.pop_expired(200);
    std::cout << "Expired at 200ms: " << expired.size() << " timers" << std::endl;
    
    expired = queue.pop_expired(300);
    std::cout << "Expired at 300ms: " << expired.size() << " timers" << std::endl;
    
    std::cout << std::endl;
}

void timer_cancel_example() {
    std::cout << "=== Timer Cancel Example ===" << std::endl;
    
    timer_manager mgr;
    
    bool fired1 = false;
    bool fired2 = false;
    
    timer_id id1 = mgr.add_once(100, [&]() { fired1 = true; });
    timer_id id2 = mgr.add_once(200, [&]() { fired2 = true; });
    
    // 取消 id1
    mgr.cancel(id1);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    mgr.tick();
    
    std::cout << "Timer 1 fired: " << (fired1 ? "true" : "false") << std::endl;
    std::cout << "Timer 2 fired: " << (fired2 ? "true" : "false") << std::endl;
    
    std::cout << std::endl;
}

void tick_until_empty_example() {
    std::cout << "=== Tick Until Empty Example ===" << std::endl;
    
    timer_manager mgr;
    
    int count = 0;
    mgr.add_once(50, [&]() { count++; std::cout << "Timer 1 fired" << std::endl; });
    mgr.add_once(100, [&]() { count++; std::cout << "Timer 2 fired" << std::endl; });
    mgr.add_once(150, [&]() { count++; std::cout << "Timer 3 fired" << std::endl; });
    
    std::cout << "Running timers..." << std::endl;
    mgr.tick_until_empty(10, 500);
    
    std::cout << "All timers fired: " << count << std::endl;
    
    std::cout << std::endl;
}

void high_frequency_example() {
    std::cout << "=== High Frequency Timer Example ===" << std::endl;
    
    timer_manager mgr;
    
    int count = 0;
    // 1ms 间隔的定时器
    mgr.add_repeat(1, [&]() { count++; });
    
    auto start = std::chrono::steady_clock::now();
    
    // 运行 100ms
    for (int i = 0; i < 100; ++i) {
        mgr.tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    auto end = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Elapsed: " << elapsed.count() << " ms" << std::endl;
    std::cout << "Fired: " << count << " times" << std::endl;
    std::cout << "Frequency: " << (count * 1000.0 / elapsed.count()) << " Hz" << std::endl;
    
    std::cout << std::endl;
}

int main() {
    basic_timer_example();
    timer_queue_example();
    timer_cancel_example();
    tick_until_empty_example();
    high_frequency_example();
    
    return 0;
}
