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

TEST(Math, simple)
{
    using namespace math;
    using vec_t = vec<float, 2>;
    vec_t a {1, 2};
    vec_t b {3, 4};
    EXPECT_EQ((a + b), (vec_t {4, 6}));
    EXPECT_EQ((a - b), (vec_t {-2, -2}));
    EXPECT_EQ((a * b), (vec_t {3, 8}));
    EXPECT_EQ((a / b), (vec_t {1.f / 3, 2.f / 4}));
}

template <typename T>
void
mathVecOpsTypeHelper()
{
    using vec_t   = T;
    using value_t = typename vec_t::value_t;

    static const value_t values1[] {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    static const value_t values2[] {3, 5, 7, 9, 11, 13, 15, 17, 19, 21};
    vec_t                a;
    vec_t                b;
    vec_t                sum, diff, mult, div;

    for (size_t i = 0; i < vec_t::dim; ++i) {
        a[i] = values1[i];
        b[i] = values2[i];
    }
    sum  = a + b;
    diff = a - b;
    mult = a * b;
    div  = a / b;
    for (size_t i = 0; i < vec_t::dim; ++i) {
        EXPECT_EQ(sum[i], a[i] + b[i]);
        EXPECT_EQ(diff[i], a[i] - b[i]);
        EXPECT_EQ(mult[i], a[i] * b[i]);
        EXPECT_EQ(div[i], a[i] / b[i]);
    }

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
template <size_t DIM>
void
mathVecOpsSizeHelper()
{
    using namespace math;
    mathVecOpsTypeHelper<vec<double, DIM>>();
    mathVecOpsTypeHelper<vec<float, DIM>>();
    mathVecOpsTypeHelper<vec<uint32_t, DIM>>();
    mathVecOpsTypeHelper<vec<int8_t, DIM>>();
}

TEST(Math, multi)
{
    mathVecOpsSizeHelper<2>();
    mathVecOpsSizeHelper<3>();
    mathVecOpsSizeHelper<4>();
    mathVecOpsSizeHelper<5>();
    mathVecOpsSizeHelper<6>();
    mathVecOpsSizeHelper<7>();
}

/////////////////////////////////////////////////////////////////////////////////
}   // namespace
