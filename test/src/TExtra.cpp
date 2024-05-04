#include "gtest/gtest.h"
#include <Extra.hpp>

TEST(Extra, sizeof)
{
    EXPECT_EQ(sizeof(Extra), 260);
}

TEST(Extra, read_write)
{
    Extra extra;
    float value = 10.f;
    // 数值读写
    extra.reset();
    EXPECT_TRUE(extra.add(value));
    extra.seek(0);
    EXPECT_TRUE(extra.get(value));
    EXPECT_FLOAT_EQ(value, 10.f);
    // 缓冲区空
    extra.reset();
    EXPECT_FALSE(extra.get(value));
    // 缓冲区满
    extra.seek(255);
    EXPECT_FALSE(extra.add(value));
    // 数组读写
    extra.reset();
    std::array<float, 16> floats;
    EXPECT_TRUE(extra.add(floats.data(), sizeof(floats)));
    extra.seek(0);
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

TEST(Extra, AES)
{
    KeyType key;
    for (size_t i = 0; i < key.size(); i++)
    {
        key[i] = 0x01;
    }

    NonceType nonce;
    for (size_t i = 0; i < nonce.size(); i++)
    {
        nonce[i] = 0x01;
    }

    Extra extra;
    // 预留校验位
    extra.reserve_tag();

    while (extra.spare() > 0)
    {
        extra.add<uint8_t>(0x01);
    }

    extra.encrypt(nonce, key);

    EXPECT_TRUE(extra.decrypt(nonce, key));

    while (extra.remain() > 0)
    {
        uint8_t val;
        EXPECT_TRUE(extra.get(val));
        EXPECT_EQ(val, 0x01);
    }
}
