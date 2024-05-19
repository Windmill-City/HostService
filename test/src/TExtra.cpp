#include "gtest/gtest.h"
#include <Extra.hpp>

TEST(Extra, sizeof)
{
    ASSERT_EQ(sizeof(Extra), 278);
}

TEST(Extra, read_write)
{
    Extra extra;
    float value = 10.f;
    // 数值读写
    extra.reset();
    ASSERT_TRUE(extra.add(value));
    extra.seek(0);
    ASSERT_TRUE(extra.get(value));
    ASSERT_FLOAT_EQ(value, 10.f);
    // 缓冲区空
    extra.reset();
    ASSERT_FALSE(extra.get(value));
    // 缓冲区满
    extra.seek(255);
    ASSERT_FALSE(extra.add(value));
    // 数组读写
    extra.reset();
    std::array<float, 16> floats;
    ASSERT_TRUE(extra.add(floats.data(), sizeof(floats)));
    extra.seek(0);
    ASSERT_EQ(extra.remain(), 64);
    ASSERT_TRUE(extra.get(floats.data(), sizeof(floats)));
    ASSERT_EQ(extra.remain(), 0);
    // 超长数组
    extra.reset();
    std::array<float, 256> floats_large;
    ASSERT_FALSE(extra.add(floats_large.data(), sizeof(floats_large)));
    ASSERT_EQ(extra.remain(), 0);
    ASSERT_FALSE(extra.get(floats_large.data(), sizeof(floats_large)));
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

    // 将剩余空间填充满
    while (extra.spare() > 0)
    {
        extra.add<uint8_t>(0x01);
    }

    // 加密参数
    extra.encrypt(nonce, key);

    // 验证是否正确解密
    ASSERT_TRUE(extra.decrypt(nonce, key));

    // 比对内容是否正确
    while (extra.remain() > 0)
    {
        uint8_t val;
        ASSERT_TRUE(extra.get(val));
        ASSERT_EQ(val, 0x01);
    }
}
