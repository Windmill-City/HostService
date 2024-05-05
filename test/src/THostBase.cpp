#include "gtest/gtest.h"
#include <Extra.hpp>
#include <HostBase.hpp>
#include <queue>

#if defined(__GNUC__) || defined(__clang__)
  #define __REV16(number) (__builtin_bswap16(number))
#else
  #define __REV16(number) (((number) >> 8) | ((number) << 8));
#endif

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

TEST(Sync, header)
{
    struct Item
    {
        uint8_t  item1;
        uint8_t  item2;
        uint16_t chksum;
    };

    Item item;
    item.item1  = 1;
    item.item2  = 2;
    item.chksum = crc_ccitt_ffff((uint8_t*)&item, sizeof(item) - sizeof(uint16_t));
    item.chksum = __REV16(item.chksum);

    Sync<Item> Q;

    for (size_t i = 0; i < Q.capacity(); i++)
    {
        Q.push(((uint8_t*)&item)[i]);
    }

    ASSERT_TRUE(Q.verify());

    Item fromQueue = Q.get();
    ASSERT_EQ(fromQueue.item1, item.item1);
    ASSERT_EQ(fromQueue.item2, item.item2);
    ASSERT_EQ(fromQueue.chksum, item.chksum);

    ASSERT_EQ(Q.size(), 0);
}