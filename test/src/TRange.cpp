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