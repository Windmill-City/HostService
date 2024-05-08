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
    Property<bool> prop_1;
    Property<bool> prop_2;

    prop_1 = true;
    prop_2 = false;

    EXPECT_EQ(prop_1, true);
    EXPECT_EQ(prop_2, false);

    prop_1 = prop_2;
    EXPECT_EQ(prop_1, false);
}

TEST(Property, Calc)
{
    Property<float> prop_1;
    Property<float> prop_2;

    prop_1 = 5;
    prop_2 = 7;

    EXPECT_EQ(prop_1 + 8, 5 + 8.f);
    EXPECT_EQ(prop_1 - 8, 5 - 8.f);
    EXPECT_EQ(prop_1 * 8, 5 * 8.f);
    EXPECT_EQ(prop_1 / 8, 5 / 8.f);

    EXPECT_EQ(prop_1 + prop_2, 5 + 7.f);
    EXPECT_EQ(prop_1 - prop_2, 5 - 7.f);
    EXPECT_EQ(prop_1 * prop_2, 5 * 7.f);
    EXPECT_EQ(prop_1 / prop_2, 5 / 7.f);
}

TEST(HostCS, Property_GetProperty)
{
    Property<float> prop = 18.8f;
    HostCS<1>       cs({
        {5, &(PropertyBase&)prop}
    });

    Extra           extra;
    extra.add<PropertyId>(0x05);
    cs.client.send_request(Command::GET_PROPERTY, extra);

    cs.Poll();

    PropertyId id;
    cs.client._extra.get(id);

    float recv;
    cs.client._extra.get(recv);

    EXPECT_EQ(recv, 18.8f);
}

TEST(HostCS, Property_SetProperty)
{
    Property<float> prop;
    HostCS<1>       cs({
        {5, &(PropertyBase&)prop}
    });

    Extra           extra;
    extra.add<PropertyId>(0x05);
    extra.add(18.8f);
    cs.client.send_request(Command::SET_PROPERTY, extra);

    cs.Poll();

    EXPECT_EQ(prop, 18.8f);
}

TEST(HostCS, Property_SetMemory)
{
    Property<float> prop;
    HostCS<1>       cs({
        {5, &(PropertyBase&)prop}
    });

    MemoryAccess    access;
    access.offset = 0;
    access.size   = sizeof(float);

    Extra extra;
    extra.add<PropertyId>(0x05);
    extra.add(access);
    extra.add(18.8f); // data
    cs.client.send_request(Command::SET_MEMORY, extra);

    cs.Poll(false);

    EXPECT_EQ(cs.client._rep.error, ErrorCode::E_NO_IMPLEMENT);
}

TEST(HostCS, Property_GetMemory)
{
    Property<float> prop;
    HostCS<1>       cs({
        {5, &(PropertyBase&)prop}
    });

    MemoryAccess    access;
    access.offset = 0;
    access.size   = sizeof(float);

    Extra extra;
    extra.add<PropertyId>(0x05);
    extra.add(access);
    cs.client.send_request(Command::GET_MEMORY, extra);

    cs.Poll(false);

    EXPECT_EQ(cs.client._rep.error, ErrorCode::E_NO_IMPLEMENT);
}

TEST(HostCS, Property_GetSize)
{
    Property<float> prop;
    HostCS<1>       cs({
        {5, &(PropertyBase&)prop}
    });

    Extra           extra;
    extra.add<PropertyId>(0x05);
    cs.client.send_request(Command::GET_SIZE, extra);

    cs.Poll();

    PropertyId id;
    cs.client._extra.get(id);

    uint16_t size;
    cs.client._extra.get(size);
    EXPECT_EQ(size, sizeof(float));
}
