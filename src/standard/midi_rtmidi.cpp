/*
 * Copyright (C) 2021, 2024 nukeykt
 * Modified by J.C. Moyer and Linosh K M
 * Original source file: src/midi_rtmidi.cpp
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
#include "common/command_line.h"
#include "common/term_io.h"
#include <RtMidi.h>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <span>
#include <string>
#include <vector>

static RtMidiIn *s_midi_in = nullptr;
static RtMidiOut *s_midi_out = nullptr;

static MIDI_Output* midi_output = nullptr;

static void MidiOnReceive(double, std::vector<uint8_t> *message, void *)
{
    midi_output->MIDIWrite(*message);
}

static void MidiOnError(RtMidiError::Type, const std::string &errorText, void *)
{
    common::Printf("RtMidi: Error has occured: %s\n", errorText.c_str());
    fflush(stderr);
}

// rtmidi will append a space and the port number to the port name which makes it useless as a stable string identifier
void MIDI_StripRtmidiPortNumber(std::string& port_name)
{
    if (port_name.size() == 0)
    {
        return;
    }

    if (port_name.back() < '0' || port_name.back() > '9')
    {
        return;
    }

    auto last_space = port_name.rfind(' ');
    if (last_space != std::string_view::npos)
    {
        port_name.resize(last_space);
    }
}

void MIDI_PrintDevices()
{
    try
    {
        std::unique_ptr<RtMidiIn> midiin = std::make_unique<RtMidiIn>();
        std::unique_ptr<RtMidiOut> midiout = std::make_unique<RtMidiOut>();

        const unsigned int num_devices_in = midiin->getPortCount();
        const unsigned int num_devices_out = midiout->getPortCount();

        if (num_devices_in == 0)
        {
            common::Printf("No MIDI Input devices found.\n");
        }
        if (num_devices_out == 0)
        {
            common::Printf("No MIDI Output devices found.\n");
        }

        common::Printf("Known MIDI Input devices:\n\n");

        for (unsigned int i = 0; i < num_devices_in; ++i)
        {
            std::string friendly_name = midiin->getPortName(i);
            MIDI_StripRtmidiPortNumber(friendly_name);

            common::Printf("  %d: %s\n", i, friendly_name.c_str());
        }

        common::Printf("\nKnown MIDI Output devices:\n\n");

        for (unsigned int i = 0; i < num_devices_out; ++i)
        {
            std::string friendly_name = midiout->getPortName(i);
            MIDI_StripRtmidiPortNumber(friendly_name);

            common::Printf("  %d: %s\n", i, friendly_name.c_str());
        }
    }
    catch (const RtMidiError& err)
    {
        // this exception shouldn't escape to the caller since it's not exception-aware
        common::Printf("Failed to enumerate midi devices: %s\n", err.getMessage().c_str());
    }
}

struct MIDI_PickedDevices
{
    unsigned int in_device_id, out_device_id;
    std::string  in_device_name, out_device_name;
};

// throws RtMidiError
bool MIDI_PickDevice(RtMidiIn& midi_in, RtMidiOut& midi_out, std::string_view preferred_in_name, std::string_view preferred_out_name, MIDI_PickedDevices& out_picked)
{
    const unsigned int num_devices_in  = midi_in.getPortCount();
    const unsigned int num_devices_out = midi_out.getPortCount();

    if (num_devices_in == 0)
    {
        common::Printf("No midi input\n");
        return false;
    }

    if (num_devices_out == 0)
    {
        common::Printf("No midi output\n");
    }

    if (preferred_out_name.size() == 0)
    {
        // default to first device
        out_picked.out_device_id   = 0;
        out_picked.out_device_name = midi_out.getPortName(0);
    }

    if (preferred_in_name.size() == 0)
    {
        // default to first device
        out_picked.in_device_id   = 0;
        out_picked.in_device_name = midi_in.getPortName(0);
        return true;
    }

    bool output_set = false;

    for (unsigned int i= 0; i < num_devices_out; i++)
    {
        std::string midi_out_name = midi_out.getPortName(i);
        if(preferred_out_name == midi_out_name)
        {
            out_picked.out_device_id   = i;
            out_picked.out_device_name = std::move(midi_out_name);
            output_set = true;
            break;
        }

        MIDI_StripRtmidiPortNumber(midi_out_name);
        if(preferred_out_name == midi_out_name)
        {
            out_picked.out_device_id   = i;
            out_picked.out_device_name = std::move(midi_out_name);
            output_set = true;
            break;
        }
    }

    for (unsigned int i= 0; i < num_devices_in; i++)
    {
        std::string midi_in_name = midi_in.getPortName(i);
        if(preferred_out_name == midi_in_name)
        {
            out_picked.in_device_id   = i;
            out_picked.in_device_name = std::move(midi_in_name);
            return true;
        }

        MIDI_StripRtmidiPortNumber(midi_in_name);
        if(preferred_out_name == midi_in_name)
        {
            out_picked.in_device_id   = i;
            out_picked.in_device_name = std::move(midi_in_name);
            return true;
        }

    }

    // user provided a number
    if (unsigned int device_id; common::TryParse(preferred_out_name, device_id))
    {
        if (device_id < num_devices_out)
        {
            out_picked.out_device_id   = device_id;
            out_picked.out_device_name = midi_out.getPortName(device_id);
            output_set = true;
        }
    }

    if (unsigned int device_id; common::TryParse(preferred_in_name, device_id))
    {
        if (device_id < num_devices_in)
        {
            out_picked.in_device_id   = device_id;
            out_picked.in_device_name = midi_in.getPortName(device_id);
            return true;
        }
    }

    fprintf(stderr, "No input device named '%s'\n", std::string(preferred_in_name).c_str());
    
    if (output_set)
        fprintf(stderr, "No output device named '%s'\n", std::string(preferred_out_name).c_str());
    
    return false;
}

bool MIDI_Init(MIDI_Output& output, std::string_view in_port_name_or_id, std::string_view out_port_name_or_id)
{
    if (s_midi_in)
    {
        common::Printf("MIDI input already running\n");
        return false; // Already running
    }

    if (s_midi_out)
    {
        common::Printf("MIDI output already running\n");
        return false; // Already running
    }

    midi_output = &output;
    
    s_midi_in = new RtMidiIn(RtMidi::UNSPECIFIED, "Nuked SC55", 1024);
    s_midi_in->ignoreTypes(false, false, false); // SysEx disabled by default
    s_midi_in->setCallback(&MidiOnReceive, nullptr); // FIXME: (local bug) Fix the linking error
    s_midi_in->setErrorCallback(&MidiOnError, nullptr);

    s_midi_out = new RtMidiOut(RtMidi::UNSPECIFIED, "Nuked SC55");
    s_midi_out->setErrorCallback(&MidiOnError, nullptr);

    MIDI_PickedDevices picked_device;

    try
    {
        if (!MIDI_PickDevice(*s_midi_in, *s_midi_out, in_port_name_or_id, out_port_name_or_id, picked_device))
        {
            common::Printf("Failed to initialize RtMidi\n");
            return false;
        }
    }
    catch (const RtMidiError& err)
    {
        common::Printf("Failed to initialize RtMidi: %s\n", err.getMessage().c_str());
    }

    s_midi_in->openPort(picked_device.in_device_id, "Nuked SC55");
    common::Printf("Opened midi in port: %s\n", picked_device.in_device_name.c_str());

    if(!out_port_name_or_id.empty())
    {
        s_midi_out->openPort(picked_device.out_device_id, "Nuked-SC55");
        common::Printf("Opened midi out port: %s\n", picked_device.out_device_name.c_str());
    }

    return true;
}

void MIDI_Quit()
{
    if (s_midi_out)
    {
        s_midi_out->closePort();
        delete s_midi_out;
        s_midi_out = nullptr;
    }
    if (s_midi_in)
    {
        s_midi_in->closePort();
        delete s_midi_in;
        s_midi_in = nullptr;
        midi_output = nullptr;
    }
    
}

void MIDI_PostShortMessage(uint8_t *message, int len) {
    if (s_midi_out) {
        s_midi_out->sendMessage(message, len);
    }
}
void MIDI_PostSysExMessage(uint8_t *message, int len) {
    if (s_midi_out) {
        s_midi_out->sendMessage(message, len);
    }
}

void MIDI_MIDIOutCallBack(void* userdata, uint8_t* message, int len)
{
    (void)userdata;
    MIDI_PostSysExMessage(message, len);

}
