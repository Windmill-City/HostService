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
    server.put(0x01, prop);

    Extra extra;
    extra.add<PropertyId>(0x01);
    client.send_request(Command::GET_PROPERTY, extra);

    Poll(true);

    EXPECT_EQ(client._rep.error, ErrorCode::E_NO_IMPLEMENT);
}

TEST_F(HostCS, Memory_SetProperty)
{
    Memory<float, 1> prop;
    server.put(0x01, prop);

    Extra extra;
    extra.add<PropertyId>(0x01);
    extra.add(18.8f);
    client.send_request(Command::SET_PROPERTY, extra);

    Poll(true);

    EXPECT_EQ(client._rep.error, ErrorCode::E_NO_IMPLEMENT);
}

TEST_F(HostCS, Memory_SetMemory)
{
    Memory<uint8_t, 1024> prop;
    server.put(0x01, prop);

    Extra extra;
    extra.add<PropertyId>(0x01);

    MemoryAccess access;
    access.offset = 0;
    access.size   = extra.spare() - sizeof(access);
    extra.add(access);

    // 将剩余空间填充满
    size_t               spare = access.size;
    std::vector<uint8_t> data;
    data.resize(spare);
    for (size_t i = 0; i < spare; i++)
    {
        data[i] = i;
    }
    extra.add(data.data(), data.size());

    client.send_request(Command::SET_MEMORY, extra);

    Poll();

    EXPECT_TRUE(memcmp(&prop, data.data(), data.size()) == 0);
}

TEST_F(HostCS, Memory_GetMemory)
{
    Memory<uint8_t, 1024> prop;
    server.put(0x01, prop);

    for (size_t i = 0; i < prop.len(); i++)
    {
        prop[i] = i;
    }

    Extra extra;
    extra.add<PropertyId>(0x01);

    MemoryAccess access;
    access.offset = 0;
    access.size   = extra.spare() - sizeof(access);
    extra.add(access);

    client.send_request(Command::GET_MEMORY, extra);

    Poll();

    PropertyId id;
    client._extra.get(id);
    client._extra.get(access);

    // 读取数组
    std::vector<uint8_t> recv;
    recv.resize(access.size);
    client._extra.get(recv.data(), recv.size());
    EXPECT_TRUE(memcmp(recv.data(), &prop, access.size) == 0);
}

TEST_F(HostCS, Memory_SetMemory_OutOfRange)
{
    Memory<uint8_t, 32> prop;
    server.put(0x01, prop);

    Extra extra;
    extra.add<PropertyId>(0x01);

    MemoryAccess access;
    access.offset = 0;
    access.size   = extra.spare() - sizeof(access);
    extra.add(access);

    // 将剩余空间填充满
    size_t               spare = access.size;
    std::vector<uint8_t> data;
    data.resize(spare);
    for (size_t i = 0; i < spare; i++)
    {
        data[i] = i;
    }
    extra.add(data.data(), data.size());

    client.send_request(Command::SET_MEMORY, extra);

    Poll(true);

    EXPECT_EQ(client._rep.error, ErrorCode::E_OUT_OF_INDEX);
}

TEST_F(HostCS, Memory_GetMemory_OutOfRange)
{
    Memory<uint8_t, 32> prop;
    server.put(0x01, prop);

    Extra extra;
    extra.add<PropertyId>(0x01);

    MemoryAccess access;
    access.offset = 0;
    access.size   = 255;
    extra.add(access);

    client.send_request(Command::GET_MEMORY, extra);

    Poll(true);

    EXPECT_EQ(client._rep.error, ErrorCode::E_OUT_OF_INDEX);
}

TEST_F(HostCS, Memory_GetMemory_OutOfBuffer)
{
    Memory<uint8_t, 1024> prop;
    server.put(0x01, prop);

    MemoryAccess access;
    access.offset = 0;
    access.size   = 255; // 加上Id和内存参数, 超出最大帧长限制

    Extra extra;
    extra.add<PropertyId>(0x01);
    extra.add(access);

    client.send_request(Command::GET_MEMORY, extra);

    Poll(true);

    EXPECT_EQ(client._rep.error, ErrorCode::E_OUT_OF_BUFFER);
}

TEST_F(HostCS, Memory_GetSize)
{
    Memory<float, 1> prop;
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