#pragma once
#include <CPropertyHolder.hpp>
#include <Extra.hpp>
#include <HostClient.hpp>
#include <HostServer.hpp>
#include <thread>

struct SecretHolderImpl : public SecretHolder
{
    virtual void update_nonce() override
    {
    }
};

struct HostClientImpl : public HostClient
{
    Address           address;
    FixedQueue<2048>* Q_Server;
    FixedQueue<2048>  Q_Client;

    HostClientImpl(CPropertyHolderBase& holder, SecretHolder& secret)
        : HostClient(address, holder, secret)
    {
    }

    virtual bool rx(uint8_t& byte) override
    {
        while (!Q_Client.pop(&byte))
            std::this_thread::yield();
        return true;
    }

    virtual void tx(const void* buf, size_t size) override
    {
        for (size_t i = 0; i < size; i++)
        {
            Q_Server->push(((uint8_t*)buf)[i]);
        }
    }

    virtual void log_output(LogLevel, const uint8_t*, size_t) override
    {
    }
};

struct HostServerImpl : public HostServer
{
    Address           address;
    bool              Running = true;
    FixedQueue<2048>  Q_Server;
    FixedQueue<2048>* Q_Client;

    HostServerImpl(const PropertyHolderBase& holder, SecretHolder& secret)
        : HostServer(address, holder, secret)
    {
    }

    virtual bool rx(uint8_t& byte) override
    {
        if (Running)
        {
            while (Running && !Q_Server.pop(&byte))
                std::this_thread::yield();
            return true;
        }
        else
            return false;
    }

    virtual void tx(const void* buf, size_t size) override
    {
        for (size_t i = 0; i < size; i++)
        {
            Q_Client->push(((uint8_t*)buf)[i]);
        }
    }
};

struct HostCSBase
{
    SecretHolderImpl secret;
    HostServerImpl   server;
    HostClientImpl   client;

    HostCSBase(const PropertyHolderBase& holder, CPropertyHolderBase& cholder)
        : server(holder, secret)
        , client(cholder, secret)
    {
        server.Q_Client = &client.Q_Client;
        client.Q_Server = &server.Q_Server;
    }
};
