#include "gtest/gtest.h"
#include <HostCS.hpp>

TEST(Header, sizeof)
{
    ASSERT_EQ(sizeof(Header), 5);
}

static bool                     BoolVal;
static Property                 Prop_1(BoolVal);
// 静态初始化
static constexpr PropertyMap<1> Map = {
    {
     {"prop.1", &(PropertyBase&)Prop_1},
     }
};
static PropertyHolder            Holder(Map);

static constinit CPropertyMap<1> CMap = {
    {
     {"prop.1", 0},
     }
};
static CPropertyHolder CHolder(CMap);

struct HostCS
    : public HostCSBase
    , public testing::Test
{
    HostCS()
        : HostCSBase(Holder, CHolder)
    {
    }
};

TEST_F(HostCS, request)
{
    uint8_t   data[] = {0x01, 0x02, 0x03};

    Extra     extra;
    ErrorCode err;
    extra.add(data, sizeof(data));
    client.send(Command::ECHO, extra);

    ASSERT_TRUE(server.poll());
    client.recv_response(Command::ECHO, err, client.extra);

    ASSERT_TRUE(memcmp(client.extra.data(), data, sizeof(data)) == 0);
}
