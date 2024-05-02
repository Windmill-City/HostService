#include "gtest/gtest.h"
#include <Extra.hpp>

TEST(Extra, sizeof)
{
    EXPECT_EQ(sizeof(Extra), 259);
}

TEST(Extra, read_write)
{
    Extra extra;
    float value = 10.f;
    // 数值读写
    EXPECT_TRUE(extra.add(value));
    EXPECT_TRUE(extra.get(value));
    EXPECT_FLOAT_EQ(value, 10.f);
    // 缓冲区空
    EXPECT_FALSE(extra.get(value));
    // 缓冲区满
    extra.size() = 255;
    EXPECT_FALSE(extra.add(value));
    // 数组读写
    extra.reset();
    std::array<float, 16> floats;
    EXPECT_TRUE(extra.add(floats.data(), sizeof(floats)));
    EXPECT_EQ(extra.remain(), 64);
    EXPECT_TRUE(extra.get(floats.data(), sizeof(floats)));
    EXPECT_EQ(extra.remain(), 0);
    // 超长数组
    extra.reset();
    std::array<float, 256> floats_large;
    EXPECT_FALSE(extra.add(floats_large.data(), sizeof(floats_large)));
    EXPECT_EQ(extra.remain(), 0);
    EXPECT_FALSE(extra.get(floats_large.data(), sizeof(floats_large)));
}
