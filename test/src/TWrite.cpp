#include "gtest/gtest.h"
#include <HostCS.hpp>
#include <Property.hpp>

TEST(HostCS, Write_Read)
{
    Property<bool, Access::READ> prop;
    HostCS<1>                    cs({
        {5, &(PropertyBase&)prop}
    });

    Extra                        extra;
    extra.add<PropertyId>(0x05);
    cs.client.send_request(Command::SET_PROPERTY, extra);

    cs.Poll(false);

    EXPECT_EQ(cs.client._rep.error, ErrorCode::E_READ_ONLY);
}

TEST(HostCS, Write_Read_Protect_NotPrivileged)
{
    Property<bool, Access::READ_PROTECT> prop;
    HostCS<1>                            cs({
        {5, &(PropertyBase&)prop}
    });

    Extra                                extra;
    extra.add<PropertyId>(0x05);
    cs.client.send_request(Command::SET_PROPERTY, extra);

    cs.Poll(false);

    EXPECT_EQ(cs.client._rep.error, ErrorCode::E_NO_PERMISSION);
}

TEST(HostCS, Write_Read_Protect_Privileged)
{
    Property<bool, Access::READ_PROTECT> prop;
    HostCS<1>                            cs({
        {5, &(PropertyBase&)prop}
    });

    Extra                                extra;
    extra.reserve_tag();
    extra.add<PropertyId>(0x05);
    extra.encrypt(cs.server.Nonce, cs.server.Key);
    cs.client.send_request(Command::SET_PROPERTY, extra);

    cs.Poll(false);

    EXPECT_EQ(cs.client._rep.error, ErrorCode::E_READ_ONLY);
}

TEST(HostCS, Write_Read_Write_Protect_NotPrivileged)
{
    Property<bool, Access::READ_WRITE_PROTECT> prop;
    HostCS<1>                                  cs({
        {5, &(PropertyBase&)prop}
    });

    Extra                                      extra;
    extra.add<PropertyId>(0x05);
    cs.client.send_request(Command::SET_PROPERTY, extra);

    cs.Poll(false);

    EXPECT_EQ(cs.client._rep.error, ErrorCode::E_NO_PERMISSION);
}

TEST(HostCS, Write_Read_Write_Protect_Privileged)
{
    Property<bool, Access::READ_WRITE_PROTECT> prop;
    HostCS<1>                                  cs({
        {5, &(PropertyBase&)prop}
    });

    Extra                                      extra;
    extra.reserve_tag();
    extra.add<PropertyId>(0x05);
    extra.encrypt(cs.server.Nonce, cs.server.Key);
    cs.client.send_request(Command::SET_PROPERTY, extra);

    cs.Poll(false);

    EXPECT_EQ(cs.client._rep.error, ErrorCode::E_INVALID_ARG);
}

TEST(HostCS, Write_Write_Protect_NotPrivileged)
{
    Property<bool, Access::WRITE_PROTECT> prop;
    HostCS<1>                             cs({
        {5, &(PropertyBase&)prop}
    });

    Extra                                 extra;
    extra.add<PropertyId>(0x05);
    cs.client.send_request(Command::SET_PROPERTY, extra);

    cs.Poll(false);

    EXPECT_EQ(cs.client._rep.error, ErrorCode::E_NO_PERMISSION);
}

TEST(HostCS, Write_Write_Protect_Privileged)
{
    Property<bool, Access::WRITE_PROTECT> prop;
    HostCS<1>                             cs({
        {5, &(PropertyBase&)prop}
    });

    Extra                                 extra;
    extra.reserve_tag();
    extra.add<PropertyId>(0x05);
    extra.encrypt(cs.server.Nonce, cs.server.Key);
    cs.client.send_request(Command::SET_PROPERTY, extra);

    cs.Poll(false);

    EXPECT_EQ(cs.client._rep.error, ErrorCode::E_INVALID_ARG);
}

TEST(HostCS, Write_Read_Write)
{
    Property<bool, Access::READ_WRITE> prop;
    HostCS<1>                          cs({
        {5, &(PropertyBase&)prop}
    });

    Extra                              extra;
    extra.add<PropertyId>(0x05);
    cs.client.send_request(Command::SET_PROPERTY, extra);

    cs.Poll(false);

    EXPECT_EQ(cs.client._rep.error, ErrorCode::E_INVALID_ARG);
}