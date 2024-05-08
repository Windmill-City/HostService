#include "gtest/gtest.h"
#include <HostCS.hpp>
#include <Struct.hpp>

struct FloatSt
{
    float val;
};

TEST(Struct, sizeof)
{
    EXPECT_EQ(sizeof(Struct<FloatSt>), 8);
    EXPECT_EQ(sizeof(Struct<FloatSt>), 8);
}

TEST(HostCS, Struct_GetProperty)
{
    Struct<FloatSt> prop;
    HostCS<1>       cs({
        {5, &(PropertyBase&)prop}
    });
    prop.ref().val = 18.8f;

    Extra extra;
    extra.add<PropertyId>(0x05);
    cs.client.send_request(Command::GET_PROPERTY, extra);

    cs.Poll();

    PropertyId id;
    cs.client._extra.get(id);

    float recv;
    cs.client._extra.get(recv);

    EXPECT_EQ(recv, 18.8f);
}

TEST(HostCS, Struct_SetProperty)
{
    Struct<FloatSt> prop;
    HostCS<1>       cs({
        {5, &(PropertyBase&)prop}
    });

    Extra           extra;
    extra.add<PropertyId>(0x05);
    extra.add(18.8f);
    cs.client.send_request(Command::SET_PROPERTY, extra);

    cs.Poll();

    EXPECT_FLOAT_EQ(prop.ref().val, 18.8f);
}

TEST(HostCS, Struct_SetMemory)
{
    Struct<FloatSt> prop;
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

TEST(HostCS, Struct_GetMemory)
{
    Struct<FloatSt> prop;
    HostCS<1>       cs({
        {5, &(PropertyBase&)prop}
    });
    prop.ref().val = 18.8f;

    MemoryAccess access;
    access.offset = 0;
    access.size   = sizeof(float);

    Extra extra;
    extra.add<PropertyId>(0x05);
    extra.add(access);
    cs.client.send_request(Command::GET_MEMORY, extra);

    cs.Poll(false);

    EXPECT_EQ(cs.client._rep.error, ErrorCode::E_NO_IMPLEMENT);
}

TEST(HostCS, Struct_GetSize)
{
    Struct<FloatSt> prop;
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
