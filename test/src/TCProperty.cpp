#include "gtest/gtest.h"
#include <CProperty.hpp>
#include <future>
#include <HostCS.hpp>
#include <thread>

static Property<float>           prop;
// 静态初始化
static constexpr PropertyMap<1>  map = {{{"prop", &(PropertyBase&)prop}}};
static PropertyHolder            holder(map);

static constinit CPropertyMap<1> cmap = {{{"prop", 0}}};
static CPropertyHolder           cholder(cmap);

struct TCProperty
    : public HostCSBase
    , public testing::Test
{
    bool              Running = true;
    std::future<void> end;

    TCProperty()
        : HostCSBase(holder, cholder)
    {
    }

    virtual void SetUp()
    {
        end = std::async(std::launch::async,
                         [this]()
                         {
                             while (Running)
                             {
                                 server.poll();
                             }
                         });
    }

    virtual void TearDown()
    {
        Running        = false;
        server.Running = false;
        end.get();
    }
};

TEST_F(TCProperty, Set)
{
    CProperty<float> c_prop("prop");
    c_prop = 18.8;

    EXPECT_EQ(c_prop.set(client), ErrorCode::S_OK);
    EXPECT_TRUE(memcmp(&c_prop, &prop, sizeof(float)) == 0);
}

TEST_F(TCProperty, Get)
{
    CProperty<float> c_prop("prop");
    prop = 16.7f;

    EXPECT_EQ(c_prop.get(client), ErrorCode::S_OK);
    EXPECT_TRUE(memcmp(&c_prop, &prop, sizeof(float)) == 0);
}