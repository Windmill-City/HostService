#pragma once
#include <Extra.hpp>
#include <HostClient.hpp>
#include <HostServer.hpp>

#include <queue>

struct HostClientImpl : public HostClient
{
    FixedQueue<255>* Q_Server;
    FixedQueue<255>  Q_Client;

    virtual uint8_t  rx() override
    {
        uint8_t res;
        Q_Client.pop(&res);
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

template <size_t _size>
struct HostServerImpl : public HostServer<_size>
{
    using parent = HostServer<_size>;

    FixedQueue<255>  Q_Server;
    FixedQueue<255>* Q_Client;

    HostServerImpl(std::initializer_list<Item> items)
        : parent(items)
    {
    }

    virtual uint8_t rx() override
    {
        uint8_t res;
        Q_Server.pop(&res);
        return res;
    }

    virtual void tx(const uint8_t* buf, const size_t size) override
    {
        for (size_t i = 0; i < size; i++)
        {
            Q_Client->push(buf[i]);
        }
    }

    virtual void update_nonce() override
    {
    }
};

template <size_t _size = 0>
struct HostCS
{
    HostServerImpl<_size> server;
    HostClientImpl        client;

    HostCS(std::initializer_list<Item> items = {})
        : server(items)
    {
        server.Q_Client = &client.Q_Client;
        client.Q_Server = &server.Q_Server;
    }

    /**
     * @brief 轮询处理请求和响应
     *
     * @param ok 请求是否会失败?
     */
    void Poll(bool ok = true)
    {
        for (size_t i = 0; i < sizeof(Request) - 1; i++)
        {
            ASSERT_FALSE(server.poll());
        }
        if (ok)
            ASSERT_TRUE(server.poll());
        else
            ASSERT_FALSE(server.poll());

        for (size_t i = 0; i < sizeof(Response) - 1; i++)
        {
            ASSERT_FALSE(client.poll());
        }
        ASSERT_TRUE(client.poll());
    }
};
