#include "audio_sdl.h"

#include <cstdio>

const char* SDLAudioFormatToString(SDL_AudioFormat format)
{
    switch (format)
    {
    case AUDIO_S8:
        return "AUDIO_S8";
    case AUDIO_U8:
        return "AUDIO_U8";
    case AUDIO_S16MSB:
        return "AUDIO_S16MSB";
    case AUDIO_S16LSB:
        return "AUDIO_S16LSB";
    case AUDIO_U16MSB:
        return "AUDIO_U16MSB";
    case AUDIO_U16LSB:
        return "AUDIO_U16LSB";
    case AUDIO_S32MSB:
        return "AUDIO_S32MSB";
    case AUDIO_S32LSB:
        return "AUDIO_S32LSB";
    case AUDIO_F32MSB:
        return "AUDIO_F32MSB";
    case AUDIO_F32LSB:
        return "AUDIO_F32LSB";
    }
    return "Unknown audio format";
}

SDL_AudioFormat AudioFormatToSDLAudioFormat(AudioFormat format)
{
    switch (format)
    {
    case AudioFormat::S16:
        return AUDIO_S16;
    case AudioFormat::S32:
        return AUDIO_S32;
    case AudioFormat::F32:
        return AUDIO_F32;
    default:
        fprintf(stderr, "Invalid audio format conversion\n");
        exit(1);
    }
}
