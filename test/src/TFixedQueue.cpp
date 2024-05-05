#include "gtest/gtest.h"
#include <FixedQueue.hpp>

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
