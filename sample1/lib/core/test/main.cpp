#include "core/core.h"

#include <gtest/gtest.h>

namespace {

TEST(Core, assertion) {
    using namespace core;

    EXPECT_EQ(0, 0);
}

}   // namespace
