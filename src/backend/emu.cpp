/*
 * Copyright (C) 2021, 2024 nukeykt
 *
 *  Redistribution and use of this code or any derivative works are permitted
 *  provided that the following conditions are met:
 *
 *   - Redistributions may not be sold, nor may they be used in a commercial
 *     product or activity.
 *
 *   - Redistributions that are modified from the original source must include the
 *     complete source code, including the source code for all components used by a
 *     binary built from the modified sources. However, as a special exception, the
 *     source code distributed need not include anything that is normally distributed
 *     (in either source or binary form) with the major components (compiler, kernel,
 *     and so on) of the operating system on which the executable runs, unless that
 *     component itself accompanies the executable.
 *
 *   - Redistributions must reproduce the above copyright notice, this list of
 *     conditions and the following disclaimer in the documentation and/or other
 *     materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */
#include "emu.h"
#include "mcu.h"
#include "submcu.h"
#include "mcu_timer.h"
#include "lcd.h"
#include "pcm.h"
#include "submcu.h"
#include <fstream>
#include <string>
#include <span>
#include <vector>
#include "common/bit_util.h"


Emulator::~Emulator()
{
    WriteSRAM();
    WriteNVRAM();
}

bool Emulator::Init(const EMU_Options& options)
{
    m_options = options;

    try
    {
        m_mcu   = std::make_unique<mcu_t>();
        m_sm    = std::make_unique<submcu_t>();
        m_timer = std::make_unique<mcu_timer_t>();
        m_lcd   = std::make_unique<lcd_t>();
        m_pcm   = std::make_unique<pcm_t>();
    }
    catch (const std::bad_alloc&)
    {
        m_mcu.reset();
        m_sm.reset();
        m_timer.reset();
        m_lcd.reset();
        m_pcm.reset();
        return false;
    }

    MCU_Init(*m_mcu, *m_sm, *m_pcm, *m_timer, *m_lcd, options.serial_type);
    SM_Init(*m_sm, *m_mcu);
    PCM_Init(*m_pcm, *m_mcu);
    TIMER_Init(*m_timer, *m_mcu);
    LCD_Init(*m_lcd, *m_mcu);
    m_lcd->backend = options.lcd_backend;

    return true;
}

void Emulator::Reset()
{
    MCU_Reset(*m_mcu);
    SM_Reset(*m_sm);
}


bool Emulator::StartLCD()
{
    return LCD_Start(*m_lcd);
}

void Emulator::StopLCD()
{
    LCD_Stop(*m_lcd);
}

void Emulator::SetSampleCallback(mcu_sample_callback callback, void* userdata)
{
    m_mcu->callback_userdata = userdata;
    m_mcu->sample_callback   = callback;
}

void Emulator::SetMidiOutCallback(mcu_midiout_callback callback)
{
    m_mcu->midiout_callback = callback;
}

void Emulator::SetSerialPostCallback(sm_serial_post_callback callback)
{
    m_sm->serial_post_callback = callback;
}

constexpr int ROM_SET_N_FILES = 7;

const char* roms[(size_t)ROMSET_COUNT][ROM_SET_N_FILES] =
{
    {
        "rom1.bin",
        "rom2.bin",
        "waverom1.bin",
        "waverom2.bin",
        "rom_sm.bin",
        "",
        "memory.bin"
    },

    {
        "rom1.bin",
        "rom2_st.bin",
        "waverom1.bin",
        "waverom2.bin",
        "rom_sm.bin",
        "",
        "",
    },

    {
        "sc55_rom1.bin",
        "sc55_rom2.bin",
        "sc55_waverom1.bin",
        "sc55_waverom2.bin",
        "sc55_waverom3.bin",
        "",
        "sc55_memory.bin",
    },

    {
        "cm300_rom1.bin",
        "cm300_rom2.bin",
        "cm300_waverom1.bin",
        "cm300_waverom2.bin",
        "cm300_waverom3.bin",
        "",
        "",
    },

    {
        "jv880_rom1.bin",
        "jv880_rom2.bin",
        "jv880_waverom1.bin",
        "jv880_waverom2.bin",
        "jv880_waverom_expansion.bin",
        "jv880_waverom_pcmcard.bin",
        "jv880_memory.bin"
    },

    {
        "scb55_rom1.bin",
        "scb55_rom2.bin",
        "scb55_waverom1.bin",
        "scb55_waverom2.bin",
        "",
        "",
        "",
    },

    {
        "rlp3237_rom1.bin",
        "rlp3237_rom2.bin",
        "rlp3237_waverom1.bin",
        "",
        "",
        "",
        "",
    },

    {
        "sc155_rom1.bin",
        "sc155_rom2.bin",
        "sc155_waverom1.bin",
        "sc155_waverom2.bin",
        "sc155_waverom3.bin",
        "",
        "sc155_memory.bin"
    },

    {
        "rom1.bin",
        "rom2.bin",
        "waverom1.bin",
        "waverom2.bin",
        "rom_sm.bin",
        "",
        "memory.bin",
    },
};

