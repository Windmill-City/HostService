#include "gtest/gtest.h"
#include <HostCS.hpp>

TEST(HostClient, sizeof)
{
    EXPECT_EQ(sizeof(HostClient), 276);
}

TEST(HostServer, sizeof)
{
    HostServerImpl hs;
    EXPECT_EQ(sizeof(HostServer), 276);
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
