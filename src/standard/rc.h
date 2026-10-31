/*
 * Copyright (C) 2021, 2026 Linosh K M
 *
 * This file is part of Nuked-SC55-GUI-Float (Originally Nuked-SC55).
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#pragma once

#include <cstdint>
#include <string>

// Abstractly represents something that can accept MIDI data and do something
// with it. MIDI backends like win32 or rtmidi should forward MIDI data to the
// output they receive at initialization.
struct REMOTE_Output
{
    virtual void RemoteWrite(uint8_t rbyte) = 0;
};

bool REMOTE_Init(REMOTE_Output& output, std::string_view rc_pipe);
void REMOTE_Quit();