void unscramble(uint8_t *src, uint8_t *dst, int len)
{
    for (int i = 0; i < len; i++)
    {
        int address = i & ~0xfffff;
        static const int aa[] = {
            2, 0, 3, 4, 1, 9, 13, 10, 18, 17, 6, 15, 11, 16, 8, 5, 12, 7, 14, 19
        };
        for (int j = 0; j < 20; j++)
        {
            if (i & (1 << j))
                address |= 1<<aa[j];
        }
        uint8_t srcdata = src[address];
        uint8_t data = 0;
        static const int dd[] = {
            2, 0, 4, 5, 7, 6, 3, 1
        };
        for (int j = 0; j < 8; j++)
        {
            if (srcdata & (1 << dd[j]))
                data |= 1<<j;
        }
        dst[i] = data;
    }
}

Romset EMU_DetectRomset(const std::filesystem::path& base_path)
{
    for (size_t i = 0; i < (size_t)ROMSET_COUNT; i++)
    {
        bool good = true;
        for (size_t j = 0; j < 5; j++)
        {
            if (roms[i][j][0] == '\0')
                continue;
            if (!std::filesystem::exists(base_path / roms[i][j]))
            {
                good = false;
                break;
            }
        }
        if (good)
        {
            return (Romset)i;
        }
    }
    return Romset::MK2;
}

bool EMU_ReadStreamExact(std::ifstream& s, void* into, std::streamsize byte_count)
{
    if (s.read((char*)into, byte_count))
    {
        return s.gcount() == byte_count;
    }
    return false;
}

bool EMU_ReadStreamExact(std::ifstream& s, std::span<uint8_t> into, std::streamsize byte_count)
{
    return EMU_ReadStreamExact(s, into.data(), byte_count);
}

bool EMU_WriteStreamExact(std::ofstream& s, void* offrom, std::streamsize byte_count)
{
    auto start = s.tellp();
    if (s.write((char*)offrom, byte_count))
    {
        auto end = s.tellp();
        return (end - start) == byte_count;
    }
    return false;
}

bool EMU_WriteStreamExact(std::ofstream& s, std::span<uint8_t> offrom, std::streamsize byte_count)
{
    return EMU_WriteStreamExact(s, offrom.data(), byte_count);
}

std::streamsize EMU_ReadStreamUpTo(std::ifstream& s, void* into, std::streamsize byte_count)
{
    s.read((char*)into, byte_count);
    return s.gcount();
}

std::span<uint8_t> Emulator::MapBuffer(RomLocation location)
{
    switch (location)
    {
    case RomLocation::ROM1:
        return GetMCU().rom1;
    case RomLocation::ROM2:
        return GetMCU().rom2;
    case RomLocation::WAVEROM1:
        return GetPCM().waverom1;
    case RomLocation::WAVEROM2:
        return GetPCM().waverom2;
    case RomLocation::WAVEROM3:
        return GetPCM().waverom3;
    case RomLocation::WAVEROM_CARD:
        return GetPCM().waverom_card;
    case RomLocation::WAVEROM_EXP:
        return GetPCM().waverom_exp;
    case RomLocation::SMROM:
        return m_sm->rom;
    }
    fprintf(stderr, "FATAL: MapBuffer called with invalid location %d\n", (int)location);
    std::abort();
}

