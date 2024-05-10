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

static RangedProperty<float, 0, 100> prop1;
static Range<float, 0, 100>          prop2;
using PropertyMap                = frozen::map<frozen::string, PropertyBase*, 2>;
// 静态初始化
static constexpr PropertyMap map = {
    {"prop1", &(PropertyBase&)prop1},
    {"prop2", &(PropertyBase&)prop2},
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
    prop1 = 18.8f;

    Extra extra;
    extra.add<PropertyId>(0);
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

TEST_F(TRange, Get_Range_P)
{
    prop1.min() = 10;
    prop1.max() = 80;

    Extra extra;
    extra.add<PropertyId>(0);
    extra.add(RangeAccess::Range);
    client.send_request(Command::GET_PROPERTY, extra);

    Poll();

    PropertyId id;
    client._extra.get(id);

    RangeAccess access;
    client._extra.get(access);

    float min;
    float max;
    client._extra.get(min);
    client._extra.get(max);
    EXPECT_EQ(min, 10);
    EXPECT_EQ(max, 80);
}

TEST_F(TRange, Get_Range_R)
{
    prop2.min() = 10;
    prop2.max() = 80;

    Extra extra;
    extra.add<PropertyId>(1);
    extra.add(RangeAccess::Range);
    client.send_request(Command::GET_PROPERTY, extra);

    Poll();

    PropertyId id;
    client._extra.get(id);

    RangeAccess access;
    client._extra.get(access);

    float min;
    float max;
    client._extra.get(min);
    client._extra.get(max);
    EXPECT_EQ(min, 10);
    EXPECT_EQ(max, 80);
}

TEST_F(TRange, Get_Absolute_P)
{
    prop1.min() = 10;
    prop1.max() = 80;

    Extra extra;
    extra.add<PropertyId>(0);
    extra.add(RangeAccess::Absolute);
    client.send_request(Command::GET_PROPERTY, extra);

    Poll();

    PropertyId id;
    client._extra.get(id);

    RangeAccess access;
    client._extra.get(access);

    float min;
    float max;
    client._extra.get(min);
    client._extra.get(max);
    EXPECT_EQ(min, 0);
    EXPECT_EQ(max, 100);
}

TEST_F(TRange, Get_Absolute_R)
{
    prop2.min() = 10;
    prop2.max() = 80;

    Extra extra;
    extra.add<PropertyId>(1);
    extra.add(RangeAccess::Absolute);
    client.send_request(Command::GET_PROPERTY, extra);

    Poll();

    PropertyId id;
    client._extra.get(id);

    RangeAccess access;
    client._extra.get(access);

    float min;
    float max;
    client._extra.get(min);
    client._extra.get(max);
    EXPECT_EQ(min, 0);
    EXPECT_EQ(max, 100);
}

TEST_F(TRange, Set_Property)
{
    Extra extra;
    extra.add<PropertyId>(0);
    extra.add(RangeAccess::Property);
    extra.add(18.8f);
    client.send_request(Command::SET_PROPERTY, extra);

    Poll();

    EXPECT_EQ(prop1, 18.8f);
}

TEST_F(TRange, Set_Range_P)
{
    prop1.min() = 0;
    prop1.max() = 100;

    Extra extra;
    extra.add<PropertyId>(0);
    extra.add(RangeAccess::Range);
    extra.add(10.f);
    extra.add(80.f);
    client.send_request(Command::SET_PROPERTY, extra);

    Poll();

    EXPECT_EQ(prop1.min(), 10.f);
    EXPECT_EQ(prop1.max(), 80.f);
}

TEST_F(TRange, Set_Range_R)
{
    prop2.min() = 0;
    prop2.max() = 100;

    Extra extra;
    extra.add<PropertyId>(1);
    extra.add(RangeAccess::Range);
    extra.add(10.f);
    extra.add(80.f);
    client.send_request(Command::SET_PROPERTY, extra);

    Poll();

    EXPECT_EQ(prop2.min(), 10.f);
    EXPECT_EQ(prop2.max(), 80.f);
}

TEST_F(TRange, Set_Absolute_P)
{
    Extra extra;
    extra.add<PropertyId>(0);
    extra.add(RangeAccess::Absolute);
    client.send_request(Command::SET_PROPERTY, extra);

    Poll(false);

    EXPECT_EQ(client._rep.error, ErrorCode::E_READ_ONLY);
}

TEST_F(TRange, Set_Absolute_R)
{
    Extra extra;
    extra.add<PropertyId>(1);
    extra.add(RangeAccess::Absolute);
    client.send_request(Command::SET_PROPERTY, extra);

    Poll(false);

    EXPECT_EQ(client._rep.error, ErrorCode::E_READ_ONLY);
}

TEST_F(TRange, GetSize_Property)
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

TEST_F(TRange, GetSize_Range_P)
{
    Extra extra;
    extra.add<PropertyId>(0);
    extra.add(RangeAccess::Range);
    client.send_request(Command::GET_SIZE, extra);

    Poll();

    PropertyId id;
    client._extra.get(id);

    RangeAccess access;
    client._extra.get(access);

    uint16_t size;
    client._extra.get(size);
    EXPECT_EQ(size, 2 * sizeof(float));
}

TEST_F(TRange, GetSize_Range_R)
{
    Extra extra;
    extra.add<PropertyId>(1);
    extra.add(RangeAccess::Range);
    client.send_request(Command::GET_SIZE, extra);

    Poll();

    PropertyId id;
    client._extra.get(id);

    RangeAccess access;
    client._extra.get(access);

    uint16_t size;
    client._extra.get(size);
    EXPECT_EQ(size, 2 * sizeof(float));
}

TEST_F(TRange, GetSize_Absolute_P)
{
    Extra extra;
    extra.add<PropertyId>(0);
    extra.add(RangeAccess::Absolute);
    client.send_request(Command::GET_SIZE, extra);

    Poll();

    PropertyId id;
    client._extra.get(id);

    RangeAccess access;
    client._extra.get(access);

    uint16_t size;
    client._extra.get(size);
    EXPECT_EQ(size, 2 * sizeof(float));
}

TEST_F(TRange, GetSize_Absolute_R)
{
    Extra extra;
    extra.add<PropertyId>(1);
    extra.add(RangeAccess::Absolute);
    client.send_request(Command::GET_SIZE, extra);

    Poll();

    PropertyId id;
    client._extra.get(id);

    RangeAccess access;
    client._extra.get(access);

    uint16_t size;
    client._extra.get(size);
    EXPECT_EQ(size, 2 * sizeof(float));
}
