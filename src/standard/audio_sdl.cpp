#include "audio_sdl.h"

#include <cstdio>

const char* SDLAudioFormatToString(SDL_AudioFormat format)
{
    switch (format)
    {
    case SDL_AUDIO_S8:
        return "SDL_AUDIO_S8";
    case SDL_AUDIO_U8:
        return "SDL_AUDIO_U8";
    case SDL_AUDIO_S16BE:
        return "SDL_AUDIO_S16BE";
    case SDL_AUDIO_S16LE:
        return "SDL_AUDIO_S16LE";
    case SDL_AUDIO_S32BE:
        return "SDL_AUDIO_S32BE";
    case SDL_AUDIO_S32LE:
        return "SDL_AUDIO_S32LE";
    case SDL_AUDIO_F32BE:
        return "SDL_AUDIO_F32BE";
    case SDL_AUDIO_F32LE:
        return "SDL_AUDIO_F32LE";
    }
    return "Unknown audio format";
}

SDL_AudioFormat AudioFormatToSDLAudioFormat(AudioFormat format)
{
    switch (format)
    {
    case AudioFormat::S16:
        return SDL_AUDIO_S16;
    case AudioFormat::S32:
        return SDL_AUDIO_S32;
    case AudioFormat::F32:
        return SDL_AUDIO_F32;
    default:
        fprintf(stderr, "Invalid audio format conversion\n");
        exit(1);
    }
}
