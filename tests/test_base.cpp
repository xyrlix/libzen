#include <gtest/gtest.h>
#include "base/macros.h"
#include "base/type_traits.h"
#include "base/compile_tools.h"

TEST(BaseTest, MacrosTest) {
    // Test basic macros
    EXPECT_EQ(ZEN_ARRAY_SIZE((int[]){1, 2, 3, 4, 5}), 5);
    int unused_var = 42;
    ZEN_UNUSED(unused_var);
}

TEST(BaseTest, TypeTraitsTest) {
    // Test type traits functionality
    EXPECT_TRUE(true); // Placeholder test
}

TEST(BaseTest, CompileToolsTest) {
    // Test compile-time utilities
    EXPECT_TRUE(true); // Placeholder test
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}