bool Emulator::LoadRom(RomLocation location, std::span<const uint8_t> source)
{
    auto buffer = MapBuffer(location);

    if (buffer.size() < source.size())
    {
        fprintf(stderr,
                "FATAL: rom for %s is too large; max size is %d bytes\n",
                ToCString(location),
                (int)buffer.size());
        return false;
    }

    if (location == RomLocation::ROM2)
    {
        if (!fe::has_single_bit(source.size()))
        {
            fprintf(stderr, "FATAL: %s requires a power-of-2 size\n", ToCString(location));
            return false;
        }
        GetMCU().rom2_mask = (int)source.size() - 1;
    }

    std::copy(source.begin(), source.end(), buffer.begin());

    return true;
}

bool Emulator::LoadRoms(Romset romset, const AllRomsetInfo& all_info, RomLocationSet* loaded, MK1version revision)
{
    if (loaded)
    {
        loaded->fill(false);
    }

    MCU_SetRomset(GetMCU(), romset);

    const RomsetInfo& info = all_info.romsets[(size_t)romset];

    for (size_t i = 0; i < ROMLOCATION_COUNT; ++i)
    {
        const RomLocation location = (RomLocation)i;

        // rom_data should be populated at this point
        // if it isn't, then there isn't a rom for this location
        if (info.rom_data[i].empty())
        {
            continue;
        }

        if (!LoadRom(location, info.rom_data[i]))
        {
            return false;
        }

        if (loaded)
        {
            (*loaded)[i] = true;
        }
    }

    if (m_mcu->is_mk1)
    {
        switch (MCU_DetectMKIRomVersion(*m_mcu, revision))
        {
            case 100: fprintf(stderr, "Detected MK1 ROM Version 1.00\n");
                    break;
            case 110: fprintf(stderr, "Detected MK1 ROM Version 1.10\n");
                    break;
            case 120: fprintf(stderr, "Detected MK1 ROM Version 1.20\n");
                    break;
            case 121: fprintf(stderr, "Detected MK1 ROM Version 1.21\n");
                    break;
            case 200: fprintf(stderr, "Detected MK1 ROM Version 2.00\n");
                    break;
            default : fprintf(stderr, "Rom Version Unknown, defaulting to 1.21\n");
                    break;
        }
    }

    // Initialize empty RAMs
    std::fill(m_mcu->ram, m_mcu->ram + RAM_SIZE, 0);
    std::fill(m_mcu->sram, m_mcu->sram + SRAM_SIZE, 0);

    ReadSRAM();

    if (m_mcu->is_jv880)
    {
        // Initialize NVRAM and set contrast to 04
        std::fill(m_mcu->nvram, m_mcu->nvram + NVRAM_SIZE, 0);
        m_mcu->nvram[0x10] = 0x04;

        ReadNVRAM();
    }

    MCU_PatchROM(*m_mcu);

    return true;
}

void Emulator::PostMIDI(uint8_t byte)
{
    MCU_PostUART(*m_mcu, byte);
}

void Emulator::PostMIDI(std::span<const uint8_t> data)
{
    for (uint8_t byte : data)
    {
        PostMIDI(byte);
    }
}

void Emulator::PostSerial(uint8_t byte)
{
    SM_PostSerial(*m_sm, byte);
}

void Emulator::PostSerial(std::span<const uint8_t> data)
{
    for(auto byte : data)
    {
        PostSerial(byte);
    }
}

void Emulator::PostRC(uint8_t byte)
{
    MCU_RemoteControlTrigger(*m_mcu, byte);
}

constexpr uint8_t GM_RESET_SEQ[] = { 0xF0, 0x7E, 0x7F, 0x09, 0x01, 0xF7 };
constexpr uint8_t GS_RESET_SEQ[] = { 0xF0, 0x41, 0x10, 0x42, 0x12, 0x40, 0x00, 0x7F, 0x00, 0x41, 0xF7 };

