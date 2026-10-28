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

// Abstractly represents something that can accept Serial data and do something
// with it. Serial backends like win32 or rtmidi should forward MIDI data to the
// output they receive at initialization.
struct SERIAL_Output
{
    virtual void SerialWrite(const uint8_t byte) = 0;
};

bool SERIAL_Init(SERIAL_Output& output, std::string_view serial_port);
void SERIAL_Update();
bool SERIAL_HasData();
uint8_t SERIAL_ReadUART();
void SERIAL_PostUART(uint8_t data);
void SERIAL_Quit();
