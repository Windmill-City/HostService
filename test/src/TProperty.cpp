#include "gtest/gtest.h"
#include <HostCS.hpp>
#include <Property.hpp>

TEST(Property, sizeof)
{
    EXPECT_EQ(sizeof(Property<bool>), 8);
    EXPECT_EQ(sizeof(Property<float>), 8);
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
    Property<float> prop{0.0f};
    server.insert(0x01, prop);

    RequestBuilder builder;
    builder.id(0x01);
    builder.add(18.8f);
    builder.tx(client, Command::SET_PROPERTY);

    Poll();

    EXPECT_EQ(prop, 18.8f);
}

TEST_F(HostCS, Property_SetMemory)
{
    Property<float> prop{0.0f};
    server.insert(0x01, prop);

    RequestBuilder builder;
    builder.id(0x01);
    builder.offset(0);
    builder.datlen(sizeof(float));
    builder.add(18.8f); // data
    builder.tx(client, Command::SET_MEMORY);

    Poll(true);

    EXPECT_EQ(client._rep.error, ErrorCode::E_NO_IMPLEMENT);
}

TEST_F(HostCS, Property_GetMemory)
{
    Property<float> prop{0.0f};
    server.insert(0x01, prop);

    RequestBuilder builder;
    builder.id(0x01);
    builder.offset(0);
    builder.datlen(sizeof(float));
    builder.tx(client, Command::GET_MEMORY);

    Poll(true);

    EXPECT_EQ(client._rep.error, ErrorCode::E_NO_IMPLEMENT);
}

TEST_F(HostCS, Property_GetSize)
{
    Property<float> prop{0.0f};
    server.insert(0x01, prop);

    RequestBuilder builder;
    builder.id(0x01);
    builder.tx(client, Command::GET_SIZE);

    Poll();

    EXPECT_EQ(*(uint16_t*)client._extra, sizeof(float));
}