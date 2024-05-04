#include "gtest/gtest.h"
#include <HostCS.hpp>
#include <Property.hpp>

TEST(HostClient, sizeof)
{
    EXPECT_EQ(sizeof(HostClient), 276);
}

TEST(HostServer, sizeof)
{
    HostServerImpl hs;
    EXPECT_EQ(sizeof(HostServer), 300);
}

TEST_F(HostCS, request)
{
    uint8_t data[] = {0x01, 0x02, 0x03};

    Extra   extra;
    extra.add(data, sizeof(data));
    client.send_request(Command::ECHO, extra);

    Poll();

    EXPECT_TRUE(memcmp(client._extra.data(), data, sizeof(data)) == 0);
}

TEST_F(HostCS, ids_size)
{
    PropertyId id = 0;
    Extra      extra;
    extra.add(id);
    client.send_request(Command::GET_SIZE, extra);

    Poll();

    client._extra.get(id);

    uint16_t size;
    client._extra.get(size);
    EXPECT_EQ(size, sizeof(PropertyId) * 1);
}

TEST_F(HostCS, ids_content)
{
    // 添加一个属性值
    Property<float> prop("prop.float");
    server.put(233, prop);

    PropertyId   id = 0;

    MemoryAccess access;
    access.offset = sizeof(PropertyId) * 1;
    access.size   = sizeof(PropertyId) * 1;

    Extra extra;
    extra.add(id);
    extra.add(access);
    client.send_request(Command::GET_MEMORY, extra);

    Poll();

    client._extra.get(id);
    client._extra.get(access);

    // id = 233, prop.float
    client._extra.get(id);
    EXPECT_EQ(id, 233);
}