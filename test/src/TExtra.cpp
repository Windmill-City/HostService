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

TEST(Extra, AES)
{
    AES aes;
    for (size_t i = 0; i < aes.Key.size(); i++)
    {
        aes.Key[i] = 0x01;
    }

    for (size_t i = 0; i < aes.Nonce.size(); i++)
    {
        aes.Nonce[i] = 0x01;
    }

    std::array<uint8_t, 3>  data = {0x01, 0x02, 0x03};
    std::array<uint8_t, 12> tag;

    UAES_CCM_SimpleEncrypt(aes.Key.data(),
                           aes.Key.size(),
                           aes.Nonce.data(),
                           aes.Nonce.size(),
                           NULL,
                           0,
                           data.data(),
                           data.data(),
                           data.size(),
                           tag.data(),
                           tag.size());

    UAES_CCM_SimpleDecrypt(aes.Key.data(),
                           aes.Key.size(),
                           aes.Nonce.data(),
                           aes.Nonce.size(),
                           NULL,
                           0,
                           data.data(),
                           data.data(),
                           data.size(),
                           tag.data(),
                           tag.size());

    Extra extra;
    // 预留校验位
    extra.reserve_tag();

    while (extra.spare() > 0)
    {
        extra.add<uint8_t>(0x01);
    }

    extra.encrypt(aes);

    EXPECT_TRUE(extra.decrypt(aes));

    while (extra.remain() > 0)
    {
        uint8_t val;
        EXPECT_TRUE(extra.get(val));
        EXPECT_EQ(val, 0x01);
    }
}