void Emulator::PostSystemReset(EMU_SystemReset reset)
{
    if (m_mcu->revision == MK1version::REVISION_SC55_100 || m_mcu->revision == MK1version::REVISION_SC55_110)
        fprintf(stderr, "WARNING: GM Reset not supported by SC-55mk1 verion 1.00 & 1.10, will be interpreted as GS Reset\n");

    switch (reset)
    {
        case EMU_SystemReset::NONE:
            // explicitly do nothing
            break;
        case EMU_SystemReset::GS_RESET:
            PostMIDI(GS_RESET_SEQ);
            break;
        case EMU_SystemReset::GM_RESET:
            PostMIDI(GM_RESET_SEQ);
            break;
    }
}
void Emulator::PostSerialSystemReset(EMU_SystemReset reset)
{
    if (m_mcu->revision == MK1version::REVISION_SC55_100 || m_mcu->revision == MK1version::REVISION_SC55_110)
        fprintf(stderr, "WARNING: GM Reset not supported by SC-55mk1 verion 1.00 & 1.10, will be interpreted as GS Reset\n");

    switch (reset)
    {
        case EMU_SystemReset::NONE:
            // explicitly do nothing
            break;
        case EMU_SystemReset::GS_RESET:
            PostSerial(GS_RESET_SEQ);
            break;
        case EMU_SystemReset::GM_RESET:
            PostSerial(GM_RESET_SEQ);
            break;
    }
}

void Emulator::Step()
{
    MCU_Step(*m_mcu);
}

void Emulator::ReadSRAM()
{
    // append instance number so that multiple instances don't clobber each other's sram
    std::filesystem::path sram_path = m_options.rom_directory / roms[(size_t)m_mcu->romset][6];
    sram_path                      += std::to_string(m_options.instance_id);
    
    std::ifstream s_rf = std::ifstream(sram_path.generic_string().c_str(), std::ios::binary);
    if(!s_rf || !EMU_ReadStreamExact(s_rf, m_mcu->sram, SRAM_SIZE))
    {
        fprintf(stderr, "WARNING: Failed reading SRAM: %s\n", sram_path.generic_string().c_str());
        is_sram_loaded = false;

        return;
    }
    is_sram_loaded = true;
}

void Emulator::WriteSRAM()
{
    // emulator was constructed, but never init
    if (!m_mcu)
    {
        return;
    }

    // append instance number so that multiple instances don't clobber each other's sram
    std::filesystem::path sram_path = m_options.rom_directory / roms[(size_t)m_mcu->romset][6];
    sram_path                      += std::to_string(m_options.instance_id);
    
    std::ofstream s_wf = std::ofstream(sram_path.generic_string().c_str(), std::ios::binary);
    if(!s_wf || !EMU_WriteStreamExact(s_wf, m_mcu->sram, SRAM_SIZE))
    {
        fprintf(stderr, "WARNING: Failed writing SRAM: %s\n", sram_path.generic_string().c_str());
    }
}

void Emulator::ReadNVRAM()
{
    if (!m_options.nvram_filename.empty() && m_mcu->is_jv880)
    {
        // append instance number so that multiple instances don't clobber each other's nvram
        std::filesystem::path nvram_file = m_options.nvram_filename;
        nvram_file                      += std::to_string(m_options.instance_id);

        std::ifstream file(nvram_file, std::ios::binary);
        if(!file || !EMU_ReadStreamExact(file, m_mcu->nvram, NVRAM_SIZE))
        {
            fprintf(stderr, "WARNING: Failed reading NVRAM: %s\n", nvram_file.generic_string().c_str());
            is_nvram_loaded = false;

            return;
        }
        is_nvram_loaded = true;
    }
}

void Emulator::WriteNVRAM()
{
    // emulator was constructed, but never init
    if (!m_mcu)
    {
        return;
    }

    if (!m_options.nvram_filename.empty() && m_mcu->is_jv880)
    {
        // append instance number so that multiple instances don't clobber each other's nvram
        std::filesystem::path nvram_file = m_options.nvram_filename;
        nvram_file                      += std::to_string(m_options.instance_id);

        std::ofstream file(nvram_file, std::ios::binary);
        if(!file || !EMU_WriteStreamExact(file, m_mcu->nvram, NVRAM_SIZE))
        {
            fprintf(stderr, "WARNING: Failed writing NVRAM: %s\n", nvram_file.generic_string().c_str());
        }
    }
}
