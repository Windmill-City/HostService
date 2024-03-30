#include "gtest/gtest.h"
#include <Property.hpp>

TEST(Property, sizeof)
{
    EXPECT_EQ(sizeof(Property<uint8_t>), 12);
}