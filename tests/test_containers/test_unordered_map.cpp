#include <gtest/gtest.h>
#include "containers/associative/unordered_map.h"

TEST(UnorderedMapTest, BasicOperations) {
    // Test basic unordered_map operations
    zen::unordered_map<std::string, int> map;
    EXPECT_TRUE(map.empty());
    EXPECT_EQ(map.size(), 0);
    
    map["one"] = 1;
    map["two"] = 2;
    map["three"] = 3;
    
    EXPECT_FALSE(map.empty());
    EXPECT_EQ(map.size(), 3);
    EXPECT_EQ(map["one"], 1);
    EXPECT_EQ(map["two"], 2);
    EXPECT_EQ(map["three"], 3);
}

TEST(UnorderedMapTest, Find) {
    // Test unordered_map find functionality
    zen::unordered_map<std::string, int> map = { {"one", 1}, {"two", 2}, {"three", 3} };
    
    auto it = map.find("two");
    EXPECT_NE(it, map.end());
    EXPECT_EQ(it->second, 2);
    
    it = map.find("four");
    EXPECT_EQ(it, map.end());
}

TEST(UnorderedMapTest, Erase) {
    // Test unordered_map erase functionality
    zen::unordered_map<std::string, int> map = { {"one", 1}, {"two", 2}, {"three", 3} };
    map.erase("two");
    
    EXPECT_EQ(map.size(), 2);
    EXPECT_EQ(map.find("two"), map.end());
}