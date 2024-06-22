#include "gtest/gtest.h"
#include <HostCS.hpp>
#include <Property.hpp>

static float                                       FloatVal;
static Property<float, Access::READ>               Prop_1(FloatVal);
static Property<float, Access::READ_WRITE>         Prop_2(FloatVal);
static Property<float, Access::READ_PROTECT>       Prop_3(FloatVal);
static Property<float, Access::WRITE_PROTECT>      Prop_4(FloatVal);
static Property<float, Access::READ_WRITE_PROTECT> Prop_5(FloatVal);
// 静态初始化
static constexpr PropertyMap<5>                    map = {
    {
     {"prop1", &(PropertyBase&)Prop_1},
     {"prop2", &(PropertyBase&)Prop_2},
     {"prop3", &(PropertyBase&)Prop_3},
     {"prop4", &(PropertyBase&)Prop_4},
     {"prop5", &(PropertyBase&)Prop_5},
     }
};
static PropertyHolder            holder(map);

static constinit CPropertyMap<1> cmap = {
    {
     {"prop1", 0},
     {"prop2", 1},
     {"prop3", 2},
     {"prop4", 3},
     {"prop5", 4},
     }
};
static CPropertyHolder cholder(cmap);

struct TWrite
    : public HostCSBase
    , public testing::Test
{
    TWrite()
        : HostCSBase(holder, cholder)
    {
    }
};

TEST_F(TWrite, Read)
{
    Extra     extra;
    ErrorCode err;
    extra.add<PropertyId>(0);
    extra.add(0.0f);
    client.send(Command::SET_PROPERTY, extra);

    ASSERT_FALSE(server.poll());
    client.recv_response(Command::SET_PROPERTY, err, client.extra);

    EXPECT_EQ(err, ErrorCode::E_READ_ONLY);
}

TEST_F(TWrite, Read_Write)
{
    Extra     extra;
    ErrorCode err;
    extra.add<PropertyId>(1);
    extra.add(0.0f);
    client.send(Command::SET_PROPERTY, extra);

    ASSERT_TRUE(server.poll());
    client.recv_response(Command::SET_PROPERTY, err, client.extra);

    EXPECT_EQ(err, ErrorCode::S_OK);
}

TEST_F(TWrite, Read_Protect_NotPrivileged)
{
    Extra     extra;
    ErrorCode err;
    extra.add<PropertyId>(2);
    extra.add(0.0f);
    client.send(Command::SET_PROPERTY, extra);

    ASSERT_FALSE(server.poll());
    client.recv_response(Command::SET_PROPERTY, err, client.extra);

    EXPECT_EQ(err, ErrorCode::E_NO_PERMISSION);
}

TEST_F(TWrite, Read_Protect_Privileged)
{
    Extra     extra;
    ErrorCode err;
    extra.add<PropertyId>(2);
    extra.add(0.0f);
    client.send(Command::SET_PROPERTY, extra, true);

    ASSERT_FALSE(server.poll());
    client.recv_response(Command::SET_PROPERTY, err, client.extra);

    EXPECT_EQ(err, ErrorCode::E_READ_ONLY);
}

TEST_F(TWrite, Write_Protect_NotPrivileged)
{
    Extra     extra;
    ErrorCode err;
    extra.add<PropertyId>(3);
    extra.add(0.0f);
    client.send(Command::SET_PROPERTY, extra);

    ASSERT_FALSE(server.poll());
    client.recv_response(Command::SET_PROPERTY, err, client.extra);

    EXPECT_EQ(err, ErrorCode::E_NO_PERMISSION);
}

TEST_F(TWrite, Write_Protect_Privileged)
{
    Extra     extra;
    ErrorCode err;
    extra.add<PropertyId>(3);
    extra.add(0.0f);
    client.send(Command::SET_PROPERTY, extra, true);

    ASSERT_TRUE(server.poll());
    client.recv_response(Command::SET_PROPERTY, err, client.extra);

    EXPECT_EQ(err, ErrorCode::S_OK);
}

TEST_F(TWrite, Read_Write_Protect_NotPrivileged)
{
    Extra     extra;
    ErrorCode err;
    extra.add<PropertyId>(4);
    extra.add(0.0f);
    client.send(Command::SET_PROPERTY, extra);

    ASSERT_FALSE(server.poll());
    client.recv_response(Command::SET_PROPERTY, err, client.extra);

    EXPECT_EQ(err, ErrorCode::E_NO_PERMISSION);
}

TEST_F(TWrite, Read_Write_Protect_Privileged)
{
    Extra     extra;
    ErrorCode err;
    extra.add<PropertyId>(4);
    extra.add(0.0f);
    client.send(Command::SET_PROPERTY, extra, true);

    ASSERT_TRUE(server.poll());
    client.recv_response(Command::SET_PROPERTY, err, client.extra);

    EXPECT_EQ(err, ErrorCode::S_OK);
}
