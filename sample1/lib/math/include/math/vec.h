#pragma once

#include "math/core.h"

#include <cstdint>
#include <functional>

namespace math {
/////////////////////////////////////////////////////////////////////////////////

namespace detail {
////////////////////////////////////////////////////////////////////////////////

template <typename T> struct applier1 {
    using value_t               = typename T::value_t;
    static constexpr size_t DIM = T::dim;

    static void cwise(T& a, std::function<value_t(const value_t& a)> op)
    {
        for (size_t i = 0; i < DIM; ++i) {
            a[i] = op(a[i]);
        }
    }
    static bool all(const T& a, std::function<bool(const value_t&)> op)
    {
        bool result = op(a[0]);
        for (size_t i = 1; result && i < DIM; ++i) {
            result = result && op(a[i]);
        }
        return result;
    }
    static bool any(const T& a, std::function<bool(const value_t&)> op)
    {
        bool result = op(a[0]);
        for (size_t i = 1; !result && i < DIM; ++i) {
            result = op(a[i]);
        }
        return result;
    }
};

template <typename T> struct applier2 {
    using value_t               = typename T::value_t;
    static constexpr size_t DIM = T::dim;

    static void cwise(T& c, const T& a, const T& b, std::function<value_t(const value_t& a, const value_t& b)> op)
    {
        for (size_t i = 0; i < DIM; ++i) {
            c[i] = op(a[i], b[i]);
        }
    }
    static bool all(const T& a, const T& b, std::function<bool(const value_t&, const value_t&)> op)
    {
        bool result = op(a[0], b[0]);
        for (size_t i = 1; result && i < DIM; ++i) {
            result = op(a[i], b[i]);
        }
        return result;
    }
    static bool any(const T& a, const T& b, std::function<bool(const value_t&, const value_t&)> op)
    {
        bool result = op(a[0], b[0]);
        for (size_t i = 1; !result && i < DIM; ++i) {
            result = op(a[i], b[i]);
        }
        return result;
    }
};

////////////////////////////////////////////////////////////////////////////////

template <typename T, typename OP>
void
cwise(T& c, const T& a, const T& b, OP op)
{
    applier2<T>::cwise(c, a, b, op);
}
template <typename T, typename OP>
void
cwise(T& a, OP op)
{
    applier1<T>::cwise(a, op);
}

template <typename T, typename OP>
bool
all(const T& a, const T& b, OP op)
{
    return applier2<T>::all(a, b, op);
}
template <typename T, typename OP = std::function<bool(const typename T::value_t&)>>
bool
all(
    const T& a, OP op = [](const typename T::value_t& v) -> bool { return v; })
{
    return applier1<T>::all(a, op);
}

template <typename T, typename OP>
bool
any(const T& a, const T& b, OP op)
{
    return applier2<T>::any(a, b, op);
}
template <typename T, typename OP = std::function<bool(const typename T::value_t&)>>
bool
any(
    const T& a, OP op = [](const typename T::value_t& v) { return v; })
{
    return applier1<T>::any(a, op);
}

template <typename T, typename OP>
bool
none(const T& a, const T& b, OP op)
{
    return !applier2<T>::any(a, b, op);
}
template <typename T, typename OP = std::function<bool(const typename T::value_t&)>>
bool
none(
    const T& a, OP op = [](const typename T::value_t& v) { return v; })
{
    return !applier1<T>::any(a, op);
}

////////////////////////////////////////////////////////////////////////////////
}   // namespace detail

template <typename T, size_t DIM> struct vec {
    using value_t               = T;
    static constexpr size_t dim = DIM;
    T                       values[DIM];

    constexpr size_t size() const { return dim; }
    const T&         operator[](size_t i) const { return values[i]; }
    T&               operator[](size_t i) { return values[i]; }
};

template <typename T> struct vec<T, 2> {
    static constexpr size_t dim = 2;
    using value_t               = T;

    union {
        struct {
            T x, y;
        };
        struct {
            T u, v;
        };
    };

    constexpr size_t size() const { return dim; }
    const T&         operator[](size_t i) const { return (&x)[i]; }
    T&               operator[](size_t i) { return (&x)[i]; }
};

template <typename T> struct vec<T, 3> {
    static constexpr size_t dim = 3;
    using value_t               = T;

    union {
        struct {
            T x, y, z;
        };
        struct {
            T r, g, b;
        };
    };

    constexpr size_t size() const { return dim; }
    const T&         operator[](size_t i) const { return (&x)[i]; }
    T&               operator[](size_t i) { return (&x)[i]; }
};

template <typename T> struct vec<T, 4> {
    static constexpr size_t dim = 4;
    using value_t               = T;

    union {
        struct {
            T x, y, z, w;
        };
        struct {
            vec<T, 3> xyz;
            T         _padding1;
        };
        struct {
            T r, g, b, a;
        };
        struct {
            vec<T, 3> rgb;
            T         _padding2;
        };
    };

    constexpr size_t size() const { return dim; }
    const T&         operator[](size_t i) const { return (&x)[i]; }
    T&               operator[](size_t i) { return (&x)[i]; }
};

//////////////////////////////////////////////////////////////////////////////////

template <typename T, size_t DIM>
bool
operator==(const vec<T, DIM>& a, const vec<T, DIM>& b)
{
    return detail::all(a, b, std::equal_to<T>());
}
template <typename T, size_t DIM>
bool
operator!=(const vec<T, DIM>& a, const vec<T, DIM>& b)
{
    return !operator==(a, b);
}
template <typename T, size_t DIM>
vec<T, DIM>
operator+(const vec<T, DIM>& a, const vec<T, DIM>& b)
{
    vec<T, DIM> res;
    detail::cwise(res, a, b, std::plus<T>());
    return res;
}
template <typename T, size_t DIM>
vec<T, DIM>
operator+=(vec<T, DIM>& a, const vec<T, DIM>& b)
{
    detail::cwise(a, a, b, std::plus<T>());
    return a;
}

template <typename T, size_t DIM>
vec<T, DIM>
operator-(const vec<T, DIM>& a, const vec<T, DIM>& b)
{
    vec<T, DIM> res;
    detail::cwise(res, a, b, std::minus<T>());
    return res;
}
template <typename T, size_t DIM>
vec<T, DIM>
operator-=(vec<T, DIM>& a, const vec<T, DIM>& b)
{
    detail::cwise(a, a, b, std::minus<T>());
    return a;
}
template <typename T, size_t DIM>
vec<T, DIM>
operator*(const vec<T, DIM>& a, const vec<T, DIM>& b)
{
    vec<T, DIM> res;
    detail::cwise(res, a, b, std::multiplies<T>());
    return res;
}
template <typename T, size_t DIM>
vec<T, DIM>
operator*=(vec<T, DIM>& a, const vec<T, DIM>& b)
{
    detail::cwise(a, a, b, std::multiplies<T>());
    return a;
}
template <typename T, size_t DIM>
vec<T, DIM>
operator*(const vec<T, DIM>& a, T scalar)
{
    vec<T, DIM> result = a;
    detail::cwise(result, [scalar](const T& a) { return scalar * a; });
    return result;
}
template <typename T, size_t DIM>
vec<T, DIM>
operator*=(vec<T, DIM>& a, T scalar)
{
    detail::cwise(a, [scalar](const T& a) { return scalar * a; });
    return a;
}
template <typename T, size_t DIM>
vec<T, DIM>
operator/(const vec<T, DIM>& a, const vec<T, DIM>& b)
{
    vec<T, DIM> res;
    detail::cwise(res, a, b, std::divides<T>());
    return res;
}
template <typename T, size_t DIM>
vec<T, DIM>
operator/=(vec<T, DIM>& a, const vec<T, DIM>& b)
{
    ABC_ASSERT(detail::all(b, [](const T& v) { return v > 0; }));

    detail::cwise(a, a, b, std::divides<T>());
    return a;
}
template <typename T, size_t DIM>
vec<T, DIM>
operator/(vec<T, DIM>& a, T scalar)
{
    ABC_ASSERT(scalar > 0);
    vec<T, DIM> result = a;
    detail::cwise(result, [scalar](const T& a) { return scalar / a; });
    return result;
}
template <typename T, size_t DIM>
vec<T, DIM>
operator/=(vec<T, DIM>& a, T scalar)
{
    detail::cwise(a, [scalar](const T& a) { return scalar / a; });
    return a;
}

template <typename T, size_t DIM>
vec<T, DIM>
make_vec(const T* begin, const T* end)
{
    ABC_ASSERT(begin < end);
    ABC_ASSERT(end - begin == DIM);

    vec<T, DIM> result;
    T*          valuePtr    = &result[0];
    T*          valueEndPtr = &result[DIM - 1];

    for (; begin != end && valuePtr != valueEndPtr; ++begin) {
        *valuePtr++ = *begin;
    }
}

/////////////////////////////////////////////////////////////////////////////////

#define VEC_IMPL_SPECIALIZATIONS(DIM) \
    using vec##DIM##i8   = vec<int8_t, DIM>;    \
    using vec##DIM##u8   = vec<uint8_t, DIM>;   \
    using vec##DIM##i16  = vec<int16_t, DIM>;   \
    using vec##DIM##u16  = vec<uint16_t, DIM>;  \
    using vec##DIM##i132 = vec<int32_t, DIM>;   \
    using vec##DIM##u132 = vec<uint32_t, DIM>;  \
    using vec##DIM##f    = vec<float, DIM>;     \
    using vec##DIM##f    = vec<float, DIM>;     \
    using vec##DIM##d    = vec<double, DIM>;    \
    using vec##DIM##d    = vec<double, DIM>;
VEC_IMPL_SPECIALIZATIONS(2);
VEC_IMPL_SPECIALIZATIONS(3);
VEC_IMPL_SPECIALIZATIONS(4);
#undef VEC_IMPL_SPECIALIZATIONS

/////////////////////////////////////////////////////////////////////////////////
}   // namespace math
