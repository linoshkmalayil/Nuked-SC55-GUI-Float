# IMPORTANT NOTE

This fork is dependent on the changes on the fork [jcmoyer/Nuked-SC55](https://github.com/jcmoyer/Nuked-SC55)
For **FULL CHANGELOG** from please check [jcmoyer/Nuked-SC55/CHANGELOG.md](https://github.com/jcmoyer/Nuked-SC55/blob/master/CHANGELOG.md)

# Version 0.6.3 (2026-01-23)

Jist of changes are follows
- Added JV880 GUI and Background.
- Update JV880 LCD Contrast
- Added Remote Control support for Roland SC55, SC55mkII, SC155 and SC155mkII models.
- Added Python based Remote Control client.
- Updated RAM, SRAM and NVRAM initialization.
- Cleaned up code.
- Updated some functions for MacOS compatability.

# Version 0.6.2 (2026-01-02)

- Synced to changes from [jcmoyer/Nuked-SC55-v0.6.1](https://github.com/jcmoyer/Nuked-SC55/tree/0.6.1)

Jist of changes are follows
- Added ROM Hashing and detection of ROMs using Hash.
- Added commandline gain option.
- Basic EMIDI looping support in renderer.
- Added the ability to route emulator audio to specific ASIO channels.
- Fixed no LCD segmentation fault.

# Version 0.5.6 (2025-05-06)

Jist of changes are follows
- Fixed multi-instance Serial Mode. (Hence related changes from 5.5.1 are reverted)
- Reworked Resetting when using Serial Mode.
- Fixed Audio Clipping Issue using [jcmoyer's solution](https://github.com/nukeykt/Nuked-SC55/issues/101#issuecomment-2433588784).
- Reworked Volume control code.
- Updated WIN32 MIDI Port selection routine which fixes following bugs:
  - Incorrectly using the Last available MIDI Port name when using the first available MIDI Port when no port parameter is passed.
  - Fixes issue where port numbers couldn't be passed as arguments.

# Version 0.5.5.1 (2025-05-05)
StopGap release

Jist of changes are follows
- Added Stopgap fix to prevent multi-instance launch when using Serial Mode with mkII and st. (Will be reverted once https://github.com/linoshkmalayil/Nuked-SC55-GUI-Float/issues/32 is solved)
- Added resetting when using Serial Mode.
- Fixed renderer desync when using multiple instances and one of the instances received midi data starting later than tick 0.

# Version 0.5.5 (2025-04-16)

Jist of changes are follows:
- Fixed issue with Windows Builds crashing with Segmentation Fault when exiting.
- Fixed GUI to have instance based interaction instead of global.
- Fixed GS Reset to check for MK2 model.
- Reworked icon. (Added SVG and ICO formats)

# Version 0.5.4 (2025-04-14)

Jist of changes are follows:
- Add SRAM read and write support for SC-55mk1, SC-55mk2, JV880, SC-155, SC-155mk2.
- Add NVRAM read and write support for JV880.
- Updated `DEV_SSR` value to be SC-55 accurate.
- Fixed MIDI TX and SERIAL TX.
- SC-55mk2 now has default GS Reset if SRAM read fails.
- Added SDL Application name.
- Added LCD Fade effect.

# Version 0.5.3 (2025-04-08)

Jist of changes are follows:
- Updated SC-55 background.
- Added Serial IO Support for SC-55mk2 and SC-55st.
- Prints controls when using in JV880 and SC-55mk1/mk2 mode.
- Fixed bugs in MIDI Out.
- Improved volume handling in GUI.

# Version 0.5.2 (2025-04-03)

Jist of changes are follows:
- Updated background to add SC-55 support.
- Fixed bug with MIDI Out Mask.
- Removed back.data.

# Version 0.5.1 (2025-04-01)

- Synced to changes from [jcmoyer/Nuked-SC55#9842310](https://github.com/jcmoyer/Nuked-SC55/commit/984231094fcb977af9f22a7c30e129ee8afc7914)

Jist of changes are follows:
- Added ASIO Supported builds. (Currently on SDL builds for Windows are released)
- Rework to remove SDL dependency for backend and renderer.
- Update to handle Non-English paths in Windows.
- Some code cleanup.

# Version 0.4.1 (2025-02-17)

- Synced to changes from [jcmoyer/Nuked-SC55#9f43062](https://github.com/jcmoyer/Nuked-SC55/commit/9f4306240f8edab806ceaced3917cd18e9412642)

Jist of changes are follows:
- Made frontend run independently of emulator, therefore the frontend will be responsive even if emulator falls behind.
- Fixed bug with f32 wav rendering. (Previously wav files had long periods of silence in some players)
- renderer ignores extra data at the end if MIDI files have them after end-of-track event.
- The renderer will no longer ask the user to submit a bug report when the instance count is set to a number that results in only some instances having a midi track to render.

# Version 0.3.2 (2024-11-09)

- Improved Windows MIDI Handling code.
- Updated JV880 LCD contrast code.
- Added midi out callback to handle MIDI/SysEx out.
- Updated renderer code.

# Version 0.3.2 (2024-11-09)

- Minor rework of Code
- Volume adjustment works properly.
- Updated to add and fix renderer.

# Version 0.3.1 (2024-11-05)

Initial release