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
    volatile bool      Running = true;
    std::promise<bool> start;
    std::promise<bool> end;

    TCMemory()
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

TEST_F(TCMemory, Get)
{
    CMemory<std::array<uint8_t, 1024>> c_mem("mem");

    for (size_t i = 0; i < 1024; i++)
    {
        mem[i] = i;
    }

    EXPECT_EQ(c_mem.get(client), ErrorCode::S_OK);
    EXPECT_TRUE(memcmp(&c_mem, &mem, 1024) == 0);
}