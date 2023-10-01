#include <cstdint>

namespace math {
/////////////////////////////////////////////////////////////////////////////////

template <typename T, uint32_t D> struct VecBase {
    using this_t = VecBase;

    virtual const T& operator[](int i) const = 0;
    virtual T&       operator[](int i)       = 0;

    bool operator==(const this_t& other) const
    {
        for (uint32_t i = 0; i < D; ++i) {
            if (!(other[i] == (*this)[i])) {
                return false;
            }
        }
        return true;
    }
};

template <typename T, uint32_t D> struct vec : public VecBase<T, D> {
    T elem[D];
    const T& operator[](int i) const override final { return elem[i]; }
    T&       operator[](int i) override final { return elem[i]; }
};

template <typename T> struct vec<T, 2> : public VecBase<T, 2> {
    union {
        T elem[2];
        struct {
            T x;
            T y;
        };
    };
    const T& operator[](int i) const override final { return elem[i]; }
    T&       operator[](int i) override final { return elem[i]; }
};
template <typename T> struct vec<T, 3> : public VecBase<T, 3> {
    union {
        T elem[3];
        struct {
            T x;
            T y;
            T z;
        };
    };
    const T& operator[](int i) const override final { return elem[i]; }
    T&       operator[](int i) override final { return elem[i]; }
};
template <typename T> struct vec<T, 4> : public VecBase<T, 4> {
    union {
        T elem[4];
        struct {
            T x;
            T y;
            T z;
            T w;
        };
    };
    const T& operator[](int i) const override final { return elem[i]; }
    T&       operator[](int i) override final { return elem[i]; }
};

#define MATH_TYPE_HELPER(T, TYPE) \
    using vec2##T = vec<TYPE, 2>; \
    using vec3##T = vec<TYPE, 3>; \
    using vec4##T = vec<TYPE, 4>;

MATH_TYPE_HELPER(ui8, uint8_t)
MATH_TYPE_HELPER(i8, int8_t)
MATH_TYPE_HELPER(ui16, uint16_t)
MATH_TYPE_HELPER(i16, int16_t)
MATH_TYPE_HELPER(ui32, uint32_t)
MATH_TYPE_HELPER(i32, int32_t)

MATH_TYPE_HELPER(f, float)
MATH_TYPE_HELPER(d, double)

/////////////////////////////////////////////////////////////////////////////////
}   // namespace math
