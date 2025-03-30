// This header contains SDL-specific extensions to audio.h and utilities related to dealing with SDL audio
 
#pragma once
 
#include <SDL.h>

#include "audio.h"

const char* SDLAudioFormatToString(SDL_AudioFormat format);

SDL_AudioFormat AudioFormatToSDLAudioFormat(AudioFormat format);
