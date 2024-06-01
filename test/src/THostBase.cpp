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
    PropertyAddress     addr;
    SecretHolder        secret;

    HostBaseImpl()
        : HostBase(addr, secret)
    {
    }

    virtual bool rx(uint8_t& byte) override
    {
        byte = Q.front();
        Q.pop();
        return true;
    }

    virtual void tx(const void* buf, const size_t size) override
    {
        for (size_t i = 0; i < size; i++)
        {
            Q.push(((uint8_t*)buf)[i]);
        }
    }
};

TEST(HostBase, sizeof)
{
    ASSERT_EQ(sizeof(HostBase), 28);
}

TEST(HostBase, TxRx)
{
    uint8_t      data_tx[] = {0x00, 0x01, 0x02};
    uint8_t      data_rx[sizeof(data_tx)];

    HostBaseImpl hs;

    // 发送数据
    hs.tx(data_tx, sizeof(data_tx));

    // 接收数据
    for (size_t i = 0; i < sizeof(data_tx); i++)
    {
        uint8_t byte;
        hs.rx(byte);
        data_rx[i] = byte;
    }

    // 验证收发数据是否一致
    for (size_t i = 0; i < sizeof(data_tx); i++)
    {
        ASSERT_EQ(data_tx[i], data_rx[i]);
    }
}

TEST(Sync, Sync)
{
    struct Item
    {
        uint8_t item1;
        uint8_t item2;
    };

    // 构造帧头
    Item item;
    item.item1  = 1;
    item.item2  = 2;
    auto chksum = crc_ccitt_ffff((uint8_t*)&item, sizeof(item));
    chksum      = __REV16(chksum);

    // 帧头缓冲区
    Sync<Item> Q;

    // 将帧头放到缓冲区中
    for (size_t i = 0; i < sizeof(Item); i++)
    {
        Q.push(((uint8_t*)&item)[i]);
    }

    // 将校验和放到缓冲区中
    for (size_t i = 0; i < sizeof(Chksum); i++)
    {
        Q.push(((uint8_t*)&chksum)[i]);
    }

    // 验证帧头
    ASSERT_TRUE(Q.verify());

    // 获取帧头
    Item fromQueue = Q.get();
    ASSERT_EQ(fromQueue.item1, item.item1);
    ASSERT_EQ(fromQueue.item2, item.item2);

    // 队列在获取帧头后应当被清空
    ASSERT_EQ(Q.size(), 0);
}