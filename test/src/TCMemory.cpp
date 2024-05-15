#include "gtest/gtest.h"
#include <CMemory.hpp>
#include <future>
#include <HostCS.hpp>
#include <thread>

static Memory<std::array<uint8_t, 1024>> mem;
// 静态初始化
static constexpr PropertyMap<1>          map = {{{"mem", &(PropertyBase&)mem}}};
static PropertyHolder                    holder(map);

static constinit CPropertyMap<1>         cmap = {{{"mem", 0}}};
static CPropertyHolder                   cholder(cmap);

struct TCMemory
    : public HostCSBase
    , public testing::Test
{
    volatile bool     Running = true;
    std::future<void> future;

    TCMemory()
        : HostCSBase(holder, cholder)
    {
    }

    virtual void SetUp()
    {
        future = std::async(std::launch::async,
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
        Running = false;
        future.get();
    }
};

TEST_F(TCMemory, Set)
{
    CMemory<std::array<uint8_t, 1024>> c_mem("mem");

    for (size_t i = 0; i < 1024; i++)
    {
        c_mem[i] = i;
    }

    EXPECT_EQ(c_mem.set(client), ErrorCode::S_OK);
    EXPECT_TRUE(memcmp(&c_mem, &mem, 1024) == 0);
}
