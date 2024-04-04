#include "gtest/gtest.h"
#include <HostCS.hpp>
#include <Struct.hpp>
#include <String.hpp>

struct FloatSt
{
    float val;
};

TEST(Struct, sizeof)
{
    EXPECT_EQ(sizeof(Struct<FloatSt>), 8);
    EXPECT_EQ(sizeof(Struct<FloatSt>), 8);
}

TEST(String, Use)
{
    String<32> str_1 = "Hello World";
    String<32> str_2 = "Hello World";

    // 测试赋值操作
    str_1            = "Hello";
    EXPECT_STREQ(str_1, "Hello");

    // 测试互相赋值
    str_1 = str_2 = "World";
    EXPECT_STREQ(str_1, str_2);
}

TEST_F(HostCS, Struct_GetProperty)
{
    Struct<FloatSt> prop;
    server.insert(0x01, prop);
    prop.ref().val = 18.8f;

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

    EXPECT_FLOAT_EQ(prop.ref().val, 18.8f);
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

    Poll(true);

    EXPECT_EQ(client._rep.error, ErrorCode::E_NO_IMPLEMENT);
}

TEST_F(HostCS, Struct_GetMemory)
{
    Struct<FloatSt> prop;
    server.insert(0x01, prop);
    prop.ref().val = 18.8f;

    Extra extra{Extra::Type::ID_AND_MEMORY};
    extra.id()     = 0x01;
    extra.offset() = 0;
    extra.datlen() = sizeof(float);
    client.send_request(Command::GET_MEMORY, extra);

    Poll(true);

    EXPECT_EQ(client._rep.error, ErrorCode::E_NO_IMPLEMENT);
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