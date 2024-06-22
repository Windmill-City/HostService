#include "gtest/gtest.h"
#include <CProperty.hpp>
#include <future>
#include <HostCS.hpp>

static float                               FloatVal;
static Property<float, Access::READ_WRITE> Prop_1(FloatVal);
// 静态初始化
static constexpr PropertyMap<1>            Map = {
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

struct TCProperty
    : public HostCSBase
    , public testing::Test
{
    bool              Running = true;
    std::future<void> end;

    TCProperty()
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

TEST_F(TCProperty, Set)
{
    float            CFloatVal = 18.8;
    CProperty<float> c_prop("prop.1");

    EXPECT_EQ(c_prop.set(client, CFloatVal), ErrorCode::S_OK);
    EXPECT_EQ(CFloatVal, FloatVal);
}

TEST_F(TCProperty, Get)
{
    float            CFloatVal;
    CProperty<float> c_prop("prop.1");

    EXPECT_EQ(c_prop.get(client, CFloatVal), ErrorCode::S_OK);
    EXPECT_EQ(CFloatVal, FloatVal);
}