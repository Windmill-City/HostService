#include "gtest/gtest.h"
#include <HostCS.hpp>
#include <Range.hpp>

TEST(Range, sizeof)
{
    EXPECT_EQ(sizeof(Range<bool>), 28);
    EXPECT_EQ(sizeof(Range<float>), 32);
}

TEST_F(HostCS, Range_GetProperty)
{
    Range<float> prop{18.8f, 0.0f, 25.f};
    server.insert(0x01, prop);

    RequestBuilder builder;
    builder.id(0x01);
    builder.tx(client, Command::GET_PROPERTY);

    Poll();

    EXPECT_EQ(*(float*)client._extra, 18.8f);
}

TEST_F(HostCS, Range_SetProperty)
{
    Range<float> prop{0.0f, 0.0f, 25.f};
    server.insert(0x01, prop);

    RequestBuilder builder;
    builder.id(0x01);
    builder.add(18.8f);
    builder.tx(client, Command::SET_PROPERTY);

    Poll();

    EXPECT_EQ(prop, 18.8f);
}

TEST_F(HostCS, Range_SetMemory)
{
    Range<float> prop{0.0f, 0.0f, 25.f};
    server.insert(0x01, prop);

    RequestBuilder builder;
    builder.id(0x01);
    builder.offset(0);
    builder.datlen(sizeof(float));
    builder.add(18.8f); // data
    builder.tx(client, Command::SET_MEMORY);

    Poll(true);

    EXPECT_EQ(client._rep.error, ErrorCode::E_NO_IMPLEMENT);
}

TEST_F(HostCS, Range_GetMemory)
{
    Range<float> prop{0.0f, 0.0f, 25.f};
    server.insert(0x01, prop);

    RequestBuilder builder;
    builder.id(0x01);
    builder.offset(0);
    builder.datlen(sizeof(float));
    builder.tx(client, Command::GET_MEMORY);

    Poll(true);

    EXPECT_EQ(client._rep.error, ErrorCode::E_NO_IMPLEMENT);
}

TEST_F(HostCS, Range_GetSize)
{
    Range<float> prop{0.0f, 0.0f, 25.f};
    server.insert(0x01, prop);

    RequestBuilder builder;
    builder.id(0x01);
    builder.tx(client, Command::GET_SIZE);

    Poll();

    EXPECT_EQ(*(uint16_t*)client._extra, sizeof(float));
}