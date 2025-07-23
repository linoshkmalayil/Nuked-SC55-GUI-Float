#pragma once

#include "output_common.h"

#include "ringbuffer.h"

bool Out_SDL_QueryOutputs(AudioOutputList& list);

bool Out_SDL_Create(const char* device_name, const AudioOutputParameters& params);
// Implies Out_SDL_Stop()
void Out_SDL_Destroy();

bool Out_SDL_Start();
void Out_SDL_Stop();

void Out_SDL_AddSource(RingbufferView& view);

void Out_SDL_SetVolume(float vol, AudioVolume &volume_control);
