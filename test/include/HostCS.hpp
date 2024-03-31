#pragma once
#include <Extra.hpp>
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

struct HostCS : testing::Test
{
    HostClientImpl client;
    HostServerImpl server;

    virtual void   SetUp() override
    {
        client.Q_Server = &server.Q_Server;
        server.Q_Client = &client.Q_Client;
    }

    /**
     * @brief 轮询处理请求和响应
     *
     * @param fail 请求是否会失败?
     */
    void Poll(bool fail = false)
    {
        for (size_t i = 0; i < sizeof(Request) - 1; i++)
        {
            ASSERT_FALSE(server.poll());
        }
        if (fail)
            ASSERT_FALSE(server.poll());
        else
            ASSERT_TRUE(server.poll());

        for (size_t i = 0; i < sizeof(Response) - 1; i++)
        {
            ASSERT_FALSE(client.poll());
        }
        ASSERT_TRUE(client.poll());
    }
};