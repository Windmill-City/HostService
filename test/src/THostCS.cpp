#include "gtest/gtest.h"
#include <HostCS.hpp>
#include <Property.hpp>

TEST(HostClient, sizeof)
{
    EXPECT_EQ(sizeof(HostClient), 292);
}

TEST(HostServer, sizeof)
{
    HostServerImpl hs;
    EXPECT_EQ(sizeof(HostServer), 356);
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
    EXPECT_EQ(size, sizeof(PropertyId) * 3);
}

TEST_F(HostCS, ids_content)
{
    // 添加一个属性值
    Property<float> prop;
    server.put(233, prop);

    PropertyId   id = 0;

    MemoryAccess access;
    access.offset = sizeof(PropertyId) * 3;
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

TEST_F(HostCS, nonce)
{
    PropertyId id = 1;

    for (size_t i = 0; i < server.Nonce.ref().size(); i++)
    {
        server.Nonce.ref()[i] = i;
    }

    Extra extra;
    extra.add(id);
    client.send_request(Command::GET_PROPERTY, extra);

    Poll();

    client._extra.get(id);

    std::array<uint8_t, 12> nonce;
    EXPECT_EQ(nonce.size(), client._extra.remain());
    client._extra.get(nonce.data(), nonce.size());
    EXPECT_TRUE(memcmp(nonce.data(), server.Nonce.ref().data(), nonce.size()) == 0);
}

TEST_F(HostCS, key)
{
    PropertyId                   id = 2;

    std::array<uint8_t, 256 / 8> key;

    for (size_t i = 0; i < key.size(); i++)
    {
        key[i] = i;
    }

    Extra extra;
    extra.reserve_tag();
    extra.add(id);
    extra.add(key.data(), key.size());
    extra.encrypt(server.Nonce, server.Key);
    client.send_request(Command::SET_PROPERTY, extra);

    Poll();

    EXPECT_TRUE(memcmp(key.data(), server.Key.ref().data(), key.size()) == 0);
}