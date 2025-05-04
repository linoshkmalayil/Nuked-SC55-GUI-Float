# Usage Instructions

**NOTE:** These instructions are made by reference from the original [nukedyt/Nuked-SC55](https://github.com/nukeykt/Nuked-SC55/blob/master/README.md) and [jcmoyer/Nuked-SC55](https://raw.githubusercontent.com/jcmoyer/Nuked-SC55/refs/heads/master/doc) documentation.

## Supported Models

- SC-55mk2/SC-155mk2 (v1.01 firmware is confirmed to work)
- SC-55mk1 (v1.0/v1.21/v2.0 firmwares are confirmed to work)
- CM-300/SCC-1 (v1.10/v1.20 firmwares are confirmed to work)
- SCC-1A
- SC-55st (v1.01)
- JV-880 (v1.0.0/v1.0.1)
- SCB-55/RLP-3194
- RLP-3237
- SC-155

## ROM Handling

Put ROM Images into the same folder as the emulator. Files should be names as such:

```
SC-55mk2/SC-155mk2 (v1.01):
R15199858 (H8/532 mcu) -> rom1.bin
R00233567 (H8/532 extra code) -> rom2.bin
R15199880 (M37450M2 mcu) -> rom_sm.bin
R15209359 (WAVE 16M) -> waverom1.bin
R15279813 (WAVE 8M) -> waverom2.bin

SC-55st (v1.01):
R15199858 (H8/532 mcu) -> rom1.bin
R00561413 (H8/532 extra code) -> rom2_st.bin
R15199880 (M37450M2 mcu) -> rom_sm.bin
R15209359 (WAVE 16M) -> waverom1.bin
R15279813 (WAVE 8M) -> waverom2.bin

SC-55 (v1.00):
R15199748 (H8/532 mcu) -> sc55_rom1.bin
R1544925800 (H8/532 extra code) -> sc55_rom2.bin
R15209276 (WAVE A) -> sc55_waverom1.bin
R15209277 (WAVE B) -> sc55_waverom2.bin
R15209281 (WAVE C) -> sc55_waverom3.bin

SC-55 (v1.21):
R15199778 (H8/532 mcu) -> sc55_rom1.bin
R15209363 (H8/532 extra code) -> sc55_rom2.bin
R15209276 (WAVE A) -> sc55_waverom1.bin
R15209277 (WAVE B) -> sc55_waverom2.bin
R15209281 (WAVE C) -> sc55_waverom3.bin

SC-55 (v2.0):
R15199799 (H8/532 mcu) -> sc55_rom1.bin
R15209387 (H8/532 extra code) -> sc55_rom2.bin
R15209276 (WAVE A) -> sc55_waverom1.bin
R15209277 (WAVE B) -> sc55_waverom2.bin
R15209281 (WAVE C) -> sc55_waverom3.bin

CM-300/SCC-1 (v1.10):
R15199774 (H8/532 mcu) -> cm300_rom1.bin
R15279809 (H8/532 extra code) -> cm300_rom2.bin
R15279806 (WAVE A) -> cm300_waverom1.bin
R15279807 (WAVE B) -> cm300_waverom2.bin
R15279808 (WAVE C) -> cm300_waverom3.bin

CM-300/SCC-1 (v1.20):
R15199774 (H8/532 mcu) -> cm300_rom1.bin
R15279812 (H8/532 extra code) -> cm300_rom2.bin
R15279806 (WAVE A) -> cm300_waverom1.bin
R15279807 (WAVE B) -> cm300_waverom2.bin
R15279808 (WAVE C) -> cm300_waverom3.bin

SCC-1A:
R00128523 (H8/532 mcu) -> cm300_rom1.bin
R00128567 (H8/532 extra code) -> cm300_rom2.bin
R15279806 (WAVE A) -> cm300_waverom1.bin
R15279807 (WAVE B) -> cm300_waverom2.bin
R15279808 (WAVE C) -> cm300_waverom3.bin

JV-880 (v1.0.0):
R15199810 (H8/532 mcu) -> jv880_rom1.bin
R15209386 (H8/532 extra code) -> jv880_rom2.bin
R15209312 (WAVE A) -> jv880_waverom1.bin
R15209313 (WAVE B) -> jv880_waverom2.bin
PCM Cards -> jv880_waverom_pcmcard.bin (optional)
Expansion PCBs -> jv880_waverom_expansion.bin (optional)

SCB-55/RLP-3194:
R15199827 (H8/532 mcu) -> scb55_rom1.bin
R15279828 (H8/532 extra code) -> scb55_rom2.bin
R15209359 (WAVE 16M) -> scb55_waverom1.bin
R15279813 (WAVE 8M) -> scb55_waverom2.bin

RLP-3237:
R15199827 (H8/532 mcu) -> rlp3237_rom1.bin
R15209486 (H8/532 extra code) -> rlp3237_rom2.bin
R15279824 (WAVE 16M) -> rlp3237_waverom1.bin

SC-155 (rev 1):
R15199799 (H8/532 mcu) -> sc155_rom1.bin
R15209361 (H8/532 extra code) -> sc155_rom2.bin
R15209276 (WAVE A) -> sc155_waverom1.bin
R15209277 (WAVE B) -> sc155_waverom2.bin
R15209281 (WAVE C) -> sc155_waverom3.bin

SC-155 (rev 2):
R15199799 (H8/532 mcu) -> sc155_rom1.bin
R15209400 (H8/532 extra code) -> sc155_rom2.bin
R15209276 (WAVE A) -> sc155_waverom1.bin
R15209277 (WAVE B) -> sc155_waverom2.bin
R15209281 (WAVE C) -> sc155_waverom3.bin

```

## Emulator Running

- SC-55mk2/SC-55mk1 buttons are mapped as such (currently hardcoded):

```
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
```

- JV-880 buttons are mapped as such (currently hardcoded):

```
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
```

These are the options to use the emulator frontend:

```
General options:
  -?, -h, --help                                Display emulator argument information.
  -v, --version                                 Display version information.

Audio options:
  -pi, --portin      <device_name_or_number>    Set MIDI input port.
  -po, --portout     <device_name_or_number>    Set MIDI output port.
  -a, --audio-device <device_name_or_number>    Set output audio device.
  -b, --buffer-size  <size>[:count]             Set buffer size, number of buffers.
  -f, --format       s16|s32|f32                Set output format.
  --disable-oversampling                        Halves output frequency.

Emulator options:
  -r, --reset     gs|gm                         Reset system in GS or GM mode.
  -n, --instances <count>                       Set number of emulator instances. (MIDI IO Only)
  --no-lcd                                      Run without LCDs.
  --nvram <filename>                            Saves and loads NVRAM to/from disk. JV-880 only.

ROM management options:
  -d, --rom-directory <dir>                     Sets the directory to load roms from.
  --mk2                                         Use SC-55mk2 ROM set.
  --st                                          Use SC-55st ROM set.
  --mk1                                         Use SC-55mk1 ROM set.
  --cm300                                       Use CM-300/SCC-1 ROM set.
  --jv880                                       Use JV-880 ROM set.
  --scb55                                       Use SCB-55 ROM set.
  --rlp3237                                     Use RLP-3237 ROM set.

Available only ASIO Builds:
  --asio-sample-rate <freq>                     Request frequency from the ASIO driver.

```

These are the options to use the emulator renderer:

```
General options:
  -? -h, --help                Display emulator argument information.
  -v, --version                Display version information.
  -o <filename>                Render WAVE file to filename.
  --stdout                     Render raw sample data to stdout. No header

Audio options:
  -f, --format s16|s32|f32     Set output format.
  --disable-oversampling       Halves output frequency.
  --end cut|release            Choose how the end of the track is handled:
        cut (default)              Stop rendering at the last MIDI event
        release                    Continue to render audio after the last MIDI event until silence

Emulator options:
  -r, --reset     gs|gm        Send GS or GM reset before rendering.
  -n, --instances <count>      Number of emulators to use (increases effective polyphony, but
                               takes longer to render)
  --nvram <filename>           Saves and loads NVRAM to/from disk. JV-880 only.

ROM management options:
  -d, --rom-directory <dir>    Sets the directory to load roms from. Romset will be autodetected when
                               not also passing --romset.
  --romset <name>              Sets the romset to load. Same as frontend's romsets without '--'
```

### Regarding Serial IO

With version 0.5.3, the emulator support Serial IO for SC-55mkII and SC-55st. Currently Serial IO doesn't
support multiple instances at the moment. (Multi-Instance support is being worked on)
So when attempting to launch multi-instance with serial mode will cause it not to launch with the 
error `error: Multiple Instances not supported with Serial Mode`.

Regarding Setting up Virtual serial IO. Please read these instructions for [Linux](VIRTUAL_SERIAL_SETUP.md#linux) and [Windows](VIRTUAL_SERIAL_SETUP.md#windows).

### Regarding SRAM and NVRAM

With version 0.5.4, the emulator supports SRAM saving for SC-55mk1, SC-55mk2, JV880, SC-155, SC-155mk2
and NVRAM read and write support for JV880.

JV880 needs NVRAM file passed via `--nvram`.

Also the file where SRAM and NVRAM will be saved will have a suffix incidicating the corresponding
emulator instance. Do not that the implementation of JV880 NVRAM has a [bug](https://github.com/jcmoyer/Nuked-SC55/issues/36#issuecomment-2781603485)

### Regarding Buffer Size

Buffer size ontrols the amount of audio to produce or output at a time. Lower 
values reduce latency; higher values reduce playback glitches. Optimal settings 
are hardware dependent, so experiment with this option. If no value is provided, 
it will default to `512:16` roughly mirroring upstream's intent.

`size` is the number of audio frames that the emulator will produce and the
output will consume in a single chunk. It must be a power of 2.

`count` is the number of `size` pages that can be queued up. It can be any
value greater than zero, but the best value is likely in the range `2..32`.

Having queued chunks is helpful if the emulator produces audio fast enough most
of the time but sometimes falls behind. This can happen for music containing
particularly busy sections. The queued chunks give the emulator some headroom
so that even if it slows down temporarily, the output has enough audio to work
with until the emulator catches back up.

#### Example

Consider `-b 512:16`: The SC-55mk2 outputs at 66207hz. The emulator would
produce chunks of 512/66207 = 7.7ms of audio at a time. It would be allowed to
queue up to 16 of those chunks, meaning that you could have up to 512\*16/66207
= 123ms of latency.

#### Divergence from upstream

The behavior of this option was changed because the way upstream uses it is
buggy and unintuitive.

Upstream defaults to `-b 512:32`. For this setting, it creates a ringbuffer of
512\*32 *samples*, but the buffer only holds 512\*16 *frames* (each frame
consists of two samples, one for each stereo channel). The emulator will place
one frame at a time into this buffer. Audio will be drained from the buffer for
playback 512/4 = 128 frames at a time. The /4 is arbitrary and cannot be set by
the user.

In theory, the buffer should be able to contain ~123ms of audio, but because
upstream reads from the buffer unconditionally, the read position can overtake
the write position causing dropped frames. This also causes a variable amount
of latency, because at the start of the program the read position will start
advancing - it then takes 123ms to get back to the start of the buffer where
audio has started being written.

This fork since it depends on jcmoyer's fork also corrects the ringbuffer 
behavior - instead of advancing the read position unconditionally, it only
advances when there is audio to play and produces silence otherwise. The buffer
size you provide to this option is in *frames* instead of samples, and the 
numbers you provide are used *without modification* by both the emulator and 
output.

### Regarding ASIO Sample Rate

When using `--asio-sample-rate`, if a sampling frequency is expected.
If n value is provided, the emulator will request the native frequency from
the the selected romset.

The emulator natively produces audio at 64000hz or 66207hz depending on the
romset. Some ASIO drivers cannot support these frequencies so resampling to
`<rate>` is necessary.
