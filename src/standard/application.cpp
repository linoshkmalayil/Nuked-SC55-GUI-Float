/*
 * Copyright (C) 2024-2026 J.C. Moyer
 * Modified by Linosh K M
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

#include "application.h"

#include "common/gain.h"
#include "common/path_util.h"
#include "common/term_io.h"
#include "common/bit_util.h"

Application::~Application()
{
    switch (m_audio_output.kind)
    {
    case AudioOutputKind::ASIO:
#if NUKED_ENABLE_ASIO
        Out_ASIO_Stop();
        Out_ASIO_Destroy();
#else
        common::Printf("Out_ASIO_Stop() called without ASIO support\n");
#endif
        break;
    case AudioOutputKind::SDL:
        Out_SDL_Stop();
        Out_SDL_Destroy();
        break;
    }

    SERIAL_Quit();
    MIDI_Quit();
    REMOTE_Quit();
}

bool Application::Initialize(const CliParameters& params)
{
    std::filesystem::path base_path = common::GetProcessPath().parent_path();

    if (std::filesystem::exists(base_path / "../share/nuked-sc55"))
        base_path = base_path / "../share/nuked-sc55";

    common::Printf("Base path is: %s\n", base_path.generic_string().c_str());

    std::filesystem::path rom_directory;

    if (params.rom_directory)
    {
        rom_directory = *params.rom_directory;
    }
    else
    {
        rom_directory = base_path;
    }

    common::Printf("ROM directory is: %s\n", rom_directory.generic_string().c_str());

    common::LoadRomsetResult load_result;

    common::LoadRomsetError err =
        common::LoadRomset(rom_directory, params.romset_name, params.rom_loader, params.adv.rom_overrides, load_result);

    common::PrintLoadRomsetDiagnostics(err, load_result);

    if (err != common::LoadRomsetError{})
    {
        return false;
    }

    m_romset      = load_result.romset;
    m_romset_info = std::move(load_result.romset_info);

    EMU_SystemReset reset = EMU_SystemReset::NONE;
    if (params.reset)
    {
        reset = *params.reset;
    }
    else if (!params.reset && m_romset == Romset::MK2)
    {
        // user didn't explicitly pass a reset and we're using a buggy romset
        common::Printf("WARNING: No reset specified with mk2 romset; using gs\n");
        reset = EMU_SystemReset::GS_RESET;
    }

    common::Printf("Gain set to %.2fdb\n", common::ScalarToDb(params.gain));

    for (size_t i = 0; i < params.instances; ++i)
    {
        if (!CreateInstance(params))
        {
            common::Printf("FATAL ERROR: Failed to create instance %zu\n", i);
            return false;
        }
    }

    load_result.Purge();

    if (params.serial_type != Computerswitch::MIDI)
    {
        for (Instance& inst : m_instances)
        {
            inst.GetEmulator().PostSerialSystemReset(reset);
        }

    }
    else
    {
        for (Instance& inst : m_instances)
        {
            inst.GetEmulator().PostSystemReset(reset);
        }
    }

    if (!OpenAudio(params))
    {
        common::Printf("FATAL ERROR: Failed to open the audio stream.\n");
        fflush(stderr);
        return false;
    }

    // Check if Serial IO is used and ROM is MK2 and ST
    if ((params.serial_type != Computerswitch::MIDI && !params.serial_port.empty()) && (m_romset==Romset::MK2 || m_romset==Romset::ST))
    {
        if (!SERIAL_Init(*this, params.serial_port))
        {
            fprintf(stderr, "ERROR: Failed to initialize the Serial I/O.\nWARNING: Continuing without Serial I/O...\n");
            fflush(stderr);
        }
        else
        {
            is_serial = true;
            SetSerialOutCallback();
        }
    }
    //Serial Type set but Seral Port not specified
    else if (params.serial_type != Computerswitch::MIDI && !params.serial_port.empty())
    {
        fprintf(stderr, "ERROR: Serial I/O available only for SC-55mk2 and SC-55st models.\nWARNING: Continuing without Serial I/O...\n");
        fflush(stderr);
    }
    //Continue to MIDI
    else
    {
        std::string midiout_device = params.midiout_device;
        if (!params.midiout_device.empty() && m_romset == Romset::ST)
        {
            common::Printf("WARNING: MIDI Output is not available for SC-55st, continuing with only MIDI Input...");
            midiout_device = "";
        }

        if (!MIDI_Init(*this, params.midiin_device, midiout_device))
        {
            common::Printf("ERROR: Failed to initialize the MIDI Input.\nWARNING: Continuing without MIDI Input...\n");
            fflush(stderr);
        }
        else if (!midiout_device.empty())
        {
            SetMIDIOutCallback();
        }
    }

    if (!params.remote_pipe.empty() && (m_romset == Romset::MK1 || m_romset == Romset::MK2 || m_romset == Romset::SC155 || m_romset == Romset::SC155MK2))
    {
        if (!REMOTE_Init(*this, params.remote_pipe))
        {
            common::Printf("ERROR: Failed to initialize the Remote Control Pipe.\nWARNING: Continuing without Remote Control...\n");
            fflush(stderr);
        }
    }
    else if (!params.remote_pipe.empty())
    {
        common::Printf("ERROR: Remote Control available only for SC-55, SC-55mk2, SC-155 and SC-155mk2 models.\nWARNING: Continuing without Remote Control...\n");
        fflush(stderr);
    }

    PrintControls(m_romset);

    return true;
}

bool Application::AllocateInstance(Instance** result)
{
    if (m_instances.IsFull())
    {
        return false;
    }

    Instance& fe = m_instances.EmplaceBack();

    if (result)
    {
        *result = &fe;
    }

    return true;
}

void Application::SendMIDI(size_t instance_id, std::span<const uint8_t> bytes)
{
    m_instances[instance_id].GetEmulator().PostMIDI(bytes);
}

void Application::BroadcastMIDI(std::span<const uint8_t> bytes)
{
    for (size_t i = 0; i < m_instances.Count(); ++i)
    {
        SendMIDI(i, bytes);
    }
}

void Application::RouteMIDI(std::span<const uint8_t> bytes)
{
    if (bytes.size() == 0)
    {
        return;
    }

    uint8_t first = bytes[0];

    if (first < 0x80)
    {
        common::Printf("Application::RouteMIDI received data byte %02x\n", first);
        return;
    }

    const bool    is_sysex = first == 0xF0;
    const uint8_t channel  = first & 0x0F;

    if (is_sysex)
    {
        BroadcastMIDI(bytes);
    }
    else
    {
        SendMIDI(channel % m_instances.Count(), bytes);
    }
}

void Application::SendSerial(const size_t instance_id, const uint8_t sbyte)
{
    m_instances[instance_id].GetEmulator().PostSerial(sbyte);
}

void Application::BroadcastSerial(const uint8_t sbyte)
{
    for (size_t i = 0; i < m_instances.Count(); ++i)
    {
        SendSerial(i, sbyte);
    }
}

void Application::RouteSerial(const uint8_t sbyte)
{
    if (sbyte == 0xF0)
        current_instance = 16; //Broadcast
    else if (sbyte >= 0x80 && sbyte <= 0xDF)
    {
        current_instance = (sbyte  & 0x0F) % m_instances.Count();
    }

    if (current_instance == 16)
    {
        BroadcastSerial(sbyte);
    }
    else
    {
        SendSerial(current_instance, sbyte);
    }
}

void Application::BroadcastRC(const uint8_t rbyte)
{
    for (Instance &inst:m_instances)
    {
        inst.GetEmulator().PostRC(rbyte);
    }
}

void Application::RouteRC(uint8_t rbyte)
{
    BroadcastRC(rbyte);
}

bool Application::OpenSDLAudio(const AudioOutputParameters& params, const char* device_name)
{
    if (!Out_SDL_Create(device_name, params))
    {
        common::Printf("Failed to create SDL audio output\n");
        return false;
    }

    for (size_t id = 0; id < m_instances.Count(); ++id)
    {
        Instance& inst = m_instances[id];
        inst.OpenSDLAudio();
    }

    if (!Out_SDL_Start())
    {
        common::Printf("Failed to start SDL audio output\n");
        return false;
    }

    return true;
}

#if NUKED_ENABLE_ASIO
bool Application::OpenASIOAudio(const ASIO_OutputParameters& params, const char* name)
{
    if (!Out_ASIO_Create(name, params))
    {
        common::Printf("Failed to create ASIO output\n");
        return false;
    }

    for (Instance& inst : m_instances)
    {
        inst.OpenASIOAudio();
    }

    if (!Out_ASIO_Start())
    {
        common::Printf("Failed to create ASIO output\n");
        return false;
    }

    return true;
}
#endif

void FixupParameters(CliParameters& params)
{
    if (!std::has_single_bit(params.buffer_size))
    {
        const uint32_t next_low  = common::bit_floor(params.buffer_size);
        const uint32_t next_high = common::bit_ceil(params.buffer_size);
        const uint32_t closer =
            (uint32_t)PickCloser<int64_t>((int64_t)params.buffer_size, (int64_t)next_low, (int64_t)next_high);
        common::Printf("WARNING: Audio buffer size must be a power-of-two; got %d\n", params.buffer_size);
        common::Printf("         The next valid values are %d and %d\n", next_low, next_high);
        common::Printf("         Continuing with the closer value %d\n", closer);
        params.buffer_size = closer;
    }
}

bool Application::OpenAudio(const CliParameters& params)
{
    AudioOutput      output;
    PickOutputResult output_result = PickOutputDevice(params.audio_device, output);

    m_audio_output = output;

    AudioOutputParameters out_params;
    out_params.frequency = PCM_GetOutputFrequency(m_instances[0].GetEmulator().GetPCM());
    switch (output.kind)
    {
    case AudioOutputKind::SDL:
        // explicitly do nothing
        break;
    case AudioOutputKind::ASIO:
        if (params.asio_sample_rate.has_value())
        {
            out_params.frequency = params.asio_sample_rate.value();
        }
        break;
    }
    out_params.buffer_size = params.buffer_size;
    out_params.format      = params.output_format;

    switch (output_result)
    {
    case PickOutputResult::WantMatchedName:
        if (output.kind == AudioOutputKind::SDL)
        {
            return OpenSDLAudio(out_params, output.name.c_str());
        }
        else if (output.kind == AudioOutputKind::ASIO)
        {
#if NUKED_ENABLE_ASIO
            ASIO_OutputParameters asio_params;
            asio_params.common        = out_params;
            asio_params.left_channel  = params.asio_left_channel;
            asio_params.right_channel = params.asio_right_channel;
            return OpenASIOAudio(asio_params, output.name.c_str());
#else
            common::Printf("Attempted to open ASIO output without ASIO support\n");
#endif
        }
        return false;
    case PickOutputResult::WantDefaultDevice:
        return OpenSDLAudio(out_params, nullptr);
    case PickOutputResult::NoOutputDevices:
        // in some cases this may still work
        common::Printf("No output devices found; attempting to open default device\n");
        return OpenSDLAudio(out_params, nullptr);
    case PickOutputResult::NoMatchingName:
        // in some cases SDL cannot list all audio devices so we should still try
        common::Printf("No output device named '%s'; attempting to open it anyways...\n", params.audio_device.c_str());
        return OpenSDLAudio(out_params, output.name.c_str());
    }

    return false;
}

void Application::SetMIDIOutCallback()
{
    for (Instance& inst:m_instances)
    {
        inst.GetEmulator().SetMidiOutCallback(MIDI_MIDIOutCallBack);
    }
}

void Application::SetSerialOutCallback()
{
    for (Instance& inst:m_instances)
    {
        inst.GetEmulator().SetSerialPostCallback(SERIAL_PostUART);
    }
}

bool Application::HandleGlobalEvent(const SDL_Event& ev)
{
    switch (ev.type)
    {
    case SDL_QUIT:
        m_running = false;
        return true;
    default:
        return false;
    }
}

void Application::RunEventLoop()
{
    while (m_running)
    {
#if NUKED_ENABLE_ASIO
        if (Out_ASIO_IsResetRequested())
        {
            Out_ASIO_Reset();
        }
#endif

        for (Instance& inst : m_instances)
        {
            if (inst.IsQuitRequested())
            {
                m_running = false;
            }
            inst.Render();
        }

        SDL_Event ev;
        while (SDL_PollEvent(&ev))
        {
            if (HandleGlobalEvent(ev))
            {
                // not directed at any particular window; don't let LCDs
                // handle this one
                continue;
            }

            for (Instance& inst : m_instances)
            {
                inst.HandleEvent(ev);
            }
        }

        if (is_serial)
        {
            SERIAL_Update();
        }

        SDL_Delay(15);
    }
}

void Application::Run()
{
    m_running = true;

    for (Instance& inst : m_instances)
    {
        inst.StartThread();
    }

    RunEventLoop();

    for (Instance& inst : m_instances)
    {
        inst.JoinThread();
    }
}

bool Application::CreateInstance(const CliParameters& app_params)
{
    Instance* inst = nullptr;

    const size_t instance_id = m_instances.Count();

    if (!AllocateInstance(&inst))
    {
        common::Printf("ERROR: Failed to allocate instance.\n");
        return false;
    }

    InstanceParameters inst_params{
        .instance_id         = instance_id,
        .output_format       = app_params.output_format,
        .buffer_size         = app_params.buffer_size,
        .buffer_count        = app_params.buffer_count,
        .gain                = app_params.gain,
        .enable_lcd          = !app_params.no_lcd,
        .enable_oversampling = !app_params.disable_oversampling,
        .nvram_filename      = app_params.nvram_filename,
        .romset_info         = &m_romset_info,
        .romset              = m_romset,
        .serial_type         = app_params.serial_type,
    };

    if (!inst->Initialize(inst_params))
    {
        common::Printf("ERROR: Failed to initialize instance #%02zu.\n", instance_id);
        return false;
    }

    return true;
}

void Application::PrintControls(const Romset romset)
{
    constexpr const char* SC_55_CONTROLS = R"(
Roland SC-55 mk1/mk2 Controls:

Q -> POWER
W -> INST ALL
E -> INST MUTE
R -> PART L
T -> PART R
Y -> INST L
U -> INST R
I -> KEY SHIFT L
O -> KEY SHIFT R
P -> LEVEL L
LEFT BRACKET -> LEVEL R
A -> MIDI CH L
S -> MIDI CH R
D -> PAN L
F -> PAN R
G -> REVERB L
H -> REVERB R
J -> CHORUS L
K -> CHORUS R
LEFT -> PART L
RIGHT -> PART R
SCROLL WHEEL UP (On Volume Knob) -> INCREASE VOLUME
SCROLL WHEEL DOWN (On Volume Knob) -> DECREASE VOLUME
LMB + MOUSE MOVEMENT (O Volume Knob) -> ADJUST VOLUME

)";

    constexpr const char* JV880_CONTROLS = R"(
Roland JV880 Controls:

P -> PREVIEW
LEFT -> CURSOR L
RIGHT -> CURSOR R
TAB -> DATA
Q -> TONE_SELECT
A -> PATCH_PERFORM
W -> EDIT
E -> SYSTEM
R -> RHYTHM
T -> UTILITY
S -> MUTE
D -> MONITOR
F -> COMPARE
G -> ENTER
COMMA -> ENCODER L
PERIOD -> ENCODER R
RMB (On Volume Knob) -> PATCH_PERFORM
SCROLL WHEEL UP (On Volume Knob) -> INCREASE VOLUME
SCROLL WHEEL DOWN (On Volume Knob) -> DECREASE VOLUME
LMB + MOUSE MOVEMENT (O Volume Knob) -> ADJUST VOLUME
SCROLL WHEEL UP (On Encoder Knob) -> ENCODER R
SCROLL WHEEL DOWN (On Encoder Knob) -> ENCODER L
LMB + MOUSE MOVEMENT (O Encoder Knob) -> ROTATE ENCODER

)";

    if (romset == Romset::JV880)
    {
        common::Printf(JV880_CONTROLS);
    }
    else if (romset == Romset::MK1 || romset == Romset::MK2)
    {
        common::Printf(SC_55_CONTROLS);
    }
}
