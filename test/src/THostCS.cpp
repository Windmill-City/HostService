#include "gtest/gtest.h"
#include <HostCS.hpp>

TEST(HostClient, sizeof)
{
    EXPECT_EQ(sizeof(HostClient), 284);
}

TEST(HostServer, sizeof)
{
    HostServerImpl hs;
    EXPECT_EQ(sizeof(HostServer), 296);
}

TEST_F(HostCS, request)
{
    uint8_t data[] = {0x01, 0x02, 0x03};

    Extra   extra{Extra::Type::RAW};
    extra.add(data, sizeof(data));
    client.send_request(Command::ECHO, extra);

    Poll();

    EXPECT_TRUE(memcmp(client._extra.data(), data, sizeof(data)) == 0);
}
