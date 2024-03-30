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

    RequestBuilder builder;
    builder.id(0x01);
    builder.tx(client, Command::GET_PROPERTY);

    Poll(true);

    EXPECT_EQ(client._rep.error, ErrorCode::E_NO_IMPLEMENT);
}

TEST_F(HostCS, Memory_SetProperty)
{
    Memory<float, 1> prop;
    server.insert(0x01, prop.base());

    RequestBuilder builder;
    builder.id(0x01);
    builder.add(18.8f);
    builder.tx(client, Command::SET_PROPERTY);

    Poll(true);

    EXPECT_EQ(client._rep.error, ErrorCode::E_NO_IMPLEMENT);
}

TEST_F(HostCS, Memory_SetMemory)
{
    Memory<float, 1> prop;
    server.insert(0x01, prop.base());

    RequestBuilder builder;
    builder.id(0x01);
    builder.offset(0);
    builder.datlen(sizeof(float));
    builder.add(18.8f); // data
    builder.tx(client, Command::SET_MEMORY);

    Poll();

    EXPECT_EQ(prop[0], 18.8f);
}

TEST_F(HostCS, Memory_GetMemory)
{
    Memory<float, 1> prop;
    server.insert(0x01, prop.base());
    prop[0] = 18.8f;

    RequestBuilder builder;
    builder.id(0x01);
    builder.offset(0);
    builder.datlen(sizeof(float));
    builder.tx(client, Command::GET_MEMORY);

    Poll();

    EXPECT_EQ(*(float*)client._extra, 18.8f);
}

TEST_F(HostCS, Memory_GetSize)
{
    Memory<float, 1> prop;
    server.insert(0x01, prop.base());

    RequestBuilder builder;
    builder.id(0x01);
    builder.tx(client, Command::GET_SIZE);

    Poll();

    EXPECT_EQ(*(uint16_t*)client._extra, sizeof(float));
}