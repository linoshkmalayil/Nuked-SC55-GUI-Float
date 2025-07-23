#pragma once

#include "math_util.h"
#include <cstddef>
#include <cstdint>

const uint32_t DEFAULT_VOLUME = 10386; // -16 dB

static float volume = (float) DEFAULT_VOLUME / 65535.0f;
static uint32_t volume_fp = DEFAULT_VOLUME;

enum class AudioFormat
{
    S16,
    S32,
    F32,
};

template <typename T>
struct AudioFrame
{
    T left;
    T right;

    static constexpr size_t channel_count = 2;
};

inline void Normalize(const AudioFrame<int32_t>& in, AudioFrame<int16_t>& out)
{
    out.left  = (int16_t)Clamp<int64_t>(((int64_t)in.left  * (int64_t)volume_fp) >> 28, INT16_MIN, INT16_MAX);
    out.right = (int16_t)Clamp<int64_t>(((int64_t)in.right * (int64_t)volume_fp) >> 28, INT16_MIN, INT16_MAX);
}

inline void Normalize(const AudioFrame<int32_t>& in, AudioFrame<int32_t>& out)
{
    out.left  = (int32_t)Clamp<int64_t>(((int64_t)in.left  * (int64_t)volume_fp) >> 12, INT32_MIN, INT32_MAX);
    out.right = (int32_t)Clamp<int64_t>(((int64_t)in.right * (int64_t)volume_fp) >> 12, INT32_MIN, INT32_MAX);
}

inline void Normalize(const AudioFrame<int32_t>& in, AudioFrame<float>& out)
{
    constexpr float DIV_REC = 1.0f / 67108864.0f;

    out.left  = (float)in.left  * DIV_REC * volume;
    out.right = (float)in.right * DIV_REC * volume;
}


inline void MixFrame(AudioFrame<int16_t>& dest, const AudioFrame<int16_t>& src)
{
    dest.left  = SaturatingAdd(dest.left, src.left);
    dest.right = SaturatingAdd(dest.right, src.right);
}

inline void MixFrame(AudioFrame<int32_t>& dest, const AudioFrame<int32_t>& src)
{
    dest.left  = SaturatingAdd(dest.left, src.left);
    dest.right = SaturatingAdd(dest.right, src.right);
}

inline void MixFrame(AudioFrame<float>& dest, const AudioFrame<float>& src)
{
    dest.left  += src.left;
    dest.right += src.right;
}
