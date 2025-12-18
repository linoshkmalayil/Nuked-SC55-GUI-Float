#include "gain.h"

#include <charconv>
#include <cmath>

namespace common
{

enum class ParseUnit
{
    Scalar,
    Decibels,
};

float DbToScalar(float db)
{
    return std::powf(10.f, db / 20.f);
}

float ScalarToDb(float scalar)
{
    return 20.f * std::log10f(scalar);
}

static bool IsDigit(char ch)
{
    return ch >= '0' && ch <= '9';
}

static bool IsParsableNumber(std::string_view str)
{
    bool decimal = false;

    for (size_t i = 0; i < str.size(); ++i)
    {
        const char ch = str[i];

        if (ch == '.')
        {
            if (decimal)
            {
                return false;
            }
            else
            {
                decimal = true;
            }
        }
        else if (ch == '-' || ch == '+')
        {
            // sign may only appear in first position
            if (i > 0)
            {
                return false;
            }
        }
        else if (!IsDigit(ch))
        {
            return false;
        }
    }

    return true;
}

ParseGainResult ParseGain(std::string_view str, float& out_gain)
{
    using namespace std::literals;

    ParseUnit unit = ParseUnit::Scalar;

    if (str.ends_with("db"sv))
    {
        unit = ParseUnit::Decibels;
        str.remove_suffix(2);
    }

    if (!IsParsableNumber(str))
    {
        return ParseGainResult::InvalidNumber;
    }

    // from_chars handles leading '-' but not '+'
    if (str.starts_with('+'))
    {
        str.remove_prefix(1);
    }

    const char* const n_first = str.data();
    const char* const n_last  = str.data() + str.size();

    float num = 0.0f;

    auto fc_result = std::from_chars(n_first, n_last, num);

    if (fc_result.ec != std::errc{})
    {
        return ParseGainResult::ParseFailed;
    }

    switch (unit)
    {
    case ParseUnit::Scalar:
        out_gain = num;
        break;
    case ParseUnit::Decibels:
        out_gain = DbToScalar(num);
        break;
    }

    if (out_gain < 0)
    {
        return ParseGainResult::OutOfRange;
    }

    return ParseGainResult{};
}

} // namespace common
