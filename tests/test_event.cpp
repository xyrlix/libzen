#include <gtest/gtest.h>
#include <zen/event.h>
#include <thread>
#include <chrono>

using namespace zen;
using namespace zen::event;

TEST(EventLoopTest, StartStop) {
    event_loop loop;
    
    std::thread t([&]() {
        loop.run();
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    loop.stop();
    
    t.join();
}

TEST(EventLoopTest, Timer) {
    event_loop loop;
    
    bool fired = false;
    loop.add_timer(100, [&]() {
        fired = true;
        loop.stop();
    });
    
    loop.run();
    
    EXPECT_TRUE(fired);
}

TEST(EventLoopTest, RepeatTimer) {
    event_loop loop;
    
    int count = 0;
    loop.add_repeat(50, [&]() {
        count++;
        if (count >= 3) {
            loop.stop();
        }
    });
    
    loop.run();
    
    EXPECT_EQ(count, 3);
}

TEST(EventLoopTest, TimerCancel) {
    event_loop loop;
    
    bool fired = false;
    auto handler = loop.add_timer(100, [&]() {
        fired = true;
    });
    
    loop.cancel_timer(handler);
    
    loop.add_timer(50, [&]() {
        loop.stop();
    });
    
    loop.run();
    
    EXPECT_FALSE(fired);
}
