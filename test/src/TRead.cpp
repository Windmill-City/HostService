#include "gtest/gtest.h"
#include <HostCS.hpp>
#include <Property.hpp>

TEST_F(HostCS, Read_Read)
{
    Property<bool, Access::READ> prop{true};
    server.put(0x01, prop);

    Extra extra;
    extra.add<PropertyId>(0x01);
    client.send_request(Command::GET_PROPERTY, extra);

    Poll();

    EXPECT_EQ(client._rep.error, ErrorCode::S_OK);
}

TEST_F(HostCS, Read_Read_Protect_NotPrivileged)
{
    Property<bool, Access::READ_PROTECT> prop{true};
    server.put(0x01, prop);

    Extra extra;
    extra.add<PropertyId>(0x01);
    client.send_request(Command::GET_PROPERTY, extra);

    Poll(true);

    EXPECT_EQ(client._rep.error, ErrorCode::E_NO_PERMISSION);
}

TEST_F(HostCS, Read_Read_Protect_Privileged)
{
    Property<bool, Access::READ_PROTECT> prop{true};
    server.put(0x01, prop);

    Extra extra;
    extra.reserve_tag();
    extra.add<PropertyId>(0x01);
    extra.encrypt(PropertyBase::Key);
    client.send_request(Command::GET_PROPERTY, extra);

    Poll();

    EXPECT_EQ(client._rep.error, ErrorCode::S_OK);
}

TEST_F(HostCS, Read_Read_Write_Protect_NotPrivileged)
{
    Property<bool, Access::READ_WRITE_PROTECT> prop{true};
    server.put(0x01, prop);

    Extra extra;
    extra.add<PropertyId>(0x01);
    client.send_request(Command::GET_PROPERTY, extra);

    Poll(true);

    EXPECT_EQ(client._rep.error, ErrorCode::E_NO_PERMISSION);
}

TEST_F(HostCS, Read_Read_Write_Protect_Privileged)
{
    Property<bool, Access::READ_WRITE_PROTECT> prop{true};
    server.put(0x01, prop);

    Extra extra;
    extra.reserve_tag();
    extra.add<PropertyId>(0x01);
    extra.encrypt(PropertyBase::Key);
    client.send_request(Command::GET_PROPERTY, extra);

    Poll();

    EXPECT_EQ(client._rep.error, ErrorCode::S_OK);
}

TEST_F(HostCS, Read_Write_Protect_NotPrivileged)
{
    Property<bool, Access::WRITE_PROTECT> prop{true};
    server.put(0x01, prop);

    Extra extra;
    extra.add<PropertyId>(0x01);
    client.send_request(Command::GET_PROPERTY, extra);

    Poll();

    EXPECT_EQ(client._rep.error, ErrorCode::S_OK);
}

TEST_F(HostCS, Read_Write_Protect_Privileged)
{
    Property<bool, Access::WRITE_PROTECT> prop{true};
    server.put(0x01, prop);

    Extra extra;
    extra.reserve_tag();
    extra.add<PropertyId>(0x01);
    extra.encrypt(PropertyBase::Key);
    client.send_request(Command::GET_PROPERTY, extra);

    Poll();

    EXPECT_EQ(client._rep.error, ErrorCode::S_OK);
}

TEST_F(HostCS, Read_Read_Write)
{
    Property<bool, Access::READ_WRITE> prop{true};
    server.put(0x01, prop);

    Extra extra;
    extra.add<PropertyId>(0x01);
    client.send_request(Command::GET_PROPERTY, extra);

    Poll();

    EXPECT_EQ(client._rep.error, ErrorCode::S_OK);
}