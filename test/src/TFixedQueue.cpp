#include "gtest/gtest.h"
#include <FixedQueue.hpp>

#if defined(__GNUC__) || defined(__clang__)
  #define __REV16(number) (__builtin_bswap16(number))
#else
  #define __REV16(number) (((number) >> 8) | ((number) << 8));
#endif

TEST(FixedQueue, NoPop)
{
    FixedQueue<255> Q;

    // 检查初始状态
    ASSERT_EQ(Q.capacity(), 255);
    ASSERT_EQ(Q.size(), 0);
    ASSERT_TRUE(Q.empty());
    ASSERT_FALSE(Q.full());

    Q.reset();
    // 检查复位状态
    ASSERT_EQ(Q.capacity(), 255);
    ASSERT_EQ(Q.size(), 0);
    ASSERT_TRUE(Q.empty());
    ASSERT_FALSE(Q.full());

    // 将队列放满
    for (size_t i = 0; i < Q.capacity(); i++)
    {
        ASSERT_TRUE(Q.push(i));
    }
    // 检查是否还能继续放
    ASSERT_FALSE(Q.push(0));
    // 检查元素数量是否正确
    ASSERT_EQ(Q.size(), 255);

    // 检查数组下标访问是否正常
    for (size_t i = 0; i < Q.capacity(); i++)
    {
        ASSERT_EQ(Q[i], i);
    }

    // 将队列读空
    for (size_t i = 0; i < Q.capacity(); i++)
    {
        uint8_t item;
        ASSERT_TRUE(Q.pop(&item));
        ASSERT_EQ(item, i);
    }
    // 检查是否还能继续读
    ASSERT_FALSE(Q.pop());
    // 检查元素数量是否正确
    ASSERT_EQ(Q.size(), 0);
}

TEST(FixedQueue, PopOnPush)
{
    FixedQueue<255, PopAction::PopOnPush> Q;

    // 检查初始状态
    ASSERT_EQ(Q.capacity(), 255);
    ASSERT_EQ(Q.size(), 0);
    ASSERT_TRUE(Q.empty());
    ASSERT_FALSE(Q.full());

    Q.reset();
    // 检查复位状态
    ASSERT_EQ(Q.capacity(), 255);
    ASSERT_EQ(Q.size(), 0);
    ASSERT_TRUE(Q.empty());
    ASSERT_FALSE(Q.full());

    // 将队列放满
    for (size_t i = 0; i < Q.capacity(); i++)
    {
        ASSERT_TRUE(Q.push(i));
    }
    // 检查是否还能继续放
    ASSERT_TRUE(Q.push(0));
    // 检查元素数量是否正确
    ASSERT_EQ(Q.size(), 255);
    // 元素 0 已被弹出, 接下来是元素 1
    ASSERT_EQ(Q[0], 1);
}

TEST(FixedQueue, asHeader)
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

    FixedQueue<sizeof(Item)> Q;

    for (size_t i = 0; i < Q.capacity(); i++)
    {
        Q.push(((uint8_t*)&item)[i]);
    }

    ASSERT_TRUE(Q.verify<Item>());

    Item fromQueue = Q.as<Item>();
    ASSERT_EQ(fromQueue.item1, item.item1);
    ASSERT_EQ(fromQueue.item2, item.item2);
    ASSERT_EQ(fromQueue.chksum, item.chksum);

    ASSERT_EQ(Q.size(), 0);
}