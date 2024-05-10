#include "gtest/gtest.h"
#include <HostCS.hpp>
#include <Memory.hpp>

TEST(Memory, sizeof)
{
    EXPECT_EQ(sizeof(Memory<bool>), 8);
    EXPECT_EQ(sizeof(Memory<float>), 8);
}

TEST(MemoryAccess, sizeof)
{
    EXPECT_EQ(sizeof(MemoryAccess), 3);
}

static Memory<std::array<uint8_t, 1024>> mem;
// 静态初始化
static constexpr PropertyMap<1>          map = {
    {"mem", &(PropertyBase&)mem}
};

static PropertyHolder holder(map);

struct TMemory
    : public HostCSBase
    , public testing::Test
{
    TMemory()
        : HostCSBase(holder)
    {
    }
};

TEST_F(TMemory, Set)
{
    Extra extra;
    extra.add<PropertyId>(0);

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

    client.send_request(Command::SET_PROPERTY, extra);

    Poll();

    EXPECT_TRUE(memcmp(&mem, data.data(), data.size()) == 0);
}

TEST_F(TMemory, Get)
{
    // 初始化内存区
    for (size_t i = 0; i < mem.size(); i++)
    {
        mem[i] = i;
    }

    Extra extra;
    extra.add<PropertyId>(0);

    MemoryAccess access;
    access.offset = 0;
    access.size   = extra.spare() - sizeof(access);
    extra.add(access);

    client.send_request(Command::GET_PROPERTY, extra);

    Poll();

    // 读取 id
    PropertyId id;
    client._extra.get(id);
    // 读取 access
    client._extra.get(access);

    // 读取数组
    std::vector<uint8_t> recv;
    recv.resize(access.size);
    client._extra.get(recv.data(), recv.size());
    EXPECT_TRUE(memcmp(recv.data(), &mem, access.size) == 0);
}

TEST_F(TMemory, Set_OutOfRange)
{
    Extra extra;
    extra.add<PropertyId>(0);

    MemoryAccess access;
    access.offset = 1024;
    access.size   = extra.spare() - sizeof(access);
    extra.add(access);
    extra.seek(255);

    client.send_request(Command::SET_PROPERTY, extra);

    Poll(false);

    EXPECT_EQ(client._rep.error, ErrorCode::E_OUT_OF_INDEX);
}

TEST_F(TMemory, Get_OutOfRange)
{
    Extra extra;
    extra.add<PropertyId>(0);

    MemoryAccess access;
    access.offset = 1024;
    access.size   = 255;
    extra.add(access);

    client.send_request(Command::GET_PROPERTY, extra);

    Poll(false);

    EXPECT_EQ(client._rep.error, ErrorCode::E_OUT_OF_INDEX);
}

TEST_F(TMemory, Get_OutOfBuffer)
{
    MemoryAccess access;
    access.offset = 0;
    access.size   = 255; // 加上Id和内存参数, 超出最大帧长限制

    Extra extra;
    extra.add<PropertyId>(0);
    extra.add(access);

    client.send_request(Command::GET_PROPERTY, extra);

    Poll(false);

    EXPECT_EQ(client._rep.error, ErrorCode::E_OUT_OF_BUFFER);
}

TEST_F(TMemory, GetSize)
{
    Extra extra;
    extra.add<PropertyId>(0);
    client.send_request(Command::GET_SIZE, extra);

    Poll();

    PropertyId id;
    client._extra.get(id);

    uint16_t size;
    client._extra.get(size);
    EXPECT_EQ(size, 1024);
}
