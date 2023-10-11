#include "math/vec.h"

#include "abc/profiler.hpp"

namespace test {
////////////////////////////////////////////////////////////////////////////////

template <typename T> struct vec4 {
    T x, y, z, w;

    using this_t = vec4<T>;

    bool operator==(const this_t& other) const { return x == other.x && y == other.y && z == other.z && w == other.w; }
    bool operator!=(const this_t& other) const { return !operator==(other); }

    this_t& operator=(const this_t& other)
    {
        x = other.x;
        y = other.y;
        z = other.z;
        w = other.w;
        return *this;
    }

    this_t& operator+=(const this_t& other)
    {
        this->x += other.x;
        this->y += other.y;
        this->z += other.z;
        this->w += other.w;
        return *this;
    }
    this_t operator+(const this_t& other) const
    {
        this_t result = *this;
        result += other;
        return result;
    }
    this_t& operator*=(const this_t& other)
    {
        this->x = other.x;
        this->y = other.y;
        this->z = other.z;
        this->w = other.w;
        return *this;
    }
    this_t operator*(const this_t& other) const
    {
        this_t result = *this;
        result += other;
        return result;
    }
};
////////////////////////////////////////////////////////////////////////////////
}   // namespace test{

template <typename T>
int
mathVecOpsHelper()
{
    using vec_t   = T;
    using value_t = typename vec_t::value_t;

    vec_t a;
    vec_t b;
    static value_t avalues[] {1, 2, 3, 4, 5, 6};
    static value_t bvalues[] {7, 9, 6, 4, 5, 4};
    for (size_t i = 0; i < vec_t::dim; ++i) {
        a[i] = avalues[i % (sizeof(avalues)/sizeof(avalues[0]))];
        b[i] = bvalues[i % (sizeof(bvalues) / sizeof(avalues[0]))];
    }

    b = a;

    vec_t c;
    c = a;
    c += b;
    c = a;
    c -= b;
    c = a;
    c *= b;
    c = a;
    c /= b;

    c = a;
    c *= value_t(2);
    c = a;
    c /= value_t(2);

    value_t result = 0;
    for (size_t i = 0; i < vec_t::dim; ++i) {
        result += c[i];
    }
    return result;
}

int
main(int, char**)
{
    ABC_PROFILE_INIT();
    size_t COUNT  = 10000;
    size_t COUNT2 = 20;

    for (size_t i = 0; i < COUNT2; ++i) {
        {
            ABC_PROFILE_BEGIN("vecT2");
            for (size_t i = 0; i < COUNT; ++i) {
                mathVecOpsHelper<math::vec<float,    2>>();
                mathVecOpsHelper<math::vec<uint32_t, 2>>();
                mathVecOpsHelper<math::vec<int8_t,   2>>();
            }
            ABC_PROFILE_END("vecT2");

            ABC_PROFILE_BEGIN("vecT3");
            for (size_t i = 0; i < COUNT; ++i) {
                mathVecOpsHelper<math::vec<float,    3>>();
                mathVecOpsHelper<math::vec<uint32_t, 3>>();
                mathVecOpsHelper<math::vec<int8_t,   3>>();
            }
            ABC_PROFILE_END("vecT3");

            ABC_PROFILE_BEGIN("vecT4");
            for (size_t i = 0; i < COUNT; ++i) {
                mathVecOpsHelper<math::vec<float,    4>>();
                mathVecOpsHelper<math::vec<uint32_t, 4>>();
                mathVecOpsHelper<math::vec<int8_t,   4>>();
            }
            ABC_PROFILE_END("vecT4");
            ABC_PROFILE_BEGIN("vecTN");
            const size_t DIM = 10;
            for (size_t i = 0; i < COUNT; ++i) {
                mathVecOpsHelper<math::vec<float,    DIM>>();
                mathVecOpsHelper<math::vec<uint32_t, DIM>>();
                mathVecOpsHelper<math::vec<int8_t,   DIM>>();
            }
            ABC_PROFILE_END("vecTN");
        }
        {
            ABC_PROFILE_BEGIN("vec2");
            for (size_t i = 0; i < COUNT; ++i) {
                mathVecOpsHelper<math::vec2<float>>();
                mathVecOpsHelper<math::vec2<uint32_t>>();
                mathVecOpsHelper<math::vec2<int8_t>>();
            }
            ABC_PROFILE_END("vec2");
            ABC_PROFILE_BEGIN("vec3");
            for (size_t i = 0; i < COUNT; ++i) {
                mathVecOpsHelper<math::vec3<float>>();
                mathVecOpsHelper<math::vec3<uint32_t>>();
                mathVecOpsHelper<math::vec3<int8_t>>();
            }
            ABC_PROFILE_END("vec3");
            ABC_PROFILE_BEGIN("vec4");
            for (size_t i = 0; i < COUNT; ++i) {
                mathVecOpsHelper<math::vec4<float>>();
                mathVecOpsHelper<math::vec4<uint32_t>>();
                mathVecOpsHelper<math::vec4<int8_t>>();
            }
            ABC_PROFILE_END("vec4");
        }
        ABC_PROFILE_SUMMARY();
    }
    ABC_PROFILE_SUMMARY();

    return 0;
}