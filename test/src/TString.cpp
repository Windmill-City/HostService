#include "gtest/gtest.h"
#include <String.hpp>

TEST(String, Use)
{
    String<32> str_1 = "Hello World";
    String<32> str_2 = "Hello World";

    // 测试赋值操作
    str_1            = "Hello";
    EXPECT_STREQ(str_1, "Hello");

    // 测试互相赋值
    str_1 = str_2 = "World";
    EXPECT_STREQ(str_1, str_2);
}