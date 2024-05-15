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

static SecretHolder              secret;
// 静态初始化
static constexpr PropertyMap<1>  map = {{{"nonce", &(PropertyBase&)secret.nonce}}};
static PropertyHolder            holder(map);

static constinit CPropertyMap<1> cmap = {{{"nonce", 0}}};
static CPropertyHolder           cholder(cmap);

struct HostCS
    : public HostCSBase
    , public testing::Test
{
    HostCS()
        : HostCSBase(holder, cholder)
    {
    }
};

TEST_F(HostCS, request)
{
    uint8_t   data[] = {0x01, 0x02, 0x03};

    Extra     extra;
    ErrorCode err;
    extra.add(data, sizeof(data));
    client.send_request(Command::ECHO, extra);

    ASSERT_TRUE(server.poll());
    client.recv_response(Command::ECHO, err, client.extra);

    ASSERT_TRUE(memcmp(client.extra.data(), data, sizeof(data)) == 0);
}
