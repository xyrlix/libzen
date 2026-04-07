#include <gtest/gtest.h>
#include <zen/timer.h>
#include <thread>
#include <chrono>

using namespace zen;

TEST(TimerQueueTest, PushPop) {
    timer_queue queue;
    
    int fired = 0;
    timer_callback_t cb = [&]() { fired++; };
    
    timer_id id1 = queue.push(50, cb);
    timer_id id2 = queue.push(30, cb);
    timer_id id3 = queue.push(40, cb);
    
    EXPECT_EQ(queue.next_expire(), 30);
    
    auto expired = queue.pop_expired(30);
    EXPECT_EQ(expired.size(), 1);
    EXPECT_EQ(expired[0], id2);
}

TEST(TimerQueueTest, Order) {
    timer_queue queue;
    
    std::vector<timer_id> ids;
    timer_callback_t cb = [&]() {};
    
    ids.push_back(queue.push(50, cb));
    ids.push_back(queue.push(10, cb));
    ids.push_back(queue.push(30, cb));
    ids.push_back(queue.push(20, cb));
    
    EXPECT_EQ(queue.next_expire(), 10);
    
    auto expired = queue.pop_expired(20);
    EXPECT_EQ(expired.size(), 2);
    EXPECT_EQ(expired[0], ids[1]);
    EXPECT_EQ(expired[1], ids[3]);
}

TEST(TimerManagerTest, Once) {
    timer_manager mgr;
    
    bool fired = false;
    timer_id id = mgr.add_once(50, [&]() { fired = true; });
    
    EXPECT_FALSE(fired);
    
    mgr.tick();
    EXPECT_FALSE(fired);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(60));
    mgr.tick();
    
    EXPECT_TRUE(fired);
}

TEST(TimerManagerTest, Repeat) {
    timer_manager mgr;
    
    int count = 0;
    mgr.add_repeat(50, [&]() { count++; });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(160));
    mgr.tick();
    
    EXPECT_GE(count, 3);
}

TEST(TimerManagerTest, Cancel) {
    timer_manager mgr;
    
    bool fired = false;
    timer_id id = mgr.add_once(50, [&]() { fired = true; });
    
    mgr.cancel(id);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(60));
    mgr.tick();
    
    EXPECT_FALSE(fired);
}

TEST(TimerManagerTest, TickUntilEmpty) {
    timer_manager mgr;
    
    int count = 0;
    mgr.add_once(10, [&]() { count++; });
    mgr.add_once(20, [&]() { count++; });
    mgr.add_once(30, [&]() { count++; });
    
    mgr.tick_until_empty(5, 100);
    
    EXPECT_EQ(count, 3);
}
