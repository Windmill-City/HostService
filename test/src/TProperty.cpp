#include "gtest/gtest.h"
#include <HostCS.hpp>
#include <Property.hpp>

TEST(Property, sizeof)
{
    EXPECT_EQ(sizeof(Property<bool>), 8);
    EXPECT_EQ(sizeof(Property<float>), 8);
}

TEST(Property, Assign)
{
    Property<bool> prop_1 = true;
    Property<bool> prop_2 = false;

    EXPECT_EQ(prop_1, true);
    EXPECT_EQ(prop_2, false);

    prop_1 = prop_2;
    EXPECT_EQ(prop_1, false);
}

TEST(Property, Calc)
{
    Property<float> prop_1 = 5;
    Property<float> prop_2 = 7;

    EXPECT_EQ(prop_1 + 8, 5 + 8.f);
    EXPECT_EQ(prop_1 - 8, 5 - 8.f);
    EXPECT_EQ(prop_1 * 8, 5 * 8.f);
    EXPECT_EQ(prop_1 / 8, 5 / 8.f);

    EXPECT_EQ(prop_1 + prop_2, 5 + 7.f);
    EXPECT_EQ(prop_1 - prop_2, 5 - 7.f);
    EXPECT_EQ(prop_1 * prop_2, 5 * 7.f);
    EXPECT_EQ(prop_1 / prop_2, 5 / 7.f);
}

static Property<float>          prop;
// 静态初始化
static constexpr PropertyMap<1> map = {{{"prop", &(PropertyBase&)prop}}};
static PropertyHolder holder(map);

struct TProperty
    : public HostCSBase
    , public testing::Test
{
    TProperty()
        : HostCSBase(holder)
    {
    }
};

TEST_F(TProperty, Get)
{
    prop = 18.8f;

    Extra extra;
    ErrorCode err;
    extra.add<PropertyId>(0);
    client.send_request(Command::GET_PROPERTY, extra);

    ASSERT_TRUE(server.poll());
    client.recv_response(Command::GET_PROPERTY, err, client.extra);

    PropertyId id;
    client.extra.get(id);

    float recv;
    client.extra.get(recv);

    EXPECT_EQ(recv, 18.8f);
}

TEST_F(TProperty, Set)
{
    prop = 0.0f;

    Extra extra;
    ErrorCode err;
    extra.add<PropertyId>(0);
    extra.add(18.8f);
    client.send_request(Command::SET_PROPERTY, extra);

    ASSERT_TRUE(server.poll());
    client.recv_response(Command::SET_PROPERTY, err, client.extra);

    EXPECT_EQ(prop, 18.8f);
}

TEST_F(TProperty, GetSize)
{
    Extra extra;
    ErrorCode err;
    extra.add<PropertyId>(0);
    client.send_request(Command::GET_SIZE, extra);

    ASSERT_TRUE(server.poll());
    client.recv_response(Command::GET_SIZE, err, client.extra);

    PropertyId id;
    client.extra.get(id);

    uint16_t size;
    client.extra.get(size);
    EXPECT_EQ(size, sizeof(float));
}
