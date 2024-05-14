#pragma once
#include <Extra.hpp>
#include <HostClient.hpp>
#include <HostServer.hpp>

#include <queue>

struct HostClientImpl : public HostClient
{
    FixedQueue<1024>* Q_Server;
    FixedQueue<1024>  Q_Client;

    PropertyAddress   addr;

    HostClientImpl()
        : HostClient(addr)
    {
    }

    virtual bool rx(uint8_t& byte) override
    {
        Q_Client.pop(&byte);
        return true;
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
    FixedQueue<1024>  Q_Server;
    FixedQueue<1024>* Q_Client;

    PropertyAddress   addr;

    HostServerImpl(const PropertyHolderBase& holder, SecretHolder& secret)
        : HostServer(addr, holder, secret)
    {
    }

    virtual bool rx(uint8_t& byte) override
    {
        Q_Server.pop(&byte);
        return true;
    }

    virtual void tx(const uint8_t* buf, const size_t size) override
    {
        for (size_t i = 0; i < size; i++)
        {
            Q_Client->push(buf[i]);
        }
    }
};

struct HostCSBase
{
    SecretHolder   secret;
    HostServerImpl server;
    HostClientImpl client;

    HostCSBase(const PropertyHolderBase& holder)
        : server(holder, secret)
    {
        server.Q_Client = &client.Q_Client;
        client.Q_Server = &server.Q_Server;
    }
};
