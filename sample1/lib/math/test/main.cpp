#include "math/vec.h"

// #include "abc/timer.hpp"

#include <gtest/gtest.h>

namespace {
/////////////////////////////////////////////////////////////////////////////////

TEST(Math, boolVec)
{
    using namespace math;

    vec<bool, 10> bvecTrue {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    EXPECT_TRUE(detail::all(bvecTrue));
    EXPECT_TRUE(detail::any(bvecTrue));
    vec<bool, 10> bvecFalse {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    EXPECT_TRUE(detail::none(bvecFalse));
    EXPECT_FALSE(detail::any(bvecFalse));

    bvecFalse[bvecFalse.size() - 1] = true;
    EXPECT_TRUE(bvecFalse[bvecFalse.size() - 1]);
    EXPECT_TRUE(detail::any(bvecFalse));
}

template <typename T>
void
mathVecOpsHelper()
{
    using vec_t   = T;
    using value_t = typename vec_t::value_t;

    vec_t a {1, 2};
    vec_t b {3, 4};
    EXPECT_EQ((a + b), (vec_t {4, 6}));
    EXPECT_EQ((a - b), (vec_t {-2, -2}));
    EXPECT_EQ((a * b), (vec_t {3, 8}));
    EXPECT_EQ((a / b), (vec_t {1.f / 3, 2.f / 4}));

    EXPECT_NE(a, b);
    b = a;
    EXPECT_EQ(a, b);

    vec_t c;
    c = a;
    c += b;
    EXPECT_EQ(c, a + b);
    c = a;
    c -= b;
    EXPECT_EQ(c, a - b);
    c = a;
    c *= b;
    EXPECT_EQ(c, a * b);
    c = a;
    c /= b;
    EXPECT_EQ(c, a / b);

    c = a;
    c *= value_t(2);
    EXPECT_EQ(c, a * value_t(2));
    c = a;
    c /= value_t(2);
    EXPECT_EQ(c, a / value_t(2));
}
TEST(Math, mathVecNOps)
{
    using namespace math;
    mathVecOpsHelper<vec<float, 2>>();
    mathVecOpsHelper<vec<uint32_t, 2>>();
    mathVecOpsHelper<vec<int8_t, 2>>();
}
TEST(Math, mathVecOps)
{
    using namespace math;
    mathVecOpsHelper<vec2<float>>();
    mathVecOpsHelper<vec2<uint32_t>>();
    mathVecOpsHelper<vec2<int8_t>>();
}
// TEST(Math, vec)
// {
//     using namespace math;
//     vec2f a, b;
//     a.x = 1.f;
//     a.y = 2.f;
//     b.x = a.x;
//     b.y = a.y;
//     EXPECT_EQ(a.x, b.x);
//     EXPECT_EQ(a.y, b.y);

//     a = b;
//     EXPECT_EQ(a.x, b.x);
//     EXPECT_EQ(a.y, b.y);
// }

template <typename T> struct vec2_ {
    T x, y;
};

// TEST(Math, performance)
// {
//     using namespace math;
//     abc::chrono::timer timer;

//     const int COUNT = 100000;
//     timer.reset();
//     vec2f a, b;
//     for (int i = 0; i < COUNT; ++i) {
//         a.x = 1.f;
//         a.y = 2.f;
//         b.x = a.x;
//         b.y = a.y;
//     }
//     auto elapsed1 = timer.get_elapsed_time_as<abc::chrono::microsecondsf>();

//     timer.reset();
//     for (int i = 0; i < COUNT; ++i) {
//         a.x = 1.f;
//         a.y = 2.f;
//         b.x = a.x;
//         b.y = a.y;
//     }
//     auto elapsed2 = timer.get_elapsed_time_as<abc::chrono::microsecondsf>();
//     ASSERT_NEAR(elapsed1.count(), elapsed2.count(), 100.f);
// }

/////////////////////////////////////////////////////////////////////////////////
}   // namespace
