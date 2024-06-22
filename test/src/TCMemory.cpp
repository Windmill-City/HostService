#include "gtest/gtest.h"
#include <CMemory.hpp>
#include <future>
#include <HostCS.hpp>
#include <thread>

static std::array<uint8_t, 1024>                      ArrayVal;
static Memory<decltype(ArrayVal), Access::READ_WRITE> Prop_1(ArrayVal);
// 静态初始化
static constexpr PropertyMap<1>                       Map = {
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

struct TCMemory
    : public HostCSBase
    , public testing::Test
{
    bool              Running = true;
    std::future<void> end;

    TCMemory()
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

TEST_F(TCMemory, Set)
{
    std::array<uint8_t, 1024>    CArrayVal;
    CMemory<decltype(CArrayVal)> c_prop("prop.1");

    EXPECT_EQ(c_prop.set(client, 0, CArrayVal.data(), CArrayVal.size()), ErrorCode::S_OK);
    EXPECT_TRUE(memcmp(CArrayVal.data(), ArrayVal.data(), 1024) == 0);
}

TEST_F(TCMemory, Get)
{
    std::array<uint8_t, 1024>    CArrayVal;
    CMemory<decltype(CArrayVal)> c_prop("prop.1");

    EXPECT_EQ(c_prop.get(client, 0, CArrayVal.data(), CArrayVal.size()), ErrorCode::S_OK);
    EXPECT_TRUE(memcmp(CArrayVal.data(), ArrayVal.data(), 1024) == 0);
}