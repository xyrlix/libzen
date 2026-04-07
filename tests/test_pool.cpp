#include <gtest/gtest.h>
#include <zen/pool.h>
#include <thread>

using namespace zen;

struct test_connection {
    int fd;
    bool valid;
    
    test_connection() : fd(-1), valid(false) {}
    test_connection(int f) : fd(f), valid(true) {}
    
    bool is_valid() const { return valid; }
    void reconnect() { valid = true; }
    void close() { valid = false; fd = -1; }
};

TEST(ObjectPoolTest, AcquireRelease) {
    object_pool<test_connection> pool(3);
    
    auto conn1 = pool.acquire();
    EXPECT_NE(conn1.get(), nullptr);
    EXPECT_EQ(pool.size(), 2);
    
    auto conn2 = pool.acquire();
    EXPECT_NE(conn2.get(), nullptr);
    EXPECT_EQ(pool.size(), 1);
    
    {
        auto conn3 = pool.acquire();
        EXPECT_NE(conn3.get(), nullptr);
        EXPECT_EQ(pool.size(), 0);
    }
    
    EXPECT_EQ(pool.size(), 1);
}

TEST(ObjectPoolTest, Exhaustion) {
    object_pool<test_connection> pool(2);
    
    auto conn1 = pool.acquire();
    auto conn2 = pool.acquire();
    auto conn3 = pool.try_acquire();
    
    EXPECT_NE(conn1.get(), nullptr);
    EXPECT_NE(conn2.get(), nullptr);
    EXPECT_EQ(conn3.get(), nullptr);
}
