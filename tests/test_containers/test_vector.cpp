#include <gtest/gtest.h>
#include "containers/sequential/vector.h"

TEST(VectorTest, BasicOperations) {
    // Test basic vector operations
    zen::vector<int> vec;
    EXPECT_TRUE(vec.empty());
    EXPECT_EQ(vec.size(), 0);
    
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    
    EXPECT_FALSE(vec.empty());
    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 2);
    EXPECT_EQ(vec[2], 3);
}

TEST(VectorTest, Iteration) {
    // Test vector iteration
    zen::vector<int> vec = {1, 2, 3, 4, 5};
    int sum = 0;
    for (const auto& elem : vec) {
        sum += elem;
    }
    EXPECT_EQ(sum, 15);
}

TEST(VectorTest, Erase) {
    // Test vector erase functionality
    zen::vector<int> vec = {1, 2, 3, 4, 5};
    vec.erase(vec.begin() + 2);
    EXPECT_EQ(vec.size(), 4);
    EXPECT_EQ(vec[2], 4);
}