#include "gtest/gtest.h"
#include <Extra.hpp>
#include <HostBase.hpp>
#include <queue>

struct HostBaseImpl : public HostBase
{
    std::queue<uint8_t> Q;

    virtual bool        poll() override
    {
        return true;
    }

    virtual uint8_t rx() override
    {
        uint8_t res = Q.front();
        Q.pop();
        return res;
    }

    virtual void tx(const uint8_t* buf, const size_t size) override
    {
        for (size_t i = 0; i < size; i++)
        {
            Q.push(buf[i]);
        }
    }
};

TEST(HostBase, sizeof)
{
    EXPECT_EQ(sizeof(HostBase), 8);
}

TEST(HostBase, tx_rx)
{
    uint8_t      data_tx[] = {0x00, 0x01, 0x02};
    uint8_t      data_rx[sizeof(data_tx)];

    HostBaseImpl hs;

    hs.tx(data_tx, sizeof(data_tx));

    for (size_t i = 0; i < sizeof(data_tx); i++)
    {
        data_rx[i] = hs.rx();
    }

    for (size_t i = 0; i < sizeof(data_tx); i++)
    {
        EXPECT_EQ(data_tx[i], data_rx[i]);
    }
}
