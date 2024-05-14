#include "gtest/gtest.h"
#include <HostCS.hpp>
#include <Range.hpp>

TEST(RangedProperty, sizeof)
{
    EXPECT_EQ(sizeof(RangedProperty<float, 0, 0>), 12);
    EXPECT_EQ(sizeof(RangeVal<float>), 8);
}

TEST(RangedProperty, Mode)
{
    Range<int, 0, 10>                            range;
    RangedProperty<int, 0, 10, RangeMode::Hard>  hard(range);
    RangedProperty<int, 0, 10, RangeMode::Soft>  soft(range);
    RangedProperty<int, 0, 10, RangeMode::Clamp> clamp(range);

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
    range = RangeVal<float>{-100, 100};
    EXPECT_FLOAT_EQ(range.min(), 0);
    EXPECT_FLOAT_EQ(range.max(), 0.1);
}

static Range<float, 0, 100>          prop1;
static RangedProperty<float, 0, 100> prop2(prop1);
// 静态初始化
static constexpr PropertyMap<2>      map = {
    {{"prop1", &(PropertyBase&)prop1}, {"prop2", &(PropertyBase&)prop2}}
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

TEST_F(TRange, Get_Property)
{
    prop2 = 18.8f;

    Extra     extra;
    ErrorCode err;
    extra.add<PropertyId>(1);
    client.send_request(Command::GET_PROPERTY, extra);

    ASSERT_TRUE(server.poll());
    client.recv_response(Command::GET_PROPERTY, err, client.extra);

    PropertyId id;
    client.extra.get(id);

    float value;
    client.extra.get(value);
    EXPECT_EQ(value, 18.8f);
}

TEST_F(TRange, Get_Range_R)
{
    prop1.min() = 10;
    prop1.max() = 80;

    Extra     extra;
    ErrorCode err;
    extra.add<PropertyId>(0);
    extra.add(RangeAccess::Range);
    client.send_request(Command::GET_PROPERTY, extra);

    ASSERT_TRUE(server.poll());
    client.recv_response(Command::GET_PROPERTY, err, client.extra);

    PropertyId id;
    client.extra.get(id);

    RangeAccess access;
    client.extra.get(access);

    float min;
    float max;
    client.extra.get(min);
    client.extra.get(max);
    EXPECT_EQ(min, 10);
    EXPECT_EQ(max, 80);
}

TEST_F(TRange, Get_Absolute_R)
{
    prop1.min() = 10;
    prop1.max() = 80;

    Extra     extra;
    ErrorCode err;
    extra.add<PropertyId>(0);
    extra.add(RangeAccess::Absolute);
    client.send_request(Command::GET_PROPERTY, extra);

    ASSERT_TRUE(server.poll());
    client.recv_response(Command::GET_PROPERTY, err, client.extra);

    PropertyId id;
    client.extra.get(id);

    RangeAccess access;
    client.extra.get(access);

    float min;
    float max;
    client.extra.get(min);
    client.extra.get(max);
    EXPECT_EQ(min, 0);
    EXPECT_EQ(max, 100);
}

TEST_F(TRange, Set_Property)
{
    Extra     extra;
    ErrorCode err;
    extra.add<PropertyId>(1);
    extra.add(18.8f);
    client.send_request(Command::SET_PROPERTY, extra);

    ASSERT_TRUE(server.poll());
    client.recv_response(Command::SET_PROPERTY, err, client.extra);

    EXPECT_EQ(prop2, 18.8f);
}

TEST_F(TRange, Set_Range_R)
{
    prop1.min() = 0;
    prop1.max() = 100;

    Extra     extra;
    ErrorCode err;
    extra.add<PropertyId>(0);
    extra.add(RangeAccess::Range);
    extra.add(10.f);
    extra.add(80.f);
    client.send_request(Command::SET_PROPERTY, extra);

    ASSERT_TRUE(server.poll());
    client.recv_response(Command::SET_PROPERTY, err, client.extra);

    EXPECT_EQ(prop1.min(), 10.f);
    EXPECT_EQ(prop1.max(), 80.f);
}

TEST_F(TRange, Set_Absolute_R)
{
    Extra     extra;
    ErrorCode err;
    extra.add<PropertyId>(0);
    extra.add(RangeAccess::Absolute);
    client.send_request(Command::SET_PROPERTY, extra);

    ASSERT_FALSE(server.poll());
    client.recv_response(Command::SET_PROPERTY, err, client.extra);

    EXPECT_EQ(err, ErrorCode::E_READ_ONLY);
}

TEST_F(TRange, GetSize_Property)
{
    Extra     extra;
    ErrorCode err;
    extra.add<PropertyId>(1);
    client.send_request(Command::GET_SIZE, extra);

    ASSERT_TRUE(server.poll());
    client.recv_response(Command::GET_SIZE, err, client.extra);

    PropertyId id;
    client.extra.get(id);

    uint16_t size;
    client.extra.get(size);
    EXPECT_EQ(size, sizeof(float));
}

TEST_F(TRange, GetSize_Range_R)
{
    Extra     extra;
    ErrorCode err;
    extra.add<PropertyId>(0);
    extra.add(RangeAccess::Range);
    client.send_request(Command::GET_SIZE, extra);

    ASSERT_TRUE(server.poll());
    client.recv_response(Command::GET_SIZE, err, client.extra);

    PropertyId id;
    client.extra.get(id);

    RangeAccess access;
    client.extra.get(access);

    uint16_t size;
    client.extra.get(size);
    EXPECT_EQ(size, 2 * sizeof(float));
}

TEST_F(TRange, GetSize_Absolute_R)
{
    Extra     extra;
    ErrorCode err;
    extra.add<PropertyId>(0);
    extra.add(RangeAccess::Absolute);
    client.send_request(Command::GET_SIZE, extra);

    ASSERT_TRUE(server.poll());
    client.recv_response(Command::GET_SIZE, err, client.extra);

    PropertyId id;
    client.extra.get(id);

    RangeAccess access;
    client.extra.get(access);

    uint16_t size;
    client.extra.get(size);
    EXPECT_EQ(size, 2 * sizeof(float));
}
