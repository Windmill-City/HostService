#include "gtest/gtest.h"
#include <Property.hpp>

TEST(Property, sizeof)
{
    EXPECT_EQ(sizeof(Property<bool>), 12);
    EXPECT_EQ(sizeof(Property<float>), 12);
}