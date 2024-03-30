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

    RequestBuilder builder;
    builder.id(0x01);
    builder.tx(client, Command::GET_PROPERTY);

    Poll();

    EXPECT_FLOAT_EQ(*(float*)client._extra, 18.8f);
}

TEST_F(HostCS, Struct_SetProperty)
{
    Struct<FloatSt> prop;
    server.insert(0x01, prop);

    RequestBuilder builder;
    builder.id(0x01);
    builder.add(18.8f);
    builder.tx(client, Command::SET_PROPERTY);

    Poll();

    EXPECT_FLOAT_EQ(prop.get().val, 18.8f);
}

TEST_F(HostCS, Struct_SetMemory)
{
    Struct<FloatSt> prop;
    server.insert(0x01, prop);

    RequestBuilder builder;
    builder.id(0x01);
    builder.offset(0);
    builder.datlen(sizeof(float));
    builder.add(18.8f); // data
    builder.tx(client, Command::SET_MEMORY);

    Poll();

    EXPECT_FLOAT_EQ(prop.get().val, 18.8f);
}

TEST_F(HostCS, Struct_GetMemory)
{
    Struct<FloatSt> prop;
    server.insert(0x01, prop);
    prop.get().val = 18.8f;

    RequestBuilder builder;
    builder.id(0x01);
    builder.offset(0);
    builder.datlen(sizeof(float));
    builder.tx(client, Command::GET_MEMORY);

    Poll();

    EXPECT_FLOAT_EQ(*(float*)client._extra, 18.8f);
}

TEST_F(HostCS, Struct_GetSize)
{
    Struct<FloatSt> prop;
    server.insert(0x01, prop);

    RequestBuilder builder;
    builder.id(0x01);
    builder.tx(client, Command::GET_SIZE);

    Poll();

    EXPECT_EQ(*(uint16_t*)client._extra, sizeof(float));
}