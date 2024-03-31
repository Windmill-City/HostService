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
    Memory<uint8_t, 1024> prop;
    server.insert(0x01, prop.base());

    Extra extra{Extra::Type::ID_AND_MEMORY};
    extra.id()     = 0x01;
    extra.offset() = 27;
    extra.datlen() = extra.remain();

    // 添加255个元素
    for (size_t i = 0; i < extra.datlen(); i++)
    {
        extra.add((uint8_t)i);
    }

    client.send_request(Command::SET_MEMORY, extra);

    Poll();

    for (size_t i = 0; i < extra.datlen(); i++)
    {
        EXPECT_EQ(prop[i + extra.offset()], i);
    }
}

TEST_F(HostCS, Memory_GetMemory)
{
    Memory<uint8_t, 1024> prop;
    server.insert(0x01, prop.base());

    for (size_t i = 0; i < prop.len(); i++)
    {
        prop[i] = i;
    }

    Extra extra{Extra::Type::ID_AND_MEMORY};
    extra.id()     = 0x01;
    extra.offset() = 368;
    extra.datlen() = extra.remain();
    client.send_request(Command::GET_MEMORY, extra);

    Poll();

    for (size_t i = 0; i < extra.datlen(); i++)
    {
        EXPECT_EQ(client._extra[i], prop[i + extra.offset()]);
    }
}

TEST_F(HostCS, Memory_SetMemory_OutOfRange)
{
    Memory<uint8_t, 32> prop;
    server.insert(0x01, prop.base());

    Extra extra{Extra::Type::ID_AND_MEMORY};
    extra.id()     = 0x01;
    extra.offset() = 27;
    extra.datlen() = extra.remain();

    // 添加255个元素
    for (size_t i = 0; i < extra.datlen(); i++)
    {
        extra.add((uint8_t)i);
    }

    client.send_request(Command::SET_MEMORY, extra);

    Poll(true);

    EXPECT_EQ(client._rep.error, ErrorCode::E_OUT_OF_INDEX);
}

TEST_F(HostCS, Memory_GetMemory_OutOfRange)
{
    Memory<uint8_t, 32> prop;
    server.insert(0x01, prop.base());

    Extra extra{Extra::Type::ID_AND_MEMORY};
    extra.id()     = 0x01;
    extra.offset() = 368;
    extra.datlen() = extra.remain();
    client.send_request(Command::GET_MEMORY, extra);

    Poll(true);

    EXPECT_EQ(client._rep.error, ErrorCode::E_OUT_OF_INDEX);
}

TEST_F(HostCS, Memory_GetMemory_OutOfBuffer)
{
    Memory<uint8_t, 1024> prop;
    server.insert(0x01, prop.base());

    Extra extra{Extra::Type::ID_AND_MEMORY};
    extra.id()     = 0x01;
    extra.offset() = 422;
    extra.datlen() = 255; // 加上Id和内存参数, 超出最大帧长限制
    client.send_request(Command::GET_MEMORY, extra);

    Poll(true);

    EXPECT_EQ(client._rep.error, ErrorCode::E_OUT_OF_BUFFER);
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