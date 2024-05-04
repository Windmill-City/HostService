#include "gtest/gtest.h"
#include <HostCS.hpp>
#include <Property.hpp>

TEST(Property, sizeof)
{
    EXPECT_EQ(sizeof(Property<bool>), 8);
    EXPECT_EQ(sizeof(Property<float>), 8);
}

TEST(Property, Assign)
{
    Property<bool> prop_1 = true;
    Property<bool> prop_2 = false;

    EXPECT_EQ(prop_1, true);
    EXPECT_EQ(prop_2, false);

    prop_1 = prop_2;
    EXPECT_EQ(prop_1, false);
}

TEST(Property, Calc)
{
    Property<float> prop_1 = 5;
    Property<float> prop_2 = 7;

    EXPECT_EQ(prop_1 + 8, 5 + 8.f);
    EXPECT_EQ(prop_1 - 8, 5 - 8.f);
    EXPECT_EQ(prop_1 * 8, 5 * 8.f);
    EXPECT_EQ(prop_1 / 8, 5 / 8.f);

    EXPECT_EQ(prop_1 + prop_2, 5 + 7.f);
    EXPECT_EQ(prop_1 - prop_2, 5 - 7.f);
    EXPECT_EQ(prop_1 * prop_2, 5 * 7.f);
    EXPECT_EQ(prop_1 / prop_2, 5 / 7.f);
}

TEST_F(HostCS, Property_GetProperty)
{
    Property<float> prop{18.8f};
    server.put(0x01, prop);

    Extra extra;
    extra.add<PropertyId>(0x01);
    client.send_request(Command::GET_PROPERTY, extra);

    Poll();

    PropertyId id;
    client._extra.get(id);

    float recv;
    client._extra.get(recv);

    EXPECT_EQ(recv, 18.8f);
}

TEST_F(HostCS, Property_SetProperty)
{
    Property<float> prop{0.0f};
    server.put(0x01, prop);

    Extra extra;
    extra.add<PropertyId>(0x01);
    extra.add(18.8f);
    client.send_request(Command::SET_PROPERTY, extra);

    Poll();

    EXPECT_EQ(prop, 18.8f);
}

TEST_F(HostCS, Property_SetMemory)
{
    Property<float> prop{0.0f};
    server.put(0x01, prop);

    MemoryAccess access;
    access.offset = 0;
    access.size   = sizeof(float);

    Extra extra;
    extra.add<PropertyId>(0x01);
    extra.add(access);
    extra.add(18.8f); // data
    client.send_request(Command::SET_MEMORY, extra);

    Poll(true);

    EXPECT_EQ(client._rep.error, ErrorCode::E_NO_IMPLEMENT);
}

TEST_F(HostCS, Property_GetMemory)
{
    Property<float> prop{0.0f};
    server.put(0x01, prop);

    MemoryAccess access;
    access.offset = 0;
    access.size   = sizeof(float);

    Extra extra;
    extra.add<PropertyId>(0x01);
    extra.add(access);
    client.send_request(Command::GET_MEMORY, extra);

    Poll(true);

    EXPECT_EQ(client._rep.error, ErrorCode::E_NO_IMPLEMENT);
}

TEST_F(HostCS, Property_GetSize)
{
    Property<float> prop{0.0f};
    server.put(0x01, prop);

    Extra extra;
    extra.add<PropertyId>(0x01);
    client.send_request(Command::GET_SIZE, extra);

    Poll();

    PropertyId id;
    client._extra.get(id);

    uint16_t size;
    client._extra.get(size);
    EXPECT_EQ(size, sizeof(float));
}

TEST_F(HostCS, Property_GetDesc)
{
    Property<float> prop{0.0f};
    server.put(0x01, prop);

    Extra extra;
    extra.add<PropertyId>(0x01);
    client.send_request(Command::GET_DESC, extra);

    Poll();

    PropertyId id;
    client._extra.get(id);

    std::string name{(const char*)client._extra.data(), client._extra.remain()};
    EXPECT_STREQ(name.c_str(), "struct Property<float,1>");
}