#include "gtest/gtest.h"
#include <HostCS.hpp>
#include <Range.hpp>

TEST(Range, sizeof)
{
    EXPECT_EQ(sizeof(Range<bool>), 16);
    EXPECT_EQ(sizeof(Range<bool>::_Range), 2);

    EXPECT_EQ(sizeof(Range<float>), 20);
    EXPECT_EQ(sizeof(Range<float>::_Range), 8);
}

TEST(Range, Mode)
{
    Range<int, RangeMode::Hard>  hard{100, 0, 10};
    Range<int, RangeMode::Soft>  soft{100, 0, 10};
    Range<int, RangeMode::Clamp> clamp{-100, 0, 10};

    // hard 和 clamp 在初始化时会进行clamp
    EXPECT_EQ(hard, 10);
    EXPECT_EQ(clamp, 0);
    // soft 不会进行clamp
    EXPECT_EQ(soft, 100);

    // 测试边界能否赋值
    hard = 10;
    EXPECT_EQ(hard, 10);
    hard = 0;
    EXPECT_EQ(hard, 0);

    // 测试不同模式能否赋值
    hard = soft = 10;
    EXPECT_EQ(hard, soft);

    // 测试是否拦截赋值
    hard = 100;
    EXPECT_EQ(hard, 10);

    // 测试是否允许赋值
    soft = 100;
    EXPECT_EQ(soft, 100);
    // 测试in_range
    EXPECT_FALSE(soft.in_range());

    // 测试in_range
    soft = 7;
    EXPECT_EQ(soft, 7);
    EXPECT_TRUE(soft.in_range());

    // 测试clamp是否正常
    EXPECT_EQ(clamp = 100, 10);
    EXPECT_EQ(clamp = -100, 0);
}

TEST_F(HostCS, Range_GetProperty)
{
    Range<float> prop{18.8f, 0.0f, 25.f};
    server.insert(0x01, prop);

    Extra extra;
    extra.id() = 0x01;
    client.send_request(Command::GET_PROPERTY, extra);

    Poll();

    EXPECT_EQ(*(float*)client._extra.data(), 18.8f);
}

TEST_F(HostCS, Range_SetProperty)
{
    Range<float> prop{0.0f, 0.0f, 25.f};
    server.insert(0x01, prop);

    Extra extra;
    extra.id() = 0x01;
    extra.add(18.8f);
    client.send_request(Command::SET_PROPERTY, extra);

    Poll();

    EXPECT_EQ(prop, 18.8f);
}

TEST_F(HostCS, Range_SetMemory)
{
    Range<float> prop{0.0f, 0.0f, 25.f};
    server.insert(0x01, prop);

    Extra extra{Extra::Type::ID_AND_MEMORY};
    extra.id()     = 0x01;
    extra.offset() = 0;
    extra.datlen() = sizeof(float);
    extra.add(18.8f); // data
    client.send_request(Command::SET_MEMORY, extra);

    Poll(true);

    EXPECT_EQ(client._rep.error, ErrorCode::E_NO_IMPLEMENT);
}

TEST_F(HostCS, Range_GetMemory)
{
    Range<float> prop{0.0f, 0.0f, 25.f};
    server.insert(0x01, prop);

    Extra extra{Extra::Type::ID_AND_MEMORY};
    extra.id()     = 0x01;
    extra.offset() = 0;
    extra.datlen() = sizeof(float);
    client.send_request(Command::GET_MEMORY, extra);

    Poll(true);

    EXPECT_EQ(client._rep.error, ErrorCode::E_NO_IMPLEMENT);
}

TEST_F(HostCS, Range_GetSize)
{
    Range<float> prop{0.0f, 0.0f, 25.f};
    server.insert(0x01, prop);

    Extra extra;
    extra.id() = 0x01;
    client.send_request(Command::GET_SIZE, extra);

    Poll();

    EXPECT_EQ(*(uint16_t*)client._extra.data(), sizeof(float));
}