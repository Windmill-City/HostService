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
    server.put(0x01, prop);
    prop.ref().val = 18.8f;

    Extra extra;
    extra.add<PropertyId>(0x01);
    client.send_request(Command::GET_PROPERTY, extra);

    Poll();

    PropertyId id;
    client._extra.get(id);

    float recv;
    client._extra.get(recv);

    EXPECT_EQ(recv, 18.8f);
}

TEST_F(HostCS, Struct_SetProperty)
{
    Struct<FloatSt> prop;
    server.put(0x01, prop);

    Extra extra;
    extra.add<PropertyId>(0x01);
    extra.add(18.8f);
    client.send_request(Command::SET_PROPERTY, extra);

    Poll();

    EXPECT_FLOAT_EQ(prop.ref().val, 18.8f);
}

TEST_F(HostCS, Struct_SetMemory)
{
    Struct<FloatSt> prop;
    server.put(0x01, prop);

    MemoryAccess access;
    access.offset = 0;
    access.size   = sizeof(float);

    Extra extra;
    extra.add<PropertyId>(0x01);
    extra.add(access);
    extra.add(18.8f); // data
    client.send_request(Command::SET_MEMORY, extra);

    Poll(true);

    EXPECT_EQ(client._rep.error, ErrorCode::E_NO_IMPLEMENT);
}

TEST_F(HostCS, Struct_GetMemory)
{
    Struct<FloatSt> prop;
    server.put(0x01, prop);
    prop.ref().val = 18.8f;

    MemoryAccess access;
    access.offset = 0;
    access.size   = sizeof(float);

    Extra extra;
    extra.add<PropertyId>(0x01);
    extra.add(access);
    client.send_request(Command::GET_MEMORY, extra);

    Poll(true);

    EXPECT_EQ(client._rep.error, ErrorCode::E_NO_IMPLEMENT);
}

TEST_F(HostCS, Struct_GetSize)
{
    Struct<FloatSt> prop;
    server.put(0x01, prop);

    Extra extra;
    extra.add<PropertyId>(0x01);
    client.send_request(Command::GET_SIZE, extra);

    Poll();

    PropertyId id;
    client._extra.get(id);

    uint16_t size;
    client._extra.get(size);
    EXPECT_EQ(size, sizeof(float));
}