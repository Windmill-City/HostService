#include "gtest/gtest.h"
#include <HostCS.hpp>
#include <Struct.hpp>

struct FloatSt
{
    float val;
};

TEST(Struct, sizeof)
{
    EXPECT_EQ(sizeof(Struct<FloatSt>), 8);
    EXPECT_EQ(sizeof(Struct<FloatSt>), 8);
}

TEST_F(HostCS, Struct_GetProperty)
{
    Struct<FloatSt> prop;
    server.insert(0x01, prop);
    prop.get().val = 18.8f;

    Extra extra;
    extra.id() = 0x01;
    client.send_request(Command::GET_PROPERTY, extra);

    Poll();

    EXPECT_FLOAT_EQ(*(float*)client._extra.data(), 18.8f);
}

TEST_F(HostCS, Struct_SetProperty)
{
    Struct<FloatSt> prop;
    server.insert(0x01, prop);

    Extra extra;
    extra.id() = 0x01;
    extra.add(18.8f);
    client.send_request(Command::SET_PROPERTY, extra);

    Poll();

    EXPECT_FLOAT_EQ(prop.get().val, 18.8f);
}

TEST_F(HostCS, Struct_SetMemory)
{
    Struct<FloatSt> prop;
    server.insert(0x01, prop);

    Extra extra{Extra::Type::ID_AND_MEMORY};
    extra.id()     = 0x01;
    extra.offset() = 0;
    extra.datlen() = sizeof(float);
    extra.add(18.8f); // data
    client.send_request(Command::SET_MEMORY, extra);

    Poll();

    EXPECT_FLOAT_EQ(prop.get().val, 18.8f);
}

TEST_F(HostCS, Struct_GetMemory)
{
    Struct<FloatSt> prop;
    server.insert(0x01, prop);
    prop.get().val = 18.8f;

    Extra extra{Extra::Type::ID_AND_MEMORY};
    extra.id()     = 0x01;
    extra.offset() = 0;
    extra.datlen() = sizeof(float);
    client.send_request(Command::GET_MEMORY, extra);

    Poll();

    EXPECT_FLOAT_EQ(*(float*)client._extra.data(), 18.8f);
}

TEST_F(HostCS, Struct_GetSize)
{
    Struct<FloatSt> prop;
    server.insert(0x01, prop);

    Extra extra;
    extra.id() = 0x01;
    client.send_request(Command::GET_SIZE, extra);

    Poll();

    EXPECT_EQ(*(uint16_t*)client._extra.data(), sizeof(float));
}