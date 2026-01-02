#pragma once

#include <string_view>

namespace common
{

enum class ParseGainResult
{
    TooShort = 1,
    InvalidNumber,
    ParseFailed,
    OutOfRange,
};

ParseGainResult ParseGain(std::string_view str, float& out_gain);

float DbToScalar(float db);
float ScalarToDb(float scalar);

} // namespace common
