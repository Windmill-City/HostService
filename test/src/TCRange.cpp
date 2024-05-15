#include "gtest/gtest.h"
#include <CRange.hpp>
#include <future>
#include <HostCS.hpp>
#include <thread>

static Range<float>             prop1({0, 100});
static RangedProperty<float>    prop2(prop1.ref());
// 静态初始化
static constexpr PropertyMap<2> map = {
    {{"prop1", &(PropertyBase&)prop1}, {"prop2", &(PropertyBase&)prop2}}
};
static PropertyHolder            holder(map);

static constinit CPropertyMap<1> cmap = {
    {{"prop1", 0}, {"prop2", 1}}
};
static CPropertyHolder cholder(cmap);

struct TCRange
    : public HostCSBase
    , public testing::Test
{
    volatile bool      Running = true;
    std::promise<bool> start;
    std::promise<bool> end;

    TCRange()
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

TEST_F(TCRange, Set_R)
{
    CRange<float> c_prop("prop1", {0, 100});
    c_prop.set({16, 17});

    EXPECT_EQ(c_prop.set(client), ErrorCode::S_OK);
    EXPECT_TRUE(c_prop == prop1);
}

TEST_F(TCRange, Get_R)
{
    CRange<float> c_prop("prop1", {16, 17});

    EXPECT_EQ(c_prop.get(client), ErrorCode::S_OK);
    EXPECT_TRUE(c_prop == prop1);
    EXPECT_TRUE(c_prop.Absolute == prop1.Absolute);
}