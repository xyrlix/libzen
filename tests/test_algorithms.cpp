#include <gtest/gtest.h>
#include "algorithms/sort.h"
#include "algorithms/find.h"
#include "algorithms/transform.h"
#include "algorithms/numeric.h"
#include "containers/sequential/vector.h"

TEST(AlgorithmsTest, SortTest) {
    // Test sorting algorithms
    zen::vector<int> vec = {3, 1, 4, 1, 5, 9, 2, 6};
    zen::sort(vec.begin(), vec.end());
    
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 1);
    EXPECT_EQ(vec[2], 2);
    EXPECT_EQ(vec[3], 3);
    EXPECT_EQ(vec[4], 4);
    EXPECT_EQ(vec[5], 5);
    EXPECT_EQ(vec[6], 6);
    EXPECT_EQ(vec[7], 9);
}

TEST(AlgorithmsTest, FindTest) {
    // Test searching algorithms
    zen::vector<int> vec = {1, 2, 3, 4, 5};
    auto it = zen::find(vec.begin(), vec.end(), 3);
    EXPECT_NE(it, vec.end());
    EXPECT_EQ(*it, 3);
    
    it = zen::find(vec.begin(), vec.end(), 6);
    EXPECT_EQ(it, vec.end());
}

TEST(AlgorithmsTest, TransformTest) {
    // Test transform algorithms
    zen::vector<int> vec = {1, 2, 3, 4, 5};
    zen::vector<int> result(vec.size());
    
    auto square = [](int x) { return x * x; };
    zen::transform(vec.begin(), vec.end(), result.begin(), square);
    
    EXPECT_EQ(result[0], 1);
    EXPECT_EQ(result[1], 4);
    EXPECT_EQ(result[2], 9);
    EXPECT_EQ(result[3], 16);
    EXPECT_EQ(result[4], 25);
}

TEST(AlgorithmsTest, AccumulateTest) {
    // Test accumulate algorithm
    zen::vector<int> vec = {1, 2, 3, 4, 5};
    int sum = zen::accumulate(vec.begin(), vec.end(), 0);
    EXPECT_EQ(sum, 15);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}