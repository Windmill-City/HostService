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

struct TRead
    : public HostCSBase
    , public testing::Test
{
    TRead()
        : HostCSBase(holder)
    {
    }
};

TEST_F(TRead, Read)
{
    Extra extra;
    extra.add<PropertyId>(0);
    client.send_request(Command::GET_PROPERTY, extra);

    Poll();

    EXPECT_EQ(client._rep.error, ErrorCode::S_OK);
}

TEST_F(TRead, Read_Write)
{
    Extra extra;
    extra.add<PropertyId>(1);
    client.send_request(Command::GET_PROPERTY, extra);

    Poll();

    EXPECT_EQ(client._rep.error, ErrorCode::S_OK);
}

TEST_F(TRead, Read_Protect_NotPrivileged)
{
    Extra extra;
    extra.add<PropertyId>(2);
    client.send_request(Command::GET_PROPERTY, extra);

    Poll(false);

    EXPECT_EQ(client._rep.error, ErrorCode::E_NO_PERMISSION);
}

TEST_F(TRead, Read_Protect_Privileged)
{
    Extra extra;
    extra.reserve_tag();
    extra.add<PropertyId>(2);
    extra.encrypt(server._secret.nonce, server._secret.key);
    client.send_request(Command::GET_PROPERTY, extra);

    Poll();

    EXPECT_EQ(client._rep.error, ErrorCode::S_OK);
}

TEST_F(TRead, Write_Protect_NotPrivileged)
{
    Extra extra;
    extra.add<PropertyId>(3);
    client.send_request(Command::GET_PROPERTY, extra);

    Poll();

    EXPECT_EQ(client._rep.error, ErrorCode::S_OK);
}

TEST_F(TRead, Write_Protect_Privileged)
{
    Extra extra;
    extra.reserve_tag();
    extra.add<PropertyId>(3);
    extra.encrypt(server._secret.nonce, server._secret.key);
    client.send_request(Command::GET_PROPERTY, extra);

    Poll();

    EXPECT_EQ(client._rep.error, ErrorCode::S_OK);
}

TEST_F(TRead, Read_Write_Protect_NotPrivileged)
{
    Extra extra;
    extra.add<PropertyId>(4);
    client.send_request(Command::GET_PROPERTY, extra);

    Poll(false);

    EXPECT_EQ(client._rep.error, ErrorCode::E_NO_PERMISSION);
}

TEST_F(TRead, Read_Write_Protect_Privileged)
{
    Extra extra;
    extra.reserve_tag();
    extra.add<PropertyId>(4);
    extra.encrypt(server._secret.nonce, server._secret.key);
    client.send_request(Command::GET_PROPERTY, extra);

    Poll();

    EXPECT_EQ(client._rep.error, ErrorCode::S_OK);
}
