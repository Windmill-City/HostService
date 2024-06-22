#include "gtest/gtest.h"
#include <HostCS.hpp>
#include <Range.hpp>

TEST(RangeVal, sizeof)
{
    EXPECT_EQ(sizeof(RangeVal<float>), 8);
}

static RangeVal<float>          RangeVal_1{0, 100};
static Range                    Prop_1(RangeVal_1, {0, 100});
// 静态初始化
static constexpr PropertyMap<1> Map = {
    {
     {"prop.1", &(PropertyBase&)Prop_1},
     }
};
static PropertyHolder            Holder(Map);

static constinit CPropertyMap<1> CMap = {
    {
     {"prop1", 0},
     }
};
static CPropertyHolder CHolder(CMap);

struct TRange
    : public HostCSBase
    , public testing::Test
{
    TRange()
        : HostCSBase(Holder, CHolder)
    {
    }
};

TEST_F(TRange, Get_Range)
{
    RangeVal_1.min = 10;
    RangeVal_1.max = 80;

    Extra     extra;
    ErrorCode err;
    extra.add<PropertyId>(0);
    extra.add(RangeAccess::Range);
    client.send(Command::GET_PROPERTY, extra);

    ASSERT_TRUE(server.poll());
    client.recv_response(Command::GET_PROPERTY, err, client.extra);

    float min;
    float max;
    client.extra.get(min);
    client.extra.get(max);
    EXPECT_EQ(min, 10);
    EXPECT_EQ(max, 80);
}

TEST_F(TRange, Get_Absolute)
{
    RangeVal_1.min = 10;
    RangeVal_1.max = 80;

    Extra     extra;
    ErrorCode err;
    extra.add<PropertyId>(0);
    extra.add(RangeAccess::Absolute);
    client.send(Command::GET_PROPERTY, extra);

    ASSERT_TRUE(server.poll());
    client.recv_response(Command::GET_PROPERTY, err, client.extra);

    float min;
    float max;
    client.extra.get(min);
    client.extra.get(max);
    EXPECT_EQ(min, 0);
    EXPECT_EQ(max, 100);
}

TEST_F(TRange, Set_Range)
{
    RangeVal_1.min = 0;
    RangeVal_1.max = 100;

    Extra     extra;
    ErrorCode err;
    extra.add<PropertyId>(0);
    extra.add(RangeAccess::Range);
    extra.add(10.f);
    extra.add(80.f);
    client.send(Command::SET_PROPERTY, extra);

    ASSERT_TRUE(server.poll());
    client.recv_response(Command::SET_PROPERTY, err, client.extra);

    EXPECT_EQ(RangeVal_1.min, 10.f);
    EXPECT_EQ(RangeVal_1.max, 80.f);
}

TEST_F(TRange, Set_Absolute)
{
    Extra     extra;
    ErrorCode err;
    extra.add<PropertyId>(0);
    extra.add(RangeAccess::Absolute);
    client.send(Command::SET_PROPERTY, extra);

    ASSERT_FALSE(server.poll());
    client.recv_response(Command::SET_PROPERTY, err, client.extra);

    EXPECT_EQ(err, ErrorCode::E_READ_ONLY);
}

TEST_F(TRange, GetSize)
{
    Extra     extra;
    ErrorCode err;
    extra.add<PropertyId>(0);
    client.send(Command::GET_SIZE, extra);

    ASSERT_TRUE(server.poll());
    client.recv_response(Command::GET_SIZE, err, client.extra);

    Size size;
    client.extra.get(size);
    EXPECT_EQ(size, 2 * sizeof(float));
}

TEST_F(TRange, GetAccess)
{
    Extra     extra;
    ErrorCode err;
    extra.add<PropertyId>(0);
    client.send(Command::GET_ACCESS, extra);

    ASSERT_TRUE(server.poll());
    client.recv_response(Command::GET_ACCESS, err, client.extra);

    Access access;
    client.extra.get((uint8_t&)access);
    EXPECT_EQ(access, Access::READ_WRITE);
}
