#include "gtest/gtest.h"
#include <cstdint>
#include <HostCS.hpp>
#include <Memory.hpp>

TEST(Memory, sizeof)
{
    EXPECT_EQ(sizeof(Memory<bool>), 8);
    EXPECT_EQ(sizeof(Memory<float>), 8);
}

TEST(MemoryAccess, sizeof)
{
    EXPECT_EQ(sizeof(MemoryAccess), 4);
}

static std::array<uint8_t, 1024>                      ArrayVal;
static Memory<decltype(ArrayVal), Access::READ_WRITE> Prop_1(ArrayVal);
// 静态初始化
static constexpr PropertyMap<1>                       Map = {
    {
     {"prop.1", &(PropertyBase&)Prop_1},
     }
};
static PropertyHolder            Holder(Map);

static constinit CPropertyMap<1> CMap = {
    {
     {"prop.1", 0},
     }
};
static CPropertyHolder CHolder(CMap);

struct TMemory
    : public HostCSBase
    , public testing::Test
{
    TMemory()
        : HostCSBase(Holder, CHolder)
    {
    }
};

TEST_F(TMemory, Set)
{
    Extra     extra;
    ErrorCode err;
    extra.add<PropertyId>(0);

    MemoryAccess access;
    access.offset = 0;
    access.size   = 1024;
    extra.add(access);

    // 将剩余空间填充满
    std::vector<uint8_t> data;
    data.resize(1024);
    for (size_t i = 0; i < 1024; i++)
    {
        data[i] = i;
    }
    ASSERT_TRUE(extra.add(data.data(), data.size()));
    client.send(Command::SET_PROPERTY, extra);

    ASSERT_TRUE(server.poll());
    client.recv_response(Command::SET_PROPERTY, err, client.extra);

    EXPECT_TRUE(memcmp(ArrayVal.data(), data.data(), data.size()) == 0);
}

TEST_F(TMemory, Get)
{
    // 初始化内存区
    for (size_t i = 0; i < ArrayVal.size(); i++)
    {
        ArrayVal[i] = i;
    }

    Extra     extra;
    ErrorCode err;
    extra.add<PropertyId>(0);

    MemoryAccess access;
    access.offset = 0;
    access.size   = 1024;
    extra.add(access);
    client.send(Command::GET_PROPERTY, extra);

    ASSERT_TRUE(server.poll());
    client.recv_response(Command::GET_PROPERTY, err, client.extra);

    // 读取数组
    std::vector<uint8_t> recv;
    recv.resize(access.size);
    client.extra.get(recv.data(), recv.size());
    EXPECT_TRUE(memcmp(recv.data(), ArrayVal.data(), access.size) == 0);
}

TEST_F(TMemory, Set_OutOfRange)
{
    Extra     extra;
    ErrorCode err;
    extra.add<PropertyId>(0);

    MemoryAccess access;
    access.offset = 1024;
    access.size   = 256;
    extra.add(access);
    extra.seek(256 + sizeof(PropertyId) + sizeof(access));
    client.send(Command::SET_PROPERTY, extra);

    ASSERT_FALSE(server.poll());
    client.recv_response(Command::SET_PROPERTY, err, client.extra);

    EXPECT_EQ(err, ErrorCode::E_OUT_OF_INDEX);
}

TEST_F(TMemory, Get_OutOfRange)
{
    Extra     extra;
    ErrorCode err;
    extra.add<PropertyId>(0);

    MemoryAccess access;
    access.offset = 1024;
    access.size   = 256;
    extra.add(access);
    client.send(Command::GET_PROPERTY, extra);

    ASSERT_FALSE(server.poll());
    client.recv_response(Command::GET_PROPERTY, err, client.extra);

    EXPECT_EQ(err, ErrorCode::E_OUT_OF_INDEX);
}

TEST_F(TMemory, GetSize)
{
    Extra     extra;
    ErrorCode err;
    extra.add<PropertyId>(0);
    client.send(Command::GET_SIZE, extra);

    ASSERT_TRUE(server.poll());
    client.recv_response(Command::GET_SIZE, err, client.extra);

    uint16_t size;
    client.extra.get(size);
    EXPECT_EQ(size, 1024);
}
