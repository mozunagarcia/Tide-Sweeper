#include <gtest/gtest.h>

// Basic example test to verify Google Test is working
TEST(ExampleTest, BasicAssertion) {
    EXPECT_EQ(1 + 1, 2);
    EXPECT_TRUE(true);
}

TEST(ExampleTest, StringComparison) {
    std::string str = "TideSweeper";
    EXPECT_EQ(str, "TideSweeper");
    EXPECT_NE(str, "OtherGame");
}
