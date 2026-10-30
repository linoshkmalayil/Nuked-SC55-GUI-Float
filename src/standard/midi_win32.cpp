/*
 * Copyright (C) 2021, 2024 nukeykt
 * Modified by J.C. Moyer and Linosh K M
 * Original source file: src/midi_win32.cpp
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

#include "midi.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "common/command_line.h"
#include "common/term_io.h"
#include <cstdint>
#include <mmsystem.h>
#include <span>
#include <string>

static HMIDIIN  midi_in_handle;
static HMIDIOUT midi_out_handle;
static MIDIHDR midi_buffer;

static uint8_t midi_in_buffer[1024];

static MIDI_Output* midi_output = nullptr;

void CALLBACK MIDIIN_Callback(
    HMIDIIN   hMidiIn,
    UINT      wMsg,
    DWORD_PTR dwInstance,
    DWORD_PTR dwParam1,
    DWORD_PTR dwParam2
)
{
    (void)hMidiIn;
    (void)dwInstance;
    (void)dwParam2;

    switch (wMsg)
    {
        case MIM_OPEN:
            break;
        case MIM_DATA:
        {
            uint8_t b1 = dwParam1 & 0xff;
            switch (b1 & 0xf0)
            {
                case 0x80:
                case 0x90:
                case 0xa0:
                case 0xb0:
                case 0xe0:
                    {
                        uint8_t buf[3] = {
                            (uint8_t)b1,
                            (uint8_t)((dwParam1 >> 8) & 0xff),
                            (uint8_t)((dwParam1 >> 16) & 0xff),
                        };
                        midi_output->MIDIWrite(buf);
                    }
                    break;
                case 0xc0:
                case 0xd0:
                    {
                        uint8_t buf[2] = {
                            (uint8_t)b1,
                            (uint8_t)((dwParam1 >> 8) & 0xff),
                        };
                        midi_output->MIDIWrite(buf);
                    }
                    break;
            }
            break;
        }
        case MIM_LONGDATA:
        case MIM_LONGERROR:
        {
            MMRESULT result = midiInUnprepareHeader(midi_in_handle, &midi_buffer, sizeof(MIDIHDR));
            if (result == MMSYSERR_INVALHANDLE)
            {
                // If this happens, the frontend probably called MIDI_Quit and
                // midi_frontend is no longer valid. We got here because this
                // callback is running in a separate thread and might be called
                // after MIDI_Quit.
                break;
            }

            if (wMsg == MIM_LONGDATA)
            {
                midi_output->MIDIWrite(std::span(midi_in_buffer, midi_buffer.dwBytesRecorded));
            }

            midiInPrepareHeader(midi_in_handle, &midi_buffer, sizeof(MIDIHDR));
            midiInAddBuffer(midi_in_handle, &midi_buffer, sizeof(MIDIHDR));

            break;
        }
        default:
            common::Printf("hmm");
            break;
    }
}

void MIDI_PrintDevices()
{
    const UINT num_devices_in  = midiInGetNumDevs();
    const UINT num_devices_out = midiOutGetNumDevs();

    if (num_devices_in == 0)
    {
        common::Printf("No midi input devices found.\n");
    }

    if (num_devices_out == 0)
    {
        common::Printf("No midi output devices found.\n");
    }

    MMRESULT     result;
    MIDIINCAPSA  device_in_caps;
    MIDIOUTCAPSA device_out_caps;

    common::Printf("Known midi input devices:\n\n");

    for (UINT i = 0; i < num_devices_in; ++i)
    {
        result = midiInGetDevCapsA(i, &device_in_caps, sizeof(MIDIINCAPSA));
        if (result == MMSYSERR_NOERROR)
        {
            common::Printf("  %d: %s\n", i, device_in_caps.szPname);
        }
    }

    common::Printf("\nKnown midi output devices:\n\n");

    for (UINT i = 0; i < num_devices_out; ++i)
    {
        result = midiOutGetDevCapsA(i, &device_out_caps, sizeof(MIDIOUTCAPSA));
        if (result == MMSYSERR_NOERROR)
        {
            common::Printf("  %d: %s\n", i, device_out_caps.szPname);
        }
    }
}

struct MIDI_PickedDevice
{
    INT          device_in_id  = -1;
    MIDIINCAPSA  device_in_caps;
    INT          device_out_id = -1;
    MIDIOUTCAPSA device_out_caps;
};

bool MIDI_PickDevice(std::string_view preferred_in_name, std::string_view preferred_out_name, MIDI_PickedDevice& out_picked)
{
    const UINT num_in_devices  = midiInGetNumDevs();
    const UINT num_out_devices = midiOutGetNumDevs();

    if (num_in_devices == 0)
    {
        common::Printf("No midi input\n");
        return false;
    }

    MMRESULT result;

    if (preferred_out_name.size() == 0)
    {
        // default to first device
        result = midiOutGetDevCapsA(0, &out_picked.device_out_caps, sizeof(MIDIOUTCAPSA));
        if (result != MMSYSERR_NOERROR)
        {
            common::Printf("midiOutGetDevCapsA failed\n");
            return false;
        }
        out_picked.device_out_id = 0;
    }
    else
    {
        if (num_out_devices == 0)
        {
            common::Printf("No midi output\n");
        }
    }

    if (preferred_in_name.size() == 0)
    {
        // default to first device
        result = midiInGetDevCapsA(0, &out_picked.device_in_caps, sizeof(MIDIINCAPSA));
        if (result != MMSYSERR_NOERROR)
        {
            common::Printf("midiInGetDevCapsA failed\n");
            return false;
        }
        out_picked.device_in_id = 0;

        return true;
    }

    for (UINT i = 0; i < num_in_devices; ++i)
    {
        result = midiInGetDevCapsA(i, &out_picked.device_in_caps, sizeof(MIDIINCAPSA));
        if (result != MMSYSERR_NOERROR)
        {
            common::Printf("midiInGetDevCapsA failed\n");
            return false;
        }
        if (std::string_view(out_picked.device_in_caps.szPname) == preferred_in_name)
        {
            out_picked.device_in_id = i;
            break;
        }
    }
    for (UINT j = 0; j < num_out_devices; ++j)
    {
        result = midiOutGetDevCapsA(j, &out_picked.device_out_caps, sizeof(MIDIOUTCAPSA));
        if (result != MMSYSERR_NOERROR)
        {
            common::Printf("midiOutGetDevCapsA failed\n");
            return false;
        }
        if (std::string_view(out_picked.device_out_caps.szPname) == preferred_out_name)
        {
            out_picked.device_out_id = j;
            break;
        }
    }

    // user provided a number
    if (UINT device_in_id; common::TryParse(preferred_in_name, device_in_id))
    {
        if (device_in_id < num_in_devices)
        {
            result = midiInGetDevCaps(device_in_id, &out_picked.device_in_caps, sizeof(MIDIINCAPSA));
            if (result != MMSYSERR_NOERROR)
            {
                common::Printf("midiInGetDevCapsA failed\n");
                return false;
            }
            out_picked.device_in_id = device_in_id;
        }
    }
    if (UINT device_out_id; common::TryParse(preferred_out_name, device_out_id))
    {
        if (device_out_id < num_out_devices)
        {
            result = midiOutGetDevCaps(device_out_id, &out_picked.device_out_caps, sizeof(MIDIOUTCAPSA));
            if (result != MMSYSERR_NOERROR)
            {
                common::Printf("midiOutGetDevCapsA failed\n");
                return false;
            }
            out_picked.device_out_id = device_out_id;
        }
    
    }
    
    if (out_picked.device_in_id < 0)
    {
        common::Printf("No input device named '%s'\n", std::string(preferred_in_name).c_str());
    }
    if (out_picked.device_out_id < 0)
    {
        common::Printf("No output device named '%s'\n", std::string(preferred_out_name).c_str());
    }
    
    return out_picked.device_in_id >= 0 || out_picked.device_out_id >= 0;
}

bool MIDI_Init(MIDI_Output& output, std::string_view in_port_name_or_id, std::string_view out_port_name_or_id)
{
    midi_output = &output;

 MIDI_PickedDevice picked_device;
    if (!MIDI_PickDevice(in_port_name_or_id, out_port_name_or_id, picked_device))
    {
        return false;
    }

    if (picked_device.device_in_id > -1)
    {
        MMRESULT resultin = midiInOpen(&midi_in_handle, picked_device.device_in_id, (DWORD_PTR)MIDIIN_Callback, 0, CALLBACK_FUNCTION);
        if (resultin != MMSYSERR_NOERROR)
        {
            common::Printf("midiInOpen failed\n");
            return false;
        }

        common::Printf("Opened midi input port: %s\n", picked_device.device_in_caps.szPname);

        midi_buffer.lpData         = (LPSTR)midi_in_buffer;
        midi_buffer.dwBufferLength = sizeof(midi_in_buffer);

        resultin = midiInPrepareHeader(midi_in_handle, &midi_buffer, sizeof(MIDIHDR));
        if (resultin != MMSYSERR_NOERROR)
        {
            common::Printf("midiInPrepareHeader failed\n");
            return false;
        }

        resultin = midiInAddBuffer(midi_in_handle, &midi_buffer, sizeof(MIDIHDR));
        if (resultin != MMSYSERR_NOERROR)
        {
            common::Printf("midiInAddBuffer failed\n");
            return false;
        }

        resultin = midiInStart(midi_in_handle);
        if (resultin != MMSYSERR_NOERROR)
        {
            common::Printf("midiInStart failed\n");
            return false;
        }
    }

    if (out_port_name_or_id.size() && picked_device.device_out_id > -1)
    {
        MMRESULT resultout = midiOutOpen(&midi_out_handle, picked_device.device_out_id, (DWORD_PTR)NULL, 0, CALLBACK_NULL);
        if (resultout == MMSYSERR_NOERROR)
            common::Printf("Opened midi output port: %s\n", picked_device.device_out_caps.szPname);
    }

    return true;
}

void MIDI_Quit()
{
    if (midi_in_handle)
    {
        midiInStop(midi_in_handle);
        midiInClose(midi_in_handle);
        midi_in_handle = 0;
    }
    if (midi_out_handle)
    {
        midiOutClose(midi_out_handle);
        midi_out_handle = 0;
    }
    midi_output = nullptr;
}

void MIDI_PostShortMessage(uint8_t *message, int len) {
    if (midi_out_handle) {
        DWORD msg = 0;
        for (int i = 0; i < len; i++) {
            msg |= *message++ << i * 8;
        }
        midiOutShortMsg(midi_out_handle, msg);
    }
}
void MIDI_PostSysExMessage(uint8_t *message, int len) {
    if (midi_out_handle) {
        MIDIHDR buffer;
        memset(&buffer, 0, sizeof buffer);
        buffer.dwBufferLength = len;
        buffer.lpData = (LPSTR) message;
        midiOutPrepareHeader(midi_out_handle, &buffer, sizeof buffer);
        midiOutLongMsg(midi_out_handle, &buffer, sizeof buffer);
        midiOutUnprepareHeader(midi_out_handle, &buffer, sizeof buffer);
    }
}

void MIDI_MIDIOutCallBack(void* userdata, uint8_t* message, int len)
{
    (void)userdata;
    MIDI_PostSysExMessage(message, len);

}
