#include "math/vec.h"

#include <gtest/gtest.h>

namespace {

using namespace math;

TEST(Math, vec)
{
    vec2f a, b;
    a.x = 1.f;
    a.y = 2.f;
    b.x = a.x;
    b.y = a.y;
    ASSERT_EQ(a.x, b.x);
    ASSERT_EQ(a.y, b.y);
}
}   // namespace
