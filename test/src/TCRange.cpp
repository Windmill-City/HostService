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

static constinit CPropertyMap<2> cmap = {
    {{"prop1", 0}, {"prop2", 1}}
};
static CPropertyHolder cholder(cmap);

struct TCRange
    : public HostCSBase
    , public testing::Test
{
    bool              Running = true;
    std::future<void> end;

    TCRange()
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

TEST_F(TCRange, Set_R)
{
    CRange<float> c_prop("prop1", {0, 100});
    c_prop.set({16, 17});

    EXPECT_EQ(c_prop.set(client), ErrorCode::S_OK);
    EXPECT_EQ(c_prop, prop1);
}

TEST_F(TCRange, Get_R)
{
    CRange<float> c_prop("prop1", {16, 17});

    EXPECT_EQ(c_prop.get(client), ErrorCode::S_OK);
    EXPECT_EQ(c_prop, prop1);
    EXPECT_EQ(c_prop.Absolute, prop1.Absolute);
}

TEST_F(TCRange, Set_P)
{
    RangeVal<float>        range{0, 100};
    CRangedProperty<float> c_prop("prop2", range);
    c_prop.set(7);
    prop1 = {0, 100};

    EXPECT_EQ(c_prop.set(client), ErrorCode::S_OK);
    EXPECT_EQ(c_prop, prop2);
}

TEST_F(TCRange, Get_P)
{
    RangeVal<float>        range{0, 100};
    CRangedProperty<float> c_prop("prop2", range);
    prop2 = 10;

    EXPECT_EQ(c_prop.get(client), ErrorCode::S_OK);
    EXPECT_EQ(c_prop, prop2);
}