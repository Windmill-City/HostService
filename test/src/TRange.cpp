#include "gtest/gtest.h"
#include <HostCS.hpp>
#include <Range.hpp>

TEST(RangedProperty, sizeof)
{
    EXPECT_EQ(sizeof(RangedProperty<float, 0, 0>), 16);
    EXPECT_EQ(sizeof(_Range<float>), 8);
}

TEST(RangedProperty, Mode)
{
    RangedProperty<int, 0, 10, RangeMode::Hard>  hard;
    RangedProperty<int, 0, 10, RangeMode::Soft>  soft;
    RangedProperty<int, 0, 10, RangeMode::Clamp> clamp;

    // 测试边界能否赋值
    hard = 10;
    EXPECT_EQ(hard, 10);
    hard = 0;
    EXPECT_EQ(hard, 0);

    // 测试不同模式能否赋值
    hard = soft = 10;
    EXPECT_EQ(hard, 10);
    EXPECT_EQ(soft, 10);

    // 测试是否拦截赋值
    hard = 100;
    EXPECT_EQ(hard, 10);

    // 测试是否允许赋值
    soft = 100;
    EXPECT_EQ(soft, 100);
    // 测试in_range
    EXPECT_FALSE(soft.in_range());

    // 测试in_range
    soft = 7;
    EXPECT_EQ(soft, 7);
    EXPECT_TRUE(soft.in_range());

    // 测试clamp是否正常
    EXPECT_EQ(clamp = 100, 10);
    EXPECT_EQ(clamp = -100, 0);
}

TEST(Range, BoundTest)
{
    Range<float, 0, 0.1> range;

    // 测试是否拦截赋值
    range = _Range<float>{-100, 100};
    EXPECT_FLOAT_EQ(range.min(), 0);
    EXPECT_FLOAT_EQ(range.max(), 0.1);
}

static RangedProperty<float, 0, 100> prop = 18.8f;
using PropertyMap                  = frozen::map<PropertyId, PropertyBase*, 1>;
// 静态初始化
static constexpr PropertyMap map   = {
    {0, &(PropertyBase&)prop}
};

static PropertyHolder holder(map);

struct TRange
    : public HostCSBase
    , public testing::Test
{
    TRange()
        : HostCSBase(holder)
    {
    }
};

TEST_F(TRange, Get)
{
    Extra extra;
    extra.add(0);
    extra.add(RangeAccess::Property);
    client.send_request(Command::GET_PROPERTY, extra);

    Poll();

    PropertyId id;
    client._extra.get(id);

    RangeAccess access;
    client._extra.get(access);

    float value;
    client._extra.get(value);
    EXPECT_EQ(value, 18.8f);
}

TEST_F(TRange, Set)
{
    Extra extra;
    extra.add(0);
    extra.add(RangeAccess::Property);
    extra.add(18.8f);
    client.send_request(Command::SET_PROPERTY, extra);

    Poll();

    EXPECT_EQ(prop, 18.8f);
}

TEST_F(TRange, GetSize)
{
    Extra extra;
    extra.add<PropertyId>(0);
    extra.add(RangeAccess::Property);
    client.send_request(Command::GET_SIZE, extra);

    Poll();

    PropertyId id;
    client._extra.get(id);

    RangeAccess access;
    client._extra.get(access);

    uint16_t size;
    client._extra.get(size);
    EXPECT_EQ(size, sizeof(float));
}
