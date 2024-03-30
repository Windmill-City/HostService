#include "gtest/gtest.h"
#include <HostCS.hpp>

TEST(HostClient, sizeof)
{
    EXPECT_EQ(sizeof(HostClient), UINT8_MAX + sizeof(Response) + sizeof(HostBase) + 3);
}

TEST(HostServer, sizeof)
{
    HostServerImpl hs;
    EXPECT_EQ(sizeof(HostServer),
              UINT8_MAX + sizeof(Request) + sizeof(HostBase) + sizeof(PropertyHolder) + sizeof(bool) + 3);
}

TEST_F(HostCS, request)
{
    uint8_t        data[] = {0x01, 0x02, 0x03};

    RequestBuilder builder;
    builder.add(data, sizeof(data));
    builder.tx(client, Command::ECHO);

    Poll();

    EXPECT_TRUE(memcmp(client._extra, data, sizeof(data)) == 0);
}
