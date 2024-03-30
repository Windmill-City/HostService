#include "gtest/gtest.h"
#include <HostClient.hpp>
#include <HostServer.hpp>
#include <queue>

struct HostClientImpl : public HostClient
{
    std::queue<uint8_t>* Q_Server;
    std::queue<uint8_t>  Q_Client;

    virtual uint8_t      rx() override
    {
        uint8_t res = Q_Client.front();
        Q_Client.pop();
        return res;
    }

    virtual void tx(const uint8_t* buf, const size_t size) override
    {
        for (size_t i = 0; i < size; i++)
        {
            Q_Server->push(buf[i]);
        }
    }
};

struct HostServerImpl : public HostServer
{
    std::queue<uint8_t>  Q_Server;
    std::queue<uint8_t>* Q_Client;

    virtual uint8_t      rx() override
    {
        uint8_t res = Q_Server.front();
        Q_Server.pop();
        return res;
    }

    virtual void tx(const uint8_t* buf, const size_t size) override
    {
        for (size_t i = 0; i < size; i++)
        {
            Q_Client->push(buf[i]);
        }
    }
};

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

TEST(HostCS, request)
{
}
