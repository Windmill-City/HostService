#include "gtest/gtest.h"
#include <HostCS.hpp>
#include <Property.hpp>

TEST_F(HostCS, Read_Read)
{
    Property<bool, Access::READ> prop{true};
    server.insert(0x01, prop);

    RequestBuilder builder;
    builder.id(0x01);
    builder.tx(client, Command::GET_PROPERTY);

    Poll();

    EXPECT_EQ(client._rep.error, ErrorCode::S_OK);
}

TEST_F(HostCS, Read_Read_Protect_NotPrivileged)
{
    Property<bool, Access::READ_PROTECT> prop{true};
    server.insert(0x01, prop);

    RequestBuilder builder;
    builder.id(0x01);
    builder.tx(client, Command::GET_PROPERTY);

    Poll(true);

    EXPECT_EQ(client._rep.error, ErrorCode::E_NO_PERMISSION);
}

TEST_F(HostCS, Read_Read_Protect_Privileged)
{
    server.privileged = true;
    Property<bool, Access::READ_PROTECT> prop{true};
    server.insert(0x01, prop);
    
    RequestBuilder builder;
    builder.id(0x01);
    builder.tx(client, Command::GET_PROPERTY);

    Poll();

    EXPECT_EQ(client._rep.error, ErrorCode::S_OK);
}

TEST_F(HostCS, Read_Read_Write_Protect_NotPrivileged)
{
    Property<bool, Access::READ_WRITE_PROTECT> prop{true};
    server.insert(0x01, prop);
    
    RequestBuilder builder;
    builder.id(0x01);
    builder.tx(client, Command::GET_PROPERTY);

    Poll(true);

    EXPECT_EQ(client._rep.error, ErrorCode::E_NO_PERMISSION);
}

TEST_F(HostCS, Read_Read_Write_Protect_Privileged)
{
    server.privileged = true;
    Property<bool, Access::READ_WRITE_PROTECT> prop{true};
    server.insert(0x01, prop);
    
    RequestBuilder builder;
    builder.id(0x01);
    builder.tx(client, Command::GET_PROPERTY);

    Poll();

    EXPECT_EQ(client._rep.error, ErrorCode::S_OK);
}

TEST_F(HostCS, Read_Write_Protect_NotPrivileged)
{
    Property<bool, Access::WRITE_PROTECT> prop{true};
    server.insert(0x01, prop);
    
    RequestBuilder builder;
    builder.id(0x01);
    builder.tx(client, Command::GET_PROPERTY);

    Poll();

    EXPECT_EQ(client._rep.error, ErrorCode::S_OK);
}

TEST_F(HostCS, Read_Write_Protect_Privileged)
{
    server.privileged = true;
    Property<bool, Access::WRITE_PROTECT> prop{true};
    server.insert(0x01, prop);
    
    RequestBuilder builder;
    builder.id(0x01);
    builder.tx(client, Command::GET_PROPERTY);

    Poll();

    EXPECT_EQ(client._rep.error, ErrorCode::S_OK);
}

TEST_F(HostCS, Read_Read_Write)
{
    Property<bool, Access::WRITE_PROTECT> prop{true};
    server.insert(0x01, prop);
    
    RequestBuilder builder;
    builder.id(0x01);
    builder.tx(client, Command::GET_PROPERTY);

    Poll();

    EXPECT_EQ(client._rep.error, ErrorCode::S_OK);
}