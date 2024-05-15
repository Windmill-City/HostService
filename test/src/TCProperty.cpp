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
    volatile bool      Running = true;
    std::promise<bool> start;
    std::promise<bool> end;

    TCProperty()
        : HostCSBase(holder, cholder)
    {
    }

    virtual void SetUp()
    {
        std::thread(
            [this]()
            {
                start.set_value(true);
                while (Running)
                {
                    server.poll();
                }
                end.set_value(true);
            })
            .detach();
        start.get_future().get();
    }

    virtual void TearDown()
    {
        Running = false;
        end.get_future().get();
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