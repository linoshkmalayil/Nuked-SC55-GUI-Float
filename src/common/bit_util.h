#pragma once

#include <limits>
#include <type_traits>
#include <bit>

namespace fe
{
#if defined(__cpp_lib_int_pow2) && (__cpp_lib_int_pow2 >= 202002L)
template <typename T>
constexpr bool has_single_bit(T value)
{
    return std::has_single_bit(value);
}

template <typename T>
constexpr T bit_floor(T value)
{
    return std::bit_floor(value);
}

template <typename T>
constexpr T bit_ceil(T value)
{
    return std::bit_ceil(value);
}
#else
template <typename T>
constexpr bool has_single_bit(T value)
{
    static_assert(std::is_integral_v<T>, "has_single_bit requires an integral type");
    using U = std::make_unsigned_t<T>;
    U uvalue = static_cast<U>(value);
    return uvalue != 0 && (uvalue & (uvalue - 1)) == 0;
}

template <typename T>
constexpr T bit_floor(T value)
{
    static_assert(std::is_integral_v<T>, "bit_floor requires an integral type");
    using U = std::make_unsigned_t<T>;
    U uvalue = static_cast<U>(value);
    if (uvalue == 0)
    {
        return 0;
    }

    U result = 1;
    while (result <= (uvalue >> 1))
    {
        result <<= 1;
    }
    return static_cast<T>(result);
}

template <typename T>
constexpr T bit_ceil(T value)
{
    static_assert(std::is_integral_v<T>, "bit_ceil requires an integral type");
    using U = std::make_unsigned_t<T>;
    U uvalue = static_cast<U>(value);
    if (uvalue <= 1)
    {
        return 1;
    }

    U result = 1;
    while (result < uvalue)
    {
        if (result > (std::numeric_limits<U>::max() >> 1))
        {
            return 0;
        }
        result <<= 1;
    }
    return static_cast<T>(result);
}
#endif
}
