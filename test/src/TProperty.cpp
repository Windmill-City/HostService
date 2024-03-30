#include "gtest/gtest.h"
#include <HostCS.hpp>
#include <Property.hpp>

TEST(Property, sizeof)
{
    EXPECT_EQ(sizeof(Property<bool>), 12);
    EXPECT_EQ(sizeof(Property<float>), 12);
}

TEST_F(HostCS, Property_GetProperty)
{
    Property<bool> prop_bool = true;
    server.insert(0x01, prop_bool);

    RequestBuilder builder;
    builder.id(0x01);
    builder.tx(client, Command::GET_PROPERTY);

    Poll();

    EXPECT_EQ(*(bool*)client._extra, true);
}

TEST_F(HostCS, Property_SetProperty)
{
    Property<float> prop_float{0.0f, Access::READ_WRITE};
    server.insert(0x01, prop_float);

    RequestBuilder builder;
    builder.id(0x01);
    builder.add(18.8f);
    builder.tx(client, Command::SET_PROPERTY);

    Poll();

    EXPECT_EQ(prop_float, 18.8f);
}

TEST_F(HostCS, Property_SetMemory)
{
    Property<float> prop_float{0.0f, Access::READ_WRITE};
    server.insert(0x01, prop_float);

    RequestBuilder builder;
    builder.id(0x01);
    builder.add(18.8f);
    builder.tx(client, Command::SET_PROPERTY);

    Poll();

    EXPECT_EQ(prop_float, 18.8f);
}