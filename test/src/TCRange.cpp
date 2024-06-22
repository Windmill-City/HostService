#include "gtest/gtest.h"
#include <CRange.hpp>
#include <future>
#include <HostCS.hpp>
#include <thread>

static RangeVal<float>                  RangeVal_1;
static Range<float, Access::READ_WRITE> Prop_1(RangeVal_1, {0, 100});
// 静态初始化
static constexpr PropertyMap<1>         Map = {
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

struct TCRange
    : public HostCSBase
    , public testing::Test
{
    bool              Running = true;
    std::future<void> end;

    TCRange()
        : HostCSBase(Holder, CHolder)
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

TEST_F(TCRange, Set_Range)
{
    RangeVal<float> CRangeVal{1, 5};
    CRange<float>   c_prop("prop.1");

    EXPECT_EQ(c_prop.set(client, CRangeVal), ErrorCode::S_OK);
    EXPECT_EQ(CRangeVal, RangeVal_1);
}

TEST_F(TCRange, Get_Range)
{
    RangeVal<float> CRangeVal;
    CRange<float>   c_prop("prop.1");

    EXPECT_EQ(c_prop.get(client, RangeAccess::Range, CRangeVal), ErrorCode::S_OK);
    EXPECT_EQ(CRangeVal, RangeVal_1);
}

TEST_F(TCRange, Get_Absolute)
{
    RangeVal<float> CRangeVal;
    CRange<float>   c_prop("prop.1");

    EXPECT_EQ(c_prop.get(client, RangeAccess::Absolute, CRangeVal), ErrorCode::S_OK);
    EXPECT_EQ(CRangeVal, Prop_1.Absolute);
}