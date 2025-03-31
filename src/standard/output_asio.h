#pragma once

#include "output_common.h"

#include <SDL.h>

bool Out_ASIO_QueryOutputs(AudioOutputList& list);

bool Out_ASIO_Create(const char* driver_name, const AudioOutputParameters& params);
// Implies Out_ASIO_Stop()
void Out_ASIO_Destroy();

bool Out_ASIO_Start();
void Out_ASIO_Stop();

bool Out_ASIO_IsResetRequested();
bool Out_ASIO_Reset();

// Adds a stream to be mixed into the ASIO output. It should not be freed until ASIO shuts down.
void Out_ASIO_AddSource(SDL_AudioStream* stream);

int             Out_ASIO_GetFrequency();
SDL_AudioFormat Out_ASIO_GetFormat();

// Returns the size of a single sample in bytes.
size_t Out_ASIO_GetFormatSampleSizeBytes();

// Returns the size of a single frame in bytes.
size_t Out_ASIO_GetFormatFrameSizeBytes();

size_t Out_ASIO_GetBufferSize();
