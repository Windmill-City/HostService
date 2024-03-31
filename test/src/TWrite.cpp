#include "gtest/gtest.h"
#include <HostCS.hpp>
#include <Property.hpp>

TEST_F(HostCS, Write_Read)
{
    Property<bool, Access::READ> prop{true};
    server.insert(0x01, prop);

    Extra extra;
    extra.id() = 0x01;
    client.send_request(Command::SET_PROPERTY, extra);

    Poll(true);

    EXPECT_EQ(client._rep.error, ErrorCode::E_READ_ONLY);
}

TEST_F(HostCS, Write_Read_Protect_NotPrivileged)
{
    Property<bool, Access::READ_PROTECT> prop{true};
    server.insert(0x01, prop);

    Extra extra;
    extra.id() = 0x01;
    client.send_request(Command::SET_PROPERTY, extra);

    Poll(true);

    EXPECT_EQ(client._rep.error, ErrorCode::E_NO_PERMISSION);
}

TEST_F(HostCS, Write_Read_Protect_Privileged)
{
    server.privileged = true;
    Property<bool, Access::READ_PROTECT> prop{true};
    server.insert(0x01, prop);

    Extra extra;
    extra.id() = 0x01;
    client.send_request(Command::SET_PROPERTY, extra);

    Poll(true);

    EXPECT_EQ(client._rep.error, ErrorCode::E_READ_ONLY);
}

TEST_F(HostCS, Write_Read_Write_Protect_NotPrivileged)
{
    Property<bool, Access::READ_WRITE_PROTECT> prop{true};
    server.insert(0x01, prop);

    Extra extra;
    extra.id() = 0x01;
    client.send_request(Command::SET_PROPERTY, extra);

    Poll(true);

    EXPECT_EQ(client._rep.error, ErrorCode::E_NO_PERMISSION);
}

TEST_F(HostCS, Write_Read_Write_Protect_Privileged)
{
    server.privileged = true;
    Property<bool, Access::READ_WRITE_PROTECT> prop{true};
    server.insert(0x01, prop);

    Extra extra;
    extra.id() = 0x01;
    client.send_request(Command::SET_PROPERTY, extra);

    Poll(true);

    EXPECT_EQ(client._rep.error, ErrorCode::E_INVALID_ARG);
}

TEST_F(HostCS, Write_Write_Protect_NotPrivileged)
{
    Property<bool, Access::WRITE_PROTECT> prop{true};
    server.insert(0x01, prop);

    Extra extra;
    extra.id() = 0x01;
    client.send_request(Command::SET_PROPERTY, extra);

    Poll(true);

    EXPECT_EQ(client._rep.error, ErrorCode::E_NO_PERMISSION);
}

TEST_F(HostCS, Write_Write_Protect_Privileged)
{
    server.privileged = true;
    Property<bool, Access::WRITE_PROTECT> prop{true};
    server.insert(0x01, prop);

    Extra extra;
    extra.id() = 0x01;
    client.send_request(Command::SET_PROPERTY, extra);

    Poll(true);

    EXPECT_EQ(client._rep.error, ErrorCode::E_INVALID_ARG);
}

TEST_F(HostCS, Write_Read_Write)
{
    Property<bool, Access::READ_WRITE> prop{true};
    server.insert(0x01, prop);

    Extra extra;
    extra.id() = 0x01;
    client.send_request(Command::SET_PROPERTY, extra);

    Poll(true);

    EXPECT_EQ(client._rep.error, ErrorCode::E_INVALID_ARG);
}