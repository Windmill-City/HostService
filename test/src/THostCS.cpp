#include "gtest/gtest.h"
#include <HostCS.hpp>
#include <Property.hpp>

TEST(Request, sizeof)
{
    ASSERT_EQ(sizeof(Request), 5);
}

TEST(Response, sizeof)
{
    ASSERT_EQ(sizeof(Response), 6);
}

static SecretHolder secret;
using PropertyMap                = frozen::map<frozen::string, PropertyBase*, 1>;
// 静态初始化
static constexpr PropertyMap map = {
    {"nonce", &(PropertyBase&)secret.nonce}
};

static PropertyHolder holder(map);

struct HostCS
    : public HostCSBase
    , public testing::Test
{
    HostCS()
        : HostCSBase(holder)
    {
    }
};

TEST_F(HostCS, request)
{
    uint8_t data[] = {0x01, 0x02, 0x03};

    Extra   extra;
    extra.add(data, sizeof(data));
    client.send_request(Command::ECHO, extra);

    Poll();

    ASSERT_TRUE(memcmp(client._extra.data(), data, sizeof(data)) == 0);
}
