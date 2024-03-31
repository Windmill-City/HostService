#include "gtest/gtest.h"
#include <HostCS.hpp>
#include <Memory.hpp>

TEST(Memory, sizeof)
{
    EXPECT_EQ(sizeof(Memory<bool, 1>), 8);
    EXPECT_EQ(sizeof(Memory<float, 1>), 8);
}

TEST_F(HostCS, Memory_GetProperty)
{
    Memory<float, 1> prop;
    server.insert(0x01, prop.base());

    Extra extra;
    extra.id() = 0x01;
    client.send_request(Command::GET_PROPERTY, extra);

    Poll(true);

    EXPECT_EQ(client._rep.error, ErrorCode::E_NO_IMPLEMENT);
}

TEST_F(HostCS, Memory_SetProperty)
{
    Memory<float, 1> prop;
    server.insert(0x01, prop.base());

    Extra extra;
    extra.id() = 0x01;
    extra.add(18.8f);
    client.send_request(Command::SET_PROPERTY, extra);

    Poll(true);

    EXPECT_EQ(client._rep.error, ErrorCode::E_NO_IMPLEMENT);
}

TEST_F(HostCS, Memory_SetMemory)
{
    Memory<float, 1> prop;
    server.insert(0x01, prop.base());

    Extra extra{Extra::Type::ID_AND_MEMORY};
    extra.id()     = 0x01;
    extra.offset() = 0;
    extra.datlen() = sizeof(float);
    extra.add(18.8f); // data
    client.send_request(Command::SET_MEMORY, extra);

    Poll();

    EXPECT_EQ(prop[0], 18.8f);
}

TEST_F(HostCS, Memory_GetMemory)
{
    Memory<float, 1> prop;
    server.insert(0x01, prop.base());
    prop[0] = 18.8f;

    Extra extra{Extra::Type::ID_AND_MEMORY};
    extra.id()     = 0x01;
    extra.offset() = 0;
    extra.datlen() = sizeof(float);
    client.send_request(Command::GET_MEMORY, extra);

    Poll();

    EXPECT_EQ(*(float*)client._extra.data(), 18.8f);
}

TEST_F(HostCS, Memory_GetSize)
{
    Memory<float, 1> prop;
    server.insert(0x01, prop.base());

    Extra extra;
    extra.id() = 0x01;
    client.send_request(Command::GET_SIZE, extra);

    Poll();

    EXPECT_EQ(*(uint16_t*)client._extra.data(), sizeof(float));
}