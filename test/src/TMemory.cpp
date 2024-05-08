#include "gtest/gtest.h"
#include <HostCS.hpp>
#include <Memory.hpp>

TEST(Memory, sizeof)
{
    EXPECT_EQ(sizeof(Memory<bool, 1>), 8);
    EXPECT_EQ(sizeof(Memory<float, 1>), 8);
}

TEST(HostCS, Memory_GetProperty)
{
    Memory<float, 1> prop;
    HostCS<1>        cs({
        {5, &(PropertyBase&)prop}
    });

    Extra            extra;
    extra.add<PropertyId>(0x05);
    cs.client.send_request(Command::GET_PROPERTY, extra);

    cs.Poll(false);

    EXPECT_EQ(cs.client._rep.error, ErrorCode::E_NO_IMPLEMENT);
}

TEST(HostCS, Memory_SetProperty)
{
    Memory<float, 1> prop;
    HostCS<1>        cs({
        {5, &(PropertyBase&)prop}
    });

    Extra            extra;
    extra.add<PropertyId>(0x05);
    extra.add(18.8f);
    cs.client.send_request(Command::SET_PROPERTY, extra);

    cs.Poll(false);

    EXPECT_EQ(cs.client._rep.error, ErrorCode::E_NO_IMPLEMENT);
}

TEST(HostCS, Memory_SetMemory)
{
    Memory<uint8_t, 1024> prop;
    HostCS<1>        cs({
        {5, &(PropertyBase&)prop}
    });

    Extra            extra;
    extra.add<PropertyId>(0x05);

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

    cs.client.send_request(Command::SET_MEMORY, extra);

    cs.Poll();

    EXPECT_TRUE(memcmp(&prop, data.data(), data.size()) == 0);
}

TEST(HostCS, Memory_GetMemory)
{
    Memory<uint8_t, 1024> prop;
    HostCS<1>        cs({
        {5, &(PropertyBase&)prop}
    });

    for (size_t i = 0; i < prop.len(); i++)
    {
        prop[i] = i;
    }

    Extra extra;
    extra.add<PropertyId>(0x05);

    MemoryAccess access;
    access.offset = 0;
    access.size   = extra.spare() - sizeof(access);
    extra.add(access);

    cs.client.send_request(Command::GET_MEMORY, extra);

    cs.Poll();

    PropertyId id;
    cs.client._extra.get(id);
    cs.client._extra.get(access);

    // 读取数组
    std::vector<uint8_t> recv;
    recv.resize(access.size);
    cs.client._extra.get(recv.data(), recv.size());
    EXPECT_TRUE(memcmp(recv.data(), &prop, access.size) == 0);
}

TEST(HostCS, Memory_SetMemory_OutOfRange)
{
    Memory<float, 1> prop;
    HostCS<1>        cs({
        {5, &(PropertyBase&)prop}
    });

    Extra            extra;
    extra.add<PropertyId>(0x05);

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

    cs.client.send_request(Command::SET_MEMORY, extra);

    cs.Poll(false);

    EXPECT_EQ(cs.client._rep.error, ErrorCode::E_OUT_OF_INDEX);
}

TEST(HostCS, Memory_GetMemory_OutOfRange)
{
    Memory<float, 1> prop;
    HostCS<1>        cs({
        {5, &(PropertyBase&)prop}
    });

    Extra            extra;
    extra.add<PropertyId>(0x05);

    MemoryAccess access;
    access.offset = 0;
    access.size   = 255;
    extra.add(access);

    cs.client.send_request(Command::GET_MEMORY, extra);

    cs.Poll(false);

    EXPECT_EQ(cs.client._rep.error, ErrorCode::E_OUT_OF_INDEX);
}

TEST(HostCS, Memory_GetMemory_OutOfBuffer)
{
    Memory<uint8_t, 1024> prop;
    HostCS<1>        cs({
        {5, &(PropertyBase&)prop}
    });

    MemoryAccess     access;
    access.offset = 0;
    access.size   = 255; // 加上Id和内存参数, 超出最大帧长限制

    Extra extra;
    extra.add<PropertyId>(0x05);
    extra.add(access);

    cs.client.send_request(Command::GET_MEMORY, extra);

    cs.Poll(false);

    EXPECT_EQ(cs.client._rep.error, ErrorCode::E_OUT_OF_BUFFER);
}

TEST(HostCS, Memory_GetSize)
{
    Memory<float, 1> prop;
    HostCS<1>        cs({
        {5, &(PropertyBase&)prop}
    });

    Extra            extra;
    extra.add<PropertyId>(0x05);
    cs.client.send_request(Command::GET_SIZE, extra);

    cs.Poll();

    PropertyId id;
    cs.client._extra.get(id);

    uint16_t size;
    cs.client._extra.get(size);
    EXPECT_EQ(size, sizeof(float));
}
