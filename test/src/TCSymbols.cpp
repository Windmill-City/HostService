#include "gtest/gtest.h"
#include <future>

#include <CMemory.hpp>
#include <HostCS.hpp>

static bool                                 bool1;
static bool                                 bool2;

static PropertySymbols                      symbols;
static Property                             prop_1(bool1);
static Property<bool, Access::READ_PROTECT> prop_2(bool2);
// 静态初始化
static constexpr PropertyMap<3>             map = {
    {
     {"symbols", &(PropertyBase&)symbols},
     {"prop_1", &(PropertyBase&)prop_1},
     {"prop_2", &(PropertyBase&)prop_2},
     }
};
static PropertyHolder            holder(map, symbols);

static constinit CPropertyMap<3> cmap = {
    {
     {"symbols", 0},
     {"prop_1", 0},
     {"prop_2", 0},
     }
};
static CPropertyHolder cholder(cmap);

struct TCSymbols
    : public HostCSBase
    , public testing::Test
{
    bool              Running = true;
    std::future<void> end;

    TCSymbols()
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

TEST_F(TCSymbols, Get)
{
    EXPECT_EQ(client.holder.refresh(client), ErrorCode::S_OK);

    PropertyId id;

    EXPECT_EQ(client.holder.get_id_by_name("prop_1", id), ErrorCode::S_OK);
    EXPECT_EQ(id, 1);

    EXPECT_EQ(client.holder.get_id_by_name("prop_2", id), ErrorCode::S_OK);
    EXPECT_EQ(id, 2);
}