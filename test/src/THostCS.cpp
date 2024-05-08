#include "gtest/gtest.h"
#include <HostCS.hpp>
#include <Property.hpp>

TEST(Request, sizeof)
{
    ASSERT_EQ(sizeof(Request), 5);
}

TEST(Response, sizeof)
{
    ASSERT_EQ(sizeof(Response), 6);
}

TEST(HostServer, frozen)
{
    // 验证 frozen map 是否能正常编译
    HostServerImpl<1> Server({
        {3, (PropertyBase*)8}
    });

    // 验证map大小是否正确
    ASSERT_EQ(3, Server._props.size());
    // 验证元素是否正确
    ASSERT_EQ(8, (uint32_t)Server.get(3));
}

TEST(HostCS, request)
{
    HostCS<1> cs({
        {1, (PropertyBase*)8}
    });

    uint8_t   data[] = {0x01, 0x02, 0x03};

    Extra     extra;
    extra.add(data, sizeof(data));
    cs.client.send_request(Command::ECHO, extra);

    cs.Poll();

    ASSERT_TRUE(memcmp(cs.client._extra.data(), data, sizeof(data)) == 0);
}

TEST(HostCS, nonce)
{
    HostCS     cs;

    PropertyId id = 1;

    for (size_t i = 0; i < cs.server.Nonce.ref().size(); i++)
    {
        cs.server.Nonce.ref()[i] = i;
    }

    Extra extra;
    extra.add(id);
    cs.client.send_request(Command::GET_PROPERTY, extra);

    cs.Poll();

    cs.client._extra.get(id);

    std::array<uint8_t, 12> nonce;
    ASSERT_EQ(nonce.size(), cs.client._extra.remain());
    cs.client._extra.get(nonce.data(), nonce.size());
    ASSERT_TRUE(memcmp(nonce.data(), cs.server.Nonce.ref().data(), nonce.size()) == 0);
}

TEST(HostCS, key)
{
    HostCS                       cs;

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
    extra.encrypt(cs.server.Nonce, cs.server.Key);
    cs.client.send_request(Command::SET_PROPERTY, extra);

    cs.Poll();

    ASSERT_TRUE(memcmp(key.data(), cs.server.Key.ref().data(), key.size()) == 0);
}