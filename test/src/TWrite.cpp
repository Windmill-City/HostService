#include "gtest/gtest.h"
#include <HostCS.hpp>
#include <Property.hpp>

static Property<float, Access::READ>               prop1;
static Property<float, Access::READ_WRITE>         prop2;
static Property<float, Access::READ_PROTECT>       prop3;
static Property<float, Access::WRITE_PROTECT>      prop4;
static Property<float, Access::READ_WRITE_PROTECT> prop5;
// 静态初始化
static constexpr PropertyMap<5>                    map = {
    {{"prop1", &(PropertyBase&)prop1},
     {"prop2", &(PropertyBase&)prop2},
     {"prop3", &(PropertyBase&)prop3},
     {"prop4", &(PropertyBase&)prop4},
     {"prop5", &(PropertyBase&)prop5}}
};
static PropertyHolder holder(map);

struct TWrite
    : public HostCSBase
    , public testing::Test
{
    TWrite()
        : HostCSBase(holder)
    {
    }
};

TEST_F(TWrite, Read)
{
    Extra     extra;
    ErrorCode err;
    extra.add<PropertyId>(0);
    extra.add(0.0f);
    client.send_request(Command::SET_PROPERTY, extra);

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
    client.send_request(Command::SET_PROPERTY, extra);

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
    client.send_request(Command::SET_PROPERTY, extra);

    ASSERT_FALSE(server.poll());
    client.recv_response(Command::SET_PROPERTY, err, client.extra);

    EXPECT_EQ(err, ErrorCode::E_NO_PERMISSION);
}

TEST_F(TWrite, Read_Protect_Privileged)
{
    Extra     extra;
    ErrorCode err;
    extra.reserve_tag();
    extra.add<PropertyId>(2);
    extra.add(0.0f);
    client.send_request(Command::SET_PROPERTY, extra, true);

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
    client.send_request(Command::SET_PROPERTY, extra);

    ASSERT_FALSE(server.poll());
    client.recv_response(Command::SET_PROPERTY, err, client.extra);

    EXPECT_EQ(err, ErrorCode::E_NO_PERMISSION);
}

TEST_F(TWrite, Write_Protect_Privileged)
{
    Extra     extra;
    ErrorCode err;
    extra.reserve_tag();
    extra.add<PropertyId>(3);
    extra.add(0.0f);
    client.send_request(Command::SET_PROPERTY, extra, true);

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
    client.send_request(Command::SET_PROPERTY, extra);

    ASSERT_FALSE(server.poll());
    client.recv_response(Command::SET_PROPERTY, err, client.extra);

    EXPECT_EQ(err, ErrorCode::E_NO_PERMISSION);
}

TEST_F(TWrite, Read_Write_Protect_Privileged)
{
    Extra     extra;
    ErrorCode err;
    extra.reserve_tag();
    extra.add<PropertyId>(4);
    extra.add(0.0f);
    client.send_request(Command::SET_PROPERTY, extra, true);

    ASSERT_TRUE(server.poll());
    client.recv_response(Command::SET_PROPERTY, err, client.extra);

    EXPECT_EQ(err, ErrorCode::S_OK);
}
