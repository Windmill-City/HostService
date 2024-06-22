#include "gtest/gtest.h"
#include <HostCS.hpp>

TEST(Property, sizeof)
{
    EXPECT_EQ(sizeof(Property<bool>), 8);
    EXPECT_EQ(sizeof(Property<float>), 8);
}

static float                               FloatVal;
static Property<float, Access::READ_WRITE> Prop_1(FloatVal);
// 静态初始化
static constexpr PropertyMap<1>            Map = {
    {
     {"prop.1", &(PropertyBase&)Prop_1},
     }
};
static PropertyHolder            Holder(Map);

static constinit CPropertyMap<1> CMap = {
    {
     {"prop.1", 0},
     }
};
static CPropertyHolder CHolder(CMap);

struct TProperty
    : public HostCSBase
    , public testing::Test
{
    TProperty()
        : HostCSBase(Holder, CHolder)
    {
    }
};

TEST_F(TProperty, Get)
{
    FloatVal = 18.8f;

    Extra     extra;
    ErrorCode err;
    extra.add<PropertyId>(0);
    client.send(Command::GET_PROPERTY, extra);

    ASSERT_TRUE(server.poll());
    client.recv_response(Command::GET_PROPERTY, err, client.extra);

    float recv;
    client.extra.get(recv);

    EXPECT_EQ(recv, 18.8f);
}

TEST_F(TProperty, Set)
{
    FloatVal = 0.0f;

    Extra     extra;
    ErrorCode err;
    extra.add<PropertyId>(0);
    extra.add(18.8f);
    client.send(Command::SET_PROPERTY, extra);

    ASSERT_TRUE(server.poll());
    client.recv_response(Command::SET_PROPERTY, err, client.extra);

    EXPECT_EQ(FloatVal, 18.8f);
}

TEST_F(TProperty, GetSize)
{
    Extra     extra;
    ErrorCode err;
    extra.add<PropertyId>(0);
    client.send(Command::GET_SIZE, extra);

    ASSERT_TRUE(server.poll());
    client.recv_response(Command::GET_SIZE, err, client.extra);

    uint16_t size;
    client.extra.get(size);
    EXPECT_EQ(size, sizeof(float));
}
