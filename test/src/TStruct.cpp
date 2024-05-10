#include "gtest/gtest.h"
#include <HostCS.hpp>
#include <Struct.hpp>

struct FloatSt
{
    float val;
};

TEST(Struct, sizeof)
{
    EXPECT_EQ(sizeof(Struct<FloatSt>), 8);
    EXPECT_EQ(sizeof(Struct<FloatSt>), 8);
}

static Struct<FloatSt>          prop;
// 静态初始化
static constexpr PropertyMap<1> map = {
    {"prop", &(PropertyBase&)prop}
};

static PropertyHolder holder(map);

struct TStruct
    : public HostCSBase
    , public testing::Test
{
    TStruct()
        : HostCSBase(holder)
    {
    }
};

TEST_F(TStruct, Get)
{
    prop.ref().val = 18.8f;

    Extra extra;
    extra.add<PropertyId>(0);
    client.send_request(Command::GET_PROPERTY, extra);

    Poll();

    PropertyId id;
    client._extra.get(id);

    float recv;
    client._extra.get(recv);

    EXPECT_EQ(recv, 18.8f);
}

TEST_F(TStruct, Set)
{
    Extra extra;
    extra.add<PropertyId>(0);
    extra.add(18.8f);
    client.send_request(Command::SET_PROPERTY, extra);

    Poll();

    EXPECT_FLOAT_EQ(prop.ref().val, 18.8f);
}

TEST_F(TStruct, GetSize)
{
    Extra extra;
    extra.add<PropertyId>(0);
    client.send_request(Command::GET_SIZE, extra);

    Poll();

    PropertyId id;
    client._extra.get(id);

    uint16_t size;
    client._extra.get(size);
    EXPECT_EQ(size, sizeof(float));
}
