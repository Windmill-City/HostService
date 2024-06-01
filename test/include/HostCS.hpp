#pragma once
#include <CPropertyHolder.hpp>
#include <Extra.hpp>
#include <HostClient.hpp>
#include <HostServer.hpp>

struct HostClientImpl : public HostClient
{
    FixedQueue<1024>* Q_Server;
    FixedQueue<1024>  Q_Client;

    PropertyAddress   addr;

    HostClientImpl(CPropertyHolderBase& holder)
        : HostClient(addr, holder)
    {
    }

    virtual bool rx(uint8_t& byte) override
    {
        while (!Q_Client.pop(&byte))
            std::this_thread::yield();
        return true;
    }

    virtual void tx(const uint8_t* buf, const size_t size) override
    {
        for (size_t i = 0; i < size; i++)
        {
            Q_Server->push(buf[i]);
        }
    }

    virtual void log_output(const char* log, const size_t size)override{}
};

struct HostServerImpl : public HostServer
{
    bool              Running = true;
    FixedQueue<1024>  Q_Server;
    FixedQueue<1024>* Q_Client;

    PropertyAddress   addr;

    HostServerImpl(const PropertyHolderBase& holder, SecretHolder& secret)
        : HostServer(addr, holder, secret)
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

    HostCSBase(const PropertyHolderBase& holder, CPropertyHolderBase& cholder)
        : server(holder, secret)
        , client(cholder)
    {
        server.Q_Client = &client.Q_Client;
        client.Q_Server = &server.Q_Server;

        client.key      = server.secret.key;
        client.nonce    = server.secret.nonce;
    }
};
